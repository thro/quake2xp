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
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

/*

  full screen console
  put up loading plaque
  blanked background with loading plaque
  blanked background with menu
  cinematics
  full screen image for quit and victory

  end of unit intermissions

  */

#include "client.h"

float scr_con_current;			// aproaches scr_conlines at scr_conspeed
float scr_conlines;				// 0.0 to 1.0 lines of console to display

qboolean scr_initialized;		// ready to draw

int scr_draw_loading;

vrect_t scr_vrect;				// position of render window on screen


cvar_t *scr_viewsize;
cvar_t *scr_conspeed;
cvar_t *scr_centertime;
cvar_t *scr_showturtle;
cvar_t *scr_showpause;
cvar_t *scr_printspeed;

cvar_t *scr_netgraph;
cvar_t *scr_timegraph;
cvar_t *scr_graphheight;
cvar_t *scr_graphscale;
cvar_t *scr_graphshift;
cvar_t *scr_drawall;

extern cvar_t *cl_drawTime;	// JKnife -- HUD Clock

typedef struct {
	int x1, y1, x2, y2;
} dirty_t;

dirty_t scr_dirty, scr_old_dirty[2];

char crosshair_pic[MAX_QPATH];
int crosshair_width, crosshair_height;

void SCR_TimeRefresh_f (void);
void SCR_Loading_f (void);
/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char scr_centerstring[1024];
float scr_centertime_start;		// for slow victory printing
float scr_centertime_off;
int scr_center_lines;
int scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (char *str) {
	char *s;
	char line[64];
	int i, j, l;

	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime->value;
	scr_centertime_start = cl.time;

	// count the number of lines for centering
	scr_center_lines = 1;
	s = str;
	while (*s) {
		if (*s == '\n')
			scr_center_lines++;
		s++;
	}

	// echo it to the console
	Com_Printf
		("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	s = str;
	do {
		// scan the width of the line
		for (l = 0; l < 40; l++)
		if (s[l] == '\n' || !s[l])
			break;
		for (i = 0; i < (40 - l) * 0.5; i++)
			line[i] = ' ';

		for (j = 0; j < l; j++) {
			line[i++] = s[j];
		}

		line[i] = '\n';
		line[i + 1] = 0;

		Com_Printf ("%s", line);

		while (*s && *s != '\n')
			s++;

		if (!*s)
			break;
		s++;					// skip the \n
	} while (1);
	Com_Printf
		("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_ClearNotify ();
}


void SCR_DrawCenterString (void) {
	char *start;
	int l;
	int j;
	int x, y;
	int remaining;

	// the finale prints the characters one at a time
	remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = viddef.height * 0.35;
	else
		y = 48;

	Set_FontShader (qtrue);

	do {
		// scan the width of the line
		for (l = 0; l < 40; l++)
		if (start[l] == '\n' || !start[l])
			break;
		x = (viddef.width - l * 6 * cl_fontScale->value) * 0.5;
		SCR_AddDirtyPoint (x, y);

		for (j = 0; j < l; j++, x += 6 * cl_fontScale->value) {
			Draw_CharScaled (x, y, cl_fontScale->value, cl_fontScale->value, start[j]);
			if (!remaining--)
				return;
		}
		SCR_AddDirtyPoint (x, y + 6 * cl_fontScale->value);

		y += 8 * cl_fontScale->value;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;				// skip the \n
	} while (1);

	Set_FontShader (qfalse);
}

void SCR_CheckDrawCenterString (void) {
	scr_centertime_off -= cls.frametime;

	if (scr_centertime_off <= 0)
		return;

	SCR_DrawCenterString ();
}

//=============================================================================

/*
=================
SCR_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
static void SCR_CalcVrect (void) {
	int size;

	// bound viewsize
	if (scr_viewsize->value < 40)
		Cvar_Set ("viewsize", "40");
	if (scr_viewsize->value > 100)
		Cvar_Set ("viewsize", "100");

	size = scr_viewsize->value;

	scr_vrect.width = viddef.width * size / 100;
	scr_vrect.width &= ~7;

	scr_vrect.height = viddef.height * size / 100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (viddef.width - scr_vrect.width) * 0.5;
	scr_vrect.y = (viddef.height - scr_vrect.height) * 0.5;
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void) {
	Cvar_SetValue ("viewsize", scr_viewsize->value + 10);
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void) {
	Cvar_SetValue ("viewsize", scr_viewsize->value - 10);
}

/*
=================
SCR_Sky_f

Set a specific sky and rotation speed
=================
*/
void SCR_Sky_f (void) {
	float rotate;
	vec3_t axis;

	if (Cmd_Argc () < 2) {
		Com_Printf ("Usage: sky <basename> <rotate> <axis x y z>\n");
		return;
	}
	if (Cmd_Argc () > 2)
		rotate = atof (Cmd_Argv (2));
	else
		rotate = 0;
	if (Cmd_Argc () == 6) {
		axis[0] = atof (Cmd_Argv (3));
		axis[1] = atof (Cmd_Argv (4));
		axis[2] = atof (Cmd_Argv (5));
	}
	else {
		axis[0] = 0;
		axis[1] = 0;
		axis[2] = 1;
	}

	R_SetSky (Cmd_Argv (1), rotate, axis);
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void) {
	scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);
	scr_conspeed = Cvar_Get ("scr_conspeed", "3", 0);
	scr_showturtle = Cvar_Get ("scr_showturtle", "0", 0);
	scr_showpause = Cvar_Get ("scr_showpause", "1", 0);
	scr_centertime = Cvar_Get ("scr_centertime", "2.5", 0);
	scr_printspeed = Cvar_Get ("scr_printspeed", "8", 0);
	scr_netgraph = Cvar_Get ("netgraph", "0", 0);
	scr_timegraph = Cvar_Get ("timegraph", "0", 0);
	scr_graphheight = Cvar_Get ("graphheight", "32", 0);
	scr_graphscale = Cvar_Get ("graphscale", "1", 0);
	scr_graphshift = Cvar_Get ("graphshift", "0", 0);
	scr_drawall = Cvar_Get ("scr_drawall", "0", 0);

	//
	// register our commands
	//
	Cmd_AddCommand ("timerefresh", SCR_TimeRefresh_f);
	Cmd_AddCommand ("loading", SCR_Loading_f);
	Cmd_AddCommand ("sizeup", SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown", SCR_SizeDown_f);
	Cmd_AddCommand ("sky", SCR_Sky_f);

	scr_initialized = qtrue;
}


/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void) {
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged
		< CMD_BACKUP - 1)
		return;

	Draw_Pic2 (scr_vrect.x + 64, scr_vrect.y, i_net);
}

