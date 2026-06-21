/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

//////////////////////////////////////////////
// DigiTracker (MDL) module loader          //
//////////////////////////////////////////////
#include "libmodplug.h"

typedef struct MDLSONGHEADER
{
	DWORD id;	// "DMDL" = 0x4C444D44
	BYTE version;
} MDLSONGHEADER;


typedef struct MDLINFOBLOCK
{
	CHAR songname[32];
	CHAR composer[20];
	WORD norders;
	WORD repeatpos;
	BYTE globalvol;
	BYTE speed;
	BYTE tempo;
	BYTE channelinfo[32];
	BYTE seq[256];
} MDLINFOBLOCK;


typedef struct MDLPATTERNDATA
{
	BYTE channels;
	BYTE lastrow;	// nrows = lastrow+1
	CHAR name[16];
	WORD data[1];
} MDLPATTERNDATA;


static void ConvertMDLCommand(MODCOMMAND *m, UINT eff, UINT data)
//--------------------------------------------------------
{
	UINT command = 0, param = data;
	switch(eff)
	{
	case 0x01:	command = CMD_PORTAMENTOUP; break;
	case 0x02:	command = CMD_PORTAMENTODOWN; break;
	case 0x03:	command = CMD_TONEPORTAMENTO; break;
	case 0x04:	command = CMD_VIBRATO; break;
	case 0x05:	command = CMD_ARPEGGIO; break;
	case 0x07:	command = (param < 0x20) ? CMD_SPEED : CMD_TEMPO; break;
	case 0x08:	command = CMD_PANNING8; param <<= 1; break;
	case 0x0B:	command = CMD_POSITIONJUMP; break;
	case 0x0C:	command = CMD_GLOBALVOLUME; break;
	case 0x0D:	command = CMD_PATTERNBREAK; param = (data & 0x0F) + (data>>4)*10; break;
	case 0x0E:
		command = CMD_S3MCMDEX;
		switch(data & 0xF0)
		{
		case 0x00:	command = 0; break; // What is E0x in MDL (there is a bunch) ?
		case 0x10:	if (param & 0x0F) { param |= 0xF0; command = CMD_PANNINGSLIDE; } else command = 0; break;
		case 0x20:	if (param & 0x0F) { param = (param << 4) | 0x0F; command = CMD_PANNINGSLIDE; } else command = 0; break;
		case 0x30:	param = (data & 0x0F) | 0x10; break; // glissando
		case 0x40:	param = (data & 0x0F) | 0x30; break; // vibrato waveform
		case 0x60:	param = (data & 0x0F) | 0xB0; break;
		case 0x70:	param = (data & 0x0F) | 0x40; break; // tremolo waveform
		case 0x90:	command = CMD_RETRIG; param &= 0x0F; break;
		case 0xA0:	param = (data & 0x0F) << 4; command = CMD_GLOBALVOLSLIDE; break;
		case 0xB0:	param = data & 0x0F; command = CMD_GLOBALVOLSLIDE; break;
		case 0xF0:	param = ((data >> 8) & 0x0F) | 0xA0; break;
		}
		break;
	case 0x0F:	command = CMD_SPEED; break;
	case 0x10:	if ((param & 0xF0) != 0xE0) { command = CMD_VOLUMESLIDE; if ((param & 0xF0) == 0xF0) param = ((param << 4) | 0x0F); else param >>= 2; } break;
	case 0x20:	if ((param & 0xF0) != 0xE0) { command = CMD_VOLUMESLIDE; if ((param & 0xF0) != 0xF0) param >>= 2; } break;
	case 0x30:	command = CMD_RETRIG; break;
	case 0x40:	command = CMD_TREMOLO; break;
	case 0x50:	command = CMD_TREMOR; break;
	case 0xEF:	if (param > 0xFF) param = 0xFF; command = CMD_OFFSET; break;
	}
	if (command)
	{
		m->command = command;
		m->param = param;
	}
}


