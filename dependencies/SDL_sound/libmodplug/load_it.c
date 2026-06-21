/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (Endian and char fixes for PPC)
 *          Marco Trillo     <toad@arsystel.com> (Endian fixes for SaveIT, XM->IT Sample Converter)
 *
*/

#include "libmodplug.h"

static const
BYTE autovibit2xm[8] =
{ 0, 3, 1, 4, 2, 0, 0, 0 };

//////////////////////////////////////////////////////////
// Impulse Tracker IT file support

static BOOL ITInstrToMPT(const void *p, INSTRUMENTHEADER *penv, UINT trkvers)
//--------------------------------------------------------------------------------
{
	UINT j;
	if (trkvers < 0x0200)
	{
		const ITOLDINSTRUMENT *pis = (const ITOLDINSTRUMENT *)p;
		SDL_memcpy(penv->filename, pis->filename, 12);
		penv->nFadeOut = bswapLE16(pis->fadeout) << 6;
		penv->nGlobalVol = 64;
		for (j=0; j<NOTE_MAX; j++)
		{
			UINT note = pis->keyboard[j*2];
			UINT ins = pis->keyboard[j*2+1];
			if (ins < MAX_SAMPLES) penv->Keyboard[j] = ins;
			if (note < 128) penv->NoteMap[j] = note+1;
			else if (note >= 0xFE) penv->NoteMap[j] = note;
		}
		if (pis->flags & 0x01) penv->dwFlags |= ENV_VOLUME;
		if (pis->flags & 0x02) penv->dwFlags |= ENV_VOLLOOP;
		if (pis->flags & 0x04) penv->dwFlags |= ENV_VOLSUSTAIN;
		penv->nVolLoopStart = pis->vls;
		penv->nVolLoopEnd = pis->vle;
		penv->nVolSustainBegin = pis->sls;
		penv->nVolSustainEnd = pis->sle;
		penv->nVolEnv = 25;
		for (j=0; j<25; j++)
		{
			if ((penv->VolPoints[j] = pis->nodes[j*2]) == 0xFF)
			{
				penv->nVolEnv = j;
				break;
			}
			penv->VolEnv[j] = pis->nodes[j*2+1];
		}
		penv->nNNA = pis->nna;
		penv->nDCT = pis->dnc;
		penv->nPan = 0x80;
	} else
	{
		const ITINSTRUMENT *pis = (const ITINSTRUMENT *)p;
		SDL_memcpy(penv->filename, pis->filename, 12);
		penv->nMidiProgram = pis->mpr;
		penv->nMidiChannel = pis->mch;
		penv->wMidiBank = bswapLE16(pis->mbank);
		penv->nFadeOut = bswapLE16(pis->fadeout) << 5;
		penv->nGlobalVol = pis->gbv >> 1;
		if (penv->nGlobalVol > 64) penv->nGlobalVol = 64;
		for (j=0; j<NOTE_MAX; j++)
		{
			UINT note = pis->keyboard[j*2];
			UINT ins = pis->keyboard[j*2+1];
			if (ins < MAX_SAMPLES) penv->Keyboard[j] = ins;
			if (note < 128) penv->NoteMap[j] = note+1;
			else if (note >= 0xFE) penv->NoteMap[j] = note;
		}
		// Volume Envelope
		if (pis->volenv.flags & 1) penv->dwFlags |= ENV_VOLUME;
		if (pis->volenv.flags & 2) penv->dwFlags |= ENV_VOLLOOP;
		if (pis->volenv.flags & 4) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (pis->volenv.flags & 8) penv->dwFlags |= ENV_VOLCARRY;
		penv->nVolEnv = pis->volenv.num;
		if (penv->nVolEnv > 25) penv->nVolEnv = 25;

		penv->nVolLoopStart = pis->volenv.lpb;
		penv->nVolLoopEnd = pis->volenv.lpe;
		penv->nVolSustainBegin = pis->volenv.slb;
		penv->nVolSustainEnd = pis->volenv.sle;
		// Panning Envelope
		if (pis->panenv.flags & 1) penv->dwFlags |= ENV_PANNING;
		if (pis->panenv.flags & 2) penv->dwFlags |= ENV_PANLOOP;
		if (pis->panenv.flags & 4) penv->dwFlags |= ENV_PANSUSTAIN;
		if (pis->panenv.flags & 8) penv->dwFlags |= ENV_PANCARRY;
		penv->nPanEnv = pis->panenv.num;
		if (penv->nPanEnv > 25) penv->nPanEnv = 25;
		penv->nPanLoopStart = pis->panenv.lpb;
		penv->nPanLoopEnd = pis->panenv.lpe;
		penv->nPanSustainBegin = pis->panenv.slb;
		penv->nPanSustainEnd = pis->panenv.sle;
		// Pitch Envelope
		if (pis->pitchenv.flags & 1) penv->dwFlags |= ENV_PITCH;
		if (pis->pitchenv.flags & 2) penv->dwFlags |= ENV_PITCHLOOP;
		if (pis->pitchenv.flags & 4) penv->dwFlags |= ENV_PITCHSUSTAIN;
		if (pis->pitchenv.flags & 8) penv->dwFlags |= ENV_PITCHCARRY;
		if (pis->pitchenv.flags & 0x80) penv->dwFlags |= ENV_FILTER;
		penv->nPitchEnv = pis->pitchenv.num;
		if (penv->nPitchEnv > 25) penv->nPitchEnv = 25;
		penv->nPitchLoopStart = pis->pitchenv.lpb;
		penv->nPitchLoopEnd = pis->pitchenv.lpe;
		penv->nPitchSustainBegin = pis->pitchenv.slb;
		penv->nPitchSustainEnd = pis->pitchenv.sle;
		// Envelopes Data
		for (j=0; j<25; j++)
		{
			penv->VolEnv[j] = pis->volenv.data[j*3];
			penv->VolPoints[j] = (pis->volenv.data[j*3+2] << 8) | (pis->volenv.data[j*3+1]);
			penv->PanEnv[j] = pis->panenv.data[j*3] + 32;
			penv->PanPoints[j] = (pis->panenv.data[j*3+2] << 8) | (pis->panenv.data[j*3+1]);
			penv->PitchEnv[j] = pis->pitchenv.data[j*3] + 32;
			penv->PitchPoints[j] = (pis->pitchenv.data[j*3+2] << 8) | (pis->pitchenv.data[j*3+1]);
		}
		penv->nNNA = pis->nna;
		penv->nDCT = pis->dct;
		penv->nDNA = pis->dca;
		penv->nPPS = pis->pps;
		penv->nPPC = pis->ppc;
		penv->nIFC = pis->ifc;
		penv->nIFR = pis->ifr;
		penv->nVolSwing = pis->rv;
		penv->nPanSwing = pis->rp;
		penv->nPan = (pis->dfp & 0x7F) << 2;
		if (penv->nPan > 256) penv->nPan = 128;
		if (pis->dfp < 0x80) penv->dwFlags |= ENV_SETPANNING;
	}
	if ((penv->nVolLoopStart >= 25) || (penv->nVolLoopEnd >= 25)) penv->dwFlags &= ~ENV_VOLLOOP;
	if ((penv->nVolSustainBegin >= 25) || (penv->nVolSustainEnd >= 25)) penv->dwFlags &= ~ENV_VOLSUSTAIN;
	return TRUE;
}


