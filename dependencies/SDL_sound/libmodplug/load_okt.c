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

#define MAGIC(a,b,c,d) (((a) << 24UL) | ((b) << 16UL) | ((c) << 8UL) | (d))

#pragma pack(1)
typedef struct OKTFILEHEADER
{
	DWORD okta;		// "OKTA"
	DWORD song;		// "SONG"
	DWORD cmod;		// "CMOD"
	DWORD cmodlen;
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
#pragma pack()


static DWORD readBE32(const BYTE *v)
{
	return (v[0] << 24UL) | (v[1] << 16UL) | (v[2] << 8UL) | v[3];
}

BOOL CSoundFile_ReadOKT(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	const OKTFILEHEADER *pfh = (OKTFILEHEADER *)lpStream;
	DWORD dwMemPos = sizeof(OKTFILEHEADER), dwSize;
	UINT nsamples = 0, norders = 0, npat = 0, nsmp = 1;//, npatterns = 0
	UINT i;

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((bswapBE32(pfh->okta) != MAGIC('O','K','T','A'))
	 || (bswapBE32(pfh->song) != MAGIC('S','O','N','G'))
	 || (bswapBE32(pfh->cmod) != MAGIC('C','M','O','D'))
	 || (bswapBE32(pfh->cmodlen) != 8)
	 || (pfh->chnsetup[0]) || (pfh->chnsetup[2])
	 || (pfh->chnsetup[4]) || (pfh->chnsetup[6])
	 || (bswapBE32(pfh->samp) != MAGIC('S','A','M','P'))) return FALSE;
	_this->m_nType = MOD_TYPE_OKT;
	_this->m_nChannels = 4 + pfh->chnsetup[1] + pfh->chnsetup[3] + pfh->chnsetup[5] + pfh->chnsetup[7];
	if (_this->m_nChannels > MAX_CHANNELS) _this->m_nChannels = MAX_CHANNELS;
	nsamples = bswapBE32(pfh->samplen) >> 5;
	_this->m_nSamples = nsamples;
	if (_this->m_nSamples >= MAX_SAMPLES) _this->m_nSamples = MAX_SAMPLES-1;
	// Reading samples
	for (i=1; i <= nsamples; i++)
	{
		if (dwMemPos >= dwMemLength - sizeof(OKTSAMPLE)) return TRUE;
		if (i < MAX_SAMPLES)
		{
			const OKTSAMPLE *psmp = (const OKTSAMPLE *)(lpStream + dwMemPos);
			MODINSTRUMENT *pins = &_this->Ins[i];

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
	if (dwMemPos >= dwMemLength - 12) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('S','P','E','E'))
	{
		_this->m_nDefaultSpeed = lpStream[dwMemPos+9];

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// SLEN
	if (dwMemPos + 10 > dwMemLength) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('S','L','E','N'))
	{
	//	npatterns = lpStream[dwMemPos+9];

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// PLEN
	if (dwMemPos + 10 > dwMemLength) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('P','L','E','N'))
	{
		norders = lpStream[dwMemPos+9];

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// PATT
	if (dwMemPos + 8 > dwMemLength) return TRUE;
	if (readBE32(lpStream + dwMemPos) == MAGIC('P','A','T','T'))
	{
		UINT orderlen = norders;
		if (orderlen >= MAX_ORDERS) orderlen = MAX_ORDERS-1;
		if (dwMemPos + 8 + orderlen > dwMemLength) return TRUE;
		for (i=0; i<orderlen; i++) _this->Order[i] = lpStream[dwMemPos+8+i];
		for (i=orderlen; i>1; i--) { if (_this->Order[i-1]) break; _this->Order[i-1] = 0xFF; }

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// PBOD
	while ((dwMemPos < dwMemLength - 10) && (readBE32(lpStream + dwMemPos) == MAGIC('P','B','O','D')))
	{
		DWORD dwPos = dwMemPos + 10;
		UINT rows = lpStream[dwMemPos+9];
		if (!rows) rows = 64;
		if (npat < MAX_PATTERNS)
		{
			MODCOMMAND *m;
			UINT imax;
			if ((_this->Patterns[npat] = CSoundFile_AllocatePattern(rows, _this->m_nChannels)) == NULL) return TRUE;
			m = _this->Patterns[npat];
			_this->PatternSize[npat] = rows;
			imax = _this->m_nChannels*rows;
			for (i=0; i<imax; i++, m++, dwPos+=4)
			{
				const BYTE *p;
				UINT note, command, param;
				if (dwPos+4 > dwMemLength) break;
				p = lpStream+dwPos;
				note = p[0];
				if (note)
				{
					m->note = note + 48;
					m->instr = p[1] + 1;
				}
				command = p[2];
				param = p[3];
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

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	// SBOD
	while ((dwMemPos < dwMemLength-10) && (readBE32(lpStream + dwMemPos) == MAGIC('S','B','O','D')))
	{
		if (nsmp < MAX_SAMPLES) CSoundFile_ReadSample(_this, &_this->Ins[nsmp], RS_PCM8S, (LPSTR)(lpStream+dwMemPos+8), dwMemLength-dwMemPos-8);
		nsmp++;

		dwSize = readBE32(lpStream + dwMemPos + 4);
		if (dwSize > dwMemLength - 8 || dwMemPos > dwMemLength - dwSize - 8) return TRUE;
		dwMemPos += dwSize + 8;
	}
	return TRUE;
}
