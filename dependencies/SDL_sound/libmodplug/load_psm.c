/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

///////////////////////////////////////////////////
//
// PSM module loader
//
///////////////////////////////////////////////////
#include "libmodplug.h"

#define PSM_ID_NEW	0x204d5350
#define PSM_ID_OLD	0xfe4d5350
#define IFFID_FILE	0x454c4946
#define IFFID_TITL	0x4c544954
#define IFFID_SDFT	0x54464453
#define IFFID_PBOD	0x444f4250
#define IFFID_SONG	0x474e4f53
#define IFFID_PATT	0x54544150
#define IFFID_DSMP	0x504d5344
#define IFFID_OPLH	0x484c504f

#pragma pack(1)

typedef struct _PSMCHUNK
{
	DWORD id;
	DWORD len;
	DWORD listid;
} PSMCHUNK;

void swap_PSMCHUNK(PSMCHUNK* p){
	p->id = bswapLE32(p->id);
	p->len = bswapLE32(p->len);
	p->listid = bswapLE32(p->listid);
}

typedef struct _PSMSONGHDR
{
	CHAR songname[8];	// "MAINSONG"
	BYTE reserved1;
	BYTE reserved2;
	BYTE channels;
} PSMSONGHDR;

typedef struct _PSMPATTERN
{
	DWORD size;
	DWORD name;
	WORD rows;
	WORD reserved1;
	BYTE data[4];
} PSMPATTERN;

void swap_PSMPATTERN(PSMPATTERN* p){
	p->size = bswapLE32(p->size);
	p->name = bswapLE32(p->name);
	p->rows = bswapLE16(p->rows);
}

typedef struct _PSMSAMPLE
{
	BYTE flags;
	CHAR songname[8];
	DWORD smpid;
	CHAR samplename[34];
	DWORD reserved1;
	BYTE reserved2;
	BYTE insno;
	BYTE reserved3;
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	WORD reserved4;
	BYTE defvol;
	DWORD reserved5;
	DWORD samplerate;
	BYTE reserved6[19];
} PSMSAMPLE;

void swap_PSMSAMPLE(PSMSAMPLE* p){
	p->smpid = bswapLE32(p->smpid);
	p->length = bswapLE32(p->length);
	p->loopstart = bswapLE32(p->loopstart);
	p->loopend = bswapLE32(p->loopend);
	p->samplerate = bswapLE32(p->samplerate);
}

#pragma pack()


