/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_PAT

  PAT sample loader.
	by Peter Grootswagers (2006)
	<email:pgrootswagers@planet.nl>

 It's primary purpose is loading samples for the .abc and .mid modules
 Can also be used stand alone, in that case a tune (frere Jacques)
 is generated using al samples available in the .pat file

 Portability:
	All systems - all compilers (hopefully)
*/

#include "libmodplug.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifndef _WIN32
#include <limits.h> // for PATH_MAX
#endif
#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#include "load_pat.h"

#ifdef MSC_VER
#define DIRDELIM		'\\'
#define TIMIDITYCFG	"C:\\TIMIDITY\\TIMIDITY.CFG"
#define PATHFORPAT	"C:\\TIMIDITY\\INSTRUMENTS"
#else
#define DIRDELIM		'/'
#define TIMIDITYCFG	"/usr/local/share/timidity/timidity.cfg"
#define PATHFORPAT	"/usr/local/share/timidity/instruments"
#endif

#define PAT_ENV_PATH2CFG			"MMPAT_PATH_TO_CFG"

/* !!! FIXME: global state */
// 128 gm and 63 drum
#define MAXSMP				191
static char midipat[MAXSMP][PATH_MAX];
static char pathforpat[PATH_MAX];
static char timiditycfg[PATH_MAX];

#pragma pack(1)

typedef struct {
	char header[12];	// ascizz GF1PATCH110
	char gravis_id[10];	// allways ID#000002
	char description[60];
	BYTE instruments;
	BYTE voices;
	BYTE channels;
	WORD waveforms;
	WORD master_volume;
	DWORD data_size;
	char reserved[36];
} PatchHeader;

typedef struct {
	WORD instrument_id;
	char instrument_name[16];
	DWORD instrument_size;
	BYTE layers;
	char reserved[40];
} InstrumentHeader;

typedef struct {
	BYTE layer_dup;
	BYTE layer_id;
	DWORD layer_size;
	BYTE samples;
	char reserved[40];
} LayerHeader;

typedef struct {
	char wave_name[7];
	BYTE fractions;
	DWORD wave_size;
	DWORD start_loop;
	DWORD end_loop;
	WORD sample_rate;
	DWORD low_frequency ;
	DWORD high_frequency;
	DWORD root_frequency;
	short int tune;
	BYTE balance;
	BYTE envelope_rate[6];
	BYTE envelope_offset[6];
	BYTE tremolo_sweep;
	BYTE tremolo_rate;
	BYTE tremolo_depth;
	BYTE vibrato_sweep;
	BYTE vibrato_rate;
	BYTE vibrato_depth;
	BYTE modes;
	DWORD scale_frequency;
	DWORD scale_factor;
	char reserved[32];
} WaveHeader;

// WaveHeader.modes bits
#define PAT_16BIT     1
#define PAT_UNSIGNED  2
#define PAT_LOOP      4
#define PAT_PINGPONG  8
#define PAT_BACKWARD 16
#define PAT_SUSTAIN  32
#define PAT_ENVELOPE 64
#define PAT_CLAMPED 128

#define C4SPD	8363
#define C4mHz	523251
#define C4	  523.251f
#define PI 	  3.141592653589793f
#define OMEGA	((2.0f * PI * C4)/(float)C4SPD)

/**************************************************************************
**************************************************************************/
/* !!! FIXME: global state */
static BYTE pat_gm_used[MAXSMP];
static BYTE pat_loops[MAXSMP];

/**************************************************************************
**************************************************************************/

static SDL_INLINE int IsAlpha(const char c) {
    return ( ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) );
}

static SDL_INLINE int IsBlank(const char c) {
    return ( (c == ' ') || (c == '\t') );
}


typedef struct _PATHANDLE
{
	char patname[16];
	int samples;
} PATHANDLE;

// local prototypes
static int pat_getopt(const char *s, const char *o, int dflt);

static void pat_message(const char *s1, const char *s2)
{
	char txt[256];
	if( SDL_strlen(s1) + SDL_strlen(s2) > 255 ) return;
	SDL_snprintf(txt, sizeof (txt), s1, s2);
	SDL_LogInfo(SDL_LOG_CATEGORY_AUDIO, "load_pat > %s\n", txt);
}

void pat_resetsmp(void)
{
	int i;
	for( i=0; i<MAXSMP; i++ ) {
		pat_loops[i] = 0;
		pat_gm_used[i] = 0;
	}
}

int pat_numsmp()
{
	return SDL_strlen((const char *)pat_gm_used);
}

int pat_numinstr(void)
{
	return SDL_strlen((const char *)pat_gm_used);
}

int pat_smptogm(int smp)
{
	if( smp < MAXSMP )
		return pat_gm_used[smp - 1];
	return 1;
}

int pat_gmtosmp(int gm)
{
	int smp;
	for( smp=0; pat_gm_used[smp]; smp++ )
		if( pat_gm_used[smp] == gm )
			return smp+1;
	if( smp < MAXSMP ) {
		pat_gm_used[smp] = gm;
		return smp+1;
	}
	return 1;
}

int pat_smplooped(int smp)
{
	if( smp < MAXSMP ) return pat_loops[smp - 1];
	return 1;
}

