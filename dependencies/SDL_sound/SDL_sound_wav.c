/**
 * SDL_sound; A sound processing toolkit.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/*
 * WAV decoder for SDL_sound.
 *
 * This driver handles Microsoft .WAVs, in as many of the thousands of
 *  variations as we can.
 */

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"

#if SOUND_SUPPORTS_WAV

/* Better than SDL_ReadLE16, since you can detect i/o errors... */
static SDL_INLINE int read_le16(SDL_RWops *rw, Uint16 *ui16)
{
    int rc = SDL_RWread(rw, ui16, sizeof (Uint16), 1);
    BAIL_IF_MACRO(rc != 1, ERR_IO_ERROR, 0);
    *ui16 = SDL_SwapLE16(*ui16);
    return 1;
} /* read_le16 */


/* Better than SDL_ReadLE32, since you can detect i/o errors... */
static SDL_INLINE int read_le32(SDL_RWops *rw, Uint32 *ui32)
{
    int rc = SDL_RWread(rw, ui32, sizeof (Uint32), 1);
    BAIL_IF_MACRO(rc != 1, ERR_IO_ERROR, 0);
    *ui32 = SDL_SwapLE32(*ui32);
    return 1;
} /* read_le32 */

static SDL_INLINE int read_le16s(SDL_RWops *rw, Sint16 *si16)
{
    return read_le16(rw, (Uint16 *) si16);
} /* read_le16s */

static SDL_INLINE int read_le32s(SDL_RWops *rw, Sint32 *si32)
{
    return read_le32(rw, (Uint32 *) si32);
} /* read_le32s */


/* This is just cleaner on the caller's end... */
static SDL_INLINE int read_uint8(SDL_RWops *rw, Uint8 *ui8)
{
    int rc = SDL_RWread(rw, ui8, sizeof (Uint8), 1);
    BAIL_IF_MACRO(rc != 1, ERR_IO_ERROR, 0);
    return 1;
} /* read_uint8 */


    /* Chunk management code... */

#define riffID 0x46464952  /* "RIFF", in ascii. */
#define waveID 0x45564157  /* "WAVE", in ascii. */
#define factID 0x74636166  /* "fact", in ascii. */


/*****************************************************************************
 * The FORMAT chunk...                                                       *
 *****************************************************************************/

#define fmtID  0x20746D66  /* "fmt ", in ascii. */

#define FMT_NORMAL 0x0001    /* Uncompressed waveform data.     */
#define FMT_ADPCM  0x0002    /* ADPCM compressed waveform data. */

typedef struct
{
    Sint16 iCoef1;
    Sint16 iCoef2;
} ADPCMCOEFSET;

typedef struct
{
    Uint8 bPredictor;
    Uint16 iDelta;
    Sint16 iSamp1;
    Sint16 iSamp2;
} ADPCMBLOCKHEADER;

typedef struct S_WAV_FMT_T
{
    Uint32 chunkID;
    Sint32 chunkSize;
    Sint16 wFormatTag;
    Uint16 wChannels;
    Uint32 dwSamplesPerSec;
    Uint32 dwAvgBytesPerSec;
    Uint16 wBlockAlign;
    Uint16 wBitsPerSample;

    Uint32 next_chunk_offset;
    
    Uint32 sample_frame_size;
    Uint32 data_starting_offset;
    Uint32 total_bytes;

    void (*free)(struct S_WAV_FMT_T *fmt);
    Uint32 (*read_sample)(Sound_Sample *sample);
    int (*rewind_sample)(Sound_Sample *sample);
    int (*seek_sample)(Sound_Sample *sample, Uint32 ms);

    union
    {
        struct
        {
            Uint16 cbSize;
            Uint16 wSamplesPerBlock;
            Uint16 wNumCoef;
            ADPCMCOEFSET *aCoef;
            ADPCMBLOCKHEADER *blockheaders;
            Uint32 samples_left_in_block;
            int nibble_state;
            Sint8 nibble;
        } adpcm;

        /* put other format-specific data here... */
    } fmt;
} fmt_t;


/*
 * Read in a fmt_t from disk. This makes this process safe regardless of
 *  the processor's byte order or how the fmt_t structure is packed.
 * Note that the union "fmt" is not read in here; that is handled as 
 *  needed in the read_fmt_* functions.
 */
