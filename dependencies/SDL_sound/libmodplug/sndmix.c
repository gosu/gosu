/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"
#include "tables.h"

// Volume ramp length, in 1/10 ms
#define VOLUMERAMPLEN	146	// 1.46ms = 64 samples at 44.1kHz

typedef DWORD (MPPASMCALL * LPCONVERTPROC)(LPVOID, int *, DWORD, LPLONG, LPLONG);

extern DWORD MPPASMCALL X86_Convert32To8(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern DWORD MPPASMCALL X86_Convert32To16(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern DWORD MPPASMCALL X86_Convert32To24(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern DWORD MPPASMCALL X86_Convert32To32(LPVOID lpBuffer, int *, DWORD nSamples, LPLONG, LPLONG);
extern UINT MPPASMCALL X86_AGC(int *pBuffer, UINT nSamples, UINT nAGC);
extern VOID MPPASMCALL X86_Dither(int *pBuffer, UINT nSamples, UINT nBits);
extern VOID MPPASMCALL X86_InterleaveFrontRear(int *pFrontBuf, int *pRearBuf, DWORD nSamples);
extern VOID MPPASMCALL X86_StereoFill(int *pBuffer, UINT nSamples, LPLONG lpROfs, LPLONG lpLOfs);
extern VOID MPPASMCALL X86_MonoFromStereo(int *pMixBuf, UINT nSamples);


// Log tables for pre-amp
// We don't want the tracker to get too loud
const UINT PreAmpTable[16] =
{
	0x60, 0x60, 0x60, 0x70,	// 0-7
	0x80, 0x88, 0x90, 0x98,	// 8-15
	0xA0, 0xA4, 0xA8, 0xB0,	// 16-23
	0xB4, 0xB8, 0xBC, 0xC0,	// 24-31
};

const UINT PreAmpAGCTable[16] =
{
	0x60, 0x60, 0x60, 0x60,
	0x68, 0x70, 0x78, 0x80,
	0x84, 0x88, 0x8C, 0x90,
	0x94, 0x98, 0x9C, 0xA0,
};


// Return (a*b)/c - no divide error
int _muldiv(long a, long b, long c)
{
	return ((uint64_t) a * (uint64_t) b ) / c;
}


// Return (a*b+c/2)/c - no divide error
int _muldivr(long a, long b, long c)
{
	return ((uint64_t) a * (uint64_t) b + (c >> 1)) / c;
}


BOOL CSoundFile_InitPlayer(CSoundFile *_this, BOOL bReset)
//--------------------------------------
{
	if (_this->m_nMaxMixChannels > MAX_CHANNELS) _this->m_nMaxMixChannels = MAX_CHANNELS;
	if (_this->gdwMixingFreq < 4000) _this->gdwMixingFreq = 4000;
	if (_this->gdwMixingFreq > MAX_SAMPLE_RATE) _this->gdwMixingFreq = MAX_SAMPLE_RATE;
	_this->gnVolumeRampSamples = (_this->gdwMixingFreq * VOLUMERAMPLEN) / 100000;
	if (_this->gnVolumeRampSamples < 8) _this->gnVolumeRampSamples = 8;
	_this->gnDryROfsVol = _this->gnDryLOfsVol = 0;
	_this->gnRvbROfsVol = _this->gnRvbLOfsVol = 0;
	_this->gbInitPlugins = (bReset) ? 3 : 1;
	CSoundFile_InitializeDSP(_this, bReset);
	return TRUE;
}


BOOL CSoundFile_FadeSong(CSoundFile *_this, UINT msec)
//----------------------------------
{
	LONG nsamples = _muldiv(msec, _this->gdwMixingFreq, 1000);
	if (nsamples <= 0) return FALSE;
	if (nsamples > 0x100000) nsamples = 0x100000;
	_this->m_nBufferCount = nsamples;
	LONG nRampLength = _this->m_nBufferCount;
	// Ramp everything down
	for (UINT noff=0; noff < _this->m_nMixChannels; noff++)
	{
		MODCHANNEL *pramp = &_this->Chn[_this->ChnMix[noff]];
		if (!pramp) continue;
		pramp->nNewLeftVol = pramp->nNewRightVol = 0;
		pramp->nRightRamp = (-pramp->nRightVol << VOLUMERAMPPRECISION) / nRampLength;
		pramp->nLeftRamp = (-pramp->nLeftVol << VOLUMERAMPPRECISION) / nRampLength;
		pramp->nRampRightVol = pramp->nRightVol << VOLUMERAMPPRECISION;
		pramp->nRampLeftVol = pramp->nLeftVol << VOLUMERAMPPRECISION;
		pramp->nRampLength = nRampLength;
		pramp->dwFlags |= CHN_VOLUMERAMP;
	}
	_this->m_dwSongFlags |= SONG_FADINGSONG;
	return TRUE;
}


BOOL CSoundFile_GlobalFadeSong(CSoundFile *_this, UINT msec)
//----------------------------------------
{
	if (_this->m_dwSongFlags & SONG_GLOBALFADE) return FALSE;
	_this->m_nGlobalFadeMaxSamples = _muldiv(msec, _this->gdwMixingFreq, 1000);
	_this->m_nGlobalFadeSamples = _this->m_nGlobalFadeMaxSamples;
	_this->m_dwSongFlags |= SONG_GLOBALFADE;
	return TRUE;
}


UINT CSoundFile_Read(CSoundFile *_this, LPVOID lpDestBuffer, UINT cbBuffer)
//-------------------------------------------------------
{
	LPBYTE lpBuffer = (LPBYTE)lpDestBuffer;
	LPCONVERTPROC pCvt = X86_Convert32To8;
	UINT lRead, lMax, lSampleSize, lCount, lSampleCount, nStat=0;
	LONG nVUMeterMin = 0x7FFFFFFF, nVUMeterMax = -0x7FFFFFFF;

	_this->m_nMixStat = 0;
	lSampleSize = _this->gnChannels;
	if (_this->gnBitsPerSample == 16) { lSampleSize *= 2; pCvt = X86_Convert32To16; }
	else if (_this->gnBitsPerSample == 24) { lSampleSize *= 3; pCvt = X86_Convert32To24; }
	else if (_this->gnBitsPerSample == 32) { lSampleSize *= 4; pCvt = X86_Convert32To32; }
	lMax = cbBuffer / lSampleSize;
	if ((!lMax) || (!lpBuffer) || (!_this->m_nChannels)) return 0;
	lRead = lMax;
	if (_this->m_dwSongFlags & SONG_ENDREACHED) goto MixDone;
	while (lRead > 0)
	{
		// Update Channel Data
		if (!_this->m_nBufferCount)
		{
			if (_this->m_dwSongFlags & SONG_FADINGSONG)
			{
				_this->m_dwSongFlags |= SONG_ENDREACHED;
				_this->m_nBufferCount = lRead;
			} else
			if (!CSoundFile_ReadNote(_this))
			{
				if (!CSoundFile_FadeSong(_this, FADESONGDELAY))
				{
					_this->m_dwSongFlags |= SONG_ENDREACHED;
					if (lRead == lMax) goto MixDone;
					_this->m_nBufferCount = lRead;
				}
			}
		}
		lCount = _this->m_nBufferCount;
		if (lCount > MIXBUFFERSIZE) lCount = MIXBUFFERSIZE;
		if (lCount > lRead) lCount = lRead;
		if (!lCount) break;
		lSampleCount = lCount;
#ifndef MODPLUG_NO_REVERB
		_this->gnReverbSend = 0;
#endif
		// Resetting sound buffer
		X86_StereoFill(_this->MixSoundBuffer, lSampleCount, &_this->gnDryROfsVol, &_this->gnDryLOfsVol);
		if (_this->gnChannels >= 2)
		{
			lSampleCount *= 2;
			_this->m_nMixStat += CSoundFile_CreateStereoMix(_this, lCount);
			CSoundFile_ProcessStereoDSP(_this, lCount);
		} else
		{
			_this->m_nMixStat += CSoundFile_CreateStereoMix(_this, lCount);
			CSoundFile_ProcessStereoDSP(_this, lCount);
			X86_MonoFromStereo(_this->MixSoundBuffer, lCount);
		}
		nStat++;
		UINT lTotalSampleCount = lSampleCount;
		// Multichannel
		if (_this->gnChannels > 2)
		{
			X86_InterleaveFrontRear(_this->MixSoundBuffer, _this->MixRearBuffer, lSampleCount);
			lTotalSampleCount *= 2;
		}
		// Perform clipping + VU-Meter
		lpBuffer += pCvt(lpBuffer, _this->MixSoundBuffer, lTotalSampleCount, &nVUMeterMin, &nVUMeterMax);
		// Buffer ready
		lRead -= lCount;
		_this->m_nBufferCount -= lCount;
	}
MixDone:
	if (lRead) SDL_memset(lpBuffer, (_this->gnBitsPerSample == 8) ? 0x80 : 0, lRead * lSampleSize);
	if (nStat) { _this->m_nMixStat += nStat-1; _this->m_nMixStat /= nStat; }
	return lMax - lRead;
}



/////////////////////////////////////////////////////////////////////////////
// Handles navigation/effects

BOOL CSoundFile_ProcessRow(CSoundFile *_this)
//---------------------------
{
	if (++_this->m_nTickCount >= _this->m_nMusicSpeed * (_this->m_nPatternDelay+1) + _this->m_nFrameDelay)
	{
		_this->m_nPatternDelay = 0;
		_this->m_nFrameDelay = 0;
		_this->m_nTickCount = 0;
		_this->m_nRow = _this->m_nNextRow;
		// Reset Pattern Loop Effect
		if (_this->m_nCurrentPattern != _this->m_nNextPattern) _this->m_nCurrentPattern = _this->m_nNextPattern;
		// Check if pattern is valid
		if (!(_this->m_dwSongFlags & SONG_PATTERNLOOP))
		{
			_this->m_nPattern = (_this->m_nCurrentPattern < MAX_ORDERS) ? _this->Order[_this->m_nCurrentPattern] : 0xFF;
			if ((_this->m_nPattern < MAX_PATTERNS) && (!_this->Patterns[_this->m_nPattern])) _this->m_nPattern = 0xFE;
			while (_this->m_nPattern >= MAX_PATTERNS)
			{
				// End of song ?
				if ((_this->m_nPattern == 0xFF) || (_this->m_nCurrentPattern >= MAX_ORDERS))
				{
					//if (!_this->m_nRepeatCount)
						return FALSE;     //never repeat entire song
					if (!_this->m_nRestartPos)
					{
						_this->m_nMusicSpeed = _this->m_nDefaultSpeed;
						_this->m_nMusicTempo = _this->m_nDefaultTempo;
						_this->m_nGlobalVolume = _this->m_nDefaultGlobalVolume;
						for (UINT i=0; i<MAX_CHANNELS; i++)
						{
							_this->Chn[i].dwFlags |= CHN_NOTEFADE | CHN_KEYOFF;
							_this->Chn[i].nFadeOutVol = 0;
							if (i < _this->m_nChannels)
							{
								_this->Chn[i].nGlobalVol = _this->ChnSettings[i].nVolume;
								_this->Chn[i].nVolume = _this->ChnSettings[i].nVolume;
								_this->Chn[i].nPan = _this->ChnSettings[i].nPan;
								_this->Chn[i].nPanSwing = _this->Chn[i].nVolSwing = 0;
								_this->Chn[i].nOldVolParam = 0;
								_this->Chn[i].nOldOffset = 0;
								_this->Chn[i].nOldHiOffset = 0;
								_this->Chn[i].nPortamentoDest = 0;
								if (!_this->Chn[i].nLength)
								{
									_this->Chn[i].dwFlags = _this->ChnSettings[i].dwFlags;
									_this->Chn[i].nLoopStart = 0;
									_this->Chn[i].nLoopEnd = 0;
									_this->Chn[i].pHeader = NULL;
									_this->Chn[i].pSample = NULL;
									_this->Chn[i].pInstrument = NULL;
								}
							}
						}
					}
//					if (_this->m_nRepeatCount > 0) _this->m_nRepeatCount--;
					_this->m_nCurrentPattern = _this->m_nRestartPos;
					_this->m_nRow = 0;
					if ((_this->Order[_this->m_nCurrentPattern] >= MAX_PATTERNS) || (!_this->Patterns[_this->Order[_this->m_nCurrentPattern]])) return FALSE;
				} else
				{
					_this->m_nCurrentPattern++;
				}
				_this->m_nPattern = (_this->m_nCurrentPattern < MAX_ORDERS) ? _this->Order[_this->m_nCurrentPattern] : 0xFF;
				if ((_this->m_nPattern < MAX_PATTERNS) && (!_this->Patterns[_this->m_nPattern])) _this->m_nPattern = 0xFE;
			}
			_this->m_nNextPattern = _this->m_nCurrentPattern;
		}
		// Weird stuff?
		if ((_this->m_nPattern >= MAX_PATTERNS) || (!_this->Patterns[_this->m_nPattern])) return FALSE;
		// Should never happen
		if (_this->m_nRow >= _this->PatternSize[_this->m_nPattern]) _this->m_nRow = 0;
		_this->m_nNextRow = _this->m_nRow + 1;
		if (_this->m_nNextRow >= _this->PatternSize[_this->m_nPattern])
		{
			if (!(_this->m_dwSongFlags & SONG_PATTERNLOOP)) _this->m_nNextPattern = _this->m_nCurrentPattern + 1;
			_this->m_nNextRow = 0;
		}
		// Reset channel values
		MODCHANNEL *pChn = _this->Chn;
		MODCOMMAND *m = _this->Patterns[_this->m_nPattern] + _this->m_nRow * _this->m_nChannels;
		for (UINT nChn=0; nChn<_this->m_nChannels; pChn++, nChn++, m++)
		{
			pChn->nRowNote = m->note;
			pChn->nRowInstr = m->instr;
			pChn->nRowVolCmd = m->volcmd;
			pChn->nRowVolume = m->vol;
			pChn->nRowCommand = m->command;
			pChn->nRowParam = m->param;

			pChn->nLeftVol = pChn->nNewLeftVol;
			pChn->nRightVol = pChn->nNewRightVol;
			pChn->dwFlags &= ~(CHN_PORTAMENTO | CHN_VIBRATO | CHN_TREMOLO | CHN_PANBRELLO);
			pChn->nCommand = 0;
		}
	}
	// Should we process tick0 effects?
	if (!_this->m_nMusicSpeed) _this->m_nMusicSpeed = 1;
	_this->m_dwSongFlags |= SONG_FIRSTTICK;
	if (_this->m_nTickCount)
	{
		_this->m_dwSongFlags &= ~SONG_FIRSTTICK;
		if ((!(_this->m_nType & MOD_TYPE_XM)) && (_this->m_nTickCount < _this->m_nMusicSpeed * (1 + _this->m_nPatternDelay)))
		{
			if (!(_this->m_nTickCount % _this->m_nMusicSpeed)) _this->m_dwSongFlags |= SONG_FIRSTTICK;
		}

	}
	// Update Effects
	return CSoundFile_ProcessEffects(_this);
}


////////////////////////////////////////////////////////////////////////////////////////////
// Handles envelopes & mixer setup

BOOL CSoundFile_ReadNote(CSoundFile *_this)
//-------------------------
{
	if (!CSoundFile_ProcessRow(_this)) return FALSE;
	////////////////////////////////////////////////////////////////////////////////////
	_this->m_nTotalCount++;
	if (!_this->m_nMusicTempo) return FALSE;
	_this->m_nBufferCount = (_this->gdwMixingFreq * 5 * _this->m_nTempoFactor) / (_this->m_nMusicTempo << 8);
	// Master Volume + Pre-Amplification / Attenuation setup
	DWORD nMasterVol;
	{
		int nchn32 = (_this->m_nChannels < 32) ? _this->m_nChannels : 31;
		if ((_this->m_nType & MOD_TYPE_IT) && (_this->m_nInstruments) && (nchn32 < 6)) nchn32 = 6;
		int realmastervol = _this->m_nMasterVolume;
		if (realmastervol > 0x80)
		{
			realmastervol = 0x80 + ((realmastervol - 0x80) * (nchn32+4)) / 16;
		}
		UINT attenuation = (_this->gdwSoundSetup & SNDMIX_AGC) ? PreAmpAGCTable[nchn32>>1] : PreAmpTable[nchn32>>1];
		DWORD mastervol = (realmastervol * (_this->m_nSongPreAmp + 0x10)) >> 6;
		if (mastervol > 0x200) mastervol = 0x200;
		if ((_this->m_dwSongFlags & SONG_GLOBALFADE) && (_this->m_nGlobalFadeMaxSamples))
		{
			mastervol = _muldiv(mastervol, _this->m_nGlobalFadeSamples, _this->m_nGlobalFadeMaxSamples);
		}
		nMasterVol = (mastervol << 7) / attenuation;
		if (nMasterVol > 0x180) nMasterVol = 0x180;
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Update channels data
	_this->m_nMixChannels = 0;
	MODCHANNEL *pChn = _this->Chn;
	for (UINT nChn=0; nChn<MAX_CHANNELS; nChn++,pChn++)
	{
		if ((pChn->dwFlags & CHN_NOTEFADE) && (!(pChn->nFadeOutVol|pChn->nRightVol|pChn->nLeftVol)))
		{
			pChn->nLength = 0;
			pChn->nROfs = pChn->nLOfs = 0;
		}
		// Check for unused channel
		if ((pChn->dwFlags & CHN_MUTE) || ((nChn >= _this->m_nChannels) && (!pChn->nLength)))
		{
			continue;
		}
		// Reset channel data
		pChn->nInc = 0;
		pChn->nRealVolume = 0;
		pChn->nRealPan = pChn->nPan + pChn->nPanSwing;
		if (pChn->nRealPan < 0) pChn->nRealPan = 0;
		if (pChn->nRealPan > 256) pChn->nRealPan = 256;
		pChn->nRampLength = 0;
		// Calc Frequency
		if ((pChn->nPeriod)	&& (pChn->nLength))
		{
			int vol = pChn->nVolume + pChn->nVolSwing;

			if (vol < 0) vol = 0;
			if (vol > 256) vol = 256;
			// Tremolo
			if (pChn->dwFlags & CHN_TREMOLO)
			{
				UINT trempos = pChn->nTremoloPos & 0x3F;
				if (vol > 0)
				{
					int tremattn = (_this->m_nType & MOD_TYPE_XM) ? 5 : 6;
					switch (pChn->nTremoloType & 0x03)
					{
					case 1:
						vol += (ModRampDownTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
						break;
					case 2:
						vol += (ModSquareTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
						break;
					case 3:
						vol += (ModRandomTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
						break;
					default:
						vol += (ModSinusTable[trempos] * (int)pChn->nTremoloDepth) >> tremattn;
					}
				}
				if ((_this->m_nTickCount) || ((_this->m_nType & (MOD_TYPE_STM|MOD_TYPE_S3M|MOD_TYPE_IT)) && (!(_this->m_dwSongFlags & SONG_ITOLDEFFECTS))))
				{
					pChn->nTremoloPos = (trempos + pChn->nTremoloSpeed) & 0x3F;
				}
			}
			// Tremor
			if (pChn->nCommand == CMD_TREMOR)
			{
				UINT n = (pChn->nTremorParam >> 4) + (pChn->nTremorParam & 0x0F);
				UINT ontime = pChn->nTremorParam >> 4;
				if ((!(_this->m_nType & MOD_TYPE_IT)) || (_this->m_dwSongFlags & SONG_ITOLDEFFECTS)) { n += 2; ontime++; }
				UINT tremcount = (UINT)pChn->nTremorCount;
				if (tremcount >= n) tremcount = 0;
				if ((_this->m_nTickCount) || (_this->m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT)))
				{
					if (tremcount >= ontime) vol = 0;
					pChn->nTremorCount = (BYTE)(tremcount + 1);
				}
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			// Clip volume
			if (vol < 0) vol = 0;
			if (vol > 0x100) vol = 0x100;
			vol <<= 6;
			// Process Envelopes
			if (pChn->pHeader)
			{
				INSTRUMENTHEADER *penv = pChn->pHeader;
				// Volume Envelope
				if ((pChn->dwFlags & CHN_VOLENV) && (penv->nVolEnv))
				{
					int envpos = pChn->nVolEnvPosition;
					UINT pt = penv->nVolEnv - 1;
					for (UINT i=0; i<(UINT)(penv->nVolEnv-1); i++)
					{
						if (envpos <= penv->VolPoints[i])
						{
							pt = i;
							break;
						}
					}
					int x2 = penv->VolPoints[pt];
					int x1, envvol;
					if (envpos >= x2)
					{
						envvol = penv->VolEnv[pt] << 2;
						x1 = x2;
					} else
					if (pt)
					{
						envvol = penv->VolEnv[pt-1] << 2;
						x1 = penv->VolPoints[pt-1];
					} else
					{
						envvol = 0;
						x1 = 0;
					}
					if (envpos > x2) envpos = x2;
					if ((x2 > x1) && (envpos > x1))
					{
						envvol += ((envpos - x1) * (((int)penv->VolEnv[pt]<<2) - envvol)) / (x2 - x1);
					}
					if (envvol < 0) envvol = 0;
					if (envvol > 256) envvol = 256;
					vol = (vol * envvol) >> 8;
				}
				// Panning Envelope
				if ((pChn->dwFlags & CHN_PANENV) && (penv->nPanEnv))
				{
					int envpos = pChn->nPanEnvPosition;
					UINT pt = penv->nPanEnv - 1;
					for (UINT i=0; i<(UINT)(penv->nPanEnv-1); i++)
					{
						if (envpos <= penv->PanPoints[i])
						{
							pt = i;
							break;
						}
					}
					int x2 = penv->PanPoints[pt], y2 = penv->PanEnv[pt];
					int x1, envpan;
					if (envpos >= x2)
					{
						envpan = y2;
						x1 = x2;
					} else
					if (pt)
					{
						envpan = penv->PanEnv[pt-1];
						x1 = penv->PanPoints[pt-1];
					} else
					{
						envpan = 128;
						x1 = 0;
					}
					if ((x2 > x1) && (envpos > x1))
					{
						envpan += ((envpos - x1) * (y2 - envpan)) / (x2 - x1);
					}
					if (envpan < 0) envpan = 0;
					if (envpan > 64) envpan = 64;
					int pan = pChn->nPan;
					if (pan >= 128)
					{
						pan += ((envpan - 32) * (256 - pan)) / 32;
					} else
					{
						pan += ((envpan - 32) * (pan)) / 32;
					}
					if (pan < 0) pan = 0;
					if (pan > 256) pan = 256;
					pChn->nRealPan = pan;
				}
				// FadeOut volume
				if (pChn->dwFlags & CHN_NOTEFADE)
				{
					UINT fadeout = penv->nFadeOut;
					if (fadeout)
					{
						pChn->nFadeOutVol -= fadeout << 1;
						if (pChn->nFadeOutVol <= 0) pChn->nFadeOutVol = 0;
						vol = (vol * pChn->nFadeOutVol) >> 16;
					} else
					if (!pChn->nFadeOutVol)
					{
						vol = 0;
					}
				}
				// Pitch/Pan separation
				if ((penv->nPPS) && (pChn->nRealPan) && (pChn->nNote))
				{
					int pandelta = (int)pChn->nRealPan + (int)((int)(pChn->nNote - penv->nPPC - 1) * (int)penv->nPPS) / (int)8;
					if (pandelta < 0) pandelta = 0;
					if (pandelta > 256) pandelta = 256;
					pChn->nRealPan = pandelta;
				}
			} else
			{
				// No Envelope: key off => note cut
				if (pChn->dwFlags & CHN_NOTEFADE) // 1.41-: CHN_KEYOFF|CHN_NOTEFADE
				{
					pChn->nFadeOutVol = 0;
					vol = 0;
				}
			}
			// vol is 14-bits
			if (vol)
			{
				// IMPORTANT: pChn->nRealVolume is 14 bits !!!
				// -> _muldiv( 14+8, 6+6, 18); => RealVolume: 14-bit result (22+12-20)
				pChn->nRealVolume = _muldiv(vol * _this->m_nGlobalVolume, pChn->nGlobalVol * pChn->nInsVol, 1 << 20);
			}
			if (pChn->nPeriod < _this->m_nMinPeriod) pChn->nPeriod = _this->m_nMinPeriod;
			int period = pChn->nPeriod;
			if ((pChn->dwFlags & (CHN_GLISSANDO|CHN_PORTAMENTO)) ==	(CHN_GLISSANDO|CHN_PORTAMENTO))
			{
				period = CSoundFile_GetPeriodFromNote(_this, CSoundFile_GetNoteFromPeriod(_this, period), pChn->nFineTune, pChn->nC4Speed);
			}

			// Arpeggio ?
			if (pChn->nCommand == CMD_ARPEGGIO)
			{
				switch(_this->m_nTickCount % 3)
				{
				case 1:	period = CSoundFile_GetPeriodFromNote(_this, pChn->nNote + (pChn->nArpeggio >> 4), pChn->nFineTune, pChn->nC4Speed); break;
				case 2:	period = CSoundFile_GetPeriodFromNote(_this, pChn->nNote + (pChn->nArpeggio & 0x0F), pChn->nFineTune, pChn->nC4Speed); break;
				}
			}

			if (_this->m_dwSongFlags & SONG_AMIGALIMITS)
			{
				if (period < 113*4) period = 113*4;
				if (period > 856*4) period = 856*4;
			}

			// Pitch/Filter Envelope
			if ((pChn->pHeader) && (pChn->dwFlags & CHN_PITCHENV) && (pChn->pHeader->nPitchEnv))
			{
				INSTRUMENTHEADER *penv = pChn->pHeader;
				int envpos = pChn->nPitchEnvPosition;
				UINT pt = penv->nPitchEnv - 1;
				for (UINT i=0; i<(UINT)(penv->nPitchEnv-1); i++)
				{
					if (envpos <= penv->PitchPoints[i])
					{
						pt = i;
						break;
					}
				}
				int x2 = penv->PitchPoints[pt];
				int x1, envpitch;
				if (envpos >= x2)
				{
					envpitch = (((int)penv->PitchEnv[pt]) - 32) * 8;
					x1 = x2;
				} else
				if (pt)
				{
					envpitch = (((int)penv->PitchEnv[pt-1]) - 32) * 8;
					x1 = penv->PitchPoints[pt-1];
				} else
				{
					envpitch = 0;
					x1 = 0;
				}
				if (envpos > x2) envpos = x2;
				if ((x2 > x1) && (envpos > x1))
				{
					int envpitchdest = (((int)penv->PitchEnv[pt]) - 32) * 8;
					envpitch += ((envpos - x1) * (envpitchdest - envpitch)) / (x2 - x1);
				}
				if (envpitch < -256) envpitch = -256;
				if (envpitch > 256) envpitch = 256;
				// Filter Envelope: controls cutoff frequency
				if (penv->dwFlags & ENV_FILTER)
				{
#ifndef NO_FILTER
					CSoundFile_SetupChannelFilter(_this, pChn, (pChn->dwFlags & CHN_FILTER) ? FALSE : TRUE, envpitch);
#endif // NO_FILTER
				} else
				// Pitch Envelope
				{
					int l = envpitch;
					if (l < 0)
					{
						l = -l;
						if (l > 255) l = 255;
						period = _muldiv(period, LinearSlideUpTable[l], 0x10000);
					} else
					{
						if (l > 255) l = 255;
						period = _muldiv(period, LinearSlideDownTable[l], 0x10000);
					}
				}
			}

			// Vibrato
			if (pChn->dwFlags & CHN_VIBRATO)
			{
				UINT vibpos = pChn->nVibratoPos;
				LONG vdelta;
				switch (pChn->nVibratoType & 0x03)
				{
				case 1:
					vdelta = ModRampDownTable[vibpos];
					break;
				case 2:
					vdelta = ModSquareTable[vibpos];
					break;
				case 3:
					vdelta = ModRandomTable[vibpos];
					break;
				default:
					vdelta = ModSinusTable[vibpos];
				}
				UINT vdepth = ((_this->m_nType != MOD_TYPE_IT) || (_this->m_dwSongFlags & SONG_ITOLDEFFECTS)) ? 6 : 7;
				vdelta = (vdelta * (int)pChn->nVibratoDepth) >> vdepth;
				if ((_this->m_dwSongFlags & SONG_LINEARSLIDES) && (_this->m_nType & MOD_TYPE_IT))
				{
					LONG l = vdelta;
					if (l < 0)
					{
						l = -l;
						vdelta = _muldiv(period, LinearSlideDownTable[l >> 2], 0x10000) - period;
						if (l & 0x03) vdelta += _muldiv(period, FineLinearSlideDownTable[l & 0x03], 0x10000) - period;

					} else
					{
						vdelta = _muldiv(period, LinearSlideUpTable[l >> 2], 0x10000) - period;
						if (l & 0x03) vdelta += _muldiv(period, FineLinearSlideUpTable[l & 0x03], 0x10000) - period;

					}
				}
				period += vdelta;
				if ((_this->m_nTickCount) || ((_this->m_nType & MOD_TYPE_IT) && (!(_this->m_dwSongFlags & SONG_ITOLDEFFECTS))))
				{
					pChn->nVibratoPos = (vibpos + pChn->nVibratoSpeed) & 0x3F;
				}
			}
			// Panbrello
			if (pChn->dwFlags & CHN_PANBRELLO)
			{
				UINT panpos = ((pChn->nPanbrelloPos+0x10) >> 2) & 0x3F;
				LONG pdelta;
				switch (pChn->nPanbrelloType & 0x03)
				{
				case 1:
					pdelta = ModRampDownTable[panpos];
					break;
				case 2:
					pdelta = ModSquareTable[panpos];
					break;
				case 3:
					pdelta = ModRandomTable[panpos];
					break;
				default:
					pdelta = ModSinusTable[panpos];
				}
				pChn->nPanbrelloPos += pChn->nPanbrelloSpeed;
				pdelta = ((pdelta * (int)pChn->nPanbrelloDepth) + 2) >> 3;
				pdelta += pChn->nRealPan;
				if (pdelta < 0) pdelta = 0;
				if (pdelta > 256) pdelta = 256;
				pChn->nRealPan = pdelta;
			}
			int nPeriodFrac = 0;
			// Instrument Auto-Vibrato
			if ((pChn->pInstrument) && (pChn->pInstrument->nVibDepth))
			{
				MODINSTRUMENT *pins = pChn->pInstrument;
				if (pins->nVibSweep == 0)
				{
					pChn->nAutoVibDepth = pins->nVibDepth << 8;
				} else
				{
					if (_this->m_nType & MOD_TYPE_IT)
					{
						pChn->nAutoVibDepth += pins->nVibSweep << 3;
					} else
					if (!(pChn->dwFlags & CHN_KEYOFF))
					{
						pChn->nAutoVibDepth += (pins->nVibDepth << 8) /	pins->nVibSweep;
					}
					if ((pChn->nAutoVibDepth >> 8) > pins->nVibDepth)
						pChn->nAutoVibDepth = pins->nVibDepth << 8;
				}
				pChn->nAutoVibPos += pins->nVibRate;
				int val;
				switch(pins->nVibType)
				{
				case 4:	// Random
					val = ModRandomTable[pChn->nAutoVibPos & 0x3F];
					pChn->nAutoVibPos++;
					break;
				case 3:	// Ramp Down
					val = ((0x40 - (pChn->nAutoVibPos >> 1)) & 0x7F) - 0x40;
					break;
				case 2:	// Ramp Up
					val = ((0x40 + (pChn->nAutoVibPos >> 1)) & 0x7f) - 0x40;
					break;
				case 1:	// Square
					val = (pChn->nAutoVibPos & 128) ? +64 : -64;
					break;
				default:	// Sine
					val = ft2VibratoTable[pChn->nAutoVibPos & 255];
				}
				int n =	((val * pChn->nAutoVibDepth) >> 8);
				if (_this->m_nType & MOD_TYPE_IT)
				{
					int df1, df2;
					if (n < 0)
					{
						n = -n;
						UINT n1 = n >> 8;
						df1 = LinearSlideUpTable[n1];
						df2 = LinearSlideUpTable[n1+1];
					} else
					{
						UINT n1 = n >> 8;
						df1 = LinearSlideDownTable[n1];
						df2 = LinearSlideDownTable[n1+1];
					}
					n >>= 2;
					period = _muldiv(period, df1 + ((df2-df1)*(n&0x3F)>>6), 256);
					nPeriodFrac = period & 0xFF;
					period >>= 8;
				} else
				{
					period += (n >> 6);
				}
			}
			// Final Period
			if (period <= _this->m_nMinPeriod)
			{
				if (_this->m_nType & MOD_TYPE_S3M) pChn->nLength = 0;
				period = _this->m_nMinPeriod;
			}
			if (period > _this->m_nMaxPeriod)
			{
				if ((_this->m_nType & MOD_TYPE_IT) || (period >= 0x100000))
				{
					pChn->nFadeOutVol = 0;
					pChn->dwFlags |= CHN_NOTEFADE;
					pChn->nRealVolume = 0;
				}
				period = _this->m_nMaxPeriod;
				nPeriodFrac = 0;
			}
			UINT freq = CSoundFile_GetFreqFromPeriod(_this, period, pChn->nC4Speed, nPeriodFrac);
			if ((_this->m_nType & MOD_TYPE_IT) && (freq < 256))
			{
				pChn->nFadeOutVol = 0;
				pChn->dwFlags |= CHN_NOTEFADE;
				pChn->nRealVolume = 0;
			}
			UINT ninc = _muldiv(freq, 0x10000, _this->gdwMixingFreq);
			if ((ninc >= 0xFFB0) && (ninc <= 0x10090)) ninc = 0x10000;
			if (_this->m_nFreqFactor != 128) ninc = (ninc * _this->m_nFreqFactor) >> 7;
			if (ninc > 0xFF0000) ninc = 0xFF0000;
			pChn->nInc = (ninc+1) & ~3;
		}

		// Increment envelope position
		if (pChn->pHeader)
		{
			INSTRUMENTHEADER *penv = pChn->pHeader;
			// Volume Envelope
			if (pChn->dwFlags & CHN_VOLENV)
			{
				// Increase position
				pChn->nVolEnvPosition++;
				// Volume Loop ?
				if (penv->dwFlags & ENV_VOLLOOP)
				{
					UINT volloopend = penv->VolPoints[penv->nVolLoopEnd];
					if (_this->m_nType != MOD_TYPE_XM) volloopend++;
					if (pChn->nVolEnvPosition == volloopend)
					{
						pChn->nVolEnvPosition = penv->VolPoints[penv->nVolLoopStart];
						if ((penv->nVolLoopEnd == penv->nVolLoopStart) && (!penv->VolEnv[penv->nVolLoopStart])
						 && ((!(_this->m_nType & MOD_TYPE_XM)) || (penv->nVolLoopEnd+1 == penv->nVolEnv)))
						{
							pChn->dwFlags |= CHN_NOTEFADE;
							pChn->nFadeOutVol = 0;
						}
					}
				}
				// Volume Sustain ?
				if ((penv->dwFlags & ENV_VOLSUSTAIN) && (!(pChn->dwFlags & CHN_KEYOFF)))
				{
					if (pChn->nVolEnvPosition == (UINT)penv->VolPoints[penv->nVolSustainEnd]+1)
						pChn->nVolEnvPosition = penv->VolPoints[penv->nVolSustainBegin];
				} else
				// End of Envelope ?
				if (pChn->nVolEnvPosition > penv->VolPoints[penv->nVolEnv - 1])
				{
					if ((_this->m_nType & MOD_TYPE_IT) || (pChn->dwFlags & CHN_KEYOFF)) pChn->dwFlags |= CHN_NOTEFADE;
					pChn->nVolEnvPosition = penv->VolPoints[penv->nVolEnv - 1];
					if ((!penv->VolEnv[penv->nVolEnv-1]) && ((nChn >= _this->m_nChannels) || (_this->m_nType & MOD_TYPE_IT)))
					{
						pChn->dwFlags |= CHN_NOTEFADE;
						pChn->nFadeOutVol = 0;

						pChn->nRealVolume = 0;
					}
				}
			}
			// Panning Envelope
			if (pChn->dwFlags & CHN_PANENV)
			{
				pChn->nPanEnvPosition++;
				if (penv->dwFlags & ENV_PANLOOP)
				{
					UINT panloopend = penv->PanPoints[penv->nPanLoopEnd];
					if (_this->m_nType != MOD_TYPE_XM) panloopend++;
					if (pChn->nPanEnvPosition == panloopend)
						pChn->nPanEnvPosition = penv->PanPoints[penv->nPanLoopStart];
				}
				// Panning Sustain ?
				if ((penv->dwFlags & ENV_PANSUSTAIN) && (pChn->nPanEnvPosition == (UINT)penv->PanPoints[penv->nPanSustainEnd]+1)
				 && (!(pChn->dwFlags & CHN_KEYOFF)))
				{
					// Panning sustained
					pChn->nPanEnvPosition = penv->PanPoints[penv->nPanSustainBegin];
				} else
				{
					if (pChn->nPanEnvPosition > penv->PanPoints[penv->nPanEnv - 1])
						pChn->nPanEnvPosition = penv->PanPoints[penv->nPanEnv - 1];
				}
			}
			// Pitch Envelope
			if (pChn->dwFlags & CHN_PITCHENV)
			{
				// Increase position
				pChn->nPitchEnvPosition++;
				// Pitch Loop ?
				if (penv->dwFlags & ENV_PITCHLOOP)
				{
					if (pChn->nPitchEnvPosition >= penv->PitchPoints[penv->nPitchLoopEnd])
						pChn->nPitchEnvPosition = penv->PitchPoints[penv->nPitchLoopStart];
				}
				// Pitch Sustain ?
				if ((penv->dwFlags & ENV_PITCHSUSTAIN) && (!(pChn->dwFlags & CHN_KEYOFF)))
				{
					if (pChn->nPitchEnvPosition == (UINT)penv->PitchPoints[penv->nPitchSustainEnd]+1)
						pChn->nPitchEnvPosition = penv->PitchPoints[penv->nPitchSustainBegin];
				} else
				{
					if (pChn->nPitchEnvPosition > penv->PitchPoints[penv->nPitchEnv - 1])
						pChn->nPitchEnvPosition = penv->PitchPoints[penv->nPitchEnv - 1];
				}
			}
		}
		// Volume ramping
		pChn->dwFlags &= ~CHN_VOLUMERAMP;
		if ((pChn->nRealVolume) || (pChn->nLeftVol) || (pChn->nRightVol))
			pChn->dwFlags |= CHN_VOLUMERAMP;
		// Check for too big nInc
		if (((pChn->nInc >> 16) + 1) >= (LONG)(pChn->nLoopEnd - pChn->nLoopStart)) pChn->dwFlags &= ~CHN_LOOP;
		pChn->nNewRightVol = pChn->nNewLeftVol = 0;
		pChn->pCurrentSample = ((pChn->pSample) && (pChn->nLength) && (pChn->nInc)) ? pChn->pSample : NULL;
		if (pChn->pCurrentSample)
		{
#ifdef MODPLUG_TRACKER
			UINT kChnMasterVol = (pChn->dwFlags & CHN_EXTRALOUD) ? 0x100 : nMasterVol;
#else
#define		kChnMasterVol	nMasterVol
#endif // MODPLUG_TRACKER
			// Adjusting volumes
			if (_this->gnChannels >= 2)
			{
				int pan = ((int)pChn->nRealPan) - 128;
				pan *= (int)_this->m_nStereoSeparation;
				pan /= 128;
				pan += 128;

				if (pan < 0) pan = 0;
				if (pan > 256) pan = 256;
				if (_this->gdwSoundSetup & SNDMIX_REVERSESTEREO) pan = 256 - pan;
				LONG realvol = (pChn->nRealVolume * kChnMasterVol) >> (8-1);
				if (_this->gdwSoundSetup & SNDMIX_SOFTPANNING)
				{
					if (pan < 128)
					{
						pChn->nNewLeftVol = (realvol * pan) >> 8;
						pChn->nNewRightVol = (realvol * 128) >> 8;
					} else
					{
						pChn->nNewLeftVol = (realvol * 128) >> 8;
						pChn->nNewRightVol = (realvol * (256 - pan)) >> 8;
					}
				} else
				{
					pChn->nNewLeftVol = (realvol * pan) >> 8;
					pChn->nNewRightVol = (realvol * (256 - pan)) >> 8;
				}
			} else
			{
				pChn->nNewRightVol = (pChn->nRealVolume * kChnMasterVol) >> 8;
				pChn->nNewLeftVol = pChn->nNewRightVol;
			}
			// Clipping volumes
			if (pChn->nNewRightVol > 0xFFFF) pChn->nNewRightVol = 0xFFFF;
			if (pChn->nNewLeftVol > 0xFFFF) pChn->nNewLeftVol = 0xFFFF;
			// Check IDO
			if (_this->gdwSoundSetup & SNDMIX_NORESAMPLING)
			{
				pChn->dwFlags |= CHN_NOIDO;
			} else
			{
				pChn->dwFlags &= ~(CHN_NOIDO|CHN_HQSRC);
				if( pChn->nInc == 0x10000 )
				{	pChn->dwFlags |= CHN_NOIDO;
				}
				else
				{	if( ((_this->gdwSoundSetup & SNDMIX_HQRESAMPLER) == 0) && ((_this->gdwSoundSetup & SNDMIX_ULTRAHQSRCMODE) == 0) )
					{	if (pChn->nInc >= 0xFF00) pChn->dwFlags |= CHN_NOIDO;
					}
				}
			}
			pChn->nNewRightVol >>= MIXING_ATTENUATION;
			pChn->nNewLeftVol >>= MIXING_ATTENUATION;
			pChn->nRightRamp = pChn->nLeftRamp = 0;
			// Dolby Pro-Logic Surround
			if ((pChn->dwFlags & CHN_SURROUND) && (_this->gnChannels <= 2)) pChn->nNewLeftVol = - pChn->nNewLeftVol;
			// Checking Ping-Pong Loops
			if (pChn->dwFlags & CHN_PINGPONGFLAG) pChn->nInc = -pChn->nInc;
			// Setting up volume ramp
			if ((pChn->dwFlags & CHN_VOLUMERAMP)
			 && ((pChn->nRightVol != pChn->nNewRightVol)
			  || (pChn->nLeftVol != pChn->nNewLeftVol)))
			{
				LONG nRampLength = _this->gnVolumeRampSamples;
				LONG nRightDelta = ((pChn->nNewRightVol - pChn->nRightVol) << VOLUMERAMPPRECISION);
				LONG nLeftDelta = ((pChn->nNewLeftVol - pChn->nLeftVol) << VOLUMERAMPPRECISION);
				if ((_this->gdwSoundSetup & SNDMIX_DIRECTTODISK) || (_this->gdwSoundSetup & SNDMIX_HQRESAMPLER))
				{
					if ((pChn->nRightVol|pChn->nLeftVol) && (pChn->nNewRightVol|pChn->nNewLeftVol) && (!(pChn->dwFlags & CHN_FASTVOLRAMP)))
					{
						nRampLength = _this->m_nBufferCount;
						if (nRampLength > (1 << (VOLUMERAMPPRECISION-1))) nRampLength = (1 << (VOLUMERAMPPRECISION-1));
						if (nRampLength < (LONG)_this->gnVolumeRampSamples) nRampLength = _this->gnVolumeRampSamples;
					}
				}
				pChn->nRightRamp = nRightDelta / nRampLength;
				pChn->nLeftRamp = nLeftDelta / nRampLength;
				pChn->nRightVol = pChn->nNewRightVol - ((pChn->nRightRamp * nRampLength) >> VOLUMERAMPPRECISION);
				pChn->nLeftVol = pChn->nNewLeftVol - ((pChn->nLeftRamp * nRampLength) >> VOLUMERAMPPRECISION);
				if (pChn->nRightRamp|pChn->nLeftRamp)
				{
					pChn->nRampLength = nRampLength;
				} else
				{
					pChn->dwFlags &= ~CHN_VOLUMERAMP;
					pChn->nRightVol = pChn->nNewRightVol;
					pChn->nLeftVol = pChn->nNewLeftVol;
				}
			} else
			{
				pChn->dwFlags &= ~CHN_VOLUMERAMP;
				pChn->nRightVol = pChn->nNewRightVol;
				pChn->nLeftVol = pChn->nNewLeftVol;
			}
			pChn->nRampRightVol = pChn->nRightVol << VOLUMERAMPPRECISION;
			pChn->nRampLeftVol = pChn->nLeftVol << VOLUMERAMPPRECISION;
			// Adding the channel in the channel list
			_this->ChnMix[_this->m_nMixChannels++] = nChn;
			if (_this->m_nMixChannels >= MAX_CHANNELS) break;
		} else
		{
			pChn->nLeftVol = pChn->nRightVol = 0;
			pChn->nLength = 0;
		}
	}
	// Checking Max Mix Channels reached: ordering by volume
	if ((_this->m_nMixChannels >= _this->m_nMaxMixChannels) && (!(_this->gdwSoundSetup & SNDMIX_DIRECTTODISK)))
	{
		for (UINT i=0; i<_this->m_nMixChannels; i++)
		{
			UINT j=i;
			while ((j+1<_this->m_nMixChannels) && (_this->Chn[_this->ChnMix[j]].nRealVolume < _this->Chn[_this->ChnMix[j+1]].nRealVolume))
			{
				UINT n = _this->ChnMix[j];
				_this->ChnMix[j] = _this->ChnMix[j+1];
				_this->ChnMix[j+1] = n;
				j++;
			}
		}
	}
	if (_this->m_dwSongFlags & SONG_GLOBALFADE)
	{
		if (!_this->m_nGlobalFadeSamples)
		{
			_this->m_dwSongFlags |= SONG_ENDREACHED;
			return FALSE;
		}
		if (_this->m_nGlobalFadeSamples > _this->m_nBufferCount)
			_this->m_nGlobalFadeSamples -= _this->m_nBufferCount;
		else
			_this->m_nGlobalFadeSamples = 0;
	}
	return TRUE;
}


