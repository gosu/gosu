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
	UINT i;
	
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
	for (i=0; i < nOrders; i++)
	{
		_this->Order[i] = lpStream[dwMemPos+i*2+1];
		if (i >= MAX_ORDERS-2) break;
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
			// Skip duplicate chunks.
			if (_this->m_nInstruments) continue;

			if (nInstruments >= MAX_INSTRUMENTS) nInstruments = MAX_INSTRUMENTS-1;
			for (i=0; i<nInstruments; i++)
			{
				MODINSTRUMENT *psmp;
				INSTRUMENTHEADER *penv;
				DBMINSTRUMENT *pih;
				UINT nsmp, j;

				if (chunk_pos + sizeof(DBMINSTRUMENT) > dwMemPos) break;
				if ((penv = (INSTRUMENTHEADER *) SDL_calloc(1, sizeof (INSTRUMENTHEADER))) == NULL) break;
				pih = (DBMINSTRUMENT *)(lpStream+chunk_pos);
				nsmp = bswapBE16(pih->sampleno);
				psmp = ((nsmp) && (nsmp < MAX_SAMPLES)) ? &_this->Ins[nsmp] : NULL;
				_this->Headers[i+1] = penv;
				penv->nFadeOut = 1024;	// ???
				penv->nGlobalVol = 64;
				penv->nPan = bswapBE16(pih->panning);
				if ((penv->nPan) && (penv->nPan < 256))
					penv->dwFlags = ENV_SETPANNING;
				else
					penv->nPan = 128;
				penv->nPPC = 5*12;
				for (j=0; j<NOTE_MAX; j++)
				{
					penv->Keyboard[j] = nsmp;
					penv->NoteMap[j] = j+1;
				}
				// Sample Info
				if (psmp)
				{
					DWORD sflags = bswapBE16(pih->flags);
					int f2t;
					psmp->nVolume = bswapBE16(pih->volume) * 4;
					if ((!psmp->nVolume) || (psmp->nVolume > 256)) psmp->nVolume = 256;
					psmp->nGlobalVol = 64;
					psmp->nC4Speed = bswapBE32(pih->finetune);
					f2t = CSoundFile_FrequencyToTranspose(psmp->nC4Speed);
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
				_this->m_nInstruments = i+1;
			}
		} else
		// Volume Envelopes
		if (chunk_id == bswapLE32(DBM_ID_VENV))
		{
			UINT nEnvelopes = lpStream[chunk_pos+1];
			
			chunk_pos += 2;
			for (i=0; i<nEnvelopes; i++)
			{
				DBMENVELOPE *peh;
				UINT nins;
				
				if (chunk_pos + sizeof(DBMENVELOPE) > dwMemPos) break;
				peh = (DBMENVELOPE *)(lpStream+chunk_pos);
				nins = bswapBE16(peh->instrument);
				if ((nins) && (nins < MAX_INSTRUMENTS) && (_this->Headers[nins]) && (peh->numpoints))
				{
					INSTRUMENTHEADER *penv = _this->Headers[nins];
					UINT j;

					if (peh->flags & 1) penv->dwFlags |= ENV_VOLUME;
					if (peh->flags & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
					if (peh->flags & 4) penv->dwFlags |= ENV_VOLLOOP;
					penv->nVolEnv = peh->numpoints + 1;
					if (penv->nVolEnv > MAX_ENVPOINTS) penv->nVolEnv = MAX_ENVPOINTS;
					penv->nVolLoopStart = peh->loopbegin;
					penv->nVolLoopEnd = peh->loopend;
					penv->nVolSustainBegin = penv->nVolSustainEnd = peh->sustain1;
					for (j=0; j<penv->nVolEnv; j++)
					{
						penv->VolPoints[j] = bswapBE16(peh->volenv[j*2]);
						penv->VolEnv[j] = (BYTE)bswapBE16(peh->volenv[j*2+1]);
					}
				}
				chunk_pos += sizeof(DBMENVELOPE);
			}
		} else
		// Packed Pattern Data
		if (chunk_id == bswapLE32(DBM_ID_PATT))
		{
			if (nPatterns > MAX_PATTERNS) nPatterns = MAX_PATTERNS;
			for (i=0; i<nPatterns; i++)
			{
				DBMPATTERN *pph;
				DWORD pksize;
				UINT nRows;

				// Skip duplicate chunks.
				if (_this->Patterns[i]) break;

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
						LPBYTE pkdata = (LPBYTE)pph->patterndata;
						UINT row = 0;
						UINT n = 0;

						_this->PatternSize[i] = nRows;
						_this->Patterns[i] = m;
						while ((n+3<pksize) && (row < nRows))
						{
							UINT ch = pkdata[n++];

							if (ch)
							{
								BYTE b = pkdata[n++];
								ch--;
								if (ch < _this->m_nChannels)
								{
									if (b & 0x01)
									{
										UINT note = pkdata[n++];

										if (note == 0x1F) note = 0xFF; else
										if ((note) && (note < 0xFE))
										{
											note = ((note >> 4)*12) + (note & 0x0F) + 13;
										}
										m[ch].note = note;
									}
									if (b & 0x02) m[ch].instr = pkdata[n++];
									if (b & 0x3C)
									{
										UINT cmd1 = 0xFF, param1 = 0, cmd2 = 0xFF, param2 = 0;
										if (b & 0x04) cmd1 = (UINT)pkdata[n++];
										if (b & 0x08) param1 = pkdata[n++];
										if (b & 0x10) cmd2 = (UINT)pkdata[n++];
										if (b & 0x20) param2 = pkdata[n++];
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
									if (b & 0x01) n++;
									if (b & 0x02) n++;
									if (b & 0x04) n++;
									if (b & 0x08) n++;
									if (b & 0x10) n++;
									if (b & 0x20) n++;
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
			for (i=1; i<=nSamples; i++)
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
				pins = &_this->Ins[i];
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
