/**
 * SDL_sound; A sound processing toolkit.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/*
 * Shorten decoder for SDL_sound.
 *
 * This driver handles Shorten-compressed waveforms. Despite the fact that
 *  SHNs tend to be much bigger than MP3s, they are still the de facto
 *  standard in online music trading communities. If an MP3 crunches the
 *  waveform to 10-20 percent of its original size, SHNs only go to about
 *  50-60%. Why do the Phish fans of the world use this format then? Rabid
 *  music traders appreciate the sound quality; SHNs, unlike MP3s, do not
 *  throw away any part of the waveform. Yes, there are people that notice
 *  this, and further more, they demand it...and if they can't get a good
 *  transfer of those larger files over the 'net, they haven't underestimated
 *  the bandwidth of CDs travelling the world through the postal system.
 *
 * Shorten homepage: http://www.softsound.com/Shorten.html
 *
 * !!! FIXME: softsound.com is gone, I think.
 *
 * The Shorten format was gleaned from the shorten codebase, by Tony
 *  Robinson and SoftSound Limited.
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_SHN

#define SHN_BUFSIZ  512

typedef struct
{
    Sint32 version;
    Sint32 datatype;
    Sint32 nchan;
    Sint32 blocksize;
    Sint32 maxnlpc;
    Sint32 nmean;
    Sint32 nwrap;
    Sint32 **buffer;
    Sint32 **offset;
    Sint32 *qlpc;
    Sint32 lpcqoffset;
    Sint32 bitshift;
    int nbitget;
    int nbyteget;
    Uint8 *getbuf;
    Uint8 *getbufp;
    Uint32 gbuffer;
    Uint8 *backBuffer;
    Uint32 backBufferSize;
    Uint32 backBufLeft;
    Uint32 start_pos;
} shn_t;


static const Uint32 mask_table[] =
{
    0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F,
    0x0000003F, 0x0000007F, 0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
    0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF, 0x0001FFFF,
    0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
    0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF,
    0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};


static const Uint8 ulaw_outward[13][256] = {
{127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128},
{112,114,116,118,120,122,124,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,113,115,117,119,121,123,125,255,253,251,249,247,245,243,241,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,252,250,248,246,244,242,240},
{96,98,100,102,104,106,108,110,112,113,114,116,117,118,120,121,122,124,125,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,99,101,103,105,107,109,111,115,119,123,255,251,247,243,239,237,235,233,231,229,227,225,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,253,252,250,249,248,246,245,244,242,241,240,238,236,234,232,230,228,226,224},
{80,82,84,86,88,90,92,94,96,97,98,100,101,102,104,105,106,108,109,110,112,113,114,115,116,117,118,120,121,122,123,124,125,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,81,83,85,87,89,91,93,95,99,103,107,111,119,255,247,239,235,231,227,223,221,219,217,215,213,211,209,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,253,252,251,250,249,248,246,245,244,243,242,241,240,238,237,236,234,233,232,230,229,228,226,225,224,222,220,218,216,214,212,210,208},
{64,66,68,70,72,74,76,78,80,81,82,84,85,86,88,89,90,92,93,94,96,97,98,99,100,101,102,104,105,106,107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,65,67,69,71,73,75,77,79,83,87,91,95,103,111,255,239,231,223,219,215,211,207,205,203,201,199,197,195,193,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,238,237,236,235,234,233,232,230,229,228,227,226,225,224,222,221,220,218,217,216,214,213,212,210,209,208,206,204,202,200,198,196,194,192},
{49,51,53,55,57,59,61,63,64,66,67,68,70,71,72,74,75,76,78,79,80,81,82,84,85,86,87,88,89,90,92,93,94,95,96,97,98,99,100,101,102,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,50,52,54,56,58,60,62,65,69,73,77,83,91,103,255,231,219,211,205,201,197,193,190,188,186,184,182,180,178,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,230,229,228,227,226,225,224,223,222,221,220,218,217,216,215,214,213,212,210,209,208,207,206,204,203,202,200,199,198,196,195,194,192,191,189,187,185,183,181,179,177},
{32,34,36,38,40,42,44,46,48,49,51,52,53,55,56,57,59,60,61,63,64,65,66,67,68,70,71,72,73,74,75,76,78,79,80,81,82,83,84,85,86,87,88,89,90,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,33,35,37,39,41,43,45,47,50,54,58,62,69,77,91,255,219,205,197,190,186,182,178,175,173,171,169,167,165,163,161,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,218,217,216,215,214,213,212,211,210,209,208,207,206,204,203,202,201,200,199,198,196,195,194,193,192,191,189,188,187,185,184,183,181,180,179,177,176,174,172,170,168,166,164,162,160},
{16,18,20,22,24,26,28,30,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,51,52,53,55,56,57,58,59,60,61,63,64,65,66,67,68,69,70,71,72,73,74,75,76,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,17,19,21,23,25,27,29,31,35,39,43,47,54,62,77,255,205,190,182,175,171,167,163,159,157,155,153,151,149,147,145,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,204,203,202,201,200,199,198,197,196,195,194,193,192,191,189,188,187,186,185,184,183,181,180,179,178,177,176,174,173,172,170,169,168,166,165,164,162,161,160,158,156,154,152,150,148,146,144},
{2,4,6,8,10,12,14,16,17,18,20,21,22,24,25,26,28,29,30,32,33,34,35,36,37,38,40,41,42,43,44,45,46,48,49,50,51,52,53,54,55,56,57,58,59,60,61,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,1,3,5,7,9,11,13,15,19,23,27,31,39,47,62,255,190,175,167,159,155,151,147,143,141,139,137,135,133,131,129,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,189,188,187,186,185,184,183,182,181,180,179,178,177,176,174,173,172,171,170,169,168,166,165,164,163,162,161,160,158,157,156,154,153,152,150,149,148,146,145,144,142,140,138,136,134,132,130,128},
{1,2,4,5,6,8,9,10,12,13,14,16,17,18,19,20,21,22,24,25,26,27,28,29,30,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,3,7,11,15,23,31,47,255,175,159,151,143,139,135,131,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,158,157,156,155,154,153,152,150,149,148,147,146,145,144,142,141,140,138,137,136,134,133,132,130,129,128},
{1,2,3,4,5,6,8,9,10,11,12,13,14,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,7,15,31,255,159,143,135,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,142,141,140,139,138,137,136,134,133,132,131,130,129,128},
{1,2,3,4,5,6,7,8,9,10,11,12,13,14,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,15,255,143,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128},
{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,0,255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128}
};


#ifndef	MIN_MACRO
#define MIN_MACRO(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef	MAX_MACRO
#define MAX_MACRO(a,b) (((a)>(b))?(a):(b))
#endif

#define POSITIVE_ULAW_ZERO 0xff
#define NEGATIVE_ULAW_ZERO 0x7f

#define CAPMAXSCHAR(x)  ((x > 127) ? 127 : x)
#define CAPMAXUCHAR(x)  ((x > 255) ? 255 : x)
#define CAPMAXSHORT(x)  ((x > 32767) ? 32767 : x)
#define CAPMAXUSHORT(x) ((x > 65535) ? 65535 : x)

#define UNDEFINED_UINT      -1
#define DEFAULT_BLOCK_SIZE  256
#define DEFAULT_V0NMEAN     0
#define DEFAULT_V2NMEAN     4
#define DEFAULT_MAXNLPC     0
#define DEFAULT_NCHAN       1
#define DEFAULT_NSKIP       0
#define DEFAULT_NDISCARD    0
#define NBITPERLONG         32
#define DEFAULT_MINSNR      256
#define DEFAULT_QUANTERROR  0
#define MINBITRATE          2.5

#define MEAN_VERSION0   0
#define MEAN_VERSION2   4

#define SHN_FN_DIFF0        0
#define SHN_FN_DIFF1        1
#define SHN_FN_DIFF2        2
#define SHN_FN_DIFF3        3
#define SHN_FN_QUIT         4
#define SHN_FN_BLOCKSIZE    5
#define SHN_FN_BITSHIFT     6
#define SHN_FN_QLPC         7
#define SHN_FN_ZERO         8
#define SHN_FN_VERBATIM     9

#define SHN_TYPE_AU1              0
#define SHN_TYPE_S8               1
#define SHN_TYPE_U8               2
#define SHN_TYPE_S16HL            3
#define SHN_TYPE_U16HL            4
#define SHN_TYPE_S16LH            5
#define SHN_TYPE_U16LH            6
#define SHN_TYPE_ULAW             7
#define SHN_TYPE_AU2              8
#define SHN_TYPE_AU3              9
#define SHN_TYPE_ALAW            10
#define SHN_TYPE_RIFF_WAVE       11
#define SHN_TYPE_EOF             12
#define SHN_TYPE_GENERIC_ULAW   128
#define SHN_TYPE_GENERIC_ALAW   129

#define SHN_FNSIZE                  2
#define SHN_CHANNELSIZE             0
#define SHN_TYPESIZE                4
#define SHN_ULONGSIZE               2
#define SHN_NSKIPSIZE               1
#define SHN_LPCQSIZE                2
#define SHN_LPCQUANT                5
#define SHN_XBYTESIZE               7
#define SHN_VERBATIM_CKSIZE_SIZE    5
#define SHN_VERBATIM_BYTE_SIZE      8
#define SHN_ENERGYSIZE              3
#define SHN_BITSHIFTSIZE            2

#define SHN_LPCQOFFSET_VER2 (1 << SHN_LPCQUANT)


#define SHN_MAGIC  0x676B6A61   /* looks like "ajkg" as chars. */

