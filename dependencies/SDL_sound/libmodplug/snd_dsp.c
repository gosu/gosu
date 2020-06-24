/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"

static UINT GetMaskFromSize(UINT len)
//-----------------------------------
{
	UINT n = 2;
	while (n <= len) n <<= 1;
	return ((n >> 1) - 1);
}


void CSoundFile_InitializeDSP(CSoundFile *_this, BOOL bReset)
//-----------------------------------------
{
	if (!_this->m_nReverbDelay) _this->m_nReverbDelay = 100;
	if (!_this->m_nXBassRange) _this->m_nXBassRange = XBASS_DELAY;
	if (!_this->m_nProLogicDelay) _this->m_nProLogicDelay = 20;
	if (_this->m_nXBassDepth > 8) _this->m_nXBassDepth = 8;
	if (_this->m_nXBassDepth < 2) _this->m_nXBassDepth = 2;
	if (bReset)
	{
		// Noise Reduction
		_this->nLeftNR = _this->nRightNR = 0;
	}
	// Pro-Logic Surround
	_this->nSurroundPos = _this->nSurroundSize = 0;
	_this->nDolbyLoFltPos = _this->nDolbyLoFltSum = _this->nDolbyLoDlyPos = 0;
	_this->nDolbyHiFltPos = _this->nDolbyHiFltSum = 0;
	if (_this->gdwSoundSetup & SNDMIX_SURROUND)
	{
		SDL_memset(_this->DolbyLoFilterBuffer, 0, sizeof(_this->DolbyLoFilterBuffer));
		SDL_memset(_this->DolbyHiFilterBuffer, 0, sizeof(_this->DolbyHiFilterBuffer));
		SDL_memset(_this->DolbyLoFilterDelay, 0, sizeof(_this->DolbyLoFilterDelay));
		SDL_memset(_this->SurroundBuffer, 0, sizeof(_this->SurroundBuffer));
		_this->nSurroundSize = (_this->gdwMixingFreq * _this->m_nProLogicDelay) / 1000;
		if (_this->nSurroundSize > SURROUNDBUFFERSIZE) _this->nSurroundSize = SURROUNDBUFFERSIZE;
		if (_this->m_nProLogicDepth < 8) _this->nDolbyDepth = (32 >> _this->m_nProLogicDepth) + 32;
		else _this->nDolbyDepth = (_this->m_nProLogicDepth < 16) ? (8 + (_this->m_nProLogicDepth - 8) * 7) : 64;
		_this->nDolbyDepth >>= 2;
	}
	// Reverb Setup
#ifndef MODPLUG_NO_REVERB
	if (_this->gdwSoundSetup & SNDMIX_REVERB)
	{
		UINT nrs = (_this->gdwMixingFreq * _this->m_nReverbDelay) / 1000;
		UINT nfa = _this->m_nReverbDepth+1;
		if (nrs > REVERBBUFFERSIZE) nrs = REVERBBUFFERSIZE;
		if ((bReset) || (nrs != (UINT)_this->nReverbSize) || (nfa != (UINT)_this->nFilterAttn))
		{
			_this->nFilterAttn = nfa;
			_this->nReverbSize = nrs;
			_this->nReverbBufferPos = _this->nReverbBufferPos2 = _this->nReverbBufferPos3 = _this->nReverbBufferPos4 = 0;
			_this->nReverbLoFltSum = _this->nReverbLoFltPos = _this->nReverbLoDlyPos = 0;
			_this->gRvbLPSum = _this->gRvbLPPos = 0;
			_this->nReverbSize2 = (_this->nReverbSize * 13) / 17;
			if (_this->nReverbSize2 > REVERBBUFFERSIZE2) _this->nReverbSize2 = REVERBBUFFERSIZE2;
			_this->nReverbSize3 = (_this->nReverbSize * 7) / 13;
			if (_this->nReverbSize3 > REVERBBUFFERSIZE3) _this->nReverbSize3 = REVERBBUFFERSIZE3;
			_this->nReverbSize4 = (_this->nReverbSize * 7) / 19;
			if (_this->nReverbSize4 > REVERBBUFFERSIZE4) _this->nReverbSize4 = REVERBBUFFERSIZE4;
			SDL_memset(_this->ReverbLoFilterBuffer, 0, sizeof(_this->ReverbLoFilterBuffer));
			SDL_memset(_this->ReverbLoFilterDelay, 0, sizeof(_this->ReverbLoFilterDelay));
			SDL_memset(_this->ReverbBuffer, 0, sizeof(_this->ReverbBuffer));
			SDL_memset(_this->ReverbBuffer2, 0, sizeof(_this->ReverbBuffer2));
			SDL_memset(_this->ReverbBuffer3, 0, sizeof(_this->ReverbBuffer3));
			SDL_memset(_this->ReverbBuffer4, 0, sizeof(_this->ReverbBuffer4));
			SDL_memset(_this->gRvbLowPass, 0, sizeof(_this->gRvbLowPass));
		}
	} else _this->nReverbSize = 0;
#endif
	BOOL bResetBass = FALSE;
	// Bass Expansion Reset
	if (_this->gdwSoundSetup & SNDMIX_MEGABASS)
	{
		UINT nXBassSamples = (_this->gdwMixingFreq * _this->m_nXBassRange) / 10000;
		if (nXBassSamples > XBASSBUFFERSIZE) nXBassSamples = XBASSBUFFERSIZE;
		UINT mask = GetMaskFromSize(nXBassSamples);
		if ((bReset) || (mask != (UINT)_this->nXBassMask))
		{
			_this->nXBassMask = mask;
			bResetBass = TRUE;
		}
	} else
	{
		_this->nXBassMask = 0;
		bResetBass = TRUE;
	}
	if (bResetBass)
	{
		_this->nXBassSum = _this->nXBassBufferPos = _this->nXBassDlyPos = 0;
		SDL_memset(_this->XBassBuffer, 0, sizeof(_this->XBassBuffer));
		SDL_memset(_this->XBassDelay, 0, sizeof(_this->XBassDelay));
	}
}


