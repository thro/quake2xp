/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../qcommon/qcommon.h"
#include "winquake.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

//===============================================================================

int		hunkcount;


byte	*membase;
int		hunkmaxsize;
int		cursize;
char	hunk_name[MAX_OSPATH];

#define	VIRTUAL_ALLOC

void *Hunk_Begin (int maxsize, char *name) {
	// reserve a huge chunk of memory, but don't commit any yet
	cursize = 0;
	hunkmaxsize = maxsize;
	memset (hunk_name, 0, strlen (name) + 1);
	strcpy (hunk_name, name);

#ifdef VIRTUAL_ALLOC
	membase = VirtualAlloc (NULL, maxsize, MEM_RESERVE, PAGE_NOACCESS);
#else
	membase = malloc (maxsize);
	memset (membase, 0, maxsize);

#endif
	if (!membase)
		Sys_Error ("VirtualAlloc reserve failed %s", hunk_name);
	return (void *)membase;


}

void *Hunk_Alloc (int size) {
	void	*buf;

	// round to cacheline
	size = (size + 31)&~31;

#ifdef VIRTUAL_ALLOC
	// commit pages as needed
	//	buf = VirtualAlloc (membase+cursize, size, MEM_COMMIT, PAGE_READWRITE);
	buf = VirtualAlloc (membase, cursize + size, MEM_COMMIT, PAGE_READWRITE);
	if (!buf) {
		FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError (), MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, 0, NULL);
		Sys_Error ("VirtualAlloc commit failed.\n%s", buf);
	}
#endif
	cursize += size;
	if (cursize > hunkmaxsize)
		Sys_Error ("Hunk_Alloc overflow");

	return (void *)(membase + cursize - size);
}

/*
int Hunk_End (void)
{

// free the remaining unused virtual memory
#if 0
void	*buf;

// write protect it
buf = VirtualAlloc (membase, cursize, MEM_COMMIT, PAGE_READONLY);
if (!buf)
Sys_Error ("VirtualAlloc commit failed");
#endif

hunkcount++;
//Com_Printf ("hunkcount: %i\n", hunkcount);
return cursize;
}
*/

int Hunk_End () {
	/// Berserker: освободим неиспользуемые, но зарезервированные блоки памяти!
#ifdef VIRTUAL_ALLOC
	int size;
	size = ceil (cursize / 4096.0f) * 4096;     // page size always is 4096, 8192, etc...
	if (hunkmaxsize > size)
		VirtualFree (membase + size, hunkmaxsize - size, MEM_RELEASE);
	hunkmaxsize = 0;
#endif
	///     hunkcount++;
	return cursize;
}



void Hunk_Free (void *base) {
	if (base)
#ifdef VIRTUAL_ALLOC
		VirtualFree (base, 0, MEM_RELEASE);
#else
		free (base);
#endif

	hunkcount--;
}

//===============================================================================


/*
================
Sys_Milliseconds
================
*/

int	curtime;
int Sys_Milliseconds (void) {
	static int		base;
	static qboolean	initialized = qfalse;

	if (!initialized) {	// let base retain 16 bits of effectively random data
		base = timeGetTime () & 0xffff0000;
		initialized = qtrue;
	}

	curtime = timeGetTime () - base;

	return curtime;
}

void Sys_Mkdir (char *path) {
	_mkdir (path);
}
void Sys_ChDir (char *path) {
	_chdir (path);
}
//============================================

char	findbase[MAX_OSPATH];
char	findpath[MAX_OSPATH];
int		findhandle;

static qboolean CompareAttributes (unsigned found, unsigned musthave, unsigned canthave) {
	if ((found & _A_RDONLY) && (canthave & SFF_RDONLY))
		return qfalse;
	if ((found & _A_HIDDEN) && (canthave & SFF_HIDDEN))
		return qfalse;
	if ((found & _A_SYSTEM) && (canthave & SFF_SYSTEM))
		return qfalse;
	if ((found & _A_SUBDIR) && (canthave & SFF_SUBDIR))
		return qfalse;
	if ((found & _A_ARCH) && (canthave & SFF_ARCH))
		return qfalse;

	if ((musthave & SFF_RDONLY) && !(found & _A_RDONLY))
		return qfalse;
	if ((musthave & SFF_HIDDEN) && !(found & _A_HIDDEN))
		return qfalse;
	if ((musthave & SFF_SYSTEM) && !(found & _A_SYSTEM))
		return qfalse;
	if ((musthave & SFF_SUBDIR) && !(found & _A_SUBDIR))
		return qfalse;
	if ((musthave & SFF_ARCH) && !(found & _A_ARCH))
		return qfalse;

	return qtrue;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canthave) {
	struct _finddata_t findinfo;

	if (findhandle)
		Sys_Error ("Sys_BeginFind without close");
	findhandle = 0;

	COM_FilePath (path, findbase);
	findhandle = _findfirst (path, &findinfo);

	while ((findhandle != -1)) {
		if (CompareAttributes (findinfo.attrib, musthave, canthave)) {
			Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
			return findpath;
		}
		else if (_findnext (findhandle, &findinfo) == -1) {
			_findclose (findhandle);
			findhandle = -1;
		}
	}

	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canthave) {
	struct _finddata_t findinfo;

	if (findhandle == -1)
		return NULL;


	while (_findnext (findhandle, &findinfo) != -1) {
		if (CompareAttributes (findinfo.attrib, musthave, canthave)) {
			Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
			return findpath;
		}
	}

	return NULL;
}

void Sys_FindClose (void) {
	if (findhandle != -1)
		_findclose (findhandle);
	findhandle = 0;
}


//============================================

