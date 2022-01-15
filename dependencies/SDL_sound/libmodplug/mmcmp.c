/*
 * This source code is public domain.
 *
 * Handles unpacking of Powerpack PP20
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"

BOOL PP20_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength);

#pragma pack(1)
typedef struct MMCMPFILEHEADER
{
	char id[8]; /* string 'ziRCONia' */
	WORD hdrsize; /* sizeof MMCMPHEADER */
} MMCMPFILEHEADER, *LPMMCMPFILEHEADER;

typedef struct MMCMPHEADER
{
	WORD version;
	WORD nblocks;
	DWORD filesize;
	DWORD blktable;
	BYTE glb_comp;
	BYTE fmt_comp;
} MMCMPHEADER, *LPMMCMPHEADER;

typedef struct MMCMPBLOCK
{
	DWORD unpk_size;
	DWORD pk_size;
	DWORD xor_chk;
	WORD sub_blk;
	WORD flags;
	WORD tt_entries;
	USHORT num_bits;
} MMCMPBLOCK, *LPMMCMPBLOCK;

typedef struct MMCMPSUBBLOCK
{
	DWORD unpk_pos;
	DWORD unpk_size;
} MMCMPSUBBLOCK, *LPMMCMPSUBBLOCK;
#pragma pack()

/* make sure of structure sizes */
SDL_COMPILE_TIME_ASSERT(MMCMPFILEHEADER,sizeof(struct MMCMPFILEHEADER) == 10);
SDL_COMPILE_TIME_ASSERT(MMCMPHEADER,sizeof(struct MMCMPHEADER) == 14);
SDL_COMPILE_TIME_ASSERT(MMCMPBLOCK,sizeof(struct MMCMPBLOCK) == 20);
SDL_COMPILE_TIME_ASSERT(MMCMPSUBBLOCK,sizeof(struct MMCMPSUBBLOCK) == 8);

#define MMCMP_COMP		0x0001
#define MMCMP_DELTA		0x0002
#define MMCMP_16BIT		0x0004
#define MMCMP_STEREO	0x0100
#define MMCMP_ABS16		0x0200
#define MMCMP_ENDIAN	0x0400

typedef struct MMCMPBITBUFFER
{
	UINT bitcount;
	DWORD bitbuffer;
	LPCBYTE pSrc;
	LPCBYTE pEnd;
} MMCMPBITBUFFER;


static DWORD MMCMPBITBUFFER_GetBits(MMCMPBITBUFFER *bbuf, UINT nBits)
{
	DWORD d;
	if (!nBits) return 0;
	while (bbuf->bitcount < 24)
	{
		bbuf->bitbuffer |= ((bbuf->pSrc < bbuf->pEnd) ? *bbuf->pSrc++ : 0) << bbuf->bitcount;
		bbuf->bitcount += 8;
	}
	d = bbuf->bitbuffer & ((1 << nBits) - 1);
	bbuf->bitbuffer >>= nBits;
	bbuf->bitcount -= nBits;
	return d;
}

static const DWORD MMCMP8BitCommands[8] =
{
	0x01, 0x03,	0x07, 0x0F,	0x1E, 0x3C,	0x78, 0xF8
};

static const UINT MMCMP8BitFetch[8] =
{
	3, 3, 3, 3, 2, 1, 0, 0
};

static const DWORD MMCMP16BitCommands[16] =
{
	0x01, 0x03,	0x07, 0x0F,	0x1E, 0x3C,	0x78, 0xF0,
	0x1F0, 0x3F0, 0x7F0, 0xFF0, 0x1FF0, 0x3FF0, 0x7FF0, 0xFFF0
};

