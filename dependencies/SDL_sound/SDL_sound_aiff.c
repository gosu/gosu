/**
 * SDL_sound; An abstract sound format decoding API.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Torbjörn Andersson.
 */

/*
 * AIFF decoder for SDL_sound
 *
 * [Insert something profound about the AIFF file format here.]
 *
 * This code was ripped from a decoder I had written for SDL_mixer, which was
 * based on SDL_mixer's old AIFF music loader. (This loader was unfortunately
 * completely broken, but it was still useful because all the pieces were
 * still there, so to speak.)
 *
 * When rewriting it for SDL_sound, I changed its structure to be more like
 * the WAV loader Ryan wrote. Had they not both been part of the same project
 * it would have been embarrassing how similar they are.
 *
 * It is not the most feature-complete AIFF loader the world has ever seen.
 * For instance, it only makes a token attempt at implementing the AIFF-C
 * standard; basically the parts of it that I can easily understand and test.
 * It's a start, though.
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_AIFF

/*****************************************************************************
 * aiff_t is what we store in our internal->decoder_private field...         *
 *****************************************************************************/
typedef struct S_AIFF_FMT_T
{
    Uint32 type;

    Uint32 total_bytes;
    Uint32 data_starting_offset;

    void (*free)(struct S_AIFF_FMT_T *fmt);
    Uint32 (*read_sample)(Sound_Sample *sample);
    int (*rewind_sample)(Sound_Sample *sample);
    int (*seek_sample)(Sound_Sample *sample, Uint32 ms);


#if 0
/*
   this is ripped from wav.c as ann example of format-specific data.
   please replace with something more appropriate when the need arises.
*/
    union
    {
        struct
        {
            Uint16 cbSize;
            Uint16 wSamplesPerBlock;
            Uint16 wNumCoef;
            ADPCMCOEFSET *aCoeff;
        } adpcm;

        /* put other format-specific data here... */
    } fmt;
#endif
} fmt_t;


typedef struct
{
    fmt_t fmt;
    Sint32 bytesLeft;
} aiff_t;



    /* Chunk management code... */

#define formID 0x4D524F46  /* "FORM", in ascii. */
#define aiffID 0x46464941  /* "AIFF", in ascii. */
#define aifcID 0x43464941  /* "AIFC", in ascii. */
#define ssndID 0x444E5353  /* "SSND", in ascii. */


/*****************************************************************************
 * The COMM chunk...                                                         *
 *****************************************************************************/

#define commID 0x4D4D4F43  /* "COMM", in ascii. */

/* format/compression types... */
#define noneID 0x454E4F4E  /* "NONE", in ascii. */

typedef struct
{
    Uint32 ckID;
    Uint32 ckDataSize;
    Uint16 numChannels;
    Uint32 numSampleFrames;
    Uint16 sampleSize;
    Uint32 sampleRate;
        /*
         * We don't handle AIFF-C compressed audio yet, but for those
         * interested the allowed compression types are supposed to be
         *
         *   compressionType   compressionName   meaning
         *   ---------------------------------------------------------------
         *   'NONE'            "not compressed"  uncompressed, that is,
         *                                        straight digitized samples
         *   'ACE2'            "ACE 2-to-1"      2-to-1 IIGS ACE (Audio
         *                                        Compression / Expansion)
         *   'ACE8'            "ACE 8-to-3"      8-to-3 IIGS ACE (Audio
         *                                        Compression / Expansion)
         *   'MAC3'            "MACE 3-to-1"     3-to-1 Macintosh Audio
         *                                        Compression / Expansion
         *   'MAC6'            "MACE 6-to-1"     6-to-1 Macintosh Audio
         *                                        Compression / Expansion
         *
         * A pstring is a "Pascal-style string", that is, "one byte followed
         * by test bytes followed when needed by one pad byte. The total
         * number of bytes in a pstring must be even. The pad byte is
         * included when the number of text bytes is even, so the total of
         * text bytes + one count byte + one pad byte will be even. This pad
         * byte is not reflected in the count."
         *
         * As for how these compression algorithms work, your guess is as
         * good as mine.
         */
    Uint32 compressionType;
#if 0
    pstring compressionName;
#endif
} comm_t;