static int read_fmt_chunk(SDL_RWops *rw, fmt_t *fmt)
{
    /* skip reading the chunk ID, since it was already read at this point... */
    fmt->chunkID = fmtID;

    BAIL_IF_MACRO(!read_le32s(rw, &fmt->chunkSize), NULL, 0);
    BAIL_IF_MACRO(fmt->chunkSize < 16, "WAV: Invalid chunk size", 0);
    fmt->next_chunk_offset = SDL_RWtell(rw) + fmt->chunkSize;
    
    BAIL_IF_MACRO(!read_le16s(rw, &fmt->wFormatTag), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->wChannels), NULL, 0);
    BAIL_IF_MACRO(!read_le32(rw, &fmt->dwSamplesPerSec), NULL, 0);
    BAIL_IF_MACRO(!read_le32(rw, &fmt->dwAvgBytesPerSec), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->wBlockAlign), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->wBitsPerSample), NULL, 0);

    return 1;
} /* read_fmt_chunk */



/*****************************************************************************
 * The DATA chunk...                                                         *
 *****************************************************************************/

#define dataID 0x61746164  /* "data", in ascii. */

typedef struct
{
    Uint32 chunkID;
    Sint32 chunkSize;
    /* Then, (chunkSize) bytes of waveform data... */
} data_t;


/*
 * Read in a data_t from disk. This makes this process safe regardless of
 *  the processor's byte order or how the fmt_t structure is packed.
 */
static int read_data_chunk(SDL_RWops *rw, data_t *data)
{
    /* skip reading the chunk ID, since it was already read at this point... */
    data->chunkID = dataID;
    BAIL_IF_MACRO(!read_le32s(rw, &data->chunkSize), NULL, 0);
    return 1;
} /* read_data_chunk */




/*****************************************************************************
 * this is what we store in our internal->decoder_private field...           *
 *****************************************************************************/

typedef struct
{
    fmt_t *fmt;
    Sint32 bytesLeft;
} wav_t;




/*****************************************************************************
 * Normal, uncompressed waveform handler...                                  *
 *****************************************************************************/

/*
 * Sound_Decode() lands here for uncompressed WAVs...
 */
static Uint32 read_sample_fmt_normal(Sound_Sample *sample)
{
    Uint32 retval;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    Uint32 max = (internal->buffer_size < (Uint32) w->bytesLeft) ?
                    internal->buffer_size : (Uint32) w->bytesLeft;

    SDL_assert(max > 0);

        /*
         * We don't actually do any decoding, so we read the wav data
         *  directly into the internal buffer...
         */
    retval = SDL_RWread(internal->rw, internal->buffer, 1, max);

    w->bytesLeft -= retval;

        /* Make sure the read went smoothly... */
    if ((retval == 0) || (w->bytesLeft == 0))
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (retval == -1)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

        /* (next call this EAGAIN may turn into an EOF or error.) */
    else if (retval < internal->buffer_size)
        sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

    return retval;
} /* read_sample_fmt_normal */


static int seek_sample_fmt_normal(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int pos = (int) (fmt->data_starting_offset + offset);
    int rc = SDL_RWseek(internal->rw, pos, SEEK_SET);
    BAIL_IF_MACRO(rc != pos, ERR_IO_ERROR, 0);
    w->bytesLeft = fmt->total_bytes - offset;
    return 1;  /* success. */
} /* seek_sample_fmt_normal */


static int rewind_sample_fmt_normal(Sound_Sample *sample)
{
    /* no-op. */
    return 1;
} /* rewind_sample_fmt_normal */


static int read_fmt_normal(SDL_RWops *rw, fmt_t *fmt)
{
    /* (don't need to read more from the RWops...) */
    fmt->free = NULL;
    fmt->read_sample = read_sample_fmt_normal;
    fmt->rewind_sample = rewind_sample_fmt_normal;
    fmt->seek_sample = seek_sample_fmt_normal;
    return 1;
} /* read_fmt_normal */



/*****************************************************************************
 * ADPCM compression handler...                                              *
 *****************************************************************************/

#define FIXED_POINT_COEF_BASE      256
#define FIXED_POINT_ADAPTION_BASE  256
#define SMALLEST_ADPCM_DELTA       16