static const UINT MMCMP16BitFetch[16] =
{
	4, 4, 4, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


static void swap_mfh(LPMMCMPFILEHEADER fh)
{
	fh->hdrsize = bswapLE16(fh->hdrsize);
}

static void swap_mmh(LPMMCMPHEADER mh)
{
	mh->version = bswapLE16(mh->version);
	mh->nblocks = bswapLE16(mh->nblocks);
	mh->filesize = bswapLE32(mh->filesize);
	mh->blktable = bswapLE32(mh->blktable);
}

static void swap_block (LPMMCMPBLOCK blk)
{
	blk->unpk_size = bswapLE32(blk->unpk_size);
	blk->pk_size = bswapLE32(blk->pk_size);
	blk->xor_chk = bswapLE32(blk->xor_chk);
	blk->sub_blk = bswapLE16(blk->sub_blk);
	blk->flags = bswapLE16(blk->flags);
	blk->tt_entries = bswapLE16(blk->tt_entries);
	blk->num_bits = bswapLE16(blk->num_bits);
}

static void swap_subblock (LPMMCMPSUBBLOCK sblk)
{
	sblk->unpk_pos = bswapLE32(sblk->unpk_pos);
	sblk->unpk_size = bswapLE32(sblk->unpk_size);
}

static BOOL MMCMP_IsDstBlockValid(const MMCMPSUBBLOCK *psub, DWORD dstlen)
{
	if (psub->unpk_pos >= dstlen) return FALSE;
	if (psub->unpk_size > dstlen) return FALSE;
	if (psub->unpk_size > dstlen - psub->unpk_pos) return FALSE;
	return TRUE;
}


BOOL MMCMP_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength)
{
	DWORD dwMemLength;
	LPCBYTE lpMemFile;
	LPBYTE pBuffer,pBufEnd;
	LPMMCMPFILEHEADER pmfh;
	LPMMCMPHEADER pmmh;
	const DWORD *pblk_table;
	DWORD dwFileSize;
	BYTE tmp0[32], tmp1[32];

	if (PP20_Unpack(ppMemFile, pdwMemLength))
	{
		return TRUE;
	}

	dwMemLength = *pdwMemLength;
	lpMemFile = *ppMemFile;
	if ((dwMemLength < 256) || (!lpMemFile)) return FALSE;
	SDL_memcpy(tmp0, lpMemFile, 24);
	pmfh = (LPMMCMPFILEHEADER)(tmp0);
	pmmh = (LPMMCMPHEADER)(tmp0+10);
	swap_mfh(pmfh);
	swap_mmh(pmmh);

	if ((SDL_memcmp(pmfh->id,"ziRCONia",8) != 0) || (pmfh->hdrsize != 14))
		return FALSE;
	if ((!pmmh->nblocks) || (pmmh->filesize < 16) || (pmmh->filesize > 0x8000000) ||
	    (pmmh->blktable >= dwMemLength) || (pmmh->blktable + 4*pmmh->nblocks > dwMemLength)) {
		return FALSE;
	}
	dwFileSize = pmmh->filesize;
	if ((pBuffer = (LPBYTE)SDL_malloc(dwFileSize)) == NULL)
		return FALSE;
	pBufEnd = pBuffer + dwFileSize;
	pblk_table = (const DWORD *)(lpMemFile+pmmh->blktable);
	for (UINT nBlock=0; nBlock<pmmh->nblocks; nBlock++)
	{
		DWORD dwMemPos = bswapLE32(pblk_table[nBlock]);
		DWORD dwSubPos;
		LPMMCMPBLOCK pblk;
		LPMMCMPSUBBLOCK psubblk;

		if (dwMemPos >= dwMemLength - 20)
			goto err;
		SDL_memcpy(tmp1,lpMemFile+dwMemPos,28);
		pblk = (LPMMCMPBLOCK)(tmp1);
		psubblk = (LPMMCMPSUBBLOCK)(tmp1+20);
		swap_block(pblk);
		swap_subblock(psubblk);

		if (!pblk->unpk_size || !pblk->pk_size || !pblk->sub_blk)
			goto err;
		if (pblk->pk_size <= pblk->tt_entries)
			goto err;
		if (pblk->sub_blk*8 >= dwMemLength - dwMemPos - 20)
			goto err;
		if (pblk->flags & MMCMP_COMP) {
			if (pblk->flags & MMCMP_16BIT) {
				if (pblk->num_bits >= 16)
					goto err;
			}
			else {
				if (pblk->num_bits >=  8)
					goto err;
			}
		}

		dwSubPos = dwMemPos + 20;
		dwMemPos += 20 + pblk->sub_blk*8;
		if (!(pblk->flags & MMCMP_COMP))
		{ /* Data is not packed */
			UINT i=0;
			while (1) {
				if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
					goto err;
				SDL_memcpy(pBuffer+psubblk->unpk_pos, lpMemFile+dwMemPos, psubblk->unpk_size);
				dwMemPos += psubblk->unpk_size;
				if (++i == pblk->sub_blk) break;
				SDL_memcpy(tmp1+20,lpMemFile+dwSubPos+i*8,8);
				swap_subblock(psubblk);
			}
		}
		else if (pblk->flags & MMCMP_16BIT)
		{ /* Data is 16-bit packed */
			MMCMPBITBUFFER bb;
			LPBYTE pDest = pBuffer + psubblk->unpk_pos;
			DWORD dwSize = psubblk->unpk_size;
			DWORD dwPos = 0;
			UINT numbits = pblk->num_bits;
			UINT subblk = 0, oldval = 0;

			if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
				goto err;
			bb.bitcount = 0;
			bb.bitbuffer = 0;
			bb.pSrc = lpMemFile+dwMemPos+pblk->tt_entries;
			bb.pEnd = lpMemFile+dwMemPos+pblk->pk_size;
			while (1)
			{
				UINT newval = 0x10000;
				DWORD d = MMCMPBITBUFFER_GetBits(&bb, numbits+1);

				if (d >= MMCMP16BitCommands[numbits])
				{
					UINT nFetch = MMCMP16BitFetch[numbits];
					UINT newbits = MMCMPBITBUFFER_GetBits(&bb, nFetch) + ((d - MMCMP16BitCommands[numbits]) << nFetch);
					if (newbits != numbits)
					{
						numbits = newbits & 0x0F;
					} else
					{
						if ((d = MMCMPBITBUFFER_GetBits(&bb, 4)) == 0x0F)
						{
							if (MMCMPBITBUFFER_GetBits(&bb, 1)) break;
							newval = 0xFFFF;
						} else
						{
							newval = 0xFFF0 + d;
						}
					}
				} else
				{
					newval = d;
				}
				if (newval < 0x10000)
				{
					newval = (newval & 1) ? (UINT)(-(LONG)((newval+1) >> 1)) : (UINT)(newval >> 1);
					if (pblk->flags & MMCMP_DELTA)
					{
						newval += oldval;
						oldval = newval;
					} else
					if (!(pblk->flags & MMCMP_ABS16))
					{
						newval ^= 0x8000;
					}
					if (pBufEnd - pDest < 2) goto err;
					dwPos += 2;
					*pDest++ = (BYTE) (((WORD)newval) & 0xff);
					*pDest++ = (BYTE) (((WORD)newval) >> 8);
				}
				if (dwPos >= dwSize)
				{
					if (++subblk == pblk->sub_blk) break;
					dwPos = 0;
					SDL_memcpy(tmp1+20,lpMemFile+dwSubPos+subblk*8,8);
					swap_subblock(psubblk);
					if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
						goto err;
					dwSize = psubblk->unpk_size;
					pDest = pBuffer + psubblk->unpk_pos;
				}
			}
		}
		else
		{ /* Data is 8-bit packed */
			MMCMPBITBUFFER bb;
			LPBYTE pDest = pBuffer + psubblk->unpk_pos;
			DWORD dwSize = psubblk->unpk_size;
			DWORD dwPos = 0;
			UINT numbits = pblk->num_bits;
			UINT subblk = 0, oldval = 0;
			LPCBYTE ptable = lpMemFile+dwMemPos;

			if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
				goto err;
			bb.bitcount = 0;
			bb.bitbuffer = 0;
			bb.pSrc = lpMemFile+dwMemPos+pblk->tt_entries;
			bb.pEnd = lpMemFile+dwMemPos+pblk->pk_size;
			while (1)
			{
				UINT newval = 0x100;
				DWORD d = MMCMPBITBUFFER_GetBits(&bb, numbits+1);

				if (d >= MMCMP8BitCommands[numbits])
				{
					UINT nFetch = MMCMP8BitFetch[numbits];
					UINT newbits = MMCMPBITBUFFER_GetBits(&bb, nFetch) + ((d - MMCMP8BitCommands[numbits]) << nFetch);
					if (newbits != numbits)
					{
						numbits = newbits & 0x07;
					} else
					{
						if ((d = MMCMPBITBUFFER_GetBits(&bb, 3)) == 7)
						{
							if (MMCMPBITBUFFER_GetBits(&bb, 1)) break;
							newval = 0xFF;
						} else
						{
							newval = 0xF8 + d;
						}
					}
				} else
				{
					newval = d;
				}
				if (newval < 0x100)
				{
					int n = ptable[newval];
					if (pblk->flags & MMCMP_DELTA)
					{
						n += oldval;
						oldval = n;
					}
					pDest[dwPos++] = (BYTE)n;
				}
				if (dwPos >= dwSize)
				{
					if (++subblk == pblk->sub_blk) break;
					dwPos = 0;
					SDL_memcpy(tmp1+20,lpMemFile+dwSubPos+subblk*8,8);
					swap_subblock(psubblk);
					if (!MMCMP_IsDstBlockValid(psubblk, dwFileSize))
						goto err;
					dwSize = psubblk->unpk_size;
					pDest = pBuffer + psubblk->unpk_pos;
				}
			}
		}
	}
	*ppMemFile = pBuffer;
	*pdwMemLength = dwFileSize;
	return TRUE;

  err:
	SDL_free(pBuffer);
	return FALSE;
}


