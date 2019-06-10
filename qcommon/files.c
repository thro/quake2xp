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

#include "qcommon.h"
#include "unzip.h" // unzip stuff thx to Vic

// define this to dissalow any data but the demo pak file
//#define   NO_ADDONS

// if a packfile directory differs from this, it is assumed to be hacked
// Full version
#define	PAK0_CHECKSUM	0x40e614e0
// Demo
//#define   PAK0_CHECKSUM   0xb2c6d7ea
// OEM
//#define   PAK0_CHECKSUM   0x78e135c

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/


// in memory
typedef struct {
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef enum {
	pt_zip,
	pt_pak,
	pt_file
} packtype_t;

typedef struct pack_s {
	char		filename[MAX_OSPATH];
	qFILE		qfile;
	int			numfiles;
	int			size;
	packfile_t	*files;
	packtype_t	packtype;
} pack_t;

char	fs_gamedir[MAX_OSPATH];
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_gamedirvar;


typedef struct searchpath_s {
	char				filename[MAX_OSPATH];
	pack_t				*pack;		// only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t *curr_search = NULL;
int curr_pak_index = 0;

searchpath_t	*fs_searchpaths;
searchpath_t	*fs_base_searchpaths;	// without gamedirs

char gameDLLPath[MAX_OSPATH];

/*

All of Quake's data access is through a hierchal file system, but the contents of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This can be overridden with the "-basedir" command line parm to allow code debugging in a different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated files (savegames, screenshots, demos, config files) will be saved to.  This can be overridden with the "-game" command line parameter.  The game directory can never be changed while quake is executing.  This is a precacution against having a malicious server instruct clients to write files over areas they shouldn't.

*/


/*
================
FS_filelength
================
*/
int FS_filelength (qFILE *f) {
	int		pos;
	int		end;

	pos = ftell (f->f);
	fseek (f->f, 0, SEEK_END);
	end = ftell (f->f);
	fseek (f->f, pos, SEEK_SET);

	return end;
}

int FS_filelength2 (FILE *f) {
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}


/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
void FS_CreatePath (char *path) {
	char	*ofs;

	for (ofs = path + 1; *ofs; ofs++) {
		if (*ofs == '/') {	// create the directory
			*ofs = 0;
			Sys_Mkdir (path);
			*ofs = '/';
		}
	}
}


/*
==============
FS_FCloseFile

For some reason, other dll's can't just call fclose()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile (qFILE *f) {
	if (f->f) {
		fclose (f->f);
	}
	else if (f->z) {
		unzCloseCurrentFile (f->z);
	}
	f->f = NULL;
	f->z = NULL;
}


// RAFAEL
/*
	Developer_searchpath
	*/
int	Developer_searchpath (int who) {

	int				ch;
	searchpath_t	*search;

	if (who == 1) // xatrix
		ch = 'x';
	else if (who == 2)
		ch = 'r';

	for (search = fs_searchpaths; search; search = search->next) {
		if (strstr (search->filename, "xatrix"))
			return 1;

		if (strstr (search->filename, "rogue"))
			return 2;
	}
	return (0);
}

qboolean modName(const char *gameDir) {

	searchpath_t	*search;

	for (search = fs_searchpaths; search; search = search->next)
		if (strstr(search->filename, gameDir))
			return qtrue;

	return qfalse;
}

/*
===========
FS_FOpenFile

Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/
int file_from_pak = 0;
int zipdata;
int FS_FOpenFile (const char *filename, qFILE *qfile) {
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;

	qfile->f = 0;
	qfile->z = 0;
	file_from_pak = 0;

	// search through the path, one element at a time
	for (search = fs_searchpaths; search; search = search->next) {
		// is the element a pak file?
		if (search->pack) {
			if (search->pack->packtype == pt_pak) { // pack is a pack
				// look through all the pak file elements
				pak = search->pack;
				for (i = 0; i < pak->numfiles; i++) {
					if (!Q_strcasecmp (pak->files[i].name, filename)) {	// found it!
						file_from_pak = 1;
						Com_DPrintf ("PackFile: %s : %s\n", pak->filename, filename);
						// open a new file on the pakfile
						qfile->f = fopen (pak->filename, "rb");
						if (!qfile->f)
							Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename);
						fseek (qfile->f, pak->files[i].filepos, SEEK_SET);
						return pak->files[i].filelen;
					}
				}
			}
			else if (search->pack->packtype == pt_zip) {
				// pack is zip
				// look through all the pak file elements
				pak = search->pack;
				if (unzLocateFile (pak->qfile.z, filename, 2) == UNZ_OK) {
					// found it!
					if (unzOpenCurrentFile (pak->qfile.z) == UNZ_OK) {
						unz_file_info info;
						file_from_pak = 1;
						Com_DPrintf ("PackFile: %s : %s\n", pak->filename, filename);
						if (unzGetCurrentFileInfo (pak->qfile.z, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
							Com_Error (ERR_FATAL, "Couldn't get size of %s in %s", filename, pak->filename);
						}
						qfile->z = pak->qfile.z;
						return info.uncompressed_size;
					}
				}
			}
		}
		else {
			// check a file in the directory tree
			Com_sprintf (netpath, sizeof(netpath), "%s/%s", search->filename, filename);
			qfile->f = fopen (netpath, "rb");
			if (!qfile->f) continue;
			Com_DPrintf ("FindFile: %s\n", netpath);
			return FS_filelength (qfile);
		}
	}
	Com_DPrintf ("FindFile: can't find %s\n", filename);

	return -1;
}


qboolean FS_FileExists (char *path) {
	qFILE f;

	if (FS_FOpenFile (path, &f)) {
		FS_FCloseFile (&f);
		return (qtrue);
	}

	return (qfalse);
}

/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
void CDAudio_Stop (void);
#define	MAX_READ	0x10000		// read in blocks of 64k
void FS_Read (void *buffer, int len, qFILE *qfile) {
	int		block, remaining;
	int		read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	if (qfile->z) {
		read = unzReadCurrentFile (qfile->z, buf, len);
		if (read == -1) {
			Com_Error (ERR_FATAL, "FS_ReadFromZipFile: -1 bytes read");
		}
		return;
	}

	// read in chunks for progress bar
	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		if (block > MAX_READ)
			block = MAX_READ;
		read = fread (buf, 1, block, qfile->f);
		if (read == 0) {
			// we might have been trying to read from a CD
			if (!tries) {
				tries = 1;
				CDAudio_Stop ();
			}
			else {
				Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
			}
		}

		if (read == -1) {
			Com_Error (ERR_FATAL, "FS_Read: -1 bytes read");
		}
		// do some progress bar thing here...
		remaining -= read;
		buf += read;
	}
}

/*
============
FS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/
int FS_LoadFile (const char *path, void **buffer) {
	qFILE	qfile;
	byte	*buf;
	int		len;

	// quiet compiler warning
	buf = NULL;

	// look for it in the filesystem or pack files
	qfile.f = NULL;
	qfile.z = NULL;
	len = FS_FOpenFile (path, &qfile);

	if (!qfile.f && !qfile.z) {
		if (buffer)
			*buffer = NULL;
		return -1;
	}

	if (!buffer) {
		FS_FCloseFile (&qfile);
		return len;
	}

	buf = Z_Malloc (len + 1);

	buf[len] = 0; // safety backup for some cases (eg. parser)
	*buffer = buf;

	FS_Read (buf, len, &qfile);
	FS_FCloseFile (&qfile);

	return len;
}


/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer) {
	Z_Free (buffer);
}

/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/


pack_t *FS_LoadZipFile (char *packfile) {
	pack_t	*pack;

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);
	pack->qfile.z = unzOpen (packfile);

	if (!pack->qfile.z) {
		Z_Free (pack);
		Com_Error (ERR_FATAL, "%s is not really a zip file", packfile);
	}
	pack->numfiles = Unz_NumEntries (pack->qfile.z);
	pack->packtype = pt_zip;

	Com_Printf (S_COLOR_YELLOW"Added packfile "S_COLOR_GREEN"%s"S_COLOR_YELLOW" ("S_COLOR_GREEN"%i "S_COLOR_YELLOW"files)\n", packfile, pack->numfiles);
	return pack;
}


pack_t *FS_LoadPackFile (char *packfile) {
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned		checksum;

	if (fs_OriginalPaksOnly->integer) { //Load ONLY original q2 data!!!
#ifdef _WIN32
		strlwr (packfile);
#endif
		if (!strstr (packfile, "pak0.pak") && !strstr (packfile, "pak1.pak") && !strstr (packfile, "pak2.pak"))
			return NULL;
	}

	packhandle = fopen (packfile, "rb");

	if (!packhandle) return NULL;

	fread (&header, 1, sizeof(header), packhandle);

	if (LittleLong (header.ident) != IDPAKHEADER) {
		Com_Error (ERR_FATAL, "%s is not a packfile", packfile);
	}
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK) {
		Com_Error (ERR_FATAL, "%s has %i files", packfile, numpackfiles);
	}
	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

	// crc the directory to check for modifications
	checksum = Com_BlockChecksum ((void *)info, header.dirlen);

	// parse the directory
	for (i = 0; i < numpackfiles; i++) {
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong (info[i].filepos);
		newfiles[i].filelen = LittleLong (info[i].filelen);
	}

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);
	pack->qfile.f = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	pack->packtype = pt_pak;

	Com_Printf (S_COLOR_YELLOW"Added packfile "S_COLOR_GREEN"%s"S_COLOR_YELLOW" ("S_COLOR_GREEN"%i "S_COLOR_YELLOW"files)\n", packfile, numpackfiles);
	return pack;
}

static int SortListPtrs (const void *data1, const void *data2) {
	// XXX: we have pointers to strings here!
	return Q_stricmp (*(char * const *)data1, *(char * const *)data2);
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ...
================
*/
void FS_AddGameDirectory (char *dir) {
	searchpath_t		*search;
	pack_t				*pak;
	char				pattern[MAX_OSPATH];
	int					i;
	char				**paklist;
	int					nfiles;

	Com_DPrintf ("Added search path '%s'\n", dir);
	strcpy (fs_gamedir, dir);

	// Get list of PAK files
	sprintf (pattern, "%s/*.pak", dir);
	paklist = FS_ListFiles (pattern, &nfiles, 0, SFF_SUBDIR);
	if (paklist != NULL) {
		qsort ((void *)paklist, nfiles, sizeof(char*), SortListPtrs);

		// Add each pak file from our list to the search path
		for (i = 0; i < nfiles; i++) {
			pak = FS_LoadPackFile (paklist[i]);
			if (!pak) continue;

			search = Z_Malloc (sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;
		}
		FS_FreeList (paklist, nfiles);
	}
	// -----------------------------------------------------

	// Get list of PKX files
	sprintf (pattern, "%s/*.pkx", dir);
	paklist = FS_ListFiles (pattern, &nfiles, 0, SFF_SUBDIR);
	if (paklist != NULL) {
		qsort ((void *)paklist, nfiles, sizeof(char*), SortListPtrs);

		// Add each pak file from our list to the search path
		for (i = 0; i < nfiles; i++) {
			pak = FS_LoadZipFile (paklist[i]);
			if (!pak)
				continue;

			search = Z_Malloc (sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;
		}
		FS_FreeList (paklist, nfiles);
	}

	// add the directory to the search path here, so it overrides pak/pkx
	search = Z_Malloc (sizeof(searchpath_t));
	strcpy (search->filename, dir);
	search->next = fs_searchpaths;
	fs_searchpaths = search;
}

/*
============
FS_Gamedir

Called to find where to write a file (demos, savegames, etc)
============
*/
char *FS_Gamedir (void) {
	if (*fs_gamedir)
		return fs_gamedir;
	else
		return BASEDIRNAME;
}

/*
=============
FS_ExecAutoexec
=============
*/
void FS_ExecAutoexec (void) {
	char *dir;
	char name[MAX_QPATH];

	dir = Cvar_VariableString ("gamedir");
	if (*dir)
		Com_sprintf (name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, dir);
	else
		Com_sprintf (name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, BASEDIRNAME);
	if (Sys_FindFirst (name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText ("exec autoexec.cfg\n");
	Sys_FindClose ();
}

/*
 * ================
 * FS_AddHomeAsGameDirectory
 *
 * Use ~/.quake2xp/dir as fs_gamedir.
 * ================
 */
static void
FS_AddHomeAsGameDirectory (char *dir) {
#ifndef _WIN32
	char		gdir[MAX_OSPATH];	/* Game directory. */
	char           *homedir;		/* Home directory. */

	if ((homedir = getenv ("HOME")) != NULL) {
		Com_sprintf (gdir, sizeof(gdir), "%s/.quake2xp/%s", homedir, dir);
		FS_AddGameDirectory (gdir);
	}
#else
	// TODO: add C:\Users\username\...\Quake2XP\dir to path here, to use for saving files
#endif
}


/*
================
FS_SetGamedir

Sets the gamedir and path to a different directory.
================
*/
void FS_SetGamedir (char *dir) {
	searchpath_t	*next;

	if (strstr (dir, "..") || strstr (dir, "/")
		|| strstr (dir, "\\") || strstr (dir, ":")) {
		Com_Printf ("Gamedir should be a single filename, not a path\n");
		return;
	}

	//
	// free up any current game dir info
	//
	while (fs_searchpaths != fs_base_searchpaths) {
		if (fs_searchpaths->pack) {
			FS_FCloseFile (&fs_searchpaths->pack->qfile);
			if (fs_searchpaths->pack->qfile.f) {
				Z_Free (fs_searchpaths->pack->files);
			}
			Z_Free (fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		Z_Free (fs_searchpaths);
		fs_searchpaths = next;
	}

	//
	// flush all data, so it will be forced to reload
	//
	if (dedicated && !dedicated->value)
		Cbuf_AddText ("vid_restart\nsnd_restart\n");

	Com_sprintf (fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);

	if (!strcmp (dir, BASEDIRNAME) || (*dir == 0)) {
		Cvar_FullSet ("gamedir", "", CVAR_SERVERINFO | CVAR_NOSET);
		Cvar_FullSet ("game", "", CVAR_LATCH | CVAR_SERVERINFO);
	}
	else {
		Cvar_FullSet ("gamedir", dir, CVAR_SERVERINFO | CVAR_NOSET);
		if (fs_cddir->string[0])
			FS_AddGameDirectory (va ("%s/%s", fs_cddir->string, dir));
		FS_AddGameDirectory (va ("%s/%s", fs_basedir->string, dir));
		FS_AddHomeAsGameDirectory (dir);
	}
}

//
// Pattern matching and listing
//

static int FS_ListFilesDir (char *findname, char **list, int len, unsigned musthave, unsigned canthave) {
	searchpath_t* search;
	const char *s;
	int nfound = 0;

	for (search = fs_searchpaths; search; search = search->next) {
		char path[MAX_OSPATH];

		if (search->pack != NULL)
			continue;

		Com_sprintf (path, sizeof(path), "%s/%s", search->filename, findname);
		s = Sys_FindFirst (path, musthave, canthave);
		while (s) {
			assert (nfound < len && "Please increase FSLF_MAX");

			if (s[strlen (s) - 1] != '.') {
				list[nfound] = strdup (s + strlen (search->filename) + 1);
				nfound++;
			}
			s = Sys_FindNext (musthave, canthave);
		}
		Sys_FindClose ();
	}

	return nfound;
}

static int countchs (const char *s, char c) {
	int i = 0, res = 0;
	while (s[i] != '\0') {
		if (s[i] == c)
			res++;
		i++;
	}
	return res;
}

/*
 * Compare file attributes (musthave and canthave) in packed files. If
 * "output" is not NULL, "size" is greater than zero and the file matches the
 * attributes then a copy of the matching string will be placed there (with
 * SFF_SUBDIR it changes).
 *
 * XXX: Sys_Find* doesn't support multi-depth patterns, but FS_MathPath does
 */

qboolean
FS_MatchPath (const char *findname, const char *name, char **output, unsigned musthave, unsigned canthave) {
	qboolean	 retval;
	char		*ptr;
	char		 buffer[MAX_OSPATH];

	strncpy (buffer, name, sizeof(buffer));

	if ((canthave & SFF_SUBDIR) && name[strlen (name) - 1] == '/')
		return (qfalse);

	if (musthave & SFF_SUBDIR) {
		if ((ptr = strrchr (buffer, '/')) != NULL)
			*ptr = '\0';

		else
			return (qfalse);
	}

	if ((musthave & SFF_HIDDEN) || (canthave & SFF_HIDDEN)) {
		if ((ptr = strrchr (buffer, '/')) == NULL)
			ptr = buffer;

		if (((musthave & SFF_HIDDEN) && ptr[1] != '.') ||
			((canthave & SFF_HIDDEN) && ptr[1] == '.'))
			return (qfalse);
	}

	if (canthave & SFF_RDONLY)
		return (qfalse);

	retval = Com_glob_match (findname, buffer);

	// check that the path depth does not increase
	if (countchs (buffer, '/') > countchs (findname, '/'))
		retval = qfalse;

	if (retval && output != NULL)
		*output = strdup (buffer);

	return (retval);
}

char *sInf = "OpenGL validation failed!\nEngine protection detected unsigned opengl driver!";

static int FS_ListFilesPacks (char *findname, char **list, int len, unsigned musthave, unsigned canthave) {
	searchpath_t	*search;
	int				i;
	int				nfound = 0;

	for (search = fs_searchpaths; search; search = search->next) {
		const pack_t* pak = search->pack;

		if (pak == NULL)
			continue;

		assert (nfound < len && "Please increase FSLF_MAX");

		if (search->pack->packtype == pt_pak) {
			for (i = 0; i < pak->numfiles; i++) {
				char *s;

				if (FS_MatchPath (findname, pak->files[i].name, &s, musthave, canthave)) {
					// XXX: in case of SFF_SUBDIR, a directory will appear as many times as nodes below itself
					if (nfound > 0 && strcmp (list[nfound - 1], s) == 0)
						continue;
					list[nfound] = s;
					nfound++;
				}
			}
		}
		else if (search->pack->packtype == pt_zip) {
			nfound += Unz_ListFiles (pak->qfile.z, findname, list + nfound, len - nfound, musthave, canthave);
		}
	}

	return nfound;
}

static char **FS_ListSortUnique (char **list, int len, int *numfiles) {
	int i, j, nfound = 0;

	if (len == 0) {
		*numfiles = 0;
		return list;
	}

	qsort ((void *)list, len, sizeof(char*), SortListPtrs);

	// Remove duplicate elements, replacing by NULL
	i = 0;
	while (i < len) {
		j = i + 1;
		while (j < len && Q_stricmp (list[i], list[j]) == 0) {
			free (list[j]);
			list[j] = NULL;
			j++;
		}
		i = j;
		nfound++;
	}

	// remove holes added in the previous step
	for (i = 0; i < len; i++) {
		// skip value if not a hole
		if (list[i] != NULL)
			continue;
		// find a valid string to fill the current hole
		j = i + 1;
		while (list[j] == NULL && j < len)
			j++;
		// if there are not any more values, exit
		if (j == len)
			break;
		// otherwise, fill the hole with the value found
		list[i] = list[j];
		list[j] = 0;
	}

	*numfiles = nfound;
	// adjust the size of the list
	return realloc (list, nfound * sizeof(char*));
}

/*
 * FS_ListFiles
 *
 * Create a list of files matching "findname" pattern under specified real path.
 *
 * XXX: use FS_ListFilesAll instead if possible.
 */
#define FSLF_MAX (1024*16)

char **FS_ListFiles (char *findname, int *numfiles, unsigned musthave, unsigned canthave) {
	const char *s;
	int nfound = 0;
	char **list = 0;

	list = malloc (FSLF_MAX * sizeof(char*));

	s = Sys_FindFirst (findname, musthave, canthave);
	while (s) {
		assert (nfound < FSLF_MAX && "Please increase FSLF_MAX");

		if (s[strlen (s) - 1] != '.') {
			list[nfound] = strdup (s);
			nfound++;
		}
		s = Sys_FindNext (musthave, canthave);
	}
	Sys_FindClose ();

	*numfiles = nfound;
	if (nfound > 0) {
		list = realloc (list, nfound * sizeof(char*));
		qsort ((void *)list, nfound, sizeof(char*), SortListPtrs);
		return list;
	}
	else {
		free (list);
		return NULL;
	}
}

/*
 * FS_ListFilesAll
 *
 * Create a list of files matching "findname" pattern under all search paths, including PAKs/PKXs.
 */
char **FS_ListFilesAll(char *findname, int *numfiles, unsigned musthave, unsigned canthave) {
	int nfound = 0;
	char **list = 0;

	list = malloc(FSLF_MAX * sizeof(char*));

	nfound += FS_ListFilesDir(findname, list + nfound, FSLF_MAX - nfound, musthave, canthave);
	if (!(canthave & SFF_RDONLY))
		nfound += FS_ListFilesPacks(findname, list + nfound, FSLF_MAX - nfound, musthave, canthave);

	list = FS_ListSortUnique(list, nfound, &nfound);

	*numfiles = nfound;
	if (nfound > 0) {
		list = realloc(list, nfound * sizeof(char*));
		return list;
	}
	else {
		free(list);
		return NULL;
	}
}

/*
 * Free list of files created by FS_ListFiles().
 */
void
FS_FreeList (char **list, int nfiles) {
	int		i;

	if (list == NULL || nfiles == 0)
		return;

	for (i = 0; i < nfiles; i++)
		free (list[i]);

	free (list);
}

/*
** FS_Dir_f
*/
void FS_Dir_f (void) {
	char	*path = NULL;
	char	findname[1024];
	char	wildcard[1024] = "*.*";
	char	**dirnames;
	int		ndirs;

	if (Cmd_Argc () != 1) {
		strcpy (wildcard, Cmd_Argv (1));
	}

	while ((path = FS_NextPath (path)) != NULL) {
		char *tmp = findname;

		Com_sprintf (findname, sizeof(findname), "%s/%s", path, wildcard);

		while (*tmp != 0) {
			if (*tmp == '\\')
				*tmp = '/';
			tmp++;
		}
		Com_Printf ("Directory of %s\n", findname);
		Com_Printf ("----\n");

		if ((dirnames = FS_ListFiles (findname, &ndirs, 0, 0)) != 0) {
			int i;

			for (i = 0; i < ndirs - 1; i++) {
				if (strrchr (dirnames[i], '/'))
					Com_Printf ("%s\n", strrchr (dirnames[i], '/') + 1);
				else
					Com_Printf ("%s\n", dirnames[i]);

				free (dirnames[i]);
			}
			free (dirnames);
		}
		Com_Printf ("\n");
	};
}

/*
============
FS_Path_f

============
*/
void FS_Path_f (void) {
	searchpath_t	*s;

	Com_Printf ("Current search path:\n");
	for (s = fs_searchpaths; s; s = s->next) {
		if (s == fs_base_searchpaths)
			Com_Printf ("----------\n");
		if (s->pack)
			Com_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf ("%s\n", s->filename);
	}
}


/*
============
FS_StripExtension
============
*/
void FS_StripExtension (const char *in, char *out, size_t size_out) {
	char *last = NULL;

	if (size_out == 0)
		return;

	while (*in && size_out > 1) {
		if (*in == '.')
			last = out;
		else if (*in == '/' || *in == '\\' || *in == ':')
			last = NULL;
		*out++ = *in++;
		size_out--;
	}
	if (last)
		*last = 0;
	else
		*out = 0;
}

/*
================
FS_NextPath

Allows enumerating all of the directories in the search path
================
*/
char *FS_NextPath (char *prevpath) {
	searchpath_t	*s;
	char			*prev;

	if (!prevpath)
		return fs_gamedir;

	prev = fs_gamedir;
	for (s = fs_searchpaths; s; s = s->next) {
		if (s->pack)
			continue;
		if (prevpath == prev)
			return s->filename;
		prev = s->filename;
	}

	return NULL;
}

extern cvar_t *net_compatibility;

static void FS_ScanForGameDLL (void) {
	FILE		*fp;
	char		*path;
#ifdef _WIN32
	static const char	*gamenames[] = { "gamex86xp.dll", "gamex86.dll" };
#else
	static const char	*gamenames[] = { "gamexp.so", "game.so" };
#endif
	int			ncv;

	// possible values: 0 (gamexp), 1 (game), 2 (auto detect)
	net_compatibility = Cvar_Get ("net_compatibility", "2", CVAR_SERVERINFO | CVAR_NOSET);
	ncv = net_compatibility->integer;

	if (ncv != 0 && ncv != 1 && ncv != 2) {
		Cvar_ForceSetValue ("net_compatibility", 2);
		ncv = net_compatibility->integer;
	}

#ifdef _WIN32
	// check the current debug directory first for development purposes
#ifdef NDEBUG
	path = "release";
#else
	path = "debug";
#endif
	if (ncv != 2) {
		// cases 0 and 1: option was specified by user
		Com_sprintf (gameDLLPath, sizeof(gameDLLPath), "%s/%s", path, gamenames[ncv]);
		fp = fopen (gameDLLPath, "rb");
	}
	else {
		// case 2: autodetect
		int i;
		for (i = 0; i < 2; i++) {
			Com_sprintf (gameDLLPath, sizeof(gameDLLPath), "%s/%s", path, gamenames[i]);
			fp = fopen (gameDLLPath, "rb");
			if (fp != NULL) {
				Cvar_ForceSetValue ("net_compatibility", i);
				break;
			}
		}
	}

	if (fp != NULL) {
		fclose (fp);
		return;
	}
#endif  // _WIN32

	/* now run through the search paths */
	path = NULL;
	while ((path = FS_NextPath (path)) != NULL) {
		if (ncv != 2) {
			Com_sprintf (gameDLLPath, sizeof(gameDLLPath), "%s/%s", path, gamenames[ncv]);
			fp = fopen (gameDLLPath, "rb");
			if (fp == NULL)
				continue;
		}
		else {
			int i;
			for (i = 0; i < 2; i++) {
				Com_sprintf (gameDLLPath, sizeof(gameDLLPath), "%s/%s", path, gamenames[i]);
				fp = fopen (gameDLLPath, "rb");
				if (fp != NULL)
					break;
			}
			if (fp == NULL)
				continue;
			else
				Cvar_ForceSetValue ("net_compatibility", i);
		}

		fclose (fp);
		return;
	}

	Com_Error (ERR_FATAL, "Could not find any game DLLs\n");
}

/*
================
FS_InitFilesystem
================
*/
void FS_InitFilesystem (void) {
	Com_Printf ("====== File System Initialization ======\n\n");
	fs_OriginalPaksOnly = Cvar_Get ("fs_OriginalPaksOnly", "1", 0);
	Cmd_AddCommand ("path", FS_Path_f);
	Cmd_AddCommand ("dir", FS_Dir_f);

	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
	fs_basedir = Cvar_Get ("basedir", ".", CVAR_NOSET);

	//
	// cddir <path>
	// Logically concatenates the cddir after the basedir for 
	// allows the game to run from outside the data tree
	//
#ifdef SYSTEMWIDE
	fs_cddir = Cvar_Get ("cddir", SYSTEMWIDE, CVAR_NOSET);
#else
	fs_cddir = Cvar_Get ("cddir", "", CVAR_NOSET);
#endif
	if (fs_cddir->string[0])
		FS_AddGameDirectory (va ("%s/"BASEDIRNAME, fs_cddir->string));

	// start up with baseq2 by default
	FS_AddGameDirectory (va ("%s/"BASEDIRNAME, fs_basedir->string));
	FS_AddHomeAsGameDirectory (BASEDIRNAME);

	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// check for game override
	fs_gamedirvar = Cvar_Get ("game", "", CVAR_LATCH | CVAR_SERVERINFO);
	if (fs_gamedirvar->string[0])
		FS_SetGamedir (fs_gamedirvar->string);

	/* Create directory if it does not exist. */
	Sys_Mkdir (fs_gamedir);

	Com_Printf ("\n");
	Com_Printf (S_COLOR_YELLOW "Using " S_COLOR_GREEN "'%s'" S_COLOR_YELLOW " for writing\n", fs_gamedir);

	FS_ScanForGameDLL ();
	Com_Printf (S_COLOR_YELLOW "Found game library at " S_COLOR_GREEN "'%s'\n", gameDLLPath);
}