static void UnpackMDLTrack(MODCOMMAND *pat, UINT nChannels, UINT nRows, UINT nTrack, const BYTE *lpTracks, UINT len)
//-------------------------------------------------------------------------------------------------
{
	MODCOMMAND cmd, *m = pat;
	UINT pos = 0, row = 0, i;

	cmd.note = cmd.instr = 0;
	cmd.volcmd = cmd.vol = 0;
	cmd.command = cmd.param = 0;
	while ((row < nRows) && (pos < len))
	{
		UINT xx;
		BYTE b = lpTracks[pos++];
		xx = b >> 2;
		switch(b & 0x03)
		{
		case 0x01:
			for (i=0; i<=xx; i++)
			{
				if (row) *m = *(m-nChannels);
				m += nChannels;
				row++;
				if (row >= nRows) break;
			}
			break;

		case 0x02:
			if (xx < row) *m = pat[nChannels*xx];
			m += nChannels;
			row++;
			break;

		case 0x03:
			{
				UINT volume, commands, command1, command2, param1, param2;
				cmd.note = (xx & 0x01) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				cmd.instr = (xx & 0x02) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				cmd.volcmd = cmd.vol = 0;
				cmd.command = cmd.param = 0;
				if ((cmd.note < NOTE_MAX-12) && (cmd.note)) cmd.note += 12;
				volume = (xx & 0x04) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				commands = (xx & 0x08) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				command1 = commands & 0x0F;
				command2 = commands & 0xF0;
				param1 = (xx & 0x10) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				param2 = (xx & 0x20) ? (pos < len ? lpTracks[pos++] : 0) : 0;
				if ((command1 == 0x0E) && ((param1 & 0xF0) == 0xF0) && (!command2))
				{
					param1 = ((param1 & 0x0F) << 8) | param2;
					command1 = 0xEF;
					command2 = param2 = 0;
				}
				if (volume)
				{
					cmd.volcmd = VOLCMD_VOLUME;
					cmd.vol = (volume+1) >> 2;
				}
				ConvertMDLCommand(&cmd, command1, param1);
				if ((cmd.command != CMD_SPEED)
				 && (cmd.command != CMD_TEMPO)
				 && (cmd.command != CMD_PATTERNBREAK))
					ConvertMDLCommand(&cmd, command2, param2);
				*m = cmd;
				m += nChannels;
				row++;
			}
			break;

		// Empty Slots
		default:
			row += xx+1;
			m += (xx+1)*nChannels;
			if (row >= nRows) break;
		}
	}
}