/*
==============
SCR_DrawPause
==============
*/
void SCR_DrawPause (void) {

	if (!scr_showpause->integer)	// turn off for screenshots
		return;

	if (!cl_paused->integer)
		return;
	
	if (cls.menuActive)
		return;

	Draw_ScaledPic((viddef.width - (i_pause->width - i_pause->width * 0.25)) * 0.5f,
					viddef.height * 0.5f - i_pause->height * 0.5,
					cl_fontScale->value, cl_fontScale->value,
					i_pause);

	Draw_PicBumpScaled((viddef.width - (i_pause->width - i_pause->width * 0.25)) * 0.5f,
						viddef.height * 0.5f - i_pause->height * 0.5,
						cl_fontScale->value, cl_fontScale->value,
						"pause", "pause_bump");
}

/*
==============
SCR_DrawLoading
==============
*/

void SCR_DrawLoadingBar (float percent, float scale) {

	Draw_Fill (2, viddef.height - scale * 10 + 3, viddef.width * percent * 0.01, scale * 3 - 6, 0.0, 1.0, 0.0, 0.13);

}

void Draw_LoadingScreen (int x, int y, int w, int h, char *pic);

void SCR_DrawLoading (void) {
	int		scaled, center;
	char	mapfile[32];
	char	*mapname;
	int		fontscale = (int)cl_fontScale->value;

	if (!scr_draw_loading)
		return;

	scr_draw_loading = 0;

	if (loadingMessage && cl.configstrings[CS_MODELS + 1][0]) {
		
		strcpy (mapfile, cl.configstrings[CS_MODELS + 1] + 5);	// skip "maps/"
		mapfile[strlen (mapfile) - 4] = 0;	// cut off ".bsp"
		
		if (Draw_FindPic(va("/levelshots/%s.jpg", mapfile)))
			Draw_LoadingScreen(0, 0, viddef.width, viddef.height, va("/levelshots/%s.jpg", mapfile));
		else
			Draw_LoadingScreen(0, 0, viddef.width, viddef.height, "/gfx/defshot.jpg");

		scaled = 8 * fontscale;
		SCR_DrawLoadingBar (loadingPercent, scaled);

		mapname = cl.configstrings[CS_NAME];

		Set_FontShader (qtrue);
		
		center = viddef.width / 2 - (int)strlen(mapname) * fontscale * 6;
		RE_SetColor(colorGreen);
		Draw_StringScaled (center, 20 * fontscale, fontscale * 2, fontscale * 2, mapname);
		
		RE_SetColor (colorYellow);
		Draw_StringScaled (0, 44 * fontscale, fontscale, fontscale,
			va ("%s", loadingMessages[0]));
		Draw_StringScaled (0, 54 * fontscale, fontscale, fontscale,
			va ("%s", loadingMessages[1]));
		Draw_StringScaled (0, 64 * fontscale, fontscale, fontscale,
			va ("%s", loadingMessages[2]));
		Draw_StringScaled (0, 74 * fontscale, fontscale, fontscale,
			va ("%s", loadingMessages[3]));
		RE_SetColor (colorWhite);
		Set_FontShader (qfalse);
	}
}



