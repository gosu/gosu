/*
 * This source code is public domain.
 *
 * Authors: Rani Assaf <rani@magic.metawire.com>,
 *          Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
 */

#ifndef _INCL_LIBMODPLUG_H_
#define _INCL_LIBMODPLUG_H_

#include "SDL.h"

#if (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#pragma GCC visibility push(hidden)
#endif

#ifdef _WIN32

#ifdef MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4514)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>

#define srandom(_seed)  srand(_seed)
#define random()        rand()

#else

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef Sint8 CHAR;
typedef Uint8 UCHAR;
typedef Uint8* PUCHAR;
typedef Uint16 USHORT;
typedef Uint32 ULONG;
typedef Uint32 UINT;
typedef Uint32 DWORD;
typedef Sint32 LONG;
typedef Sint64 LONGLONG;
typedef Sint32* LPLONG;
typedef Uint32* LPDWORD;
typedef Uint16 WORD;
typedef Uint8 BYTE;
typedef Uint8* LPBYTE;
typedef int BOOL;
typedef char* LPSTR;
typedef void* LPVOID;
typedef Uint16* LPWORD;
typedef const char* LPCSTR;
typedef void* PVOID;
typedef void VOID;

inline LONG MulDiv (long a, long b, long c)
{
  // if (!c) return 0;
  return ((uint64_t) a * (uint64_t) b ) / c;
}

#define LPCTSTR LPCSTR
#define WAVE_FORMAT_PCM 1
//#define ENABLE_EQ

#define  GHND   0

#define GlobalAllocPtr(x, size) ((int8_t *) SDL_calloc(1, (size)))
#define GlobalFreePtr(p) SDL_free((void *)(p))

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#endif // _WIN32


typedef struct {
	char *mm;
	int sz;
	int pos;
	int error;
} MMFILE;

void mmfclose(MMFILE *mmfile);
int mmfeof(MMFILE *mmfile);
int mmfgetc(MMFILE *mmfile);
void mmfgets(char buf[], unsigned int bufsz, MMFILE *mmfile);
long mmftell(MMFILE *mmfile);
void mmfseek(MMFILE *mmfile, long p, int whence);
BYTE mmreadUBYTE(MMFILE *mmfile);
void mmreadUBYTES(BYTE *buf, long sz, MMFILE *mmfile);
void mmreadSBYTES(char *buf, long sz, MMFILE *mmfile);
char *rwops_fgets(char *buf, int buflen, SDL_RWops *rwops);

#define MMSTREAM					SDL_RWops
#define _mm_fopen(name,mode)		SDL_RWFromFile(name, mode)
#define _mm_fgets(f,buf,sz)			rwops_fgets(buf,sz,f)
#define _mm_fseek(f,pos,whence)		SDL_RWseek(f,pos,whence)
#define _mm_ftell(f)				SDL_RWtell(f)
#define _mm_read_UBYTES(buf,sz,f)	SDL_RWread(f, buf, 1, sz)
#define _mm_read_SBYTES(buf,sz,f)	SDL_RWread(f, buf, 1, sz)
#define _mm_feof(f)					(SDL_RWtell(f) >= SDL_RWsize(f))
#define _mm_fclose(f)				SDL_RWclose(f)
#define DupStr(h,buf,sz)			SDL_strdup(buf)
#define _mm_calloc(h,n,sz)			SDL_calloc(n,sz)
#define _mm_recalloc(h,buf,sz,elsz)	SDL_realloc(buf,sz)
#define _mm_free(h,p)				SDL_free(p)


#define MODPLUG_EXPORT



/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (endian and char fixes for PPC)
*/

#ifndef LPCBYTE
typedef const BYTE * LPCBYTE;
#endif

#define MOD_AMIGAC2			0x1AB
#define MAX_SAMPLE_LENGTH	16000000
#define MAX_SAMPLE_RATE		192000
#define MAX_ORDERS			256
#define MAX_PATTERNS		240
#define MAX_SAMPLES			240
#define MAX_INSTRUMENTS		MAX_SAMPLES
#define MAX_CHANNELS		128
#define MAX_BASECHANNELS	64
#define MAX_ENVPOINTS		32
#define MIN_PERIOD			0x0020
#define MAX_PERIOD			0xFFFF
#define MAX_PATTERNNAME		32
#define MAX_CHANNELNAME		20
#define MAX_INFONAME		80
#define MAX_EQ_BANDS		6
#define MAX_MIXPLUGINS		8


#define MOD_TYPE_NONE		0x00
#define MOD_TYPE_MOD		0x01
#define MOD_TYPE_S3M		0x02
#define MOD_TYPE_XM			0x04
#define MOD_TYPE_MED		0x08
#define MOD_TYPE_MTM		0x10
#define MOD_TYPE_IT			0x20
#define MOD_TYPE_669		0x40
#define MOD_TYPE_ULT		0x80
#define MOD_TYPE_STM		0x100
#define MOD_TYPE_FAR		0x200
#define MOD_TYPE_WAV		0x400
#define MOD_TYPE_AMF		0x800
#define MOD_TYPE_AMS		0x1000
#define MOD_TYPE_DSM		0x2000
#define MOD_TYPE_MDL		0x4000
#define MOD_TYPE_OKT		0x8000
#define MOD_TYPE_MID		0x10000
#define MOD_TYPE_DMF		0x20000
#define MOD_TYPE_PTM		0x40000
#define MOD_TYPE_DBM		0x80000
#define MOD_TYPE_MT2		0x100000
#define MOD_TYPE_AMF0		0x200000
#define MOD_TYPE_PSM		0x400000
#define MOD_TYPE_J2B		0x800000
#define MOD_TYPE_ABC		0x1000000
#define MOD_TYPE_PAT		0x2000000
#define MOD_TYPE_UMX		0x80000000 // Fake type
#define MAX_MODTYPE		24



