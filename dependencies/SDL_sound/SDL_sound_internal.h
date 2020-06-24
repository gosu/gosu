/**
 * SDL_sound; An abstract sound format decoding API.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/*
 * Internal function/structure declaration. Do NOT include in your
 *  application.
 */

#ifndef _INCLUDE_SDL_SOUND_INTERNAL_H_
#define _INCLUDE_SDL_SOUND_INTERNAL_H_

#ifndef __SDL_SOUND_INTERNAL__
#error Do not include this header from your applications.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SDL_sound.h"

#if (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#define SOUND_HAVE_PRAGMA_VISIBILITY 1
#endif

#if SOUND_HAVE_PRAGMA_VISIBILITY
#pragma GCC visibility push(hidden)
#endif

#if (defined DEBUG_CHATTER)
#define SNDDBG(x) SDL_LogDebug x
#else
#define SNDDBG(x)
#endif

#ifndef SOUND_SUPPORTS_MP3
#define SOUND_SUPPORTS_MP3 1
#endif
#ifndef SOUND_SUPPORTS_MODPLUG
#define SOUND_SUPPORTS_MODPLUG 1
#endif
#ifndef SOUND_SUPPORTS_WAV
#define SOUND_SUPPORTS_WAV 1
#endif
#ifndef SOUND_SUPPORTS_AIFF
#define SOUND_SUPPORTS_AIFF 1
#endif
#ifndef SOUND_SUPPORTS_AU
#define SOUND_SUPPORTS_AU 1
#endif
#ifndef SOUND_SUPPORTS_VORBIS
#define SOUND_SUPPORTS_VORBIS 1
#endif
#ifndef SOUND_SUPPORTS_VOC
#define SOUND_SUPPORTS_VOC 1
#endif
#ifndef SOUND_SUPPORTS_RAW
#define SOUND_SUPPORTS_RAW 1
#endif
#ifndef SOUND_SUPPORTS_SHN
#define SOUND_SUPPORTS_SHN 1
#endif
#ifndef SOUND_SUPPORTS_FLAC
#define SOUND_SUPPORTS_FLAC 1
#endif
#ifndef SOUND_SUPPORTS_COREAUDIO
#define SOUND_SUPPORTS_COREAUDIO 1
#endif

/* only build CoreAudio support if on an Apple platform. */
#if SOUND_SUPPORTS_COREAUDIO && !defined(__APPLE__)
#undef SOUND_SUPPORTS_COREAUDIO
#define SOUND_SUPPORTS_COREAUDIO 0
#endif


/*
 * SDL itself only supports mono and stereo output, but hopefully we can
 *  raise this value someday...there's probably a lot of assumptions in
 *  SDL_sound that rely on it, though.
 * !!! FIXME: SDL2 supports more channels.
 */
#define MAX_CHANNELS 2