/* 
 * Sample rate is encoded as an "80 bit IEEE Standard 754 floating point
 * number (Standard Apple Numeric Environment [SANE] data type Extended)".
 * Whose bright idea was that?
 *
 * This function was adapted from libsndfile, and while I do know a little
 * bit about the IEEE floating point standard I don't pretend to fully
 * understand this.
 */
static Uint32 SANE_to_Uint32 (Uint8 *sanebuf)
{
    /* Is the frequency outside of what we can represent with Uint32? */
    if ( (sanebuf[0] & 0x80)
      || (sanebuf[0] <= 0x3F)
      || (sanebuf[0] > 0x40)
      || (sanebuf[0] == 0x40 && sanebuf[1] > 0x1C) )
        return 0;

    return ((sanebuf[2] << 23) | (sanebuf[3] << 15) | (sanebuf[4] << 7)
        | (sanebuf[5] >> 1)) >> (29 - sanebuf[1]);
} /* SANE_to_Uint32 */


/*
 * Read in a comm_t from disk. This makes this process safe regardless of
 *  the processor's byte order or how the comm_t structure is packed.
 */
static int read_comm_chunk(SDL_RWops *rw, comm_t *comm)
{
    Uint8 sampleRate[10];

    /* skip reading the chunk ID, since it was already read at this point... */
    comm->ckID = commID;

    if (SDL_RWread(rw, &comm->ckDataSize, sizeof (comm->ckDataSize), 1) != 1)
        return 0;
    comm->ckDataSize = SDL_SwapBE32(comm->ckDataSize);

    if (SDL_RWread(rw, &comm->numChannels, sizeof (comm->numChannels), 1) != 1)
        return 0;
    comm->numChannels = SDL_SwapBE16(comm->numChannels);

    if (SDL_RWread(rw, &comm->numSampleFrames,
                   sizeof (comm->numSampleFrames), 1) != 1)
        return 0;
    comm->numSampleFrames = SDL_SwapBE32(comm->numSampleFrames);

    if (SDL_RWread(rw, &comm->sampleSize, sizeof (comm->sampleSize), 1) != 1)
        return 0;
    comm->sampleSize = SDL_SwapBE16(comm->sampleSize);

    if (SDL_RWread(rw, sampleRate, sizeof (sampleRate), 1) != 1)
        return 0;
    comm->sampleRate = SANE_to_Uint32(sampleRate);

    if (comm->ckDataSize > sizeof(comm->numChannels)
                         + sizeof(comm->numSampleFrames)
                         + sizeof(comm->sampleSize)
                         + sizeof(sampleRate))
    {
        if (SDL_RWread(rw, &comm->compressionType,
                       sizeof (comm->compressionType), 1) != 1)
            return 0;
        comm->compressionType = SDL_SwapBE32(comm->compressionType);
    } /* if */
    else
    {
        comm->compressionType = noneID;
    } /* else */

    return 1;
} /* read_comm_chunk */



/*****************************************************************************
 * The SSND chunk...                                                         *
 *****************************************************************************/

typedef struct
{
    Uint32 ckID;
    Uint32 ckDataSize;
    Uint32 offset;
    Uint32 blockSize;
    /*
     * Then, comm->numSampleFrames sample frames. (It's better to get the
     * length from numSampleFrames than from ckDataSize.)
     */
} ssnd_t;


static int read_ssnd_chunk(SDL_RWops *rw, ssnd_t *ssnd)
{
    /* skip reading the chunk ID, since it was already read at this point... */
    ssnd->ckID = ssndID;

    if (SDL_RWread(rw, &ssnd->ckDataSize, sizeof (ssnd->ckDataSize), 1) != 1)
        return 0;
    ssnd->ckDataSize = SDL_SwapBE32(ssnd->ckDataSize);

    if (SDL_RWread(rw, &ssnd->offset, sizeof (ssnd->offset), 1) != 1)
        return 0;
    ssnd->offset = SDL_SwapBE32(ssnd->offset);

    if (SDL_RWread(rw, &ssnd->blockSize, sizeof (ssnd->blockSize), 1) != 1)
        return 0;
    ssnd->blockSize = SDL_SwapBE32(ssnd->blockSize);

    /* Leave the SDL_RWops position indicator at the start of the samples */
    if (SDL_RWseek(rw, (int) ssnd->offset, SEEK_CUR) == -1)
        return 0;

    return 1;
} /* read_ssnd_chunk */



/*****************************************************************************
 * Normal, uncompressed aiff handler...                                      *
 *****************************************************************************/