// Channel flags:
// Bits 0-7:	Sample Flags
#define CHN_16BIT               0x01
#define CHN_LOOP                0x02
#define CHN_PINGPONGLOOP        0x04
#define CHN_SUSTAINLOOP         0x08
#define CHN_PINGPONGSUSTAIN     0x10
#define CHN_PANNING             0x20
#define CHN_STEREO              0x40
#define CHN_PINGPONGFLAG	0x80
// Bits 8-31:	Channel Flags
#define CHN_MUTE                0x100
#define CHN_KEYOFF              0x200
#define CHN_NOTEFADE		0x400
#define CHN_SURROUND            0x800
#define CHN_NOIDO               0x1000
#define CHN_HQSRC               0x2000
#define CHN_FILTER              0x4000
#define CHN_VOLUMERAMP		0x8000
#define CHN_VIBRATO             0x10000
#define CHN_TREMOLO             0x20000
#define CHN_PANBRELLO		0x40000
#define CHN_PORTAMENTO		0x80000
#define CHN_GLISSANDO		0x100000
#define CHN_VOLENV              0x200000
#define CHN_PANENV              0x400000
#define CHN_PITCHENV		0x800000
#define CHN_FASTVOLRAMP		0x1000000
#define CHN_EXTRALOUD		0x2000000
#define CHN_REVERB              0x4000000
#define CHN_NOREVERB		0x8000000


#define ENV_VOLUME              0x0001
#define ENV_VOLSUSTAIN		0x0002
#define ENV_VOLLOOP             0x0004
#define ENV_PANNING             0x0008
#define ENV_PANSUSTAIN		0x0010
#define ENV_PANLOOP             0x0020
#define ENV_PITCH               0x0040
#define ENV_PITCHSUSTAIN	0x0080
#define ENV_PITCHLOOP		0x0100
#define ENV_SETPANNING		0x0200
#define ENV_FILTER              0x0400
#define ENV_VOLCARRY		0x0800
#define ENV_PANCARRY		0x1000
#define ENV_PITCHCARRY		0x2000

#define CMD_NONE                        0
#define CMD_ARPEGGIO			1
#define CMD_PORTAMENTOUP		2
#define CMD_PORTAMENTODOWN		3
#define CMD_TONEPORTAMENTO		4
#define CMD_VIBRATO                     5
#define CMD_TONEPORTAVOL		6
#define CMD_VIBRATOVOL			7
#define CMD_TREMOLO                     8
#define CMD_PANNING8			9
#define CMD_OFFSET                      10
#define CMD_VOLUMESLIDE			11
#define CMD_POSITIONJUMP		12
#define CMD_VOLUME                      13
#define CMD_PATTERNBREAK		14
#define CMD_RETRIG                      15
#define CMD_SPEED                       16
#define CMD_TEMPO                       17
#define CMD_TREMOR                      18
#define CMD_MODCMDEX			19
#define CMD_S3MCMDEX			20
#define CMD_CHANNELVOLUME		21
#define CMD_CHANNELVOLSLIDE		22
#define CMD_GLOBALVOLUME		23
#define CMD_GLOBALVOLSLIDE		24
#define CMD_KEYOFF                      25
#define CMD_FINEVIBRATO			26
#define CMD_PANBRELLO			27
#define CMD_XFINEPORTAUPDOWN            28
#define CMD_PANNINGSLIDE		29
#define CMD_SETENVPOSITION		30
#define CMD_MIDI                        31


// Volume Column commands
#define VOLCMD_VOLUME			1
#define VOLCMD_PANNING			2
#define VOLCMD_VOLSLIDEUP		3
#define VOLCMD_VOLSLIDEDOWN		4
#define VOLCMD_FINEVOLUP		5
#define VOLCMD_FINEVOLDOWN		6
#define VOLCMD_VIBRATOSPEED		7
#define VOLCMD_VIBRATO			8
#define VOLCMD_PANSLIDELEFT		9
#define VOLCMD_PANSLIDERIGHT	        10
#define VOLCMD_TONEPORTAMENTO	        11
#define VOLCMD_PORTAUP			12
#define VOLCMD_PORTADOWN		13

#define RSF_16BIT		0x04
#define RSF_STEREO		0x08