const char *pat_gm_name(int gm)
{
	static char buf[40];
	if( gm < 1 || gm > MAXSMP ) {
		SDL_snprintf(buf, sizeof (buf), "invalid gm %d", gm);
		return buf;
	}
	return midipat[gm - 1];
}

int pat_gm_drumnr(int n)
{
	if( n < 25 ) return 129;
	if( n+129-25 < MAXSMP )
		return 129+n-25; // timidity.cfg drum patches start at 25
	return MAXSMP;
}

int pat_gm_drumnote(int n)
{
	char *p;
	p = SDL_strchr(midipat[pat_gm_drumnr(n)-1], ':');
	if( p ) return pat_getopt(p+1, "note", n);
	return n;
}

static float pat_sinus(int i)
{
	float res = SDL_sinf(OMEGA * (float)i);
	return res;
}

static float pat_square(int i)
{
	float res = 30.0f * SDL_sinf(OMEGA * (float)i);
	if( res > 0.99f ) return 0.99f;
	if( res < -0.99f ) return -0.99f;
	return res;
}

static float pat_sawtooth(int i)
{
	float res = OMEGA * (float)i;
	while( res > 2 * PI )
		res -= 2 * PI;
	i = 2;
	if( res > PI ) {
		res = PI - res;
		i = -2;
	}
	res = (float)i * res / PI;
	if( res > 0.9f ) return 1.0f - res;
	if( res < -0.9f ) return 1.0f + res;
	return res;
}

typedef float (*PAT_SAMPLE_FUN)(int);

static PAT_SAMPLE_FUN pat_fun[] = { pat_sinus, pat_square, pat_sawtooth };

long _mm_getfsize(MMSTREAM *mmpat) {
	long fsize;
	_mm_fseek(mmpat, 0L, SEEK_END);
	fsize = _mm_ftell(mmpat);
	_mm_fseek(mmpat, 0L, SEEK_SET);
	return(fsize);
}

void pat_init_patnames(void)
{
	int z, i, nsources, isdrumset, nskip, pfnlen;
	char *p, *q;
	char line[PATH_MAX];
	char cfgsources[5][PATH_MAX];
	MMSTREAM *mmcfg;
    SDL_memset(cfgsources, 0, sizeof (cfgsources));
	SDL_strlcpy(pathforpat, PATHFORPAT, PATH_MAX);
	SDL_strlcpy(timiditycfg, TIMIDITYCFG, PATH_MAX);
	p = SDL_getenv(PAT_ENV_PATH2CFG);
	if( p ) {
        SDL_snprintf(timiditycfg, sizeof (timiditycfg), "%s/timidity.cfg", p);
		SDL_snprintf(pathforpat, sizeof (pathforpat), "%s", p);
	}
	SDL_strlcpy(cfgsources[0], timiditycfg, PATH_MAX - 1);
	nsources = 1;

	for( i=0; i<MAXSMP; i++ )	midipat[i][0] = '\0';

	for ( z=0; z<5; z++ ) {
		if (cfgsources[z][0] == 0) continue;
		mmcfg = _mm_fopen(cfgsources[z],"r");
		if( !mmcfg ) {
			pat_message("can not open %s, use environment variable " PAT_ENV_PATH2CFG " for the directory", cfgsources[z]);
		}
		else {
			// read in bank 0 and drum patches
			isdrumset = 0;
			_mm_fgets(mmcfg, line, PATH_MAX);
			while( !_mm_feof(mmcfg) ) {
			if( SDL_isdigit(line[0]) || (IsBlank(line[0]) && SDL_isdigit(line[1])) ) {
				p = line;
				// get pat number
				while ( SDL_isspace(*p) ) p ++;
				i = SDL_atoi(p);
				while ( SDL_isdigit(*p) ) p ++;
				while ( SDL_isspace(*p) ) p ++;
				// get pat file name
				if( *p && i < MAXSMP && i >= 0 && *p != '#' ) {
					q = isdrumset ? midipat[pat_gm_drumnr(i)-1] : midipat[i];
					pfnlen = 0;
					while( *p && !SDL_isspace(*p) && *p != '#' && pfnlen < 128 ) {
						pfnlen ++;
						*q++ = *p++;
					}
					if( IsBlank(*p) && *(p+1) != '#' && pfnlen < 128 ) {
						*q++ = ':'; pfnlen ++;
						while( SDL_isspace(*p) ) {
							while( SDL_isspace(*p) ) p++;
							if ( *p == '#' ) { // comment

							} else while( *p && !SDL_isspace(*p) && pfnlen < 128 ) {
								pfnlen ++;
								*q++ = *p++;
							}
							if( SDL_isspace(*p) ) { *q++ = ' '; pfnlen++; }
						}
					}
					*q++ = '\0';
				}
			}
			if( !SDL_strncmp(line,"drumset",7) ) isdrumset = 1;
			if( !SDL_strncmp(line,"source",6) && nsources < 5 ) {
				q = cfgsources[nsources];
				p = &line[7];
				while ( SDL_isspace(*p) ) p ++;
				pfnlen = 0;
				while ( *p && *p != '#' && !SDL_isspace(*p) && pfnlen < 128 ) {
					pfnlen ++;
					*q++ = *p++;
				}
				*q = 0; // null termination
				nsources++;
			}
			_mm_fgets(mmcfg, line, PATH_MAX);

			} /* end file parsing */
			_mm_fclose(mmcfg);
		}
	}
	q = midipat[0];
	nskip = 0;
	// make all empty patches duplicates the previous valid one.
	for( i=0; i<MAXSMP; i++ )	{
		if( midipat[i][0] ) q = midipat[i];
		else {
			if( midipat[i] != q)
				SDL_strlcpy(midipat[i], q, PATH_MAX);
			if( midipat[i][0] == '\0' ) nskip++;
		}
	}
	if( nskip ) {
		for( i=MAXSMP; i-- > 0; )	{
			if( midipat[i][0] ) q = midipat[i];
			else if( midipat[i] != q )
				SDL_strlcpy(midipat[i], q, PATH_MAX);
		}
	}
}