#ifndef M_LN2
#define M_LN2   0.69314718055994530942
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif


static int word_get(shn_t *shn, SDL_RWops *rw, Uint32 *word)
{
    if (shn->nbyteget < 4)
    {
        shn->nbyteget += SDL_RWread(rw, shn->getbuf, 1, SHN_BUFSIZ);
        BAIL_IF_MACRO(shn->nbyteget < 4, NULL, 0);
        shn->getbufp = shn->getbuf;
    } /* if */

    if (word != NULL)
    {
        *word = (((Sint32) shn->getbufp[0]) << 24) |
                (((Sint32) shn->getbufp[1]) << 16) |
                (((Sint32) shn->getbufp[2]) <<  8) |
                (((Sint32) shn->getbufp[3])      );
    } /* if */

    shn->getbufp += 4;
    shn->nbyteget -= 4;

    return 1;
} /* word_get */


static int uvar_get(int nbin, shn_t *shn, SDL_RWops *rw, Sint32 *word)
{
    Sint32 result;

    if (shn->nbitget == 0)
    {
        BAIL_IF_MACRO(!word_get(shn, rw, &shn->gbuffer), NULL, 0);
        shn->nbitget = 32;
    } /* if */

    for (result = 0; !(shn->gbuffer & (1L << --shn->nbitget)); result++)
    {
        if (shn->nbitget == 0)
        {
            BAIL_IF_MACRO(!word_get(shn, rw, &shn->gbuffer), NULL, 0);
            shn->nbitget = 32;
        } /* if */
    } /* for */

    while (nbin != 0)
    {
        if (shn->nbitget >= nbin)
        {
            result = ( (result << nbin) |
                        ((shn->gbuffer >> (shn->nbitget - nbin)) &
                        mask_table[nbin]) );
            shn->nbitget -= nbin;
            break;
        } /* if */
        else
        {
            result = (result << shn->nbitget) |
                     (shn->gbuffer & mask_table[shn->nbitget]);
            BAIL_IF_MACRO(!word_get(shn, rw, &shn->gbuffer), NULL, 0);
            nbin -= shn->nbitget;
            shn->nbitget = 32;
        } /* else */
    } /* while */

    if (word != NULL)
        *word = result;

    return 1;
} /* uvar_get */


