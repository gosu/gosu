/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"
#include <stdlib.h>
#include "tables.h"

DWORD CSoundFile_GetLength(CSoundFile *_this, BOOL bAdjust, BOOL bTotal)
//----------------------------------------------------
{
	UINT dwElapsedTime=0, nRow=0, nCurrentPattern=0, nNextPattern=0, nPattern=0;
	UINT nMusicSpeed=_this->m_nDefaultSpeed, nMusicTempo=_this->m_nDefaultTempo, nNextRow=0;
	UINT nMaxRow = 0, nMaxPattern = 0;
	LONG nGlbVol = _this->m_nDefaultGlobalVolume, nOldGlbVolSlide = 0;
	BYTE samples[MAX_CHANNELS];
	BYTE instr[MAX_CHANNELS];
	BYTE notes[MAX_CHANNELS];
	BYTE vols[MAX_CHANNELS];
	BYTE oldparam[MAX_CHANNELS];
	BYTE chnvols[MAX_CHANNELS];
	DWORD patloop[MAX_CHANNELS];

	SDL_memset(instr, 0, sizeof(instr));
	SDL_memset(notes, 0, sizeof(notes));
	SDL_memset(vols, 0xFF, sizeof(vols));
	SDL_memset(patloop, 0, sizeof(patloop));
	SDL_memset(oldparam, 0, sizeof(oldparam));
	SDL_memset(chnvols, 64, sizeof(chnvols));
	SDL_memset(samples, 0, sizeof(samples));
	for (UINT icv=0; icv<_this->m_nChannels; icv++)
		chnvols[icv] = _this->ChnSettings[icv].nVolume;
	nMaxRow = _this->m_nNextRow;
	nMaxPattern = _this->m_nNextPattern;
	for (;;)
	{
		UINT nSpeedCount = 0;
		nRow = nNextRow;
		nCurrentPattern = nNextPattern;
		// Check if pattern is valid
		nPattern = _this->Order[nCurrentPattern];
		while (nPattern >= MAX_PATTERNS)
		{
			// End of song ?
			if ((nPattern == 0xFF) || (nCurrentPattern >= MAX_ORDERS))
			{
				goto EndMod;
			} else
			{
				nCurrentPattern++;
				nPattern = (nCurrentPattern < MAX_ORDERS) ? _this->Order[nCurrentPattern] : 0xFF;
			}
			nNextPattern = nCurrentPattern;
		}
		// Weird stuff?
		if ((nPattern >= MAX_PATTERNS) || (!_this->Patterns[nPattern])) break;
		// Should never happen
		if (nRow >= _this->PatternSize[nPattern]) nRow = 0;
		// Update next position
		nNextRow = nRow + 1;
		if (nNextRow >= _this->PatternSize[nPattern])
		{
			nNextPattern = nCurrentPattern + 1;
			nNextRow = 0;
		}
		if (!nRow)
		{
			for (UINT ipck=0; ipck<_this->m_nChannels; ipck++) patloop[ipck] = dwElapsedTime;
		}
		if (!bTotal)
		{
			if ((nCurrentPattern > nMaxPattern) || ((nCurrentPattern == nMaxPattern) && (nRow >= nMaxRow)))
			{
				if (bAdjust)
				{
					_this->m_nMusicSpeed = nMusicSpeed;
					_this->m_nMusicTempo = nMusicTempo;
				}
				break;
			}
		}
		MODCHANNEL *pChn = _this->Chn;
		MODCOMMAND *p = _this->Patterns[nPattern] + nRow * _this->m_nChannels;
		for (UINT nChn=0; nChn<_this->m_nChannels; p++,pChn++, nChn++) if (*((DWORD *)p))
		{
			UINT command = p->command;
			UINT param = p->param;
			UINT note = p->note;
			if (p->instr) { instr[nChn] = p->instr; notes[nChn] = 0; vols[nChn] = 0xFF; }
			if ((note) && (note <= NOTE_MAX)) notes[nChn] = note;
			if (p->volcmd == VOLCMD_VOLUME)	{ vols[nChn] = p->vol; }
			if (command) switch (command)
			{
			// Position Jump
			case CMD_POSITIONJUMP:
				if (param <= nCurrentPattern) goto EndMod;
				nNextPattern = param;
				nNextRow = 0;
				if (bAdjust)
				{
					pChn->nPatternLoopCount = 0;
					pChn->nPatternLoop = 0;
				}
				break;
			// Pattern Break
			case CMD_PATTERNBREAK:
				nNextRow = param;
				nNextPattern = nCurrentPattern + 1;
				if (bAdjust)
				{
					pChn->nPatternLoopCount = 0;
					pChn->nPatternLoop = 0;
				}
				break;
			// Set Speed
			case CMD_SPEED:
				if (!param) break;
				if ((param <= 0x20) || (_this->m_nType != MOD_TYPE_MOD))
				{
					if (param < 128) nMusicSpeed = param;
				}
				break;
			// Set Tempo
			case CMD_TEMPO:
				if ((bAdjust) && (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)))
				{
					if (param) pChn->nOldTempo = param; else param = pChn->nOldTempo;
				}
				if (param >= 0x20) nMusicTempo = param; else
				// Tempo Slide
				if ((param & 0xF0) == 0x10)
				{
					nMusicTempo += param & 0x0F;
					if (nMusicTempo > 255) nMusicTempo = 255;
				} else
				{
					nMusicTempo -= param & 0x0F;
					if (nMusicTempo < 32) nMusicTempo = 32;
				}
				break;
			// Pattern Delay
			case CMD_S3MCMDEX:
				if ((param & 0xF0) == 0x60) { nSpeedCount = param & 0x0F; break; } else
				if ((param & 0xF0) == 0xB0) { param &= 0x0F; param |= 0x60; }
			case CMD_MODCMDEX:
				if ((param & 0xF0) == 0xE0) nSpeedCount = (param & 0x0F) * nMusicSpeed; else
				if ((param & 0xF0) == 0x60)
				{
					if (param & 0x0F) dwElapsedTime += (dwElapsedTime - patloop[nChn]) * (param & 0x0F);
					else patloop[nChn] = dwElapsedTime;
				}
				break;
			}
			if (!bAdjust) continue;
			switch(command)
			{
			// Portamento Up/Down
			case CMD_PORTAMENTOUP:
			case CMD_PORTAMENTODOWN:
				if (param) pChn->nOldPortaUpDown = param;
				break;
			// Tone-Portamento
			case CMD_TONEPORTAMENTO:
				if (param) pChn->nPortamentoSlide = param << 2;
				break;
			// Offset
			case CMD_OFFSET:
				if (param) pChn->nOldOffset = param;
				break;
			// Volume Slide
			case CMD_VOLUMESLIDE:
			case CMD_TONEPORTAVOL:
			case CMD_VIBRATOVOL:
				if (param) pChn->nOldVolumeSlide = param;
				break;
			// Set Volume
			case CMD_VOLUME:
				vols[nChn] = param;
				break;
			// Global Volume
			case CMD_GLOBALVOLUME:
				if (!(_this->m_nType & (MOD_TYPE_IT))) param <<= 1;
				if (param > 128) param = 128;
				nGlbVol = param << 1;
				break;
			// Global Volume Slide
			case CMD_GLOBALVOLSLIDE:
				if (param) nOldGlbVolSlide = param; else param = nOldGlbVolSlide;
				if (((param & 0x0F) == 0x0F) && (param & 0xF0))
				{
					param >>= 4;
					if (_this->m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol += param << 1;
				} else
				if (((param & 0xF0) == 0xF0) && (param & 0x0F))
				{
					param = (param & 0x0F) << 1;
					if (_this->m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol -= param;
				} else
				if (param & 0xF0)
				{
					param >>= 4;
					param <<= 1;
					if (_this->m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol += param * nMusicSpeed;
				} else
				{
					param = (param & 0x0F) << 1;
					if (_this->m_nType != MOD_TYPE_IT) param <<= 1;
					nGlbVol -= param * nMusicSpeed;
				}
				if (nGlbVol < 0) nGlbVol = 0;
				if (nGlbVol > 256) nGlbVol = 256;
				break;
			case CMD_CHANNELVOLUME:
				if (param <= 64) chnvols[nChn] = param;
				break;
			case CMD_CHANNELVOLSLIDE:
				if (param) oldparam[nChn] = param; else param = oldparam[nChn];
				pChn->nOldChnVolSlide = param;
				if (((param & 0x0F) == 0x0F) && (param & 0xF0))
				{
					param = (param >> 4) + chnvols[nChn];
				} else
				if (((param & 0xF0) == 0xF0) && (param & 0x0F))
				{
					if (chnvols[nChn] > (int)(param & 0x0F)) param = chnvols[nChn] - (param & 0x0F);
					else param = 0;
				} else
				if (param & 0x0F)
				{
					param = (param & 0x0F) * nMusicSpeed;
					param = (chnvols[nChn] > param) ? chnvols[nChn] - param : 0;
				} else param = ((param & 0xF0) >> 4) * nMusicSpeed + chnvols[nChn];
				if (param > 64) param = 64;
				chnvols[nChn] = param;
				break;
			}
		}
		nSpeedCount += nMusicSpeed;
		dwElapsedTime += (2500 * nSpeedCount) / nMusicTempo;
	}
EndMod:
	if ((bAdjust) && (!bTotal))
	{
		_this->m_nGlobalVolume = nGlbVol;
		_this->m_nOldGlbVolSlide = nOldGlbVolSlide;
		for (UINT n=0; n<_this->m_nChannels; n++)
		{
			_this->Chn[n].nGlobalVol = chnvols[n];
			if (notes[n]) _this->Chn[n].nNewNote = notes[n];
			if (instr[n]) _this->Chn[n].nNewIns = instr[n];
			if (vols[n] != 0xFF)
			{
				if (vols[n] > 64) vols[n] = 64;
				_this->Chn[n].nVolume = vols[n] << 2;
			}
		}
	}
	return (dwElapsedTime+500) / 1000;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Effects

void CSoundFile_InstrumentChange(CSoundFile *_this, MODCHANNEL *pChn, UINT instr, BOOL bPorta, BOOL bUpdVol, BOOL bResetEnv)
//--------------------------------------------------------------------------------------------------------
{
	BOOL bInstrumentChanged = FALSE;

	if (instr >= MAX_INSTRUMENTS) return;
	INSTRUMENTHEADER *penv = _this->Headers[instr];
	MODINSTRUMENT *psmp = &_this->Ins[instr];
	UINT note = pChn->nNewNote;
	if ((penv) && (note) && (note <= 128))
	{
		if (penv->NoteMap[note-1] >= 0xFE) return;
		UINT n = penv->Keyboard[note-1];
		psmp = ((n) && (n < MAX_SAMPLES)) ? &_this->Ins[n] : NULL;
	} else
	if (_this->m_nInstruments)
	{
		if (note >= 0xFE) return;
		psmp = NULL;
	}
	// Update Volume
	if (bUpdVol) pChn->nVolume = (psmp) ? psmp->nVolume : 0;
	// bInstrumentChanged is used for IT carry-on env option
	if (penv != pChn->pHeader)
	{
		bInstrumentChanged = TRUE;
		pChn->pHeader = penv;
	} else
	{
		// Special XM hack
		if ((bPorta) && (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) && (penv)
		&& (pChn->pInstrument) && (psmp != pChn->pInstrument))
		{
			// FT2 doesn't change the sample in this case,
			// but still uses the sample info from the old one (bug?)
			return;
		}
	}
	// Instrument adjust
	pChn->nNewIns = 0;
	if (psmp)
	{
		if (penv)
		{
			pChn->nInsVol = (psmp->nGlobalVol * penv->nGlobalVol) >> 6;
			if (penv->dwFlags & ENV_SETPANNING) pChn->nPan = penv->nPan;
			pChn->nNNA = penv->nNNA;
		} else
		{
			pChn->nInsVol = psmp->nGlobalVol;
		}
		if (psmp->uFlags & CHN_PANNING) pChn->nPan = psmp->nPan;
	}
	// Reset envelopes
	if (bResetEnv)
	{
		if ((!bPorta) || (!(_this->m_nType & MOD_TYPE_IT)) || (_this->m_dwSongFlags & SONG_ITCOMPATMODE)
		 || (!pChn->nLength) || ((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol)))
		{
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			if ((_this->m_nType & MOD_TYPE_IT) && (!bInstrumentChanged) && (penv) && (!(pChn->dwFlags & (CHN_KEYOFF|CHN_NOTEFADE))))
			{
				if (!(penv->dwFlags & ENV_VOLCARRY)) pChn->nVolEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PANCARRY)) pChn->nPanEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PITCHCARRY)) pChn->nPitchEnvPosition = 0;
			} else
			{
				pChn->nVolEnvPosition = 0;
				pChn->nPanEnvPosition = 0;
				pChn->nPitchEnvPosition = 0;
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		} else
		if ((penv) && (!(penv->dwFlags & ENV_VOLUME)))
		{
			pChn->nVolEnvPosition = 0;
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
	}
	// Invalid sample ?
	if (!psmp)
	{
		pChn->pInstrument = NULL;
		pChn->nInsVol = 0;
		return;
	}
	// Tone-Portamento doesn't reset the pingpong direction flag
	if ((bPorta) && (psmp == pChn->pInstrument))
	{
		if (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)) return;
		pChn->dwFlags &= ~(CHN_KEYOFF|CHN_NOTEFADE);
		pChn->dwFlags = (pChn->dwFlags & (0xFFFFFF00 | CHN_PINGPONGFLAG)) | (psmp->uFlags);
	} else
	{
		pChn->dwFlags &= ~(CHN_KEYOFF|CHN_NOTEFADE|CHN_VOLENV|CHN_PANENV|CHN_PITCHENV);
		pChn->dwFlags = (pChn->dwFlags & 0xFFFFFF00) | (psmp->uFlags);
		if (penv)
		{
			if (penv->dwFlags & ENV_VOLUME) pChn->dwFlags |= CHN_VOLENV;
			if (penv->dwFlags & ENV_PANNING) pChn->dwFlags |= CHN_PANENV;
			if (penv->dwFlags & ENV_PITCH) pChn->dwFlags |= CHN_PITCHENV;
			if ((penv->dwFlags & ENV_PITCH) && (penv->dwFlags & ENV_FILTER))
			{
				if (!pChn->nCutOff) pChn->nCutOff = 0x7F;
			}
			if (penv->nIFC & 0x80) pChn->nCutOff = penv->nIFC & 0x7F;
			if (penv->nIFR & 0x80) pChn->nResonance = penv->nIFR & 0x7F;
		}
		pChn->nVolSwing = pChn->nPanSwing = 0;
	}
	pChn->pInstrument = psmp;
	pChn->nLength = psmp->nLength;
	pChn->nLoopStart = psmp->nLoopStart;
	pChn->nLoopEnd = psmp->nLoopEnd;
	pChn->nC4Speed = psmp->nC4Speed;
	pChn->pSample = psmp->pSample;
	pChn->nTranspose = psmp->RelativeTone;
	pChn->nFineTune = psmp->nFineTune;
	if (pChn->dwFlags & CHN_SUSTAINLOOP)
	{
		pChn->nLoopStart = psmp->nSustainStart;
		pChn->nLoopEnd = psmp->nSustainEnd;
		pChn->dwFlags |= CHN_LOOP;
		if (pChn->dwFlags & CHN_PINGPONGSUSTAIN) pChn->dwFlags |= CHN_PINGPONGLOOP;
	}
	if ((pChn->dwFlags & CHN_LOOP) && (pChn->nLoopEnd < pChn->nLength)) pChn->nLength = pChn->nLoopEnd;
}