/* PowerPack PP20 Unpacker */

/* Code from Heikki Orsila's amigadepack 0.02
 * based on code by Stuart Caie <kyzer@4u.net>
 * This software is in the Public Domain
 *
 * Modified for xmp by Claudio Matsuoka, 08/2007
 * - merged mld's checks from the old depack sources. Original credits:
 *   - corrupt file and data detection
 *     (thanks to Don Adan and Dirk Stoecker for help and infos)
 *   - implemeted "efficiency" checks
 *   - further detection based on code by Georg Hoermann
 *
 * Modified for xmp by Claudio Matsuoka, 05/2013
 * - decryption code removed
 *
 * Modified for libmodplug by O. Sezer, Apr. 2015
 */

#define PP_READ_BITS(nbits, var) do {                          \
  bit_cnt = (nbits);                                           \
  while (bits_left < bit_cnt) {                                \
    if (buf_src <= src) return 0; /* out of source bits */     \
    bit_buffer |= (*--buf_src << bits_left);                   \
    bits_left += 8;                                            \
  }                                                            \
  (var) = 0;                                                   \
  bits_left -= bit_cnt;                                        \
  while (bit_cnt--) {                                          \
    (var) = ((var) << 1) | (bit_buffer & 1);                   \
    bit_buffer >>= 1;                                          \
  }                                                            \
} while(0)