static SDL_INLINE int read_adpcm_block_headers(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    int i;
    int max = fmt->wChannels;

    if (w->bytesLeft < fmt->wBlockAlign)
    {
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
        return 0;
    } /* if */

    w->bytesLeft -= fmt->wBlockAlign;

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_uint8(rw, &headers[i].bPredictor), NULL, 0);

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_le16(rw, &headers[i].iDelta), NULL, 0);

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_le16s(rw, &headers[i].iSamp1), NULL, 0);

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_le16s(rw, &headers[i].iSamp2), NULL, 0);

    fmt->fmt.adpcm.samples_left_in_block = fmt->fmt.adpcm.wSamplesPerBlock;
    fmt->fmt.adpcm.nibble_state = 0;
    return 1;
} /* read_adpcm_block_headers */


static SDL_INLINE void do_adpcm_nibble(Uint8 nib,
                                       ADPCMBLOCKHEADER *header,
                                       Sint32 lPredSamp)
{
	static const Sint32 max_audioval = ((1<<(16-1))-1);
	static const Sint32 min_audioval = -(1<<(16-1));
	static const Sint32 AdaptionTable[] =
    {
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};

    Sint32 lNewSamp;
    Sint32 delta;

    if (nib & 0x08)
        lNewSamp = lPredSamp + (header->iDelta * (nib - 0x10));
	else
        lNewSamp = lPredSamp + (header->iDelta * nib);

        /* clamp value... */
    if (lNewSamp < min_audioval)
        lNewSamp = min_audioval;
    else if (lNewSamp > max_audioval)
        lNewSamp = max_audioval;

    delta = ((Sint32) header->iDelta * AdaptionTable[nib]) /
              FIXED_POINT_ADAPTION_BASE;

	if (delta < SMALLEST_ADPCM_DELTA)
	    delta = SMALLEST_ADPCM_DELTA;

    header->iDelta = delta;
	header->iSamp2 = header->iSamp1;
	header->iSamp1 = lNewSamp;
} /* do_adpcm_nibble */


static SDL_INLINE int decode_adpcm_sample_frame(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    SDL_RWops *rw = internal->rw;
    int i;
    int max = fmt->wChannels;
    Sint32 delta;
    Uint8 nib = fmt->fmt.adpcm.nibble;

    for (i = 0; i < max; i++)
    {
        Uint8 byte;
        Sint16 iCoef1 = fmt->fmt.adpcm.aCoef[headers[i].bPredictor].iCoef1;
        Sint16 iCoef2 = fmt->fmt.adpcm.aCoef[headers[i].bPredictor].iCoef2;
        Sint32 lPredSamp = ((headers[i].iSamp1 * iCoef1) +
                            (headers[i].iSamp2 * iCoef2)) / 
                             FIXED_POINT_COEF_BASE;

        if (fmt->fmt.adpcm.nibble_state == 0)
        {
            BAIL_IF_MACRO(!read_uint8(rw, &nib), NULL, 0);
            fmt->fmt.adpcm.nibble_state = 1;
            do_adpcm_nibble(nib >> 4, &headers[i], lPredSamp);
        } /* if */
        else
        {
            fmt->fmt.adpcm.nibble_state = 0;
            do_adpcm_nibble(nib & 0x0F, &headers[i], lPredSamp);
        } /* else */
    } /* for */

    fmt->fmt.adpcm.nibble = nib;
    return 1;
} /* decode_adpcm_sample_frame */


static SDL_INLINE void put_adpcm_sample_frame1(void *_buf, fmt_t *fmt)
{
    Uint16 *buf = (Uint16 *) _buf;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    int i;
    for (i = 0; i < fmt->wChannels; i++)
        *(buf++) = headers[i].iSamp1;
} /* put_adpcm_sample_frame1 */


static SDL_INLINE void put_adpcm_sample_frame2(void *_buf, fmt_t *fmt)
{
    Uint16 *buf = (Uint16 *) _buf;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    int i;
    for (i = 0; i < fmt->wChannels; i++)
        *(buf++) = headers[i].iSamp2;
} /* put_adpcm_sample_frame2 */


/*
 * Sound_Decode() lands here for ADPCM-encoded WAVs...
 */
