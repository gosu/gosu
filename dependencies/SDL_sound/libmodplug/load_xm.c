/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

#include "libmodplug.h"

////////////////////////////////////////////////////////
// FastTracker II XM file support

#ifdef MSC_VER
#pragma warning(disable:4244)
#endif

#pragma pack(1)
typedef struct tagXMFILEHEADER
{
	DWORD size;
	WORD norder;
	WORD restartpos;
	WORD channels;
	WORD patterns;
	WORD instruments;
	WORD flags;
	WORD speed;
	WORD tempo;
	BYTE order[256];
} XMFILEHEADER;


typedef struct tagXMINSTRUMENTHEADER
{
	DWORD size;
	CHAR name[22];
	BYTE type;
	BYTE samples;
	BYTE samplesh;
} XMINSTRUMENTHEADER;


typedef struct tagXMSAMPLEHEADER
{
	DWORD shsize;
	BYTE snum[96];
	WORD venv[24];
	WORD penv[24];
	BYTE vnum, pnum;
	BYTE vsustain, vloops, vloope, psustain, ploops, ploope;
	BYTE vtype, ptype;
	BYTE vibtype, vibsweep, vibdepth, vibrate;
	WORD volfade;
	WORD res;
	BYTE reserved1[20];
} XMSAMPLEHEADER;

typedef struct tagXMSAMPLESTRUCT
{
	DWORD samplen;
	DWORD loopstart;
	DWORD looplen;
	BYTE vol;
	signed char finetune;
	BYTE type;
	BYTE pan;
	signed char relnote;
	BYTE res;
	char name[22];
} XMSAMPLESTRUCT;
#pragma pack()


