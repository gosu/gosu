/**
 * SDL_sound; An abstract sound format decoding API.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Mattias Engdegård.
 */

/*
 * Sun/NeXT .au decoder for SDL_sound.
 * Formats supported: 8 and 16 bit linear PCM, 8 bit µ-law.
 * Files without valid header are assumed to be 8 bit µ-law, 8kHz, mono.
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_AU

/* no init/deinit needed */
static int AU_init(void)
{
    return 1;
} /* AU_init */

static void AU_quit(void)
{
    /* no-op. */
} /* AU_quit */

struct au_file_hdr
{
    Uint32 magic;
    Uint32 hdr_size;
    Uint32 data_size;
    Uint32 encoding;
    Uint32 sample_rate;
    Uint32 channels;
};

#define HDR_SIZE 24

enum
{
    AU_ENC_ULAW_8       = 1,        /* 8-bit ISDN µ-law */
    AU_ENC_LINEAR_8     = 2,        /* 8-bit linear PCM */
    AU_ENC_LINEAR_16    = 3,        /* 16-bit linear PCM */

    /* the rest are unsupported (I have never seen them in the wild) */
    AU_ENC_LINEAR_24    = 4,        /* 24-bit linear PCM */
    AU_ENC_LINEAR_32    = 5,        /* 32-bit linear PCM  */
    AU_ENC_FLOAT        = 6,        /* 32-bit IEEE floating point */
    AU_ENC_DOUBLE       = 7,        /* 64-bit IEEE floating point */
    /* more Sun formats, not supported either */
    AU_ENC_ADPCM_G721   = 23,
    AU_ENC_ADPCM_G722   = 24,
    AU_ENC_ADPCM_G723_3 = 25,
    AU_ENC_ADPCM_G723_5 = 26,
    AU_ENC_ALAW_8       = 27
};

struct audec
{
    Uint32 total;
    Uint32 remaining;
    Uint32 start_offset;
    int encoding;
};


/*
 * Read in the AU header from disk. This makes this process safe
 *  regardless of the processor's byte order or how the au_file_hdr
 *  structure is packed.
 */
static int read_au_header(SDL_RWops *rw, struct au_file_hdr *hdr)
{
    if (SDL_RWread(rw, &hdr->magic, sizeof (hdr->magic), 1) != 1)
        return 0;
    hdr->magic = SDL_SwapBE32(hdr->magic);

    if (SDL_RWread(rw, &hdr->hdr_size, sizeof (hdr->hdr_size), 1) != 1)
        return 0;
    hdr->hdr_size = SDL_SwapBE32(hdr->hdr_size);

    if (SDL_RWread(rw, &hdr->data_size, sizeof (hdr->data_size), 1) != 1)
        return 0;
    hdr->data_size = SDL_SwapBE32(hdr->data_size);

    if (SDL_RWread(rw, &hdr->encoding, sizeof (hdr->encoding), 1) != 1)
        return 0;
    hdr->encoding = SDL_SwapBE32(hdr->encoding);

    if (SDL_RWread(rw, &hdr->sample_rate, sizeof (hdr->sample_rate), 1) != 1)
        return 0;
    hdr->sample_rate = SDL_SwapBE32(hdr->sample_rate);

    if (SDL_RWread(rw, &hdr->channels, sizeof (hdr->channels), 1) != 1)
        return 0;
    hdr->channels = SDL_SwapBE32(hdr->channels);

    return 1;
} /* read_au_header */


#define AU_MAGIC 0x2E736E64  /* ".snd", in ASCII (bigendian number) */

static int AU_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = sample->opaque;
    SDL_RWops *rw = internal->rw;
    int skip, hsize, i, bytes_per_second;
    struct au_file_hdr hdr;
    struct audec *dec;
    char c;

    /* read_au_header() will do byte order swapping. */
    BAIL_IF_MACRO(!read_au_header(rw, &hdr), "AU: bad header", 0);

    dec = SDL_malloc(sizeof *dec);
    BAIL_IF_MACRO(dec == NULL, ERR_OUT_OF_MEMORY, 0);
    internal->decoder_private = dec;

    if (hdr.magic == AU_MAGIC)
    {
        /* valid magic */
        dec->encoding = hdr.encoding;
        switch(dec->encoding)
        {
            case AU_ENC_ULAW_8:
                /* Convert 8-bit µ-law to 16-bit linear on the fly. This is
                   slightly wasteful if the audio driver must convert them
                   back, but µ-law only devices are rare (mostly _old_ Suns) */
                sample->actual.format = AUDIO_S16SYS;
                break;

            case AU_ENC_LINEAR_8:
                sample->actual.format = AUDIO_S8;
                break;

            case AU_ENC_LINEAR_16:
                sample->actual.format = AUDIO_S16MSB;
                break;

            default:
                SDL_free(dec);
                BAIL_MACRO("AU: Unsupported .au encoding", 0);
        } /* switch */

        sample->actual.rate = hdr.sample_rate;
        sample->actual.channels = hdr.channels;
        dec->remaining = hdr.data_size;
        hsize = hdr.hdr_size;

        /* skip remaining part of header (input may be unseekable) */
        for (i = HDR_SIZE; i < hsize; i++)
        {
            if (SDL_RWread(rw, &c, 1, 1) != 1)
            {
                SDL_free(dec);
                BAIL_MACRO(ERR_IO_ERROR, 0);
            } /* if */
        } /* for */
    } /* if */

    else if (SDL_strcasecmp(ext, "au") == 0)
    {
        /*
         * A number of files in the wild have the .au extension but no valid
         * header; these are traditionally assumed to be 8kHz µ-law. Handle
         * them here only if the extension is recognized.
         */

        SNDDBG(("AU: Invalid header, assuming raw 8kHz µ-law.\n"));
        /* if seeking fails, we lose 24 samples. big deal */
        SDL_RWseek(rw, -HDR_SIZE, SEEK_CUR);
        dec->encoding = AU_ENC_ULAW_8;
        dec->remaining = (Uint32)-1; 		/* no limit */
        sample->actual.format = AUDIO_S16SYS;
        sample->actual.rate = 8000;
        sample->actual.channels = 1;
    } /* else if */

    else
    {
        SDL_free(dec);
        BAIL_MACRO("AU: Not an .AU stream.", 0);
    } /* else */    

    bytes_per_second = ( ( dec->encoding == AU_ENC_LINEAR_16 ) ? 2 : 1 )
        * sample->actual.rate * sample->actual.channels ;
    internal->total_time = ((dec->remaining == -1) ? (-1) :
                            ( ( dec->remaining / bytes_per_second ) * 1000 ) +
                            ( ( dec->remaining % bytes_per_second ) * 1000 /
                              bytes_per_second ) );

    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;
    dec->total = dec->remaining;
    dec->start_offset = SDL_RWtell(rw);

    SNDDBG(("AU: Accepting data stream.\n"));
    return 1;
} /* AU_open */


