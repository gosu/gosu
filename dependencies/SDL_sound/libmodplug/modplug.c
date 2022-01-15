/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

#include "modplug.h"
#include "libmodplug.h"

void ModPlug_Quit(void) {
}
int ModPlug_Init(void)
{
	init_modplug_filters();
	SDL_srand(SDL_GetPerformanceCounter());
	return 1;
}

ModPlugFile* ModPlug_Load(const void* data, int size, const ModPlug_Settings *settings)
{
	return (ModPlugFile *) new_CSoundFile((const BYTE*)data, size, settings);
}

void ModPlug_Unload(ModPlugFile* file)
{
	delete_CSoundFile((CSoundFile *) file);
}

int ModPlug_Read(ModPlugFile* file, void* buffer, int size)
{
	CSoundFile *sndfile = (CSoundFile *) file;
	return CSoundFile_Read(sndfile, buffer, size) * sndfile->gSampleSize;
}

int ModPlug_GetLength(ModPlugFile* file)
{
	return CSoundFile_GetLength((CSoundFile *) file, FALSE, TRUE) * 1000;
}

void ModPlug_Seek(ModPlugFile* file, int millisecond)
{
	int maxpos;
	int maxtime = CSoundFile_GetLength((CSoundFile *) file, FALSE, TRUE) * 1000;
	float postime;

	if(millisecond > maxtime)
		millisecond = maxtime;
	maxpos = CSoundFile_GetMaxPosition((CSoundFile *) file);
	postime = 0.0f;
	if (maxtime != 0)
		postime = (float)maxpos / (float)maxtime;

	CSoundFile_SetCurrentPos((CSoundFile *) file, (int)(millisecond * postime));
}
