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

#include "../client/client.h"
#include "../client/input.h"
#include "../client/keys.h"
#include "../client/snd_loc.h"

/* Console variables that we need to access from this module */
cvar_t *r_gamma;
cvar_t *vid_ref;
cvar_t *r_fullScreen;
cvar_t *r_customWidth;
cvar_t *r_customHeight;

/* Global variables used internally by this module */
viddef_t	viddef;		/* global video state; used by other modules */
qboolean	reflib_active = 0;

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

/*
 * ==========================================================================
 *
 * DLL GLUE
 *
 * ==========================================================================
 */

//#define	MAXPRINTMSG	4096
void
Con_Printf(int print_level, char *fmt,...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if (print_level == PRINT_ALL)
		Com_Printf("%s", msg);
	else
		Com_DPrintf("%s", msg);
}

void
VID_Error(int err_level, char *fmt,...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	Com_Error(err_level, "%s", msg);
}

/* ========================================================================== */

/*
 * ============ VID_Restart_f
 *
 * Console command to re-start the video mode and refresh DLL. We do this simply
 * by setting the modified flag for the vid_ref variable, which will cause
 * the entire video mode and refresh DLL to be reset on the next frame.
 * ============
 */
void
VID_Restart_f(void)
{
	vid_ref->modified = qtrue;
}

/*
 * * VID_GetModeInfo
 */
typedef struct vidmode_s {
	const char     *description;
	int		width     , height;
	int		mode;
} vidmode_t;


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>

static vidmode_t vid_modes[] = {
	{ "Desktop",	-1, -1, 0 },		// desktop native
	// generic screen
	{ "1024x768",	1024, 768, 1 },     // 4:3
	{ "1152x864",	1152, 864, 2 },     // 4:3
	{ "1280x1024",	1280, 1024, 3 },    // 5:4
	{ "1600x1200",	1600, 1200, 4 },    // 4:3
	{ "2048x1536",	2048, 1536, 5 },    // 4:3
	// wide screen
	{ "1280x720",	1280, 720, 6 },     // 16:9 720p HDTV
	{ "1280x800",	1280, 800, 7 },     // 16:10
	{ "1366x768",	1366, 768, 8 },     // 16:9, plasma & LCD TV
	{ "1440x900",	1440, 900, 9 },     // 16:10
	{ "1600x900",	1600, 900, 10 },    // 16:9 TV
	{ "1680x1050",	1680, 1050, 11 },   // 16:10
	{ "1920x1080",	1920, 1080, 12 },   // 16:9 1080p full HDTV
	{ "1920x1200",	1920, 1200, 13 },   // 16:10
	{ "2560x1440",	2560, 1440, 14 },   // 16:9 WQHD
	{ "2560x1600",	2560, 1600, 15 },   // 16:10
	{ "3440x1440",  3440, 1440, 16 },   // 21:9 QHD
	{ "3840x1600",  3840, 1600, 17 },   // 12:5 Ultra Wide TV
	{ "3840x2160",  3840, 2160, 18 },   // 16:19 ULTRA HD
	{ "4096x1716",  4096, 1716, 19 },   // 2.39:1 DCI 4K WIDE
	{ "4096x2160",  4096, 2160, 20 },   // 1.89:1 DCI 4K
	{ "Custom",		-1, -1, 21 }		// custom
};

qboolean
VID_GetModeInfo(int *width, int *height, int mode)
{
	if (mode < 0 || mode >= VID_NUM_MODES)
		return qfalse;
  
  if (mode == 0) {
  Display* d = XOpenDisplay(NULL);
  Screen*  s = DefaultScreenOfDisplay(d);
  
  *width = s->width;
  *height = s->height;  
  
  } else
    if (mode == 21) {
  
        *width = r_customWidth->value;
        *height = r_customHeight->value;
  
    } else {
  
        *width = vid_modes[mode].width;
        *height = vid_modes[mode].height;
  
    }

	return qtrue;
}

/*
 * * VID_NewWindow
 */
void
VID_NewWindow(int width, int height)
{
	viddef.width = width;
	viddef.height = height;

	cl.force_refdef = qtrue;		// can't use a paused refdef
}

void
VID_FreeReflib(void)
{
	reflib_active = qfalse;
}

qboolean VID_StartRefresh()
{
	if (reflib_active) {
        R_Shutdown();
		VID_FreeReflib();
	}

	if (R_Init(0, 0) == -1)
	{
		R_Shutdown();
		VID_FreeReflib();
		return qfalse;
	}

	Key_ClearStates();
	reflib_active = qtrue;

	return qtrue;
}


/*
 * ============ VID_CheckChanges
 *
 * This function gets called once just before drawing each frame, and it's sole
 * purpose in life is to check to see if any of the video mode parameters
 * have changed, and if they have to update the rendering DLL and/or video
 * mode to match. ============
 */
void
VID_CheckChanges(void)
{
	while (vid_ref->modified) {
		/*
         * * refresh has changed
         */

        cl.force_refdef = qtrue;
        S_StopAllSounds();

        vid_ref->modified = qfalse;
		r_fullScreen->modified = qtrue;
		cl.refresh_prepped = qfalse;
		cls.disable_screen = qtrue;
        CL_ClearDecals();

		if (!VID_StartRefresh()) {
			Com_Error(ERR_FATAL, "Couldn't start refresh");

			// drop the console if we fail to load a refresh
			if (cls.key_dest != key_console) 
				Con_ToggleConsole_f();
		}
		cls.disable_screen = qfalse;
		CL_InitImages();

		// XXX: as SDL was restarted, key repeat settings were lost
		IN_Shutdown();
		IN_Init();
	}
}

/*
 * ============ VID_Init ============
 */
void
VID_Init(void)
{

	/* Create the video variables so we know how to start the graphics drivers */
	vid_ref = Cvar_Get ("vid_ref", "xpgl", CVAR_ARCHIVE);
	r_gamma = Cvar_Get("r_gamma", "1.0", CVAR_ARCHIVE);
	r_fullScreen = Cvar_Get ("r_fullScreen", "1", CVAR_ARCHIVE);
	r_customWidth = Cvar_Get ("r_customWidth", "1024", CVAR_ARCHIVE);
	r_customHeight = Cvar_Get ("r_customHeight", "768", CVAR_ARCHIVE);

	/* Add some console commands that we want to handle */
	Cmd_AddCommand("vid_restart", VID_Restart_f);

	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges();
}

/*
 * ============ VID_Shutdown ============
 */
void
VID_Shutdown(void)
{
	if (reflib_active) {
        R_Shutdown();
		VID_FreeReflib();
	}
}