static char *pat_build_path(char *fname, const size_t fnamelen, int pat)
{
	char *ps;
	char *patfile = midipat[pat];
	int isabspath = (patfile[0] == '/');
	if ( isabspath ) patfile ++;
	ps = SDL_strrchr(patfile, ':');
	if( ps ) {
		SDL_snprintf(fname, fnamelen, "%s%c%s", isabspath ? "" : pathforpat, DIRDELIM, patfile);
        char *replaceptr = SDL_strrchr(fname, ':');
        SDL_assert(replaceptr != NULL);
        *replaceptr = '\0';
		SDL_strlcat(fname, ".pat", fnamelen);
		return ps;
	}
	SDL_snprintf(fname, fnamelen, "%s%c%s.pat", isabspath ? "" : pathforpat, DIRDELIM, patfile);
	return 0;
}

static void pat_read_patname(PATHANDLE *h, MMFILE *mmpat) {
	InstrumentHeader ih;
	mmfseek(mmpat,sizeof(PatchHeader), SEEK_SET);
	mmreadUBYTES((BYTE *)&ih, sizeof(InstrumentHeader), mmpat);
	SDL_strlcpy(h->patname, ih.instrument_name, 16);
	h->patname[15] = '\0';
}

static void pat_read_layerheader(MMSTREAM *mmpat, LayerHeader *hl)
{
	_mm_fseek(mmpat,sizeof(PatchHeader)+sizeof(InstrumentHeader), SEEK_SET);
	_mm_read_UBYTES((BYTE *)hl, sizeof(LayerHeader), mmpat);
}

static void pat_get_layerheader(MMFILE *mmpat, LayerHeader *hl)
{
	InstrumentHeader ih;
	mmfseek(mmpat,sizeof(PatchHeader), SEEK_SET);
	mmreadUBYTES((BYTE *)&ih, sizeof(InstrumentHeader), mmpat);
	mmreadUBYTES((BYTE *)hl, sizeof(LayerHeader), mmpat);
	SDL_strlcpy(hl->reserved, ih.instrument_name, 40);
}

static int pat_read_numsmp(MMFILE *mmpat) {
	LayerHeader hl;
	pat_get_layerheader(mmpat, &hl);
	return hl.samples;
}

static void pat_read_waveheader(MMSTREAM *mmpat, WaveHeader *hw, int layer)
{
	long int pos, bestpos=0;
	LayerHeader hl;
	ULONG bestfreq, freqdist;
	int i;
	// read the very first and maybe only sample
	pat_read_layerheader(mmpat, &hl);
	if( hl.samples > 1 ) {
		if( layer ) {
			if( layer > hl.samples ) layer = hl.samples; // you don't fool me....
			for( i=1; i<layer; i++ ) {
				_mm_read_UBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				_mm_fseek(mmpat, hw->wave_size, SEEK_CUR);
			}
		}
		else {
			bestfreq = C4mHz * 1000;	// big enough
			for( i=0; i<hl.samples; i++ ) {
				pos = _mm_ftell(mmpat);
				_mm_read_UBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				if( hw->root_frequency > C4mHz )
					freqdist = hw->root_frequency - C4mHz;
				else
					freqdist = 2 * (C4mHz - hw->root_frequency);
				if( freqdist < bestfreq ) {
					bestfreq = freqdist;
					bestpos  = pos;
				}
				_mm_fseek(mmpat, hw->wave_size, SEEK_CUR);
			}
			_mm_fseek(mmpat, bestpos, SEEK_SET);
		}
	}
	_mm_read_UBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
	SDL_strlcpy(hw->reserved, hl.reserved, 32);
	hw->reserved[31] = 0;
	if( hw->start_loop >= hw->wave_size ) {
		hw->start_loop = 0;
		hw->end_loop = 0;
		hw->modes &= ~PAT_LOOP; // mask off loop indicator
	}
	if( hw->end_loop > hw->wave_size )
		hw->end_loop = hw->wave_size;
}

