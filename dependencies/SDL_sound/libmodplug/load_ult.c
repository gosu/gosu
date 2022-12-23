/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"

#define ULT_16BIT   0x04
#define ULT_LOOP    0x08
#define ULT_BIDI    0x10

#pragma pack(1)

// Raw ULT header struct:
typedef struct tagULTHEADER
{
        char id[15];             // changed from CHAR
        char songtitle[32];      // changed from CHAR
	BYTE reserved;
} ULTHEADER;


// Raw ULT sampleinfo struct:
typedef struct tagULTSAMPLE
{
	CHAR samplename[32];
	CHAR dosname[12];
	LONG loopstart;
	LONG loopend;
	LONG sizestart;
	LONG sizeend;
	BYTE volume;
	BYTE flags;
	WORD finetune;
} ULTSAMPLE;

#pragma pack()


BOOL CSoundFile_ReadUlt(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	ULTHEADER *pmh = (ULTHEADER *)lpStream;
	ULTSAMPLE *pus;
	UINT nos, nop, j;
	DWORD dwMemPos = 0;
	UINT smpsize = 64;

	// try to read module header
	if ((!lpStream) || (dwMemLength < 0x100)) return FALSE;
	if (SDL_strncmp(pmh->id,"MAS_UTrack_V00",14)) return FALSE;
	// Warning! Not supported ULT format, trying anyway
	// if ((pmh->id[14] < '1') || (pmh->id[14] > '4')) return FALSE;
	_this->m_nType = MOD_TYPE_ULT;
	_this->m_nDefaultSpeed = 6;
	_this->m_nDefaultTempo = 125;
	// read songtext
	dwMemPos = sizeof(ULTHEADER);
	if ((pmh->reserved) && (dwMemPos + pmh->reserved * 32 < dwMemLength))
	{
		UINT len = pmh->reserved * 32;
		dwMemPos += len;
	}
	if (dwMemPos >= dwMemLength) return TRUE;
	nos = lpStream[dwMemPos++];
	_this->m_nSamples = nos;
	if (_this->m_nSamples >= MAX_SAMPLES) _this->m_nSamples = MAX_SAMPLES-1;
	if (pmh->id[14] >= '4')	smpsize += 2;
	if (dwMemPos + nos*smpsize + 256 + 2 > dwMemLength) return TRUE;
	for (j=1; j<=nos; j++, dwMemPos+=smpsize) if (j<=_this->m_nSamples)
	{
		MODINSTRUMENT *pins;
		pus	= (ULTSAMPLE *)(lpStream+dwMemPos);
		pins = &_this->Ins[j];
		pins->nLoopStart = pus->loopstart;
		pins->nLoopEnd = pus->loopend;
		pins->nLength = pus->sizeend - pus->sizestart;
		pins->nVolume = pus->volume;
		pins->nGlobalVol = 64;
		pins->nC4Speed = 8363;
		if (pmh->id[14] >= '4')
		{
			pins->nC4Speed = pus->finetune;
		}
		if (pus->flags & ULT_LOOP) pins->uFlags |= CHN_LOOP;
		if (pus->flags & ULT_BIDI) pins->uFlags |= CHN_PINGPONGLOOP;
		if (pus->flags & ULT_16BIT)
		{
			pins->uFlags |= CHN_16BIT;
			pins->nLoopStart >>= 1;
			pins->nLoopEnd >>= 1;
		}
	}
	SDL_memcpy(_this->Order, lpStream+dwMemPos, 256);
	dwMemPos += 256;
	_this->m_nChannels = lpStream[dwMemPos] + 1;
	nop = lpStream[dwMemPos+1] + 1;
	dwMemPos += 2;
	if (_this->m_nChannels > 32) _this->m_nChannels = 32;
	// Default channel settings
	for (j=0; j<_this->m_nChannels; j++)
	{
		_this->ChnSettings[j].nVolume = 64;
		_this->ChnSettings[j].nPan = (j & 1) ? 0x40 : 0xC0;
	}
	// read pan position table for v1.5 and higher
	if(pmh->id[14]>='3')
	{
		if (dwMemPos + _this->m_nChannels > dwMemLength) return TRUE;
		for(j=0; j<_this->m_nChannels; j++)
		{
			_this->ChnSettings[j].nPan = (lpStream[dwMemPos++] << 4) + 8;
			if (_this->ChnSettings[j].nPan > 256) _this->ChnSettings[j].nPan = 256;
		}
	}
	// Allocating Patterns
	for (j=0; j<nop; j++)
	{
		if (j < MAX_PATTERNS)
		{
			_this->PatternSize[j] = 64;
			_this->Patterns[j] = CSoundFile_AllocatePattern(64, _this->m_nChannels);
		}
	}
	// Reading Patterns
	for (j=0; j<_this->m_nChannels; j++)
	{
		UINT nPat=0;
		for ( ; nPat<nop; nPat++)
		{
			MODCOMMAND *pat = NULL;
			UINT row = 0;

			if (nPat < MAX_PATTERNS)
			{
				pat = _this->Patterns[nPat];
				if (pat) pat += j;
			}
			while (row < 64)
			{
				UINT rep, note, instr, eff, dat1, dat2, cmd1, cmd2;
				if (dwMemPos > dwMemLength - 5) return TRUE;
				rep = 1;
				note = lpStream[dwMemPos++];
				if (note == 0xFC)
				{
					rep = lpStream[dwMemPos];
					note = lpStream[dwMemPos+1];
					dwMemPos += 2;
					if (dwMemPos > dwMemLength - 4) return TRUE;
				}

				instr = lpStream[dwMemPos++];
				eff = lpStream[dwMemPos++];
				dat1 = lpStream[dwMemPos++];
				dat2 = lpStream[dwMemPos++];
				cmd1 = eff & 0x0F;
				cmd2 = eff >> 4;
				if (cmd1 == 0x0C) dat1 >>= 2; else
				if (cmd1 == 0x0B) { cmd1 = dat1 = 0; }
				if (cmd2 == 0x0C) dat2 >>= 2; else
				if (cmd2 == 0x0B) { cmd2 = dat2 = 0; }
				while ((rep != 0) && (row < 64))
				{
					if (pat)
					{
						pat->instr = instr;
						if (note) pat->note = note + 36;
						if (cmd1 | dat1)
						{
							if (cmd1 == 0x0C)
							{
								pat->volcmd = VOLCMD_VOLUME;
								pat->vol = dat1;
							} else
							{
								pat->command = cmd1;
								pat->param = dat1;
								CSoundFile_ConvertModCommand(_this, pat);
							}
						}
						if (cmd2 == 0x0C)
						{
							pat->volcmd = VOLCMD_VOLUME;
							pat->vol = dat2;
						} else
						if ((cmd2 | dat2) && (!pat->command))
						{
							pat->command = cmd2;
							pat->param = dat2;
							CSoundFile_ConvertModCommand(_this, pat);
						}
						pat += _this->m_nChannels;
					}
					row++;
					rep--;
				}
			}
		}
	}
	// Reading Instruments
	for (j=1; j<=_this->m_nSamples; j++) if (_this->Ins[j].nLength)
	{
		UINT flags;
		if (dwMemPos >= dwMemLength) return TRUE;
		flags = (_this->Ins[j].uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
		dwMemPos += CSoundFile_ReadSample(_this, &_this->Ins[j], flags, (LPSTR)(lpStream+dwMemPos), dwMemLength - dwMemPos);
	}
	return TRUE;
}