static int var_get(int nbin, shn_t *shn, SDL_RWops *rw, Sint32 *word)
{
    BAIL_IF_MACRO(!uvar_get(nbin + 1, shn, rw, word), NULL, 0);

    if ((*word) & 1)
        *word = (Sint32) ~((*word) >> 1);
    else
        *word = (Sint32) ((*word) >> 1);

    return 1;
} /* var_get */


static int ulong_get(shn_t *shn, SDL_RWops *rw, Sint32 *word)
{
    Sint32 nbit;
    Sint32 retval;
    BAIL_IF_MACRO(!uvar_get(SHN_ULONGSIZE, shn, rw, &nbit), NULL, 0);
    BAIL_IF_MACRO(!uvar_get(nbit, shn, rw, &retval), NULL, 0);

    if (word != NULL)
        *word = retval;

    return 1;
} /* ulong_get */


static SDL_INLINE int uint_get(int nbit, shn_t *shn, SDL_RWops *rw, Sint32 *w)
{
    return (shn->version == 0) ? uvar_get(nbit, shn, rw, w) : ulong_get(shn, rw, w);
} /* uint_get */


static int SHN_init(void)
{
    return 1;  /* initialization always successful. */
} /* SHN_init */


static void SHN_quit(void)
{
    /* it's a no-op. */
} /* SHN_quit */


/*
 * Look through the whole file for a SHN magic number. This is costly, so
 *  it should only be done if the user SWEARS they have a Shorten stream...
 */
static SDL_INLINE int extended_shn_magic_search(Sound_Sample *sample)
{
    SDL_RWops *rw = ((Sound_SampleInternal *) sample->opaque)->rw;
    Uint32 word = 0;
    Uint8 ch;

    while (1)
    {
        BAIL_IF_MACRO(SDL_RWread(rw, &ch, sizeof (ch), 1) != 1, NULL, -1);
        word = ((word << 8) & 0xFFFFFF00) | ch;
        if (SDL_SwapBE32(word) == SHN_MAGIC)
        {
            BAIL_IF_MACRO(SDL_RWread(rw, &ch, sizeof (ch), 1) != 1, NULL, -1);
            return (int) ch;
        } /* if */
    } /* while */

    return (int) ch;
} /* extended_shn_magic_search */


/* look for the magic number in the RWops and see what kind of file this is. */
static SDL_INLINE int determine_shn_version(Sound_Sample *sample,
                                            const char *ext)
{
    SDL_RWops *rw = ((Sound_SampleInternal *) sample->opaque)->rw;
    Uint32 magic;
    Uint8 ch;

    /*
     * Apparently the magic number can start at any byte offset in the file,
     *  and we should just discard prior data, but I'm going to restrict it
     *  to offset zero for now, so we don't chug down every file that might
     *  happen to pass through here. If the extension is explicitly "SHN", we
     *  check the whole stream, though.
     */

    if (SDL_strcasecmp(ext, "shn") == 0)
        return extended_shn_magic_search(sample);

    BAIL_IF_MACRO(SDL_RWread(rw, &magic, sizeof (magic), 1) != 1, NULL, -1);
    BAIL_IF_MACRO(SDL_SwapLE32(magic) != SHN_MAGIC, "SHN: Not a SHN file", -1);
    BAIL_IF_MACRO(SDL_RWread(rw, &ch, sizeof (ch), 1) != 1, NULL, -1);
    BAIL_IF_MACRO(ch > 3, "SHN: Unsupported file version", -1);

    return (int) ch;
} /* determine_shn_version */


static void init_shn_offset(Sint32 **offset, int nchan, int nblock, int ftype)
{
    Sint32 mean = 0;
    int chan;

    switch (ftype)
    {
        case SHN_TYPE_AU1:
        case SHN_TYPE_S8:
        case SHN_TYPE_S16HL:
        case SHN_TYPE_S16LH:
        case SHN_TYPE_ULAW:
        case SHN_TYPE_AU2:
        case SHN_TYPE_AU3:
        case SHN_TYPE_ALAW:
            mean = 0;
            break;
        case SHN_TYPE_U8:
            mean = 0x80;
            break;
        case SHN_TYPE_U16HL:
        case SHN_TYPE_U16LH:
            mean = 0x8000;
            break;
        default:
            __Sound_SetError("SHN: unknown file type");
            return;
    } /* switch */

    for(chan = 0; chan < nchan; chan++)
    {
        int i;
        for(i = 0; i < nblock; i++)
            offset[chan][i] = mean;
    } /* for */
} /* init_shn_offset */