#define RS_PCM8S		0	// 8-bit signed
#define RS_PCM8U		1	// 8-bit unsigned
#define RS_PCM8D		2	// 8-bit delta values
#define RS_ADPCM4		3	// 4-bit ADPCM-packed
#define RS_PCM16D		4	// 16-bit delta values
#define RS_PCM16S		5	// 16-bit signed
#define RS_PCM16U		6	// 16-bit unsigned
#define RS_PCM16M		7	// 16-bit motorola order
#define RS_STPCM8S		(RS_PCM8S|RSF_STEREO)  // stereo 8-bit signed
#define RS_STPCM8U		(RS_PCM8U|RSF_STEREO)  // stereo 8-bit unsigned
#define RS_STPCM8D		(RS_PCM8D|RSF_STEREO)  // stereo 8-bit delta values
#define RS_STPCM16S		(RS_PCM16S|RSF_STEREO) // stereo 16-bit signed
#define RS_STPCM16U		(RS_PCM16U|RSF_STEREO) // stereo 16-bit unsigned
#define RS_STPCM16D		(RS_PCM16D|RSF_STEREO) // stereo 16-bit delta values
#define RS_STPCM16M		(RS_PCM16M|RSF_STEREO) // stereo 16-bit signed big endian
// IT 2.14 compressed samples
#define RS_IT2148		0x10
#define RS_IT21416		0x14
#define RS_IT2158		0x12
#define RS_IT21516		0x16
// AMS Packed Samples
#define RS_AMS8			0x11
#define RS_AMS16		0x15
// DMF Huffman compression
#define RS_DMF8			0x13
#define RS_DMF16		0x17
// MDL Huffman compression
#define RS_MDL8			0x20
#define RS_MDL16		0x24
#define RS_PTM8DTO16	0x25
// Stereo Interleaved Samples
#define RS_STIPCM8S		(RS_PCM8S|0x40|RSF_STEREO)	// stereo 8-bit signed
#define RS_STIPCM8U		(RS_PCM8U|0x40|RSF_STEREO)	// stereo 8-bit unsigned
#define RS_STIPCM16S	(RS_PCM16S|0x40|RSF_STEREO)	// stereo 16-bit signed
#define RS_STIPCM16U	(RS_PCM16U|0x40|RSF_STEREO)	// stereo 16-bit unsigned
#define RS_STIPCM16M	(RS_PCM16M|0x40|RSF_STEREO)	// stereo 16-bit signed big endian
// 24-bit signed
#define RS_PCM24S		(RS_PCM16S|0x80)			// mono 24-bit signed
#define RS_STIPCM24S	(RS_PCM16S|0x80|RSF_STEREO)	// stereo 24-bit signed
#define RS_PCM32S		(RS_PCM16S|0xC0)			// mono 24-bit signed
#define RS_STIPCM32S	(RS_PCM16S|0xC0|RSF_STEREO)	// stereo 24-bit signed

// NNA types
#define NNA_NOTECUT		0
#define NNA_CONTINUE	1
#define NNA_NOTEOFF		2
#define NNA_NOTEFADE	3

// DCT types
#define DCT_NONE		0
#define DCT_NOTE		1
#define DCT_SAMPLE		2
#define DCT_INSTRUMENT	3

// DNA types
#define DNA_NOTECUT		0
#define DNA_NOTEOFF		1
#define DNA_NOTEFADE	2

// Mixer Hardware-Dependent features
#define SYSMIX_WINDOWSNT	0x02
#define SYSMIX_SLOWCPU		0x04
#define SYSMIX_FASTCPU		0x08

// Module flags
#define SONG_EMBEDMIDICFG	0x0001
#define SONG_FASTVOLSLIDES	0x0002
#define SONG_ITOLDEFFECTS	0x0004
#define SONG_ITCOMPATMODE	0x0008
#define SONG_LINEARSLIDES	0x0010
#define SONG_PATTERNLOOP	0x0020
#define SONG_STEP			0x0040
#define SONG_PAUSED			0x0080
#define SONG_FADINGSONG		0x0100
#define SONG_ENDREACHED		0x0200
#define SONG_GLOBALFADE		0x0400
#define SONG_CPUVERYHIGH	0x0800
#define SONG_FIRSTTICK		0x1000
#define SONG_MPTFILTERMODE	0x2000
#define SONG_SURROUNDPAN	0x4000
#define SONG_EXFILTERRANGE	0x8000
#define SONG_AMIGALIMITS	0x10000

// Global Options (Renderer)
#define SNDMIX_REVERSESTEREO	0x0001
#define SNDMIX_NOISEREDUCTION	0x0002
#define SNDMIX_AGC				0x0004
#define SNDMIX_NORESAMPLING		0x0008
#define SNDMIX_HQRESAMPLER		0x0010
#define SNDMIX_MEGABASS			0x0020
#define SNDMIX_SURROUND			0x0040
#define SNDMIX_REVERB			0x0080
#define SNDMIX_EQ				0x0100
#define SNDMIX_SOFTPANNING		0x0200
#define SNDMIX_ULTRAHQSRCMODE	0x0400
// Misc Flags (can safely be turned on or off)
#define SNDMIX_DIRECTTODISK		0x10000
#define SNDMIX_NOBACKWARDJUMPS	0x40000
#define SNDMIX_MAXDEFAULTPAN	0x80000	// Used by the MOD loader

// Delayed Surround Filters
#define nDolbyHiFltAttn		6
#define nDolbyHiFltMask		3
#define DOLBYATTNROUNDUP	31

// Bass Expansion
#define XBASS_DELAY			14	// 2.5 ms

