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
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>

#include "../qcommon/qcommon.h"

unsigned	sys_frame_time;
qboolean	stdin_active = qtrue;
uid_t		saved_euid;

/* ======================================================================= */
/* General routines */
/* ======================================================================= */

// Converts Quake II color escape codes to ANSI terminal
static inline void PrintWithColors(const char *s) {
	static int convTable[] = {
		0, 1, 2, 3, 4, 6, 5, 7
	};
	int i = 0;

	while (s[i] != '\0') {
		if (s[i] == Q_COLOR_ESCAPE) {
			i++;
			if (s[i] == Q_COLOR_ESCAPE) {
				putchar(Q_COLOR_ESCAPE);
				i++;
			} else if (ColorIndex(s[i]) < 8) {
				// TODO: use cvar to control color (on/bold/off); is it possible here?
				fputs("\033[1m", stdout);	// bold font
				printf("\033[%dm", 30 + convTable[ColorIndex(s[i])]);
				i++;
			}
		} else {
			putchar(s[i]);
			i++;
		}
	}
	fputs("\033[0m", stdout);	// reset terminal colors
}

void
Sys_ConsoleOutput(char *msg)
{
	char msg_mono[MAXPRINTMSG];

	if (isatty(STDOUT_FILENO))
		PrintWithColors(msg);
	else {
		Com_DecolorizeStr(msg_mono, msg);
		fputs(msg_mono, stdout);
	}
}

static char *meminfoAwk =
  "/^MemTotal/  { print \"^7Total physical RAM:       ^2\", $2/1024, \" ^7MB\"} "
  "/^MemFree/   { print \"^7Available physical RAM:   ^2\", $2/1024, \" ^7MB\"} "
  "/^SwapFree/  { print \"^7Total Virtual Memory:     ^2\", $2/1024, \" ^7MB\"} "
  "/^SwapFree/  { print \"^7Available virtual memory: ^2\", $2/1024, \" ^7MB\"} "
  ;

static char *cpuinfoAwk =
  "/^processor\\t/  { if ($3 != 0) exit }"
  "/^vendor_id\\t/  { printf \"^7CPU vendor: ^2\"; for (i=3;i<=NF;i++) printf(\"%s \", $i); print \"\" } "
  "/^model name\\t/ { printf \"^7CPU model:  ^2\"; for (i=4;i<=NF;i++) printf(\"%s \", $i); print \"\" } "
  "/^cpu cores\\t/  { printf \"^7CPU cores:  ^2%d\\n\", $4} "
  "/^flags\\t/      { printf \"^7CPU flags:  ^2\"; for (i=3;i<=NF;i++) if ($i ~ /sse|mmx|3d/) printf(\"%s \", $i); print \"\" } "
  ;

void Sys_PrintInfo(void) {
    FILE *info;
    char buffer[4096];
    char cmd[4096];
    
	Com_Printf ("========"S_COLOR_YELLOW"System Information"S_COLOR_WHITE"========\n\n");
    
#ifdef __linux__
    // CPU
    Com_sprintf(cmd, sizeof(cmd), "awk '%s' /proc/cpuinfo", cpuinfoAwk);
    if ((info = popen(cmd, "r")) != NULL) {
        int sz = fread(&buffer, 1, sizeof(buffer), info);
        buffer[sz] = '\0';
        Com_Printf("%s\n", buffer);
        fclose(info);
    }
    
	// RAM
    Com_sprintf(cmd, sizeof(cmd), "awk '%s' /proc/meminfo", meminfoAwk);
    if ((info = popen(cmd, "r")) != NULL) {
        int sz = fread(&buffer, 1, sizeof(buffer), info);
        buffer[sz] = '\0';
        Com_Printf("%s\n", buffer);
        fclose(info);
    }
#endif
    
    // FIXME: separate platform, kernel version, etc
    if ((info = popen("uname -a", "r")) != NULL) {
        fgets(buffer, sizeof(buffer), info);
        Com_Printf(S_COLOR_WHITE "OS info: " S_COLOR_YELLOW "%s\n", buffer);
        //Com_Printf (PRINT_ALL, "\n");
        pclose(info);
    } else
        Com_Printf(PRINT_ALL, "uname failed\n");
    
    if ((info = popen("whoami", "r")) != NULL) {
        fgets(buffer, sizeof(buffer), info);
        Com_Printf(S_COLOR_WHITE "User name: " S_COLOR_YELLOW "%s\n", buffer);
        pclose(info);
    }
    
	Com_Printf ("==================================\n\n");
}