static SDL_INLINE Uint16 cvt_shnftype_to_sdlfmt(Sint16 shntype)
{
    switch (shntype)
    {
        case SHN_TYPE_S8:
            return AUDIO_S8;

        case SHN_TYPE_ALAW:
        case SHN_TYPE_ULAW:
        case SHN_TYPE_AU1:
        case SHN_TYPE_AU2:
        case SHN_TYPE_AU3:
        case SHN_TYPE_U8:
            return AUDIO_U8;

        case SHN_TYPE_S16HL:
            return AUDIO_S16MSB;

        case SHN_TYPE_S16LH:
            return AUDIO_S16LSB;

        case SHN_TYPE_U16HL:
            return AUDIO_U16MSB;

        case SHN_TYPE_U16LH:
            return AUDIO_U16LSB;
    } /* switch */

    return 0;
} /* cvt_shnftype_to_sdlfmt */


static SDL_INLINE int skip_bits(shn_t *shn, SDL_RWops *rw)
{
    int i;
    Sint32 skip;
    Sint32 trash;

    BAIL_IF_MACRO(!uint_get(SHN_NSKIPSIZE, shn, rw, &skip), NULL, 0);
    for(i = 0; i < skip; i++)
    {
        BAIL_IF_MACRO(!uint_get(SHN_XBYTESIZE, shn, rw, &trash), NULL, 0);
    } /* for */

    return 1;
} /* skip_bits */


static Sint32 **shn_long2d(Uint32 n0, Uint32 n1)
{
    Sint32 **array0;
    Uint32 size = (n0 * sizeof (Sint32 *)) + (n0 * n1 * sizeof (Sint32));

    array0 = (Sint32 **) SDL_malloc(size);
    if (array0 != NULL)
    {
        int i;
        Sint32 *array1 = (Sint32 *) (array0 + n0);
        for(i = 0; i < n0; i++)
            array0[i] = array1 + (i * n1);
    } /* if */

    return array0;
} /* shn_long2d */

#define riffID 0x46464952  /* "RIFF", in ascii. */
#define waveID 0x45564157  /* "WAVE", in ascii. */
#define fmtID  0x20746D66  /* "fmt ", in ascii. */
#define dataID 0x61746164  /* "data", in ascii. */

static int verb_ReadLE32(shn_t *shn, SDL_RWops *rw, Uint32 *word)
{
    int i;
    Uint8 chars[4];
    Sint32 byte;

    for (i = 0; i < 4; i++)
    {
        if (!uvar_get(SHN_VERBATIM_BYTE_SIZE, shn, rw, &byte))
            return 0;
        chars[i] = (Uint8) byte;
    } /* for */

    SDL_memcpy(word, chars, sizeof (*word));
    *word = SDL_SwapLE32(*word);

    return 1;
} /* verb_ReadLE32 */


static int verb_ReadLE16(shn_t *shn, SDL_RWops *rw, Uint16 *word)
{
    int i;
    Uint8 chars[2];
    Sint32 byte;

    for (i = 0; i < 2; i++)
    {
        if (!uvar_get(SHN_VERBATIM_BYTE_SIZE, shn, rw, &byte))
            return 0;
        chars[i] = (Uint8) byte;
    } /* for */

    SDL_memcpy(word, chars, sizeof (*word));
    *word = SDL_SwapLE16(*word);

    return 1;
} /* verb_ReadLE16 */


static SDL_INLINE int parse_riff_header(shn_t *shn, Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;
    Uint16 u16;
    Uint32 u32;
    Sint32 cklen;
    Uint32 bytes_per_second;

    BAIL_IF_MACRO(!uvar_get(SHN_VERBATIM_CKSIZE_SIZE, shn, rw, &cklen), NULL, 0);

    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* RIFF header */
    BAIL_IF_MACRO(u32 != riffID, "SHN: No RIFF header.", 0);
    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* length */

    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* WAVE header */
    BAIL_IF_MACRO(u32 != waveID, "SHN: No WAVE header.", 0);

    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* 'fmt ' header */
    BAIL_IF_MACRO(u32 != fmtID,  "SHN: No 'fmt ' header.", 0);

    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* chunksize */
    BAIL_IF_MACRO(!verb_ReadLE16(shn, rw, &u16), NULL, 0); /* format */
    BAIL_IF_MACRO(!verb_ReadLE16(shn, rw, &u16), NULL, 0); /* channels */
    sample->actual.channels = u16;
    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* sample rate */
    sample->actual.rate = u32;

    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* bytespersec */
    bytes_per_second = u32;
    BAIL_IF_MACRO(!verb_ReadLE16(shn, rw, &u16), NULL, 0); /* blockalign */
    BAIL_IF_MACRO(!verb_ReadLE16(shn, rw, &u16), NULL, 0); /* bitspersample */

    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* 'data' header */
    BAIL_IF_MACRO(u32 != dataID,  "SHN: No 'data' header.", 0);
    BAIL_IF_MACRO(!verb_ReadLE32(shn, rw, &u32), NULL, 0); /* chunksize */
    internal->total_time = u32 / bytes_per_second * 1000;
    internal->total_time += (u32 % bytes_per_second) * 1000 / bytes_per_second;
    return 1;
} /* parse_riff_header */


