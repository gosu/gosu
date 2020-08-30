/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"

//////////////////////////////////////////////////////////
// MTM file support (import only)

#pragma pack(1)

typedef struct tagMTMSAMPLE
{
        char samplename[22];      // changed from CHAR
	DWORD length;
	DWORD reppos;
	DWORD repend;
	CHAR finetune;
	BYTE volume;
	BYTE attribute;
} MTMSAMPLE;


typedef struct tagMTMHEADER
{
	char id[4];	    // MTM file marker + version // changed from CHAR
	char songname[20];  // ASCIIZ songname  // changed from CHAR
	WORD numtracks;	    // number of tracks saved
	BYTE lastpattern;   // last pattern number saved
	BYTE lastorder;	    // last order number to play (songlength-1)
	WORD commentsize;   // length of comment field
	BYTE numsamples;    // number of samples saved
	BYTE attribute;	    // attribute byte (unused)
	BYTE beatspertrack;
	BYTE numchannels;   // number of channels used
	BYTE panpos[32];    // voice pan positions
} MTMHEADER;

#pragma pack()


BOOL CSoundFile_ReadMTM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	MTMHEADER *pmh = (MTMHEADER *)lpStream;
	DWORD dwMemPos = 66;

	if ((!lpStream) || (dwMemLength < 0x100)) return FALSE;
	if ((SDL_strncmp(pmh->id, "MTM", 3)) || (pmh->numchannels > 32)
	 || (pmh->numsamples >= MAX_SAMPLES) || (!pmh->numsamples)
	 || (!pmh->numtracks) || (!pmh->numchannels)
	 || (!pmh->lastpattern) || (pmh->lastpattern >= MAX_PATTERNS))
		return FALSE;
	if (dwMemPos + 37*pmh->numsamples + 128 + 192*pmh->numtracks
	 + 64 * (pmh->lastpattern+1) + pmh->commentsize >= dwMemLength) 
		return FALSE;
	_this->m_nType = MOD_TYPE_MTM;
	_this->m_nSamples = pmh->numsamples;
	_this->m_nChannels = pmh->numchannels;
	// Reading instruments
	for	(UINT i=1; i<=_this->m_nSamples; i++)
	{
		MTMSAMPLE *pms = (MTMSAMPLE *)(lpStream + dwMemPos);
		_this->Ins[i].nVolume = pms->volume << 2;
		_this->Ins[i].nGlobalVol = 64;
		DWORD len = pms->length;
		if ((len > 4) && (len <= MAX_SAMPLE_LENGTH))
		{
			_this->Ins[i].nLength = len;
			_this->Ins[i].nLoopStart = pms->reppos;
			_this->Ins[i].nLoopEnd = pms->repend;
			if (_this->Ins[i].nLoopEnd > _this->Ins[i].nLength) 
				_this->Ins[i].nLoopEnd = _this->Ins[i].nLength;
			if (_this->Ins[i].nLoopStart + 4 >= _this->Ins[i].nLoopEnd) 
				_this->Ins[i].nLoopStart = _this->Ins[i].nLoopEnd = 0;
			if (_this->Ins[i].nLoopEnd) _this->Ins[i].uFlags |= CHN_LOOP;
			_this->Ins[i].nFineTune = MOD2XMFineTune(pms->finetune);
			if (pms->attribute & 0x01)
			{
				_this->Ins[i].uFlags |= CHN_16BIT;
				_this->Ins[i].nLength >>= 1;
				_this->Ins[i].nLoopStart >>= 1;
				_this->Ins[i].nLoopEnd >>= 1;
			}
			_this->Ins[i].nPan = 128;
		}
		dwMemPos += 37;
	}
	// Setting Channel Pan Position
	for (UINT ich=0; ich<_this->m_nChannels; ich++)
	{
		_this->ChnSettings[ich].nPan = ((pmh->panpos[ich] & 0x0F) << 4) + 8;
		_this->ChnSettings[ich].nVolume = 64;
	}
	// Reading pattern order
	SDL_memcpy(_this->Order, lpStream + dwMemPos, pmh->lastorder+1);
	dwMemPos += 128;
	// Reading Patterns
	LPCBYTE pTracks = lpStream + dwMemPos;
	dwMemPos += 192 * pmh->numtracks;
	LPWORD pSeq = (LPWORD)(lpStream + dwMemPos);
	for (UINT pat=0; pat<=pmh->lastpattern; pat++)
	{
		_this->PatternSize[pat] = 64;
		if ((_this->Patterns[pat] = CSoundFile_AllocatePattern(64, _this->m_nChannels)) == NULL) break;
		for (UINT n=0; n<32; n++) if ((pSeq[n]) && (pSeq[n] <= pmh->numtracks) && (n < _this->m_nChannels))
		{
			LPCBYTE p = pTracks + 192 * (pSeq[n]-1);
			MODCOMMAND *m = _this->Patterns[pat] + n;
			for (UINT i=0; i<64; i++, m+=_this->m_nChannels, p+=3)
			{
				if (p[0] & 0xFC) m->note = (p[0] >> 2) + 37;
				m->instr = ((p[0] & 0x03) << 4) | (p[1] >> 4);
				UINT cmd = p[1] & 0x0F;
				UINT param = p[2];
				if (cmd == 0x0A)
				{
					if (param & 0xF0) param &= 0xF0; else param &= 0x0F;
				}
				m->command = cmd;
				m->param = param;
				if ((cmd) || (param)) CSoundFile_ConvertModCommand(_this, m);
			}
		}
		pSeq += 32;
	}
	dwMemPos += 64*(pmh->lastpattern+1);
	dwMemPos += pmh->commentsize;
	// Reading Samples
	for (UINT ismp=1; ismp<=_this->m_nSamples; ismp++)
	{
		if (dwMemPos >= dwMemLength) break;
		dwMemPos += CSoundFile_ReadSample(_this, &_this->Ins[ismp], (_this->Ins[ismp].uFlags & CHN_16BIT) ? RS_PCM16U : RS_PCM8U,
								(LPSTR)(lpStream + dwMemPos), dwMemLength - dwMemPos);
	}
	_this->m_nMinPeriod = 64;
	_this->m_nMaxPeriod = 32767;
	return TRUE;
}