BOOL CSoundFile_ReadPSM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	PSMCHUNK *pfh = (PSMCHUNK *)lpStream;
	DWORD dwMemPos, dwSongPos;
	DWORD smpnames[MAX_SAMPLES];
	DWORD patptrs[MAX_PATTERNS];
	BYTE samplemap[MAX_SAMPLES];
	UINT nPatterns;
	
	// Swap chunk
	swap_PSMCHUNK(pfh);
	
	// Chunk0: "PSM ",filesize,"FILE"
	if (dwMemLength < 256) return FALSE;
	if (pfh->id == PSM_ID_OLD)
	{
		return FALSE;
	}
	if ((pfh->id != PSM_ID_NEW) || (pfh->len+12 > dwMemLength) || (pfh->listid != IFFID_FILE)) return FALSE;
	_this->m_nType = MOD_TYPE_PSM;
	_this->m_nChannels = 16;
	_this->m_nSamples = 0;
	nPatterns = 0;
	dwMemPos = 12;
	dwSongPos = 0;
	for (UINT iChPan=0; iChPan<16; iChPan++)
	{
		UINT pan = (((iChPan & 3) == 1) || ((iChPan&3)==2)) ? 0xC0 : 0x40;
		_this->ChnSettings[iChPan].nPan = pan;
	}
	while (dwMemPos+8 < dwMemLength)
	{
		PSMCHUNK *pchunk = (PSMCHUNK *)(lpStream+dwMemPos);
		swap_PSMCHUNK(pchunk);
		if ((pchunk->len >= dwMemLength - 8) || (dwMemPos + pchunk->len + 8 > dwMemLength)) break;
		dwMemPos += 8;
		PUCHAR pdata = (PUCHAR)(lpStream+dwMemPos);
		ULONG len = pchunk->len;
		if (len) switch(pchunk->id)
		{
		// "PBOD": Pattern
		case IFFID_PBOD:
			if ((len >= 12) && (nPatterns < MAX_PATTERNS))
			{
				patptrs[nPatterns++] = dwMemPos-8;
			}
			break;
		// "SONG": Song description
		case IFFID_SONG:
			if ((len >= sizeof(PSMSONGHDR)+8) && (!dwSongPos))
			{
				dwSongPos = dwMemPos - 8;
			}
			break;
		// "DSMP": Sample Data
		case IFFID_DSMP:
			if ((len >= sizeof(PSMSAMPLE)) && (_this->m_nSamples+1 < MAX_SAMPLES))
			{
				_this->m_nSamples++;
				MODINSTRUMENT *pins = &_this->Ins[_this->m_nSamples];
				PSMSAMPLE *psmp = (PSMSAMPLE *)pdata;
				swap_PSMSAMPLE(psmp);
				smpnames[_this->m_nSamples] = psmp->smpid;
				samplemap[_this->m_nSamples-1] = (BYTE)_this->m_nSamples;
				// Init sample
				pins->nGlobalVol = 0x40;
				pins->nC4Speed = psmp->samplerate;
				pins->nLength = psmp->length;
				pins->nLoopStart = psmp->loopstart;
				pins->nLoopEnd = psmp->loopend;
				pins->nPan = 128;
				pins->nVolume = (psmp->defvol+1) * 2;
				pins->uFlags = (psmp->flags & 0x80) ? CHN_LOOP : 0;
				if (pins->nLoopStart > 0) pins->nLoopStart--;
				// Point to sample data
				pdata += 0x60;
				len -= 0x60;
				// Load sample data
				if ((pins->nLength > 3) && (len > 3))
				{
					CSoundFile_ReadSample(_this, pins, RS_PCM8D, (LPCSTR)pdata, len);
				} else
				{
					pins->nLength = 0;
				}
			}
			break;
		}
		dwMemPos += pchunk->len;
	}
	// Step #1: convert song structure
	PSMSONGHDR *pSong = (PSMSONGHDR *)(lpStream+dwSongPos+8);
	if ((!dwSongPos) || (pSong->channels < 2) || (pSong->channels > 32)) return TRUE;
	_this->m_nChannels = pSong->channels;
	// Valid song header -> convert attached chunks
	{
		DWORD dwSongEnd = dwSongPos + 8 + *(DWORD *)(lpStream+dwSongPos+4);
		dwMemPos = dwSongPos + 8 + 11; // sizeof(PSMCHUNK)+sizeof(PSMSONGHDR)
		while (dwMemPos + 8 < dwSongEnd)
		{
			PSMCHUNK *pchunk = (PSMCHUNK *)(lpStream+dwMemPos);
			swap_PSMCHUNK(pchunk);
			dwMemPos += 8;
			if ((pchunk->len > dwSongEnd) || (dwMemPos + pchunk->len > dwSongEnd)) break;
			PUCHAR pdata = (PUCHAR)(lpStream+dwMemPos);
			ULONG len = pchunk->len;
			switch(pchunk->id)
			{
			case IFFID_OPLH:
				if (len >= 0x20)
				{
					UINT pos = len - 3;
					while (pos > 5)
					{
						BOOL bFound = FALSE;
						pos -= 5;
						DWORD dwName = *(DWORD *)(pdata+pos);
						for (UINT i=0; i<nPatterns; i++)
						{
							DWORD dwPatName = ((PSMPATTERN *)(lpStream+patptrs[i]+8))->name;
							if (dwName == dwPatName)
							{
								bFound = TRUE;
								break;
							}
						}
						if ((!bFound) && (pdata[pos+1] > 0) && (pdata[pos+1] <= 0x10)
						 && (pdata[pos+3] > 0x40) && (pdata[pos+3] < 0xC0))
						{
							_this->m_nDefaultSpeed = pdata[pos+1];
							_this->m_nDefaultTempo = pdata[pos+3];
							break;
						}
					}
					UINT iOrd = 0;
					while ((pos+5<len) && (iOrd < MAX_ORDERS))
					{
						DWORD dwName = *(DWORD *)(pdata+pos);
						for (UINT i=0; i<nPatterns; i++)
						{
							DWORD dwPatName = ((PSMPATTERN *)(lpStream+patptrs[i]+8))->name;
							if (dwName == dwPatName)
							{
								_this->Order[iOrd++] = i;
								break;
							}
						}
						pos += 5;
					}
				}
				break;
			}
			dwMemPos += pchunk->len;
		}
	}

	// Step #2: convert patterns
	for (UINT nPat=0; nPat<nPatterns; nPat++)
	{
		PSMPATTERN *pPsmPat = (PSMPATTERN *)(lpStream+patptrs[nPat]+8);
		swap_PSMPATTERN(pPsmPat);
		ULONG len = *(DWORD *)(lpStream+patptrs[nPat]+4) - 12;
		UINT nRows = pPsmPat->rows;
		if (len > pPsmPat->size) len = pPsmPat->size;
		if ((nRows < 64) || (nRows > 256)) nRows = 64;
		_this->PatternSize[nPat] = nRows;
		if ((_this->Patterns[nPat] = CSoundFile_AllocatePattern(nRows, _this->m_nChannels)) == NULL) break;
		MODCOMMAND *m = _this->Patterns[nPat];
		BYTE *p = pPsmPat->data;
		UINT pos = 0;
		UINT row = 0;
		UINT oldch = 0;
		BOOL bNewRow = FALSE;
		while ((row < nRows) && (pos+1 < len))
		{
			UINT flags = p[pos++];
			UINT ch = p[pos++];
			
			if (((flags & 0xf0) == 0x10) && (ch <= oldch) /*&& (!bNewRow)*/)
			{
				if ((pos+1<len) && (!(p[pos] & 0x0f)) && (p[pos+1] < _this->m_nChannels))
				{
					row++;
					m += _this->m_nChannels;
					oldch = ch;
					continue;
				}
			}
			if ((pos >= len) || (row >= nRows)) break;
			if (!(flags & 0xf0))
			{
				row++;
				m += _this->m_nChannels;
				bNewRow = TRUE;
				oldch = ch;
				continue;
			}
			bNewRow = FALSE;
			if (ch >= _this->m_nChannels)
			{
				ch = 0;
			}
			// Note + Instr
			if ((flags & 0x40) && (pos+1 < len))
			{
				UINT note = p[pos++];
				UINT nins = p[pos++];
				if ((note) && (note < 0x80)) note = (note>>4)*12+(note&0x0f)+12+1;
				m[ch].instr = samplemap[nins];
				m[ch].note = note;
			}
			// Volume
			if ((flags & 0x20) && (pos < len))
			{
				m[ch].volcmd = VOLCMD_VOLUME;
				m[ch].vol = p[pos++] / 2;
			}
			// Effect
			if ((flags & 0x10) && (pos+1 < len))
			{
				UINT command = p[pos++];
				UINT param = p[pos++];
				// Convert effects
				switch(command)
				{
				// 01: fine volslide up
				case 0x01:	command = CMD_VOLUMESLIDE; param |= 0x0f; break;
				// 04: fine volslide down
				case 0x04:	command = CMD_VOLUMESLIDE; param>>=4; param |= 0xf0; break;
				// 0C: portamento up
				case 0x0C:	command = CMD_PORTAMENTOUP; param = (param+1)/2; break;
				// 0E: portamento down
				case 0x0E:	command = CMD_PORTAMENTODOWN; param = (param+1)/2; break;
				// 33: Position Jump
				case 0x33:	command = CMD_POSITIONJUMP; break;
				// 34: Pattern break
				case 0x34:	command = CMD_PATTERNBREAK; break;
				// 3D: speed
				case 0x3D:	command = CMD_SPEED; break;
				// 3E: tempo
				case 0x3E:	command = CMD_TEMPO; break;
				// Unknown
				default:
					command = param = 0;
				}
				m[ch].command = (BYTE)command;
				m[ch].param = (BYTE)param;
			}
			oldch = ch;
		}
	}

	// Done (finally!)
	return TRUE;
}