static int SHN_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;
    shn_t _shn;
    shn_t *shn = &_shn;  /* malloc and copy later. */
    Sint32 cmd;
    Sint32 chan;

    SDL_memset(shn, '\0', sizeof (shn_t));
    shn->getbufp = shn->getbuf = (Uint8 *) SDL_malloc(SHN_BUFSIZ);
    shn->datatype = SHN_TYPE_EOF;
    shn->nchan = DEFAULT_NCHAN;
    shn->blocksize = DEFAULT_BLOCK_SIZE;
    shn->maxnlpc = DEFAULT_MAXNLPC;
    shn->nmean = UNDEFINED_UINT;
    shn->version = determine_shn_version(sample, ext);

    if (shn->version == -1) goto shn_open_puke;
    if (!uint_get(SHN_TYPESIZE, shn, rw, &shn->datatype)) goto shn_open_puke;
    if (!uint_get(SHN_CHANNELSIZE, shn, rw, &shn->nchan)) goto shn_open_puke;

    sample->actual.format = cvt_shnftype_to_sdlfmt(shn->datatype);
    if (sample->actual.format == 0)
    {
        SDL_SetError(ERR_UNSUPPORTED_FORMAT);
        goto shn_open_puke;
    } /* if */

    if (shn->version > 0)
    {
        int rc = uint_get((int) (SDL_log((double) DEFAULT_BLOCK_SIZE) / M_LN2),
                           shn, rw, &shn->blocksize);
        if (!rc)  goto shn_open_puke;;
        if (!uint_get(SHN_LPCQSIZE, shn, rw, &shn->maxnlpc)) goto shn_open_puke;
        if (!uint_get(0, shn, rw, &shn->nmean)) goto shn_open_puke;
        if (!skip_bits(shn, rw)) goto shn_open_puke;
    } /* else */

    shn->nwrap = (shn->maxnlpc > 3) ? shn->maxnlpc : 3;

    /* grab some space for the input buffer */
    shn->buffer = shn_long2d((Uint32) shn->nchan, shn->blocksize + shn->nwrap);
    shn->offset = shn_long2d((Uint32) shn->nchan, MAX_MACRO(1, shn->nmean));

    for (chan = 0; chan < shn->nchan; chan++)
    {
        int i;
        for(i = 0; i < shn->nwrap; i++)
            shn->buffer[chan][i] = 0;
        shn->buffer[chan] += shn->nwrap;
    } /* for */

    if (shn->maxnlpc > 0)
    {
        shn->qlpc = (int *) SDL_malloc((Uint32) (shn->maxnlpc * sizeof (Sint32)));
        if (shn->qlpc == NULL)
        {
            __Sound_SetError(ERR_OUT_OF_MEMORY);
            goto shn_open_puke;
        } /* if */
    } /* if */

    if (shn->version > 1)
        shn->lpcqoffset = SHN_LPCQOFFSET_VER2;

    init_shn_offset(shn->offset, shn->nchan,
                    MAX_MACRO(1, shn->nmean), shn->datatype);

    if ( (!uvar_get(SHN_FNSIZE, shn, rw, &cmd)) ||
         (cmd != SHN_FN_VERBATIM) ||
         (!parse_riff_header(shn, sample)) )
    {
        if (cmd != SHN_FN_VERBATIM) /* the other conditions set error state */
            __Sound_SetError("SHN: Expected VERBATIM function");

        goto shn_open_puke;
        return 0;
    } /* if */

    shn->start_pos = SDL_RWtell(rw);

    shn = (shn_t *) SDL_malloc(sizeof (shn_t));
    if (shn == NULL)
    {
        __Sound_SetError(ERR_OUT_OF_MEMORY);
        goto shn_open_puke;
    } /* if */

    SDL_memcpy(shn, &_shn, sizeof (shn_t));
    internal->decoder_private = shn;

    SNDDBG(("SHN: Accepting data stream.\n"));
    sample->flags = SOUND_SAMPLEFLAG_NONE;
    return 1; /* we'll handle this data. */

shn_open_puke:
    if (_shn.getbuf)
        SDL_free(_shn.getbuf);
    if (_shn.buffer != NULL)
        SDL_free(_shn.buffer);
    if (_shn.offset != NULL)
        SDL_free(_shn.offset);
    if (_shn.qlpc != NULL)
        SDL_free(_shn.qlpc);

    return 0;
} /* SHN_open */


static void fix_bitshift(Sint32 *buffer, int nitem, int bitshift, int ftype)
{
    int i;

    if (ftype == SHN_TYPE_AU1)
    {
        for (i = 0; i < nitem; i++)
            buffer[i] = ulaw_outward[bitshift][buffer[i] + 128];
    } /* if */
    else if (ftype == SHN_TYPE_AU2)
    {
        for(i = 0; i < nitem; i++)
        {
            if (buffer[i] >= 0)
                buffer[i] = ulaw_outward[bitshift][buffer[i] + 128];
            else if (buffer[i] == -1)
                buffer[i] = NEGATIVE_ULAW_ZERO;
            else
                buffer[i] = ulaw_outward[bitshift][buffer[i] + 129];
        } /* for */
    } /* else if */
    else
    {
        if (bitshift != 0)
        {
            for(i = 0; i < nitem; i++)
                buffer[i] <<= bitshift;
        } /* if */
    } /* else */
} /* fix_bitshift */


static void SHN_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    shn_t *shn = (shn_t *) internal->decoder_private;

    if (shn->qlpc != NULL)
        SDL_free(shn->qlpc);

    if (shn->backBuffer != NULL)
        SDL_free(shn->backBuffer);

    if (shn->offset != NULL)
        SDL_free(shn->offset);

    if (shn->buffer != NULL)
        SDL_free(shn->buffer);

    if (shn->getbuf != NULL)
        SDL_free(shn->getbuf);

    SDL_free(shn);
} /* SHN_close */


/* xLaw conversions... */

