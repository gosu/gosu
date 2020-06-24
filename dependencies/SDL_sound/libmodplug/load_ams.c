/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//////////////////////////////////////////////
// AMS module loader                        //
//////////////////////////////////////////////
#include "libmodplug.h"

#pragma pack(1)

typedef struct AMSFILEHEADER
{
	char szHeader[7];	// "Extreme"   // changed from CHAR
	BYTE verlo, verhi;	// 0x??,0x01
	BYTE chncfg;
	BYTE samples;
	WORD patterns;
	WORD orders;
	BYTE vmidi;
	WORD extra;
} AMSFILEHEADER;

typedef struct AMSSAMPLEHEADER
{
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	BYTE finetune_and_pan;
	WORD samplerate;	// C-2 = 8363
	BYTE volume;		// 0-127
	BYTE infobyte;
} AMSSAMPLEHEADER;


#pragma pack()


BOOL CSoundFile_ReadAMS(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	BYTE pkinf[MAX_SAMPLES];
	AMSFILEHEADER *pfh = (AMSFILEHEADER *)lpStream;
	DWORD dwMemPos;
	UINT tmp, tmp2;
	
	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((pfh->verhi != 0x01) || (SDL_strncmp(pfh->szHeader, "Extreme", 7))
	 || (!pfh->patterns) || (!pfh->orders) || (!pfh->samples) || (pfh->samples >= MAX_SAMPLES)
	 || (pfh->patterns > MAX_PATTERNS) || (pfh->orders > MAX_ORDERS))
	{
		return CSoundFile_ReadAMS2(_this, lpStream, dwMemLength);
	}
	dwMemPos = sizeof(AMSFILEHEADER) + pfh->extra;
	if (dwMemPos + pfh->samples * sizeof(AMSSAMPLEHEADER) + 256 >= dwMemLength) return FALSE;
	_this->m_nType = MOD_TYPE_AMS;
	_this->m_nInstruments = 0;
	_this->m_nChannels = (pfh->chncfg & 0x1F) + 1;
	_this->m_nSamples = pfh->samples;
	for (UINT nSmp=1; nSmp<=_this->m_nSamples; nSmp++, dwMemPos += sizeof(AMSSAMPLEHEADER))
	{
		AMSSAMPLEHEADER *psh = (AMSSAMPLEHEADER *)(lpStream + dwMemPos);
		MODINSTRUMENT *pins = &_this->Ins[nSmp];
		pins->nLength = psh->length;
		pins->nLoopStart = psh->loopstart;
		pins->nLoopEnd = psh->loopend;
		pins->nGlobalVol = 64;
		pins->nVolume = psh->volume << 1;
		pins->nC4Speed = psh->samplerate;
		pins->nPan = (psh->finetune_and_pan & 0xF0);
		if (pins->nPan < 0x80) pins->nPan += 0x10;
		pins->nFineTune = MOD2XMFineTune(psh->finetune_and_pan & 0x0F);
		pins->uFlags = (psh->infobyte & 0x80) ? CHN_16BIT : 0;
		if ((pins->nLoopEnd <= pins->nLength) && (pins->nLoopStart+4 <= pins->nLoopEnd)) pins->uFlags |= CHN_LOOP;
		pkinf[nSmp] = psh->infobyte;
	}
	// Read Song Name
	tmp = lpStream[dwMemPos++];
	if (dwMemPos + tmp + 1 >= dwMemLength) return TRUE;
	dwMemPos += tmp;
	// Read sample names
	for (UINT sNam=1; sNam<=_this->m_nSamples; sNam++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		dwMemPos += tmp;
	}
	// Skip Channel names
	for (UINT cNam=0; cNam<_this->m_nChannels; cNam++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		dwMemPos += tmp;
	}
	// Read Pattern Names
	_this->m_lpszPatternNames = (char *) SDL_malloc(pfh->patterns * 32);  // changed from CHAR
	if (!_this->m_lpszPatternNames) return TRUE;
	_this->m_nPatternNames = pfh->patterns;
	SDL_memset(_this->m_lpszPatternNames, 0, _this->m_nPatternNames * 32);
	for (UINT pNam=0; pNam < _this->m_nPatternNames; pNam++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		tmp2 = (tmp < 32) ? tmp : 31;
		if (tmp2) SDL_memcpy(_this->m_lpszPatternNames+pNam*32, lpStream+dwMemPos, tmp2);
		dwMemPos += tmp;
	}
	// Read Song Comments
	tmp = *((WORD *)(lpStream+dwMemPos));
	dwMemPos += 2;
	if (dwMemPos + tmp >= dwMemLength) return TRUE;
	if (tmp)
	{
		dwMemPos += tmp;
	}
	// Read Order List
	for (UINT iOrd=0; iOrd<pfh->orders; iOrd++, dwMemPos += 2)
	{
		UINT n = *((WORD *)(lpStream+dwMemPos));
		_this->Order[iOrd] = (BYTE)n;
	}
	// Read Patterns
	for (UINT iPat=0; iPat<pfh->patterns; iPat++)
	{
		if (dwMemPos + 4 >= dwMemLength) return TRUE;
		UINT len = *((DWORD *)(lpStream + dwMemPos));
		dwMemPos += 4;
		if ((len >= dwMemLength) || (dwMemPos + len > dwMemLength)) return TRUE;
		_this->PatternSize[iPat] = 64;
		MODCOMMAND *m = CSoundFile_AllocatePattern(_this->PatternSize[iPat], _this->m_nChannels);
		if (!m) return TRUE;
		_this->Patterns[iPat] = m;
		const BYTE *p = lpStream + dwMemPos;
		UINT row = 0, i = 0;
		while ((row < _this->PatternSize[iPat]) && (i+2 < len))
		{
			BYTE b0 = p[i++];
			BYTE b1 = p[i++];
			BYTE b2 = 0;
			UINT ch = b0 & 0x3F;
			// Note+Instr
			if (!(b0 & 0x40))
			{
				b2 = p[i++];
				if (ch < _this->m_nChannels)
				{
					if (b1 & 0x7F) m[ch].note = (b1 & 0x7F) + 25;
					m[ch].instr = b2;
				}
				if (b1 & 0x80)
				{
					b0 |= 0x40;
					b1 = p[i++];
				}
			}
			// Effect
			if (b0 & 0x40)
			{
			anothercommand:
				if (b1 & 0x40)
				{
					if (ch < _this->m_nChannels)
					{
						m[ch].volcmd = VOLCMD_VOLUME;
						m[ch].vol = b1 & 0x3F;
					}
				} else
				{
					b2 = p[i++];
					if (ch < _this->m_nChannels)
					{
						UINT cmd = b1 & 0x3F;
						if (cmd == 0x0C)
						{
							m[ch].volcmd = VOLCMD_VOLUME;
							m[ch].vol = b2 >> 1;
						} else
						if (cmd == 0x0E)
						{
							if (!m[ch].command)
							{
								UINT command = CMD_S3MCMDEX;
								UINT param = b2;
								switch(param & 0xF0)
								{
								case 0x00:	if (param & 0x08) { param &= 0x07; param |= 0x90; } else {command=param=0;} break;
								case 0x10:	command = CMD_PORTAMENTOUP; param |= 0xF0; break;
								case 0x20:	command = CMD_PORTAMENTODOWN; param |= 0xF0; break;
								case 0x30:	param = (param & 0x0F) | 0x10; break;
								case 0x40:	param = (param & 0x0F) | 0x30; break;
								case 0x50:	param = (param & 0x0F) | 0x20; break;
								case 0x60:	param = (param & 0x0F) | 0xB0; break;
								case 0x70:	param = (param & 0x0F) | 0x40; break;
								case 0x90:	command = CMD_RETRIG; param &= 0x0F; break;
								case 0xA0:	if (param & 0x0F) { command = CMD_VOLUMESLIDE; param = (param << 4) | 0x0F; } else command=param=0; break;
								case 0xB0:	if (param & 0x0F) { command = CMD_VOLUMESLIDE; param |= 0xF0; } else command=param=0; break;
								}
								m[ch].command = command;
								m[ch].param = param;
							}
						} else
						{
							m[ch].command = cmd;
							m[ch].param = b2;
							CSoundFile_ConvertModCommand(_this, &m[ch]);
						}
					}
				}
				if (b1 & 0x80)
				{
					b1 = p[i++];
					if (i <= len) goto anothercommand;
				}
			}
			if (b0 & 0x80)
			{
				row++;
				m += _this->m_nChannels;
			}
		}
		dwMemPos += len;
	}
	// Read Samples
	for (UINT iSmp=1; iSmp<=_this->m_nSamples; iSmp++) if (_this->Ins[iSmp].nLength)
	{
		if (dwMemPos >= dwMemLength - 9) return TRUE;
		UINT flags = (_this->Ins[iSmp].uFlags & CHN_16BIT) ? RS_AMS16 : RS_AMS8;
		dwMemPos += CSoundFile_ReadSample(_this, &_this->Ins[iSmp], flags, (LPSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// AMS 2.2 loader

#pragma pack(1)

typedef struct AMS2FILEHEADER
{
	DWORD dwHdr1;		// AMShdr
	WORD wHdr2;
	BYTE b1A;			// 0x1A
	BYTE titlelen;		// 30-bytes max
	CHAR szTitle[30];	// [titlelen]
} AMS2FILEHEADER;

typedef struct AMS2SONGHEADER
{
	WORD version;
	BYTE instruments;
	WORD patterns;
	WORD orders;
	WORD bpm;
	BYTE speed;
	BYTE channels;
	BYTE commands;
	BYTE rows;
	WORD flags;
} AMS2SONGHEADER;

typedef struct AMS2INSTRUMENT
{
	BYTE samples;
	BYTE notemap[NOTE_MAX];
} AMS2INSTRUMENT;

typedef struct AMS2ENVELOPE
{
	BYTE speed;
	BYTE sustain;
	BYTE loopbegin;
	BYTE loopend;
	BYTE points;
	BYTE info[3];
} AMS2ENVELOPE;

typedef struct AMS2SAMPLE
{
	DWORD length;
	DWORD loopstart;
	DWORD loopend;
	WORD frequency;
	BYTE finetune;
	WORD c4speed;
	CHAR transpose;
	BYTE volume;
	BYTE flags;
} AMS2SAMPLE;


#pragma pack()


BOOL CSoundFile_ReadAMS2(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength)
//------------------------------------------------------------
{
	const AMS2FILEHEADER *pfh = (AMS2FILEHEADER *)lpStream;
	AMS2SONGHEADER *psh;
	DWORD dwMemPos;
	BYTE smpmap[16];
	BYTE packedsamples[MAX_SAMPLES];

	if ((pfh->dwHdr1 != 0x68534D41) || (pfh->wHdr2 != 0x7264)
	 || (pfh->b1A != 0x1A) || (pfh->titlelen > 30)) return FALSE;
	dwMemPos = pfh->titlelen + 8;
	psh = (AMS2SONGHEADER *)(lpStream + dwMemPos);
	if (((psh->version & 0xFF00) != 0x0200) || (!psh->instruments)
	 || (psh->instruments >= MAX_INSTRUMENTS) || (!psh->patterns) || (!psh->orders)) return FALSE;
	dwMemPos += sizeof(AMS2SONGHEADER);
	_this->m_nType = MOD_TYPE_AMS;
	_this->m_nChannels = 32;
	_this->m_nDefaultTempo = psh->bpm >> 8;
	_this->m_nDefaultSpeed = psh->speed;
	_this->m_nInstruments = psh->instruments;
	_this->m_nSamples = 0;
	if (psh->flags & 0x40) _this->m_dwSongFlags |= SONG_LINEARSLIDES;
	for (UINT nIns=1; nIns<=_this->m_nInstruments; nIns++)
	{
		const UINT insnamelen = lpStream[dwMemPos];
		CHAR *pinsname = (CHAR *)(lpStream+dwMemPos+1);
		dwMemPos += insnamelen + 1;
		AMS2INSTRUMENT *pins = (AMS2INSTRUMENT *)(lpStream + dwMemPos);
		dwMemPos += sizeof(AMS2INSTRUMENT);
		if (dwMemPos + 1024 >= dwMemLength) return TRUE;
		AMS2ENVELOPE *volenv, *panenv, *pitchenv;
		volenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + volenv->points*3;
		panenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + panenv->points*3;
		pitchenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + pitchenv->points*3;
		INSTRUMENTHEADER *penv = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER));
		if (!penv) return TRUE;
		SDL_memset(smpmap, 0, sizeof(smpmap));
		SDL_memset(penv, 0, sizeof(INSTRUMENTHEADER));
		for (UINT ismpmap=0; ismpmap<pins->samples; ismpmap++)
		{
			if ((ismpmap >= 16) || (_this->m_nSamples+1 >= MAX_SAMPLES)) break;
			_this->m_nSamples++;
			smpmap[ismpmap] = _this->m_nSamples;
		}
		penv->nGlobalVol = 64;
		penv->nPan = 128;
		penv->nPPC = 60;
		_this->Headers[nIns] = penv;
		for (UINT inotemap=0; inotemap<NOTE_MAX; inotemap++)
		{
			penv->NoteMap[inotemap] = inotemap+1;
			penv->Keyboard[inotemap] = smpmap[pins->notemap[inotemap] & 0x0F];
		}
		// Volume Envelope
		{
			UINT pos = 0;
			penv->nVolEnv = (volenv->points > 16) ? 16 : volenv->points;
			penv->nVolSustainBegin = penv->nVolSustainEnd = volenv->sustain;
			penv->nVolLoopStart = volenv->loopbegin;
			penv->nVolLoopEnd = volenv->loopend;
			for (UINT i=0; i<penv->nVolEnv; i++)
			{
				penv->VolEnv[i] = (BYTE)((volenv->info[i*3+2] & 0x7F) >> 1);
				pos += volenv->info[i*3] + ((volenv->info[i*3+1] & 1) << 8);
				penv->VolPoints[i] = (WORD)pos;
			}
		}
		penv->nFadeOut = (((lpStream[dwMemPos+2] & 0x0F) << 8) | (lpStream[dwMemPos+1])) << 3;
		UINT envflags = lpStream[dwMemPos+3];
		if (envflags & 0x01) penv->dwFlags |= ENV_VOLLOOP;
		if (envflags & 0x02) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (envflags & 0x04) penv->dwFlags |= ENV_VOLUME;
		dwMemPos += 5;
		// Read Samples
		for (UINT ismp=0; ismp<pins->samples; ismp++)
		{
			MODINSTRUMENT *psmp = ((ismp < 16) && (smpmap[ismp])) ? &_this->Ins[smpmap[ismp]] : NULL;
			const UINT smpnamelen = lpStream[dwMemPos];
			dwMemPos += smpnamelen + 1;
			if (psmp)
			{
				AMS2SAMPLE *pams = (AMS2SAMPLE *)(lpStream+dwMemPos);
				psmp->nGlobalVol = 64;
				psmp->nPan = 128;
				psmp->nLength = pams->length;
				psmp->nLoopStart = pams->loopstart;
				psmp->nLoopEnd = pams->loopend;
				psmp->nC4Speed = pams->c4speed;
				psmp->RelativeTone = pams->transpose;
				psmp->nVolume = pams->volume / 2;
				packedsamples[smpmap[ismp]] = pams->flags;
				if (pams->flags & 0x04) psmp->uFlags |= CHN_16BIT;
				if (pams->flags & 0x08) psmp->uFlags |= CHN_LOOP;
				if (pams->flags & 0x10) psmp->uFlags |= CHN_PINGPONGLOOP;
			}
			dwMemPos += sizeof(AMS2SAMPLE);
		}
	}
	if (dwMemPos + 256 >= dwMemLength) return TRUE;
	// Comments
	{
		UINT composernamelen = lpStream[dwMemPos];
		dwMemPos += composernamelen + 1;
		// channel names
		for (UINT i=0; i<32; i++)
		{
			const UINT chnnamlen = lpStream[dwMemPos];
			dwMemPos += chnnamlen + 1;
			if (dwMemPos + chnnamlen + 256 >= dwMemLength) return TRUE;
		}
		// packed comments (ignored)
		UINT songtextlen = *((LPDWORD)(lpStream+dwMemPos));
		dwMemPos += songtextlen;
		if (dwMemPos + 256 >= dwMemLength) return TRUE;
	}
	// Order List
	{
		for (UINT i=0; i<MAX_ORDERS; i++)
		{
			_this->Order[i] = 0xFF;
			if (dwMemPos + 2 >= dwMemLength) return TRUE;
			if (i < psh->orders)
			{
				_this->Order[i] = lpStream[dwMemPos];
				dwMemPos += 2;
			}
		}
	}
	// Pattern Data
	for (UINT ipat=0; ipat<psh->patterns; ipat++)
	{
		if (dwMemPos+8 >= dwMemLength) return TRUE;
		UINT packedlen = *((LPDWORD)(lpStream+dwMemPos));
		UINT numrows = 1 + (UINT)(lpStream[dwMemPos+4]);
		//UINT patchn = 1 + (UINT)(lpStream[dwMemPos+5] & 0x1F);
		//UINT patcmds = 1 + (UINT)(lpStream[dwMemPos+5] >> 5);
		UINT patnamlen = lpStream[dwMemPos+6];
		dwMemPos += 4;
		if ((ipat < MAX_PATTERNS) && (packedlen < dwMemLength-dwMemPos) && (numrows >= 8))
		{
			if ((patnamlen) && (patnamlen < MAX_PATTERNNAME))
			{
				char s[MAX_PATTERNNAME]; // changed from CHAR
				SDL_memcpy(s, lpStream+dwMemPos+3, patnamlen);
				s[patnamlen] = 0;
				CSoundFile_SetPatternName(_this, ipat, s);
			}
			_this->PatternSize[ipat] = numrows;
			_this->Patterns[ipat] = CSoundFile_AllocatePattern(numrows, _this->m_nChannels);
			if (!_this->Patterns[ipat]) return TRUE;
			// Unpack Pattern Data
			LPCBYTE psrc = lpStream + dwMemPos;
			UINT pos = 3 + patnamlen;
			UINT row = 0;
			while ((pos < packedlen) && (row < numrows))
			{
				MODCOMMAND *m = _this->Patterns[ipat] + row * _this->m_nChannels;
				UINT byte1 = psrc[pos++];
				UINT ch = byte1 & 0x1F;
				// Read Note + Instr
				if (!(byte1 & 0x40))
				{
					UINT byte2 = psrc[pos++];
					UINT note = byte2 & 0x7F;
					if (note) m[ch].note = (note > 1) ? (note-1) : 0xFF;
					m[ch].instr = psrc[pos++];
					// Read Effect
					while (byte2 & 0x80)
					{
						byte2 = psrc[pos++];
						if (byte2 & 0x40)
						{
							m[ch].volcmd = VOLCMD_VOLUME;
							m[ch].vol = byte2 & 0x3F;
						} else
						{
							UINT command = byte2 & 0x3F;
							UINT param = psrc[pos++];
							if (command == 0x0C)
							{
								m[ch].volcmd = VOLCMD_VOLUME;
								m[ch].vol = param / 2;
							} else
							if (command < 0x10)
							{
								m[ch].command = command;
								m[ch].param = param;
								CSoundFile_ConvertModCommand(_this, &m[ch]);
							} else
							{
								// TODO: AMS effects
							}
						}
					}
				}
				if (byte1 & 0x80) row++;
			}
		}
		dwMemPos += packedlen;
	}
	// Read Samples
	for (UINT iSmp=1; iSmp<=_this->m_nSamples; iSmp++) if (_this->Ins[iSmp].nLength)
	{
		if (dwMemPos >= dwMemLength - 9) return TRUE;
		UINT flags;
		if (packedsamples[iSmp] & 0x03)
		{
			flags = (_this->Ins[iSmp].uFlags & CHN_16BIT) ? RS_AMS16 : RS_AMS8;
		} else
		{
			flags = (_this->Ins[iSmp].uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
		}
		dwMemPos += CSoundFile_ReadSample(_this, &_this->Ins[iSmp], flags, (LPSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// AMS Sample unpacking

void AMSUnpack(const char *psrc, UINT inputlen, char *pdest, UINT dmax, char packcharacter)
{
	UINT tmplen = dmax;
	signed char *amstmp = (signed char *) SDL_malloc(tmplen);
	
	if (!amstmp) return;
	// Unpack Loop
	{
		signed char *p = amstmp;
		UINT i=0, j=0;
		while ((i < inputlen) && (j < tmplen))
		{
			signed char ch = psrc[i++];
			if (ch == packcharacter)
			{
				BYTE ch2 = psrc[i++];
				if (ch2)
				{
					ch = psrc[i++];
					while (ch2--)
					{
						p[j++] = ch;
						if (j >= tmplen) break;
					}
				} else p[j++] = packcharacter;
			} else p[j++] = ch;
		}
	}
	// Bit Unpack Loop
	{
		signed char *p = amstmp;
		UINT bitcount = 0x80, dh;
		UINT k=0;
		for (UINT i=0; i<dmax; i++)
		{
			BYTE al = *p++;
			dh = 0;
			for (UINT count=0; count<8; count++)
			{
				UINT bl = al & bitcount;
				bl = ((bl|(bl<<8)) >> ((dh+8-count) & 7)) & 0xFF;
				bitcount = ((bitcount|(bitcount<<8)) >> 1) & 0xFF;
				pdest[k++] |= bl;
				if (k >= dmax)
				{
					k = 0;
					dh++;
				}
			}
			bitcount = ((bitcount|(bitcount<<8)) >> dh) & 0xFF;
		}
	}
	// Delta Unpack
	{
		signed char old = 0;
		for (UINT i=0; i<dmax; i++)
		{
			int pos = ((LPBYTE)pdest)[i];
			if ((pos != 128) && (pos & 0x80)) pos = -(pos & 0x7F);
			old -= (signed char)pos;
			pdest[i] = old;
		}
	}
	SDL_free(amstmp);
}

