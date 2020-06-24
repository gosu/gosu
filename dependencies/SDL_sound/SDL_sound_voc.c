/**
 * SDL_sound; A sound processing toolkit.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/*
 * VOC decoder for SDL_sound.
 *
 * This driver handles Creative Labs VOC audio data...this is a legacy format,
 *  but there's some game ports that could make use of such a decoder. Plus,
 *  VOC is fairly straightforward to decode, so this is a more complex, but
 *  still palatable example of an SDL_sound decoder. Y'know, in case the
 *  RAW decoder didn't do it for you.  :)
 *
 * This code was ripped from a decoder I had written for SDL_mixer, which was
 *  largely ripped from sox v12.17.1's voc.c.
 *
 *    SDL_mixer: https://www.libsdl.org/projects/SDL_mixer/
 *    sox: http://www.freshmeat.net/projects/sox/
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_VOC

/* Private data for VOC file */
typedef struct vocstuff {
    Uint32  rest;           /* bytes remaining in current block */
    Uint32  rate;           /* rate code (byte) of this chunk */
    int     silent;         /* sound or silence? */
    Uint32  srate;          /* rate code (byte) of silence */
    Uint32  blockseek;	    /* start of current output block */
    Uint32  samples;	    /* number of samples output */
    Uint32  size;           /* word length of data */
    Uint8   channels;       /* number of sound channels */
    int     extended;       /* Has an extended block been read? */
    Uint32  bufpos;         /* byte position in internal->buffer. */
    Uint32  start_pos;      /* offset to seek to in stream when rewinding. */
    int     error;          /* error condition (as opposed to EOF). */
} vs_t;


/* Size field */ 
/* SJB: note that the 1st 3 are sometimes used as sizeof(type) */
#define ST_SIZE_BYTE     1
#define ST_SIZE_8BIT     1
#define ST_SIZE_WORD     2
#define ST_SIZE_16BIT    2
#define ST_SIZE_DWORD    4
#define ST_SIZE_32BIT    4
#define ST_SIZE_FLOAT    5
#define ST_SIZE_DOUBLE   6
#define ST_SIZE_IEEE     7   /* IEEE 80-bit floats. */

/* Style field */
#define ST_ENCODING_UNSIGNED  1 /* unsigned linear: Sound Blaster */
#define ST_ENCODING_SIGN2     2 /* signed linear 2's comp: Mac */
#define ST_ENCODING_ULAW      3 /* U-law signed logs: US telephony, SPARC */
#define ST_ENCODING_ALAW      4 /* A-law signed logs: non-US telephony */
#define ST_ENCODING_ADPCM     5 /* Compressed PCM */
#define ST_ENCODING_IMA_ADPCM 6 /* Compressed PCM */
#define ST_ENCODING_GSM       7 /* GSM 6.10 33-byte frame lossy compression */

#define VOC_TERM      0
#define VOC_DATA      1
#define VOC_CONT      2
#define VOC_SILENCE   3
#define VOC_MARKER    4
#define VOC_TEXT      5
#define VOC_LOOP      6
#define VOC_LOOPEND   7
#define VOC_EXTENDED  8
#define VOC_DATA_16   9


static int VOC_init(void)
{
    return 1;  /* always succeeds. */
} /* VOC_init */


static void VOC_quit(void)
{
    /* it's a no-op. */
} /* VOC_quit */


static SDL_INLINE int voc_readbytes(SDL_RWops *src, vs_t *v, void *p, int size)
{
    if (SDL_RWread(src, p, size, 1) != 1)
    {
        v->error = 1;
        BAIL_MACRO("VOC: i/o error", 0);
    } /* if */

    return 1;
} /* voc_readbytes */


static SDL_INLINE int voc_check_header(SDL_RWops *src)
{
    /* VOC magic header */
    Uint8  signature[20];  /* "Creative Voice File\032" */
    Uint16 datablockofs;
    vs_t v; /* dummy struct for voc_readbytes */

    if (!voc_readbytes(src, &v, signature, sizeof (signature)))
        return 0;

    if (SDL_memcmp(signature, "Creative Voice File\032", sizeof (signature)) != 0)
    {
        BAIL_MACRO("VOC: Wrong signature; not a VOC file.", 0);
    } /* if */

        /* get the offset where the first datablock is located */
    if (!voc_readbytes(src, &v, &datablockofs, sizeof (Uint16)))
        return 0;

    datablockofs = SDL_SwapLE16(datablockofs);

    if (SDL_RWseek(src, datablockofs, SEEK_SET) != datablockofs)
    {
        BAIL_MACRO("VOC: Failed to seek to data block.", 0);
    } /* if */

    return 1;  /* success! */
} /* voc_check_header */