/* adapted by ajr for int input */
static Uint8 Slinear2ulaw(int sample)
{
/*
** This routine converts from linear to ulaw.
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) "A New Digital Technique for Implementation of Any
**     Continuous PCM Companding Law," Villeret, Michel,
**     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
**     1973, pg. 11.12-11.17
** 3) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: Signed 16 bit linear sample
** Output: 8 bit ulaw sample
*/

#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

  int sign, exponent, mantissa;
  Uint8 ulawbyte;
  static const int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                                   4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                                   5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                                   5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                                   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};

    /* Get the sample into sign-magnitude. */
    if (sample >= 0)
        sign = 0;
    else
    {
        sign = 0x80;
        sample = -sample;
    } /* else */

    /* clip the magnitude */
    if (sample > CLIP)
        sample = CLIP;

    /* Convert from 16 bit linear to ulaw. */
    sample = sample + BIAS;
    exponent = exp_lut[( sample >> 7 ) & 0xFF];
    mantissa = (sample >> (exponent + 3)) & 0x0F;
    ulawbyte = ~(sign | (exponent << 4) | mantissa);

    return ulawbyte;
} /* Slinear2ulaw */


/* this is derived from the Sun code - it is a bit simpler and has int input */
#define QUANT_MASK      (0xf)           /* Quantization field mask. */
#define NSEGS           (8)             /* Number of A-law segments. */
#define SEG_SHIFT       (4)             /* Left shift for segment number. */


static Uint8 Slinear2alaw(Sint32 linear)
{
    int	seg;
    Uint8 aval, mask;
    static const Sint32 seg_aend[NSEGS] =
    {
        0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff
    };

    linear >>= 3;
    if(linear >= 0)
        mask = 0xd5;		/* sign (7th) bit = 1 */
    else
    {
        mask = 0x55;		/* sign bit = 0 */
        linear = -linear - 1;
    } /* else */

    /* Convert the scaled magnitude to segment number. */
    for (seg = 0; (seg < NSEGS) && (linear > seg_aend[seg]); seg++);

    /* Combine the sign, segment, and quantization bits. */
    if (seg >= NSEGS)    /* out of range, return maximum value. */
        return (Uint8) (0x7F ^ mask);

    aval = (Uint8) seg << SEG_SHIFT;
    if (seg < 2)
        aval |= (linear >> 1) & QUANT_MASK;
    else
        aval |= (linear >> seg) & QUANT_MASK;

    return (aval ^ mask);
} /* Slinear2alaw */