// Buffer Sizes
#define XBASSBUFFERSIZE		64		// 2 ms at 50KHz
#define FILTERBUFFERSIZE	64		// 1.25 ms
#define SURROUNDBUFFERSIZE	((MAX_SAMPLE_RATE * 50) / 1000)
#define REVERBBUFFERSIZE	((MAX_SAMPLE_RATE * 200) / 1000)
#define REVERBBUFFERSIZE2	((REVERBBUFFERSIZE*13) / 17)
#define REVERBBUFFERSIZE3	((REVERBBUFFERSIZE*7) / 13)
#define REVERBBUFFERSIZE4	((REVERBBUFFERSIZE*7) / 19)

#define MIXBUFFERSIZE		512
#define MIXING_ATTENUATION	4
#define MIXING_CLIPMIN		(-0x08000000)
#define MIXING_CLIPMAX		(0x07FFFFFF)
#define VOLUMERAMPPRECISION	12
#define FADESONGDELAY		100
#define EQ_BUFFERSIZE		(MIXBUFFERSIZE)
#define AGC_PRECISION		9
#define AGC_UNITY			(1 << AGC_PRECISION)

#define MPPASMCALL
#define MPPFASTCALL

#define MOD2XMFineTune(k)	((int)( (signed char)((k)<<4) ))
#define XM2MODFineTune(k)	((int)( (k>>4)&0x0f ))

int _muldiv(long a, long b, long c);
int _muldivr(long a, long b, long c);

#define bswapLE16(X) SDL_SwapLE16((X))
#define bswapLE32(X) SDL_SwapLE32((X))
#define bswapBE16(X) SDL_SwapBE16((X))
#define bswapBE32(X) SDL_SwapBE32((X))

// Reverb Types (GM2 Presets)
enum {
	REVERBTYPE_SMALLROOM,
	REVERBTYPE_MEDIUMROOM,
	REVERBTYPE_LARGEROOM,
	REVERBTYPE_SMALLHALL,
	REVERBTYPE_MEDIUMHALL,
	REVERBTYPE_LARGEHALL,
	NUM_REVERBTYPES
};


enum {
	SRCMODE_NEAREST,
	SRCMODE_LINEAR,
	SRCMODE_SPLINE,
	SRCMODE_POLYPHASE,
	NUM_SRC_MODES
};


// Sample Struct
typedef struct _MODINSTRUMENT
{
	UINT nLength,nLoopStart,nLoopEnd;
	UINT nSustainStart, nSustainEnd;
	signed char *pSample;
	UINT nC4Speed;
	WORD nPan;
	WORD nVolume;
	WORD nGlobalVol;
	WORD uFlags;
	signed char RelativeTone;
	signed char nFineTune;
	BYTE nVibType;
	BYTE nVibSweep;
	BYTE nVibDepth;
	BYTE nVibRate;
} MODINSTRUMENT;


// Instrument Struct
typedef struct _INSTRUMENTHEADER
{
	UINT nFadeOut;
	DWORD dwFlags;
	WORD nGlobalVol;
	WORD nPan;
	WORD VolPoints[MAX_ENVPOINTS];
	WORD PanPoints[MAX_ENVPOINTS];
	WORD PitchPoints[MAX_ENVPOINTS];
	BYTE VolEnv[MAX_ENVPOINTS];
	BYTE PanEnv[MAX_ENVPOINTS];
	BYTE PitchEnv[MAX_ENVPOINTS];
	BYTE Keyboard[128];
	BYTE NoteMap[128];

	BYTE nVolEnv;
	BYTE nPanEnv;
	BYTE nPitchEnv;
	BYTE nVolLoopStart;
	BYTE nVolLoopEnd;
	BYTE nVolSustainBegin;
	BYTE nVolSustainEnd;
	BYTE nPanLoopStart;
	BYTE nPanLoopEnd;
	BYTE nPanSustainBegin;
	BYTE nPanSustainEnd;
	BYTE nPitchLoopStart;
	BYTE nPitchLoopEnd;
	BYTE nPitchSustainBegin;
	BYTE nPitchSustainEnd;
	BYTE nNNA;
	BYTE nDCT;
	BYTE nDNA;
	BYTE nPanSwing;
	BYTE nVolSwing;
	BYTE nIFC;
	BYTE nIFR;
	WORD wMidiBank;
	BYTE nMidiProgram;
	BYTE nMidiChannel;
	BYTE nMidiDrumKey;
	signed char nPPS;
	unsigned char nPPC;
	CHAR filename[12];
} INSTRUMENTHEADER;


