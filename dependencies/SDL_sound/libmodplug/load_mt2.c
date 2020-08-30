#include "libmodplug.h"

#pragma pack(1)

typedef struct _MT2FILEHEADER
{
	DWORD dwMT20;	// 0x3032544D "MT20"
	DWORD dwSpecial;
	WORD wVersion;
	CHAR szTrackerName[32];	// "MadTracker 2.0"
	CHAR szSongName[64];
	WORD nOrders;
	WORD wRestart;
	WORD wPatterns;
	WORD wChannels;
	WORD wSamplesPerTick;
	BYTE bTicksPerLine;
	BYTE bLinesPerBeat;
	DWORD fulFlags; // b0=packed patterns
	WORD wInstruments;
	WORD wSamples;
	BYTE Orders[256];
} MT2FILEHEADER;

typedef struct _MT2PATTERN
{
	WORD wLines;
	DWORD wDataLen;
} MT2PATTERN;

typedef struct _MT2COMMAND
{
	BYTE note;	// 0=nothing, 97=note off
	BYTE instr;
	BYTE vol;
	BYTE pan;
	BYTE fxcmd;
	BYTE fxparam1;
	BYTE fxparam2;
} MT2COMMAND;

typedef struct _MT2DRUMSDATA
{
	WORD wDrumPatterns;
	WORD wDrumSamples[8];
	BYTE DrumPatternOrder[256];
} MT2DRUMSDATA;

typedef struct _MT2AUTOMATION
{
	DWORD dwFlags;
	DWORD dwEffectId;
	DWORD nEnvPoints;
} MT2AUTOMATION;

typedef struct _MT2INSTRUMENT
{
	DWORD dwDataLen;
	WORD wSamples;
	BYTE GroupsMapping[96];
	BYTE bVibType;
	BYTE bVibSweep;
	BYTE bVibDepth;
	BYTE bVibRate;
	WORD wFadeOut;
	WORD wNNA;
	WORD wInstrFlags;
	WORD wEnvFlags1;
	WORD wEnvFlags2;
} MT2INSTRUMENT;

typedef struct _MT2ENVELOPE
{
	BYTE nFlags;
	BYTE nPoints;
	BYTE nSustainPos;
	BYTE nLoopStart;
	BYTE nLoopEnd;
	BYTE bReserved[3];
	BYTE EnvData[64];
} MT2ENVELOPE;

typedef struct _MT2SYNTH
{
	BYTE nSynthId;
	BYTE nFxId;
	WORD wCutOff;
	BYTE nResonance;
	BYTE nAttack;
	BYTE nDecay;
	BYTE bReserved[25];
} MT2SYNTH;

typedef struct _MT2SAMPLE
{
	DWORD dwDataLen;
	DWORD dwLength;
	DWORD dwFrequency;
	BYTE nQuality;
	BYTE nChannels;
	BYTE nFlags;
	BYTE nLoop;
	DWORD dwLoopStart;
	DWORD dwLoopEnd;
	WORD wVolume;
	BYTE nPan;
	BYTE nBaseNote;
	WORD wSamplesPerBeat;
} MT2SAMPLE;

typedef struct _MT2GROUP
{
	BYTE nSmpNo;
	BYTE nVolume;	// 0-128
	BYTE nFinePitch;
	BYTE Reserved[5];
} MT2GROUP;

#pragma pack()


static VOID ConvertMT2Command(CSoundFile *that, MODCOMMAND *m, const MT2COMMAND *p)
//---------------------------------------------------------------------------
{
	// Note
	m->note = 0;
	if (p->note) m->note = (p->note > 96) ? 0xFF : p->note+12;
	// Instrument
	m->instr = p->instr;
	// Volume Column
	if ((p->vol >= 0x10) && (p->vol <= 0x90))
	{
		m->volcmd = VOLCMD_VOLUME;
		m->vol = (p->vol - 0x10) >> 1;
	} else
	if ((p->vol >= 0xA0) && (p->vol <= 0xAF))
	{
		m->volcmd = VOLCMD_VOLSLIDEDOWN;
		m->vol = (p->vol & 0x0f);
	} else
	if ((p->vol >= 0xB0) && (p->vol <= 0xBF))
	{
		m->volcmd = VOLCMD_VOLSLIDEUP;
		m->vol = (p->vol & 0x0f);
	} else
	if ((p->vol >= 0xC0) && (p->vol <= 0xCF))
	{
		m->volcmd = VOLCMD_FINEVOLDOWN;
		m->vol = (p->vol & 0x0f);
	} else
	if ((p->vol >= 0xD0) && (p->vol <= 0xDF))
	{
		m->volcmd = VOLCMD_FINEVOLUP;
		m->vol = (p->vol & 0x0f);
	} else
	{
		m->volcmd = 0;
		m->vol = 0;
	}
	// Effects
	m->command = 0;
	m->param = 0;
	if ((p->fxcmd) || (p->fxparam1) || (p->fxparam2))
	{
		if (!p->fxcmd)
		{
			m->command = p->fxparam2;
			m->param = p->fxparam1;
			CSoundFile_ConvertModCommand(that, m);
		} else
		{
			// TODO: MT2 Effects
		}
	}
}