BOOL CSoundFile_ReadIT(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//--------------------------------------------------------------
{
	DWORD dwMemPos = sizeof(ITFILEHEADER);
	DWORD inspos[MAX_INSTRUMENTS];
	DWORD smppos[MAX_SAMPLES];
	DWORD patpos[MAX_PATTERNS];
	BYTE chnmask[64];//, channels_used[64]
	MODCOMMAND lastvalue[64];
	ITFILEHEADER pifh;
	UINT j, npatterns, siz;

	if ((!lpStream) || (dwMemLength < sizeof(ITFILEHEADER))) return FALSE;
	SDL_memcpy(&pifh, lpStream, sizeof(pifh));

	pifh.id = bswapLE32(pifh.id);
	pifh.reserved1 = bswapLE16(pifh.reserved1);
	pifh.ordnum = bswapLE16(pifh.ordnum);
	pifh.insnum = bswapLE16(pifh.insnum);
	pifh.smpnum = bswapLE16(pifh.smpnum);
	pifh.patnum = bswapLE16(pifh.patnum);
	pifh.cwtv = bswapLE16(pifh.cwtv);
	pifh.cmwt = bswapLE16(pifh.cmwt);
	pifh.flags = bswapLE16(pifh.flags);
	pifh.special = bswapLE16(pifh.special);
	pifh.msglength = bswapLE16(pifh.msglength);
	pifh.msgoffset = bswapLE32(pifh.msgoffset);
	pifh.reserved2 = bswapLE32(pifh.reserved2);

	if ((pifh.id != 0x4D504D49) || (pifh.insnum >= MAX_INSTRUMENTS)
	 || (!pifh.smpnum) || (pifh.smpnum >= MAX_INSTRUMENTS) || (!pifh.ordnum)) return FALSE;
	if (dwMemPos + pifh.ordnum + pifh.insnum*4
	 + pifh.smpnum*4 + pifh.patnum*4 > dwMemLength) return FALSE;
	_this->m_nType = MOD_TYPE_IT;
	if (pifh.flags & 0x08) _this->m_dwSongFlags |= SONG_LINEARSLIDES;
	if (pifh.flags & 0x10) _this->m_dwSongFlags |= SONG_ITOLDEFFECTS;
	if (pifh.flags & 0x20) _this->m_dwSongFlags |= SONG_ITCOMPATMODE;
	if (pifh.flags & 0x80) _this->m_dwSongFlags |= SONG_EMBEDMIDICFG;
	if (pifh.flags & 0x1000) _this->m_dwSongFlags |= SONG_EXFILTERRANGE;
	// Global Volume
	if (pifh.globalvol)
	{
		_this->m_nDefaultGlobalVolume = pifh.globalvol << 1;
		if (!_this->m_nDefaultGlobalVolume) _this->m_nDefaultGlobalVolume = 256;
		if (_this->m_nDefaultGlobalVolume > 256) _this->m_nDefaultGlobalVolume = 256;
	}
	if (pifh.speed) _this->m_nDefaultSpeed = pifh.speed;
	if (pifh.tempo) _this->m_nDefaultTempo = pifh.tempo;
	_this->m_nSongPreAmp = pifh.mv & 0x7F;
	// Reading Channels Pan Positions
	for (j=0; j<64; j++) if (pifh.chnpan[j] != 0xFF)
	{
		UINT n;
		_this->ChnSettings[j].nVolume = pifh.chnvol[j];
		_this->ChnSettings[j].nPan = 128;
		if (pifh.chnpan[j] & 0x80) _this->ChnSettings[j].dwFlags |= CHN_MUTE;
		n = pifh.chnpan[j] & 0x7F;
		if (n <= 64) _this->ChnSettings[j].nPan = n << 2;
		if (n == 100) _this->ChnSettings[j].dwFlags |= CHN_SURROUND;
	}
	if (_this->m_nChannels < 4) _this->m_nChannels = 4;
	// Reading orders
	siz = pifh.ordnum;
	if (siz > MAX_ORDERS) siz = MAX_ORDERS;
	SDL_memcpy(_this->Order, lpStream+dwMemPos, siz);
	dwMemPos += pifh.ordnum;
	// Reading Instrument Offsets
	SDL_memset(inspos, 0, sizeof(inspos));
	siz = pifh.insnum;
	if (siz > MAX_INSTRUMENTS) siz = MAX_INSTRUMENTS;
	siz <<= 2;
	SDL_memcpy(inspos, lpStream+dwMemPos, siz);
	for (j=0; j < (siz>>2); j++) {
	    inspos[j] = bswapLE32(inspos[j]);
	}
	dwMemPos += pifh.insnum * 4;
	// Reading Samples Offsets
	SDL_memset(smppos, 0, sizeof(smppos));
	siz = pifh.smpnum;
	if (siz > MAX_SAMPLES) siz = MAX_SAMPLES;
	siz <<= 2;
	SDL_memcpy(smppos, lpStream+dwMemPos, siz);
	for (j=0; j < (siz>>2); j++) {
	    smppos[j] = bswapLE32(smppos[j]);
	}
	dwMemPos += pifh.smpnum * 4;
	// Reading Patterns Offsets
	SDL_memset(patpos, 0, sizeof(patpos));
	siz = pifh.patnum;
	if (siz > MAX_PATTERNS) siz = MAX_PATTERNS;
	siz <<= 2;
	SDL_memcpy(patpos, lpStream+dwMemPos, siz);
	for (j=0; j < (siz>>2); j++) {
	    patpos[j] = bswapLE32(patpos[j]);
	}
	dwMemPos += pifh.patnum * 4;
	// Reading IT Extra Info
	if (dwMemPos + 2 < dwMemLength)
	{
		UINT nflt = bswapLE16(*((WORD *)(lpStream + dwMemPos)));
		dwMemPos += 2;
		if (dwMemPos + nflt * 8 < dwMemLength) dwMemPos += nflt * 8;
	}
	// Reading Midi Output & Macros
	if (_this->m_dwSongFlags & SONG_EMBEDMIDICFG)
	{
		if (dwMemPos + sizeof(MODMIDICFG) < dwMemLength)
		{
			SDL_memcpy(&_this->m_MidiCfg, lpStream+dwMemPos, sizeof(MODMIDICFG));
			dwMemPos += sizeof(MODMIDICFG);
		}
	}
	// Read pattern names: "PNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e50))
	{
		UINT len = bswapLE32(*((DWORD *)(lpStream+dwMemPos+4)));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= MAX_PATTERNS*MAX_PATTERNNAME) && (len >= MAX_PATTERNNAME))
		{
			_this->m_lpszPatternNames = (char *) SDL_malloc(len);
			if (_this->m_lpszPatternNames)
			{
				_this->m_nPatternNames = len / MAX_PATTERNNAME;
				SDL_memcpy(_this->m_lpszPatternNames, lpStream+dwMemPos, len);
			}
			dwMemPos += len;
		}
	}
	// 4-channels minimum
	_this->m_nChannels = 4;
	// Read channel names: "CNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e43))
	{
		const UINT len = bswapLE32(*((DWORD *)(lpStream+dwMemPos+4)));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= 64*MAX_CHANNELNAME))
		{
			const UINT n = len / MAX_CHANNELNAME;
			if (n > _this->m_nChannels) _this->m_nChannels = n;
			dwMemPos += len;
		}
	}
	// Read mix plugins information
	if (dwMemPos + 8 < dwMemLength)
	{
		dwMemPos += CSoundFile_LoadMixPlugins(_this, lpStream+dwMemPos, dwMemLength-dwMemPos);
	}
	// Checking for unused channels
	npatterns = pifh.patnum;
	if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
	for (j=0; j<npatterns; j++)
	{
		UINT i, len, rows, nrow;
		const BYTE *p;
		SDL_memset(chnmask, 0, sizeof(chnmask));
		if ((!patpos[j]) || ((DWORD)patpos[j] >= dwMemLength - 4)) continue;
		len = bswapLE16(*((WORD *)(lpStream+patpos[j])));
		rows = bswapLE16(*((WORD *)(lpStream+patpos[j]+2)));
		if ((rows < 4) || (rows > 256)) continue;
		if (8+len > dwMemLength || patpos[j] > dwMemLength - (8+len)) continue;
		p = lpStream+patpos[j]+8;
		i = nrow = 0;
		while (nrow<rows)
		{
			UINT ch;
			BYTE b;
			if (i >= len) break;
			b = p[i++];
			if (!b)
			{
				nrow++;
				continue;
			}
			ch = b & 0x7F;
			if (ch) ch = (ch - 1) & 0x3F;
			if (b & 0x80)
			{
				if (i >= len) break;
				chnmask[ch] = p[i++];
			}
			// Channel used
			if (chnmask[ch] & 0x0F)
			{
				if ((ch >= _this->m_nChannels) && (ch < 64)) _this->m_nChannels = ch+1;
			}
			// Note
			if (chnmask[ch] & 1) i++;
			// Instrument
			if (chnmask[ch] & 2) i++;
			// Volume
			if (chnmask[ch] & 4) i++;
			// Effect
			if (chnmask[ch] & 8) i += 2;
			if (i >= len) break;
		}
	}
	// Reading Instruments
	_this->m_nInstruments = 0;
	if (pifh.flags & 0x04) _this->m_nInstruments = pifh.insnum;
	if (_this->m_nInstruments >= MAX_INSTRUMENTS) _this->m_nInstruments = MAX_INSTRUMENTS-1;
	for (j=0; j<_this->m_nInstruments; j++)
	{
		if ((inspos[j] > 0) && dwMemLength > sizeof(ITOLDINSTRUMENT) &&
			(inspos[j] < dwMemLength - sizeof(ITOLDINSTRUMENT)))
		{
			INSTRUMENTHEADER *penv = (INSTRUMENTHEADER *) SDL_calloc(1, sizeof (INSTRUMENTHEADER));
			if (!penv) continue;
			_this->Headers[j+1] = penv;
			ITInstrToMPT(lpStream + inspos[j], penv, pifh.cmwt);
		}
	}
	// Reading Samples
	_this->m_nSamples = pifh.smpnum;
	if (_this->m_nSamples >= MAX_SAMPLES) _this->m_nSamples = MAX_SAMPLES-1;
	for (j=0; j<pifh.smpnum; j++) if ((smppos[j]) && (smppos[j] <= dwMemLength - sizeof(ITSAMPLESTRUCT)))
	{
		ITSAMPLESTRUCT pis;
		SDL_memcpy(&pis, lpStream+smppos[j], sizeof (pis));
		pis.id = bswapLE32(pis.id);
		pis.length = bswapLE32(pis.length);
		pis.loopbegin = bswapLE32(pis.loopbegin);
		pis.loopend = bswapLE32(pis.loopend);
		pis.C5Speed = bswapLE32(pis.C5Speed);
		pis.susloopbegin = bswapLE32(pis.susloopbegin);
		pis.susloopend = bswapLE32(pis.susloopend);
		pis.samplepointer = bswapLE32(pis.samplepointer);

		if (pis.id == 0x53504D49)
		{
			MODINSTRUMENT *pins = &_this->Ins[j+1];
			pins->uFlags = 0;
			pins->nLength = 0;
			pins->nLoopStart = pis.loopbegin;
			pins->nLoopEnd = pis.loopend;
			pins->nSustainStart = pis.susloopbegin;
			pins->nSustainEnd = pis.susloopend;
			pins->nC4Speed = pis.C5Speed;
			if (!pins->nC4Speed) pins->nC4Speed = 8363;
			if (pis.C5Speed < 256) pins->nC4Speed = 256;
			pins->nVolume = pis.vol << 2;
			if (pins->nVolume > 256) pins->nVolume = 256;
			pins->nGlobalVol = pis.gvl;
			if (pins->nGlobalVol > 64) pins->nGlobalVol = 64;
			if (pis.flags & 0x10) pins->uFlags |= CHN_LOOP;
			if (pis.flags & 0x20) pins->uFlags |= CHN_SUSTAINLOOP;
			if (pis.flags & 0x40) pins->uFlags |= CHN_PINGPONGLOOP;
			if (pis.flags & 0x80) pins->uFlags |= CHN_PINGPONGSUSTAIN;
			pins->nPan = (pis.dfp & 0x7F) << 2;
			if (pins->nPan > 256) pins->nPan = 256;
			if (pis.dfp & 0x80) pins->uFlags |= CHN_PANNING;
			pins->nVibType = autovibit2xm[pis.vit & 7];
			pins->nVibRate = pis.vis;
			pins->nVibDepth = pis.vid & 0x7F;
			pins->nVibSweep = (pis.vir + 3) / 4;
			if ((pis.samplepointer) && (pis.samplepointer < dwMemLength) && (pis.length))
			{
				UINT flags = (pis.cvt & 1) ? RS_PCM8S : RS_PCM8U;
				pins->nLength = pis.length;
				if (pins->nLength > MAX_SAMPLE_LENGTH) pins->nLength = MAX_SAMPLE_LENGTH;
				if (pis.flags & 2)
				{
					flags += 5;
					pins->uFlags |= CHN_16BIT;
					// IT 2.14 16-bit packed sample ?
					if (pis.flags & 8) flags = ((pifh.cmwt >= 0x215) && (pis.cvt & 4)) ? RS_IT21516 : RS_IT21416;
					if (pis.flags & 4) flags |= RSF_STEREO;
				} else
				{
					if (pis.cvt == 0xFF) flags = RS_ADPCM4; else
					// IT 2.14 8-bit packed sample ?
					if (pis.flags & 8)	flags =	((pifh.cmwt >= 0x215) && (pis.cvt & 4)) ? RS_IT2158 : RS_IT2148;
					if (pis.flags & 4) flags |= RSF_STEREO;
				}
				CSoundFile_ReadSample(_this, &_this->Ins[j+1], flags, (LPSTR)(lpStream+pis.samplepointer), dwMemLength - pis.samplepointer);
			}
		}
	}
	// Reading Patterns
	for (j=0; j<npatterns; j++)
	{
		MODCOMMAND *m;
		const BYTE *p;
		UINT i, len, rows, nrow;
		if ((!patpos[j]) || ((DWORD)patpos[j] >= dwMemLength - 4))
		{
			_this->PatternSize[j] = 64;
			_this->Patterns[j] = CSoundFile_AllocatePattern(64, _this->m_nChannels);
			continue;
		}

		len = bswapLE16(*((WORD *)(lpStream+patpos[j])));
		rows = bswapLE16(*((WORD *)(lpStream+patpos[j]+2)));
		if ((rows < 4) || (rows > 256)) continue;
		if (8+len > dwMemLength || patpos[j] > dwMemLength - (8+len)) continue;
		_this->PatternSize[j] = rows;
		if ((_this->Patterns[j] = CSoundFile_AllocatePattern(rows, _this->m_nChannels)) == NULL) continue;
		SDL_memset(lastvalue, 0, sizeof(lastvalue));
		SDL_memset(chnmask, 0, sizeof(chnmask));
		m = _this->Patterns[j];
		p = lpStream+patpos[j]+8;
		i = nrow = 0;
		while (nrow<rows)
		{
			UINT ch;
			BYTE b;
			if (i >= len) break;
			b = p[i++];
			if (!b)
			{
				nrow++;
				m+=_this->m_nChannels;
				continue;
			}
			ch = b & 0x7F;
			if (ch) ch = (ch - 1) & 0x3F;
			if (b & 0x80)
			{
				if (i >= len) break;
				chnmask[ch] = p[i++];
			}
			if ((chnmask[ch] & 0x10) && (ch < _this->m_nChannels))
			{
				m[ch].note = lastvalue[ch].note;
			}
			if ((chnmask[ch] & 0x20) && (ch < _this->m_nChannels))
			{
				m[ch].instr = lastvalue[ch].instr;
			}
			if ((chnmask[ch] & 0x40) && (ch < _this->m_nChannels))
			{
				m[ch].volcmd = lastvalue[ch].volcmd;
				m[ch].vol = lastvalue[ch].vol;
			}
			if ((chnmask[ch] & 0x80) && (ch < _this->m_nChannels))
			{
				m[ch].command = lastvalue[ch].command;
				m[ch].param = lastvalue[ch].param;
			}
			if (chnmask[ch] & 1)	// Note
			{
				UINT note;
				if (i >= len) break;
				note = p[i++];
				if (ch < _this->m_nChannels)
				{
					if (note < 0x80) note++;
					m[ch].note = note;
					lastvalue[ch].note = note;
				//	channels_used[ch] = TRUE;
				}
			}
			if (chnmask[ch] & 2)
			{
				UINT instr;
				if (i >= len) break;
				instr = p[i++];
				if (ch < _this->m_nChannels)
				{
					m[ch].instr = instr;
					lastvalue[ch].instr = instr;
				}
			}
			if (chnmask[ch] & 4)
			{
				UINT vol;
				if (i >= len) break;
				vol = p[i++];
				if (ch < _this->m_nChannels)
				{
					// 0-64: Set Volume
					if (vol <= 64) { m[ch].volcmd = VOLCMD_VOLUME; m[ch].vol = vol; } else
					// 128-192: Set Panning
					if ((vol >= 128) && (vol <= 192)) { m[ch].volcmd = VOLCMD_PANNING; m[ch].vol = vol - 128; } else
					// 65-74: Fine Volume Up
					if (vol < 75) { m[ch].volcmd = VOLCMD_FINEVOLUP; m[ch].vol = vol - 65; } else
					// 75-84: Fine Volume Down
					if (vol < 85) { m[ch].volcmd = VOLCMD_FINEVOLDOWN; m[ch].vol = vol - 75; } else
					// 85-94: Volume Slide Up
					if (vol < 95) { m[ch].volcmd = VOLCMD_VOLSLIDEUP; m[ch].vol = vol - 85; } else
					// 95-104: Volume Slide Down
					if (vol < 105) { m[ch].volcmd = VOLCMD_VOLSLIDEDOWN; m[ch].vol = vol - 95; } else
					// 105-114: Pitch Slide Up
					if (vol < 115) { m[ch].volcmd = VOLCMD_PORTADOWN; m[ch].vol = vol - 105; } else
					// 115-124: Pitch Slide Down
					if (vol < 125) { m[ch].volcmd = VOLCMD_PORTAUP; m[ch].vol = vol - 115; } else
					// 193-202: Portamento To
					if ((vol >= 193) && (vol <= 202)) { m[ch].volcmd = VOLCMD_TONEPORTAMENTO; m[ch].vol = vol - 193; } else
					// 203-212: Vibrato
					if ((vol >= 203) && (vol <= 212)) { m[ch].volcmd = VOLCMD_VIBRATOSPEED; m[ch].vol = vol - 203; }
					lastvalue[ch].volcmd = m[ch].volcmd;
					lastvalue[ch].vol = m[ch].vol;
				}
			}
			// Reading command/param
			if (chnmask[ch] & 8)
			{
				UINT cmd, param;
				if (i > len - 2) break;
				cmd = p[i++];
				param = p[i++];
				if (ch < _this->m_nChannels)
				{
					if (cmd)
					{
						m[ch].command = cmd;
						m[ch].param = param;
						CSoundFile_S3MConvert(&m[ch], TRUE);
						lastvalue[ch].command = m[ch].command;
						lastvalue[ch].param = m[ch].param;
					}
				}
			}
		}
	}
	for (j=0; j<MAX_BASECHANNELS; j++)
	{
		if (j>=_this->m_nChannels)
		{
			_this->ChnSettings[j].nVolume = 64;
			_this->ChnSettings[j].dwFlags &= ~CHN_MUTE;
		}
	}
	_this->m_nMinPeriod = 8;
	_this->m_nMaxPeriod = 0xF000;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// IT 2.14 compression