void
Sys_Printf(char *fmt,...)
{
	va_list		argptr;
	char		text[1024];
	unsigned char  *p;

	va_start(argptr, fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}

void
Sys_Quit(void)
{
	CL_Shutdown();
	Qcommon_Shutdown();
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~FNDELAY);
	exit(0);
}

extern cvar_t *net_compatibility;

void
Sys_Init(void)
{
	Sys_PrintInfo();
}

void
Sys_Error(char *error,...)
{
	va_list		argptr;
	char		string[1024];

	/* change stdin to non blocking */
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~FNDELAY);

	CL_Shutdown();
	Qcommon_Shutdown();

	va_start(argptr, error);
	vsnprintf(string, sizeof(string), error, argptr);
	va_end(argptr);
	fprintf(stderr, "Error: %s\n", string);

	exit(1);

}

void
Sys_Warn(char *warning,...)
{
	va_list		argptr;
	char		string[1024];

	va_start(argptr, warning);
	vsnprintf(string, sizeof(string), warning, argptr);
	va_end(argptr);
	fprintf(stderr, "Warning: %s", string);
}

/*
 * ============ Sys_FileTime
 *
 * returns -1 if not present ============
 */
int
Sys_FileTime(char *path)
{
	struct stat	buf;

	if (stat(path, &buf) == -1)
		return -1;

	return buf.st_mtime;
}

void
floating_point_exception_handler(int whatever)
{
	signal(SIGFPE, floating_point_exception_handler);
}

char           *
Sys_ConsoleInput(void)
{
	static char	text[256];
	int		len;
	fd_set		fdset;
	struct timeval	timeout;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (!stdin_active)
		return NULL;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset);	/* stdin */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select(1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read(0, text, sizeof(text));
	if (len == 0) {		/* eof! */
		stdin_active = qfalse;
		return NULL;
	}
	if (len < 1)
		return NULL;
	text[len - 1] = 0;	/* rip off the /n and terminate */

	return text;
}

/*****************************************************************************/

static void    *game_library;

/*
 * ================= Sys_UnloadGame =================
 */
void
Sys_UnloadGame(void)
{
	if (game_library)
		dlclose(game_library);
	game_library = NULL;
}

extern char gameDLLPath[];

/*
 * ==============
 * Sys_GetGameAPI
 * ==============
 *
 * Loads the game dll
 */
void           *
Sys_GetGameAPI(void *parms)
{
	void		*(*GetGameAPI) (void *);

	if (game_library)
		Com_Error(ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	setreuid(getuid(), getuid());
	setegid(getgid());

	Com_Printf("Loading %s\n\n", gameDLLPath);
	game_library = dlopen(gameDLLPath, RTLD_LAZY);
	
	if (game_library)
		Com_DPrintf("LoadLibrary (%s)\n", gameDLLPath);
	else {
		char *str_p, *path;
		
		Com_Printf("LoadLibrary (%s):", gameDLLPath);

		path = (char *)dlerror();
		str_p = strchr(path, ':');	/* skip the path (already shown) */
		if (str_p == NULL)
			str_p = path;
		else
			str_p++;

		Com_Printf("%s\n", str_p);

		return NULL;
	}

	GetGameAPI = (void *)dlsym(game_library, "GetGameAPI");
	if (!GetGameAPI) {
		Sys_UnloadGame();
		return NULL;
	}

	return GetGameAPI(parms);
}

/*****************************************************************************/

void
Sys_AppActivate(void)
{
}

void IN_Update(void);
void Sys_SendKeyEvents(void)
{
    IN_Update();

	/* grab frame time  */
	sys_frame_time = Sys_Milliseconds();
}

char *Sys_GetClipboardData(void) {
    return NULL;
}

/*****************************************************************************/


int
main(int argc, char **argv)
{
	int	mytime, oldtime, newtime;
	
	/* go back to real user for config loads */
	saved_euid = geteuid();
	seteuid( getuid() );

	Qcommon_Init(argc, argv);
	oldtime = Sys_Milliseconds();

	while (1) {
		/* find time spent rendering last frame */
		do {
			newtime = Sys_Milliseconds();
			mytime = newtime - oldtime;
		} while (mytime < 1);

		Qcommon_Frame(mytime);

		oldtime = newtime;
	}
    // never gets here
    return 0;
}
