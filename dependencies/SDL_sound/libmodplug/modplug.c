/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

#include "modplug.h"
#include "libmodplug.h"

void ModPlug_Quit(void) { /* does nothing. */ }

int ModPlug_Init(void)
{
    extern void init_modplug_filters(void);
    init_modplug_filters();
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
	if (maxtime != 0.0f)
		postime = (float)maxpos / (float)maxtime;

	CSoundFile_SetCurrentPos((CSoundFile *) file, (int)(millisecond * postime));
}

// inefficient, but oh well.
char *rwops_fgets(char *buf, int buflen, SDL_RWops *rwops)
{
    char *retval = buf;
    if (!buflen) return buf;
    while (buflen > 1) {
        char ch;
        if (SDL_RWread(rwops, &ch, 1, 1) != 1) {
            break;
        }
        *(buf++) = ch;
        buflen--;
        if (ch == '\n') {
            break;
        }
    }
    *(buf) = '\0';
    return retval;
}

long mmftell(MMFILE *mmfile)
{
	return mmfile->pos;
}

void mmfseek(MMFILE *mmfile, long p, int whence)
{
	int newpos = mmfile->pos;
	switch(whence) {
		case SEEK_SET:
			newpos = p;
			break;
		case SEEK_CUR:
			newpos += p;
			break;
		case SEEK_END:
			newpos = mmfile->sz + p;
			break;
	}
	if (newpos < mmfile->sz)
		mmfile->pos = newpos;
	else {
		mmfile->error = 1;
//		printf("WARNING: seeking too far\n");
	}
}

void mmreadUBYTES(BYTE *buf, long sz, MMFILE *mmfile)
{
	int sztr = sz;
	// do not overread.
	if (sz > mmfile->sz - mmfile->pos)
		sztr = mmfile->sz - mmfile->pos;
	SDL_memcpy(buf, &mmfile->mm[mmfile->pos], sztr);
	mmfile->pos += sz;
	// if truncated read, populate the rest of the array with zeros.
	if (sz > sztr)
		SDL_memset(buf+sztr, 0, sz-sztr);
}

void mmreadSBYTES(char *buf, long sz, MMFILE *mmfile)
{
	// do not overread.
	if (sz > mmfile->sz - mmfile->pos)
		sz = mmfile->sz - mmfile->pos;
	SDL_memcpy(buf, &mmfile->mm[mmfile->pos], sz);
	mmfile->pos += sz;
}

BYTE mmreadUBYTE(MMFILE *mmfile)
{
	BYTE b;
	b = (BYTE)mmfile->mm[mmfile->pos];
	mmfile->pos++;
	return b;
}

void mmfclose(MMFILE *mmfile)
{
	SDL_free(mmfile);
}

int mmfeof(MMFILE *mmfile)
{
	if( mmfile->pos < 0 ) return TRUE;
	if( mmfile->pos < mmfile->sz ) return FALSE;
	return TRUE;
}

int mmfgetc(MMFILE *mmfile)
{
	int b = EOF;
	if( mmfeof(mmfile) ) return EOF;
	b = mmfile->mm[mmfile->pos];
	mmfile->pos++;
	if( b=='\r' && !mmfeof(mmfile) && mmfile->mm[mmfile->pos] == '\n' ) {
		b = '\n';
		mmfile->pos++;
	}
	return b;
}

void mmfgets(char buf[], unsigned int bufsz, MMFILE *mmfile)
{
	int i = 0;
    int b;
	for( i=0; i<(int)bufsz-1; i++ ) {
		b = mmfgetc(mmfile);
		if( b==EOF ) break;
		buf[i] = b;
		if( b == '\n' ) break;
	}
	buf[i] = '\0';
}