static void pat_get_waveheader(MMFILE *mmpat, WaveHeader *hw, int layer)
{
	long int pos, bestpos=0;
	LayerHeader hl;
	ULONG bestfreq, freqdist;
	int i;
	// read the very first and maybe only sample
	pat_get_layerheader(mmpat, &hl);
	if( hl.samples > 1 ) {
		if( layer ) {
			if( layer > hl.samples ) layer = hl.samples; // you don't fool me....
			for( i=1; i<layer; i++ ) {
				mmreadUBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				mmfseek(mmpat, hw->wave_size, SEEK_CUR);
				if ( mmpat->error ) {
					hw->wave_size = 0;
					return;
				}
			}
		}
		else {
			bestfreq = C4mHz * 1000;	// big enough
			for( i=0; i<hl.samples; i++ ) {
				pos = mmftell(mmpat);
				mmreadUBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
				if( hw->root_frequency > C4mHz )
					freqdist = hw->root_frequency - C4mHz;
				else
					freqdist = 2 * (C4mHz - hw->root_frequency);
				if( freqdist < bestfreq ) {
					bestfreq = freqdist;
					bestpos  = pos;
				}
				mmfseek(mmpat, hw->wave_size, SEEK_CUR);
			}
			mmfseek(mmpat, bestpos, SEEK_SET);
		}
	}
	mmreadUBYTES((BYTE *)hw, sizeof(WaveHeader), mmpat);
	if( hw->start_loop >= hw->wave_size ) {
		hw->start_loop = 0;
		hw->end_loop = 0;
		hw->modes &= ~PAT_LOOP; // mask off loop indicator
	}
	if( hw->end_loop > hw->wave_size )
		hw->end_loop = hw->wave_size;
}

static int pat_readpat_attr(int pat, WaveHeader *hw, int layer)
{
	char fname[128];
	unsigned long fsize;
	MMSTREAM *mmpat;
	pat_build_path(fname, sizeof (fname), pat);
	mmpat = _mm_fopen(fname, "r");
	if( !mmpat )
		return 0;
	fsize = _mm_getfsize(mmpat);
	pat_read_waveheader(mmpat, hw, layer);
	_mm_fclose(mmpat);
	if (hw->wave_size > fsize)
		return 0;
	return 1;
}

static void pat_amplify(char *b, int num, int amp, int m)
{
	char *pb;
	BYTE *pu;
	short int *pi;
	WORD *pw;
	int i,n,v;
	n = num;
	if( m & PAT_16BIT ) { // 16 bit
		n >>= 1;
		if( m & 2 ) {	// unsigned
			pw = (WORD *)b;
			for( i=0; i<n; i++ ) {
				v = (((int)(*pw) - 0x8000) * amp) / 100;
				if( v < -0x8000 ) v = -0x8000;
				if( v >  0x7fff ) v =  0x7fff;
				*pw++ = v + 0x8000;
			}
		}
		else {
			pi = (short int *)b;
			for( i=0; i<n; i++ ) {
				v = ((*pi) * amp) / 100;
				if( v < -0x8000 ) v = -0x8000;
				if( v >  0x7fff ) v =  0x7fff;
				*pi++ = v;
			}
		}
	}
	else {
		if( m & 2 ) {	// unsigned
			pu = (BYTE *)b;
			for( i=0; i<n; i++ ) {
				v = (((int)(*pu) - 0x80) * amp) / 100;
				if( v < -0x80 ) v = -0x80;
				if( v >  0x7f ) v =  0x7f;
				*pu++ = v + 0x80;
			}
		}
		else {
			pb = (char *)b;
			for( i=0; i<n; i++ ) {
				v = ((*pb) * amp) / 100;
				if( v < -0x80 ) v = -0x80;
				if( v >  0x7f ) v =  0x7f;
				*pb++ = v;
			}
		}
	}
}

static int pat_getopt(const char *s, const char *o, int dflt)
{
	const char *p;
	if( !s ) return dflt;
	p = SDL_strstr(s,o);
	if( !p ) return dflt;
	return SDL_atoi(SDL_strchr(p,'=')+1);
}

static void pat_readpat(int pat, char *dest, int num)
{
	static int readlasttime = 0, wavesize = 0;
	static MMSTREAM *mmpat = 0;
	static char *opt = 0;
	int amp;
	char fname[128];
	WaveHeader hw;
	if( !readlasttime ) {
		opt=pat_build_path(fname, sizeof (fname), pat);
		mmpat = _mm_fopen(fname, "r");
		if( !mmpat )
			return;
		pat_read_waveheader(mmpat, &hw, 0);
		wavesize = hw.wave_size;
	}
	_mm_read_SBYTES(dest, num, mmpat);
	amp = pat_getopt(opt,"amp",100);
	if( amp != 100 ) pat_amplify(dest, num, amp, hw.modes);
	readlasttime += num;
	if( readlasttime < wavesize ) return;
	readlasttime = 0;
	_mm_fclose(mmpat);
	mmpat = 0;
}

static BOOL dec_pat_Decompress16Bit(short int *dest, int cbcount, int samplenum)
{
	int i;
	PAT_SAMPLE_FUN f;
	if( samplenum < MAXSMP ) pat_readpat(samplenum, (char *)dest, cbcount*2);
	else {
		f = pat_fun[(samplenum - MAXSMP) % 3];
		for( i=0; i<cbcount; i++ )
			dest[i] = (short int)(32000.0*f(i));
	}
  return cbcount;
}