/* Read next block header, save info, leave position at start of data */
static int voc_get_block(Sound_Sample *sample, vs_t *v)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *src = internal->rw;
    Uint8 bits24[3];
    Uint8 uc, block;
    Uint32 sblen;
    Uint16 new_rate_short;
    Uint32 new_rate_long;
    Uint8 trash[6];
    Uint16 period;
    Uint32 bytes_per_second;
    int i;

    v->silent = 0;
    while (v->rest == 0)
    {
        if (SDL_RWread(src, &block, sizeof (block), 1) != 1)
            return 1;  /* assume that's the end of the file. */

        if (block == VOC_TERM)
            return 1;

        if (SDL_RWread(src, bits24, sizeof (bits24), 1) != 1)
            return 1;  /* assume that's the end of the file. */
        
        /* Size is an 24-bit value. Ugh. */
        sblen = ( (bits24[0]) | (bits24[1] << 8) | (bits24[2] << 16) );

        switch(block)
        {
            case VOC_DATA:
                if (!voc_readbytes(src, v, &uc, sizeof (uc)))
                    return 0;

                /* When DATA block preceeded by an EXTENDED     */
                /* block, the DATA blocks rate value is invalid */
                if (!v->extended)
                {
                    BAIL_IF_MACRO(uc == 0, "VOC: Sample rate is zero?", 0);

                    if ((v->rate != -1) && (uc != v->rate))
                        BAIL_MACRO("VOC sample rate codes differ", 0);

                    v->rate = uc;
                    sample->actual.rate = 1000000.0/(256 - v->rate);
                    sample->actual.channels = 1;
                    v->channels = 1;
                } /* if */

                if (!voc_readbytes(src, v, &uc, sizeof (uc)))
                    return 0;

                BAIL_IF_MACRO(uc != 0, "VOC: only supports 8-bit data", 0);

                v->extended = 0;
                v->rest = sblen - 2;
                v->size = ST_SIZE_BYTE;

                bytes_per_second = sample->actual.rate
                    * sample->actual.channels;
                internal->total_time += ( v->rest ) / bytes_per_second * 1000;
                internal->total_time += (v->rest % bytes_per_second) * 1000
                                            / bytes_per_second;
                return 1;

            case VOC_DATA_16:
                if (!voc_readbytes(src, v, &new_rate_long, sizeof (Uint32)))
                    return 0;

                new_rate_long = SDL_SwapLE32(new_rate_long);
                BAIL_IF_MACRO(!new_rate_long, "VOC: Sample rate is zero?", 0);

                if ((v->rate != -1) && (new_rate_long != v->rate))
                    BAIL_MACRO("VOC: sample rate codes differ", 0);

                v->rate = new_rate_long;
                sample->actual.rate = new_rate_long;

                if (!voc_readbytes(src, v, &uc, sizeof (uc)))
                    return 0;

                switch (uc)
                {
                    case 8:  v->size = ST_SIZE_BYTE; break;
                    case 16: v->size = ST_SIZE_WORD; break;
                    default:
                        BAIL_MACRO("VOC: unknown data size", 0);
                } /* switch */

                if (!voc_readbytes(src, v, &v->channels, sizeof (Uint8)))
                    return 0;

                if (!voc_readbytes(src, v, trash, sizeof (Uint8) * 6))
                    return 0;
                v->rest = sblen - 12;

                bytes_per_second = ((v->size == ST_SIZE_WORD) ? (2) : (1)) *
                                    sample->actual.rate * v->channels;
                internal->total_time += v->rest / bytes_per_second * 1000;
                internal->total_time += ( v->rest % bytes_per_second ) * 1000
                                            / bytes_per_second;
                return 1;

            case VOC_CONT:
                v->rest = sblen;
                return 1;

            case VOC_SILENCE:
                if (!voc_readbytes(src, v, &period, sizeof (period)))
                    return 0;

                period = SDL_SwapLE16(period);

                if (!voc_readbytes(src, v, &uc, sizeof (uc)))
                    return 0;

                BAIL_IF_MACRO(uc == 0, "VOC: silence sample rate is zero", 0);

                /*
                 * Some silence-packed files have gratuitously
                 * different sample rate codes in silence.
                 * Adjust period.
                 */
                if ((v->rate != -1) && (uc != v->rate))
                    period = (period * (256 - uc))/(256 - v->rate);
                else
                    v->rate = uc;
                v->rest = period;
                v->silent = 1;

                internal->total_time += (period) / (v->rate) * 1000;
                internal->total_time += (period % v->rate) * 1000 / v->rate;
                return 1;

            case VOC_LOOP:
            case VOC_LOOPEND:
                for(i = 0; i < sblen; i++)   /* skip repeat loops. */
                {
                   if (!voc_readbytes(src, v, trash, sizeof (Uint8)))
                        return 0;
                } /* for */
                break;

            case VOC_EXTENDED:
                /* An Extended block is followed by a data block */
                /* Set this byte so we know to use the rate      */
                /* value from the extended block and not the     */
                /* data block.                     */
                v->extended = 1;
                if (!voc_readbytes(src, v, &new_rate_short, sizeof (Uint16)))
                    return 0;

                new_rate_short = SDL_SwapLE16(new_rate_short);
                BAIL_IF_MACRO(!new_rate_short, "VOC: sample rate is zero", 0);

                if ((v->rate != -1) && (new_rate_short != v->rate))
                   BAIL_MACRO("VOC: sample rate codes differ", 0);

                v->rate = new_rate_short;

                if (!voc_readbytes(src, v, &uc, sizeof (uc)))
                    return 0;

                BAIL_IF_MACRO(uc != 0, "VOC: only supports 8-bit data", 0);

                if (!voc_readbytes(src, v, &uc, sizeof (uc)))
                    return 0;

                if (uc)
                    sample->actual.channels = 2;  /* Stereo */

                /* Needed number of channels before finishing
                   compute for rate */
                sample->actual.rate =
                     (256000000L/(65536L - v->rate)) / sample->actual.channels;
                /* An extended block must be followed by a data */
                /* block to be valid so loop back to top so it  */
                /* can be grabed.                */
                continue;

            case VOC_MARKER:
                if (!voc_readbytes(src, v, trash, sizeof (Uint8) * 2))
                    return 0;

                /* Falling! Falling! */

            default:  /* text block or other krapola. */
                for(i = 0; i < sblen; i++)   /* skip repeat loops. */
                {
                   if (!voc_readbytes(src, v, trash, sizeof (Uint8)))
                        return 0;
                } /* for */

                if (block == VOC_TEXT)
                    continue;    /* get next block */
        } /* switch */
    } /* while */

    return 1;
} /* voc_get_block */