static Uint32 read_sample_fmt_normal(Sound_Sample *sample)
{
    Uint32 retval;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    aiff_t *a = (aiff_t *) internal->decoder_private;
    Uint32 max = (internal->buffer_size < (Uint32) a->bytesLeft) ?
                    internal->buffer_size : (Uint32) a->bytesLeft;

    SDL_assert(max > 0);

        /*
         * We don't actually do any decoding, so we read the AIFF data
         *  directly into the internal buffer...
         */
    retval = SDL_RWread(internal->rw, internal->buffer, 1, max);

    a->bytesLeft -= retval;

        /* Make sure the read went smoothly... */
    if ((retval == 0) || (a->bytesLeft == 0))
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (retval == -1)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

        /* (next call this EAGAIN may turn into an EOF or error.) */
    else if (retval < internal->buffer_size)
        sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

    return retval;
} /* read_sample_fmt_normal */


static int rewind_sample_fmt_normal(Sound_Sample *sample)
{
    /* no-op. */
    return 1;
} /* rewind_sample_fmt_normal */


static int seek_sample_fmt_normal(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    aiff_t *a = (aiff_t *) internal->decoder_private;
    fmt_t *fmt = &a->fmt;
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int pos = (int) (fmt->data_starting_offset + offset);
    int rc = SDL_RWseek(internal->rw, pos, SEEK_SET);
    BAIL_IF_MACRO(rc != pos, ERR_IO_ERROR, 0);
    a->bytesLeft = fmt->total_bytes - offset;
    return 1;  /* success. */
} /* seek_sample_fmt_normal */


static void free_fmt_normal(fmt_t *fmt)
{
    /* it's a no-op. */
} /* free_fmt_normal */


static int read_fmt_normal(SDL_RWops *rw, fmt_t *fmt)
{
    /* (don't need to read more from the RWops...) */
    fmt->free = free_fmt_normal;
    fmt->read_sample = read_sample_fmt_normal;
    fmt->rewind_sample = rewind_sample_fmt_normal;
    fmt->seek_sample = seek_sample_fmt_normal;
    return 1;
} /* read_fmt_normal */




/*****************************************************************************
 * Everything else...                                                        *
 *****************************************************************************/

static int AIFF_init(void)
{
    return 1;  /* always succeeds. */
} /* AIFF_init */


static void AIFF_quit(void)
{
    /* it's a no-op. */
} /* AIFF_quit */


static int find_chunk(SDL_RWops *rw, Uint32 id)
{
    Sint32 siz = 0;
    Uint32 _id = 0;

    while (1)
    {
        BAIL_IF_MACRO(SDL_RWread(rw, &_id, sizeof (_id), 1) != 1, NULL, 0);
        if (SDL_SwapLE32(_id) == id)
            return 1;

        BAIL_IF_MACRO(SDL_RWread(rw, &siz, sizeof (siz), 1) != 1, NULL, 0);
        siz = SDL_SwapBE32(siz);
        SDL_assert(siz > 0);
        BAIL_IF_MACRO(SDL_RWseek(rw, siz, SEEK_CUR) == -1, NULL, 0);
    } /* while */

    return 0;  /* shouldn't hit this, but just in case... */
} /* find_chunk */


static int read_fmt(SDL_RWops *rw, comm_t *c, fmt_t *fmt)
{
    fmt->type = c->compressionType;

    /* if it's in this switch statement, we support the format. */
    switch (fmt->type)
    {
        case noneID:
            SNDDBG(("AIFF: Appears to be uncompressed audio.\n"));
            return read_fmt_normal(rw, fmt);

        /* add other types here. */

        default:
            SNDDBG(("AIFF: Format %lu is unknown.\n",
                    (unsigned int) fmt->type));
            BAIL_MACRO("AIFF: Unsupported format", 0);
    } /* switch */

    SDL_assert(0);  /* shouldn't hit this point. */
    return 0;
} /* read_fmt */


