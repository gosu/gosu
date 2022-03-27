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
#define DRFLAC_ASSERT(x) SDL_assert((x))
#define DRFLAC_MALLOC(sz) SDL_malloc((sz))
#define DRFLAC_REALLOC(p, sz) SDL_realloc((p), (sz))
#define DRFLAC_FREE(p) SDL_free((p))
#define DRFLAC_COPY_MEMORY(dst, src, sz) SDL_memcpy((dst), (src), (sz))
#define DRFLAC_ZERO_MEMORY(p, sz) SDL_memset((p), 0, (sz))
#ifndef __WATCOMC__ /* #@!.!.. */
#if SDL_VERSION_ATLEAST(2, 0, 9)
#define exp SDL_exp
#endif
#define floor SDL_floor
#endif

#include "dr_flac.h"

static size_t flac_read(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    /* !!! FIXME: dr_flac treats returning less than bytesToRead as EOF. So we can't EAGAIN. */
    Uint8 *ptr = (Uint8 *) pBufferOut;
    Sound_Sample *sample = (Sound_Sample *) pUserData;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rwops = internal->rw;
    size_t retval = 0;

    /* !!! FIXME: dr_flac treats returning less than bytesToRead as EOF. So we can't EAGAIN. */
    while (bytesToRead)
    {
        const size_t rc = SDL_RWread(rwops, ptr, 1, bytesToRead);
        if (rc == 0) break;
        bytesToRead -= rc;
        retval += rc;
        ptr += rc;
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


static SDL_bool FLAC_init(void)
{
    return SDL_TRUE;  /* always succeeds. */
} /* FLAC_init */


static void FLAC_quit(void)
{
    /* it's a no-op. */
} /* FLAC_quit */


static int FLAC_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = drflac_open(flac_read, flac_seek, sample, NULL);

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

    if (dr->totalPCMFrameCount == 0)
        internal->total_time = -1;
    else
    {
        const Uint32 rate = (Uint32) dr->sampleRate;
        const Uint64 frames = (Uint64) dr->totalPCMFrameCount;
        internal->total_time = (frames / rate) * 1000;
        internal->total_time += ((dr->totalPCMFrameCount % dr->sampleRate) * 1000) / dr->sampleRate;
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
    const int channels = (int) sample->actual.channels;
    drflac *dr = (drflac *) internal->decoder_private;
    const drflac_uint64 frames_to_read = (internal->buffer_size / channels) / sizeof (drflac_int32);
    const drflac_uint64 rc = drflac_read_pcm_frames_s32(dr, frames_to_read, (drflac_int32 *) internal->buffer);
    /* !!! FIXME: we only set the EOF flags, but this only tells you we're done, not about i/o errors, nor corruption. */
    if (rc < frames_to_read)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
    return rc * channels * sizeof (drflac_int32);
} /* FLAC_read */

static int FLAC_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = (drflac *) internal->decoder_private;
    return (drflac_seek_to_pcm_frame(dr, 0) == DRFLAC_TRUE);
} /* FLAC_rewind */

static int FLAC_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    drflac *dr = (drflac *) internal->decoder_private;
    const float frames_per_ms = ((float) sample->actual.rate) / 1000.0f;
    const drflac_uint64 frame_offset = (drflac_uint64) (frames_per_ms * ((float) ms));
    return (drflac_seek_to_pcm_frame(dr, frame_offset) == DRFLAC_TRUE);
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