BOOL CSoundFile_ReadMDL(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	DWORD dwMemPos, dwPos, blocklen, dwTrackPos;
	const MDLSONGHEADER *pmsh = (const MDLSONGHEADER *)lpStream;
	const MDLINFOBLOCK *pmib;
	UINT i,j, norders = 0, npatterns = 0, ntracks = 0;
	UINT ninstruments = 0, nsamples = 0;
	WORD block;
	WORD patterntracks[MAX_PATTERNS*32];
	BYTE smpinfo[MAX_SAMPLES];
	BYTE insvolenv[MAX_INSTRUMENTS];
	BYTE inspanenv[MAX_INSTRUMENTS];
	LPCBYTE pvolenv, ppanenv, ppitchenv;
	UINT nvolenv, npanenv, npitchenv;
	UINT hdrLen;

	if ((!lpStream) || (dwMemLength < 1024)) return FALSE;
	if ((pmsh->id != 0x4C444D44) || ((pmsh->version & 0xF0) > 0x10)) return FALSE;
	hdrLen = (pmsh->version>0)? 59 : 57;
	SDL_memset(patterntracks, 0, sizeof(patterntracks));
	SDL_memset(smpinfo, 0, sizeof(smpinfo));
	SDL_memset(insvolenv, 0, sizeof(insvolenv));
	SDL_memset(inspanenv, 0, sizeof(inspanenv));
	dwMemPos = 5;
	dwTrackPos = 0;
	pvolenv = ppanenv = ppitchenv = NULL;
	nvolenv = npanenv = npitchenv = 0;
	_this->m_nSamples = _this->m_nInstruments = 0;
	while (dwMemPos+6 < dwMemLength)
	{
		const BYTE *pp = lpStream + dwMemPos;
		block = pp[0] | (pp[1] << 8);
		blocklen = pp[2] | (pp[3] << 8) | (pp[4] << 16) | (pp[5] << 24);
		dwMemPos += 6;
		if (blocklen > dwMemLength - dwMemPos)
		{
			if (dwMemPos == 11) return FALSE;
			break;
		}
		switch(block)
		{
		// IN: infoblock
		case 0x4E49:
			if (blocklen < sizeof(MDLINFOBLOCK)) break;
			pmib = (const MDLINFOBLOCK *)(lpStream+dwMemPos);
			norders = pmib->norders;
			if (norders > MAX_ORDERS) norders = MAX_ORDERS;
			if (blocklen < sizeof(MDLINFOBLOCK) + norders - sizeof(pmib->seq)) return FALSE;
			_this->m_nRestartPos = pmib->repeatpos;
			_this->m_nDefaultGlobalVolume = pmib->globalvol;
			_this->m_nDefaultTempo = pmib->tempo;
			_this->m_nDefaultSpeed = pmib->speed;
			_this->m_nChannels = 4;
			for (i=0; i<32; i++)
			{
				_this->ChnSettings[i].nVolume = 64;
				_this->ChnSettings[i].nPan = (pmib->channelinfo[i] & 0x7F) << 1;
				if (pmib->channelinfo[i] & 0x80)
					_this->ChnSettings[i].dwFlags |= CHN_MUTE;
				else
					_this->m_nChannels = i+1;
			}
			for (j=0; j<norders; j++) _this->Order[j] = pmib->seq[j];
			break;
		// ME: song message
		case 0x454D:
			break;
		// PA: Pattern Data
		case 0x4150:
			npatterns = lpStream[dwMemPos];
			if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
			dwPos = dwMemPos + 1;
			for (i=0; i<npatterns; i++)
			{
				const BYTE *data;
				UINT ch;
				if (pmsh->version == 0) {
					if (_this->m_nChannels < 32) _this->m_nChannels = 32;
					ch = 32;
				} else {
					const MDLPATTERNDATA *pmpd;
					if (dwPos+18 >= dwMemLength) break;
					pmpd = (const MDLPATTERNDATA *)(lpStream + dwPos);
					if (pmpd->channels > 32) break;
					_this->PatternSize[i] = pmpd->lastrow+1;
					if (_this->m_nChannels < pmpd->channels) _this->m_nChannels = pmpd->channels;
					ch = pmpd->channels;
				}

				if (2 * ch >= dwMemLength - dwPos) break;
				data = lpStream + dwPos;
				dwPos += 2 * ch;
				for (j=0; j<ch && j<_this->m_nChannels; j++, data+=2)
				{
					patterntracks[i*32+j] = data[0] | (data[1] << 8);
				}
			}
			break;
		// TR: Track Data
		case 0x5254:
			if (blocklen < 2) break;
			if (dwTrackPos) break;
			pp = lpStream + dwMemPos;
			ntracks = pp[0] | (pp[1] << 8);
			dwTrackPos = dwMemPos+2;
			break;
		// II: Instruments
		case 0x4949:
			ninstruments = lpStream[dwMemPos];
			dwPos = dwMemPos+1;
			if (blocklen < sizeof(INSTRUMENTHEADER)*ninstruments + 1) break;

			for (i=0; i<ninstruments; i++)
			{
				UINT nins = lpStream[dwPos];
				if ((nins >= MAX_INSTRUMENTS) || (!nins)) break;
				if (_this->m_nInstruments < nins) _this->m_nInstruments = nins;
				if (!_this->Headers[nins])
				{
					INSTRUMENTHEADER *penv;
					UINT note = 12;
					if ((_this->Headers[nins] = (INSTRUMENTHEADER *) SDL_calloc(1, sizeof (INSTRUMENTHEADER))) == NULL) break;
					penv = _this->Headers[nins];
					if (dwPos > dwMemLength - 34) break;
					penv->nGlobalVol = 64;
					penv->nPPC = 5*12;
					if (34 + 14u*lpStream[dwPos+1] > dwMemLength - dwPos) break;
					for (j=0; j<lpStream[dwPos+1]; j++)
					{
						const BYTE *ps = lpStream+dwPos+34+14*j;
						while ((note < (UINT)(ps[1]+12)) && (note < NOTE_MAX))
						{
							penv->NoteMap[note] = note+1;
							if (ps[0] < MAX_SAMPLES)
							{
								int ismp = ps[0];
								penv->Keyboard[note] = ps[0];
								_this->Ins[ismp].nVolume = ps[2];
								_this->Ins[ismp].nPan = ps[4] << 1;
								_this->Ins[ismp].nVibType = ps[11];
								_this->Ins[ismp].nVibSweep = ps[10];
								_this->Ins[ismp].nVibDepth = ps[9];
								_this->Ins[ismp].nVibRate = ps[8];
							}
							penv->nFadeOut = (ps[7] << 8) | ps[6];
							if (penv->nFadeOut == 0xFFFF) penv->nFadeOut = 0;
							note++;
						}
						// Use volume envelope ?
						if (ps[3] & 0x80)
						{
							penv->dwFlags |= ENV_VOLUME;
							insvolenv[nins] = (ps[3] & 0x3F) + 1;
						}
						// Use panning envelope ?
						if (ps[5] & 0x80)
						{
							penv->dwFlags |= ENV_PANNING;
							inspanenv[nins] = (ps[5] & 0x3F) + 1;
						}
					}
				}
				dwPos += 34 + 14*lpStream[dwPos+1];
				if (dwPos > dwMemLength - 2) break;
			}
			for (j=1; j<=_this->m_nInstruments; j++) if (!_this->Headers[j])
			{
				_this->Headers[j] = (INSTRUMENTHEADER *) SDL_calloc(1, sizeof (INSTRUMENTHEADER));
			}
			break;
		// VE: Volume Envelope
		case 0x4556:
			if (nvolenv || (nvolenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + nvolenv*33 + 1 <= dwMemLength) pvolenv = lpStream + dwMemPos + 1;
			break;
		// PE: Panning Envelope
		case 0x4550:
			if (npanenv || (npanenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npanenv*33 + 1 <= dwMemLength) ppanenv = lpStream + dwMemPos + 1;
			break;
		// FE: Pitch Envelope
		case 0x4546:
			if (npitchenv || (npitchenv = lpStream[dwMemPos]) == 0) break;
			if (dwMemPos + npitchenv*33 + 1 <= dwMemLength) ppitchenv = lpStream + dwMemPos + 1;
			break;
		// IS: Sample Infoblock
		case 0x5349:
			nsamples = lpStream[dwMemPos];
			i = blocklen / hdrLen;
			if (i< nsamples) nsamples = i;
			dwPos = dwMemPos+1;
			for (i=0; i<nsamples; i++, dwPos += hdrLen)
			{
				UINT nins = lpStream[dwPos];
				MODINSTRUMENT *pins;
				if ((nins >= MAX_SAMPLES) || (!nins)) continue;
				if (_this->m_nSamples < nins) _this->m_nSamples = nins;
				pins = &_this->Ins[nins];
				pp = lpStream + dwPos + 41;
				pins->nC4Speed = pp[0] | (pp[1] << 8); pp += 2;
				if (pmsh->version > 0) {
					pins->nC4Speed |= (pp[0] << 16) | (pp[1] << 24); pp += 2;
				}
				pins->nLength = pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24); pp += 4;
				pins->nLoopStart = pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24); pp += 4;
				pins->nLoopEnd = pins->nLoopStart + (pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24));
				if (pins->nLoopEnd > pins->nLoopStart) pins->uFlags |= CHN_LOOP;
				pins->nGlobalVol = 64;
				if (pmsh->version == 0) pins->nVolume = pp[4];
				if (lpStream[dwPos+hdrLen-1] & 0x01)
				{
					pins->uFlags |= CHN_16BIT;
					pins->nLength >>= 1;
					pins->nLoopStart >>= 1;
					pins->nLoopEnd >>= 1;
				}
				if (lpStream[dwPos+hdrLen-1] & 0x02) pins->uFlags |= CHN_PINGPONGLOOP;
				smpinfo[nins] = (lpStream[dwPos+hdrLen-1] >> 2) & 3;
			}
			break;
		// SA: Sample Data
		case 0x4153:
			dwPos = dwMemPos;
			for (i=1; i<=_this->m_nSamples; i++) if ((_this->Ins[i].nLength) && (!_this->Ins[i].pSample) && (smpinfo[i] != 3) && (dwPos < dwMemLength))
			{
				MODINSTRUMENT *pins = &_this->Ins[i];
				UINT flags = (pins->uFlags & CHN_16BIT) ? RS_PCM16S : RS_PCM8S;
				pp = lpStream + dwPos;
				if (!smpinfo[i])
				{
					dwPos += CSoundFile_ReadSample(_this, pins, flags, (LPCSTR)pp, dwMemLength - dwPos);
				} else
				{
					DWORD dwLen = pp[0] | (pp[1] << 8) | (pp[2] << 16) | (pp[3] << 24); pp += 4;
					dwPos += 4;
					if ((dwLen <= dwMemLength) && (dwPos <= dwMemLength - dwLen) && (dwLen > 4))
					{
						flags = (pins->uFlags & CHN_16BIT) ? RS_MDL16 : RS_MDL8;
						CSoundFile_ReadSample(_this, pins, flags, (LPCSTR)pp, dwLen);
					}
					dwPos += dwLen;
				}
			}
			break;
		}
		dwMemPos += blocklen;
	}
	// Unpack Patterns
	if ((dwTrackPos) && (npatterns) && (_this->m_nChannels) && (ntracks))
	{
		for (i=0; i<npatterns; i++)
		{
			UINT chn;
			if ((_this->Patterns[i] = CSoundFile_AllocatePattern(_this->PatternSize[i], _this->m_nChannels)) == NULL) break;
			for (chn=0; chn<_this->m_nChannels; chn++) if ((patterntracks[i*32+chn]) && (patterntracks[i*32+chn] <= ntracks))
			{
			    const BYTE *lpTracks = lpStream + dwTrackPos;
			    UINT len = 0;
			    if (dwTrackPos + 2 < dwMemLength)
				len = lpTracks[0] | (lpTracks[1] << 8);

			    if (len < dwMemLength-dwTrackPos) {
				MODCOMMAND *m = _this->Patterns[i] + chn;
				UINT nTrack = patterntracks[i*32+chn];

				lpTracks += 2;
				for (j=1; j<nTrack && lpTracks < (dwMemLength + lpStream - len - 2); j++)
				{
					lpTracks += len;
					len = lpTracks[0] | (lpTracks[1] << 8);
					lpTracks += 2;
				}

				if ( len > dwMemLength - (lpTracks - lpStream) ) len = 0;

				UnpackMDLTrack(m, _this->m_nChannels, _this->PatternSize[i], nTrack, lpTracks, len);
			    }
			}
		}
	}
	// Set up envelopes
	for (i=1; i<=_this->m_nInstruments; i++) if (_this->Headers[i])
	{
		INSTRUMENTHEADER *penv = _this->Headers[i];
		// Setup volume envelope
		if ((nvolenv) && (pvolenv) && (insvolenv[i]))
		{
			LPCBYTE pve = pvolenv;
			UINT nve;
			for (nve=0; nve<nvolenv; nve++, pve+=33) if (pve[0]+1 == insvolenv[i])
			{
				WORD vtick = 1;
				penv->nVolEnv = 15;
				for (j=0; j<15; j++)
				{
					if (j) vtick += pve[j*2+1];
					penv->VolPoints[j] = vtick;
					penv->VolEnv[j] = pve[j*2+2];
					if (!pve[j*2+1])
					{
						penv->nVolEnv = j+1;
						break;
					}
				}
				penv->nVolSustainBegin = penv->nVolSustainEnd = pve[31] & 0x0F;
				if (pve[31] & 0x10) penv->dwFlags |= ENV_VOLSUSTAIN;
				if (pve[31] & 0x20) penv->dwFlags |= ENV_VOLLOOP;
				penv->nVolLoopStart = pve[32] & 0x0F;
				penv->nVolLoopEnd = pve[32] >> 4;
			}
		}
		// Setup panning envelope
		if ((npanenv) && (ppanenv) && (inspanenv[i]))
		{
			LPCBYTE ppe = ppanenv;
			UINT npe;
			for (npe=0; npe<npanenv; npe++, ppe+=33) if (ppe[0]+1 == inspanenv[i])
			{
				WORD vtick = 1;
				penv->nPanEnv = 15;
				for (j=0; j<15; j++)
				{
					if (j) vtick += ppe[j*2+1];
					penv->PanPoints[j] = vtick;
					penv->PanEnv[j] = ppe[j*2+2];
					if (!ppe[j*2+1])
					{
						penv->nPanEnv = j+1;
						break;
					}
				}
				if (ppe[31] & 0x10) penv->dwFlags |= ENV_PANSUSTAIN;
				if (ppe[31] & 0x20) penv->dwFlags |= ENV_PANLOOP;
				penv->nPanLoopStart = ppe[32] & 0x0F;
				penv->nPanLoopEnd = ppe[32] >> 4;
			}
		}
	}
	_this->m_dwSongFlags |= SONG_LINEARSLIDES;
	_this->m_nType = MOD_TYPE_MDL;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////
// MDL Sample Unpacking

// MDL Huffman ReadBits compression
WORD MDLReadBits(DWORD *bitbuf, UINT *bitnum, LPBYTE *_ibuf, CHAR n)
//-----------------------------------------------------------------
{
	LPBYTE ibuf = *_ibuf;
	const WORD v = (WORD)(*bitbuf & ((1 << n) - 1) );
	*bitbuf >>= n;
	*bitnum -= n;
	if (*bitnum <= 24)
	{
		*bitbuf |= (((DWORD)(*ibuf++)) << *bitnum);
		*bitnum += 8;
	}
	*_ibuf = ibuf;
	return v;
}
