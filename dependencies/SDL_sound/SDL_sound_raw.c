/**
 * SDL_sound; A sound processing toolkit.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/*
 * RAW decoder for SDL_sound. This is as simple as it gets.
 *
 * This driver handles raw audio data. You must, regardless of where the
 *  data is actually coming from, specify the string "RAW" in the extension
 *  parameter of Sound_NewSample() (or, alternately, open a file with the
 *  extension ".raw" in Sound_NewSampleFromFile()). The string is checked
 *  case-insensitive. We need this check, because raw data, being raw, has
 *  no headers or magic number we can use to determine if we should handle a
 *  given file, so we needed some way to have this "decoder" discriminate.
 *
 * When calling Sound_NewSample*(), you must also specify a "desired"
 *  audio format. The "actual" format will always match what you specify, so
 *  there will be no conversion overhead, but these routines need to know how
 *  to treat the bits, since it's all random garbage otherwise.
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_RAW

static SDL_bool RAW_init(void)
{
    return SDL_TRUE;  /* always succeeds. */
} /* RAW_init */


static void RAW_quit(void)
{
    /* it's a no-op. */
} /* RAW_quit */


static int RAW_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = sample->opaque;
    Sint64 pos;
    Uint32 sample_rate;

        /*
         * We check this explicitly, since we have no other way to
         *  determine whether we should handle this data or not.
         */
    if (!ext || SDL_strcasecmp(ext, "RAW") != 0)
        BAIL_MACRO("RAW: extension isn't explicitly \"RAW\".", 0);

        /*
         * You must also specify a desired format, so we know how to
         *  treat the bits that are otherwise binary garbage.
         */
    if ( (sample->desired.channels < 1)  ||
         (sample->desired.channels > 2)  ||
         (sample->desired.rate == 0)     ||
         (sample->desired.format == 0) )
    {
        BAIL_MACRO("RAW: invalid desired format.", 0);
    } /* if */

    SNDDBG(("RAW: Accepting data stream.\n"));

        /*
         * We never convert raw samples; what you ask for is what you get.
         */
    SDL_memcpy(&sample->actual, &sample->desired, sizeof (Sound_AudioInfo));
    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;

    if ((pos = SDL_RWseek(internal->rw, 0, RW_SEEK_END)) <= 0) {
        BAIL_MACRO("RAW: can't seek to the end of the file.", 0);
    }
    if ( SDL_RWseek(internal->rw, 0, RW_SEEK_SET) != 0) {
        BAIL_MACRO("RAW: can't reset file.", 0);
    }

    sample_rate = (sample->actual.rate * sample->actual.channels * ((sample->actual.format & 0x0018) >> 3));
    internal->total_time = ( pos ) / sample_rate * 1000;
    internal->total_time += (pos % sample_rate) * 1000 / sample_rate;

    return 1; /* we'll handle this data. */
} /* RAW_open */


static void RAW_close(Sound_Sample *sample)
{
    /* we don't allocate anything that we need to free. That's easy, eh? */
} /* RAW_close */


static Uint32 RAW_read(Sound_Sample *sample)
{
    Uint32 retval;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

        /*
         * We don't actually do any decoding, so we read the raw data
         *  directly into the internal buffer...
         */
    retval = SDL_RWread(internal->rw, internal->buffer,
                        1, internal->buffer_size);

        /* Make sure the read went smoothly... */
    if (retval == 0)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (retval == -1) /** FIXME: this error check is broken **/
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

        /* (next call this EAGAIN may turn into an EOF or error.) */
    else if (retval < internal->buffer_size)
        sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

    return retval;
} /* RAW_read */


static int RAW_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    BAIL_IF_MACRO(SDL_RWseek(internal->rw, 0, RW_SEEK_SET) != 0, ERR_IO_ERROR, 0);
    return 1;
} /* RAW_rewind */


static int RAW_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    const Sint64 pos = __Sound_convertMsToBytePos(&sample->actual, ms);
    const int err = (SDL_RWseek(internal->rw, pos, RW_SEEK_SET) != pos);
    BAIL_IF_MACRO(err, ERR_IO_ERROR, 0);
    return 1;
} /* RAW_seek */

static const char *extensions_raw[] = { "RAW", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_RAW =
{
    {
        extensions_raw,
        "Raw audio",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/SDL_sound/"
    },

    RAW_init,       /*   init() method */
    RAW_quit,       /*   quit() method */
    RAW_open,       /*   open() method */
    RAW_close,      /*  close() method */
    RAW_read,       /*   read() method */
    RAW_rewind,     /* rewind() method */
    RAW_seek        /*   seek() method */
};

#endif /* SOUND_SUPPORTS_RAW */

/* end of SDL_sound_raw.c ... */