static Uint32 read_sample_fmt_adpcm(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    Uint32 bw = 0;

    while (bw < internal->buffer_size)
    {
        /* write ongoing sample frame before reading more data... */
        switch (fmt->fmt.adpcm.samples_left_in_block)
        {
            case 0:  /* need to read a new block... */
                if (!read_adpcm_block_headers(sample))
                {
                    if ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0)
                        sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                    return bw;
                } /* if */

                /* only write first sample frame for now. */
                put_adpcm_sample_frame2((Uint8 *) internal->buffer + bw, fmt);
                fmt->fmt.adpcm.samples_left_in_block--;
                bw += fmt->sample_frame_size;
                break;

            case 1:  /* output last sample frame of block... */
                put_adpcm_sample_frame1((Uint8 *) internal->buffer + bw, fmt);
                fmt->fmt.adpcm.samples_left_in_block--;
                bw += fmt->sample_frame_size;
                break;

            default: /* output latest sample frame and read a new one... */
                put_adpcm_sample_frame1((Uint8 *) internal->buffer + bw, fmt);
                fmt->fmt.adpcm.samples_left_in_block--;
                bw += fmt->sample_frame_size;

                if (!decode_adpcm_sample_frame(sample))
                {
                    sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                    return bw;
                } /* if */
        } /* switch */
    } /* while */

    return bw;
} /* read_sample_fmt_adpcm */


/*
 * Sound_FreeSample() lands here for ADPCM-encoded WAVs...
 */
static void free_fmt_adpcm(fmt_t *fmt)
{
    if (fmt->fmt.adpcm.aCoef != NULL)
        SDL_free(fmt->fmt.adpcm.aCoef);

    if (fmt->fmt.adpcm.blockheaders != NULL)
        SDL_free(fmt->fmt.adpcm.blockheaders);
} /* free_fmt_adpcm */


static int rewind_sample_fmt_adpcm(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    w->fmt->fmt.adpcm.samples_left_in_block = 0;
    return 1;
} /* rewind_sample_fmt_adpcm */


static int seek_sample_fmt_adpcm(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    Uint32 origsampsleft = fmt->fmt.adpcm.samples_left_in_block;
    int origpos = SDL_RWtell(internal->rw);
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int bpb = (fmt->fmt.adpcm.wSamplesPerBlock * fmt->sample_frame_size);
    int skipsize = (offset / bpb) * fmt->wBlockAlign;
    int pos = skipsize + fmt->data_starting_offset;
    int rc = SDL_RWseek(internal->rw, pos, SEEK_SET);
    BAIL_IF_MACRO(rc != pos, ERR_IO_ERROR, 0);

    /* The offset we need is in this block, so we need to decode to there. */
    skipsize += (offset % bpb);
    rc = (offset % bpb);  /* bytes into this block we need to decode */
    if (!read_adpcm_block_headers(sample))
    {
        SDL_RWseek(internal->rw, origpos, SEEK_SET);  /* try to make sane. */
        return 0;
    } /* if */

    /* first sample frame of block is a freebie. :) */
    fmt->fmt.adpcm.samples_left_in_block--;
    rc -= fmt->sample_frame_size;
    while (rc > 0)
    {
        if (!decode_adpcm_sample_frame(sample))
        {
            SDL_RWseek(internal->rw, origpos, SEEK_SET);
            fmt->fmt.adpcm.samples_left_in_block = origsampsleft;
            return 0;
        } /* if */

        fmt->fmt.adpcm.samples_left_in_block--;
        rc -= fmt->sample_frame_size;
    } /* while */

    w->bytesLeft = fmt->total_bytes - skipsize;
    return 1;  /* success. */
} /* seek_sample_fmt_adpcm */


/*
 * Read in the adpcm-specific info from disk. This makes this process
 *  safe regardless of the processor's byte order or how the fmt_t 
 *  structure is packed.
 */