#define PP_BYTE_OUT(byte) do {                                 \
  if (out <= dest) return 0; /* output overflow */             \
  *--out = (byte);                                             \
  written++;                                                   \
} while (0)

static BOOL ppDecrunch(LPCBYTE src, LPBYTE dest,
                       LPCBYTE offset_lens,
                       DWORD src_len, DWORD dest_len,
                       BYTE skip_bits)
{
  DWORD bit_buffer, x, todo, offbits, offset, written;
  LPCBYTE buf_src;
  LPBYTE out, dest_end;
  BYTE bits_left, bit_cnt;

  /* set up input and output pointers */
  buf_src = src + src_len;
  out = dest_end = dest + dest_len;

  written = 0;
  bit_buffer = 0;
  bits_left = 0;

  /* skip the first few bits */
  PP_READ_BITS(skip_bits, x);

  /* while there are input bits left */
  while (written < dest_len) {
    PP_READ_BITS(1, x);
    if (x == 0) {
      /* 1bit==0: literal, then match. 1bit==1: just match */
      todo = 1; do { PP_READ_BITS(2, x); todo += x; } while (x == 3);
      while (todo--) { PP_READ_BITS(8, x); PP_BYTE_OUT(x); }

      /* should we end decoding on a literal, break out of the main loop */
      if (written == dest_len) break;
    }

    /* match: read 2 bits for initial offset bitlength / match length */
    PP_READ_BITS(2, x);
    offbits = offset_lens[x];
    todo = x+2;
    if (x == 3) {
      PP_READ_BITS(1, x);
      if (x==0) offbits = 7;
      PP_READ_BITS(offbits, offset);
      do { PP_READ_BITS(3, x); todo += x; } while (x == 7);
    }
    else {
      PP_READ_BITS(offbits, offset);
    }
    if ((out + offset) >= dest_end) return 0; /* match overflow */
    while (todo--) { x = out[offset]; PP_BYTE_OUT(x); }
  }

  /* all output bytes written without error */
  return 1;
  /* return (src == buf_src) ? 1 : 0; */
}

