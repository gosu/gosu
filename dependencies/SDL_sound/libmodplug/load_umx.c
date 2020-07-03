/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
*/

#include "libmodplug.h"

#define MODMAGIC_OFFSET	(20+31*30+130)

BOOL CSoundFile_ReadUMX(CSoundFile *_this, const BYTE *lpStream, DWORD dwMemLength)
//---------------------------------------------------------------
{
	if ((!lpStream) || (dwMemLength < 0x800)) return FALSE;
	// Rip Mods from UMX
	if ((bswapLE32(*((DWORD *)(lpStream+0x20))) < dwMemLength)
	 && (bswapLE32(*((DWORD *)(lpStream+0x18))) <= dwMemLength - 0x10)
	 && (bswapLE32(*((DWORD *)(lpStream+0x18))) >= dwMemLength - 0x200))
	{
		for (UINT uscan=0x40; uscan<0x500; uscan++)
		{
			DWORD dwScan = bswapLE32(*((DWORD *)(lpStream+uscan)));
			// IT
			if (dwScan == 0x4D504D49)
			{
				DWORD dwRipOfs = uscan;
				return CSoundFile_ReadIT(_this, lpStream + dwRipOfs, dwMemLength - dwRipOfs);
			}
			// S3M
			if (dwScan == 0x4D524353)
			{
				DWORD dwRipOfs = uscan - 44;
				return CSoundFile_ReadS3M(_this, lpStream + dwRipOfs, dwMemLength - dwRipOfs);
			}
			// XM
			if (!SDL_strncasecmp((LPCSTR)(lpStream+uscan), "Extended Module", 15))
			{
				DWORD dwRipOfs = uscan;
				return CSoundFile_ReadXM(_this, lpStream + dwRipOfs, dwMemLength - dwRipOfs);
			}
			// MOD
			if ((uscan > MODMAGIC_OFFSET) && (dwScan == 0x2e4b2e4d))
			{
				DWORD dwRipOfs = uscan - MODMAGIC_OFFSET;
				return CSoundFile_ReadMod(_this, lpStream+dwRipOfs, dwMemLength-dwRipOfs);
			}
		}
	}
	return FALSE;
}