// convert 8 bit data to 16 bit!
// We do the conversion in reverse so that the data we're converting isn't overwritten
// by the result.
static void	pat_blowup_to16bit(short int *dest, int cbcount) {
	char *s;
	short int *d;
	int t;
	s  = (char *)dest;
	d  = dest;
	s += cbcount;
	d += cbcount;
	for(t=0; t<cbcount; t++)
	{   s--;
			d--;
			*d = (*s) << 8;
	}
}

static BOOL dec_pat_Decompress8Bit(short int *dest, int cbcount, int samplenum)
{
	int i;
	PAT_SAMPLE_FUN f;
	if( samplenum < MAXSMP ) pat_readpat(samplenum, (char *)dest, cbcount);
	else {
		f = pat_fun[(samplenum - MAXSMP) % 3];
		for( i=0; i<cbcount; i++ )
			dest[i] = (char)(120.0*f(i));
	}
	pat_blowup_to16bit(dest, cbcount);
	return cbcount;
}

BOOL CSoundFile_TestPAT(const BYTE *lpStream, DWORD dwMemLength)
// =====================================================================================
{
	PatchHeader ph;
	if( dwMemLength < sizeof(PatchHeader) ) return 0;
	SDL_memcpy((BYTE *)&ph, lpStream, sizeof(PatchHeader));
	if( !SDL_strcmp(ph.header,"GF1PATCH110") && !SDL_strcmp(ph.gravis_id,"ID#000002") ) return 1;
	return 0;
}

// =====================================================================================
static PATHANDLE *PAT_Init(void)
{
    PATHANDLE   *retval;
    retval = (PATHANDLE *)SDL_calloc(1,sizeof(PATHANDLE));
		if( !retval ) return NULL;
    return retval;
}

// =====================================================================================
static void PAT_Cleanup(PATHANDLE *handle)
// =====================================================================================
{
	if(handle) {
		SDL_free(handle);
	}
}

static char tune[] = "c d e c|c d e c|e f g..|e f g..|gagfe c|gagfe c|c G c..|c G c..|";
static int pat_note(int abc)
{
	switch( abc ) {
		case 'C': return 48;
		case 'D': return 50;
		case 'E': return 52;
		case 'F': return 53;
		case 'G': return 55;
		case 'A': return 57;
		case 'B': return 59;
		case 'c': return 60;
		case 'd': return 62;
		case 'e': return 64;
		case 'f': return 65;
		case 'g': return 67;
		case 'a': return 69;
		case 'b': return 71;
		default:
			break;
	}
	return 0;
}

int pat_modnote(int midinote)
{
	int n;
	n = midinote;
	n += 13;
	return n;
}

static void PAT_ReadPatterns(MODCOMMAND *pattern[], WORD psize[], PATHANDLE *h, int numpat)
// =====================================================================================
{
	int pat,row,i,ch;
	BYTE n,ins,vol;
	int t;
	int tt1, tt2;
	MODCOMMAND *m;
	if( numpat > MAX_PATTERNS ) numpat = MAX_PATTERNS;

	tt2 = (h->samples - 1) * 16 + 128;
	for( pat = 0; pat < numpat; pat++ ) {
		pattern[pat] = CSoundFile_AllocatePattern(64, h->samples);
		if( !pattern[pat] ) return;
		psize[pat] = 64;
		for( row = 0; row < 64; row++ ) {
			tt1 = (pat * 64 + row);
			for( ch = 0; ch < h->samples; ch++ ) {
				t = tt1 - ch * 16;
				m = &pattern[pat][row * h->samples + ch];
				m->param   = 0;
				m->command = CMD_NONE;
				if( t >= 0 ) {
					i = tt2 - 16 * ((h->samples - 1 - ch) & 3);
					if( tt1 < i ) {
						t = t % 64;
						if( IsAlpha(tune[t]) ) {
							n   = pat_modnote(pat_note(tune[t]));
							ins = ch + 1;
							vol = 40;
							if( (t % 16) == 0 ) {
								vol += vol / 10;
								if( vol > 64 ) vol = 64;
							}
							m->instr  = ins;
							m->note   = n; // <- normal note
							m->volcmd = VOLCMD_VOLUME;
							m->vol    = vol;
						}
						if( tt1 ==  i - 1 && ch == 0 && row < 63 ) {
							m->command = CMD_PATTERNBREAK;
						}
					}
					else {
						if( tt1 == i ) {
							m->param   = 0;
							m->command = CMD_KEYOFF;
							m->volcmd = VOLCMD_VOLUME;
							m->vol    = 0;
						}
					}
				}
			}
		}
	}
}

// calculate the best speed that approximates the pat root frequency as a C note
static ULONG pat_patrate_to_C4SPD(ULONG patRate , ULONG patMilliHz)
{
	ULONG u;
	double x, y;
	u = patMilliHz;
	x = 0.1 * patRate;
	x = x * C4mHz;
	y = u * 0.4;
	x = x / y;
	u = (ULONG)(x+0.5);
	return u;
}