// Channel Struct
typedef struct _MODCHANNEL
{
	// First 32-bytes: Most used mixing information: don't change it
	signed char * pCurrentSample;
	DWORD nPos;
	DWORD nPosLo;	// actually 16-bit
	LONG nInc;		// 16.16
	LONG nRightVol;
	LONG nLeftVol;
	LONG nRightRamp;
	LONG nLeftRamp;
	// 2nd cache line
	DWORD nLength;
	DWORD dwFlags;
	DWORD nLoopStart;
	DWORD nLoopEnd;
	LONG nRampRightVol;
	LONG nRampLeftVol;
	LONG nFilter_Y1, nFilter_Y2, nFilter_Y3, nFilter_Y4;
	LONG nFilter_A0, nFilter_B0, nFilter_B1;
	LONG nROfs, nLOfs;
	LONG nRampLength;
	// Information not used in the mixer
	signed char * pSample;
	LONG nNewRightVol, nNewLeftVol;
	LONG nRealVolume, nRealPan;
	LONG nVolume, nPan, nFadeOutVol;
	LONG nPeriod, nC4Speed, nPortamentoDest;
	INSTRUMENTHEADER *pHeader;
	MODINSTRUMENT *pInstrument;
	DWORD nVolEnvPosition, nPanEnvPosition, nPitchEnvPosition;
	DWORD nMasterChn;
	LONG nGlobalVol, nInsVol;
	LONG nFineTune, nTranspose;
	LONG nPortamentoSlide, nAutoVibDepth;
	UINT nAutoVibPos, nVibratoPos, nTremoloPos, nPanbrelloPos;
	// 16-bit members
	signed short nVolSwing, nPanSwing;
	// 8-bit members
	BYTE nNote, nNNA;
	BYTE nNewNote, nNewIns, nCommand, nArpeggio;
	BYTE nOldVolumeSlide, nOldFineVolUpDown;
	BYTE nOldPortaUpDown, nOldFinePortaUpDown;
	BYTE nOldPanSlide, nOldChnVolSlide;
	BYTE nVibratoType, nVibratoSpeed, nVibratoDepth;
	BYTE nTremoloType, nTremoloSpeed, nTremoloDepth;
	BYTE nPanbrelloType, nPanbrelloSpeed, nPanbrelloDepth;
	BYTE nOldCmdEx, nOldVolParam, nOldTempo;
	BYTE nOldOffset, nOldHiOffset;
	BYTE nCutOff, nResonance;
	BYTE nRetrigCount, nRetrigParam;
	BYTE nTremorCount, nTremorParam;
	BYTE nPatternLoop, nPatternLoopCount;
	BYTE nRowNote, nRowInstr;
	BYTE nRowVolCmd, nRowVolume;
	BYTE nRowCommand, nRowParam;
	BYTE nActiveMacro, nPadding;
} MODCHANNEL;


typedef struct _MODCHANNELSETTINGS
{
	UINT nPan;
	UINT nVolume;
	DWORD dwFlags;
	UINT nMixPlugin;
} MODCHANNELSETTINGS;


typedef struct _MODCOMMAND
{
	BYTE note;
	BYTE instr;
	BYTE volcmd;
	BYTE command;
	BYTE vol;
	BYTE param;
} MODCOMMAND, *LPMODCOMMAND;

////////////////////////////////////////////////////////////////////

enum {
	MIDIOUT_START=0,
	MIDIOUT_STOP,
	MIDIOUT_TICK,
	MIDIOUT_NOTEON,
	MIDIOUT_NOTEOFF,
	MIDIOUT_VOLUME,
	MIDIOUT_PAN,
	MIDIOUT_BANKSEL,
	MIDIOUT_PROGRAM,
};


typedef struct MODMIDICFG
{
        char szMidiGlb[9*32];      // changed from CHAR
        char szMidiSFXExt[16*32];  // changed from CHAR
        char szMidiZXXExt[128*32]; // changed from CHAR
} MODMIDICFG, *LPMODMIDICFG;

#define NOTE_MAX                        120 //Defines maximum notevalue as well as maximum number of notes.