static void AU_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = sample->opaque;
    SDL_free(internal->decoder_private);
} /* AU_close */


/* table to convert from µ-law encoding to signed 16-bit samples,
   generated by a throwaway perl script */
static Sint16 ulaw_to_linear[256] = {
    -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
    -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
    -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
    -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
     -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
     -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
     -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
     -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
     -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
     -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
      -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
      -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
      -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
      -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
      -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
       -56,   -48,   -40,   -32,   -24,   -16,    -8,     0,
     32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
     23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
     15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
     11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
      7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
      5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
      3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
      2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
      1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
      1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
       876,   844,   812,   780,   748,   716,   684,   652,
       620,   588,   556,   524,   492,   460,   428,   396,
       372,   356,   340,   324,   308,   292,   276,   260,
       244,   228,   212,   196,   180,   164,   148,   132,
       120,   112,   104,    96,    88,    80,    72,    64,
        56,    48,    40,    32,    24,    16,     8,     0
};


static Uint32 AU_read(Sound_Sample *sample)
{
    int ret;
    Sound_SampleInternal *internal = sample->opaque;
    struct audec *dec = internal->decoder_private;
    int maxlen;
    Uint8 *buf;

    maxlen = internal->buffer_size;
    buf = internal->buffer;
    if (dec->encoding == AU_ENC_ULAW_8)
    {
        /* We read µ-law samples into the second half of the buffer, so
           we can expand them to 16-bit samples afterwards */
        maxlen >>= 1;
        buf += maxlen;
    } /* if */

    if (maxlen > dec->remaining)
        maxlen = dec->remaining;
    ret = SDL_RWread(internal->rw, buf, 1, maxlen);
    if (ret == 0)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
    else if (ret == -1)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;
    else
    {
        dec->remaining -= ret;
        if (ret < maxlen)
            sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

        if (dec->encoding == AU_ENC_ULAW_8)
        {
            int i;
            Sint16 *dst = internal->buffer;
            for (i = 0; i < ret; i++)
                dst[i] = ulaw_to_linear[buf[i]];
            ret <<= 1;                  /* return twice as much as read */
        } /* if */
    } /* else */

    return ret;
} /* AU_read */


static int AU_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    struct audec *dec = (struct audec *) internal->decoder_private;
    int rc = SDL_RWseek(internal->rw, dec->start_offset, SEEK_SET);
    BAIL_IF_MACRO(rc != dec->start_offset, ERR_IO_ERROR, 0);
    dec->remaining = dec->total;
    return 1;
} /* AU_rewind */


static int AU_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    struct audec *dec = (struct audec *) internal->decoder_private;
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int rc;
    int pos;

    if (dec->encoding == AU_ENC_ULAW_8)
        offset >>= 1;  /* halve the byte offset for compression. */

    pos = (int) (dec->start_offset + offset);
    rc = SDL_RWseek(internal->rw, pos, SEEK_SET);
    BAIL_IF_MACRO(rc != pos, ERR_IO_ERROR, 0);
    dec->remaining = dec->total - offset;
    return 1;
} /* AU_seek */

/*
 * Sometimes the extension ".snd" is used for these files (mostly on the NeXT),
 * and the magic number comes from this. However it may clash with other
 * formats and is somewhat of an anachronism, so only .au is used here.
 */
static const char *extensions_au[] = { "AU", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_AU =
{
    {
        extensions_au,
        "Sun/NeXT audio file format",
        "Mattias EngdegÃ¥rd <f91-men@nada.kth.se>",
        "https://icculus.org/SDL_sound/"
    },

    AU_init,        /*   init() method */
    AU_quit,        /*   quit() method */
    AU_open,        /*   open() method */
    AU_close,       /*  close() method */
    AU_read,        /*   read() method */
    AU_rewind,      /* rewind() method */
    AU_seek         /*   seek() method */
};

#endif /* SOUND_SUPPORTS_AU */

/* end of SDL_sound_au.c ... */

