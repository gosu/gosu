/**
 * SDL_sound; An abstract sound format decoding API.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/*
 * FLAC decoder for SDL_sound.
 *
 * FLAC stands for Free Lossless Audio Codec. It's lossless and
 *  high-resolution, so it's become popular with tapers and audiophiles, at
 *  the cost of a much larger filesize than lossy things like Ogg, MP3, AAC,
 *  etc. More details can be found at https://xiph.org/flac/ ...
 *
 * This doesn't use libFLAC from xiph.org, but rather dr_flac, a public
 *  domain, single-header library, for decoding. It handles both FLAC and
 *  "Ogg FLAC" (FLAC codec wrapped in an Ogg container) files.
 *
 * dr_flac is here: https://github.com/mackron/dr_libs/
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_FLAC

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO 1
#define DR_FLAC_NO_WIN32_IO 1
#define DR_FLAC_NO_CRC 1
#define DRFLAC_ASSERT(x) SDL_assert((x))
#define DRFLAC_MALLOC(sz) SDL_malloc((sz))
#define DRFLAC_REALLOC(p, sz) SDL_realloc((p), (sz))
#define DRFLAC_FREE(p) SDL_free((p))
#define DRFLAC_COPY_MEMORY(dst, src, sz) SDL_memcpy((dst), (src), (sz))
#define DRFLAC_ZERO_MEMORY(p, sz) SDL_memset((p), 0, (sz))
#include "dr_flac.h"

static size_t flac_read(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    Uint8 *ptr = (Uint8 *) pBufferOut;
    Sound_Sample *sample = (Sound_Sample *) pUserData;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rwops = internal->rw;
    size_t retval = 0;

    /* !!! FIXME: dr_flac treats returning less than bytesToRead as EOF. So we can't EAGAIN. */
    while (retval < bytesToRead)
    {
        const size_t rc = SDL_RWread(rwops, ptr, 1, bytesToRead);
        if (rc == 0)
        {
            sample->flags |= SOUND_SAMPLEFLAG_EOF;
            break;
        } /* if */
        else if (retval == -1)
        {
            sample->flags |= SOUND_SAMPLEFLAG_ERROR;
            break;
        } /* else if */
        else
        {
            retval += rc;
            ptr += rc;
        } /* else */
    } /* while */

    return retval;
} /* flac_read */

static drflac_bool32 flac_seek(void* pUserData, int offset, drflac_seek_origin origin)
{
    const int whence = (origin == drflac_seek_origin_start) ? RW_SEEK_SET : RW_SEEK_CUR;
    Sound_Sample *sample = (Sound_Sample *) pUserData;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    return (SDL_RWseek(internal->rw, offset, whence) != -1) ? DRFLAC_TRUE : DRFLAC_FALSE;
} /* flac_seek */


static int FLAC_init(void)
{
    return 1;  /* always succeeds. */
} /* FLAC_init */


static void FLAC_quit(void)
{
    /* it's a no-op. */
} /* FLAC_quit */


static int FLAC_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = drflac_open(flac_read, flac_seek, sample);

    if (!dr)
    {
        BAIL_IF_MACRO(sample->flags & SOUND_SAMPLEFLAG_ERROR, ERR_IO_ERROR, 0);
        BAIL_MACRO("FLAC: Not a FLAC stream.", 0);
    } /* if */

    SNDDBG(("FLAC: Accepting data stream.\n"));
    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;

    sample->actual.channels = dr->channels;
    sample->actual.rate = dr->sampleRate;
    sample->actual.format = AUDIO_S32SYS;  /* dr_flac only does Sint32. */

    if (dr->totalSampleCount == 0)
        internal->total_time = -1;
    else
    {
        const Uint32 rate = (Uint32) dr->sampleRate;
        const Uint64 frames = (Uint64) (dr->totalSampleCount / dr->channels);
        internal->total_time = (frames / rate) * 1000;
        internal->total_time += ((dr->totalSampleCount % dr->sampleRate) * 1000) / dr->sampleRate;
    } /* else */

    internal->decoder_private = dr;

    return 1;
} /* FLAC_open */

static void FLAC_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = (drflac *) internal->decoder_private;
    drflac_close(dr);
} /* FLAC_close */

static Uint32 FLAC_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = (drflac *) internal->decoder_private;
    const drflac_uint64 rc = drflac_read_s32(dr, internal->buffer_size / sizeof (drflac_int32), (drflac_int32 *) internal->buffer);
    /* !!! FIXME: the flac_read callback sets ERROR and EOF flags, but this only tells you about i/o errors, not corruption. */
    return rc * sizeof (drflac_int32);
} /* FLAC_read */

static int FLAC_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = (drflac *) internal->decoder_private;
    return (drflac_seek_to_sample(dr, 0) == DRFLAC_TRUE);
} /* FLAC_rewind */

static int FLAC_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = (drflac *) internal->decoder_private;
    const float frames_per_ms = ((float) sample->actual.rate) / 1000.0f;
    const drflac_uint64 frame_offset = (drflac_uint64) (frames_per_ms * ((float) ms));
    const drflac_uint64 sampnum = frame_offset * sample->actual.channels;
    return (drflac_seek_to_sample(dr, sampnum) == DRFLAC_TRUE);
} /* FLAC_seek */

static const char *extensions_flac[] = { "FLAC", "FLA", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_FLAC =
{
    {
        extensions_flac,
        "Free Lossless Audio Codec",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/SDL_sound/"
    },

    FLAC_init,       /*   init() method */
    FLAC_quit,       /*   quit() method */
    FLAC_open,       /*   open() method */
    FLAC_close,      /*  close() method */
    FLAC_read,       /*   read() method */
    FLAC_rewind,     /* rewind() method */
    FLAC_seek        /*   seek() method */
};

#endif /* SOUND_SUPPORTS_FLAC */

/* end of SDL_sound_flac.c ... */