static int voc_read_waveform(Sound_Sample *sample, int fill_buf, Uint32 max)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *src = internal->rw;
    vs_t *v = (vs_t *) internal->decoder_private;
    int done = 0;
    Uint8 silence = 0x80;
    Uint8 *buf = internal->buffer;

    if (v->rest == 0)
    {
        if (!voc_get_block(sample, v))
            return 0;
    } /* if */

    if (v->rest == 0)
        return 0;

    max = (v->rest < max) ? v->rest : max;

    if (v->silent)
    {
        if (v->size == ST_SIZE_WORD)
            silence = 0x00;

        /* Fill in silence */
        if (fill_buf)
            SDL_memset(buf + v->bufpos, silence, max);

        done = max;
        v->rest -= done;
    } /* if */

    else
    {
        if (fill_buf)
        {
            done = SDL_RWread(src, buf + v->bufpos, 1, max);
            if (done < max)
            {
                __Sound_SetError("VOC: i/o error");
                sample->flags |= SOUND_SAMPLEFLAG_ERROR;
            } /* if */
        } /* if */

        else
        {
            int cur, rc;
            cur = SDL_RWtell(src);
            if (cur >= 0)
            {
                rc = SDL_RWseek(src, max, SEEK_CUR);
                if (rc >= 0)
                    done = rc - cur;
                else
                {
                    __Sound_SetError("VOC: seek error");
                    sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                } /* else */
            } /* if */
        } /* else */

        v->rest -= done;
        v->bufpos += done;
    } /* else */

    return done;
} /* voc_read_waveform */


