/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

//////////////////////////////////////////////
// Oktalyzer (OKT) module loader            //
//////////////////////////////////////////////
#include "libmodplug.h"

typedef struct OKTFILEHEADER
{
	DWORD okta;		// "OKTA"
	DWORD song;		// "SONG"
	DWORD cmod;		// "CMOD"
	DWORD fixed8;
	BYTE chnsetup[8];
	DWORD samp;		// "SAMP"
	DWORD samplen;
} OKTFILEHEADER;


typedef struct OKTSAMPLE
{
	CHAR name[20];
	DWORD length;
	WORD loopstart;
	WORD looplen;
	BYTE pad1;
	BYTE volume;
	BYTE pad2;
	BYTE pad3;
} OKTSAMPLE;


BOOL CSoundFile_ReadOKT(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const OKTFILEHEADER *pfh = (OKTFILEHEADER *)lpStream;
	DWORD dwMemPos = sizeof(OKTFILEHEADER);
	UINT nsamples = 0, npatterns = 0, norders = 0;

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((pfh->okta != 0x41544B4F) || (pfh->song != 0x474E4F53)
	 || (pfh->cmod != 0x444F4D43) || (pfh->chnsetup[0]) || (pfh->chnsetup[2])
	 || (pfh->chnsetup[4]) || (pfh->chnsetup[6]) || (pfh->fixed8 != 0x08000000)
	 || (pfh->samp != 0x504D4153)) return FALSE;
	_this->m_nType = MOD_TYPE_OKT;
	_this->m_nChannels = 4 + pfh->chnsetup[1] + pfh->chnsetup[3] + pfh->chnsetup[5] + pfh->chnsetup[7];
	if (_this->m_nChannels > MAX_CHANNELS) _this->m_nChannels = MAX_CHANNELS;
	nsamples = bswapBE32(pfh->samplen) >> 5;
	_this->m_nSamples = nsamples;
	if (_this->m_nSamples >= MAX_SAMPLES) _this->m_nSamples = MAX_SAMPLES-1;
	// Reading samples
	for (UINT smp=1; smp <= nsamples; smp++)
	{
		if (dwMemPos >= dwMemLength) return TRUE;
		if (smp < MAX_SAMPLES)
		{
			OKTSAMPLE *psmp = (OKTSAMPLE *)(lpStream + dwMemPos);
			MODINSTRUMENT *pins = &_this->Ins[smp];
			pins->uFlags = 0;
			pins->nLength = bswapBE32(psmp->length) & ~1;
			pins->nLoopStart = bswapBE16(psmp->loopstart);
			pins->nLoopEnd = pins->nLoopStart + bswapBE16(psmp->looplen);
			if (pins->nLoopStart + 2 < pins->nLoopEnd) pins->uFlags |= CHN_LOOP;
			pins->nGlobalVol = 64;
			pins->nVolume = psmp->volume << 2;
			pins->nC4Speed = 8363;
		}
		dwMemPos += sizeof(OKTSAMPLE);
	}
	// SPEE
	if (dwMemPos >= dwMemLength) return TRUE;
	if (*((DWORD *)(lpStream + dwMemPos)) == 0x45455053)
	{
		_this->m_nDefaultSpeed = lpStream[dwMemPos+9];
		dwMemPos += bswapBE32(*((DWORD *)(lpStream + dwMemPos + 4))) + 8;
	}
	// SLEN
	if (dwMemPos >= dwMemLength) return TRUE;
	if (*((DWORD *)(lpStream + dwMemPos)) == 0x4E454C53)
	{
		npatterns = lpStream[dwMemPos+9];
		dwMemPos += bswapBE32(*((DWORD *)(lpStream + dwMemPos + 4))) + 8;
	}
	// PLEN
	if (dwMemPos >= dwMemLength) return TRUE;
	if (*((DWORD *)(lpStream + dwMemPos)) == 0x4E454C50)
	{
		norders = lpStream[dwMemPos+9];
		dwMemPos += bswapBE32(*((DWORD *)(lpStream + dwMemPos + 4))) + 8;
	}
	// PATT
	if (dwMemPos >= dwMemLength) return TRUE;
	if (*((DWORD *)(lpStream + dwMemPos)) == 0x54544150)
	{
		UINT orderlen = norders;
		if (orderlen >= MAX_ORDERS) orderlen = MAX_ORDERS-1;
		for (UINT i=0; i<orderlen; i++) _this->Order[i] = lpStream[dwMemPos+10+i];
		for (UINT j=orderlen; j>1; j--) { if (_this->Order[j-1]) break; _this->Order[j-1] = 0xFF; }
		dwMemPos += bswapBE32(*((DWORD *)(lpStream + dwMemPos + 4))) + 8;
	}
	// PBOD
	UINT npat = 0;
	while ((dwMemPos+10 < dwMemLength) && (*((DWORD *)(lpStream + dwMemPos)) == 0x444F4250))
	{
		DWORD dwPos = dwMemPos + 10;
		UINT rows = lpStream[dwMemPos+9];
		if (!rows) rows = 64;
		if (npat < MAX_PATTERNS)
		{
			if ((_this->Patterns[npat] = CSoundFile_AllocatePattern(rows, _this->m_nChannels)) == NULL) return TRUE;
			MODCOMMAND *m = _this->Patterns[npat];
			_this->PatternSize[npat] = rows;
			UINT imax = _this->m_nChannels*rows;
			for (UINT i=0; i<imax; i++, m++, dwPos+=4)
			{
				if (dwPos+4 > dwMemLength) break;
				const BYTE *p = lpStream+dwPos;
				UINT note = p[0];
				if (note)
				{
					m->note = note + 48;
					m->instr = p[1] + 1;
				}
				UINT command = p[2];
				UINT param = p[3];
				m->param = param;
				switch(command)
				{
				// 0: no effect
				case 0:
					break;
				// 1: Portamento Up
				case 1:
				case 17:
				case 30:
					if (param) m->command = CMD_PORTAMENTOUP;
					break;
				// 2: Portamento Down
				case 2:
				case 13:
				case 21:
					if (param) m->command = CMD_PORTAMENTODOWN;
					break;
				// 10: Arpeggio
				case 10:
				case 11:
				case 12:
					m->command = CMD_ARPEGGIO;
					break;
				// 15: Filter
				case 15:
					m->command = CMD_MODCMDEX;
					m->param = param & 0x0F;
					break;
				// 25: Position Jump
				case 25:
					m->command = CMD_POSITIONJUMP;
					break;
				// 28: Set Speed
				case 28:
					m->command = CMD_SPEED;
					break;
				// 31: Volume Control
				case 31:
					if (param <= 0x40) m->command = CMD_VOLUME; else
					if (param <= 0x50) { m->command = CMD_VOLUMESLIDE; m->param &= 0x0F; if (!m->param) m->param = 0x0F; } else
					if (param <= 0x60) { m->command = CMD_VOLUMESLIDE; m->param = (param & 0x0F) << 4; if (!m->param) m->param = 0xF0; } else
					if (param <= 0x70) { m->command = CMD_MODCMDEX; m->param = 0xB0 | (param & 0x0F); if (!(param & 0x0F)) m->param = 0xBF; } else
					if (param <= 0x80) { m->command = CMD_MODCMDEX; m->param = 0xA0 | (param & 0x0F); if (!(param & 0x0F)) m->param = 0xAF; }
					break;
				}
			}
		}
		npat++;
		dwMemPos += bswapBE32(*((DWORD *)(lpStream + dwMemPos + 4))) + 8;
	}
	// SBOD
	UINT nsmp = 1;
	while ((dwMemPos+10 < dwMemLength) && (*((DWORD *)(lpStream + dwMemPos)) == 0x444F4253))
	{
		if (nsmp < MAX_SAMPLES) CSoundFile_ReadSample(_this, &_this->Ins[nsmp], RS_PCM8S, (LPSTR)(lpStream+dwMemPos+8), dwMemLength-dwMemPos-8);
		dwMemPos += bswapBE32(*((DWORD *)(lpStream + dwMemPos + 4))) + 8;
		nsmp++;
	}
	return TRUE;
}

