/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

///////////////////////////////////////////////////////////////
//
// DigiBooster Pro Module Loader (*.dbm)
//
// Note: this loader doesn't handle multiple songs
//
///////////////////////////////////////////////////////////////

#include "libmodplug.h"

#define DBM_FILE_MAGIC	0x304d4244
#define DBM_ID_NAME		0x454d414e
#define DBM_NAMELEN		0x2c000000
#define DBM_ID_INFO		0x4f464e49
#define DBM_INFOLEN		0x0a000000
#define DBM_ID_SONG		0x474e4f53
#define DBM_ID_INST		0x54534e49
#define DBM_ID_VENV		0x564e4556
#define DBM_ID_PATT		0x54544150
#define DBM_ID_SMPL		0x4c504d53

#pragma pack(1)

typedef struct DBMFILEHEADER
{
	DWORD dbm_id;		// "DBM0" = 0x304d4244
	WORD trkver;		// Tracker version: 02.15
	WORD reserved;
	DWORD name_id;		// "NAME" = 0x454d414e
	DWORD name_len;		// name length: always 44
	CHAR songname[44];
	DWORD info_id;		// "INFO" = 0x4f464e49
	DWORD info_len;		// 0x0a000000
	WORD instruments;
	WORD samples;
	WORD songs;
	WORD patterns;
	WORD channels;
	DWORD song_id;		// "SONG" = 0x474e4f53
	DWORD song_len;
	CHAR songname2[44];
	WORD orders;
//	WORD orderlist[0];	// orderlist[orders] in words
} DBMFILEHEADER;

typedef struct DBMINSTRUMENT
{
	CHAR name[30];
	WORD sampleno;
	WORD volume;
	DWORD finetune;
	DWORD loopstart;
	DWORD looplen;
	WORD panning;
	WORD flags;
} DBMINSTRUMENT;

typedef struct DBMENVELOPE
{
	WORD instrument;
	BYTE flags;
	BYTE numpoints;
	BYTE sustain1;
	BYTE loopbegin;
	BYTE loopend;
	BYTE sustain2;
	WORD volenv[2*32];
} DBMENVELOPE;

typedef struct DBMPATTERN
{
	WORD rows;
	DWORD packedsize;
	BYTE patterndata[2];	// [packedsize]
} DBMPATTERN;

typedef struct DBMSAMPLE
{
	DWORD flags;
	DWORD samplesize;
	BYTE sampledata[2];		// [samplesize]
} DBMSAMPLE;

#pragma pack()