static int VOC_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    vs_t *v = NULL;

    if (!voc_check_header(internal->rw))
        return 0;

    v = (vs_t *) SDL_calloc(1, sizeof (vs_t));
    BAIL_IF_MACRO(v == NULL, ERR_OUT_OF_MEMORY, 0);

    v->start_pos = SDL_RWtell(internal->rw);
    v->rate = -1;
    if (!voc_get_block(sample, v))
    {
        SDL_free(v);
        return 0;
    } /* if */

    if (v->rate == -1)
    {
        SDL_free(v);
        BAIL_MACRO("VOC: data had no sound!", 0);
    } /* if */

    SNDDBG(("VOC: Accepting data stream.\n"));
    sample->actual.format = (v->size == ST_SIZE_WORD) ? AUDIO_S16LSB:AUDIO_U8;
    sample->actual.channels = v->channels;
    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;
    internal->decoder_private = v;
    return 1;
} /* VOC_open */


static void VOC_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_free(internal->decoder_private);
} /* VOC_close */


static Uint32 VOC_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    vs_t *v = (vs_t *) internal->decoder_private;

    v->bufpos = 0;
    while (v->bufpos < internal->buffer_size)
    {
        Uint32 rc = voc_read_waveform(sample, 1, internal->buffer_size);
        if (rc == 0)
        {
            sample->flags |= (v->error) ? 
                                 SOUND_SAMPLEFLAG_ERROR :
                                 SOUND_SAMPLEFLAG_EOF;
            break;
        } /* if */

        if (!voc_get_block(sample, v))
        {
            sample->flags |= (v->error) ? 
                                 SOUND_SAMPLEFLAG_ERROR :
                                 SOUND_SAMPLEFLAG_EOF;
            break;
        } /* if */
    } /* while */

    return v->bufpos;
} /* VOC_read */


static int VOC_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    vs_t *v = (vs_t *) internal->decoder_private;
    int rc = SDL_RWseek(internal->rw, v->start_pos, SEEK_SET);
    BAIL_IF_MACRO(rc != v->start_pos, ERR_IO_ERROR, 0);
    v->rest = 0;
    return 1;
} /* VOC_rewind */


static int VOC_seek(Sound_Sample *sample, Uint32 ms)
{
    /*
     * VOCs don't lend themselves well to seeking, since you have to
     *  parse each section, which is an arbitrary size. The best we can do
     *  is rewind, set a flag saying not to write the waveforms to a buffer,
     *  and decode to the point that we want. Ugh. Fortunately, there's
     *  really no such thing as a large VOC, due to the era and hardware that
     *  spawned them, so even though this is inefficient, this is still a
     *  relatively fast operation in most cases.
     */

    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    vs_t *v = (vs_t *) internal->decoder_private;
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int origpos = SDL_RWtell(internal->rw);
    int origrest = v->rest;

    BAIL_IF_MACRO(!VOC_rewind(sample), NULL, 0);

    v->bufpos = 0;

    while (offset > 0)
    {
        Uint32 rc = voc_read_waveform(sample, 0, offset);
        if ( (rc == 0) || (!voc_get_block(sample, v)) )
        {
            SDL_RWseek(internal->rw, origpos, SEEK_SET);
            v->rest = origrest;
            return 0;
        } /* if */

        offset -= rc;
    } /* while */

    return 1;
} /* VOC_seek */


static const char *extensions_voc[] = { "VOC", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_VOC =
{
    {
        extensions_voc,
        "Creative Labs Voice format",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/SDL_sound/"
    },

    VOC_init,       /*   init() method */
    VOC_quit,       /*   quit() method */
    VOC_open,       /*   open() method */
    VOC_close,      /*  close() method */
    VOC_read,       /*   read() method */
    VOC_rewind,     /* rewind() method */
    VOC_seek        /*   seek() method */
};

#endif /* SOUND_SUPPORTS_VOC */

/* end of SDL_sound_voc.c ... */
