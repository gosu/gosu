/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

////////////////////////////////////////////////////////////
// 669 Composer / UNIS 669 module loader
////////////////////////////////////////////////////////////

#include "libmodplug.h"

//#pragma warning(disable:4244)

typedef struct tagFILEHEADER669
{
	WORD sig;				// 'if' or 'JN'
	char songmessage[108];			// Song Message
	BYTE samples;			// number of samples (1-64)
	BYTE patterns;			// number of patterns (1-128)
	BYTE restartpos;
	BYTE orders[128];
	BYTE tempolist[128];
	BYTE breaks[128];
} FILEHEADER669;

typedef struct tagSAMPLE669
{
	BYTE filename[13];
	BYTE length[4];	// when will somebody think about DWORD align ???
	BYTE loopstart[4];
	BYTE loopend[4];
} SAMPLE669;

static DWORD lengthArrayToDWORD(const BYTE length[4]) {
	DWORD len = (length[3] << 24) +
		    (length[2] << 16) +
		    (length[1] << 8) +
		    (length[0]);

	return(len);
}


BOOL CSoundFile_Read669(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
//	BOOL b669Ext;
	const FILEHEADER669 *pfh = (const FILEHEADER669 *)lpStream;
	const SAMPLE669 *psmp = (const SAMPLE669 *)(lpStream + 0x1F1);
	DWORD dwMemPos = 0;
	DWORD donttouchme;
	UINT i;

	if ((!lpStream) || (dwMemLength < sizeof(FILEHEADER669))) return FALSE;
	if ((bswapLE16(pfh->sig) != 0x6669) && (bswapLE16(pfh->sig) != 0x4E4A)) return FALSE;
//	b669Ext = (bswapLE16(pfh->sig) == 0x4E4A) ? TRUE : FALSE;
	if ((!pfh->samples) || (pfh->samples > 64) || (pfh->restartpos >= 128)
	 || (!pfh->patterns) || (pfh->patterns > 128)) return FALSE;
	donttouchme = 0x1F1 + pfh->samples * sizeof(SAMPLE669) + pfh->patterns * 0x600;
	if (donttouchme > dwMemLength) return FALSE;
	for (i=0; i<pfh->samples; i++)
	{
		DWORD len = lengthArrayToDWORD(psmp[i].length);
		donttouchme += len;
	}
	if (donttouchme > dwMemLength) return FALSE;
	// That should be enough checking: this must be a 669 module.
	_this->m_nType = MOD_TYPE_669;
	_this->m_dwSongFlags |= SONG_LINEARSLIDES;
	_this->m_nMinPeriod = 28 << 2;
	_this->m_nMaxPeriod = 1712 << 3;
	_this->m_nDefaultTempo = 78;
	_this->m_nDefaultSpeed = 6;
	_this->m_nChannels = 8;
	_this->m_nSamples = pfh->samples;
	for (i=1; i<=_this->m_nSamples; i++, psmp++)
	{
		DWORD len = lengthArrayToDWORD(psmp->length);
		DWORD loopstart = lengthArrayToDWORD(psmp->loopstart);
		DWORD loopend = lengthArrayToDWORD(psmp->loopend);
		if (len > MAX_SAMPLE_LENGTH) len = MAX_SAMPLE_LENGTH;
		if ((loopend > len) && (!loopstart)) loopend = 0;
		if (loopend > len) loopend = len;
		if (loopstart + 4 >= loopend) loopstart = loopend = 0;
		_this->Ins[i].nLength = len;
		_this->Ins[i].nLoopStart = loopstart;
		_this->Ins[i].nLoopEnd = loopend;
		if (loopend) _this->Ins[i].uFlags |= CHN_LOOP;
		_this->Ins[i].nVolume = 256;
		_this->Ins[i].nGlobalVol = 64;
		_this->Ins[i].nPan = 128;
	}
	// Reading Orders
	SDL_memcpy(_this->Order, pfh->orders, 128);
	_this->m_nRestartPos = pfh->restartpos;
	if (_this->Order[_this->m_nRestartPos] >= pfh->patterns) _this->m_nRestartPos = 0;
	// Reading Pattern Break Locations
	for (i=0; i<8; i++)
	{
		_this->ChnSettings[i].nPan = (i & 1) ? 0x30 : 0xD0;
		_this->ChnSettings[i].nVolume = 64;
	}
	// Reading Patterns
	dwMemPos = 0x1F1 + pfh->samples * 25;
	for (i=0; i<pfh->patterns; i++)
	{
		MODCOMMAND *m;
		const BYTE *p;
		UINT row, j;
		_this->Patterns[i] = CSoundFile_AllocatePattern(64, _this->m_nChannels);
		if (!_this->Patterns[i]) break;
		_this->PatternSize[i] = 64;
		m = _this->Patterns[i];
		p = lpStream + dwMemPos;
		for (row=0; row<64; row++)
		{
			MODCOMMAND *mspeed = m;
			if ((row == pfh->breaks[i]) && (row != 63))
			{
				for (j=0; j<8; j++)
				{
					m[j].command = CMD_PATTERNBREAK;
					m[j].param = 0;
				}
			}
			for (j=0; j<8; j++, m++, p+=3)
			{
				UINT note = p[0] >> 2;
				UINT instr = ((p[0] & 0x03) << 4) | (p[1] >> 4);
				UINT vol = p[1] & 0x0F;
				if (p[0] < 0xFE)
				{
					m->note = note + 37;
					m->instr = instr + 1;
				}
				if (p[0] <= 0xFE)
				{
					m->volcmd = VOLCMD_VOLUME;
					m->vol = (vol << 2) + 2;
				}
				if (p[2] != 0xFF)
				{
					UINT command = p[2] >> 4;
					UINT param = p[2] & 0x0F;
					switch(command)
					{
					case 0x00:	command = CMD_PORTAMENTOUP; break;
					case 0x01:	command = CMD_PORTAMENTODOWN; break;
					case 0x02:	command = CMD_TONEPORTAMENTO; break;
					case 0x03:	command = CMD_MODCMDEX; param |= 0x50; break;
					case 0x04:	command = CMD_VIBRATO; param |= 0x40; break;
					case 0x05:	if (param) command = CMD_SPEED; else command = 0; break;
					case 0x06:	if (param == 0) { command = CMD_PANNINGSLIDE; param = 0xFE; }
							else if (param == 1) { command = CMD_PANNINGSLIDE; param = 0xEF; }
							else command = 0;
							break;
					default:	command = 0;
					}
					if (command)
					{
						if (command == CMD_SPEED) mspeed = NULL;
						m->command = command;
						m->param = param;
					}
				}
			}
			if ((!row) && (mspeed))
			{
				for (j=0; j<8; j++) if (!mspeed[j].command)
				{
					mspeed[j].command = CMD_SPEED;
					mspeed[j].param = pfh->tempolist[j];
					break;
				}
			}
		}
		dwMemPos += 0x600;
	}
	// Reading Samples
	for (i=1; i<=_this->m_nSamples; i++)
	{
		UINT len = _this->Ins[i].nLength;
		if (dwMemPos >= dwMemLength) break;
		if (len > 4) CSoundFile_ReadSample(_this, &_this->Ins[i], RS_PCM8U, (LPSTR)(lpStream+dwMemPos), dwMemLength - dwMemPos);
		dwMemPos += len;
	}
	return TRUE;
}