BOOL CSoundFile_ReadXM(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//--------------------------------------------------------------
{
	XMSAMPLEHEADER xmsh;
	XMSAMPLESTRUCT xmss;
	DWORD dwMemPos, dwHdrSize;
	WORD norders=0, restartpos=0, channels=0, patterns=0, instruments=0;
	WORD xmflags=0, deftempo=125, defspeed=6;
	BOOL InstUsed[256];
	BYTE channels_used[MAX_CHANNELS];
	BYTE pattern_map[256];
	BOOL samples_used[MAX_SAMPLES];
	UINT unused_samples;
	XMFILEHEADER xmhead;

	_this->m_nChannels = 0;
	if ((!lpStream) || (dwMemLength < 0x200)) return FALSE;
	if (SDL_strncasecmp((LPCSTR)lpStream, "Extended Module", 15)) return FALSE;

	SDL_memcpy(&xmhead, lpStream+60, sizeof (xmhead));
	dwHdrSize = bswapLE32(xmhead.size);
	norders = bswapLE16(xmhead.norder);
	if ((!norders) || (norders > MAX_ORDERS)) return FALSE;
	restartpos = bswapLE16(xmhead.restartpos);
	channels = bswapLE16(xmhead.channels);
	if ((!channels) || (channels > 64)) return FALSE;
	_this->m_nType = MOD_TYPE_XM;
	_this->m_nMinPeriod = 27;
	_this->m_nMaxPeriod = 54784;
	_this->m_nChannels = channels;
	if (restartpos < norders) _this->m_nRestartPos = restartpos;
	patterns = bswapLE16(xmhead.patterns);
	if (patterns > 256) patterns = 256;
	instruments = bswapLE16(xmhead.instruments);
	if (instruments >= MAX_INSTRUMENTS) instruments = MAX_INSTRUMENTS-1;
	_this->m_nInstruments = instruments;
	_this->m_nSamples = 0;
	xmflags = bswapLE16(xmhead.flags);
	if (xmflags & 1) _this->m_dwSongFlags |= SONG_LINEARSLIDES;
	if (xmflags & 0x1000) _this->m_dwSongFlags |= SONG_EXFILTERRANGE;
	defspeed = bswapLE16(xmhead.speed);
	deftempo = bswapLE16(xmhead.tempo);
	if ((deftempo >= 32) && (deftempo < 256)) _this->m_nDefaultTempo = deftempo;
	if ((defspeed > 0) && (defspeed < 40)) _this->m_nDefaultSpeed = defspeed;
	SDL_memcpy(_this->Order, lpStream+80, norders);
	SDL_memset(InstUsed, 0, sizeof(InstUsed));
	if (patterns > MAX_PATTERNS)
	{
		UINT i, j;
		for (i=0; i<norders; i++)
		{
			if (_this->Order[i] < patterns) InstUsed[_this->Order[i]] = TRUE;
		}
		j = 0;
		for (i=0; i<256; i++)
		{
			if (InstUsed[i]) pattern_map[i] = j++;
		}
		for (i=0; i<256; i++)
		{
			if (!InstUsed[i])
			{
				pattern_map[i] = (j < MAX_PATTERNS) ? j : 0xFE;
				j++;
			}
		}
		for (i=0; i<norders; i++)
		{
			_this->Order[i] = pattern_map[_this->Order[i]];
		}
	} else
	{
		for (UINT i=0; i<256; i++) pattern_map[i] = i;
	}
	SDL_memset(InstUsed, 0, sizeof(InstUsed));
	dwMemPos = dwHdrSize + 60;
	if (dwMemPos + 8 >= dwMemLength) return TRUE;
	// Reading patterns
	SDL_memset(channels_used, 0, sizeof(channels_used));
	for (UINT ipat=0; ipat<patterns; ipat++)
	{
		UINT ipatmap = pattern_map[ipat];
		DWORD dwSize = 0;
		WORD rows=64, packsize=0;
		dwSize = bswapLE32(*((DWORD *)(lpStream+dwMemPos)));
		while ((dwMemPos + dwSize >= dwMemLength) || (dwSize & 0xFFFFFF00))
		{
			if (dwMemPos + 4 >= dwMemLength) break;
			dwMemPos++;
			dwSize = bswapLE32(*((DWORD *)(lpStream+dwMemPos)));
		}
		rows = bswapLE16(*((WORD *)(lpStream+dwMemPos+5)));
		if ((!rows) || (rows > 256)) rows = 64;
		packsize = bswapLE16(*((WORD *)(lpStream+dwMemPos+7)));
		if (dwMemPos + dwSize + 4 > dwMemLength) return TRUE;
		dwMemPos += dwSize;
		if (dwMemPos + packsize + 4 > dwMemLength) return TRUE;
		MODCOMMAND *p;
		if (ipatmap < MAX_PATTERNS)
		{
			_this->PatternSize[ipatmap] = rows;
			if ((_this->Patterns[ipatmap] = CSoundFile_AllocatePattern(rows, _this->m_nChannels)) == NULL) return TRUE;
			if (!packsize) continue;
			p = _this->Patterns[ipatmap];
		} else p = NULL;
		const BYTE *src = lpStream+dwMemPos;
		UINT j=0;
		for (UINT row=0; row<rows; row++)
		{
			for (UINT chn=0; chn<_this->m_nChannels; chn++)
			{
				if ((p) && (j < packsize))
				{
					BYTE b = src[j++];
					UINT vol = 0;
					if (b & 0x80)
					{
						if (b & 1) p->note = src[j++];
						if (b & 2) p->instr = src[j++];
						if (b & 4) vol = src[j++];
						if (b & 8) p->command = src[j++];
						if (b & 16) p->param = src[j++];
					} else
					{
						p->note = b;
						p->instr = src[j++];
						vol = src[j++];
						p->command = src[j++];
						p->param = src[j++];
					}
					if (p->note == 97) p->note = 0xFF; else
					if ((p->note) && (p->note < 97)) p->note += 12;
					if (p->note) channels_used[chn] = 1;
					if (p->command | p->param) CSoundFile_ConvertModCommand(_this, p);
					if (p->instr == 0xff) p->instr = 0;
					if (p->instr) InstUsed[p->instr] = TRUE;
					if ((vol >= 0x10) && (vol <= 0x50))
					{
						p->volcmd = VOLCMD_VOLUME;
						p->vol = vol - 0x10;
					} else
					if (vol >= 0x60)
					{
						UINT v = vol & 0xF0;
						vol &= 0x0F;
						p->vol = vol;
						switch(v)
						{
						// 60-6F: Volume Slide Down
						case 0x60:	p->volcmd = VOLCMD_VOLSLIDEDOWN; break;
						// 70-7F: Volume Slide Up:
						case 0x70:	p->volcmd = VOLCMD_VOLSLIDEUP; break;
						// 80-8F: Fine Volume Slide Down
						case 0x80:	p->volcmd = VOLCMD_FINEVOLDOWN; break;
						// 90-9F: Fine Volume Slide Up
						case 0x90:	p->volcmd = VOLCMD_FINEVOLUP; break;
						// A0-AF: Set Vibrato Speed
						case 0xA0:	p->volcmd = VOLCMD_VIBRATOSPEED; break;
						// B0-BF: Vibrato
						case 0xB0:	p->volcmd = VOLCMD_VIBRATO; break;
						// C0-CF: Set Panning
						case 0xC0:	p->volcmd = VOLCMD_PANNING; p->vol = (vol << 2) + 2; break;
						// D0-DF: Panning Slide Left
						case 0xD0:	p->volcmd = VOLCMD_PANSLIDELEFT; break;
						// E0-EF: Panning Slide Right
						case 0xE0:	p->volcmd = VOLCMD_PANSLIDERIGHT; break;
						// F0-FF: Tone Portamento
						case 0xF0:	p->volcmd = VOLCMD_TONEPORTAMENTO; break;
						}
					}
					p++;
				} else
				if (j < packsize)
				{
					BYTE b = src[j++];
					if (b & 0x80)
					{
						if (b & 1) j++;
						if (b & 2) j++;
						if (b & 4) j++;
						if (b & 8) j++;
						if (b & 16) j++;
					} else j += 4;
				} else break;
			}
		}
		dwMemPos += packsize;
	}
	// Wrong offset check
	while (dwMemPos + 4 < dwMemLength)
	{
		DWORD d = bswapLE32(*((DWORD *)(lpStream+dwMemPos)));
		if (d < 0x300) break;
		dwMemPos++;
	}
	SDL_memset(samples_used, 0, sizeof(samples_used));
	unused_samples = 0;
	// Reading instruments
	for (UINT iIns=1; iIns<=instruments; iIns++)
	{
		XMINSTRUMENTHEADER *pih;
		BYTE flags[32];
		DWORD samplesize[32];
		UINT samplemap[32];
		WORD nsamples;

		if (dwMemPos + sizeof(XMINSTRUMENTHEADER) >= dwMemLength) return TRUE;
		pih = (XMINSTRUMENTHEADER *)(lpStream+dwMemPos);
		if (dwMemPos + bswapLE32(pih->size) > dwMemLength) return TRUE;
		if ((_this->Headers[iIns] = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER))) == NULL) continue;
		SDL_memset(_this->Headers[iIns], 0, sizeof(INSTRUMENTHEADER));
		if ((nsamples = pih->samples) > 0)
		{
			if (dwMemPos + sizeof(XMSAMPLEHEADER) > dwMemLength) return TRUE;
			SDL_memcpy(&xmsh, lpStream+dwMemPos+sizeof(XMINSTRUMENTHEADER), sizeof(XMSAMPLEHEADER));
			xmsh.shsize = bswapLE32(xmsh.shsize);
			for (int i = 0; i < 24; ++i) {
			  xmsh.venv[i] = bswapLE16(xmsh.venv[i]);
			  xmsh.penv[i] = bswapLE16(xmsh.penv[i]);
			}
			xmsh.volfade = bswapLE16(xmsh.volfade);
			xmsh.res = bswapLE16(xmsh.res);
			dwMemPos += bswapLE32(pih->size);
		} else
		{
			if (bswapLE32(pih->size)) dwMemPos += bswapLE32(pih->size);
			else dwMemPos += sizeof(XMINSTRUMENTHEADER);
			continue;
		}
		SDL_memset(samplemap, 0, sizeof(samplemap));
		if (nsamples > 32) return TRUE;
		UINT newsamples = _this->m_nSamples;
		for (UINT nmap=0; nmap<nsamples; nmap++)
		{
			UINT n = _this->m_nSamples+nmap+1;
			if (n >= MAX_SAMPLES)
			{
				n = _this->m_nSamples;
				while (n > 0)
				{
					if (!_this->Ins[n].pSample)
					{
						for (UINT xmapchk=0; xmapchk < nmap; xmapchk++)
						{
							if (samplemap[xmapchk] == n) goto alreadymapped;
						}
						for (UINT clrs=1; clrs<iIns; clrs++) if (_this->Headers[clrs])
						{
							INSTRUMENTHEADER *pks = _this->Headers[clrs];
							for (UINT ks=0; ks<128; ks++)
							{
								if (pks->Keyboard[ks] == n) pks->Keyboard[ks] = 0;
							}
						}
						break;
					}
				alreadymapped:
					n--;
				}
				// Damn! more than 200 samples: look for duplicates
				if (!n)
				{
					if (!unused_samples)
					{
						unused_samples = CSoundFile_DetectUnusedSamples(_this, samples_used);
						if (!unused_samples) unused_samples = 0xFFFF;
					}
					if ((unused_samples) && (unused_samples != 0xFFFF))
					{
						for (UINT iext=_this->m_nSamples; iext>=1; iext--) if (!samples_used[iext])
						{
							unused_samples--;
							samples_used[iext] = TRUE;
							CSoundFile_DestroySample(_this, iext);
							n = iext;
							for (UINT mapchk=0; mapchk<nmap; mapchk++)
							{
								if (samplemap[mapchk] == n) samplemap[mapchk] = 0;
							}
							for (UINT clrs=1; clrs<iIns; clrs++) if (_this->Headers[clrs])
							{
								INSTRUMENTHEADER *pks = _this->Headers[clrs];
								for (UINT ks=0; ks<128; ks++)
								{
									if (pks->Keyboard[ks] == n) pks->Keyboard[ks] = 0;
								}
							}
							SDL_memset(&_this->Ins[n], 0, sizeof(_this->Ins[0]));
							break;
						}
					}
				}
			}
			if (newsamples < n) newsamples = n;
			samplemap[nmap] = n;
		}
		_this->m_nSamples = newsamples;
		// Reading Volume Envelope
		INSTRUMENTHEADER *penv = _this->Headers[iIns];
		penv->nMidiProgram = pih->type;
		penv->nFadeOut = xmsh.volfade;
		penv->nPan = 128;
		penv->nPPC = 5*12;
		if (xmsh.vtype & 1) penv->dwFlags |= ENV_VOLUME;
		if (xmsh.vtype & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (xmsh.vtype & 4) penv->dwFlags |= ENV_VOLLOOP;
		if (xmsh.ptype & 1) penv->dwFlags |= ENV_PANNING;
		if (xmsh.ptype & 2) penv->dwFlags |= ENV_PANSUSTAIN;
		if (xmsh.ptype & 4) penv->dwFlags |= ENV_PANLOOP;
		if (xmsh.vnum > 12) xmsh.vnum = 12;
		if (xmsh.pnum > 12) xmsh.pnum = 12;
		penv->nVolEnv = xmsh.vnum;
		if (!xmsh.vnum) penv->dwFlags &= ~ENV_VOLUME;
		if (!xmsh.pnum) penv->dwFlags &= ~ENV_PANNING;
		penv->nPanEnv = xmsh.pnum;
		penv->nVolSustainBegin = penv->nVolSustainEnd = xmsh.vsustain;
		if (xmsh.vsustain >= 12) penv->dwFlags &= ~ENV_VOLSUSTAIN;
		penv->nVolLoopStart = xmsh.vloops;
		penv->nVolLoopEnd = xmsh.vloope;
		if (penv->nVolLoopEnd >= 12) penv->nVolLoopEnd = 0;
		if (penv->nVolLoopStart >= penv->nVolLoopEnd) penv->dwFlags &= ~ENV_VOLLOOP;
		penv->nPanSustainBegin = penv->nPanSustainEnd = xmsh.psustain;
		if (xmsh.psustain >= 12) penv->dwFlags &= ~ENV_PANSUSTAIN;
		penv->nPanLoopStart = xmsh.ploops;
		penv->nPanLoopEnd = xmsh.ploope;
		if (penv->nPanLoopEnd >= 12) penv->nPanLoopEnd = 0;
		if (penv->nPanLoopStart >= penv->nPanLoopEnd) penv->dwFlags &= ~ENV_PANLOOP;
		penv->nGlobalVol = 64;
		for (UINT ienv=0; ienv<12; ienv++)
		{
			penv->VolPoints[ienv] = (WORD)xmsh.venv[ienv*2];
			penv->VolEnv[ienv] = (BYTE)xmsh.venv[ienv*2+1];
			penv->PanPoints[ienv] = (WORD)xmsh.penv[ienv*2];
			penv->PanEnv[ienv] = (BYTE)xmsh.penv[ienv*2+1];
			if (ienv)
			{
				if (penv->VolPoints[ienv] < penv->VolPoints[ienv-1])
				{
					penv->VolPoints[ienv] &= 0xFF;
					penv->VolPoints[ienv] += penv->VolPoints[ienv-1] & 0xFF00;
					if (penv->VolPoints[ienv] < penv->VolPoints[ienv-1]) penv->VolPoints[ienv] += 0x100;
				}
				if (penv->PanPoints[ienv] < penv->PanPoints[ienv-1])
				{
					penv->PanPoints[ienv] &= 0xFF;
					penv->PanPoints[ienv] += penv->PanPoints[ienv-1] & 0xFF00;
					if (penv->PanPoints[ienv] < penv->PanPoints[ienv-1]) penv->PanPoints[ienv] += 0x100;
				}
			}
		}
		for (UINT j=0; j<96; j++)
		{
			penv->NoteMap[j+12] = j+1+12;
			if (xmsh.snum[j] < nsamples)
				penv->Keyboard[j+12] = samplemap[xmsh.snum[j]];
		}
		// Reading samples
		for (UINT ins=0; ins<nsamples; ins++)
		{
			if ((dwMemPos + sizeof(xmss) > dwMemLength)
			 || (dwMemPos + xmsh.shsize > dwMemLength)) return TRUE;
			SDL_memcpy(&xmss, lpStream+dwMemPos, sizeof(xmss));
			xmss.samplen = bswapLE32(xmss.samplen);
			xmss.loopstart = bswapLE32(xmss.loopstart);
			xmss.looplen = bswapLE32(xmss.looplen);
			dwMemPos += xmsh.shsize;
			flags[ins] = (xmss.type & 0x10) ? RS_PCM16D : RS_PCM8D;
			if (xmss.type & 0x20) flags[ins] = (xmss.type & 0x10) ? RS_STPCM16D : RS_STPCM8D;
			samplesize[ins] = xmss.samplen;
			if (!samplemap[ins]) continue;
			if (xmss.type & 0x10)
			{
				xmss.looplen >>= 1;
				xmss.loopstart >>= 1;
				xmss.samplen >>= 1;
			}
			if (xmss.type & 0x20)
			{
				xmss.looplen >>= 1;
				xmss.loopstart >>= 1;
				xmss.samplen >>= 1;
			}
			if (xmss.samplen > MAX_SAMPLE_LENGTH) xmss.samplen = MAX_SAMPLE_LENGTH;
			if (xmss.loopstart >= xmss.samplen) xmss.type &= ~3;
			xmss.looplen += xmss.loopstart;
			if (xmss.looplen > xmss.samplen) xmss.looplen = xmss.samplen;
			if (!xmss.looplen) xmss.type &= ~3;
			UINT imapsmp = samplemap[ins];
			MODINSTRUMENT *pins = &_this->Ins[imapsmp];
			pins->nLength = (xmss.samplen > MAX_SAMPLE_LENGTH) ? MAX_SAMPLE_LENGTH : xmss.samplen;
			pins->nLoopStart = xmss.loopstart;
			pins->nLoopEnd = xmss.looplen;
			if (pins->nLoopEnd > pins->nLength) pins->nLoopEnd = pins->nLength;
			if (pins->nLoopStart >= pins->nLoopEnd)
			{
				pins->nLoopStart = pins->nLoopEnd = 0;
			}
			if (xmss.type & 3) pins->uFlags |= CHN_LOOP;
			if (xmss.type & 2) pins->uFlags |= CHN_PINGPONGLOOP;
			pins->nVolume = xmss.vol << 2;
			if (pins->nVolume > 256) pins->nVolume = 256;
			pins->nGlobalVol = 64;
			if ((xmss.res == 0xAD) && (!(xmss.type & 0x30)))
			{
				flags[ins] = RS_ADPCM4;
				samplesize[ins] = (samplesize[ins]+1)/2 + 16;
			}
			pins->nFineTune = xmss.finetune;
			pins->RelativeTone = (int)xmss.relnote;
			pins->nPan = xmss.pan;
			pins->uFlags |= CHN_PANNING;
			pins->nVibType = xmsh.vibtype;
			pins->nVibSweep = xmsh.vibsweep;
			pins->nVibDepth = xmsh.vibdepth;
			pins->nVibRate = xmsh.vibrate;
		}
		for (UINT ismpd=0; ismpd<nsamples; ismpd++)
		{
			if ((samplemap[ismpd]) && (samplesize[ismpd]) && (dwMemPos < dwMemLength))
			{
				CSoundFile_ReadSample(_this, &_this->Ins[samplemap[ismpd]], flags[ismpd], (LPSTR)(lpStream + dwMemPos), dwMemLength - dwMemPos);
			}
			dwMemPos += samplesize[ismpd];
			if (dwMemPos >= dwMemLength) break;
		}
	}
	// Read song comments: "TEXT"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x74786574))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len < 16384))
		{
			dwMemPos += len;
		}
	}
	// Read midi config: "MIDI"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4944494D))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if (len == sizeof(MODMIDICFG))
		{
			SDL_memcpy(&_this->m_MidiCfg, lpStream+dwMemPos, len);
			_this->m_dwSongFlags |= SONG_EMBEDMIDICFG;
		}
	}
	// Read pattern names: "PNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e50))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
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
	// Read channel names: "CNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e43))
	{
		UINT len = *((DWORD *)(lpStream+dwMemPos+4));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= MAX_BASECHANNELS*MAX_CHANNELNAME))
		{
			dwMemPos += len;
		}
	}
	// Read mix plugins information
	if (dwMemPos + 8 < dwMemLength)
	{
		dwMemPos += CSoundFile_LoadMixPlugins(_this, lpStream+dwMemPos, dwMemLength-dwMemPos);
	}
	return TRUE;
}