BOOL CSoundFile_ReadDBM(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const DBMFILEHEADER *pfh = (DBMFILEHEADER *)lpStream;
	DWORD dwMemPos;
	UINT nOrders, nSamples, nInstruments, nPatterns;
	
	if ((!lpStream) || (dwMemLength <= sizeof(DBMFILEHEADER)) || (!pfh->channels)
	 || (pfh->dbm_id != bswapLE32(DBM_FILE_MAGIC)) || (!pfh->songs) || (pfh->song_id != bswapLE32(DBM_ID_SONG))
	 || (pfh->name_id != bswapLE32(DBM_ID_NAME)) || (pfh->name_len != bswapLE32(DBM_NAMELEN))
	 || (pfh->info_id != bswapLE32(DBM_ID_INFO)) || (pfh->info_len != bswapLE32(DBM_INFOLEN))) return FALSE;
	dwMemPos = sizeof(DBMFILEHEADER);
	nOrders = bswapBE16(pfh->orders);
	if (dwMemPos + 2 * nOrders + 8*3 >= dwMemLength) return FALSE;
	nInstruments = bswapBE16(pfh->instruments);
	nSamples = bswapBE16(pfh->samples);
	nPatterns = bswapBE16(pfh->patterns);
	_this->m_nType = MOD_TYPE_DBM;
	_this->m_nChannels = bswapBE16(pfh->channels);
	if (_this->m_nChannels < 4) _this->m_nChannels = 4;
	if (_this->m_nChannels > 64) _this->m_nChannels = 64;
	for (UINT iOrd=0; iOrd < nOrders; iOrd++)
	{
		_this->Order[iOrd] = lpStream[dwMemPos+iOrd*2+1];
		if (iOrd >= MAX_ORDERS-2) break;
	}
	dwMemPos += 2*nOrders;
	while (dwMemPos + 10 < dwMemLength)
	{
		DWORD chunk_id = ((LPDWORD)(lpStream+dwMemPos))[0];
		DWORD chunk_size = bswapBE32(((LPDWORD)(lpStream+dwMemPos))[1]);
		DWORD chunk_pos;
		
		dwMemPos += 8;
		chunk_pos = dwMemPos;
		if ((dwMemPos + chunk_size > dwMemLength) || (chunk_size > dwMemLength)) break;
		dwMemPos += chunk_size;
		// Instruments
		if (chunk_id == bswapLE32(DBM_ID_INST))
		{
			if (nInstruments >= MAX_INSTRUMENTS) nInstruments = MAX_INSTRUMENTS-1;
			for (UINT iIns=0; iIns<nInstruments; iIns++)
			{
				MODINSTRUMENT *psmp;
				INSTRUMENTHEADER *penv;
				DBMINSTRUMENT *pih;
				UINT nsmp;

				if (chunk_pos + sizeof(DBMINSTRUMENT) > dwMemPos) break;
				if ((penv = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER))) == NULL) break;
				pih = (DBMINSTRUMENT *)(lpStream+chunk_pos);
				nsmp = bswapBE16(pih->sampleno);
				psmp = ((nsmp) && (nsmp < MAX_SAMPLES)) ? &_this->Ins[nsmp] : NULL;
				SDL_memset(penv, 0, sizeof(INSTRUMENTHEADER));
				_this->Headers[iIns+1] = penv;
				penv->nFadeOut = 1024;	// ???
				penv->nGlobalVol = 64;
				penv->nPan = bswapBE16(pih->panning);
				if ((penv->nPan) && (penv->nPan < 256))
					penv->dwFlags = ENV_SETPANNING;
				else
					penv->nPan = 128;
				penv->nPPC = 5*12;
				for (UINT i=0; i<NOTE_MAX; i++)
				{
					penv->Keyboard[i] = nsmp;
					penv->NoteMap[i] = i+1;
				}
				// Sample Info
				if (psmp)
				{
					DWORD sflags = bswapBE16(pih->flags);
					psmp->nVolume = bswapBE16(pih->volume) * 4;
					if ((!psmp->nVolume) || (psmp->nVolume > 256)) psmp->nVolume = 256;
					psmp->nGlobalVol = 64;
					psmp->nC4Speed = bswapBE32(pih->finetune);
					int f2t = CSoundFile_FrequencyToTranspose(psmp->nC4Speed);
					psmp->RelativeTone = f2t >> 7;
					psmp->nFineTune = f2t & 0x7F;
					if ((pih->looplen) && (sflags & 3))
					{
						psmp->nLoopStart = bswapBE32(pih->loopstart);
						psmp->nLoopEnd = psmp->nLoopStart + bswapBE32(pih->looplen);
						psmp->uFlags |= CHN_LOOP;
						psmp->uFlags &= ~CHN_PINGPONGLOOP;
						if (sflags & 2) psmp->uFlags |= CHN_PINGPONGLOOP;
					}
				}
				chunk_pos += sizeof(DBMINSTRUMENT);
				_this->m_nInstruments = iIns+1;
			}
		} else
		// Volume Envelopes
		if (chunk_id == bswapLE32(DBM_ID_VENV))
		{
			UINT nEnvelopes = lpStream[chunk_pos+1];
			
			chunk_pos += 2;
			for (UINT iEnv=0; iEnv<nEnvelopes; iEnv++)
			{
				DBMENVELOPE *peh;
				UINT nins;
				
				if (chunk_pos + sizeof(DBMENVELOPE) > dwMemPos) break;
				peh = (DBMENVELOPE *)(lpStream+chunk_pos);
				nins = bswapBE16(peh->instrument);
				if ((nins) && (nins < MAX_INSTRUMENTS) && (_this->Headers[nins]) && (peh->numpoints))
				{
					INSTRUMENTHEADER *penv = _this->Headers[nins];

					if (peh->flags & 1) penv->dwFlags |= ENV_VOLUME;
					if (peh->flags & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
					if (peh->flags & 4) penv->dwFlags |= ENV_VOLLOOP;
					penv->nVolEnv = peh->numpoints + 1;
					if (penv->nVolEnv > MAX_ENVPOINTS) penv->nVolEnv = MAX_ENVPOINTS;
					penv->nVolLoopStart = peh->loopbegin;
					penv->nVolLoopEnd = peh->loopend;
					penv->nVolSustainBegin = penv->nVolSustainEnd = peh->sustain1;
					for (UINT i=0; i<penv->nVolEnv; i++)
					{
						penv->VolPoints[i] = bswapBE16(peh->volenv[i*2]);
						penv->VolEnv[i] = (BYTE)bswapBE16(peh->volenv[i*2+1]);
					}
				}
				chunk_pos += sizeof(DBMENVELOPE);
			}
		} else
		// Packed Pattern Data
		if (chunk_id == bswapLE32(DBM_ID_PATT))
		{
			if (nPatterns > MAX_PATTERNS) nPatterns = MAX_PATTERNS;
			for (UINT iPat=0; iPat<nPatterns; iPat++)
			{
				DBMPATTERN *pph;
				DWORD pksize;
				UINT nRows;

				if (chunk_pos + sizeof(DBMPATTERN) > dwMemPos) break;
				pph = (DBMPATTERN *)(lpStream+chunk_pos);
				pksize = bswapBE32(pph->packedsize);
				if ((chunk_pos + pksize + 6 > dwMemPos) || (pksize > dwMemPos)) break;
				nRows = bswapBE16(pph->rows);
				if ((nRows >= 4) && (nRows <= 256))
				{
					MODCOMMAND *m = CSoundFile_AllocatePattern(nRows, _this->m_nChannels);
					if (m)
					{
						LPBYTE pkdata = (LPBYTE)&pph->patterndata;
						UINT row = 0;
						UINT i = 0;

						_this->PatternSize[iPat] = nRows;
						_this->Patterns[iPat] = m;
						while ((i+3<pksize) && (row < nRows))
						{
							UINT ch = pkdata[i++];

							if (ch)
							{
								BYTE b = pkdata[i++];
								ch--;
								if (ch < _this->m_nChannels)
								{
									if (b & 0x01)
									{
										UINT note = pkdata[i++];

										if (note == 0x1F) note = 0xFF; else
										if ((note) && (note < 0xFE))
										{
											note = ((note >> 4)*12) + (note & 0x0F) + 13;
										}
										m[ch].note = note;
									}
									if (b & 0x02) m[ch].instr = pkdata[i++];
									if (b & 0x3C)
									{
										UINT cmd1 = 0xFF, param1 = 0, cmd2 = 0xFF, param2 = 0;
										if (b & 0x04) cmd1 = (UINT)pkdata[i++];
										if (b & 0x08) param1 = pkdata[i++];
										if (b & 0x10) cmd2 = (UINT)pkdata[i++];
										if (b & 0x20) param2 = pkdata[i++];
										if (cmd1 == 0x0C)
										{
											m[ch].volcmd = VOLCMD_VOLUME;
											m[ch].vol = param1;
											cmd1 = 0xFF;
										} else
										if (cmd2 == 0x0C)
										{
											m[ch].volcmd = VOLCMD_VOLUME;
											m[ch].vol = param2;
											cmd2 = 0xFF;
										}
										if ((cmd1 > 0x13) || ((cmd1 >= 0x10) && (cmd2 < 0x10)))
										{
											cmd1 = cmd2;
											param1 = param2;
											cmd2 = 0xFF;
										}
										if (cmd1 <= 0x13)
										{
											m[ch].command = cmd1;
											m[ch].param = param1;
											CSoundFile_ConvertModCommand(_this, &m[ch]);
										}
									}
								} else
								{
									if (b & 0x01) i++;
									if (b & 0x02) i++;
									if (b & 0x04) i++;
									if (b & 0x08) i++;
									if (b & 0x10) i++;
									if (b & 0x20) i++;
								}
							} else
							{
								row++;
								m += _this->m_nChannels;
							}
						}
					}
				}
				chunk_pos += 6 + pksize;
			}
		} else
		// Reading Sample Data
		if (chunk_id == bswapLE32(DBM_ID_SMPL))
		{
			if (nSamples >= MAX_SAMPLES) nSamples = MAX_SAMPLES-1;
			_this->m_nSamples = nSamples;
			for (UINT iSmp=1; iSmp<=nSamples; iSmp++)
			{
				MODINSTRUMENT *pins;
				DBMSAMPLE *psh;
				DWORD samplesize;
				DWORD sampleflags;

				if (chunk_pos + sizeof(DBMSAMPLE) >= dwMemPos) break;
				psh = (DBMSAMPLE *)(lpStream+chunk_pos);
				chunk_pos += 8;
				samplesize = bswapBE32(psh->samplesize);
				sampleflags = bswapBE32(psh->flags);
				pins = &_this->Ins[iSmp];
				pins->nLength = samplesize;
				if (sampleflags & 2)
				{
					pins->uFlags |= CHN_16BIT;
					samplesize <<= 1;
				}
				if ((chunk_pos+samplesize > dwMemPos) || (samplesize > dwMemLength)) break;
				if (sampleflags & 3)
				{
					CSoundFile_ReadSample(_this, pins, (pins->uFlags & CHN_16BIT) ? RS_PCM16M : RS_PCM8S,
								(LPSTR)(psh->sampledata), samplesize);
				}
				chunk_pos += samplesize;
			}
		}
	}
	return TRUE;
}