void CSoundFile_NoteChange(CSoundFile *_this, UINT nChn, int note, BOOL bPorta, BOOL bResetEnv)
//---------------------------------------------------------------------------
{
	if (note < 1) return;
	MODCHANNEL * const pChn = &_this->Chn[nChn];
	MODINSTRUMENT *pins = pChn->pInstrument;
	INSTRUMENTHEADER *penv = pChn->pHeader;
	if ((penv) && (note <= 0x80))
	{
		UINT n = penv->Keyboard[note - 1];
		if ((n) && (n < MAX_SAMPLES)) pins = &_this->Ins[n];
		note = penv->NoteMap[note-1];
	}
	// Key Off
	if (note >= 0x80)	// 0xFE or invalid note => key off
	{
		// Key Off
		CSoundFile_KeyOff(_this, nChn);
		// Note Cut
		if (note == 0xFE)
		{
			pChn->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
			if ((!(_this->m_nType & MOD_TYPE_IT)) || (_this->m_nInstruments)) pChn->nVolume = 0;
			pChn->nFadeOutVol = 0;
		}
		return;
	}
	if (!pins) return;
	if ((!bPorta) && (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MED|MOD_TYPE_MT2)))
	{
		pChn->nTranspose = pins->RelativeTone;
		pChn->nFineTune = pins->nFineTune;
	}
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2|MOD_TYPE_MED)) note += pChn->nTranspose;
	if (note < 1) note = 1;
	if (note > 132) note = 132;
	pChn->nNote = note;
	if ((!bPorta) || (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))) pChn->nNewIns = 0;
	UINT period = CSoundFile_GetPeriodFromNote(_this, note, pChn->nFineTune, pChn->nC4Speed);
	if (period)
	{
		if ((!bPorta) || (!pChn->nPeriod)) pChn->nPeriod = period;
		pChn->nPortamentoDest = period;
		if ((!bPorta) || ((!pChn->nLength) && (!(_this->m_nType & MOD_TYPE_S3M))))
		{
			pChn->pInstrument = pins;
			pChn->pSample = pins->pSample;
			pChn->nLength = pins->nLength;
			pChn->nLoopEnd = pins->nLength;
			pChn->nLoopStart = 0;
			pChn->dwFlags = (pChn->dwFlags & 0xFFFFFF00) | (pins->uFlags);
			if (pChn->dwFlags & CHN_SUSTAINLOOP)
			{
				pChn->nLoopStart = pins->nSustainStart;
				pChn->nLoopEnd = pins->nSustainEnd;
				pChn->dwFlags &= ~CHN_PINGPONGLOOP;
				pChn->dwFlags |= CHN_LOOP;
				if (pChn->dwFlags & CHN_PINGPONGSUSTAIN) pChn->dwFlags |= CHN_PINGPONGLOOP;
				if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
			} else
			if (pChn->dwFlags & CHN_LOOP)
			{
				pChn->nLoopStart = pins->nLoopStart;
				pChn->nLoopEnd = pins->nLoopEnd;
				if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
			}
			pChn->nPos = 0;
			pChn->nPosLo = 0;
			if (pChn->nVibratoType < 4) pChn->nVibratoPos = ((_this->m_nType & MOD_TYPE_IT) && (!(_this->m_dwSongFlags & SONG_ITOLDEFFECTS))) ? 0x10 : 0;
			if (pChn->nTremoloType < 4) pChn->nTremoloPos = 0;
		}
		if (pChn->nPos >= pChn->nLength) pChn->nPos = pChn->nLoopStart;
	} else bPorta = FALSE;
	if ((!bPorta) || (!(_this->m_nType & MOD_TYPE_IT))
	 || ((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
	 || ((_this->m_dwSongFlags & SONG_ITCOMPATMODE) && (pChn->nRowInstr)))
	{
		if ((_this->m_nType & MOD_TYPE_IT) && (pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
		{
			pChn->nVolEnvPosition = 0;
			pChn->nPanEnvPosition = 0;
			pChn->nPitchEnvPosition = 0;
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
			pChn->dwFlags &= ~CHN_NOTEFADE;
			pChn->nFadeOutVol = 65536;
		}
		if ((!bPorta) || (!(_this->m_dwSongFlags & SONG_ITCOMPATMODE)) || (pChn->nRowInstr))
		{
			if ((!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) || (pChn->nRowInstr))
			{
				pChn->dwFlags &= ~CHN_NOTEFADE;
				pChn->nFadeOutVol = 65536;
			}
		}
	}
	pChn->dwFlags &= ~(CHN_EXTRALOUD|CHN_KEYOFF);
	// Enable Ramping
	if (!bPorta)
	{
		pChn->dwFlags &= ~CHN_FILTER;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
		pChn->nRetrigCount = 0;
		pChn->nTremorCount = 0;
		if (bResetEnv)
		{
			pChn->nVolSwing = pChn->nPanSwing = 0;
			if (penv)
			{
				if (!(penv->dwFlags & ENV_VOLCARRY)) pChn->nVolEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PANCARRY)) pChn->nPanEnvPosition = 0;
				if (!(penv->dwFlags & ENV_PITCHCARRY)) pChn->nPitchEnvPosition = 0;
				if (_this->m_nType & MOD_TYPE_IT)
				{
					// Volume Swing
					if (penv->nVolSwing)
					{
						int d = ((LONG)penv->nVolSwing*(LONG)((rand() & 0xFF) - 0x7F)) / 128;
						pChn->nVolSwing = (signed short)((d * pChn->nVolume + 1)/128);
					}
					// Pan Swing
					if (penv->nPanSwing)
					{
						int d = ((LONG)penv->nPanSwing*(LONG)((rand() & 0xFF) - 0x7F)) / 128;
						pChn->nPanSwing = (signed short)d;
					}
				}
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
		pChn->nLeftVol = pChn->nRightVol = 0;
		BOOL bFlt = (_this->m_dwSongFlags & SONG_MPTFILTERMODE) ? FALSE : TRUE;
		// Setup Initial Filter for this note
		if (penv)
		{
			if (penv->nIFR & 0x80) { pChn->nResonance = penv->nIFR & 0x7F; bFlt = TRUE; }
			if (penv->nIFC & 0x80) { pChn->nCutOff = penv->nIFC & 0x7F; bFlt = TRUE; }
		} else
		{
			pChn->nVolSwing = pChn->nPanSwing = 0;
		}
#ifndef NO_FILTER
		if ((pChn->nCutOff < 0x7F) && (bFlt)) CSoundFile_SetupChannelFilter(_this, pChn, TRUE, 256);
#endif // NO_FILTER
	}
}


UINT CSoundFile_GetNNAChannel(CSoundFile *_this, UINT nChn)
//---------------------------------------------
{
	const MODCHANNEL *pChn = &_this->Chn[nChn];
	// Check for empty channel
	const MODCHANNEL *pi = &_this->Chn[_this->m_nChannels];
	for (UINT i=_this->m_nChannels; i<MAX_CHANNELS; i++, pi++) if (!pi->nLength) return i;
	if (!pChn->nFadeOutVol) return 0;
	// All channels are used: check for lowest volume
	UINT result = 0;
	DWORD vol = 64*65536;	// 25%
	DWORD envpos = 0xFFFFFF;
	const MODCHANNEL *pj = &_this->Chn[_this->m_nChannels];
	for (UINT j=_this->m_nChannels; j<MAX_CHANNELS; j++, pj++)
	{
		if (!pj->nFadeOutVol) return j;
		DWORD v = pj->nVolume;
		if (pj->dwFlags & CHN_NOTEFADE)
			v = v * pj->nFadeOutVol;
		else
			v <<= 16;
		if (pj->dwFlags & CHN_LOOP) v >>= 1;
		if ((v < vol) || ((v == vol) && (pj->nVolEnvPosition > envpos)))
		{
			envpos = pj->nVolEnvPosition;
			vol = v;
			result = j;
		}
	}
	return result;
}


void CSoundFile_CheckNNA(CSoundFile *_this, UINT nChn, UINT instr, int note, BOOL bForceCut)
//------------------------------------------------------------------------
{
	MODCHANNEL *pChn = &_this->Chn[nChn];
	INSTRUMENTHEADER *penv = pChn->pHeader, *pHeader = 0;
	signed char *pSample;
	if (note > 0x80) note = 0;
	if (note < 1) return;
	// Always NNA cut - using
	if ((!(_this->m_nType & (MOD_TYPE_IT|MOD_TYPE_MT2))) || (!_this->m_nInstruments) || (bForceCut))
	{
		if ((_this->m_dwSongFlags & SONG_CPUVERYHIGH)
		 || (!pChn->nLength) || (pChn->dwFlags & CHN_MUTE)
		 || ((!pChn->nLeftVol) && (!pChn->nRightVol))) return;
		UINT n = CSoundFile_GetNNAChannel(_this, nChn);
		if (!n) return;
		MODCHANNEL *p = &_this->Chn[n];
		// Copy Channel
		SDL_memcpy(p, pChn, sizeof (*p));
		p->dwFlags &= ~(CHN_VIBRATO|CHN_TREMOLO|CHN_PANBRELLO|CHN_MUTE|CHN_PORTAMENTO);
		p->nMasterChn = nChn+1;
		p->nCommand = 0;
		// Cut the note
		p->nFadeOutVol = 0;
		p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
		// Stop this channel
		pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
		pChn->nROfs = pChn->nLOfs = 0;
		pChn->nLeftVol = pChn->nRightVol = 0;
		return;
	}
	if (instr >= MAX_INSTRUMENTS) instr = 0;
	pSample = pChn->pSample;
	pHeader = pChn->pHeader;
	if ((instr) && (note))
	{
		pHeader = _this->Headers[instr];
		if (pHeader)
		{
			UINT n = 0;
			if (note <= 0x80)
			{
				n = pHeader->Keyboard[note-1];
				note = pHeader->NoteMap[note-1];
				if ((n) && (n < MAX_SAMPLES)) pSample = _this->Ins[n].pSample;
			}
		} else pSample = NULL;
	}
	if (!penv) return;
	MODCHANNEL *p = pChn;
	for (UINT i=nChn; i<MAX_CHANNELS; p++, i++)
	if ((i >= _this->m_nChannels) || (p == pChn))
	{
		if (((p->nMasterChn == nChn+1) || (p == pChn)) && (p->pHeader))
		{
			BOOL bOk = FALSE;
			// Duplicate Check Type
			switch(p->pHeader->nDCT)
			{
			// Note
			case DCT_NOTE:
				if ((note) && (p->nNote == note) && (pHeader == p->pHeader)) bOk = TRUE;
				break;
			// Sample
			case DCT_SAMPLE:
				if ((pSample) && (pSample == p->pSample)) bOk = TRUE;
				break;
			// Instrument
			case DCT_INSTRUMENT:
				if (pHeader == p->pHeader) bOk = TRUE;
				break;
			}
			// Duplicate Note Action
			if (bOk)
			{
				switch(p->pHeader->nDNA)
				{
				// Cut
				case DNA_NOTECUT:
					CSoundFile_KeyOff(_this, i);
					p->nVolume = 0;
					break;
				// Note Off
				case DNA_NOTEOFF:
					CSoundFile_KeyOff(_this, i);
					break;
				// Note Fade
				case DNA_NOTEFADE:
					p->dwFlags |= CHN_NOTEFADE;
					break;
				}
				if (!p->nVolume)
				{
					p->nFadeOutVol = 0;
					p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
				}
			}
		}
	}
	if (pChn->dwFlags & CHN_MUTE) return;
	// New Note Action
	if ((pChn->nVolume) && (pChn->nLength))
	{
		UINT n = CSoundFile_GetNNAChannel(_this, nChn);
		if (n)
		{
			MODCHANNEL *p = &_this->Chn[n];
			// Copy Channel
			SDL_memcpy(p, pChn, sizeof (*p));
			p->dwFlags &= ~(CHN_VIBRATO|CHN_TREMOLO|CHN_PANBRELLO|CHN_MUTE|CHN_PORTAMENTO);
			p->nMasterChn = nChn+1;
			p->nCommand = 0;
			// Key Off the note
			switch(pChn->nNNA)
			{
			case NNA_NOTEOFF:	CSoundFile_KeyOff(_this, n); break;
			case NNA_NOTECUT:
				p->nFadeOutVol = 0;
			case NNA_NOTEFADE:	p->dwFlags |= CHN_NOTEFADE; break;
			}
			if (!p->nVolume)
			{
				p->nFadeOutVol = 0;
				p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
			}
			// Stop this channel
			pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
			pChn->nROfs = pChn->nLOfs = 0;
		}
	}
}


BOOL CSoundFile_ProcessEffects(CSoundFile *_this)
//-------------------------------
{
	MODCHANNEL *pChn = _this->Chn;
	int nBreakRow = -1, nPosJump = -1, nPatLoopRow = -1;

	for (UINT nChn=0; nChn<_this->m_nChannels; nChn++, pChn++)
	{
		UINT instr = pChn->nRowInstr;
		UINT volcmd = pChn->nRowVolCmd;
		UINT vol = pChn->nRowVolume;
		UINT cmd = pChn->nRowCommand;
		UINT param = pChn->nRowParam;
		int bPorta = ((cmd != CMD_TONEPORTAMENTO) && (cmd != CMD_TONEPORTAVOL) && (volcmd != VOLCMD_TONEPORTAMENTO)) ? FALSE : TRUE;
		UINT nStartTick = 0;

		pChn->dwFlags &= ~CHN_FASTVOLRAMP;
		// Process special effects (note delay, pattern delay, pattern loop)
		if ((cmd == CMD_MODCMDEX) || (cmd == CMD_S3MCMDEX))
		{
			if ((!param) && (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))) param = pChn->nOldCmdEx; else pChn->nOldCmdEx = param;
			// Note Delay ?
			if ((param & 0xF0) == 0xD0)
			{
				nStartTick = param & 0x0F;
			} else
			if (!_this->m_nTickCount)
			{
				// Pattern Loop ?
				if ((((param & 0xF0) == 0x60) && (cmd == CMD_MODCMDEX))
				 || (((param & 0xF0) == 0xB0) && (cmd == CMD_S3MCMDEX)))
				{
					int nloop = CSoundFile_PatternLoop(_this, pChn, param & 0x0F);
					if (nloop >= 0) nPatLoopRow = nloop;
				} else
				// Pattern Delay
				if ((param & 0xF0) == 0xE0)
				{
					_this->m_nPatternDelay = param & 0x0F;
				}
			}
		}

		// Handles note/instrument/volume changes
		if (_this->m_nTickCount == nStartTick) // can be delayed by a note delay effect
		{
			UINT note = pChn->nRowNote;
			if (instr) pChn->nNewIns = instr;
			// XM: Key-Off + Sample == Note Cut
			if (_this->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				if ((note == 0xFF) && ((!pChn->pHeader) || (!(pChn->pHeader->dwFlags & ENV_VOLUME))))
				{
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolume = 0;
					note = instr = 0;
				}
			}
			if ((!note) && (instr))
			{
				if (_this->m_nInstruments)
				{
					if (pChn->pInstrument) pChn->nVolume = pChn->pInstrument->nVolume;
					if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
					{
						pChn->dwFlags |= CHN_FASTVOLRAMP;
						pChn->nVolEnvPosition = 0;
						pChn->nPanEnvPosition = 0;
						pChn->nPitchEnvPosition = 0;
						pChn->nAutoVibDepth = 0;
						pChn->nAutoVibPos = 0;
						pChn->dwFlags &= ~CHN_NOTEFADE;
						pChn->nFadeOutVol = 65536;
					}
				} else
				{
					if (instr < MAX_SAMPLES) pChn->nVolume = _this->Ins[instr].nVolume;
				}
				if (!(_this->m_nType & MOD_TYPE_IT)) instr = 0;
			}
			// Invalid Instrument ?
			if (instr >= MAX_INSTRUMENTS) instr = 0;
			// Note Cut/Off => ignore instrument
			if (note >= 0xFE) instr = 0;
			if ((note) && (note <= 128)) pChn->nNewNote = note;
			// New Note Action ?
			if ((note) && (note <= 128) && (!bPorta))
			{
				CSoundFile_CheckNNA(_this, nChn, instr, note, FALSE);
			}
			// Instrument Change ?
			if (instr)
			{
				MODINSTRUMENT *psmp = pChn->pInstrument;
				CSoundFile_InstrumentChange(_this, pChn, instr, bPorta, TRUE, TRUE);
				pChn->nNewIns = 0;
				// Special IT case: portamento+note causes sample change -> ignore portamento
				if ((_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
				 && (psmp != pChn->pInstrument) && (note) && (note < 0x80))
				{
					bPorta = FALSE;
				}
			}
			// New Note ?
			if (note)
			{
				if ((!instr) && (pChn->nNewIns) && (note < 0x80))
				{
					CSoundFile_InstrumentChange(_this, pChn, pChn->nNewIns, bPorta, FALSE, (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? FALSE : TRUE);
					pChn->nNewIns = 0;
				}
				CSoundFile_NoteChange(_this, nChn, note, bPorta, (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? FALSE : TRUE);
				if ((bPorta) && (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) && (instr))
				{
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolEnvPosition = 0;
					pChn->nPanEnvPosition = 0;
					pChn->nPitchEnvPosition = 0;
					pChn->nAutoVibDepth = 0;
					pChn->nAutoVibPos = 0;
				}
			}
			// Tick-0 only volume commands
			if (volcmd == VOLCMD_VOLUME)
			{
				if (vol > 64) vol = 64;
				pChn->nVolume = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			} else
			if (volcmd == VOLCMD_PANNING)
			{
				if (vol > 64) vol = 64;
				pChn->nPan = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
		}

		// Volume Column Effect (except volume & panning)
		if ((volcmd > VOLCMD_PANNING) && (_this->m_nTickCount >= nStartTick))
		{
			if (volcmd == VOLCMD_TONEPORTAMENTO)
			{
				if (_this->m_nType & MOD_TYPE_IT)
					CSoundFile_TonePortamento(_this, pChn, ImpulseTrackerPortaVolCmd[vol & 0x0F]);
				else
					CSoundFile_TonePortamento(_this, pChn, vol * 16);
			} else
			{
				if (vol) pChn->nOldVolParam = vol; else vol = pChn->nOldVolParam;
				switch(volcmd)
				{
				case VOLCMD_VOLSLIDEUP:
					CSoundFile_VolumeSlide(_this, pChn, vol << 4);
					break;

				case VOLCMD_VOLSLIDEDOWN:
					CSoundFile_VolumeSlide(_this, pChn, vol);
					break;

				case VOLCMD_FINEVOLUP:
					if (_this->m_nType & MOD_TYPE_IT)
					{
						if (_this->m_nTickCount == nStartTick) CSoundFile_VolumeSlide(_this, pChn, (vol << 4) | 0x0F);
					} else
						CSoundFile_FineVolumeUp(_this, pChn, vol);
					break;

				case VOLCMD_FINEVOLDOWN:
					if (_this->m_nType & MOD_TYPE_IT)
					{
						if (_this->m_nTickCount == nStartTick) CSoundFile_VolumeSlide(_this, pChn, 0xF0 | vol);
					} else
						CSoundFile_FineVolumeDown(_this, pChn, vol);
					break;

				case VOLCMD_VIBRATOSPEED:
					CSoundFile_Vibrato(_this, pChn, vol << 4);
					break;

				case VOLCMD_VIBRATO:
					CSoundFile_Vibrato(_this, pChn, vol);
					break;

				case VOLCMD_PANSLIDELEFT:
					CSoundFile_PanningSlide(_this, pChn, vol);
					break;

				case VOLCMD_PANSLIDERIGHT:
					CSoundFile_PanningSlide(_this, pChn, vol << 4);
					break;

				case VOLCMD_PORTAUP:
					CSoundFile_PortamentoUp(_this, pChn, vol << 2);
					break;

				case VOLCMD_PORTADOWN:
					CSoundFile_PortamentoDown(_this, pChn, vol << 2);
					break;
				}
			}
		}

		// Effects
		if (cmd) switch (cmd)
		{
		// Set Volume
		case CMD_VOLUME:
			if (!_this->m_nTickCount)
			{
				pChn->nVolume = (param < 64) ? param*4 : 256;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			break;

		// Portamento Up
		case CMD_PORTAMENTOUP:
			if ((!param) && (_this->m_nType & MOD_TYPE_MOD)) break;
			CSoundFile_PortamentoUp(_this, pChn, param);
			break;

		// Portamento Down
		case CMD_PORTAMENTODOWN:
			if ((!param) && (_this->m_nType & MOD_TYPE_MOD)) break;
			CSoundFile_PortamentoDown(_this, pChn, param);
			break;

		// Volume Slide
		case CMD_VOLUMESLIDE:
			if ((param) || (_this->m_nType != MOD_TYPE_MOD)) CSoundFile_VolumeSlide(_this, pChn, param);
			break;

		// Tone-Portamento
		case CMD_TONEPORTAMENTO:
			CSoundFile_TonePortamento(_this, pChn, param);
			break;

		// Tone-Portamento + Volume Slide
		case CMD_TONEPORTAVOL:
			if ((param) || (_this->m_nType != MOD_TYPE_MOD)) CSoundFile_VolumeSlide(_this, pChn, param);
			CSoundFile_TonePortamento(_this, pChn, 0);
			break;

		// Vibrato
		case CMD_VIBRATO:
			CSoundFile_Vibrato(_this, pChn, param);
			break;

		// Vibrato + Volume Slide
		case CMD_VIBRATOVOL:
			if ((param) || (_this->m_nType != MOD_TYPE_MOD)) CSoundFile_VolumeSlide(_this, pChn, param);
			CSoundFile_Vibrato(_this, pChn, 0);
			break;

		// Set Speed
		case CMD_SPEED:
			if (!_this->m_nTickCount) CSoundFile_SetSpeed(_this, param);
			break;

		// Set Tempo
		case CMD_TEMPO:
			if (!_this->m_nTickCount)
			{
				if (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
				{
					if (param) pChn->nOldTempo = param; else param = pChn->nOldTempo;
				}
				CSoundFile_SetTempo(_this, param);
			}
			break;

		// Set Offset
		case CMD_OFFSET:
			if (_this->m_nTickCount) break;
			if (param) pChn->nOldOffset = param; else param = pChn->nOldOffset;
			param <<= 8;
			param |= (UINT)(pChn->nOldHiOffset) << 16;
			if ((pChn->nRowNote) && (pChn->nRowNote < 0x80))
			{
				if (bPorta)
					pChn->nPos = param;
				else
					pChn->nPos += param;
				if (pChn->nPos >= pChn->nLength)
				{
					if (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)))
					{
						pChn->nPos = pChn->nLoopStart;
						if ((_this->m_dwSongFlags & SONG_ITOLDEFFECTS) && (pChn->nLength > 4))
						{
							pChn->nPos = pChn->nLength - 2;
						}
					}
				}
			} else
			if ((param < pChn->nLength) && (_this->m_nType & (MOD_TYPE_MTM|MOD_TYPE_DMF)))
			{
				pChn->nPos = param;
			}
			break;

		// Arpeggio
		case CMD_ARPEGGIO:
			if ((_this->m_nTickCount) || (!pChn->nPeriod) || (!pChn->nNote)) break;
			if ((!param) && (!(_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)))) break;
			pChn->nCommand = CMD_ARPEGGIO;
			if (param) pChn->nArpeggio = param;
			break;

		// Retrig
		case CMD_RETRIG:
			if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				if (!(param & 0xF0)) param |= pChn->nRetrigParam & 0xF0;
				if (!(param & 0x0F)) param |= pChn->nRetrigParam & 0x0F;
				param |= 0x100; // increment retrig count on first row
			}
			if (param) pChn->nRetrigParam = (BYTE)(param & 0xFF); else param = pChn->nRetrigParam;
			CSoundFile_RetrigNote(_this, nChn, param);
			break;

		// Tremor
		case CMD_TREMOR:
			if (_this->m_nTickCount) break;
			pChn->nCommand = CMD_TREMOR;
			if (param) pChn->nTremorParam = param;
			break;

		// Set Global Volume
		case CMD_GLOBALVOLUME:
			if (_this->m_nTickCount) break;
			if (_this->m_nType != MOD_TYPE_IT) param <<= 1;
			if (param > 128) param = 128;
			_this->m_nGlobalVolume = param << 1;
			break;

		// Global Volume Slide
		case CMD_GLOBALVOLSLIDE:
			CSoundFile_GlobalVolSlide(_this, param);
			break;

		// Set 8-bit Panning
		case CMD_PANNING8:
			if (_this->m_nTickCount) break;
			if (!(_this->m_dwSongFlags & SONG_SURROUNDPAN)) pChn->dwFlags &= ~CHN_SURROUND;
			if (_this->m_nType & (MOD_TYPE_IT|MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				pChn->nPan = param;
			} else
			if (param <= 0x80)
			{
				pChn->nPan = param << 1;
			} else
			if (param == 0xA4)
			{
				pChn->dwFlags |= CHN_SURROUND;
				pChn->nPan = 0x80;
			}
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			break;

		// Panning Slide
		case CMD_PANNINGSLIDE:
			CSoundFile_PanningSlide(_this, pChn, param);
			break;

		// Tremolo
		case CMD_TREMOLO:
			CSoundFile_Tremolo(_this, pChn, param);
			break;

		// Fine Vibrato
		case CMD_FINEVIBRATO:
			CSoundFile_FineVibrato(_this, pChn, param);
			break;

		// MOD/XM Exx Extended Commands
		case CMD_MODCMDEX:
			CSoundFile_ExtendedMODCommands(_this, nChn, param);
			break;

		// S3M/IT Sxx Extended Commands
		case CMD_S3MCMDEX:
			CSoundFile_ExtendedS3MCommands(_this, nChn, param);
			break;

		// Key Off
		case CMD_KEYOFF:
			if (!_this->m_nTickCount) CSoundFile_KeyOff(_this, nChn);
			break;

		// Extra-fine porta up/down
		case CMD_XFINEPORTAUPDOWN:
			switch(param & 0xF0)
			{
			case 0x10: CSoundFile_ExtraFinePortamentoUp(_this, pChn, param & 0x0F); break;
			case 0x20: CSoundFile_ExtraFinePortamentoDown(_this, pChn, param & 0x0F); break;
			// Modplug XM Extensions
			case 0x50:
			case 0x60:
			case 0x70:
			case 0x90:
			case 0xA0: CSoundFile_ExtendedS3MCommands(_this, nChn, param); break;
			}
			break;

		// Set Channel Global Volume
		case CMD_CHANNELVOLUME:
			if (_this->m_nTickCount) break;
			if (param <= 64)
			{
				pChn->nGlobalVol = param;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			break;

		// Channel volume slide
		case CMD_CHANNELVOLSLIDE:
			CSoundFile_ChannelVolSlide(_this, pChn, param);
			break;

		// Panbrello (IT)
		case CMD_PANBRELLO:
			CSoundFile_Panbrello(_this, pChn, param);
			break;

		// Set Envelope Position
		case CMD_SETENVPOSITION:
			if (!_this->m_nTickCount)
			{
				pChn->nVolEnvPosition = param;
				pChn->nPanEnvPosition = param;
				pChn->nPitchEnvPosition = param;
				if (pChn->pHeader)
				{
					INSTRUMENTHEADER *penv = pChn->pHeader;
					if ((pChn->dwFlags & CHN_PANENV) && (penv->nPanEnv) && (param > penv->PanPoints[penv->nPanEnv-1]))
					{
						pChn->dwFlags &= ~CHN_PANENV;
					}
				}
			}
			break;

		// Position Jump
		case CMD_POSITIONJUMP:
			nPosJump = param;
			break;

		// Pattern Break
		case CMD_PATTERNBREAK:
			nBreakRow = param;
			break;

		// Midi Controller
		case CMD_MIDI:
			if (_this->m_nTickCount) break;
			if (param < 0x80){
				CSoundFile_ProcessMidiMacro(_this, nChn, &_this->m_MidiCfg.szMidiSFXExt[pChn->nActiveMacro << 5], param);
			} else {
				CSoundFile_ProcessMidiMacro(_this, nChn, &_this->m_MidiCfg.szMidiZXXExt[(param & 0x7F) << 5], 0);
			}
			break;
		}
	}

	// Navigation Effects
	if (!_this->m_nTickCount)
	{
		// Pattern Loop
		if (nPatLoopRow >= 0)
		{
			_this->m_nNextPattern = _this->m_nCurrentPattern;
			_this->m_nNextRow = nPatLoopRow;
			if (_this->m_nPatternDelay) _this->m_nNextRow++;
		} else
		// Pattern Break / Position Jump only if no loop running
		if ((nBreakRow >= 0) || (nPosJump >= 0))
		{
			BOOL bNoLoop = FALSE;
			if (nPosJump < 0) nPosJump = _this->m_nCurrentPattern+1;
			if (nBreakRow < 0) nBreakRow = 0;
			// Modplug Tracker & ModPlugin allow backward jumps
			if ((nPosJump < (int)_this->m_nCurrentPattern)
			 || ((nPosJump == (int)_this->m_nCurrentPattern) && (nBreakRow <= (int)_this->m_nRow)))
			{
				if (!CSoundFile_IsValidBackwardJump(_this, _this->m_nCurrentPattern, _this->m_nRow, nPosJump, nBreakRow))
				{
					if (_this->m_nRepeatCount)
					{
						if (_this->m_nRepeatCount > 0) _this->m_nRepeatCount--;
					} else
					{
					#ifdef MODPLUG_TRACKER
						if (_this->gdwSoundSetup & SNDMIX_NOBACKWARDJUMPS)
					#endif
						// Backward jump disabled
						bNoLoop = TRUE;
						//reset repeat count incase there are multiple loops.
						//(i.e. Unreal tracks)
						_this->m_nRepeatCount = _this->m_nInitialRepeatCount;
					}
				}
			}
			if (((!bNoLoop) && (nPosJump < MAX_ORDERS))
			 && ((nPosJump != (int)_this->m_nCurrentPattern) || (nBreakRow != (int)_this->m_nRow)))
			{
				if (nPosJump != (int)_this->m_nCurrentPattern)
				{
					for (UINT i=0; i<_this->m_nChannels; i++) _this->Chn[i].nPatternLoopCount = 0;
				}
				_this->m_nNextPattern = nPosJump;
				_this->m_nNextRow = (UINT)nBreakRow;
			}
		}
	}
	return TRUE;
}


////////////////////////////////////////////////////////////
// Channels effects

void CSoundFile_PortamentoUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	if (param) pChn->nOldPortaUpDown = param; else param = pChn->nOldPortaUpDown;
	if ((_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM)) && ((param & 0xF0) >= 0xE0))
	{
		if (param & 0x0F)
		{
			if ((param & 0xF0) == 0xF0)
			{
				CSoundFile_FinePortamentoUp(_this, pChn, param & 0x0F);
			} else
			if ((param & 0xF0) == 0xE0)
			{
				CSoundFile_ExtraFinePortamentoUp(_this, pChn, param & 0x0F);
			}
		}
		return;
	}
	// Regular Slide
	if (!(_this->m_dwSongFlags & SONG_FIRSTTICK) || (_this->m_nMusicSpeed == 1))  //rewbs.PortaA01fix
	{
		CSoundFile_DoFreqSlide(_this, pChn, -(int)(param * 4));
	}
}


void CSoundFile_PortamentoDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nOldPortaUpDown = param; else param = pChn->nOldPortaUpDown;
	if ((_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM)) && ((param & 0xF0) >= 0xE0))
	{
		if (param & 0x0F)
		{
			if ((param & 0xF0) == 0xF0)
			{
				CSoundFile_FinePortamentoDown(_this, pChn, param & 0x0F);
			} else
			if ((param & 0xF0) == 0xE0)
			{
				CSoundFile_ExtraFinePortamentoDown(_this, pChn, param & 0x0F);
			}
		}
		return;
	}
	if (!(_this->m_dwSongFlags & SONG_FIRSTTICK) || (_this->m_nMusicSpeed == 1)) { //rewbs.PortaA01fix
		CSoundFile_DoFreqSlide(_this, pChn, (int)(param << 2));
	}
}


void CSoundFile_FinePortamentoUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//-------------------------------------------------------------
{
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (_this->m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideDownTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod -= (int)(param * 4);
			}
			if (pChn->nPeriod < 1) pChn->nPeriod = 1;
		}
	}
}