static DWORD ITReadBits(DWORD *bitbuf, UINT *bitnum, LPBYTE *_ibuf, LPBYTE ibufend, CHAR n)
//-----------------------------------------------------------------
{
	LPBYTE ibuf = *_ibuf;
	DWORD retval = 0;
	UINT i = n;

	// explicit if read 0 bits, then return 0
	if (i == 0)
		return(0);

	if (n > 0)
	{
		do
		{
			if (!*bitnum)
			{
				if (ibuf >= ibufend)
				{
					*_ibuf = ibuf;
					return 0;
				}
				*bitbuf = *ibuf++;
				*bitnum = 8;
			}
			retval >>= 1;
			retval |= *bitbuf << 31;
			*bitbuf >>= 1;
			(*bitnum)--;
			i--;
		} while (i);
		i = n;
	}
	*_ibuf = ibuf;
	return (retval >> (32-i));
}

#define IT215_SUPPORT
DWORD ITUnpack8Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, DWORD channels, BOOL b215)
//-------------------------------------------------------------------------------------------------------------------
{
	signed char *pDst = pSample;
	LPBYTE pSrc = lpMemFile;
	LPBYTE pStop = lpMemFile + dwMemLength;
	DWORD writePos = 0;
//	DWORD wHdr = 0;
	DWORD wCount = 0;
	DWORD bitbuf = 0;
	UINT bitnum = 0;
	BYTE bLeft = 0, bTemp = 0, bTemp2 = 0;

	while (dwLen)
	{
		DWORD d, dwPos;
		if (!wCount)
		{
			wCount = 0x8000;
		//	wHdr = bswapLE16(*((LPWORD)pSrc));
			pSrc += 2;
			bLeft = 9;
			bTemp = bTemp2 = 0;
			bitbuf = bitnum = 0;
		}
		d = wCount;
		if (d > dwLen) d = dwLen;
		// Unpacking
		dwPos = 0;
		do
		{
			WORD wBits = (WORD)ITReadBits(&bitbuf, &bitnum, &pSrc, pStop, bLeft);
			if (bLeft < 7)
			{
				DWORD i = 1 << (bLeft-1);
				DWORD j = wBits & 0xFFFF;
				if (i != j) goto UnpackByte;
				wBits = (WORD)(ITReadBits(&bitbuf, &bitnum, &pSrc, pStop, 3) + 1) & 0xFF;
				bLeft = ((BYTE)wBits < bLeft) ? (BYTE)wBits : (BYTE)((wBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft < 9)
			{
				WORD i = (0xFF >> (9 - bLeft)) + 4;
				WORD j = i - 8;
				if ((wBits <= j) || (wBits > i)) goto UnpackByte;
				wBits -= j;
				bLeft = ((BYTE)(wBits & 0xFF) < bLeft) ? (BYTE)(wBits & 0xFF) : (BYTE)((wBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft >= 10) goto SkipByte;
			if (wBits >= 256)
			{
				bLeft = (BYTE)(wBits + 1) & 0xFF;
				goto Next;
			}
		UnpackByte:
			if (bLeft < 8)
			{
				BYTE shift = 8 - bLeft;
				signed char c = (signed char)(wBits << shift);
				c >>= shift;
				wBits = (WORD)c;
			}
			wBits += bTemp;
			bTemp = (BYTE)wBits;
			bTemp2 += bTemp;
#ifdef IT215_SUPPORT
			pDst[writePos] = (b215) ? bTemp2 : bTemp;
#else
			pDst[writePos] = bTemp;
#endif
		SkipByte:
			dwPos++;
			writePos += channels;
		Next:
			if (pSrc >= pStop + 1) return (DWORD)(pSrc - lpMemFile);
		} while (dwPos < d);
		// Move On
		wCount -= d;
		dwLen -= d;
	}
	return (DWORD)(pSrc - lpMemFile);
}


DWORD ITUnpack16Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, DWORD channels, BOOL b215)
//--------------------------------------------------------------------------------------------------------------------
{
	signed short *pDst = (signed short *)pSample;
	LPBYTE pSrc = lpMemFile;
	LPBYTE pStop = lpMemFile + dwMemLength;
	DWORD writePos = 0;
//	DWORD wHdr = 0;
	DWORD wCount = 0;
	DWORD bitbuf = 0;
	UINT bitnum = 0;
	BYTE bLeft = 0;
	signed short wTemp = 0, wTemp2 = 0;

	while (dwLen)
	{
		DWORD d, dwPos;
		if (!wCount)
		{
			wCount = 0x4000;
		//	wHdr = bswapLE16(*((LPWORD)pSrc));
			pSrc += 2;
			bLeft = 17;
			wTemp = wTemp2 = 0;
			bitbuf = bitnum = 0;
		}
		d = wCount;
		if (d > dwLen) d = dwLen;
		// Unpacking
		dwPos = 0;
		do
		{
			DWORD dwBits = ITReadBits(&bitbuf, &bitnum, &pSrc, pStop, bLeft);
			if (bLeft < 7)
			{
				DWORD i = 1 << (bLeft-1);
				DWORD j = dwBits;
				if (i != j) goto UnpackByte;
				dwBits = ITReadBits(&bitbuf, &bitnum, &pSrc, pStop, 4) + 1;
				bLeft = ((BYTE)(dwBits & 0xFF) < bLeft) ? (BYTE)(dwBits & 0xFF) : (BYTE)((dwBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft < 17)
			{
				DWORD i = (0xFFFF >> (17 - bLeft)) + 8;
				DWORD j = (i - 16) & 0xFFFF;
				if ((dwBits <= j) || (dwBits > (i & 0xFFFF))) goto UnpackByte;
				dwBits -= j;
				bLeft = ((BYTE)(dwBits & 0xFF) < bLeft) ? (BYTE)(dwBits & 0xFF) : (BYTE)((dwBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft >= 18) goto SkipByte;
			if (dwBits >= 0x10000)
			{
				bLeft = (BYTE)(dwBits + 1) & 0xFF;
				goto Next;
			}
		UnpackByte:
			if (bLeft < 16)
			{
				BYTE shift = 16 - bLeft;
				signed short c = (signed short)(dwBits << shift);
				c >>= shift;
				dwBits = (DWORD)c;
			}
			dwBits += wTemp;
			wTemp = (signed short)dwBits;
			wTemp2 += wTemp;
#ifdef IT215_SUPPORT
			pDst[writePos] = (b215) ? wTemp2 : wTemp;
#else
			pDst[writePos] = wTemp;
#endif
		SkipByte:
			dwPos++;
			writePos += channels;
		Next:
			if (pSrc >= pStop + 1) return (DWORD)(pSrc - lpMemFile);
		} while (dwPos < d);
		// Move On
		wCount -= d;
		dwLen -= d;
		if (pSrc >= pStop) break;
	}
	return (DWORD)(pSrc - lpMemFile);
}

UINT CSoundFile_LoadMixPlugins(CSoundFile *_this, const void *pData, UINT nLen)
//-----------------------------------------------------------
{
	const BYTE *p = (const BYTE *)pData;
	UINT nPos = 0, ch;

	while (nPos+8 < nLen)
	{
		DWORD nPluginSize;

		nPluginSize = bswapLE32(*(DWORD *)(p+nPos+4));
		if (nPluginSize > nLen-nPos-8) break;;
		if ((bswapLE32(*(DWORD *)(p+nPos))) == 0x58464843)
		{
			for (ch=0; ch<64; ch++) if (ch*4 < nPluginSize)
			{
				_this->ChnSettings[ch].nMixPlugin = bswapLE32(*(DWORD *)(p+nPos+8+ch*4));
			}
		} else
		{
			if ((p[nPos] != 'F') || (p[nPos+1] != 'X')
			 || (p[nPos+2] < '0') || (p[nPos+3] < '0'))
			{
				break;
			}
			// took out the (otherwise unused) plugin loading code here.  --ryan.
		}
		nPos += nPluginSize + 8;
	}
	return nPos;
}
