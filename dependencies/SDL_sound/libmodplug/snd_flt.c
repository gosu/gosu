/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"

// AWE32: cutoff = reg[0-255] * 31.25 + 100 -> [100Hz-8060Hz]
// EMU10K1 docs: cutoff = reg[0-127]*62+100
#define FILTER_PRECISION	8192

#ifndef NO_FILTER

DWORD CSoundFile_CutOffToFrequency(CSoundFile *_this, UINT nCutOff, int flt_modifier)
//-----------------------------------------------------------------------
{
	float Fc;

	if (_this->m_dwSongFlags & SONG_EXFILTERRANGE)
		Fc = 110.0f * SDL_pow(2.0f, 0.25f + ((float)(nCutOff*(flt_modifier+256)))/(21.0f*512.0f));
	else
		Fc = 110.0f * SDL_pow(2.0f, 0.25f + ((float)(nCutOff*(flt_modifier+256)))/(24.0f*512.0f));
	LONG freq = (LONG)Fc;
	if (freq < 120) return 120;
	if (freq > 10000) return 10000;
	if (freq*2 > (LONG)_this->gdwMixingFreq) freq = _this->gdwMixingFreq>>1;
	return (DWORD)freq;
}


// Simple 2-poles resonant filter
void CSoundFile_SetupChannelFilter(CSoundFile *_this, MODCHANNEL *pChn, BOOL bReset, int flt_modifier)
//----------------------------------------------------------------------------------------
{
	float fc = (float)CSoundFile_CutOffToFrequency(_this, pChn->nCutOff, flt_modifier);
	float fs = (float)_this->gdwMixingFreq;
	float fg, fb0, fb1;

	fc *= (float)(2.0*3.14159265358/fs);
	float dmpfac = SDL_pow(10.0f, -((24.0f / 128.0f)*(float)pChn->nResonance) / 20.0f);
	float d = (1.0f-2.0f*dmpfac)* fc;
	if (d>2.0) d = 2.0;
	d = (2.0f*dmpfac - d)/fc;
	float e = SDL_pow(1.0f/fc,2.0f);

	fg=1/(1+d+e);
	fb0=(d+e+e)/(1+d+e);
	fb1=-e/(1+d+e);

	pChn->nFilter_A0 = (int)(fg * FILTER_PRECISION);
	pChn->nFilter_B0 = (int)(fb0 * FILTER_PRECISION);
	pChn->nFilter_B1 = (int)(fb1 * FILTER_PRECISION);

	if (bReset)
	{
		pChn->nFilter_Y1 = pChn->nFilter_Y2 = 0;
		pChn->nFilter_Y3 = pChn->nFilter_Y4 = 0;
	}
	pChn->dwFlags |= CHN_FILTER;
}

#endif // NO_FILTER