void CSoundFile_ProcessStereoDSP(CSoundFile *_this, int count)
//------------------------------------------
{
#ifndef MODPLUG_NO_REVERB
	// Reverb
	if (_this->gdwSoundSetup & SNDMIX_REVERB)
	{
		int *pr = _this->MixSoundBuffer, *pin = _this->MixReverbBuffer, rvbcount = count;
		do
		{
			int echo = _this->ReverbBuffer[_this->nReverbBufferPos] + _this->ReverbBuffer2[_this->nReverbBufferPos2]
					+ _this->ReverbBuffer3[_this->nReverbBufferPos3] + _this->ReverbBuffer4[_this->nReverbBufferPos4];	// echo = reverb signal
			// Delay line and remove Low Frequencies			// v = original signal
			int echodly = _this->ReverbLoFilterDelay[_this->nReverbLoDlyPos];	// echodly = delayed signal
			_this->ReverbLoFilterDelay[_this->nReverbLoDlyPos] = echo >> 1;
			_this->nReverbLoDlyPos++;
			_this->nReverbLoDlyPos &= 0x1F;
			int n = _this->nReverbLoFltPos;
			_this->nReverbLoFltSum -= _this->ReverbLoFilterBuffer[n];
			int tmp = echo / 128;
			_this->ReverbLoFilterBuffer[n] = tmp;
			_this->nReverbLoFltSum += tmp;
			echodly -= _this->nReverbLoFltSum;
			_this->nReverbLoFltPos = (n + 1) & 0x3F;
			// Reverb
			int v = (pin[0]+pin[1]) >> _this->nFilterAttn;
			pr[0] += pin[0] + echodly;
			pr[1] += pin[1] + echodly;
			v += echodly >> 2;
			_this->ReverbBuffer3[_this->nReverbBufferPos3] = v;
			_this->ReverbBuffer4[_this->nReverbBufferPos4] = v;
			v += echodly >> 4;
			v >>= 1;
			_this->gRvbLPSum -= _this->gRvbLowPass[_this->gRvbLPPos];
			_this->gRvbLPSum += v;
			_this->gRvbLowPass[_this->gRvbLPPos] = v;
			_this->gRvbLPPos++;
			_this->gRvbLPPos &= 7;
			const int vlp = _this->gRvbLPSum >> 2;
			_this->ReverbBuffer[_this->nReverbBufferPos] = vlp;
			_this->ReverbBuffer2[_this->nReverbBufferPos2] = vlp;
			if (++_this->nReverbBufferPos >= _this->nReverbSize) _this->nReverbBufferPos = 0;
			if (++_this->nReverbBufferPos2 >= _this->nReverbSize2) _this->nReverbBufferPos2 = 0;
			if (++_this->nReverbBufferPos3 >= _this->nReverbSize3) _this->nReverbBufferPos3 = 0;
			if (++_this->nReverbBufferPos4 >= _this->nReverbSize4) _this->nReverbBufferPos4 = 0;
			pr += 2;
			pin += 2;
		} while (--rvbcount);
	}
#endif
	// Dolby Pro-Logic Surround
	if (_this->gdwSoundSetup & SNDMIX_SURROUND)
	{
		int *pr = _this->MixSoundBuffer, n = _this->nDolbyLoFltPos;
		for (int r=count; r; r--)
		{
			int v = (pr[0]+pr[1]+DOLBYATTNROUNDUP) >> (nDolbyHiFltAttn+1);
			v *= (int)_this->nDolbyDepth;
			// Low-Pass Filter
			_this->nDolbyHiFltSum -= _this->DolbyHiFilterBuffer[_this->nDolbyHiFltPos];
			_this->DolbyHiFilterBuffer[_this->nDolbyHiFltPos] = v;
			_this->nDolbyHiFltSum += v;
			v = _this->nDolbyHiFltSum;
			_this->nDolbyHiFltPos++;
			_this->nDolbyHiFltPos &= nDolbyHiFltMask;
			// Surround
			int secho = _this->SurroundBuffer[_this->nSurroundPos];
			_this->SurroundBuffer[_this->nSurroundPos] = v;
			// Delay line and remove low frequencies
			v = _this->DolbyLoFilterDelay[_this->nDolbyLoDlyPos];		// v = delayed signal
			_this->DolbyLoFilterDelay[_this->nDolbyLoDlyPos] = secho;	// secho = signal
			_this->nDolbyLoDlyPos++;
			_this->nDolbyLoDlyPos &= 0x1F;
			_this->nDolbyLoFltSum -= _this->DolbyLoFilterBuffer[n];
			int tmp = secho / 64;
			_this->DolbyLoFilterBuffer[n] = tmp;
			_this->nDolbyLoFltSum += tmp;
			v -= _this->nDolbyLoFltSum;
			n++;
			n &= 0x3F;
			// Add echo
			pr[0] += v;
			pr[1] -= v;
			if (++_this->nSurroundPos >= _this->nSurroundSize) _this->nSurroundPos = 0;
			pr += 2;
		}
		_this->nDolbyLoFltPos = n;
	}
	// Bass Expansion
	if (_this->gdwSoundSetup & SNDMIX_MEGABASS)
	{
		int *px = _this->MixSoundBuffer;
		int xba = _this->m_nXBassDepth+1, xbamask = (1 << xba) - 1;
		int n = _this->nXBassBufferPos;
		for (int x=count; x; x--)
		{
			_this->nXBassSum -= _this->XBassBuffer[n];
			int tmp0 = px[0] + px[1];
			int tmp = (tmp0 + ((tmp0 >> 31) & xbamask)) >> xba;
			_this->XBassBuffer[n] = tmp;
			_this->nXBassSum += tmp;
			int v = _this->XBassDelay[_this->nXBassDlyPos];
			_this->XBassDelay[_this->nXBassDlyPos] = px[0];
			px[0] = v + _this->nXBassSum;
			v = _this->XBassDelay[_this->nXBassDlyPos+1];
			_this->XBassDelay[_this->nXBassDlyPos+1] = px[1];
			px[1] = v + _this->nXBassSum;
			_this->nXBassDlyPos = (_this->nXBassDlyPos + 2) & _this->nXBassMask;
			px += 2;
			n++;
			n &= _this->nXBassMask;
		}
		_this->nXBassBufferPos = n;
	}
	// Noise Reduction
	if (_this->gdwSoundSetup & SNDMIX_NOISEREDUCTION)
	{
		int n1 = _this->nLeftNR, n2 = _this->nRightNR;
		int *pnr = _this->MixSoundBuffer;
		for (int nr=count; nr; nr--)
		{
			int vnr = pnr[0] >> 1;
			pnr[0] = vnr + n1;
			n1 = vnr;
			vnr = pnr[1] >> 1;
			pnr[1] = vnr + n2;
			n2 = vnr;
			pnr += 2;
		}
		_this->nLeftNR = n1;
		_this->nRightNR = n2;
	}
}


