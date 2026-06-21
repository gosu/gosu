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

static BOOL AMSUnpackCheck(const BYTE *lpStream, DWORD dwMemLength, MODINSTRUMENT *ins);

BOOL CSoundFile_ReadAMS(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
//	BYTE pkinf[MAX_SAMPLES];
	const AMSFILEHEADER *pfh = (AMSFILEHEADER *)lpStream;
	DWORD dwMemPos;
	UINT tmp, tmp2, i;

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
	for (i=1; i<=_this->m_nSamples; i++, dwMemPos += sizeof(AMSSAMPLEHEADER))
	{
		const AMSSAMPLEHEADER *psh = (AMSSAMPLEHEADER *)(lpStream + dwMemPos);
		MODINSTRUMENT *pins = &_this->Ins[i];
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
	//	pkinf[i] = psh->infobyte;
	}
	// Read Song Name
	tmp = lpStream[dwMemPos++];
	if (dwMemPos + tmp + 1 >= dwMemLength) return TRUE;
	dwMemPos += tmp;
	// Read sample names
	for (i=1; i<=_this->m_nSamples; i++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		dwMemPos += tmp;
	}
	// Skip Channel names
	for (i=0; i<_this->m_nChannels; i++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		dwMemPos += tmp;
	}
	// Read Pattern Names
	_this->m_lpszPatternNames = (char *) SDL_calloc(pfh->patterns, 32);  // changed from CHAR
	if (!_this->m_lpszPatternNames) return TRUE;
	_this->m_nPatternNames = pfh->patterns;
	for (i=0; i < _this->m_nPatternNames; i++)
	{
		if (dwMemPos + 32 >= dwMemLength) return TRUE;
		tmp = lpStream[dwMemPos++];
		tmp2 = (tmp < 32) ? tmp : 31;
		if (tmp2) SDL_memcpy(_this->m_lpszPatternNames+i*32, lpStream+dwMemPos, tmp2);
		dwMemPos += tmp;
	}
	// Read Song Comments
	if (dwMemPos + 2 > dwMemLength) return TRUE;
	tmp = *((WORD *)(lpStream+dwMemPos));
	dwMemPos += 2;
	if (tmp >= dwMemLength || dwMemPos > dwMemLength - tmp) return TRUE;
	if (tmp)
	{
		dwMemPos += tmp;
	}
	// Read Order List
	if (2*pfh->orders >= dwMemLength || dwMemPos > dwMemLength - 2*pfh->orders) return TRUE;
	for (i=0; i<pfh->orders; i++, dwMemPos += 2)
	{
		UINT n = *((WORD *)(lpStream+dwMemPos));
		_this->Order[i] = (BYTE)n;
	}
	// Read Patterns
	for (i=0; i<pfh->patterns; i++)
	{
		MODCOMMAND *m;
		const BYTE *p;
		UINT len, row, j;
		if (dwMemPos + 4 >= dwMemLength) return TRUE;
		len = *((DWORD *)(lpStream + dwMemPos));
		dwMemPos += 4;
		if ((len >= dwMemLength) || (dwMemPos > dwMemLength - len)) return TRUE;
		_this->PatternSize[i] = 64;
		m = CSoundFile_AllocatePattern(_this->PatternSize[i], _this->m_nChannels);
		if (!m) return TRUE;
		_this->Patterns[i] = m;
		p = lpStream + dwMemPos;
		row = 0, j = 0;
		while ((row < _this->PatternSize[i]) && (j+2 < len))
		{
			BYTE b0, b1, b2;
			UINT ch;

			b0 = p[j++];
			ch = b0 & 0x3F;

			if (b0 == 0xff)
			{
				row++;
				continue;
			}

			b1 = p[j++];
			b2 = 0;
			ch = b0 & 0x3F;
			// Note+Instr
			if (!(b0 & 0x40))
			{
				if (j+1 > len) break;
				b2 = p[j++];
				if (ch < _this->m_nChannels)
				{
					if (b1 & 0x7F) m[ch].note = (b1 & 0x7F) + 25;
					m[ch].instr = b2;
				}
				if (b1 & 0x80)
				{
					if (j+1 > len) break;
					b0 |= 0x40;
					b1 = p[j++];
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
					if (j+1 > len) break;
					b2 = p[j++];
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
					if (j+1 > len) break;
					b1 = p[j++];
					if (j <= len) goto anothercommand;
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
	for (i=1; i<=_this->m_nSamples; i++) if (_this->Ins[i].nLength)
	{
		UINT flags;
		if (dwMemPos >= dwMemLength - 9) return TRUE;
		flags = (_this->Ins[i].uFlags & CHN_16BIT) ? RS_AMS16 : RS_AMS8;
		if (!AMSUnpackCheck(lpStream+dwMemPos, dwMemLength-dwMemPos, &_this->Ins[i])) break;
		dwMemPos += CSoundFile_ReadSample(_this, &_this->Ins[i], flags, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
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
	const AMS2SONGHEADER *psh;
	DWORD dwMemPos;
	BYTE smpmap[16];
	BYTE packedsamples[MAX_SAMPLES];
	UINT i;

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
	for (i=1; i<=_this->m_nInstruments; i++)
	{
		const AMS2INSTRUMENT *pins;
		const AMS2ENVELOPE *volenv, *panenv, *pitchenv;
		INSTRUMENTHEADER *penv;
		UINT c, envflags;
		if (dwMemPos >= dwMemLength) return TRUE;
		c = lpStream[dwMemPos];/* insnamelen */
		dwMemPos += c + 1;
		pins = (AMS2INSTRUMENT *)(lpStream + dwMemPos);
		dwMemPos += sizeof(AMS2INSTRUMENT);
		if (dwMemPos + sizeof(AMS2ENVELOPE) > dwMemLength) return TRUE;
		volenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + volenv->points*3;
		if (dwMemPos + sizeof(AMS2ENVELOPE) > dwMemLength) return TRUE;
		panenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + panenv->points*3;
		if (dwMemPos + sizeof(AMS2ENVELOPE) > dwMemLength) return TRUE;
		pitchenv = (AMS2ENVELOPE *)(lpStream+dwMemPos);
		dwMemPos += 5 + pitchenv->points*3;
		if (dwMemPos >= dwMemLength) return TRUE;
		penv = (INSTRUMENTHEADER *) SDL_calloc(1, sizeof (INSTRUMENTHEADER));
		if (!penv) return TRUE;
		SDL_memset(smpmap, 0, sizeof(smpmap));
		for (c=0; c<pins->samples; c++)
		{
			if ((c >= 16) || (_this->m_nSamples+1 >= MAX_SAMPLES)) break;
			_this->m_nSamples++;
			smpmap[c] = _this->m_nSamples;
		}
		penv->nGlobalVol = 64;
		penv->nPan = 128;
		penv->nPPC = 60;
		_this->Headers[i] = penv;
		for (c=0; c<NOTE_MAX; c++)
		{
			penv->NoteMap[c] = c+1;
			penv->Keyboard[c] = smpmap[pins->notemap[c] & 0x0F];
		}
		// Volume Envelope
		{
			UINT e, pos = 0;
			penv->nVolEnv = (volenv->points > 16) ? 16 : volenv->points;
			penv->nVolSustainBegin = penv->nVolSustainEnd = volenv->sustain;
			penv->nVolLoopStart = volenv->loopbegin;
			penv->nVolLoopEnd = volenv->loopend;
			for (e=0; e<penv->nVolEnv; e++)
			{
				penv->VolEnv[e] = (BYTE)((volenv->info[e*3+2] & 0x7F) >> 1);
				pos += volenv->info[e*3] + ((volenv->info[e*3+1] & 1) << 8);
				penv->VolPoints[e] = (WORD)pos;
			}
		}
		if (dwMemPos + 5 > dwMemLength) return TRUE;
		penv->nFadeOut = (((lpStream[dwMemPos+2] & 0x0F) << 8) | (lpStream[dwMemPos+1])) << 3;
		envflags = lpStream[dwMemPos+3];
		if (envflags & 0x01) penv->dwFlags |= ENV_VOLLOOP;
		if (envflags & 0x02) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (envflags & 0x04) penv->dwFlags |= ENV_VOLUME;
		dwMemPos += 5;
		// Read Samples
		for (c=0; c<pins->samples; c++)
		{
			MODINSTRUMENT *psmp;
			UINT l;
			if (dwMemPos + 1 > dwMemLength) return TRUE;
			psmp = ((c < 16) && (smpmap[c])) ? &_this->Ins[smpmap[c]] : NULL;
			l = lpStream[dwMemPos]; /* namelen */
			if (dwMemPos + l + 1 > dwMemLength) return TRUE;
			dwMemPos += l + 1;
			if (dwMemPos + sizeof(AMS2SAMPLE) > dwMemLength) return TRUE;
			if (psmp)
			{
				const AMS2SAMPLE *pams = (AMS2SAMPLE *)(lpStream+dwMemPos);
				psmp->nGlobalVol = 64;
				psmp->nPan = 128;
				psmp->nLength = pams->length;
				psmp->nLoopStart = pams->loopstart;
				psmp->nLoopEnd = pams->loopend;
				psmp->nC4Speed = pams->c4speed;
				psmp->RelativeTone = pams->transpose;
				psmp->nVolume = pams->volume / 2;
				packedsamples[smpmap[c]] = pams->flags;
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
		UINT c = lpStream[dwMemPos]; /* namelen */
		dwMemPos += c + 1;
		// channel names
		for (i=0; i<32; i++)
		{
			c = lpStream[dwMemPos]; /* namelen */
			dwMemPos += c + 1;
			if (dwMemPos + c + 256 >= dwMemLength) return TRUE;
		}
		// packed comments (ignored)
		c = *((LPDWORD)(lpStream+dwMemPos));		/* songtextlen */
		dwMemPos += c;
		if (dwMemPos + 256 >= dwMemLength) return TRUE;
	}
	// Order List
	{
		for (i=0; i<MAX_ORDERS; i++)
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
	for (i=0; i<psh->patterns; i++)
	{
		//UINT patchn, patcmds;
		UINT packedlen;
		UINT numrows;
		UINT patnamlen;
		if (dwMemPos+8 >= dwMemLength) return TRUE;
		packedlen = *((LPDWORD)(lpStream+dwMemPos));
		numrows = 1 + (UINT)(lpStream[dwMemPos+4]);
		//patchn = 1 + (UINT)(lpStream[dwMemPos+5] & 0x1F);
		//patcmds = 1 + (UINT)(lpStream[dwMemPos+5] >> 5);
		patnamlen = lpStream[dwMemPos+6];
		dwMemPos += 4;
		if ((i < MAX_PATTERNS) && (packedlen < dwMemLength-dwMemPos) && (numrows >= 8))
		{
			LPCBYTE psrc;
			UINT pos,row;
			if ((patnamlen) && (patnamlen < MAX_PATTERNNAME))
			{
				char s[MAX_PATTERNNAME]; // changed from CHAR
				SDL_memcpy(s, lpStream+dwMemPos+3, patnamlen);
				s[patnamlen] = 0;
				CSoundFile_SetPatternName(_this, i, s);
			}
			_this->PatternSize[i] = numrows;
			_this->Patterns[i] = CSoundFile_AllocatePattern(numrows, _this->m_nChannels);
			if (!_this->Patterns[i]) return TRUE;
			// Unpack Pattern Data
			psrc = lpStream + dwMemPos;
			pos = 3 + patnamlen;
			row = 0;
			while ((pos < packedlen) && (row < numrows))
			{
				MODCOMMAND *m = _this->Patterns[i] + row * _this->m_nChannels;
				UINT byte1 = psrc[pos++];
				UINT byte2;
				UINT ch = byte1 & 0x1F;
				if (byte1 == 0xff)
				{
					row++;
					continue;
				}

				// Read Note + Instr
				if (!(byte1 & 0x40))
				{
					UINT note;
					byte2 = psrc[pos++];
					note = byte2 & 0x7F;
					if (note) m[ch].note = (note > 1) ? (note-1) : 0xFF;
					m[ch].instr = psrc[pos++];
				} else {
					byte2 = 0x80; /* row contains atleast one effect, so trigged the first parse */
				}
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
				if (byte1 & 0x80) row++;
			}
		}
		dwMemPos += packedlen;
	}
	// Read Samples
	for (i=1; i<=_this->m_nSamples; i++) if (_this->Ins[i].nLength)
	{
		UINT flags;
		if (dwMemPos >= dwMemLength - 9) return TRUE;
		if (packedsamples[i] & 0x03)
		{
			flags = (_this->Ins[i].uFlags & CHN_16BIT) ? RS_AMS16 : RS_AMS8;
			if (!AMSUnpackCheck(lpStream+dwMemPos, dwMemLength-dwMemPos, &_this->Ins[i])) break;
		} else
		{
			flags = (_this->Ins[i].uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
		}
		dwMemPos += CSoundFile_ReadSample(_this, &_this->Ins[i], flags, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
	}
	return TRUE;
}


// Precheck AMS packed sample size to determine whether or not it could fit the actual size.
static BOOL AMSUnpackCheck(const BYTE *lpStream, DWORD dwMemLength, MODINSTRUMENT *ins)
// -----------------------------------------------------------------------------------
{
	DWORD packedbytes;
	DWORD samplebytes;
	DWORD packedmin;

	if (dwMemLength < 9) return FALSE;
	packedbytes = *((DWORD *)(lpStream + 4));

	samplebytes = ins->nLength;
	if (samplebytes > MAX_SAMPLE_LENGTH) samplebytes = MAX_SAMPLE_LENGTH;
	if (ins->uFlags & CHN_16BIT) samplebytes *= 2;

	// RLE can pack a run of up to 255 bytes into 3 bytes.
	packedmin = (samplebytes * 3) >> 8;
	if (packedbytes < packedmin)
	{
		samplebytes = packedbytes * (255 / 3) + 2;
		ins->nLength = samplebytes;
		if (ins->uFlags & CHN_16BIT) ins->nLength >>= 1;
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
				BYTE ch2;
				if (i >= inputlen) break;
				ch2 = psrc[i++];
				if (ch2)
				{
					if (i >= inputlen) break;
					ch = psrc[i++];
					while (ch2--)
					{
						p[j++] = ch;
						if (j >= tmplen) break;
					}
				} else p[j++] = packcharacter;
			} else p[j++] = ch;
		}
		if (j < tmplen)
		{
			// Truncated or invalid; don't try to unpack this.
			SDL_free(amstmp);
			return;
		}
	}
	// Bit Unpack Loop
	{
		signed char *p = amstmp;
		UINT bitcount = 0x80, dh;
		UINT k=0, i, count;
		for (i=0; i<dmax; i++)
		{
			BYTE al = *p++;
			dh = 0;
			for (count=0; count<8; count++)
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
		UINT i;
		for (i=0; i<dmax; i++)
		{
			int pos = ((LPBYTE)pdest)[i];
			if ((pos != 128) && (pos & 0x80)) pos = -(pos & 0x7F);
			old -= (signed char)pos;
			pdest[i] = old;
		}
	}
	SDL_free(amstmp);
}

