/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

// required for mremap(2)
#if defined( __linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>

#include "../qcommon/qcommon.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <machine/param.h>
#endif

#if !defined(__linux__) && !defined(round_page)
#define PAGE_MASK	(getpagesize()-1)
#define round_page(x)	(((x) + PAGE_MASK) & ~PAGE_MASK)
#endif

byte           *membase;
int		maxhunksize;
int		curhunksize;
int		curtime;

static char	findbase[MAX_OSPATH];
static char	findpath[MAX_OSPATH];
static char	findpattern[MAX_OSPATH];
static DIR     *fdir;

void *
Hunk_Begin(int maxsize, char *name)
{
	/* reserve a huge chunk of memory, but don't commit any yet */
	maxhunksize = maxsize + sizeof(int);
	curhunksize = 0;

	membase = mmap(0, maxhunksize, PROT_READ | PROT_WRITE,
	    MAP_PRIVATE | MAP_ANON, -1, 0);

	if (membase == NULL || membase == (byte *) - 1)
		Sys_Error("unable to virtual allocate %d bytes for %s", maxsize, name);

	*((int *)membase) = curhunksize;

	return membase + sizeof(int);
}

void *
Hunk_Alloc(int size)
{
	byte           *buf;

	/* round to cacheline */
	size = (size + 31) & ~31;

	if (curhunksize + size > maxhunksize)
		Sys_Error("Hunk_Alloc overflow");

	buf = membase + sizeof(int) + curhunksize;
	curhunksize += size;

	return buf;
}

int
Hunk_End(void)
{
	byte           *n;

#ifndef __linux__
	size_t		old_size = maxhunksize;
	size_t		new_size = curhunksize + sizeof(int);
	void           *unmap_base;
	size_t		unmap_len;

	new_size = round_page(new_size);
	old_size = round_page(old_size);
	if (new_size > old_size)
		n = 0;		/* error */
	else if (new_size < old_size) {
		unmap_base = (caddr_t) (membase + new_size);
		unmap_len = old_size - new_size;
		n = munmap(unmap_base, unmap_len) + membase;
	}
#else
	n = mremap(membase, maxhunksize, curhunksize + sizeof(int), 0);
#endif
	if (n != membase)
		Sys_Error("Hunk_End: Could not remap virtual block (%d)", errno);
	*((int *)membase) = curhunksize + sizeof(int);

	return curhunksize;
}

void
Hunk_Free(void *base)
{
	byte           *m;

	if (base) {
		m = ((byte *) base) - sizeof(int);
		if (munmap(m, *((int *)m)))
			Sys_Error("Hunk_Free: munmap failed (%d)", errno);
	}
}

/*
 * ================ Sys_Milliseconds ================
 */
int
Sys_Milliseconds(void)
{
	struct timeval	tp;
	struct timezone	tzp;
	static int	secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase) {
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000;
	}
	curtime = (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;

	return curtime;
}

void
Sys_Mkdir(char *path)
{
	mkdir(path, 0777);
}

void
Sys_Rmdir(char *path)
{
	rmdir(path);
}

/*
 * ================= Sys_GetCurrentDirectory =================
 */
char *
Sys_GetCurrentDirectory(void)
{
	static char	dir[MAX_OSPATH];

	if (!getcwd(dir, sizeof(dir)))
		Sys_Error("Couldn't get current working directory");

	return dir;
}

char *
strlwr(char *s)
{
	char           *p = s;

	while (*s) {
		*s = tolower(*s);
		s++;
	}
	return p;
}

/* ============================================ */

static qboolean
CompareAttributes(char *path, char *name, unsigned musthave, unsigned canthave)
{
	struct stat	st;
	char		fn[MAX_OSPATH];

	/* . and .. never match */
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return qfalse;

	Com_sprintf(fn, sizeof(fn), "%s/%s", path, name);

	if (stat(fn, &st) == -1)
		return (qfalse);	/* shouldn't happen */

	if (((musthave & SFF_HIDDEN) && name[0] != '.') ||
	    ((canthave & SFF_HIDDEN) && name[0] == '.'))
		return (qfalse);

	if (((musthave & SFF_RDONLY) && access(fn, W_OK) != 0) ||
	    ((canthave & SFF_RDONLY) && access(fn, W_OK) == 0))
		return (qfalse);

	if (((musthave & SFF_SUBDIR) && !(st.st_mode & S_IFDIR)) ||
	    ((canthave & SFF_SUBDIR) && (st.st_mode & S_IFDIR)))
		return (qfalse);

	return (qtrue);
}

char *
Sys_FindFirst(char *path, unsigned musthave, unsigned canthave)
{
	char           *p;

	if (fdir)
		Sys_Error("Sys_FindFirst without close");

	/* COM_FilePath (path, findbase); */
	Q_strncpyz(findbase, path, sizeof(findbase));

	if ((p = strrchr(findbase, '/')) != NULL) {
		*p = '\0';
		Q_strncpyz(findpattern, p + 1, sizeof(findpattern));
	} else
		Q_strncpyz(findpattern, "*", sizeof(findpattern));

	if (strcmp(findpattern, "*.*") == 0)
		Q_strncpyz(findpattern, "*", sizeof(findpattern));

	fdir = opendir(findbase);

	return (Sys_FindNext(musthave, canthave));
}

char *
Sys_FindNext(unsigned musthave, unsigned canthave)
{
	struct dirent  *d;

	if (fdir == NULL)
		return (NULL);

	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || fnmatch(findpattern, d->d_name, 0) == 0) {
			/* if (*findpattern) */
			/* printf("%s matched %s\n", findpattern, d->d_name); */
			if (CompareAttributes(findbase, d->d_name, musthave, canthave)) {
				Com_sprintf(findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return (findpath);
			}
		}
	}

	return (NULL);
}

void
Sys_FindClose(void)
{
	if (fdir != NULL) {
		closedir(fdir);
		fdir = NULL;
	}
}