static int read_fmt_adpcm(SDL_RWops *rw, fmt_t *fmt)
{
    size_t i;

    SDL_memset(&fmt->fmt.adpcm, '\0', sizeof (fmt->fmt.adpcm));
    fmt->free = free_fmt_adpcm;
    fmt->read_sample = read_sample_fmt_adpcm;
    fmt->rewind_sample = rewind_sample_fmt_adpcm;
    fmt->seek_sample = seek_sample_fmt_adpcm;

    BAIL_IF_MACRO(!read_le16(rw, &fmt->fmt.adpcm.cbSize), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->fmt.adpcm.wSamplesPerBlock), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->fmt.adpcm.wNumCoef), NULL, 0);

    /* fmt->free() is always called, so these malloc()s will be cleaned up. */

    i = sizeof (ADPCMCOEFSET) * fmt->fmt.adpcm.wNumCoef;
    fmt->fmt.adpcm.aCoef = (ADPCMCOEFSET *) SDL_malloc(i);
    BAIL_IF_MACRO(fmt->fmt.adpcm.aCoef == NULL, ERR_OUT_OF_MEMORY, 0);

    for (i = 0; i < fmt->fmt.adpcm.wNumCoef; i++)
    {
        BAIL_IF_MACRO(!read_le16s(rw, &fmt->fmt.adpcm.aCoef[i].iCoef1), NULL, 0);
        BAIL_IF_MACRO(!read_le16s(rw, &fmt->fmt.adpcm.aCoef[i].iCoef2), NULL, 0);
    } /* for */

    i = sizeof (ADPCMBLOCKHEADER) * fmt->wChannels;
    fmt->fmt.adpcm.blockheaders = (ADPCMBLOCKHEADER *) SDL_malloc(i);
    BAIL_IF_MACRO(fmt->fmt.adpcm.blockheaders == NULL, ERR_OUT_OF_MEMORY, 0);

    return 1;
} /* read_fmt_adpcm */



/*****************************************************************************
 * Everything else...                                                        *
 *****************************************************************************/

static int WAV_init(void)
{
    return 1;  /* always succeeds. */
} /* WAV_init */


static void WAV_quit(void)
{
    /* it's a no-op. */
} /* WAV_quit */


static int read_fmt(SDL_RWops *rw, fmt_t *fmt)
{
    /* if it's in this switch statement, we support the format. */
    switch (fmt->wFormatTag)
    {
        case FMT_NORMAL:
            SNDDBG(("WAV: Appears to be uncompressed audio.\n"));
            return read_fmt_normal(rw, fmt);

        case FMT_ADPCM:
            SNDDBG(("WAV: Appears to be ADPCM compressed audio.\n"));
            return read_fmt_adpcm(rw, fmt);

        /* add other types here. */

        default:
            SNDDBG(("WAV: Format 0x%X is unknown.\n",
                    (unsigned int) fmt->wFormatTag));
            BAIL_MACRO("WAV: Unsupported format", 0);
    } /* switch */

    SDL_assert(0);  /* shouldn't hit this point. */
    return 0;
} /* read_fmt */


/*
 * Locate a specific chunk in the WAVE file by ID...
 */
static int find_chunk(SDL_RWops *rw, Uint32 id)
{
    Sint32 siz = 0;
    Uint32 _id = 0;
    Uint32 pos = SDL_RWtell(rw);

    while (1)
    {
        BAIL_IF_MACRO(!read_le32(rw, &_id), NULL, 0);
        if (_id == id)
            return 1;

            /* skip ahead and see what next chunk is... */
        BAIL_IF_MACRO(!read_le32s(rw, &siz), NULL, 0);
        SDL_assert(siz >= 0);
        pos += (sizeof (Uint32) * 2) + siz;
        if (siz > 0)
            BAIL_IF_MACRO(SDL_RWseek(rw, pos, SEEK_SET) != pos, NULL, 0);
    } /* while */

    return 0;  /* shouldn't hit this, but just in case... */
} /* find_chunk */