typedef struct CSoundFile
{
	MODCHANNEL Chn[MAX_CHANNELS];					// Channels
	UINT ChnMix[MAX_CHANNELS];						// Channels to be mixed
	MODINSTRUMENT Ins[MAX_SAMPLES];					// Instruments
	INSTRUMENTHEADER *Headers[MAX_INSTRUMENTS];		// Instrument Headers
	MODCHANNELSETTINGS ChnSettings[MAX_BASECHANNELS]; // Channels settings
	MODCOMMAND *Patterns[MAX_PATTERNS];				// Patterns
	WORD PatternSize[MAX_PATTERNS];					// Patterns Lengths
	BYTE Order[MAX_ORDERS];							// Pattern Orders
	MODMIDICFG m_MidiCfg;							// Midi macro config table
	UINT m_nDefaultSpeed, m_nDefaultTempo, m_nDefaultGlobalVolume;
	DWORD m_dwSongFlags;							// Song flags SONG_XXXX
	UINT m_nChannels, m_nMixChannels, m_nMixStat, m_nBufferCount;
	UINT m_nType, m_nSamples, m_nInstruments;
	UINT m_nTickCount, m_nTotalCount, m_nPatternDelay, m_nFrameDelay;
	UINT m_nMusicSpeed, m_nMusicTempo;
	UINT m_nNextRow, m_nRow;
	UINT m_nPattern,m_nCurrentPattern,m_nNextPattern,m_nRestartPos;
	UINT m_nMasterVolume, m_nGlobalVolume, m_nSongPreAmp;
	UINT m_nFreqFactor, m_nTempoFactor, m_nOldGlbVolSlide;
	LONG m_nMinPeriod, m_nMaxPeriod, m_nRepeatCount, m_nInitialRepeatCount;
	DWORD m_nGlobalFadeSamples, m_nGlobalFadeMaxSamples;
	UINT m_nMaxOrderPosition;
	UINT m_nPatternNames;
	LPSTR m_lpszPatternNames;
	CHAR CompressionTable[16];
	UINT m_nXBassDepth, m_nXBassRange;
	UINT m_nReverbDepth, m_nReverbDelay;
	UINT m_nProLogicDepth, m_nProLogicDelay;
	UINT m_nStereoSeparation;
	UINT m_nMaxMixChannels;
	DWORD gdwSoundSetup, gdwMixingFreq, gnBitsPerSample, gnChannels;
	UINT gnVolumeRampSamples;
    UINT gSampleSize;
    int MixSoundBuffer[MIXBUFFERSIZE*4];
    #ifndef MODPLUG_NO_REVERB
    int MixReverbBuffer[MIXBUFFERSIZE*2];
    UINT gnReverbSend;
    #endif
    int MixRearBuffer[MIXBUFFERSIZE*2];
    float MixFloatBuffer[MIXBUFFERSIZE*2];

    // Bass Expansion: low-pass filter
    LONG nXBassSum;
    LONG nXBassBufferPos;
    LONG nXBassDlyPos;
    LONG nXBassMask;

    // Noise Reduction: simple low-pass filter
    LONG nLeftNR;
    LONG nRightNR;

    // Surround Encoding: 1 delay line + low-pass filter + high-pass filter
    LONG nSurroundSize;
    LONG nSurroundPos;
    LONG nDolbyDepth;
    LONG nDolbyLoDlyPos;
    LONG nDolbyLoFltPos;
    LONG nDolbyLoFltSum;
    LONG nDolbyHiFltPos;
    LONG nDolbyHiFltSum;

    // Reverb: 4 delay lines + high-pass filter + low-pass filter
    #ifndef MODPLUG_NO_REVERB
    LONG nReverbSize;
    LONG nReverbBufferPos;
    LONG nReverbSize2;
    LONG nReverbBufferPos2;
    LONG nReverbSize3;
    LONG nReverbBufferPos3;
    LONG nReverbSize4;
    LONG nReverbBufferPos4;
    LONG nReverbLoFltSum;
    LONG nReverbLoFltPos;
    LONG nReverbLoDlyPos;
    LONG nFilterAttn;
    LONG gRvbLowPass[8];
    LONG gRvbLPPos;
    LONG gRvbLPSum;
    LONG ReverbLoFilterBuffer[XBASSBUFFERSIZE];
    LONG ReverbLoFilterDelay[XBASSBUFFERSIZE];
    LONG ReverbBuffer[REVERBBUFFERSIZE];
    LONG ReverbBuffer2[REVERBBUFFERSIZE2];
    LONG ReverbBuffer3[REVERBBUFFERSIZE3];
    LONG ReverbBuffer4[REVERBBUFFERSIZE4];
    #endif

    LONG XBassBuffer[XBASSBUFFERSIZE];
    LONG XBassDelay[XBASSBUFFERSIZE];
    LONG DolbyLoFilterBuffer[XBASSBUFFERSIZE];
    LONG DolbyLoFilterDelay[XBASSBUFFERSIZE];
    LONG DolbyHiFilterBuffer[FILTERBUFFERSIZE];
    LONG SurroundBuffer[SURROUNDBUFFERSIZE];
    LONG gnDryROfsVol;
    LONG gnDryLOfsVol;
    LONG gnRvbROfsVol;
    LONG gnRvbLOfsVol;
    int gbInitPlugins;
} CSoundFile;