// return relative position in samples for the rate starting with offset start ending with offset end
static int pat_envelope_rpos(int rate, int start, int end)
{
	int r, p, t, s;
	// rate byte is 3 bits exponent and 6 bits increment size
	//   eeiiiiii
	// every 8 to the power ee the volume is incremented/decremented by iiiiii
	// Thank you Gravis for this weirdness...
	r = 3 - ((rate >> 6) & 3) * 3;
	p = rate & 0x3f;
	if( !p ) return 0;
	t = end - start;
	if( !t ) return 0;
	if (t < 0) t = -t;
	s = (t << r)/ p;
	return s;
}

static void	pat_modenv(WaveHeader *hw, int mpos[6], int mvol[6])
{
	int i, sum, s;
	BYTE *prate = hw->envelope_rate, *poffset = hw->envelope_offset;
	for( i=0; i<6; i++ ) {
		mpos[i] = 0;
		mvol[i] = 64;
	}
	if( !SDL_memcmp(prate, "??????", 6) || poffset[5] >= 100 ) return; // weird rates or high env end volume
	if( !(hw->modes & PAT_SUSTAIN) ) return; // no sustain thus no need for envelope
	s = hw->wave_size;
	if (s == 0) return;
	if( hw->modes & PAT_16BIT )
		s >>= 1;
	// offsets 0 1 2 3 4 5 are distributed over 0 2 4 6 8 10, the odd numbers are set in between
	sum = 0;
	for( i=0; i<6; i++ ) {
		mvol[i] = poffset[i];
		mpos[i] = pat_envelope_rpos(prate[i], i? poffset[i-1]: 0, poffset[i]);
		sum += mpos[i];
	}
	if( sum == 0 ) return;
	if( sum > s ) {
		for( i=0; i<6; i++ )
			mpos[i] = (s * mpos[i]) / sum;
	}
	for( i=1; i<6; i++ )
		mpos[i] += mpos[i-1];
	for( i=0; i<6 ; i++ ) {
		mpos[i] = (256 * mpos[i]) / s;
		mpos[i]++;
		if( i > 0 && mpos[i] <= mpos[i-1] ) {
			if( mvol[i] == mvol[i-1] ) mpos[i] = mpos[i-1];
			else mpos[i] = mpos[i-1] + 1;
		}
		if( mpos[i] > 256 ) mpos[i] = 256;
	}
	mvol[5] = 0; // kill Bill....
}

static void pat_setpat_inst(WaveHeader *hw, INSTRUMENTHEADER *d, int smp)
{
	int u, inuse;
	int envpoint[6], envvolume[6];
	d->nMidiProgram = 0;
	d->nFadeOut = 0;
	d->nPan = 128;
	d->nPPC = 5*12;
	d->dwFlags = 0;
	if( hw->modes & PAT_ENVELOPE ) d->dwFlags |= ENV_VOLUME;
	if( hw->modes & PAT_SUSTAIN ) d->dwFlags |= ENV_VOLSUSTAIN;
	if( (hw->modes & PAT_LOOP) && (hw->start_loop != hw->end_loop) ) d->dwFlags |= ENV_VOLLOOP;
	d->nVolEnv = 6;
	//if (!d->nVolEnv) d->dwFlags &= ~ENV_VOLUME;
	d->nPanEnv = 0;
	d->nVolSustainBegin = 1;
	d->nVolSustainEnd   = 1;
	d->nVolLoopStart    = 1;
	d->nVolLoopEnd      = 2;
	d->nPanSustainBegin = 0;
	d->nPanSustainEnd   = 0;
	d->nPanLoopStart    = 0;
	d->nPanLoopEnd      = 0;
	d->nGlobalVol = 64;
	pat_modenv(hw, envpoint, envvolume);
	inuse = 0;
	for( u=0; u<6; u++)
	{
		if( envvolume[u] != 64 ) inuse = 1;
		d->VolPoints[u] = envpoint[u];
		d->VolEnv[u]    = envvolume[u];
		d->PanPoints[u] = 0;
		d->PanEnv[u]    = 0;
		if (u)
		{
			if (d->VolPoints[u] < d->VolPoints[u-1])
			{
				d->VolPoints[u] &= 0xFF;
				d->VolPoints[u] += d->VolPoints[u-1] & 0xFF00;
				if (d->VolPoints[u] < d->VolPoints[u-1]) d->VolPoints[u] += 0x100;
			}
		}
	}
	if( !inuse ) d->nVolEnv = 0;
	for( u=0; u<128; u++)
	{
		d->NoteMap[u] = u+1;
		d->Keyboard[u] = smp;
	}
}