BOOL PP20_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength)
{
	DWORD dwMemLength = *pdwMemLength;
	LPCBYTE lpMemFile = *ppMemFile;
	DWORD dwDstLen;
	BYTE tmp[4], skip;
	LPBYTE pBuffer;

	if ((!lpMemFile) || (dwMemLength < 256) || (SDL_memcmp(lpMemFile,"PP20",4) != 0))
		return FALSE;
	if (dwMemLength & 3) /* file length should be a multiple of 4 */
		return FALSE;

	/* PP FORMAT:
	 *      1 longword identifier           'PP20' or 'PX20'
	 *     [1 word checksum (if 'PX20')     $ssss]
	 *      1 longword efficiency           $eeeeeeee
	 *      X longwords crunched file       $cccccccc,$cccccccc,...
	 *      1 longword decrunch info        'decrlen' << 8 | '8 bits other info'
	 */

	SDL_memcpy(tmp,&lpMemFile[dwMemLength-4],4);
	dwDstLen = (tmp[0]<<16) | (tmp[1]<<8) | tmp[2];
	skip = tmp[3];
	if (skip > 32) return 0;

	/* original pp20 only support efficiency
	 * from 9 9 9 9 up to 9 10 12 13, afaik,
	 * but the xfd detection code says this...
	 *
	 * move.l 4(a0),d0
	 * cmp.b #9,d0
	 * blo.b .Exit
	 * and.l #$f0f0f0f0,d0
	 * bne.s .Exit
	 */
	SDL_memcpy(tmp,&lpMemFile[4],4);
	if ((tmp[0] < 9) || (tmp[0] & 0xf0)) return FALSE;
	if ((tmp[1] < 9) || (tmp[1] & 0xf0)) return FALSE;
	if ((tmp[2] < 9) || (tmp[2] & 0xf0)) return FALSE;
	if ((tmp[3] < 9) || (tmp[3] & 0xf0)) return FALSE;

	if ((dwDstLen < 512) || (dwDstLen > 0x400000) || (dwDstLen > 16*dwMemLength))
		return FALSE;
	if ((pBuffer = (LPBYTE)SDL_malloc(dwDstLen)) == NULL)
		return FALSE;

	if (!ppDecrunch(lpMemFile+8, pBuffer, tmp, dwMemLength-12, dwDstLen, skip)) {
		SDL_free(pBuffer);
		return FALSE;
	}

	*ppMemFile = pBuffer;
	*pdwMemLength = dwDstLen;
	return TRUE;
}