void CSoundFile_FinePortamentoDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------------
{
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (_this->m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideUpTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod += (int)(param * 4);
			}
			if (pChn->nPeriod > 0xFFFF) pChn->nPeriod = 0xFFFF;
		}
	}
}


void CSoundFile_ExtraFinePortamentoUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------------
{
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (_this->m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, FineLinearSlideDownTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod -= (int)(param);
			}
			if (pChn->nPeriod < 1) pChn->nPeriod = 1;
		}
	}
}


void CSoundFile_ExtraFinePortamentoDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//--------------------------------------------------------------------
{
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (_this->m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, FineLinearSlideUpTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod += (int)(param);
			}
			if (pChn->nPeriod > 0xFFFF) pChn->nPeriod = 0xFFFF;
		}
	}
}


// Portamento Slide
void CSoundFile_TonePortamento(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nPortamentoSlide = param * 4;
	pChn->dwFlags |= CHN_PORTAMENTO;
	if ((pChn->nPeriod) && (pChn->nPortamentoDest) && (!(_this->m_dwSongFlags & SONG_FIRSTTICK)))
	{
		if (pChn->nPeriod < pChn->nPortamentoDest)
		{
			LONG delta = (int)pChn->nPortamentoSlide;
			if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255) n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536) - pChn->nPeriod;
				if (delta < 1) delta = 1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod > pChn->nPortamentoDest) pChn->nPeriod = pChn->nPortamentoDest;
		} else
		if (pChn->nPeriod > pChn->nPortamentoDest)
		{
			LONG delta = - (int)pChn->nPortamentoSlide;
			if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255) n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536) - pChn->nPeriod;
				if (delta > -1) delta = -1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod < pChn->nPortamentoDest) pChn->nPeriod = pChn->nPortamentoDest;
		}
	}
}