typedef struct __SOUND_DECODERFUNCTIONS__
{
        /* This is a block of info about your decoder. See SDL_sound.h. */
    const Sound_DecoderInfo info;

        /*
         * This is called during the Sound_Init() function. Use this to
         *  set up any global state that your decoder needs, such as
         *  initializing an external library, etc.
         *
         * Return non-zero if initialization is successful, zero if there's
         *  a fatal error. If this method fails, then this decoder is
         *  flagged as unavailable until SDL_sound() is shut down and
         *  reinitialized, in which case this method will be tried again.
         *
         * Note that the decoders quit() method won't be called if this
         *  method fails, so if you can't intialize, you'll have to clean
         *  up the half-initialized state in this method.
         */
    int (*init)(void);

        /*
         * This is called during the Sound_Quit() function. Use this to
         *  clean up any global state that your decoder has used during its
         *  lifespan.
         */
    void (*quit)(void);

        /*
         * Returns non-zero if (sample) has a valid fileformat that this
         *  driver can handle. Zero if this driver can NOT handle the data.
         *
         * Extension, which may be NULL, is just a hint as to the form of
         *  data that is being passed in. Most decoders should determine if
         *  they can handle the data by the data itself, but others, like
         *  the raw data handler, need this hint to know if they should
         *  accept the data in the first place.
         *
         * (sample)'s (opaque) field should be cast to a Sound_SampleInternal
         *  pointer:
         *
         *   Sound_SampleInternal *internal;
         *   internal = (Sound_SampleInternal *) sample->opaque;
         *
         * Certain fields of sample will be filled in for the decoder before
         *  this call, and others should be filled in by the decoder. Some
         *  fields are offlimits, and should NOT be modified. The list:
         *
         * in Sound_SampleInternal section:
         *    Sound_Sample *next;  (offlimits)
         *    Sound_Sample *prev;  (offlimits)
         *    SDL_RWops *rw;       (can use, but do NOT close it)
         *    const Sound_DecoderFunctions *funcs; (that's this structure)
         *    SDL_AudioCVT sdlcvt; (offlimits)
         *    void *buffer;        (offlimits until read() method)
         *    Uint32 buffer_size;  (offlimits until read() method)
         *    void *decoder_private; (read and write access)
         *
         * in rest of Sound_Sample:
         *    void *opaque;        (this was internal section, above)
         *    const Sound_DecoderInfo *decoder;  (read only)
         *    Sound_AudioInfo desired; (read only, usually not needed here)
         *    Sound_AudioInfo actual;  (please fill this in)
         *    void *buffer;            (offlimits)
         *    Uint32 buffer_size;      (offlimits)
         *    Sound_SampleFlags flags; (set appropriately)
         */
    int (*open)(Sound_Sample *sample, const char *ext);

        /*
         * Clean up. SDL_sound is done with this sample, so the decoder should
         *  clean up any resources it allocated. Anything that wasn't
         *  explicitly allocated by the decoder should be LEFT ALONE, since
         *  the higher-level SDL_sound layer will clean up its own mess.
         */
    void (*close)(Sound_Sample *sample);

        /*
         * Get more data from (sample). The decoder should get a pointer to
         *  the internal structure...
         *
         *   Sound_SampleInternal *internal;
         *   internal = (Sound_SampleInternal *) sample->opaque;
         *
         *  ...and then start decoding. Fill in up to internal->buffer_size
         *  bytes of decoded sound in the space pointed to by
         *  internal->buffer. The encoded data is read in from internal->rw.
         *  Data should be decoded in the format specified during the
         *  decoder's open() method in the sample->actual field. The
         *  conversion to the desired format is done at a higher level.
         *
         * The return value is the number of bytes decoded into
         *  internal->buffer, which can be no more than internal->buffer_size,
         *  but can be less. If it is less, you should set a state flag:
         *
         *   If there's just no more data (end of file, etc), then do:
         *      sample->flags |= SOUND_SAMPLEFLAG_EOF;
         *
         *   If there's an unrecoverable error, then do:
         *      __Sound_SetError(ERR_EXPLAIN_WHAT_WENT_WRONG);
         *      sample->flags |= SOUND_SAMPLEFLAG_ERROR;
         *
         *   If there's more data, but you'd have to block for considerable
         *    amounts of time to get at it, or there's a recoverable error,
         *    then do:
         *      __Sound_SetError(ERR_EXPLAIN_WHAT_WENT_WRONG);
         *      sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;
         *
         * SDL_sound will not call your read() method for any samples with
         *  SOUND_SAMPLEFLAG_EOF or SOUND_SAMPLEFLAG_ERROR set. The
         *  SOUND_SAMPLEFLAG_EAGAIN flag is reset before each call to this
         *  method.
         */
    Uint32 (*read)(Sound_Sample *sample);

        /*
         * Reset the decoding to the beginning of the stream. Nonzero on
         *  success, zero on failure.
         *  
         * The purpose of this method is to allow for higher efficiency than
         *  an application could get by just recreating the sample externally;
         *  not only do they not have to reopen the RWops, reallocate buffers,
         *  and potentially pass the data through several rejecting decoders,
         *  but certain decoders will not have to recreate their existing
         *  state (search for metadata, etc) since they already know they
         *  have a valid audio stream with a given set of characteristics.
         *
         * The decoder is responsible for calling seek() on the associated
         *  SDL_RWops. A failing call to seek() should be the ONLY reason that
         *  this method should ever fail!
         */
    int (*rewind)(Sound_Sample *sample);

        /*
         * Reposition the decoding to an arbitrary point. Nonzero on
         *  success, zero on failure.
         *  
         * The purpose of this method is to allow for higher efficiency than
         *  an application could get by just rewinding the sample and 
         *  decoding to a given point.
         *
         * The decoder is responsible for calling seek() on the associated
         *  SDL_RWops.
         *
         * If there is an error, try to recover so that the next read will
         *  continue as if nothing happened.
         */
    int (*seek)(Sound_Sample *sample, Uint32 ms);
} Sound_DecoderFunctions;


typedef void (*MixFunc)(float *dst, void *src, Uint32 frames, float *gains);

typedef struct __SOUND_SAMPLEINTERNAL__
{
    Sound_Sample *next;
    Sound_Sample *prev;
    SDL_RWops *rw;
    const Sound_DecoderFunctions *funcs;
    SDL_AudioCVT sdlcvt;
    void *buffer;
    Uint32 buffer_size;
    void *decoder_private;
    Sint32 total_time;
    Uint32 mix_position;
    MixFunc mix;
} Sound_SampleInternal;


/* error messages... */
#define ERR_IS_INITIALIZED       "Already initialized"
#define ERR_NOT_INITIALIZED      "Not initialized"
#define ERR_INVALID_ARGUMENT     "Invalid argument"
#define ERR_OUT_OF_MEMORY        "Out of memory"
#define ERR_NOT_SUPPORTED        "Operation not supported"
#define ERR_UNSUPPORTED_FORMAT   "Sound format unsupported"
#define ERR_NOT_A_HANDLE         "Not a file handle"
#define ERR_NO_SUCH_FILE         "No such file"
#define ERR_PAST_EOF             "Past end of file"
#define ERR_IO_ERROR             "I/O error"
#define ERR_COMPRESSION          "(De)compression error"
#define ERR_PREV_ERROR           "Previous decoding already caused an error"
#define ERR_PREV_EOF             "Previous decoding already triggered EOF"
#define ERR_CANNOT_SEEK          "Sample is not seekable"

/*
 * Call this to set the message returned by Sound_GetError().
 *  Please only use the ERR_* constants above, or add new constants to the
 *  above group, but I want these all in one place.
 *
 * Calling this with a NULL argument is a safe no-op.
 */
void __Sound_SetError(const char *err);

/*
 * Call this to convert milliseconds to an actual byte position, based on
 *  audio data characteristics.
 */
Uint32 __Sound_convertMsToBytePos(Sound_AudioInfo *info, Uint32 ms);


/* These get used all over for lessening code clutter. */
#define BAIL_MACRO(e, r) { __Sound_SetError(e); return r; }
#define BAIL_IF_MACRO(c, e, r) if (c) { __Sound_SetError(e); return r; }

#ifdef __cplusplus
extern "C" {
#endif

#endif /* defined _INCLUDE_SDL_SOUND_INTERNAL_H_ */

/* end of SDL_sound_internal.h ... */