//=============================================================================

/*
==================
SCR_RunConsole

Scroll it up or down
==================
*/
void SCR_RunConsole (void) {
	// decide on the height of the console
	if (cls.key_dest == key_console)
		scr_conlines = 0.5;		// half screen
	else
		scr_conlines = 0;		// none visible

	if (scr_conlines < scr_con_current) {
		scr_con_current -= scr_conspeed->value * cls.frametime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current) {
		scr_con_current += scr_conspeed->value * cls.frametime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void) {
	Con_CheckResize ();

	if (cls.state == ca_disconnected || cls.state == ca_connecting) {	// forced
		// full
		// screen
		// console
		Con_DrawConsole (1.0);
		return;
	}

	if (cls.state != ca_active || !cl.refresh_prepped) {	// connected,
		// but can't
		// render
		Con_DrawConsole (0.5);
		Draw_Fill (0, viddef.height * 0.5f, viddef.width,
			viddef.height * 0.5f, 0.0, 0.0, 0.0, 1.0);
		return;
	}

	if (scr_con_current) {
		Con_DrawConsole (scr_con_current);
	}
	else {
		if (cls.key_dest == key_game || cls.key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/

qboolean needLoadingPlaque (void) {
	if (!cls.disable_screen || !scr_draw_loading)
		return qtrue;
	return qfalse;
}


void SCR_BeginLoadingPlaque (void) {
	
	S_StopAllSounds ();
	
	cl.sound_prepped = qfalse;	// don't play ambients
	
	Music_Stop ();
	
	if (cls.disable_screen)
		return;

	if (developer->integer)
		return;

	if (cls.state == ca_disconnected)
		return;					// if at console, don't bring up the
	// plaque
	if (cls.key_dest == key_console)
		return;
	if (cl.cinematictime > 0)
		scr_draw_loading = 2;	// clear to balack first
	else
		scr_draw_loading = 1;

	SCR_UpdateScreen ();
	
	cls.disable_screen = Sys_Milliseconds ();
	
	cls.disable_servercount = cl.servercount;
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque (void) {
	cls.disable_screen = 0;
	scr_draw_loading = 0;
	Con_ClearNotify ();
}

/*
================
SCR_Loading_f
================
*/
void SCR_Loading_f (void) {
	SCR_BeginLoadingPlaque ();
}

/*
================
SCR_TimeRefresh_f
================
*/
int entitycmpfnc (const entity_t * a, const entity_t * b) {
	/*
	 ** all other models are sorted by model then skin
	 */
	if (a->model == b->model) {
		return ((int)a->skin - (int)b->skin);
	}
	else {
		return ((int)a->model - (int)b->model);
	}
}

void SCR_TimeRefresh_f (void) {
	int i;
	int start, stop;
	float time;

	if (cls.state != ca_active)
		return;

	start = Sys_Milliseconds ();

	if (Cmd_Argc () == 2) {		// run without page flipping
		R_BeginFrame ();
		for (i = 0; i < 128; i++) {
			cl.refdef.viewangles[1] = i / 128.0 * 360.0;
			R_RenderFrame (&cl.refdef);
		}
		GLimp_EndFrame ();
	}
	else {
		for (i = 0; i < 128; i++) {
			cl.refdef.viewangles[1] = i / 128.0 * 360.0;

			R_BeginFrame ();
			R_RenderFrame (&cl.refdef);
			GLimp_EndFrame ();
		}
	}

	stop = Sys_Milliseconds ();
	time = (stop - start) / 1000.0;
	Com_Printf ("%f seconds ("S_COLOR_YELLOW"%f"S_COLOR_WHITE" fps)\n", time, 128 / time);
}

/*
=================
SCR_AddDirtyPoint
=================
*/
void SCR_AddDirtyPoint (int x, int y) {
	if (x < scr_dirty.x1)
		scr_dirty.x1 = x;
	if (x > scr_dirty.x2)
		scr_dirty.x2 = x;
	if (y < scr_dirty.y1)
		scr_dirty.y1 = y;
	if (y > scr_dirty.y2)
		scr_dirty.y2 = y;
}

void SCR_DirtyScreen (void) {
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width - 1, viddef.height - 1);
}

/*
==============
SCR_TileClear

Clear any parts of the tiled background that were drawn on last frame
==============
*/
void SCR_TileClear (void) {
	int i;
	int top, bottom, left, right;
	dirty_t clear;

	if (scr_drawall->integer)
		SCR_DirtyScreen ();		// for power vr or broken page flippers...

	if (scr_con_current == 1.0)
		return;					// full screen console
	if (scr_viewsize->value == 100)
		return;					// full screen rendering
	if (cl.cinematictime > 0)
		return;					// full screen cinematic

	// erase rect will be the union of the past three frames
	// so tripple buffering works properly
	clear = scr_dirty;
	for (i = 0; i < 2; i++) {
		if (scr_old_dirty[i].x1 < clear.x1)
			clear.x1 = scr_old_dirty[i].x1;
		if (scr_old_dirty[i].x2 > clear.x2)
			clear.x2 = scr_old_dirty[i].x2;
		if (scr_old_dirty[i].y1 < clear.y1)
			clear.y1 = scr_old_dirty[i].y1;
		if (scr_old_dirty[i].y2 > clear.y2)
			clear.y2 = scr_old_dirty[i].y2;
	}

	scr_old_dirty[1] = scr_old_dirty[0];
	scr_old_dirty[0] = scr_dirty;

	scr_dirty.x1 = 9999;
	scr_dirty.x2 = -9999;
	scr_dirty.y1 = 9999;
	scr_dirty.y2 = -9999;

	// don't bother with anything convered by the console)
	top = scr_con_current * viddef.height;
	if (top >= clear.y1)
		clear.y1 = top;

	if (clear.y2 <= clear.y1)
		return;					// nothing disturbed

	top = scr_vrect.y;
	bottom = top + scr_vrect.height - 1;
	left = scr_vrect.x;
	right = left + scr_vrect.width - 1;

	if (clear.y1 < top) {		// clear above view screen
		i = clear.y2 < top - 1 ? clear.y2 : top - 1;
		Draw_TileClear2 (clear.x1, clear.y1,
			clear.x2 - clear.x1 + 1, i - clear.y1 + 1,
			i_backtile);
		clear.y1 = top;
	}
	if (clear.y2 > bottom) {	// clear below view screen
		i = clear.y1 > bottom + 1 ? clear.y1 : bottom + 1;
		Draw_TileClear2 (clear.x1, i,
			clear.x2 - clear.x1 + 1, clear.y2 - i + 1,
			i_backtile);
		clear.y2 = bottom;
	}
	if (clear.x1 < left) {		// clear left of view screen
		i = clear.x2 < left - 1 ? clear.x2 : left - 1;
		Draw_TileClear2 (clear.x1, clear.y1,
			i - clear.x1 + 1, clear.y2 - clear.y1 + 1,
			i_backtile);
		clear.x1 = left;
	}
	if (clear.x2 > right) {		// clear left of view screen
		i = clear.x1 > right + 1 ? clear.x1 : right + 1;
		Draw_TileClear2 (i, clear.y1,
			clear.x2 - i + 1, clear.y2 - clear.y1 + 1,
			i_backtile);
		clear.x2 = right;
	}

}


//===============================================================


#define STAT_MINUS		10		// num frame for '-' stats digit
char *sb_nums[2][11] = {
	{ "num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
	"num_6", "num_7", "num_8", "num_9", "num_minus" },
	{ "anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5",
	"anum_6", "anum_7", "anum_8", "anum_9", "anum_minus" }
};

char *sb_nums_bump[11] = {	"num_0_bump", "num_1_bump", "num_2_bump", "num_3_bump", "num_4_bump", "num_5_bump",
							"num_6_bump", "num_7_bump", "num_8_bump", "num_9_bump", "num_minus_bump" };


#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	CHAR_WIDTH	16
#define	ICON_SPACE	8



/*
================
SizeHUDString

Allow embedded \n in the string
================
*/
void SizeHUDString (char *string, int *w, int *h) {
	int lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while (*string) {
		if (*string == '\n') {
			lines++;
			current = 0;
		}
		else {
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

	*w = width * 8;
	*h = lines * 8;
}


void DrawHUDString (float x, float y, float scale_x, float scale_y, int centerwidth, int xor, char *string) {
	float	margin;
	char	line[1024];
	int		width;
	int		i;

	margin = x;

	Set_FontShader (qtrue);

	while (*string) {
		// scan out one line of text from the string
		width = 0;
		while (*string && *string != '\n')
			line[width++] = *string++;
		line[width] = 0;

		if (centerwidth)
			x = margin + (centerwidth - width * 8)*scale_x / 2;
		else
			x = margin;

		for (i = 0; i < width; i++) {
			Draw_CharScaled (x, y, scale_x, scale_y, line[i] ^ xor);
			x += 8 * scale_x;
		}
		if (*string) {
			string++;	// skip the \n
			x = margin;
			y += 8 * scale_y;
		}
	}
	Set_FontShader (qfalse);
}

/*
==============
SCR_DrawField
==============
*/
void SCR_DrawField (int x, int y, float scale_x, float scale_y, int color, int width, int value) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;
	
	// draw number string
	if (width > 5)
		width = 5;

	SCR_AddDirtyPoint (x, y);
	SCR_AddDirtyPoint (x + (width*CHAR_WIDTH + 2)*scale_x, y + 23 * scale_y);

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen (num);
	if (l > width)
		l = width;
	x += (2 + CHAR_WIDTH*(width - l))*scale_x;

	ptr = num;

	while (*ptr && l) {
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';

		Draw_PicScaled (x, y, scale_x, scale_y, sb_nums[color][frame]);
		Draw_PicBumpScaled(x, y, scale_x, scale_y, sb_nums[color][frame], sb_nums_bump[frame]);
		
		x += CHAR_WIDTH*scale_x;
		ptr++;
		l--;
	}
}



/*
===============
SCR_TouchPics

Allows rendering code to cache all needed sbar graphics
===============
*/

void SCR_TouchPics (void) {
	int i, j;

	for (i = 0; i < 2; i++)
	for (j = 0; j < 11; j++)
		Draw_FindPic (sb_nums[i][j]);
	
	for (j = 0; j < 11; j++)
		Draw_FindPic(sb_nums_bump[j]);

	if (crosshair->integer) {
		if (crosshair->integer > 13 || crosshair->integer < 0)
			crosshair->integer = 13;

		Com_sprintf (crosshair_pic, sizeof(crosshair_pic), "chxp%i", (int)(crosshair->value));
		Draw_GetPicSize (&crosshair_width, &crosshair_height,
			crosshair_pic);

		if (!crosshair_width)
			crosshair_pic[0] = 0;

	}
}
//=======================================================

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
extern cvar_t *r_lightEditor;

extern cvar_t *r_speeds;
void SCR_DrawStats ();
void SCR_DrawLayout (void);

int c_brush_polys,
c_alias_polys,
c_flares,
c_visible_lightmaps,
c_visible_textures,
c_flares,
c_shadow_volumes,
c_decals,
c_shadow_tris,
c_part_tris,
c_decal_tris,
c_light_batch;

extern cvar_t *cl_drawFPS;
extern cvar_t *cl_hudScale;

void SCR_DrawSpeeds (void) {

	char	bsp[18], alias[18], st[18], partTris[18], shadow[18], decals[18], dtr[18], lt[18];
	float	fontscale = cl_fontScale->value;

	if (!r_speeds->integer)
		return;

	sprintf (bsp, "%i w_tris", c_brush_polys);
	sprintf (alias, "%i m_tris", c_alias_polys);
	sprintf (st, "%i s_tris", c_shadow_tris);
	sprintf (partTris, "%i p_tris", c_part_tris);
	sprintf (shadow, "%i shadows", c_shadow_volumes);
	sprintf (decals, "%i decals", c_decals);
	sprintf (dtr, "%i d_tris", c_decal_tris);
	sprintf(lt, "%i vis lights", num_visLights);

	Set_FontShader (qtrue);
	RE_SetColor (colorCyan);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4, fontscale, fontscale, bsp);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4 + 10 * fontscale, fontscale, fontscale, alias);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4 + 20 * fontscale, fontscale, fontscale, st);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4 + 30 * fontscale, fontscale, fontscale, partTris);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4 + 40 * fontscale, fontscale, fontscale, shadow);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4 + 50 * fontscale, fontscale, fontscale, dtr);
	Draw_StringScaled (viddef.width - 95 * fontscale, viddef.height*0.4 + 60 * fontscale, fontscale, fontscale, lt);
	RE_SetColor (colorWhite);
	Set_FontShader (qfalse);
}

