/**
 * SDL_sound; An abstract sound format decoding API.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Torbjörn Andersson.
 */

/*
 * Module player for SDL_sound. This driver handles anything that ModPlug does.
 *
 * ModPlug can be found at https://sourceforge.net/projects/modplug-xmms
 *
 * An unofficial version of modplug with all C++ dependencies removed is also
 *  available:  http://freecraft.net/snapshots/
 *  (Look for something like "libmodplug-johns-*.tar.gz")
 *  (update: this domain is gone.  --ryan.)
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_MODPLUG

#include "libmodplug/modplug.h"

static const char *extensions_modplug[] =
{
        /* The XMMS plugin is apparently able to load compressed modules as
         * well, but libmodplug does not handle this.
         */
    "669",   /* Composer 669 / UNIS 669 module                              */
    "AMF",   /* ASYLUM Music Format / Advanced Music Format(DSM)            */
    "AMS",   /* AMS module                                                  */
    "DBM",   /* DigiBooster Pro Module                                      */
    "DMF",   /* DMF DELUSION DIGITAL MUSIC FILEFORMAT (X-Tracker)           */
    "DSM",   /* DSIK Internal Format module                                 */
    "FAR",   /* Farandole module                                            */
    "IT",    /* Impulse Tracker IT file                                     */
    "MDL",   /* DigiTracker module                                          */
    "MED",   /* OctaMed MED file                                            */
    "MOD",   /* ProTracker / NoiseTracker MOD/NST file                      */
    "MT2",   /* MadTracker 2.0                                              */
    "MTM",   /* MTM file                                                    */
    "OKT",   /* Oktalyzer module                                            */
    "PTM",   /* PTM PolyTracker module                                      */
    "PSM",   /* PSM module                                                  */
    "S3M",   /* ScreamTracker file                                          */
    "STM",   /* ST 2.xx                                                     */
    "ULT",   
    "UMX",
    "XM",    /* FastTracker II                                              */
    "ABC",
    "MID",
    "MIDI",
    NULL
};



static int MODPLUG_init(void)
{
    return ModPlug_Init();  /* success. */
} /* MODPLUG_init */


static void MODPLUG_quit(void)
{
    /* it's a no-op. */
} /* MODPLUG_quit */


/*
 * Most MOD files I've seen have tended to be a few hundred KB, even if some
 * of them were much smaller than that.
 */
#define CHUNK_SIZE 65536

static int MODPLUG_open(Sound_Sample *sample, const char *ext)
{
    ModPlug_Settings settings;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ModPlugFile *module;
    Uint8 *data;
    size_t size;
    Uint32 retval;
    int has_extension = 0;
    int i;

    /*
     * Apparently ModPlug's loaders are too forgiving. They gladly accept
     *  streams that they shouldn't. For now, rely on file extension instead.
     */
    for (i = 0; extensions_modplug[i] != NULL; i++)
    {
        if (SDL_strcasecmp(ext, extensions_modplug[i]) == 0)
        {
            has_extension = 1;
            break;
        } /* if */
    } /* for */

    if (!has_extension)
    {
        SNDDBG(("MODPLUG: Unrecognized file type: %s\n", ext));
        BAIL_MACRO("MODPLUG: Not a module file.", 0);
    } /* if */
    
    /* ModPlug needs the entire stream in one big chunk. I don't like it,
       but I don't think there's any way around it.  !!! FIXME: rework modplug? */
    data = (Uint8 *) SDL_malloc(CHUNK_SIZE);
    BAIL_IF_MACRO(data == NULL, ERR_OUT_OF_MEMORY, 0);
    size = 0;

    do
    {
        retval = SDL_RWread(internal->rw, &data[size], 1, CHUNK_SIZE);
        size += retval;
        if (retval == CHUNK_SIZE)
        {
            data = (Uint8 *) SDL_realloc(data, size + CHUNK_SIZE);
            BAIL_IF_MACRO(data == NULL, ERR_OUT_OF_MEMORY, 0);
        } /* if */
    } while (retval > 0);

    SDL_memcpy(&sample->actual, &sample->desired, sizeof (Sound_AudioInfo));
    if (sample->actual.rate == 0) sample->actual.rate = 44100;
    if (sample->actual.channels == 0) sample->actual.channels = 2;
    if (sample->actual.format == 0) sample->actual.format = AUDIO_S16SYS;

    settings.mChannels=sample->actual.channels;
    settings.mFrequency=sample->actual.rate;
    settings.mBits = sample->actual.format & 0xFF;

    /* The settings will require some experimenting. I've borrowed some
        of them from the XMMS ModPlug plugin. */
    settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING;
    settings.mFlags |= MODPLUG_ENABLE_NOISE_REDUCTION |
                       MODPLUG_ENABLE_MEGABASS |
                       MODPLUG_ENABLE_SURROUND;

    settings.mReverbDepth = 30;
    settings.mReverbDelay = 100;
    settings.mBassAmount = 40;
    settings.mBassRange = 30;
    settings.mSurroundDepth = 20;
    settings.mSurroundDelay = 20;
    settings.mChannels = 2;
    settings.mBits = 16;
    settings.mFrequency = 44100;
    settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
    settings.mLoopCount = 0;

    /* The buffer may be a bit too large, but that doesn't matter. I think
       it's safe to free it as soon as ModPlug_Load() is finished anyway. */
    module = ModPlug_Load((void *) data, size, &settings);
    SDL_free(data);
    BAIL_IF_MACRO(module == NULL, "MODPLUG: Not a module file.", 0);

    internal->total_time = ModPlug_GetLength(module);
    internal->decoder_private = (void *) module;
    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;

    SNDDBG(("MODPLUG: Accepting data stream\n"));
    return 1; /* we'll handle this data. */
} /* MODPLUG_open */


static void MODPLUG_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ModPlugFile *module = (ModPlugFile *) internal->decoder_private;
    ModPlug_Unload(module);
} /* MODPLUG_close */


static Uint32 MODPLUG_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ModPlugFile *module = (ModPlugFile *) internal->decoder_private;
    int retval;
    retval = ModPlug_Read(module, internal->buffer, internal->buffer_size);
    if (retval == 0)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
    return retval;
} /* MODPLUG_read */


static int MODPLUG_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ModPlugFile *module = (ModPlugFile *) internal->decoder_private;
    ModPlug_Seek(module, 0);
    return 1;
} /* MODPLUG_rewind */


static int MODPLUG_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ModPlugFile *module = (ModPlugFile *) internal->decoder_private;
    ModPlug_Seek(module, ms);
    return 1;
} /* MODPLUG_seek */


const Sound_DecoderFunctions __Sound_DecoderFunctions_MODPLUG =
{
    {
        extensions_modplug,
        "Play modules through ModPlug",
        "Torbjörn Andersson <d91tan@Update.UU.SE>",
        "http://modplug-xmms.sourceforge.net/"
    },

    MODPLUG_init,       /*   init() method */
    MODPLUG_quit,       /*   quit() method */
    MODPLUG_open,       /*   open() method */
    MODPLUG_close,      /*  close() method */
    MODPLUG_read,       /*   read() method */
    MODPLUG_rewind,     /* rewind() method */
    MODPLUG_seek        /*   seek() method */
};

#endif /* SOUND_SUPPORTS_MODPLUG */

/* end of SDL_sound_modplug.c ... */