static int WAV_open_internal(Sound_Sample *sample, const char *ext, fmt_t *fmt)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    SDL_RWops *rw = internal->rw;
    data_t d;
    wav_t *w;
    Uint32 pos;

    BAIL_IF_MACRO(SDL_ReadLE32(rw) != riffID, "WAV: Not a RIFF file.", 0);
    SDL_ReadLE32(rw);  /* throw the length away; we get this info later. */
    BAIL_IF_MACRO(SDL_ReadLE32(rw) != waveID, "WAV: Not a WAVE file.", 0);
    BAIL_IF_MACRO(!find_chunk(rw, fmtID), "WAV: No format chunk.", 0);
    BAIL_IF_MACRO(!read_fmt_chunk(rw, fmt), "WAV: Can't read format chunk.", 0);

    /* !!! FIXME: need float32 format stuff, since it's not just wBitsPerSample. */

    sample->actual.channels = (Uint8) fmt->wChannels;
    sample->actual.rate = fmt->dwSamplesPerSec;
    if (fmt->wBitsPerSample == 4)
        sample->actual.format = AUDIO_S16SYS;
    else if (fmt->wBitsPerSample == 8)
        sample->actual.format = AUDIO_U8;
    else if (fmt->wBitsPerSample == 16)
        sample->actual.format = AUDIO_S16LSB;
    else if (fmt->wBitsPerSample == 32)
        sample->actual.format = AUDIO_S32LSB;
    else
    {
        SNDDBG(("WAV: %d bits per sample!?\n", (int) fmt->wBitsPerSample));
        BAIL_MACRO("WAV: Unsupported sample size.", 0);
    } /* else */

    BAIL_IF_MACRO(!read_fmt(rw, fmt), NULL, 0);
    SDL_RWseek(rw, fmt->next_chunk_offset, SEEK_SET);
    BAIL_IF_MACRO(!find_chunk(rw, dataID), "WAV: No data chunk.", 0);
    BAIL_IF_MACRO(!read_data_chunk(rw, &d), "WAV: Can't read data chunk.", 0);

    w = (wav_t *) SDL_malloc(sizeof(wav_t));
    BAIL_IF_MACRO(w == NULL, ERR_OUT_OF_MEMORY, 0);
    w->fmt = fmt;
    fmt->total_bytes = w->bytesLeft = d.chunkSize;
    fmt->data_starting_offset = SDL_RWtell(rw);
    fmt->sample_frame_size = ( ((sample->actual.format & 0xFF) / 8) *
                               sample->actual.channels );
    internal->decoder_private = (void *) w;

    internal->total_time = (fmt->total_bytes / fmt->dwAvgBytesPerSec) * 1000;
    internal->total_time += (fmt->total_bytes % fmt->dwAvgBytesPerSec)
                              *  1000 / fmt->dwAvgBytesPerSec;

    sample->flags = SOUND_SAMPLEFLAG_NONE;
    if (fmt->seek_sample != NULL)
        sample->flags |= SOUND_SAMPLEFLAG_CANSEEK;

    SNDDBG(("WAV: Accepting data stream.\n"));
    return 1; /* we'll handle this data. */
} /* WAV_open_internal */


static int WAV_open(Sound_Sample *sample, const char *ext)
{
    int rc;

    fmt_t *fmt = (fmt_t *) SDL_calloc(1, sizeof (fmt_t));
    BAIL_IF_MACRO(fmt == NULL, ERR_OUT_OF_MEMORY, 0);

    rc = WAV_open_internal(sample, ext, fmt);
    if (!rc)
    {
        if (fmt->free != NULL)
            fmt->free(fmt);
        SDL_free(fmt);
    } /* if */

    return rc;
} /* WAV_open */


static void WAV_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;

    if (w->fmt->free != NULL)
        w->fmt->free(w->fmt);

    SDL_free(w->fmt);
    SDL_free(w);
} /* WAV_close */


static Uint32 WAV_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    return w->fmt->read_sample(sample);
} /* WAV_read */


static int WAV_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    int rc = SDL_RWseek(internal->rw, fmt->data_starting_offset, SEEK_SET);
    BAIL_IF_MACRO(rc != fmt->data_starting_offset, ERR_IO_ERROR, 0);
    w->bytesLeft = fmt->total_bytes;
    return fmt->rewind_sample(sample);
} /* WAV_rewind */


static int WAV_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    return w->fmt->seek_sample(sample, ms);
} /* WAV_seek */


static const char *extensions_wav[] = { "WAV", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_WAV =
{
    {
        extensions_wav,
        "Microsoft WAVE audio format",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/SDL_sound/"
    },

    WAV_init,       /*   init() method */
    WAV_quit,       /*   quit() method */
    WAV_open,       /*   open() method */
    WAV_close,      /*  close() method */
    WAV_read,       /*   read() method */
    WAV_rewind,     /* rewind() method */
    WAV_seek        /*   seek() method */
};

#endif /* SOUND_SUPPORTS_WAV */

/* end of SDL_sound_wav.c ... */