void SCR_DrawFPS (void) {
	static	char	avrfps[10], minfps[22];
	static	int		fps = 0;
	static	int		lastUpdate;
	const	int		samPerSec = 4;
	static	float	fpsAvg = 0;
	float	fontscale = cl_fontScale->value;

	fps++;

	if (curtime - lastUpdate >= 1000 / samPerSec) {

		const float alpha = 0.45;

		if (cl.minFps == 0) // only one time per level
			cl.minFps = 999999;

		if (fpsAvg < cl.minFps)
			cl.minFps = fpsAvg;

		if (fpsAvg > cl.maxFps)
			cl.maxFps = fpsAvg;

			Com_sprintf(minfps, sizeof(minfps), "min/max %3d/%3d fps", cl.minFps, cl.maxFps);
			Com_sprintf(avrfps, sizeof(avrfps), "%4d fps", (int)fpsAvg);

		lastUpdate = curtime;
		fpsAvg = samPerSec * (alpha * fps + ((1 - alpha) * fpsAvg) / samPerSec);
		fps = 0;
	}

	int avrFpsLengh = (int)strlen(avrfps);
	int minFpsLengh = (int)strlen(minfps);
	
	if (cl_drawFPS->integer && (cls.state == ca_active)) {
		
		Set_FontShader (qtrue);

		if (cl_drawFPS->integer == 2) {
			Draw_StringScaled(viddef.width - avrFpsLengh * 6 * fontscale, viddef.height * 0.65 - 40, fontscale, fontscale, avrfps);
			Draw_StringScaled(viddef.width - minFpsLengh * 6 * fontscale, viddef.height * 0.65 - 20, fontscale, fontscale, minfps);

		} else
			Draw_StringScaled(viddef.width - avrFpsLengh * 6 * fontscale, viddef.height * 0.65, fontscale, fontscale, avrfps);

		RE_SetColor (colorWhite);
		Set_FontShader (qfalse);
	}
}