static void PATinst(INSTRUMENTHEADER *d, int smp, int gm)
{
	WaveHeader hw;
	char s[32];
	if( pat_readpat_attr(gm-1, &hw, 0) ) {
		pat_setpat_inst(&hw, d, smp);
	}
	else {
		hw.modes = PAT_16BIT|PAT_ENVELOPE|PAT_SUSTAIN|PAT_LOOP;
		hw.start_loop = 0;
		hw.end_loop = 30000;
		hw.wave_size  = 30000;
//  envelope rates and offsets pinched from timidity's acpiano.pat sample no 1
		hw.envelope_rate[0] = 0x3f;
		hw.envelope_rate[1] = 0x3f;
		hw.envelope_rate[2] = 0x3f;
		hw.envelope_rate[3] = 0x08|(3<<6);
		hw.envelope_rate[4] = 0x3f;
		hw.envelope_rate[5] = 0x3f;
		hw.envelope_offset[0] = 246;
		hw.envelope_offset[1] = 246;
		hw.envelope_offset[2] = 246;
		hw.envelope_offset[3] = 0;
		hw.envelope_offset[4] = 0;
		hw.envelope_offset[5] = 0;
		SDL_strlcpy(hw.reserved, midipat[gm-1], sizeof(hw.reserved));
		pat_setpat_inst(&hw, d, smp);
	}
	SDL_strlcpy(s, midipat[gm-1], 12);
	SDL_memset(d->filename, 0, 12);
	SDL_strlcpy((char *)d->filename, s, sizeof (d->filename));
}

static void pat_setpat_attr(WaveHeader *hw, MODINSTRUMENT *q)
{
	q->nC4Speed   = pat_patrate_to_C4SPD(hw->sample_rate , hw->root_frequency);
	q->nLength    = hw->wave_size;
	q->nLoopStart = hw->start_loop;
	q->nLoopEnd   = hw->end_loop;
	q->nVolume    = 256;
	if( hw->modes & PAT_16BIT ) {
		q->nLength    >>= 1;
		q->nLoopStart >>= 1;
		q->nLoopEnd   >>= 1;
	}
	if( hw->modes & PAT_LOOP ) {
		q->uFlags |= CHN_LOOP;
		if( hw->modes & PAT_PINGPONG ) q->uFlags |= CHN_PINGPONGSUSTAIN;
		if( hw->modes & PAT_SUSTAIN ) q->uFlags |= CHN_SUSTAINLOOP;
	}
}

// ==========================
// Load those darned Samples!
static void PATsample(CSoundFile *cs, MODINSTRUMENT *q, int smp, int gm)
{
	WaveHeader hw;
	q->nGlobalVol = 64;
	q->nPan       = 128;
	q->uFlags     = CHN_16BIT;
	if( pat_readpat_attr(gm-1, &hw, 0) ) {
		char *p;
		pat_setpat_attr(&hw, q);
		pat_loops[smp-1] = (q->uFlags & CHN_LOOP)? 1: 0;
		if( hw.modes & PAT_16BIT ) p = (char *)SDL_malloc(hw.wave_size);
		else p = (char *)SDL_malloc(hw.wave_size * sizeof(char)*2);
		if( p ) {
			if( hw.modes & PAT_16BIT ) {
				dec_pat_Decompress16Bit((short int *)p, hw.wave_size>>1, gm - 1);
				CSoundFile_ReadSample(cs, q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size);
			}
			else {
				dec_pat_Decompress8Bit((short int *)p, hw.wave_size, gm - 1);
				CSoundFile_ReadSample(cs, q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size * sizeof(short int));
			}
			SDL_free(p);
		}
	}
	else {
		char *p;
		q->nC4Speed   = C4SPD;
		q->nLength    = 30000;
		q->nLoopStart = 0;
		q->nLoopEnd   = 30000;
		q->nVolume    = 256;
		q->uFlags    |= CHN_LOOP;
		q->uFlags    |= CHN_16BIT;
		p = (char *)SDL_malloc(q->nLength*sizeof(char)*2);
		if( p ) {
			dec_pat_Decompress8Bit((short int *)p, q->nLength, smp + MAXSMP - 1);
			CSoundFile_ReadSample(cs, q, RS_PCM16S, (LPSTR)p, q->nLength*2);
			SDL_free(p);
		}
	}
}