/////////////////////////////////////////////////////////////////
// Clean DSP Effects interface

// [Reverb level 0(quiet)-100(loud)], [delay in ms, usually 40-200ms]
BOOL CSoundFile_SetReverbParameters(CSoundFile *_this, UINT nDepth, UINT nDelay)
//------------------------------------------------------------
{
	if (nDepth > 100) nDepth = 100;
	UINT gain = nDepth / 20;
	if (gain > 4) gain = 4;
	_this->m_nReverbDepth = 4 - gain;
	if (nDelay < 40) nDelay = 40;
	if (nDelay > 250) nDelay = 250;
	_this->m_nReverbDelay = nDelay;
	return TRUE;
}


// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 20-100]
BOOL CSoundFile_SetXBassParameters(CSoundFile *_this, UINT nDepth, UINT nRange)
//-----------------------------------------------------------
{
	if (nDepth > 100) nDepth = 100;
	UINT gain = nDepth / 20;
	if (gain > 4) gain = 4;
	_this->m_nXBassDepth = 8 - gain;	// filter attenuation 1/256 .. 1/16
	UINT range = nRange / 5;
	if (range > 5) range -= 5; else range = 0;
	if (nRange > 16) nRange = 16;
	_this->m_nXBassRange = 21 - range;	// filter average on 0.5-1.6ms
	return TRUE;
}


// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-50ms]
BOOL CSoundFile_SetSurroundParameters(CSoundFile *_this, UINT nDepth, UINT nDelay)
//--------------------------------------------------------------
{
	UINT gain = (nDepth * 16) / 100;
	if (gain > 16) gain = 16;
	if (gain < 1) gain = 1;
	_this->m_nProLogicDepth = gain;
	if (nDelay < 4) nDelay = 4;
	if (nDelay > 50) nDelay = 50;
	_this->m_nProLogicDelay = nDelay;
	return TRUE;
}

BOOL CSoundFile_SetWaveConfigEx(CSoundFile *_this, BOOL bSurround,BOOL bNoOverSampling,BOOL bReverb,BOOL hqido,BOOL bMegaBass,BOOL bNR,BOOL bEQ)
//----------------------------------------------------------------------------------------------------------------------------
{
	DWORD d = _this->gdwSoundSetup & ~(SNDMIX_SURROUND | SNDMIX_NORESAMPLING | SNDMIX_REVERB | SNDMIX_HQRESAMPLER | SNDMIX_MEGABASS | SNDMIX_NOISEREDUCTION | SNDMIX_EQ);
	if (bSurround) d |= SNDMIX_SURROUND;
	if (bNoOverSampling) d |= SNDMIX_NORESAMPLING;
	if (bReverb) d |= SNDMIX_REVERB;
	if (hqido) d |= SNDMIX_HQRESAMPLER;
	if (bMegaBass) d |= SNDMIX_MEGABASS;
	if (bNR) d |= SNDMIX_NOISEREDUCTION;
	if (bEQ) d |= SNDMIX_EQ;
	_this->gdwSoundSetup = d;
	CSoundFile_InitPlayer(_this, FALSE);
	return TRUE;
}