void SCR_DrawClock (void) {
	char	timebuf[20];
	char	tmpbuf[24];
	char	datebuf[20];
	char	tmpdatebuf[24];
	float	fontscale = cl_fontScale->value;

#ifndef _WIN32
	struct tm *tm;
	time_t aclock;

	time (&aclock);
	tm = localtime (&aclock);
	strftime (timebuf, sizeof(timebuf), "%T", tm);
	strftime (datebuf, sizeof(datebuf), "%D", tm);
#else
	_strtime (timebuf);
	_strdate (datebuf);
#endif

	sprintf (tmpbuf, "Time %s", timebuf);
	sprintf (tmpdatebuf, "Date %s", datebuf);

	int timebufLengh = strlen(tmpbuf);
	int datebufLengh = strlen(tmpdatebuf);

	Set_FontShader (qtrue);


	if (!cl_drawFPS->integer) {
		Draw_StringScaled (viddef.width - timebufLengh * 6 * fontscale, viddef.height*0.65, fontscale, fontscale, tmpbuf);
		Draw_StringScaled (viddef.width - datebufLengh * 6 * fontscale, viddef.height*0.65 + 10 * fontscale, fontscale, fontscale, tmpdatebuf);
	}
	else {
		Draw_StringScaled (viddef.width - timebufLengh * 6 * fontscale, viddef.height*0.65 + 10 * fontscale, fontscale, fontscale, tmpbuf);
		Draw_StringScaled (viddef.width - datebufLengh * 6 * fontscale, viddef.height*0.65 + 20 * fontscale, fontscale, fontscale, tmpdatebuf);
	}
	Set_FontShader (qfalse);

}