typedef struct _ModPlug_Settings ModPlug_Settings;
CSoundFile *new_CSoundFile(LPCBYTE lpStream, DWORD dwMemLength, const ModPlug_Settings *settings);
void delete_CSoundFile(CSoundFile *_this);

	UINT CSoundFile_GetMaxPosition(CSoundFile *_this);
	void CSoundFile_SetCurrentPos(CSoundFile *_this, UINT nPos);
	DWORD CSoundFile_GetLength(CSoundFile *_this, BOOL bAdjust, BOOL bTotal);
	void CSoundFile_SetRepeatCount(CSoundFile *_this, int n);
	BOOL CSoundFile_SetPatternName(CSoundFile *_this, UINT nPat, LPCSTR lpszName);
	// Module Loaders
	BOOL CSoundFile_ReadXM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadS3M(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadMod(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadMed(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadMTM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadSTM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadIT(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_Read669(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadUlt(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadDSM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadFAR(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadAMS(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadAMS2(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadMDL(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadOKT(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadDMF(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadPTM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadDBM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadAMF(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadMT2(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadPSM(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadJ2B(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadUMX(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadABC(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadMID(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_ReadPAT(CSoundFile *_this, LPCBYTE lpStream, DWORD dwMemLength);

	BOOL CSoundFile_TestABC(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_TestMID(LPCBYTE lpStream, DWORD dwMemLength);
	BOOL CSoundFile_TestPAT(LPCBYTE lpStream, DWORD dwMemLength);

	// MOD Convert function
	void CSoundFile_ConvertModCommand(CSoundFile *_this, MODCOMMAND *);
	void CSoundFile_S3MConvert(MODCOMMAND *m, BOOL bIT);

	// Real-time sound functions
	UINT CSoundFile_Read(CSoundFile *_this, LPVOID lpBuffer, UINT cbBuffer);
	UINT CSoundFile_CreateStereoMix(CSoundFile *_this, int count);
	BOOL CSoundFile_FadeSong(CSoundFile *_this, UINT msec);
	BOOL CSoundFile_GlobalFadeSong(CSoundFile *_this, UINT msec);

	// Mixer Config
	BOOL CSoundFile_InitPlayer(CSoundFile *_this, BOOL bReset);
	BOOL CSoundFile_SetMixConfig(CSoundFile *_this, UINT nStereoSeparation, UINT nMaxMixChannels);
	BOOL CSoundFile_SetWaveConfig(CSoundFile *_this, UINT nRate,UINT nBits,UINT nChannels);
	BOOL CSoundFile_SetResamplingMode(CSoundFile *_this, UINT nMode); // SRCMODE_XXXX
	DWORD CSoundFile_InitSysInfo(CSoundFile *_this);

	//GCCFIX -- added these functions back in!
	BOOL CSoundFile_SetWaveConfigEx(CSoundFile *_this, BOOL bSurround,BOOL bNoOverSampling,BOOL bReverb,BOOL hqido,BOOL bMegaBass,BOOL bNR,BOOL bEQ);
	// DSP Effects
	void CSoundFile_InitializeDSP(CSoundFile *_this, BOOL bReset);
	void CSoundFile_ProcessStereoDSP(CSoundFile *_this, int count);
	// [Reverb level 0(quiet)-100(loud)], [delay in ms, usually 40-200ms]
	BOOL CSoundFile_SetReverbParameters(CSoundFile *_this, UINT nDepth, UINT nDelay);
	// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 10-100]
	BOOL CSoundFile_SetXBassParameters(CSoundFile *_this, UINT nDepth, UINT nRange);
	// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-40ms]
	BOOL CSoundFile_SetSurroundParameters(CSoundFile *_this, UINT nDepth, UINT nDelay);

	BOOL CSoundFile_ReadNote(CSoundFile *_this);
	BOOL CSoundFile_ProcessRow(CSoundFile *_this);
	BOOL CSoundFile_ProcessEffects(CSoundFile *_this);
	UINT CSoundFile_GetNNAChannel(CSoundFile *_this, UINT nChn);
	void CSoundFile_CheckNNA(CSoundFile *_this, UINT nChn, UINT instr, int note, BOOL bForceCut);
	void CSoundFile_NoteChange(CSoundFile *_this, UINT nChn, int note, BOOL bPorta, BOOL bResetEnv);
	void CSoundFile_InstrumentChange(CSoundFile *_this, MODCHANNEL *pChn, UINT instr, BOOL bPorta,BOOL bUpdVol,BOOL bResetEnv);
	// Channel Effects
	void CSoundFile_PortamentoUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_PortamentoDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_FinePortamentoUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_FinePortamentoDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_ExtraFinePortamentoUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_ExtraFinePortamentoDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_TonePortamento(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_Vibrato(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_FineVibrato(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_VolumeSlide(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_PanningSlide(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_ChannelVolSlide(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_FineVolumeUp(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_FineVolumeDown(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_Tremolo(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_Panbrello(CSoundFile *_this, MODCHANNEL *pChn, UINT param);
	void CSoundFile_RetrigNote(CSoundFile *_this, UINT nChn, UINT param);
	void CSoundFile_NoteCut(CSoundFile *_this, UINT nChn, UINT nTick);
	void CSoundFile_KeyOff(CSoundFile *_this, UINT nChn);
	int CSoundFile_PatternLoop(CSoundFile *_this, MODCHANNEL *, UINT param);
	void CSoundFile_ExtendedMODCommands(CSoundFile *_this, UINT nChn, UINT param);
	void CSoundFile_ExtendedS3MCommands(CSoundFile *_this, UINT nChn, UINT param);
	void CSoundFile_ExtendedChannelEffect(CSoundFile *_this, MODCHANNEL *, UINT param);
	void CSoundFile_ProcessMidiMacro(CSoundFile *_this, UINT nChn, LPCSTR pszMidiMacro, UINT param);
	void CSoundFile_SetupChannelFilter(CSoundFile *_this, MODCHANNEL *pChn, BOOL bReset, int flt_modifier);
	// Low-Level effect processing
	void CSoundFile_DoFreqSlide(CSoundFile *_this, MODCHANNEL *pChn, LONG nFreqSlide);
	// Global Effects
	void CSoundFile_SetTempo(CSoundFile *_this, UINT param);
	void CSoundFile_SetSpeed(CSoundFile *_this, UINT param);
	void CSoundFile_GlobalVolSlide(CSoundFile *_this, UINT param);
	DWORD CSoundFile_IsSongFinished(CSoundFile *_this, UINT nOrder, UINT nRow);
	BOOL CSoundFile_IsValidBackwardJump(CSoundFile *_this, UINT nStartOrder, UINT nStartRow, UINT nJumpOrder, UINT nJumpRow);
	// Read/Write sample functions
	UINT CSoundFile_ReadSample(CSoundFile *_this, MODINSTRUMENT *pIns, UINT nFlags, LPCSTR pMemFile, DWORD dwMemLength);
	BOOL CSoundFile_DestroySample(CSoundFile *_this, UINT nSample);
	BOOL CSoundFile_DestroyInstrument(CSoundFile *_this, UINT nInstr);
	BOOL CSoundFile_IsSampleUsed(CSoundFile *_this, UINT nSample);
	BOOL CSoundFile_IsInstrumentUsed(CSoundFile *_this, UINT nInstr);
	BOOL CSoundFile_RemoveInstrumentSamples(CSoundFile *_this, UINT nInstr);
	UINT CSoundFile_DetectUnusedSamples(CSoundFile *_this, BOOL *);
	void CSoundFile_AdjustSampleLoop(CSoundFile *_this, MODINSTRUMENT *pIns);
	// I/O from another sound file
	BOOL CSoundFile_ReadInstrumentFromSong(CSoundFile *_this, UINT nInstr, CSoundFile *, UINT nSrcInstrument);
	BOOL CSoundFile_ReadSampleFromSong(CSoundFile *_this, UINT nSample, CSoundFile *, UINT nSrcSample);
	// Period/Note functions
	UINT CSoundFile_GetNoteFromPeriod(CSoundFile *_this, UINT period);
	UINT CSoundFile_GetPeriodFromNote(CSoundFile *_this, UINT note, int nFineTune, UINT nC4Speed);
	UINT CSoundFile_GetFreqFromPeriod(CSoundFile *_this, UINT period, UINT nC4Speed, int nPeriodFrac);
	// Misc functions
	void CSoundFile_ResetMidiCfg(CSoundFile *_this);
	UINT CSoundFile_MapMidiInstrument(CSoundFile *_this, DWORD dwProgram, UINT nChannel, UINT nNote);
	BOOL CSoundFile_ITInstrToMPT(const void *p, INSTRUMENTHEADER *penv, UINT trkvers);

	UINT CSoundFile_LoadMixPlugins(CSoundFile *_this, const void *pData, UINT nLen);
#ifndef NO_FILTER
	DWORD CSoundFile_CutOffToFrequency(CSoundFile *_this, UINT nCutOff, int flt_modifier); // [0-255] => [1-10KHz]
#endif

	DWORD CSoundFile_TransposeToFrequency(int transp, int ftune);
	int CSoundFile_FrequencyToTranspose(DWORD freq);
	void CSoundFile_FrequencyToTransposeInstrument(MODINSTRUMENT *psmp);

	MODCOMMAND *CSoundFile_AllocatePattern(UINT rows, UINT nchns);
	signed char* CSoundFile_AllocateSample(UINT nbytes);
	void CSoundFile_FreePattern(LPVOID pat);
	void CSoundFile_FreeSample(LPVOID p);
	UINT CSoundFile_Normalize24BitBuffer(LPBYTE pbuffer, UINT cbsizebytes, DWORD lmax24, DWORD dwByteInc);



#pragma pack(1)

typedef struct tagITFILEHEADER
{
	DWORD id;			// 0x4D504D49
	CHAR songname[26];
	WORD reserved1;		// 0x1004
	WORD ordnum;
	WORD insnum;
	WORD smpnum;
	WORD patnum;
	WORD cwtv;
	WORD cmwt;
	WORD flags;
	WORD special;
	BYTE globalvol;
	BYTE mv;
	BYTE speed;
	BYTE tempo;
	BYTE sep;
	BYTE zero;
	WORD msglength;
	DWORD msgoffset;
	DWORD reserved2;
	BYTE chnpan[64];
	BYTE chnvol[64];
} ITFILEHEADER;


typedef struct tagITENVELOPE
{
	BYTE flags;
	BYTE num;
	BYTE lpb;
	BYTE lpe;
	BYTE slb;
	BYTE sle;
	BYTE data[25*3];
	BYTE reserved;
} ITENVELOPE;

// Old Impulse Instrument Format (cmwt < 0x200)
typedef struct tagITOLDINSTRUMENT
{
	DWORD id;			// IMPI = 0x49504D49
	CHAR filename[12];	// DOS file name
	BYTE zero;
	BYTE flags;
	BYTE vls;
	BYTE vle;
	BYTE sls;
	BYTE sle;
	WORD reserved1;
	WORD fadeout;
	BYTE nna;
	BYTE dnc;
	WORD trkvers;
	BYTE nos;
	BYTE reserved2;
	WORD reserved3[3];
	BYTE keyboard[240];
	BYTE volenv[200];
	BYTE nodes[50];
} ITOLDINSTRUMENT;


// Impulse Instrument Format
typedef struct tagITINSTRUMENT
{
	DWORD id;
	CHAR filename[12];
	BYTE zero;
	BYTE nna;
	BYTE dct;
	BYTE dca;
	WORD fadeout;
	signed char pps;
	BYTE ppc;
	BYTE gbv;
	BYTE dfp;
	BYTE rv;
	BYTE rp;
	WORD trkvers;
	BYTE nos;
	BYTE reserved1;
	BYTE ifc;
	BYTE ifr;
	BYTE mch;
	BYTE mpr;
	WORD mbank;
	BYTE keyboard[240];
	ITENVELOPE volenv;
	ITENVELOPE panenv;
	ITENVELOPE pitchenv;
	BYTE dummy[4]; // was 7, but IT v2.17 saves 554 bytes
} ITINSTRUMENT;


// IT Sample Format
typedef struct ITSAMPLESTRUCT
{
	DWORD id;		// 0x53504D49
	CHAR filename[12];
	BYTE zero;
	BYTE gvl;
	BYTE flags;
	BYTE vol;
	BYTE cvt;
	BYTE dfp;
	DWORD length;
	DWORD loopbegin;
	DWORD loopend;
	DWORD C5Speed;
	DWORD susloopbegin;
	DWORD susloopend;
	DWORD samplepointer;
	BYTE vis;
	BYTE vid;
	BYTE vir;
	BYTE vit;
} ITSAMPLESTRUCT;

#pragma pack()

extern BYTE autovibit2xm[8];
extern BYTE autovibxm2it[8];

#endif