/* convert from signed ints to a given type and write */
static Uint32 put_to_buffers(Sound_Sample *sample, Uint32 bw)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    shn_t *shn = (shn_t *) internal->decoder_private;
    int i, chan;
    Sint32 *data0 = shn->buffer[0];
    Sint32 nitem = shn->blocksize;
    int datasize = ((sample->actual.format & 0xFF) / 8);
    Uint32 bsiz = shn->nchan * nitem * datasize;

    SDL_assert(shn->backBufLeft == 0);

    if (shn->backBufferSize < bsiz)
    {
        void *rc = SDL_realloc(shn->backBuffer, bsiz);
        if (rc == NULL)
        {
            sample->flags |= SOUND_SAMPLEFLAG_ERROR;
            BAIL_MACRO(ERR_OUT_OF_MEMORY, 0);
        } /* if */
        shn->backBuffer = (Uint8 *) rc;
        shn->backBufferSize = bsiz;
    } /* if */

    switch (shn->datatype)
    {
        case SHN_TYPE_AU1: /* leave the conversion to fix_bitshift() */
        case SHN_TYPE_AU2:
        {
            Uint8 *writebufp = shn->backBuffer;
            if (shn->nchan == 1)
            {
                for (i = 0; i < nitem; i++)
                    *writebufp++ = data0[i];
            } /* if */
            else
            {
                for (i = 0; i < nitem; i++)
                {
                    for (chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = shn->buffer[chan][i];
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_U8:
        {
            Uint8 *writebufp = shn->backBuffer;
            if (shn->nchan == 1)
            {
                for (i = 0; i < nitem; i++)
                    *writebufp++ = CAPMAXUCHAR(data0[i]);
            } /* if */
            else
            {
                for (i = 0; i < nitem; i++)
                {
                    for (chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = CAPMAXUCHAR(shn->buffer[chan][i]);
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_S8:
        {
            Sint8 *writebufp = (Sint8 *) shn->backBuffer;
            if (shn->nchan == 1)
            {
                for(i = 0; i < nitem; i++)
                    *writebufp++ = CAPMAXSCHAR(data0[i]);
            } /* if */
            else
            {
                for(i = 0; i < nitem; i++)
                {
                    for(chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = CAPMAXSCHAR(shn->buffer[chan][i]);
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_S16HL:
        case SHN_TYPE_S16LH:
        {
            Sint16 *writebufp = (Sint16 *) shn->backBuffer;
            if (shn->nchan == 1)
            {
                for (i = 0; i < nitem; i++)
                    *writebufp++ = CAPMAXSHORT(data0[i]);
            } /* if */
            else
            {
                for (i = 0; i < nitem; i++)
                {
                    for (chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = CAPMAXSHORT(shn->buffer[chan][i]);
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_U16HL:
        case SHN_TYPE_U16LH:
        {
            Uint16 *writebufp = (Uint16 *) shn->backBuffer;
            if (shn->nchan == 1)
            {
                for (i = 0; i < nitem; i++)
                    *writebufp++ = CAPMAXUSHORT(data0[i]);
            } /* if */
            else
            {
                for (i = 0; i < nitem; i++)
                {
                    for (chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = CAPMAXUSHORT(shn->buffer[chan][i]);
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_ULAW:
        {
            Uint8 *writebufp = shn->backBuffer;
            if (shn->nchan == 1)
            {
                for(i = 0; i < nitem; i++)
                    *writebufp++ = Slinear2ulaw(CAPMAXSHORT((data0[i] << 3)));
            } /* if */
            else
            {
                for(i = 0; i < nitem; i++)
                {
                    for(chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = Slinear2ulaw(CAPMAXSHORT((shn->buffer[chan][i] << 3)));
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_AU3:
        {
            Uint8 *writebufp = shn->backBuffer;
            if (shn->nchan == 1)
            {
                for (i = 0; i < nitem; i++)
                    if(data0[i] < 0)
                        *writebufp++ = (127 - data0[i]) ^ 0xd5;
                    else
                        *writebufp++ = (data0[i] + 128) ^ 0x55;
            } /* if */
            else
            {
                for (i = 0; i < nitem; i++)
                {
                    for (chan = 0; chan < shn->nchan; chan++)
                    {
                        if (shn->buffer[chan][i] < 0)
                            *writebufp++ = (127 - shn->buffer[chan][i]) ^ 0xd5;
                        else
                            *writebufp++ = (shn->buffer[chan][i] + 128) ^ 0x55;
                    } /* for */
                } /* for */
            } /* else */
        } /* case */
        break;

        case SHN_TYPE_ALAW:
        {
            Uint8 *writebufp = shn->backBuffer;
            if (shn->nchan == 1)
            {
                for (i = 0; i < nitem; i++)
                    *writebufp++ = Slinear2alaw(CAPMAXSHORT((data0[i] << 3)));
            } /* if */
            else
            {
                for (i = 0; i < nitem; i++)
                {
                    for(chan = 0; chan < shn->nchan; chan++)
                        *writebufp++ = Slinear2alaw(CAPMAXSHORT((shn->buffer[chan][i] << 3)));
                } /* for */
            }/* else */
        } /* case */
        break;
    } /* switch */

    i = MIN_MACRO(internal->buffer_size - bw, bsiz);
    SDL_memcpy((char *)internal->buffer + bw, shn->backBuffer, i);
    shn->backBufLeft = bsiz - i;
    SDL_memcpy(shn->backBuffer, shn->backBuffer + i, shn->backBufLeft);
    return i;
} /* put_to_buffers */


#define ROUNDEDSHIFTDOWN(x, n) (((n) == 0) ? (x) : ((x) >> ((n) - 1)) >> 1)

static Uint32 SHN_read(Sound_Sample *sample)
{
    Uint32 retval = 0;
    Sint32 chan = 0;
    Uint32 cpyBytes = 0;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;
    shn_t *shn = (shn_t *) internal->decoder_private;
    Sint32 cmd;

    SDL_assert(shn->backBufLeft >= 0);

        /* see if there are leftovers to copy... */
    if (shn->backBufLeft > 0)
    {
        retval = MIN_MACRO(shn->backBufLeft, internal->buffer_size);
        SDL_memcpy(internal->buffer, shn->backBuffer, retval);
        shn->backBufLeft -= retval;
        SDL_memcpy(shn->backBuffer, shn->backBuffer + retval, shn->backBufLeft);
    } /* if */

    SDL_assert((shn->backBufLeft == 0) || (retval == internal->buffer_size));

    /* get commands from file and execute them */
    while (retval < internal->buffer_size)
    {
        if (!uvar_get(SHN_FNSIZE, shn, rw, &cmd))
        {
            sample->flags |= SOUND_SAMPLEFLAG_ERROR;
            return retval;
        } /* if */

        if (cmd == SHN_FN_QUIT)
        {
            sample->flags |= SOUND_SAMPLEFLAG_EOF;
            return retval;
        } /* if */

        switch(cmd)
        {
            case SHN_FN_ZERO:
            case SHN_FN_DIFF0:
            case SHN_FN_DIFF1:
            case SHN_FN_DIFF2:
            case SHN_FN_DIFF3:
            case SHN_FN_QLPC:
            {
                Sint32 i;
                Sint32 coffset, *cbuffer = shn->buffer[chan];
                Sint32 resn = 0, nlpc, j;

                if (cmd != SHN_FN_ZERO)
                {
                    if (!uvar_get(SHN_ENERGYSIZE, shn, rw, &resn))
                    {
                        sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                        return retval;
                    } /* if */

                    /* version 0 differed in definition of var_get */
                    if (shn->version == 0)
                        resn--;
                } /* if */

                /* find mean offset : N.B. this code duplicated */
                if (shn->nmean == 0)
                    coffset = shn->offset[chan][0];
                else
                {
                    Sint32 sum = (shn->version < 2) ? 0 : shn->nmean / 2;
                    for (i = 0; i < shn->nmean; i++)
                        sum += shn->offset[chan][i];

                    if (shn->version < 2)
                        coffset = sum / shn->nmean;
                    else
                        coffset = ROUNDEDSHIFTDOWN(sum / shn->nmean, shn->bitshift);
                } /* else */

                switch (cmd)
                {
                    case SHN_FN_ZERO:
                        for (i = 0; i < shn->blocksize; i++)
                            cbuffer[i] = 0;
                        break;

                    case SHN_FN_DIFF0:
                        for(i = 0; i < shn->blocksize; i++)
                        {
                            if (!var_get(resn, shn, rw, &cbuffer[i]))
                            {
                                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                                return retval;
                            } /* if */
                            cbuffer[i] += coffset;
                        } /* for */
                        break;

                    case SHN_FN_DIFF1:
                        for(i = 0; i < shn->blocksize; i++)
                        {
                            if (!var_get(resn, shn, rw, &cbuffer[i]))
                            {
                                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                                return retval;
                            } /* if */
                            cbuffer[i] += cbuffer[i - 1];
                        } /* for */
                        break;

                    case SHN_FN_DIFF2:
                        for (i = 0; i < shn->blocksize; i++)
                        {
                            if (!var_get(resn, shn, rw, &cbuffer[i]))
                            {
                                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                                return retval;
                            } /* if */
                            cbuffer[i] += (2 * cbuffer[i-1] - cbuffer[i-2]);
                        } /* for */
                        break;

                    case SHN_FN_DIFF3:
                        for (i = 0; i < shn->blocksize; i++)
                        {
                            if (!var_get(resn, shn, rw, &cbuffer[i]))
                            {
                                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                                return retval;
                            } /* if */
                            cbuffer[i] += 3 * (cbuffer[i - 1] - cbuffer[i - 2]) + cbuffer[i - 3];
                        } /* for */
                        break;

                    case SHN_FN_QLPC:
                        if (!uvar_get(SHN_LPCQSIZE, shn, rw, &nlpc))
                        {
                            sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                            return retval;
                        } /* if */

                        for(i = 0; i < nlpc; i++)
                        {
                            if (!var_get(SHN_LPCQUANT, shn, rw, &shn->qlpc[i]))
                            {
                                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                                return retval;
                            } /* if */
                        } /* for */

                        for(i = 0; i < nlpc; i++)
                            cbuffer[i - nlpc] -= coffset;

                        for(i = 0; i < shn->blocksize; i++)
                        {
                            Sint32 sum = shn->lpcqoffset;

                            for(j = 0; j < nlpc; j++)
                                sum += shn->qlpc[j] * cbuffer[i - j - 1];

                            if (!var_get(resn, shn, rw, &cbuffer[i]))
                            {
                                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                                return retval;
                            } /* if */
                            cbuffer[i] += (sum >> SHN_LPCQUANT);
                        } /* for */

                        if (coffset != 0)
                        {
                            for(i = 0; i < shn->blocksize; i++)
                                cbuffer[i] += coffset;
                        } /* if */

                        break;
                } /* switch */

                /* store mean value if appropriate : N.B. Duplicated code */
                if (shn->nmean > 0)
                {
                    Sint32 sum = (shn->version < 2) ? 0 : shn->blocksize / 2;
                    for (i = 0; i < shn->blocksize; i++)
                        sum += cbuffer[i];

                    for(i = 1; i < shn->nmean; i++)
                        shn->offset[chan][i - 1] = shn->offset[chan][i];

                    if (shn->version < 2)
                        shn->offset[chan][shn->nmean - 1] = sum / shn->blocksize;
                    else
                        shn->offset[chan][shn->nmean - 1] = (sum / shn->blocksize) << shn->bitshift;
                } /* if */

                /* do the wrap */
                for(i = -shn->nwrap; i < 0; i++)
                    cbuffer[i] = cbuffer[i + shn->blocksize];

                fix_bitshift(cbuffer, shn->blocksize, shn->bitshift, shn->datatype);

                if (chan == shn->nchan - 1)
                {
                    retval += put_to_buffers(sample, retval);
                    if (sample->flags & SOUND_SAMPLEFLAG_ERROR)
                        return retval;
                } /* if */

                chan = (chan + 1) % shn->nchan;
                break;
            } /* case */

            case SHN_FN_BLOCKSIZE:
                if (!uint_get((int) (SDL_log((double) shn->blocksize) / M_LN2),
                              shn, rw, &shn->blocksize))
                {
                    sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                    return retval;
                } /* if */
                break;

            case SHN_FN_BITSHIFT:
                if (!uvar_get(SHN_BITSHIFTSIZE, shn, rw, &shn->bitshift))
                {
                    sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                    return retval;
                } /* if */
                break;

            case SHN_FN_VERBATIM:
            default:
                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                BAIL_MACRO("SHN: Unhandled function.", retval);
        } /* switch */
    } /* while */

    return retval;
} /* SHN_read */


static int SHN_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    shn_t *shn = (shn_t *) internal->decoder_private;

#if 0
    int rc = SDL_RWseek(internal->rw, shn->start_pos, SEEK_SET);
    BAIL_IF_MACRO(rc != shn->start_pos, ERR_IO_ERROR, 0);
    /* !!! FIXME: set state. */
    return 1;
#else
    /*
     * !!! FIXME: This is really unacceptable; state should be reset and
     * !!! FIXME:  the RWops should be pointed to the start of the data
     * !!! FIXME:  to decode. The below kludge adds unneeded overhead and
     * !!! FIXME:  risk of failure.
     */
    BAIL_IF_MACRO(SDL_RWseek(internal->rw, 0, SEEK_SET) != 0, ERR_IO_ERROR, 0);
    SHN_close(sample);
    return SHN_open(sample, "SHN");
#endif
} /* SHN_rewind */


static int SHN_seek(Sound_Sample *sample, Uint32 ms)
{
    /*
     * (This CAN be done for SHNs that have a seek table at the end of the
     *  stream, btw.)
     */
    BAIL_MACRO("SHN: Seeking not implemented", 0);
} /* SHN_seek */


static const char *extensions_shn[] = { "SHN", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_SHN =
{
    {
        extensions_shn,
        "Shorten-compressed audio data",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/SDL_sound/"
    },

    SHN_init,       /*   init() method */
    SHN_quit,       /*   quit() method */
    SHN_open,       /*   open() method */
    SHN_close,      /*  close() method */
    SHN_read,       /*   read() method */
    SHN_rewind,     /* rewind() method */
    SHN_seek        /*   seek() method */
};

#endif  /* defined SOUND_SUPPORTS_SHN */

/* end of SDL_sound_shn.c ... */