void SCR_ShowTexNames() {

	trace_t		trace;
	vec3_t		end, forward, right, up;

	if (!scr_showTexName->integer)
		return;
	
	if (cls.state != ca_active)
		return;

	AngleVectors(cl.refdef.viewangles, forward, right, up);
	VectorMA(cl.refdef.vieworg, 4096, forward, end);
	trace = CL_PMTraceWorld(cl.refdef.vieworg, vec3_origin, vec3_origin, end, (MASK_SOLID | MASK_WATER), qfalse);
	
	Set_FontShader(qtrue);
	RE_SetColor(colorGreen);

	if (trace.surface->name[0])
	{
		char	string[MAX_QPATH];
		Com_sprintf(string, sizeof(string), "Surface texture: %s", trace.surface->name);
		Draw_StringScaled(0, viddef.height / 2 - 50, 2.0, 2.0, string);
	} 

	Set_FontShader(qfalse);
	RE_SetColor(colorWhite);
}

void R_GammaRamp (void);
float ClampCvar(float min, float max, float value);
extern cvar_t *r_mode;

void SCR_UpdateScreen (void) {
	// if the screen is disabled (loading plaque is up, or vid mode
	// changing)
	// do nothing at all
	if (cls.disable_screen) {
		if (cls.download)		// Knightmare- don't time out on downloads
			cls.disable_screen = Sys_Milliseconds ();
		if (Sys_Milliseconds () - cls.disable_screen > 120000 && cl.refresh_prepped && !(cl.cinematictime > 0)) {
			cls.disable_screen = 0;
			Com_Printf ("Loading plaque timed out.\n");
			return;
		}
		scr_draw_loading = 2;
	}

	if (!scr_initialized || !con.initialized)
		return;					// not initialized yet


	cl_hudScale->value = ClampCvar(0.1, 1.0, cl_hudScale->value);
	cl_fontScale->value = ClampCvar(2.0, 3.0, cl_fontScale->value);

	if(viddef.height <= 1024)
		Cvar_Set("cl_fontScale", "2");
	else
		Cvar_Set("cl_fontScale", "3");

	R_BeginFrame ();

	if (scr_draw_loading == 2) {	// loading plaque over black
		// screen
		R_SetPalette (NULL);
		SCR_DrawLoading ();

		if (cls.disable_screen)
			scr_draw_loading = 2;

		// NO FULLSCREEN CONSOLE!!!
		goto next;
	}
	// if a cinematic is supposed to be running, handle menus
	// and console specially
	else if (cl.cinematictime > 0) {
		if (cls.key_dest == key_menu) {
			if (cl.cinematicpalette_active) {
				R_SetPalette (NULL);
				cl.cinematicpalette_active = qfalse;
			}
			M_Draw ();
		}
		else
			SCR_DrawCinematic ();
	}
	else {


		// make sure the game palette is active
		if (cl.cinematicpalette_active) {
			R_SetPalette (NULL);
			cl.cinematicpalette_active = qfalse;
		}
	next:

		// do 3D refresh drawing, and then update the screen
		SCR_CalcVrect ();

		// clear any dirty part of the background
		SCR_TileClear ();

		V_RenderView ();
		
		SCR_DrawSpeeds();

		SCR_DrawStats ();
		if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 1)
			SCR_DrawLayout ();
		if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 2)
			CL_DrawInventory ();

		SCR_DrawNet ();
		SCR_CheckDrawCenterString ();

		SCR_DrawPause ();

		SCR_DrawFPS ();
		SCR_ShowTexNames();

		if (cl_drawTime->value && (cls.state == ca_active))
			SCR_DrawClock ();

		SCR_DrawConsole ();

		M_Draw ();

		SCR_DrawLoading ();

	}

	R_GammaRamp ();
	GLimp_EndFrame ();
}