static int AIFF_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;
    Uint32 chunk_id;
    int bytes_per_sample;
    long pos;
    comm_t c;
    ssnd_t s;
    aiff_t *a;

    BAIL_IF_MACRO(SDL_ReadLE32(rw) != formID, "AIFF: Not a FORM file.", 0);
        SDL_ReadBE32(rw);  /* throw the length away; we don't need it. */

    chunk_id = SDL_ReadLE32(rw);
    BAIL_IF_MACRO(chunk_id != aiffID && chunk_id != aifcID,
        "AIFF: Not an AIFF or AIFC file.", 0);

    /* Chunks may appear in any order, so we establish base camp here. */
    pos = SDL_RWtell(rw);

    BAIL_IF_MACRO(!find_chunk(rw, commID), "AIFF: No common chunk.", 0);
    BAIL_IF_MACRO(!read_comm_chunk(rw, &c),
                  "AIFF: Can't read common chunk.", 0);

    sample->actual.channels = (Uint8) c.numChannels;
    sample->actual.rate = c.sampleRate;

    /* Really, sample->total_time = (c.numSampleFrames*1000) c.sampleRate */
    internal->total_time = (c.numSampleFrames / c.sampleRate) * 1000;
    internal->total_time += (c.numSampleFrames % c.sampleRate)
                             *  1000 / c.sampleRate;

    if (c.sampleSize <= 8)
    {
        sample->actual.format = AUDIO_S8;
        bytes_per_sample = c.numChannels;
    } /* if */
    else if (c.sampleSize <= 16)
    {
        sample->actual.format = AUDIO_S16MSB;
        bytes_per_sample = 2 * c.numChannels;
    } /* if */
    else
    {
        BAIL_MACRO("AIFF: Unsupported sample size.", 0);
    } /* else */

    BAIL_IF_MACRO(c.sampleRate == 0, "AIFF: Unsupported sample rate.", 0);

    a = (aiff_t *) SDL_malloc(sizeof(aiff_t));
    BAIL_IF_MACRO(a == NULL, ERR_OUT_OF_MEMORY, 0);

    if (!read_fmt(rw, &c, &(a->fmt)))
    {
        SDL_free(a);
        return 0;
    } /* if */

    SDL_RWseek(rw, pos, SEEK_SET);    /* if the seek fails, let it go... */

    if (!find_chunk(rw, ssndID))
    {
        SDL_free(a);
        BAIL_MACRO("AIFF: No sound data chunk.", 0);
    } /* if */

    if (!read_ssnd_chunk(rw, &s))
    {
        SDL_free(a);
        BAIL_MACRO("AIFF: Can't read sound data chunk.", 0);
    } /* if */

    a->fmt.total_bytes = a->bytesLeft = bytes_per_sample * c.numSampleFrames;
    a->fmt.data_starting_offset = SDL_RWtell(rw);
    internal->decoder_private = (void *) a;

    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;

    SNDDBG(("AIFF: Accepting data stream.\n"));
    return 1; /* we'll handle this data. */
} /* AIFF_open */


static void AIFF_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    aiff_t *a = (aiff_t *) internal->decoder_private;
    a->fmt.free(&(a->fmt));
    SDL_free(a);
} /* AIFF_close */


static Uint32 AIFF_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    aiff_t *a = (aiff_t *) internal->decoder_private;
    return a->fmt.read_sample(sample);
} /* AIFF_read */


static int AIFF_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    aiff_t *a = (aiff_t *) internal->decoder_private;
    fmt_t *fmt = &a->fmt;
    int rc = SDL_RWseek(internal->rw, fmt->data_starting_offset, SEEK_SET);
    BAIL_IF_MACRO(rc != fmt->data_starting_offset, ERR_IO_ERROR, 0);
    a->bytesLeft = fmt->total_bytes;
    return fmt->rewind_sample(sample);
} /* AIFF_rewind */


static int AIFF_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    aiff_t *a = (aiff_t *) internal->decoder_private;
    return a->fmt.seek_sample(sample, ms);
} /* AIFF_seek */

static const char *extensions_aiff[] = { "AIFF", "AIF", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_AIFF =
{
    {
        extensions_aiff,
        "Audio Interchange File Format",
        "TorbjÃ¶rn Andersson <d91tan@Update.UU.SE>",
        "https://icculus.org/SDL_sound/"
    },

    AIFF_init,      /*   init() method */
    AIFF_quit,      /*   quit() method */
    AIFF_open,      /*   open() method */
    AIFF_close,     /*  close() method */
    AIFF_read,      /*   read() method */
    AIFF_rewind,    /* rewind() method */
    AIFF_seek       /*   seek() method */
};


#endif /* SOUND_SUPPORTS_AIFF */

/* end of SDL_sound_aiff.c ... */