void CSoundFile_Vibrato(CSoundFile *_this, MODCHANNEL *p, UINT param)
//-------------------------------------------------
{
	if (param & 0x0F) p->nVibratoDepth = (param & 0x0F) * 4;
	if (param & 0xF0) p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CSoundFile_FineVibrato(CSoundFile *_this, MODCHANNEL *p, UINT param)
//-----------------------------------------------------
{
	if (param & 0x0F) p->nVibratoDepth = param & 0x0F;
	if (param & 0xF0) p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CSoundFile_Panbrello(CSoundFile *_this, MODCHANNEL *p, UINT param)
//---------------------------------------------------
{
	if (param & 0x0F) p->nPanbrelloDepth = param & 0x0F;
	if (param & 0xF0) p->nPanbrelloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_PANBRELLO;
}


void CSoundFile_VolumeSlide(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//--------------------------------------------------------
{
	if (param) pChn->nOldVolumeSlide = param; else param = pChn->nOldVolumeSlide;
	LONG newvolume = pChn->nVolume;
	if (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM|MOD_TYPE_AMF))
	{
		if ((param & 0x0F) == 0x0F)
		{
			if (param & 0xF0)
			{
				CSoundFile_FineVolumeUp(_this, pChn, (param >> 4));
				return;
			} else
			{
				if ((_this->m_dwSongFlags & SONG_FIRSTTICK) && (!(_this->m_dwSongFlags & SONG_FASTVOLSLIDES)))
				{
					newvolume -= 0x0F * 4;
				}
			}
		} else
		if ((param & 0xF0) == 0xF0)
		{
			if (param & 0x0F)
			{
				CSoundFile_FineVolumeDown(_this, pChn, (param & 0x0F));
				return;
			} else
			{
				if ((_this->m_dwSongFlags & SONG_FIRSTTICK) && (!(_this->m_dwSongFlags & SONG_FASTVOLSLIDES)))
				{
					newvolume += 0x0F * 4;
				}
			}
		}
	}
	if ((!(_this->m_dwSongFlags & SONG_FIRSTTICK)) || (_this->m_dwSongFlags & SONG_FASTVOLSLIDES))
	{
		if (param & 0x0F) newvolume -= (int)((param & 0x0F) * 4);
		else newvolume += (int)((param & 0xF0) >> 2);
		if (_this->m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
	if (newvolume < 0) newvolume = 0;
	if (newvolume > 256) newvolume = 256;
	pChn->nVolume = newvolume;
}


void CSoundFile_PanningSlide(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	LONG nPanSlide = 0;
	if (param) pChn->nOldPanSlide = param; else param = pChn->nOldPanSlide;
	if (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_STM))
	{
		if (((param & 0x0F) == 0x0F) && (param & 0xF0))
		{
			if (_this->m_dwSongFlags & SONG_FIRSTTICK)
			{
				param = (param & 0xF0) >> 2;
				nPanSlide = - (int)param;
			}
		} else
		if (((param & 0xF0) == 0xF0) && (param & 0x0F))
		{
			if (_this->m_dwSongFlags & SONG_FIRSTTICK)
			{
				nPanSlide = (param & 0x0F) << 2;
			}
		} else
		{
			if (!(_this->m_dwSongFlags & SONG_FIRSTTICK))
			{
				if (param & 0x0F) nPanSlide = (int)((param & 0x0F) << 2);
				else nPanSlide = -(int)((param & 0xF0) >> 2);
			}
		}
	} else
	{
		if (!(_this->m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0x0F) nPanSlide = -(int)((param & 0x0F) << 2);
			else nPanSlide = (int)((param & 0xF0) >> 2);
		}
	}
	if (nPanSlide)
	{
		nPanSlide += pChn->nPan;
		if (nPanSlide < 0) nPanSlide = 0;
		if (nPanSlide > 256) nPanSlide = 256;
		pChn->nPan = nPanSlide;
	}
}


void CSoundFile_FineVolumeUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	if (param) pChn->nOldFineVolUpDown = param; else param = pChn->nOldFineVolUpDown;
	if (_this->m_dwSongFlags & SONG_FIRSTTICK)
	{
		pChn->nVolume += param * 4;
		if (pChn->nVolume > 256) pChn->nVolume = 256;
		if (_this->m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile_FineVolumeDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nOldFineVolUpDown = param; else param = pChn->nOldFineVolUpDown;
	if (_this->m_dwSongFlags & SONG_FIRSTTICK)
	{
		pChn->nVolume -= param * 4;
		if (pChn->nVolume < 0) pChn->nVolume = 0;
		if (_this->m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile_Tremolo(CSoundFile *_this, MODCHANNEL *p, UINT param)
//-------------------------------------------------
{
	if (param & 0x0F) p->nTremoloDepth = (param & 0x0F) << 2;
	if (param & 0xF0) p->nTremoloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_TREMOLO;
}


void CSoundFile_ChannelVolSlide(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------
{
	LONG nChnSlide = 0;
	if (param) pChn->nOldChnVolSlide = param; else param = pChn->nOldChnVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0))
	{
		if (_this->m_dwSongFlags & SONG_FIRSTTICK) nChnSlide = param >> 4;
	} else
	if (((param & 0xF0) == 0xF0) && (param & 0x0F))
	{
		if (_this->m_dwSongFlags & SONG_FIRSTTICK) nChnSlide = - (int)(param & 0x0F);
	} else
	{
		if (!(_this->m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0x0F) nChnSlide = -(int)(param & 0x0F);
			else nChnSlide = (int)((param & 0xF0) >> 4);
		}
	}
	if (nChnSlide)
	{
		nChnSlide += pChn->nGlobalVol;
		if (nChnSlide < 0) nChnSlide = 0;
		if (nChnSlide > 64) nChnSlide = 64;
		pChn->nGlobalVol = nChnSlide;
	}
}


void CSoundFile_ExtendedMODCommands(CSoundFile *_this, UINT nChn, UINT param)
//---------------------------------------------------------
{
	MODCHANNEL *pChn = &_this->Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch(command)
	{
	// E0x: Set Filter
	// E1x: Fine Portamento Up
	case 0x10:	if ((param) || (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) CSoundFile_FinePortamentoUp(_this, pChn, param); break;
	// E2x: Fine Portamento Down
	case 0x20:	if ((param) || (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) CSoundFile_FinePortamentoDown(_this, pChn, param); break;
	// E3x: Set Glissando Control
	case 0x30:	pChn->dwFlags &= ~CHN_GLISSANDO; if (param) pChn->dwFlags |= CHN_GLISSANDO; break;
	// E4x: Set Vibrato WaveForm
	case 0x40:	pChn->nVibratoType = param & 0x07; break;
	// E5x: Set FineTune
	case 0x50:	if (_this->m_nTickCount) break;
				pChn->nC4Speed = S3MFineTuneTable[param];
				if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
					pChn->nFineTune = param*2;
				else
					pChn->nFineTune = MOD2XMFineTune(param);
				if (pChn->nPeriod) pChn->nPeriod = CSoundFile_GetPeriodFromNote(_this, pChn->nNote, pChn->nFineTune, pChn->nC4Speed);
				break;
	// E6x: Pattern Loop
	// E7x: Set Tremolo WaveForm
	case 0x70:	pChn->nTremoloType = param & 0x07; break;
	// E8x: Set 4-bit Panning
	case 0x80:	if (!_this->m_nTickCount) { pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP; } break;
	// E9x: Retrig
	case 0x90:	CSoundFile_RetrigNote(_this, nChn, param); break;
	// EAx: Fine Volume Up
	case 0xA0:	if ((param) || (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) CSoundFile_FineVolumeUp(_this, pChn, param); break;
	// EBx: Fine Volume Down
	case 0xB0:	if ((param) || (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) CSoundFile_FineVolumeDown(_this, pChn, param); break;
	// ECx: Note Cut
	case 0xC0:	CSoundFile_NoteCut(_this, nChn, param); break;
	// EDx: Note Delay
	// EEx: Pattern Delay
	// EFx: MOD: Invert Loop, XM: Set Active Midi Macro
	case 0xF0:	pChn->nActiveMacro = param;	break;
	}
}


void CSoundFile_ExtendedS3MCommands(CSoundFile *_this, UINT nChn, UINT param)
//---------------------------------------------------------
{
	MODCHANNEL *pChn = &_this->Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch(command)
	{
	// S0x: Set Filter
	// S1x: Set Glissando Control
	case 0x10:	pChn->dwFlags &= ~CHN_GLISSANDO; if (param) pChn->dwFlags |= CHN_GLISSANDO; break;
	// S2x: Set FineTune
	case 0x20:	if (_this->m_nTickCount) break;
				pChn->nC4Speed = S3MFineTuneTable[param & 0x0F];
				pChn->nFineTune = MOD2XMFineTune(param);
				if (pChn->nPeriod) pChn->nPeriod = CSoundFile_GetPeriodFromNote(_this, pChn->nNote, pChn->nFineTune, pChn->nC4Speed);
				break;
	// S3x: Set Vibrato WaveForm
	case 0x30:	pChn->nVibratoType = param & 0x07; break;
	// S4x: Set Tremolo WaveForm
	case 0x40:	pChn->nTremoloType = param & 0x07; break;
	// S5x: Set Panbrello WaveForm
	case 0x50:	pChn->nPanbrelloType = param & 0x07; break;
	// S6x: Pattern Delay for x frames
	case 0x60:	_this->m_nFrameDelay = param; break;
	// S7x: Envelope Control
	case 0x70:	if (_this->m_nTickCount) break;
				switch(param)
				{
				case 0:
				case 1:
				case 2:
					{
						MODCHANNEL *bkp = &_this->Chn[_this->m_nChannels];
						for (UINT i=_this->m_nChannels; i<MAX_CHANNELS; i++, bkp++)
						{
							if (bkp->nMasterChn == nChn+1)
							{
								if (param == 1) CSoundFile_KeyOff(_this, i); else
								if (param == 2) bkp->dwFlags |= CHN_NOTEFADE; else
									{ bkp->dwFlags |= CHN_NOTEFADE; bkp->nFadeOutVol = 0; }
							}
						}
					}
					break;
				case 3:		pChn->nNNA = NNA_NOTECUT; break;
				case 4:		pChn->nNNA = NNA_CONTINUE; break;
				case 5:		pChn->nNNA = NNA_NOTEOFF; break;
				case 6:		pChn->nNNA = NNA_NOTEFADE; break;
				case 7:		pChn->dwFlags &= ~CHN_VOLENV; break;
				case 8:		pChn->dwFlags |= CHN_VOLENV; break;
				case 9:		pChn->dwFlags &= ~CHN_PANENV; break;
				case 10:	pChn->dwFlags |= CHN_PANENV; break;
				case 11:	pChn->dwFlags &= ~CHN_PITCHENV; break;
				case 12:	pChn->dwFlags |= CHN_PITCHENV; break;
				}
				break;
	// S8x: Set 4-bit Panning
	case 0x80:	if (!_this->m_nTickCount) { pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP; } break;
	// S9x: Set Surround
	case 0x90:	CSoundFile_ExtendedChannelEffect(_this, pChn, param & 0x0F); break;
	// SAx: Set 64k Offset
	case 0xA0:	if (!_this->m_nTickCount)
				{
					pChn->nOldHiOffset = param;
					if ((pChn->nRowNote) && (pChn->nRowNote < 0x80))
					{
						DWORD pos = param << 16;
						if (pos < pChn->nLength) pChn->nPos = pos;
					}
				}
				break;
	// SBx: Pattern Loop
	// SCx: Note Cut
	case 0xC0:	CSoundFile_NoteCut(_this, nChn, param); break;
	// SDx: Note Delay
	// case 0xD0:	break;
	// SEx: Pattern Delay for x rows
	// SFx: S3M: Funk Repeat, IT: Set Active Midi Macro
	case 0xF0:	pChn->nActiveMacro = param; break;
	}
}


void CSoundFile_ExtendedChannelEffect(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------------
{
	// S9x and X9x commands (S3M/XM/IT only)
	if (_this->m_nTickCount) return;
	switch(param & 0x0F)
	{
	// S90: Surround Off
	case 0x00:	pChn->dwFlags &= ~CHN_SURROUND;	break;
	// S91: Surround On
	case 0x01:	pChn->dwFlags |= CHN_SURROUND; pChn->nPan = 128; break;
	////////////////////////////////////////////////////////////
	// Modplug Extensions
	// S98: Reverb Off
	case 0x08:
		pChn->dwFlags &= ~CHN_REVERB;
		pChn->dwFlags |= CHN_NOREVERB;
		break;
	// S99: Reverb On
	case 0x09:
		pChn->dwFlags &= ~CHN_NOREVERB;
		pChn->dwFlags |= CHN_REVERB;
		break;
	// S9A: 2-Channels surround mode
	case 0x0A:
		_this->m_dwSongFlags &= ~SONG_SURROUNDPAN;
		break;
	// S9B: 4-Channels surround mode
	case 0x0B:
		_this->m_dwSongFlags |= SONG_SURROUNDPAN;
		break;
	// S9C: IT Filter Mode
	case 0x0C:
		_this->m_dwSongFlags &= ~SONG_MPTFILTERMODE;
		break;
	// S9D: MPT Filter Mode
	case 0x0D:
		_this->m_dwSongFlags |= SONG_MPTFILTERMODE;
		break;
	// S9E: Go forward
	case 0x0E:
		pChn->dwFlags &= ~(CHN_PINGPONGFLAG);
		break;
	// S9F: Go backward (set position at the end for non-looping samples)
	case 0x0F:
		if ((!(pChn->dwFlags & CHN_LOOP)) && (!pChn->nPos) && (pChn->nLength))
		{
			pChn->nPos = pChn->nLength - 1;
			pChn->nPosLo = 0xFFFF;
		}
		pChn->dwFlags |= CHN_PINGPONGFLAG;
		break;
	}
}


void CSoundFile_ProcessMidiMacro(CSoundFile *_this, UINT nChn, LPCSTR pszMidiMacro, UINT param)
//---------------------------------------------------------------------------
{
	MODCHANNEL *pChn = &_this->Chn[nChn];
	DWORD dwMacro = (*((LPDWORD)pszMidiMacro)) & 0x7F5F7F5F;
	// Not Internal Device ?
	if (dwMacro != 0x30463046 && dwMacro != 0x31463046)
	{
		UINT pos = 0, nNib = 0, nBytes = 0;
		DWORD dwMidiCode = 0, dwByteCode = 0;
		while (pos+6 <= 32)
		{
			CHAR cData = pszMidiMacro[pos++];
			if (!cData) break;
			if ((cData >= '0') && (cData <= '9')) { dwByteCode = (dwByteCode<<4) | (cData-'0'); nNib++; } else
			if ((cData >= 'A') && (cData <= 'F')) { dwByteCode = (dwByteCode<<4) | (cData-'A'+10); nNib++; } else
			if ((cData >= 'a') && (cData <= 'f')) { dwByteCode = (dwByteCode<<4) | (cData-'a'+10); nNib++; } else
			if ((cData == 'z') || (cData == 'Z')) { dwByteCode = param & 0x7f; nNib = 2; } else
			if ((cData == 'x') || (cData == 'X')) { dwByteCode = param & 0x70; nNib = 2; } else
			if ((cData == 'y') || (cData == 'Y')) { dwByteCode = (param & 0x0f)<<3; nNib = 2; } else
			if (nNib >= 2)
			{
				nNib = 0;
				dwMidiCode |= dwByteCode << (nBytes*8);
				dwByteCode = 0;
				nBytes++;
				if (nBytes >= 3)
				{
					nBytes = 0;
					dwMidiCode = 0;
				}
			}

		}
		return;
	}
	// Internal device
	pszMidiMacro += 4;
	// Filter ?
	if (pszMidiMacro[0] == '0')
	{
		CHAR cData1 = pszMidiMacro[2];
		DWORD dwParam = 0;
		if ((cData1 == 'z') || (cData1 == 'Z'))
		{
			dwParam = param;
		} else
		{
			CHAR cData2 = pszMidiMacro[3];
			if ((cData1 >= '0') && (cData1 <= '9')) dwParam += (cData1 - '0') << 4; else
			if ((cData1 >= 'A') && (cData1 <= 'F')) dwParam += (cData1 - 'A' + 0x0A) << 4;
			if ((cData2 >= '0') && (cData2 <= '9')) dwParam += (cData2 - '0'); else
			if ((cData2 >= 'A') && (cData2 <= 'F')) dwParam += (cData2 - 'A' + 0x0A);
		}
		switch(pszMidiMacro[1])
		{
		// F0.F0.00.xx: Set CutOff
		case '0':
			{
				int oldcutoff = pChn->nCutOff;
				if (dwParam < 0x80) pChn->nCutOff = (BYTE)dwParam;
#ifndef NO_FILTER
				oldcutoff -= pChn->nCutOff;

				if (oldcutoff < 0) oldcutoff = -oldcutoff;
				if ((pChn->nVolume > 0) || (oldcutoff < 0x10)
				 || (!(pChn->dwFlags & CHN_FILTER)) || (!(pChn->nLeftVol|pChn->nRightVol)))
					CSoundFile_SetupChannelFilter(_this, pChn, (pChn->dwFlags & CHN_FILTER) ? FALSE : TRUE, 256);
#endif // NO_FILTER
			}
			break;

		// F0.F0.01.xx: Set Resonance
		case '1':
			if (dwParam < 0x80) pChn->nResonance = (BYTE)dwParam;
#ifndef NO_FILTER
			CSoundFile_SetupChannelFilter(_this, pChn, (pChn->dwFlags & CHN_FILTER) ? FALSE : TRUE, 256);
#endif // NO_FILTER

			break;
		}

	}
}


void CSoundFile_RetrigNote(CSoundFile *_this, UINT nChn, UINT param)
//------------------------------------------------
{
	// Retrig: bit 8 is set if it's the new XM retrig
	MODCHANNEL *pChn = &_this->Chn[nChn];
	UINT nRetrigSpeed = param & 0x0F;
	UINT nRetrigCount = pChn->nRetrigCount;
	BOOL bDoRetrig = FALSE;

	if (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))
	{
		if (!nRetrigSpeed) nRetrigSpeed = 1;
		if ((nRetrigCount) && (!(nRetrigCount % nRetrigSpeed))) bDoRetrig = TRUE;
		nRetrigCount++;
	} else
	{
		UINT realspeed = nRetrigSpeed;
		if ((param & 0x100) && (pChn->nRowVolCmd == VOLCMD_VOLUME) && (pChn->nRowParam & 0xF0)) realspeed++;
		if ((_this->m_nTickCount) || (param & 0x100))
		{
			if (!realspeed) realspeed = 1;
			if ((!(param & 0x100)) && (_this->m_nMusicSpeed) && (!(_this->m_nTickCount % realspeed))) bDoRetrig = TRUE;
			nRetrigCount++;
		} else if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) nRetrigCount = 0;
		if (nRetrigCount >= realspeed)
		{
			if ((_this->m_nTickCount) || ((param & 0x100) && (!pChn->nRowNote))) bDoRetrig = TRUE;
		}
	}
	if (bDoRetrig)
	{
		UINT dv = (param >> 4) & 0x0F;
		if (dv)
		{
			int vol = pChn->nVolume;
			if (retrigTable1[dv])
				vol = (vol * retrigTable1[dv]) >> 4;
			else
				vol += ((int)retrigTable2[dv]) << 2;
			if (vol < 0) vol = 0;
			if (vol > 256) vol = 256;
			pChn->nVolume = vol;
			pChn->dwFlags |= CHN_FASTVOLRAMP;
		}
		UINT nNote = pChn->nNewNote;
		LONG nOldPeriod = pChn->nPeriod;
		if ((nNote) && (nNote <= NOTE_MAX) && (pChn->nLength)) CSoundFile_CheckNNA(_this, nChn, 0, nNote, TRUE);
		BOOL bResetEnv = FALSE;
		if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
		{
			if ((pChn->nRowInstr) && (param < 0x100)) { CSoundFile_InstrumentChange(_this, pChn, pChn->nRowInstr, FALSE, FALSE, TRUE); bResetEnv = TRUE; }
			if (param < 0x100) bResetEnv = TRUE;
		}
		CSoundFile_NoteChange(_this, nChn, nNote, FALSE, bResetEnv);
		if ((_this->m_nType & MOD_TYPE_IT) && (!pChn->nRowNote) && (nOldPeriod)) pChn->nPeriod = nOldPeriod;
		if (!(_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT))) nRetrigCount = 0;
	}
	pChn->nRetrigCount = (BYTE)nRetrigCount;
}


void CSoundFile_DoFreqSlide(CSoundFile *_this, MODCHANNEL *pChn, LONG nFreqSlide)
//-------------------------------------------------------------
{
	// IT Linear slides
	if (!pChn->nPeriod) return;
	if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (!(_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
	{
		if (nFreqSlide < 0)
		{
			UINT n = (- nFreqSlide) >> 2;
			if (n > 255) n = 255;
			pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536);
		} else
		{
			UINT n = (nFreqSlide) >> 2;

			if (n > 255) n = 255;
			pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536);
		}
	} else
	{
		pChn->nPeriod += nFreqSlide;
	}
	if (pChn->nPeriod < 1)
	{
		pChn->nPeriod = 1;
		if (_this->m_nType & MOD_TYPE_IT)
		{
			pChn->dwFlags |= CHN_NOTEFADE;
			pChn->nFadeOutVol = 0;
		}
	}
}


void CSoundFile_NoteCut(CSoundFile *_this, UINT nChn, UINT nTick)
//---------------------------------------------
{
	if (_this->m_nTickCount == nTick)
	{
		MODCHANNEL *pChn = &_this->Chn[nChn];
		// if (_this->m_nInstruments) CSoundFile_KeyOff(_this, pChn); ?
		pChn->nVolume = 0;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile_KeyOff(CSoundFile *_this, UINT nChn)
//--------------------------------
{
	MODCHANNEL *pChn = &_this->Chn[nChn];
	BOOL bKeyOn = (pChn->dwFlags & CHN_KEYOFF) ? FALSE : TRUE;
	pChn->dwFlags |= CHN_KEYOFF;
	//if ((!pChn->pHeader) || (!(pChn->dwFlags & CHN_VOLENV)))
	if ((pChn->pHeader) && (!(pChn->dwFlags & CHN_VOLENV)))
	{
		pChn->dwFlags |= CHN_NOTEFADE;
	}
	if (!pChn->nLength) return;
	if ((pChn->dwFlags & CHN_SUSTAINLOOP) && (pChn->pInstrument) && (bKeyOn))
	{
		MODINSTRUMENT *psmp = pChn->pInstrument;
		if (psmp->uFlags & CHN_LOOP)
		{
			if (psmp->uFlags & CHN_PINGPONGLOOP)
				pChn->dwFlags |= CHN_PINGPONGLOOP;
			else
				pChn->dwFlags &= ~(CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
			pChn->dwFlags |= CHN_LOOP;
			pChn->nLength = psmp->nLength;
			pChn->nLoopStart = psmp->nLoopStart;
			pChn->nLoopEnd = psmp->nLoopEnd;
			if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
		} else
		{
			pChn->dwFlags &= ~(CHN_LOOP|CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
			pChn->nLength = psmp->nLength;
		}
	}
	if (pChn->pHeader)
	{
		INSTRUMENTHEADER *penv = pChn->pHeader;
		if (((penv->dwFlags & ENV_VOLLOOP) || (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) && (penv->nFadeOut))
			pChn->dwFlags |= CHN_NOTEFADE;
	}
}


//////////////////////////////////////////////////////////
// CSoundFile: Global Effects


void CSoundFile_SetSpeed(CSoundFile *_this, UINT param)
//-----------------------------------
{
	UINT max = (_this->m_nType == MOD_TYPE_IT) ? 256 : 128;
	// Modplug Tracker and Mod-Plugin don't do this check
#ifndef MODPLUG_TRACKER
	// Big Hack!!!
	if ((!param) || (param >= 0x80) || ((_this->m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM|MOD_TYPE_MT2)) && (param >= 0x1E)))
	{
		if (CSoundFile_IsSongFinished(_this, _this->m_nCurrentPattern, _this->m_nRow+1))
		{
			CSoundFile_GlobalFadeSong(_this, 1000);
		}
	}
#endif // MODPLUG_TRACKER
	if ((_this->m_nType & MOD_TYPE_S3M) && (param > 0x80)) param -= 0x80;
	if ((param) && (param <= max)) _this->m_nMusicSpeed = param;
}


void CSoundFile_SetTempo(CSoundFile *_this, UINT param)
//-----------------------------------
{
	if (param < 0x20)
	{
		// Tempo Slide
		if ((param & 0xF0) == 0x10)
		{
			_this->m_nMusicTempo += (param & 0x0F) * 2;
			if (_this->m_nMusicTempo > 255) _this->m_nMusicTempo = 255;
		} else
		{
			_this->m_nMusicTempo -= (param & 0x0F) * 2;
			if ((LONG)_this->m_nMusicTempo < 32) _this->m_nMusicTempo = 32;
		}
	} else
	{
		_this->m_nMusicTempo = param;
	}
}


int CSoundFile_PatternLoop(CSoundFile *_this, MODCHANNEL *pChn, UINT param)
//-------------------------------------------------------
{
	if (param)
	{
		if (pChn->nPatternLoopCount)
		{
			pChn->nPatternLoopCount--;
			if (!pChn->nPatternLoopCount) return -1;
		} else
		{
			MODCHANNEL *p = _this->Chn;
			for (UINT i=0; i<_this->m_nChannels; i++, p++) if (p != pChn)
			{
				// Loop already done
				if (p->nPatternLoopCount) return -1;
			}
			pChn->nPatternLoopCount = param;
		}
		return pChn->nPatternLoop;
	} else
	{
		pChn->nPatternLoop = _this->m_nRow;
	}
	return -1;
}


void CSoundFile_GlobalVolSlide(CSoundFile *_this, UINT param)
//-----------------------------------------
{
	LONG nGlbSlide = 0;
	if (param) _this->m_nOldGlbVolSlide = param; else param = _this->m_nOldGlbVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0))
	{
		if (_this->m_dwSongFlags & SONG_FIRSTTICK) nGlbSlide = (param >> 4) * 2;
	} else
	if (((param & 0xF0) == 0xF0) && (param & 0x0F))
	{
		if (_this->m_dwSongFlags & SONG_FIRSTTICK) nGlbSlide = - (int)((param & 0x0F) * 2);
	} else
	{
		if (!(_this->m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0xF0) nGlbSlide = (int)((param & 0xF0) >> 4) * 2;
			else nGlbSlide = -(int)((param & 0x0F) * 2);
		}
	}
	if (nGlbSlide)
	{
		if (_this->m_nType != MOD_TYPE_IT) nGlbSlide *= 2;
		nGlbSlide += _this->m_nGlobalVolume;
		if (nGlbSlide < 0) nGlbSlide = 0;
		if (nGlbSlide > 256) nGlbSlide = 256;
		_this->m_nGlobalVolume = nGlbSlide;
	}
}


DWORD CSoundFile_IsSongFinished(CSoundFile *_this, UINT nStartOrder, UINT nStartRow)
//----------------------------------------------------------------------
{
	UINT nOrd;

	for (nOrd=nStartOrder; nOrd<MAX_ORDERS; nOrd++)
	{
		UINT nPat = _this->Order[nOrd];
		if (nPat != 0xFE)
		{
			MODCOMMAND *p;

			if (nPat >= MAX_PATTERNS) break;
			p = _this->Patterns[nPat];
			if (p)
			{
				UINT len = _this->PatternSize[nPat] * _this->m_nChannels;
				UINT pos = (nOrd == nStartOrder) ? nStartRow : 0;
				pos *= _this->m_nChannels;
				while (pos < len)
				{
					UINT cmd;
					if ((p[pos].note) || (p[pos].volcmd)) return 0;
					cmd = p[pos].command;
					if (cmd == CMD_MODCMDEX)
					{
						UINT cmdex = p[pos].param & 0xF0;
						if ((!cmdex) || (cmdex == 0x60) || (cmdex == 0xE0) || (cmdex == 0xF0)) cmd = 0;
					}
					if ((cmd) && (cmd != CMD_SPEED) && (cmd != CMD_TEMPO)) return 0;
					pos++;
				}
			}
		}
	}
	return (nOrd < MAX_ORDERS) ? nOrd : MAX_ORDERS-1;
}


BOOL CSoundFile_IsValidBackwardJump(CSoundFile *_this, UINT nStartOrder, UINT nStartRow, UINT nJumpOrder, UINT nJumpRow)
//----------------------------------------------------------------------------------------------------------
{
	while ((nJumpOrder < MAX_PATTERNS) && (_this->Order[nJumpOrder] == 0xFE)) nJumpOrder++;
	if ((nStartOrder >= MAX_PATTERNS) || (nJumpOrder >= MAX_PATTERNS)) return FALSE;
	// Treat only case with jumps in the same pattern
	if (nJumpOrder > nStartOrder) return TRUE;
	if ((nJumpOrder < nStartOrder) || (nJumpRow >= _this->PatternSize[nStartOrder])
	 || (!_this->Patterns[nStartOrder]) || (nStartRow >= 256) || (nJumpRow >= 256)) return FALSE;
	// See if the pattern is being played backward
	BYTE row_hist[256];
	SDL_memset(row_hist, 0, sizeof(row_hist));
	UINT nRows = _this->PatternSize[nStartOrder], row = nJumpRow;
	if (nRows > 256) nRows = 256;
	row_hist[nStartRow] = TRUE;
	while ((row < 256) && (!row_hist[row]))
	{
		if (row >= nRows) return TRUE;
		row_hist[row] = TRUE;
		MODCOMMAND *p = _this->Patterns[nStartOrder] + row * _this->m_nChannels;
		row++;
		int breakrow = -1, posjump = 0;
		for (UINT i=0; i<_this->m_nChannels; i++, p++)
		{
			if (p->command == CMD_POSITIONJUMP)
			{
				if (p->param < nStartOrder) return FALSE;
				if (p->param > nStartOrder) return TRUE;
				posjump = TRUE;
			} else
			if (p->command == CMD_PATTERNBREAK)
			{
				breakrow = p->param;
			}
		}
		if (breakrow >= 0)
		{
			if (!posjump) return TRUE;
			row = breakrow;
		}
		if (row >= nRows) return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////
// Note/Period/Frequency functions

UINT CSoundFile_GetNoteFromPeriod(CSoundFile *_this, UINT period)
//---------------------------------------------------
{
	if (!period) return 0;
	if (_this->m_nType & (MOD_TYPE_MED|MOD_TYPE_MOD|MOD_TYPE_MTM|MOD_TYPE_669|MOD_TYPE_OKT|MOD_TYPE_AMF0))
	{
		period >>= 2;
		for (UINT i=0; i<6*12; i++)
		{
			if (period >= ProTrackerPeriodTable[i])
			{
				if ((period != ProTrackerPeriodTable[i]) && (i))
				{
					UINT p1 = ProTrackerPeriodTable[i-1];
					UINT p2 = ProTrackerPeriodTable[i];
					if (p1 - period < (period - p2)) return i+36;
				}
				return i+1+36;
			}
		}
		return 6*12+36;
	} else
	{
		for (UINT i=1; i<NOTE_MAX; i++)
		{
			LONG n = CSoundFile_GetPeriodFromNote(_this, i, 0, 0);
			if ((n > 0) && (n <= (LONG)period)) return i;
		}
		return NOTE_MAX;
	}
}



UINT CSoundFile_GetPeriodFromNote(CSoundFile *_this, UINT note, int nFineTune, UINT nC4Speed)
//-------------------------------------------------------------------------------
{
	if ((!note) || (note > 0xF0)) return 0;
	if (_this->m_nType & (MOD_TYPE_IT|MOD_TYPE_S3M|MOD_TYPE_STM|MOD_TYPE_MDL|MOD_TYPE_ULT|MOD_TYPE_WAV
				|MOD_TYPE_FAR|MOD_TYPE_DMF|MOD_TYPE_PTM|MOD_TYPE_AMS|MOD_TYPE_DBM|MOD_TYPE_AMF|MOD_TYPE_PSM))
	{
		note--;
		if (_this->m_dwSongFlags & SONG_LINEARSLIDES)
		{
			return (FreqS3MTable[note % 12] << 5) >> (note / 12);
		} else
		{
			if (!nC4Speed) nC4Speed = 8363;
			return _muldiv(8363, (FreqS3MTable[note % 12] << 5), nC4Speed << (note / 12));
		}
	} else
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (note < 13) note = 13;
		note -= 13;
		if (_this->m_dwSongFlags & SONG_LINEARSLIDES)
		{
			LONG l = ((NOTE_MAX - note) << 6) - (nFineTune / 2);
			if (l < 1) l = 1;
			return (UINT)l;
		} else
		{
			int finetune = nFineTune;
			UINT rnote = (note % 12) << 3;
			UINT roct = note / 12;
			int rfine = finetune / 16;
			int i = rnote + rfine + 8;
			if (i < 0) i = 0;
			if (i >= 104) i = 103;
			UINT per1 = XMPeriodTable[i];
			if ( finetune < 0 )
			{
				rfine--;
				finetune = -finetune;
			} else rfine++;
			i = rnote+rfine+8;
			if (i < 0) i = 0;
			if (i >= 104) i = 103;
			UINT per2 = XMPeriodTable[i];
			rfine = finetune & 0x0F;
			per1 *= 16-rfine;
			per2 *= rfine;
			return ((per1 + per2) << 1) >> roct;
		}
	} else
	{
		note--;
		nFineTune = XM2MODFineTune(nFineTune);
		if ((nFineTune) || (note < 36) || (note >= 36+6*12))
			return (ProTrackerTunedPeriods[nFineTune*12 + note % 12] << 5) >> (note / 12);
		else
			return (ProTrackerPeriodTable[note-36] << 2);
	}
}


UINT CSoundFile_GetFreqFromPeriod(CSoundFile *_this, UINT period, UINT nC4Speed, int nPeriodFrac)
//-----------------------------------------------------------------------------------
{
	if (!period) return 0;
	if (_this->m_nType & (MOD_TYPE_MED|MOD_TYPE_MOD|MOD_TYPE_MTM|MOD_TYPE_669|MOD_TYPE_OKT|MOD_TYPE_AMF0))
	{
		return (3546895L*4) / period;
	} else
	if (_this->m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (_this->m_dwSongFlags & SONG_LINEARSLIDES)
			return XMLinearTable[period % 768] >> (period / 768);
		else
			return 8363 * 1712L / period;
	} else
	{
		if (_this->m_dwSongFlags & SONG_LINEARSLIDES)
		{
			if (!nC4Speed) nC4Speed = 8363;
			return _muldiv(nC4Speed, 1712L << 8, (period << 8)+nPeriodFrac);
		} else
		{
			return _muldiv(8363, 1712L << 8, (period << 8)+nPeriodFrac);
		}
	}
}