// =====================================================================================
BOOL PAT_Load_Instruments(void *c)
{
	uint32_t t;
	CSoundFile *of=(CSoundFile *)c;
	if( !pat_numsmp() ) pat_gmtosmp(1); // make sure there is a sample
	of->m_nSamples     = pat_numsmp() + 1; // xmms modplug does not use slot zero
	of->m_nInstruments = pat_numinstr() + 1;
	for(t=1; t<of->m_nInstruments; t++) { // xmms modplug doesn't use slot zero
		if( (of->Headers[t] = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER))) == NULL ) return FALSE;
		SDL_memset(of->Headers[t], 0, sizeof(INSTRUMENTHEADER));
		PATinst(of->Headers[t], t, pat_smptogm(t));
	}
	for(t=1; t<of->m_nSamples; t++) { // xmms modplug doesn't use slot zero
		PATsample(of, &of->Ins[t], t, pat_smptogm(t));
	}
	// copy last of the mohicans to entry 0 for XMMS modinfo to work....
	t = of->m_nInstruments - 1;
	if( (of->Headers[0] = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER))) == NULL ) return FALSE;
	SDL_memcpy(of->Headers[0], of->Headers[t], sizeof(INSTRUMENTHEADER));
	t = of->m_nSamples - 1;
	SDL_memcpy(&of->Ins[0], &of->Ins[t], sizeof(MODINSTRUMENT));
	return TRUE;
}
// =====================================================================================
BOOL CSoundFile_ReadPAT(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
{
	static int avoid_reentry = 0;
	char buf[60];
	int t;
	PATHANDLE *h;
	int numpat;
	MMFILE mm, *mmfile;
	MODINSTRUMENT *q;
	INSTRUMENTHEADER *d;
	if( !CSoundFile_TestPAT(lpStream, dwMemLength) ) return FALSE;
	h = PAT_Init();
	if( !h ) return FALSE;
	mmfile = &mm;
	mm.mm = (char *)lpStream;
	mm.sz = dwMemLength;
	mm.pos = 0;
	mm.error = 0;
	while( avoid_reentry ) SDL_Delay(1);
	avoid_reentry = 1;
	pat_read_patname(h, mmfile);
	h->samples = pat_read_numsmp(mmfile);
	if( SDL_strlen(h->patname) )
		SDL_snprintf(buf,sizeof (buf),"%s canon %d-v (Fr. Jacques)", h->patname, h->samples);
	else
		SDL_snprintf(buf,sizeof (buf),"%d-voice canon (Fr. Jacques)", h->samples);
	if( SDL_strlen(buf) > 31 ) buf[31] = '\0'; // chop it of
	_this->m_nDefaultTempo = 60; // 120 / 2
	t = (h->samples - 1) * 16 + 128;
	if( t % 64 ) t += 64;
	t = t / 64;
	_this->m_nType         = MOD_TYPE_PAT;
	_this->m_nInstruments  = h->samples >= MAX_INSTRUMENTS-1 ? MAX_INSTRUMENTS-1 : h->samples + 1; // we know better but use each sample in the pat...
	_this->m_nSamples      = h->samples >= MAX_SAMPLES-1 ? MAX_SAMPLES-1 : h->samples + 1; // xmms modplug does not use slot zero
	_this->m_nDefaultSpeed = 6;
	_this->m_nChannels     = h->samples;
	numpat          = t;

	_this->m_dwSongFlags   = SONG_LINEARSLIDES;
	_this->m_nMinPeriod    = 28 << 2;
	_this->m_nMaxPeriod    = 1712 << 3;
	// orderlist
	for(t=0; t < numpat; t++)
		_this->Order[t] = t;
	for(t=1; t<(int)_this->m_nInstruments; t++) { // xmms modplug doesn't use slot zero
		WaveHeader hw;
		char s[32];
		if( (d = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER))) == NULL ) {
			avoid_reentry = 0;
			return FALSE;
		}
		SDL_memset(d, 0, sizeof(INSTRUMENTHEADER));
		_this->Headers[t] = d;
		SDL_memset(d->filename, 0, 12);
		SDL_strlcpy((char *)d->filename, s, 12);
		pat_get_waveheader(mmfile, &hw, t);
		pat_setpat_inst(&hw, d, t);
	}
	for(t=1; t<(int)_this->m_nSamples; t++) { // xmms modplug doesn't use slot zero
		WaveHeader hw;
		char *p;
		q = &_this->Ins[t];	// we do not use slot zero
		q->nGlobalVol = 64;
		q->nPan       = 128;
		q->uFlags     = CHN_16BIT;
		pat_get_waveheader(mmfile, &hw, t);
		pat_setpat_attr(&hw, q);
		if ( hw.wave_size == 0 ) p = NULL;
		else if( hw.modes & PAT_16BIT ) p = (char *)SDL_malloc(hw.wave_size);
		else p = (char *)SDL_malloc(hw.wave_size * sizeof(char) * 2);
		if( p ) {
			mmreadSBYTES(p, hw.wave_size, mmfile);
			if( hw.modes & PAT_16BIT ) {
				CSoundFile_ReadSample(_this, q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size);
			}
			else {
				pat_blowup_to16bit((short int *)p, hw.wave_size);
				CSoundFile_ReadSample(_this, q, (hw.modes&PAT_UNSIGNED)?RS_PCM16U:RS_PCM16S, (LPSTR)p, hw.wave_size * sizeof(short int));
			}
			SDL_free(p);
		}
	}
	// copy last of the mohicans to entry 0 for XMMS modinfo to work....
	t = _this->m_nInstruments - 1;
	if( (_this->Headers[0] = (INSTRUMENTHEADER *) SDL_malloc(sizeof (INSTRUMENTHEADER))) == NULL ) {
		avoid_reentry = 0;
		return FALSE;
	}
	SDL_memcpy(_this->Headers[0], _this->Headers[t], sizeof(INSTRUMENTHEADER));
	t = _this->m_nSamples - 1;
	SDL_memcpy(&_this->Ins[0], &_this->Ins[t], sizeof(MODINSTRUMENT));
	// ==============================
	// Load the pattern info now!
	PAT_ReadPatterns(_this->Patterns, _this->PatternSize, h, numpat);
	// ============================================================
	// set panning positions
	for(t=0; t<(int)_this->m_nChannels; t++) {
		_this->ChnSettings[t].nPan = 0x30+((t+2)%5)*((0xD0 - 0x30)/5);     // 0x30 = std s3m val
		_this->ChnSettings[t].nVolume = 64;
	}
	avoid_reentry = 0; // it is safe now, I'm finished
	PAT_Cleanup(h);	// we dont need it anymore
	return 1;
}