BOOL CSoundFile_ReadMT2(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength)
//-----------------------------------------------------------
{
	const MT2FILEHEADER *pfh = (MT2FILEHEADER *)lpStream;
	DWORD dwMemPos, dwDrumDataPos, dwExtraDataPos;
	UINT nDrumDataLen, nExtraDataLen;
	const MT2DRUMSDATA *pdd;
	const MT2INSTRUMENT *InstrMap[255];
	const MT2SAMPLE *SampleMap[256];

	if ((!lpStream) || (dwMemLength < sizeof(MT2FILEHEADER))
	 || (pfh->dwMT20 != 0x3032544D)
	 || (pfh->wVersion < 0x0200) || (pfh->wVersion >= 0x0300)
	 || (pfh->wChannels < 4) || (pfh->wChannels > 64)) return FALSE;
	pdd = NULL;
	_this->m_nType = MOD_TYPE_MT2;
	_this->m_nChannels = pfh->wChannels;
	_this->m_nRestartPos = pfh->wRestart;
	_this->m_nDefaultSpeed = pfh->bTicksPerLine;
	_this->m_nDefaultTempo = 125;
	if ((pfh->wSamplesPerTick > 100) && (pfh->wSamplesPerTick < 5000))
	{
		_this->m_nDefaultTempo = 110250 / pfh->wSamplesPerTick;
	}
	for (UINT iOrd=0; iOrd<MAX_ORDERS; iOrd++)
	{
		_this->Order[iOrd] = (BYTE)((iOrd < pfh->nOrders) ? pfh->Orders[iOrd] : 0xFF);
	}
	dwMemPos = sizeof(MT2FILEHEADER);
	nDrumDataLen = *(WORD *)(lpStream + dwMemPos);
	dwDrumDataPos = dwMemPos + 2;
	if (nDrumDataLen >= 2) pdd = (MT2DRUMSDATA *)(lpStream+dwDrumDataPos);
	dwMemPos += 2 + nDrumDataLen;
	if (dwMemPos >= dwMemLength-12) return TRUE;
	if (!*(DWORD *)(lpStream+dwMemPos)) dwMemPos += 4;
	if (!*(DWORD *)(lpStream+dwMemPos)) dwMemPos += 4;
	nExtraDataLen = *(DWORD *)(lpStream+dwMemPos);
	dwExtraDataPos = dwMemPos + 4;
	dwMemPos += 4;
	if (dwMemPos + nExtraDataLen >= dwMemLength) return TRUE;
	while (dwMemPos+8 < dwExtraDataPos + nExtraDataLen)
	{
		DWORD dwId = *(DWORD *)(lpStream+dwMemPos);
		DWORD dwLen = *(DWORD *)(lpStream+dwMemPos+4);
		dwMemPos += 8;
		if (dwMemPos + dwLen > dwMemLength) return TRUE;
		switch(dwId)
		{
		// MSG
		case 0x0047534D:
			break;
		// SUM -> author name (or "Unregistered")
		// TMAP
		// TRKS
		case 0x534b5254:
			break;
		}
		dwMemPos += dwLen;
	}
	// Load Patterns
	dwMemPos = dwExtraDataPos + nExtraDataLen;
	for (UINT iPat=0; iPat<pfh->wPatterns; iPat++) if (dwMemPos < dwMemLength-6)
	{
		const MT2PATTERN *pmp = (MT2PATTERN *)(lpStream+dwMemPos);
		UINT wDataLen = (pmp->wDataLen + 1) & ~1;
		dwMemPos += 6;
		if (dwMemPos + wDataLen > dwMemLength) break;
		UINT nLines = pmp->wLines;
		if ((iPat < MAX_PATTERNS) && (nLines > 0) && (nLines <= 256))
		{
			_this->PatternSize[iPat] = nLines;
			_this->Patterns[iPat] = CSoundFile_AllocatePattern(nLines, _this->m_nChannels);
			if (!_this->Patterns[iPat]) return TRUE;
			MODCOMMAND *m = _this->Patterns[iPat];
			UINT len = wDataLen;
			if (pfh->fulFlags & 1) // Packed Patterns
			{
				BYTE *p = (BYTE *)(lpStream+dwMemPos);
				UINT pos = 0, row=0, ch=0;
				while (pos < len)
				{
					MT2COMMAND cmd;
					UINT infobyte = p[pos++];
					UINT rptcount = 0;
					if (infobyte == 0xff)
					{
						rptcount = p[pos++];
						infobyte = p[pos++];
					}
					if (infobyte & 0x7f)
					{
						UINT patpos = row*_this->m_nChannels+ch;
						cmd.note = cmd.instr = cmd.vol = cmd.pan = cmd.fxcmd = cmd.fxparam1 = cmd.fxparam2 = 0;
						if (infobyte & 1) cmd.note = p[pos++];
						if (infobyte & 2) cmd.instr = p[pos++];
						if (infobyte & 4) cmd.vol = p[pos++];
						if (infobyte & 8) cmd.pan = p[pos++];
						if (infobyte & 16) cmd.fxcmd = p[pos++];
						if (infobyte & 32) cmd.fxparam1 = p[pos++];
						if (infobyte & 64) cmd.fxparam2 = p[pos++];
						ConvertMT2Command(_this, &m[patpos], &cmd);
					}
					row += rptcount+1;
					while (row >= nLines) { row-=nLines; ch++; }
					if (ch >= _this->m_nChannels) break;
				}
			} else
			{
				const MT2COMMAND *p = (MT2COMMAND *)(lpStream+dwMemPos);
				UINT n = 0;
				while ((len > sizeof(MT2COMMAND)) && (n < _this->m_nChannels*nLines))
				{
					ConvertMT2Command(_this, m, p);
					len -= sizeof(MT2COMMAND);
					n++;
					p++;
					m++;
				}
			}
		}
		dwMemPos += wDataLen;
	}
	// Skip Drum Patterns
	if (pdd)
	{
		for (UINT iDrm=0; iDrm<pdd->wDrumPatterns; iDrm++)
		{
			if (dwMemPos > dwMemLength-2) return TRUE;
			UINT nLines = *(WORD *)(lpStream+dwMemPos);
			dwMemPos += 2 + nLines * 32;
		}
	}
	// Automation
	if (pfh->fulFlags & 2)
	{
		UINT nAutoCount = _this->m_nChannels;
		if (pfh->fulFlags & 0x10) nAutoCount++; // Master Automation
		if ((pfh->fulFlags & 0x08) && (pdd)) nAutoCount += 8; // Drums Automation
		nAutoCount *= pfh->wPatterns;
		for (UINT iAuto=0; iAuto<nAutoCount; iAuto++)
		{
			if (dwMemPos+12 >= dwMemLength) return TRUE;
			const MT2AUTOMATION *pma = (MT2AUTOMATION *)(lpStream+dwMemPos);
			dwMemPos += (pfh->wVersion <= 0x201) ? 4 : 8;
			for (UINT iEnv=0; iEnv<14; iEnv++)
			{
				if (pma->dwFlags & (1 << iEnv))
				{
					dwMemPos += 260;
				}
			}
		}
	}
	// Load Instruments
	SDL_memset(InstrMap, 0, sizeof(InstrMap));
	_this->m_nInstruments = (pfh->wInstruments < MAX_INSTRUMENTS) ? pfh->wInstruments : MAX_INSTRUMENTS-1;
	for (UINT iIns=1; iIns<=255; iIns++)
	{
		if (dwMemPos+36 > dwMemLength) return TRUE;
		const MT2INSTRUMENT *pmi = (MT2INSTRUMENT *)(lpStream+dwMemPos);
		INSTRUMENTHEADER *penv = NULL;
		if (iIns <= _this->m_nInstruments)
		{
			penv = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER));
			_this->Headers[iIns] = penv;
			if (penv)
			{
				SDL_memset(penv, 0, sizeof(INSTRUMENTHEADER));
				penv->nGlobalVol = 64;
				penv->nPan = 128;
				for (UINT i=0; i<NOTE_MAX; i++)
				{
					penv->NoteMap[i] = i+1;
				}
			}
		}
		if (((LONG)pmi->dwDataLen > 0) && (dwMemPos <= dwMemLength - 40) && (pmi->dwDataLen <= dwMemLength - (dwMemPos + 40)))
		{
			InstrMap[iIns-1] = pmi;
			if (penv)
			{
				penv->nFadeOut = pmi->wFadeOut;
				penv->nNNA = pmi->wNNA & 3;
				penv->nDCT = (pmi->wNNA>>8) & 3;
				penv->nDNA = (pmi->wNNA>>12) & 3;
				MT2ENVELOPE *pehdr[4];
				WORD *pedata[4];
				if (pfh->wVersion <= 0x201)
				{
					DWORD dwEnvPos = dwMemPos + sizeof(MT2INSTRUMENT) - 4;
					pehdr[0] = (MT2ENVELOPE *)(lpStream+dwEnvPos);
					pehdr[1] = (MT2ENVELOPE *)(lpStream+dwEnvPos+8);
					pehdr[2] = pehdr[3] = NULL;
					pedata[0] = (WORD *)(lpStream+dwEnvPos+16);
					pedata[1] = (WORD *)(lpStream+dwEnvPos+16+64);
					pedata[2] = pedata[3] = NULL;
				} else
				{
					DWORD dwEnvPos = dwMemPos + sizeof(MT2INSTRUMENT);
					for (UINT i=0; i<4; i++)
					{
						if (pmi->wEnvFlags1 & (1<<i))
						{
							pehdr[i] = (MT2ENVELOPE *)(lpStream+dwEnvPos);
							pedata[i] = (WORD *)pehdr[i]->EnvData;
							dwEnvPos += sizeof(MT2ENVELOPE);
						} else
						{
							pehdr[i] = NULL;
							pedata[i] = NULL;
						}
					}
				}
				// Load envelopes
				for (UINT iEnv=0; iEnv<4; iEnv++) if (pehdr[iEnv])
				{
					const MT2ENVELOPE *pme = pehdr[iEnv];
					WORD *pEnvPoints = NULL;
					BYTE *pEnvData = NULL;
					switch(iEnv)
					{
					// Volume Envelope
					case 0:
						if (pme->nFlags & 1) penv->dwFlags |= ENV_VOLUME;
						if (pme->nFlags & 2) penv->dwFlags |= ENV_VOLSUSTAIN;
						if (pme->nFlags & 4) penv->dwFlags |= ENV_VOLLOOP;
						penv->nVolEnv = (pme->nPoints > 16) ? 16 : pme->nPoints;
						penv->nVolSustainBegin = penv->nVolSustainEnd = pme->nSustainPos;
						penv->nVolLoopStart = pme->nLoopStart;
						penv->nVolLoopEnd = pme->nLoopEnd;
						pEnvPoints = penv->VolPoints;
						pEnvData = penv->VolEnv;
						break;

					// Panning Envelope
					case 1:
						if (pme->nFlags & 1) penv->dwFlags |= ENV_PANNING;
						if (pme->nFlags & 2) penv->dwFlags |= ENV_PANSUSTAIN;
						if (pme->nFlags & 4) penv->dwFlags |= ENV_PANLOOP;
						penv->nPanEnv = (pme->nPoints > 16) ? 16 : pme->nPoints;
						penv->nPanSustainBegin = penv->nPanSustainEnd = pme->nSustainPos;
						penv->nPanLoopStart = pme->nLoopStart;
						penv->nPanLoopEnd = pme->nLoopEnd;
						pEnvPoints = penv->PanPoints;
						pEnvData = penv->PanEnv;
						break;

					// Pitch/Filter envelope
					default:
						if (pme->nFlags & 1) penv->dwFlags |= (iEnv==3) ? (ENV_PITCH|ENV_FILTER) : ENV_PITCH;
						if (pme->nFlags & 2) penv->dwFlags |= ENV_PITCHSUSTAIN;
						if (pme->nFlags & 4) penv->dwFlags |= ENV_PITCHLOOP;
						penv->nPitchEnv = (pme->nPoints > 16) ? 16 : pme->nPoints;
						penv->nPitchSustainBegin = penv->nPitchSustainEnd = pme->nSustainPos;
						penv->nPitchLoopStart = pme->nLoopStart;
						penv->nPitchLoopEnd = pme->nLoopEnd;
						pEnvPoints = penv->PitchPoints;
						pEnvData = penv->PitchEnv;
					}
					// Envelope data
					if ((pEnvPoints) && (pEnvData) && (pedata[iEnv]))
					{
						WORD *psrc = pedata[iEnv];
						for (UINT i=0; i<16; i++)
						{
							pEnvPoints[i] = psrc[i*2];
							pEnvData[i] = (BYTE)psrc[i*2+1];
						}
					}
				}
			}
			dwMemPos += pmi->dwDataLen + 36;
			if (pfh->wVersion > 0x201) dwMemPos += 4; // ?
		} else
		{
			dwMemPos += 36;
		}
	}
	SDL_memset(SampleMap, 0, sizeof(SampleMap));
	_this->m_nSamples = (pfh->wSamples < MAX_SAMPLES) ? pfh->wSamples : MAX_SAMPLES-1;
	for (UINT iSmp=1; iSmp<=256; iSmp++)
	{
		if (dwMemPos+36 > dwMemLength) return TRUE;
		const MT2SAMPLE *pms = (MT2SAMPLE *)(lpStream+dwMemPos);
		if (pms->dwDataLen > 0)
		{
			SampleMap[iSmp-1] = pms;
			if (iSmp < MAX_SAMPLES)
			{
				MODINSTRUMENT *psmp = &_this->Ins[iSmp];
				psmp->nGlobalVol = 64;
				psmp->nVolume = (pms->wVolume >> 7);
				psmp->nPan = (pms->nPan == 0x80) ? 128 : (pms->nPan^0x80);
				psmp->nLength = pms->dwLength;
				psmp->nC4Speed = pms->dwFrequency;
				psmp->nLoopStart = pms->dwLoopStart;
				psmp->nLoopEnd = pms->dwLoopEnd;
				CSoundFile_FrequencyToTransposeInstrument(psmp);
				psmp->RelativeTone -= pms->nBaseNote - 49;
				psmp->nC4Speed = CSoundFile_TransposeToFrequency(psmp->RelativeTone, psmp->nFineTune);
				if (pms->nQuality == 2) { psmp->uFlags |= CHN_16BIT; psmp->nLength >>= 1; }
				if (pms->nChannels == 2) { psmp->nLength >>= 1; }
				if (pms->nLoop == 1) psmp->uFlags |= CHN_LOOP;
				if (pms->nLoop == 2) psmp->uFlags |= CHN_LOOP|CHN_PINGPONGLOOP;
			}
			dwMemPos += pms->dwDataLen + 36;
		} else
		{
			dwMemPos += 36;
		}
	}
	for (UINT iMap=0; iMap<255; iMap++) if (InstrMap[iMap])
	{
		if (dwMemPos+8 > dwMemLength) return TRUE;
		const MT2INSTRUMENT *pmi = InstrMap[iMap];
		INSTRUMENTHEADER *penv = NULL;
		if (iMap<_this->m_nInstruments) penv = _this->Headers[iMap+1];
		for (UINT iGrp=0; iGrp<pmi->wSamples; iGrp++)
		{
			if (penv)
			{
				const MT2GROUP *pmg = (MT2GROUP *)(lpStream+dwMemPos);
				for (UINT i=0; i<96; i++)
				{
					if (pmi->GroupsMapping[i] == iGrp)
					{
						UINT nSmp = pmg->nSmpNo+1;
						penv->Keyboard[i+12] = (BYTE)nSmp;
						if (nSmp <= _this->m_nSamples)
						{
							_this->Ins[nSmp].nVibType = pmi->bVibType;
							_this->Ins[nSmp].nVibSweep = pmi->bVibSweep;
							_this->Ins[nSmp].nVibDepth = pmi->bVibDepth;
							_this->Ins[nSmp].nVibRate = pmi->bVibRate;
						}
					}
				}
			}
			dwMemPos += 8;
		}
	}
	for (UINT iData=0; iData<256; iData++) if ((iData < _this->m_nSamples) && (SampleMap[iData]))
	{
		const MT2SAMPLE *pms = SampleMap[iData];
		MODINSTRUMENT *psmp = &_this->Ins[iData+1];
		if (!(pms->nFlags & 5))
		{
			if (psmp->nLength > 0) 
			{
				UINT rsflags;
				
				if (pms->nChannels == 2)
					rsflags = (psmp->uFlags & CHN_16BIT) ? RS_STPCM16D : RS_STPCM8D;
				else
					rsflags = (psmp->uFlags & CHN_16BIT) ? RS_PCM16D : RS_PCM8D;

				dwMemPos += CSoundFile_ReadSample(_this, psmp, rsflags, (LPCSTR)(lpStream+dwMemPos), dwMemLength-dwMemPos);
			}
		} else
		if (dwMemPos+4 < dwMemLength)
		{
			UINT nNameLen = *(DWORD *)(lpStream+dwMemPos);
			dwMemPos += nNameLen + 16;
		}
		if (dwMemPos+4 >= dwMemLength) break;
	}
	return TRUE;
}

