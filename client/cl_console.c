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
// console.c

#include "client.h"

#include "../ref_gl/r_local.h"


color4ub_t	ColorTable[8] = {
	{ 0, 0, 0, 255 },
	{ 255, 0, 0, 255 },
	{ 0, 255, 0, 255 },
	{ 255, 255, 0, 255 },
	{ 0, 0, 255, 255 },
	{ 0, 255, 255, 255 },
	{ 255, 0, 255, 255 },
	{ 255, 255, 255, 255 },
};


color4ub_t		colorDefault;

void RE_SetColor (const color4ub_t color) {


	if (color[0] != colorDefault[0] ||
		color[1] != colorDefault[1] ||
		color[2] != colorDefault[2] ||
		color[3] != colorDefault[3]) {

		gl_state.fontColor[0] = color[0] / 255.0;
		gl_state.fontColor[1] = color[1] / 255.0;
		gl_state.fontColor[2] = color[2] / 255.0;
		gl_state.fontColor[3] = color[3] / 255.0;

		colorDefault[0] = color[0];
		colorDefault[1] = color[1];
		colorDefault[2] = color[2];
		colorDefault[3] = color[3];
	}
}

color4ub_t	colorBlack = { 0, 0, 0, 255 };
color4ub_t	colorRed = { 255, 0, 0, 255 };
color4ub_t	colorGreen = { 0, 255, 0, 255 };
color4ub_t	colorYellow = { 255, 255, 0, 255 };
color4ub_t	colorBlue = { 0, 0, 255, 255 };
color4ub_t	colorCyan = { 0, 255, 255, 255 };
color4ub_t	colorMagenta = { 255, 0, 255, 255 };
color4ub_t	colorWhite = { 255, 255, 255, 255 };

color4ub_t	colorLtGray = { 192, 192, 192, 255 };
color4ub_t	colorMdGray = { 128, 128, 128, 255 };
color4ub_t	colorDkGray = { 64, 64, 64, 255 };
color4ub_t	colorGold = { 255, 192, 64, 255 };

console_t con;

cvar_t *con_notifytime;


#define		MAXCMDLINE	256
extern char key_lines[32][MAXCMDLINE];
extern int edit_line;
extern int key_linepos;

void Key_ClearTyping (void) {
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;
}

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void) {
	SCR_EndLoadingPlaque ();		// get rid of loading plaque

	if (cl.attractloop) {
		Cbuf_AddText ("killserver\n");
		return;
	}

	if (cls.state == ca_disconnected) {	// start the demo loop again
		Cbuf_AddText ("d1\n");
		return;
	}

	Key_ClearTyping ();
	Con_ClearNotify ();

	if (cls.key_dest == key_console) {
		M_ForceMenuOff ();
		Cvar_Set ("paused", "0");
	}
	else {
		M_ForceMenuOff ();
		cls.key_dest = key_console;

		if (Cvar_VariableInteger ("maxclients") == 1 && Com_ServerState ())
			Cvar_Set ("paused", "1");
	}
}

/*
================
Con_ToggleChat_f
================
*/
void Con_ToggleChat_f (void) {
	Key_ClearTyping ();

	if (cls.key_dest == key_console) {
		if (cls.state == ca_active) {
			M_ForceMenuOff ();
			cls.key_dest = key_game;
		}
	}
	else
		cls.key_dest = key_console;

	Con_ClearNotify ();
}

/*
================
Con_Clear_f
================
*/

void Con_Clear_f (void) {
	int i;

	for (i = 0; i < CON_TEXTSIZE; i++)
		con.text[i] = (ColorIndex (COLOR_WHITE) << 8) | ' ';

	con.current = con.totalLines - 1;
	con.display = con.current;


}


/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f (void) {
	int l, x;
	short *line;
	FILE *f;
	char buffer[1024];
	char name[MAX_OSPATH];

	if (Cmd_Argc () != 2) {
		Com_Printf (S_COLOR_YELLOW"usage: condump <filename>\n");
		return;
	}

	Com_sprintf (name, sizeof(name), "%s/%s.txt", FS_Gamedir (),
		Cmd_Argv (1));

	Com_Printf ("Dumped console text to "S_COLOR_GREEN"%s\n", name);
	FS_CreatePath (name);
	f = fopen (name, "w");
	if (!f) {
		Com_Printf (S_COLOR_RED"ERROR: couldn't open.\n");
		return;
	}
	// skip empty lines
	for (l = con.current - con.totalLines + 1; l <= con.current; l++) {
		line = con.text + (l % con.totalLines)*con.lineWidth;
		for (x = 0; x < con.lineWidth; x++) {
			if ((line[x] & 0xff) != ' ')
				break;
		}

		if (x != con.lineWidth)
			break;
	}

	// write the remaining lines
	buffer[con.lineWidth] = 0;
	for (; l <= con.current; l++) {
		line = con.text + (l % con.totalLines)*con.lineWidth;
		for (x = 0; x < con.lineWidth; x++)
			buffer[x] = line[x] & 0xff;

		for (x = con.lineWidth - 1; x >= 0; x--) {
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}



		fprintf (f, "%s\n", buffer);
	}

	fclose (f);
}


/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void) {
	int i;

	for (i = 0; i < NUM_CON_TIMES; i++)
		con.times[i] = 0;
}


/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f (void) {
	chat_team = qfalse;
	cls.key_dest = key_message;
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void) {
	chat_team = qtrue;
	cls.key_dest = key_message;
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/

#define	CON_DEFAULT_WIDTH		78

void Con_CheckResize (void) {
	int		i, j, width, oldwidth, oldtotalLines, numlines, numchars;
	short	tbuf[CON_TEXTSIZE];

	width = (640 >> 3) - 2;

	if (width == con.lineWidth)
		return;

	if (width < 1) {			// video hasn't been initialized yet
		width = CON_DEFAULT_WIDTH;
		con.lineWidth = width;
		con.totalLines = CON_TEXTSIZE / con.lineWidth;

		for (i = 0; i<CON_TEXTSIZE; i++)
			con.text[i] = (ColorIndex (COLOR_WHITE) << 8) | ' ';
	}
	else {
		oldwidth = con.lineWidth;
		con.lineWidth = width;
		oldtotalLines = con.totalLines;
		con.totalLines = CON_TEXTSIZE / con.lineWidth;
		numlines = oldtotalLines;

		if (numlines > con.totalLines)
			numlines = con.totalLines;

		numchars = oldwidth;

		if (numchars > con.lineWidth)
			numchars = con.lineWidth;

		Q_memcpy (tbuf, con.text, CON_TEXTSIZE * sizeof(short));

		for (i = 0; i < CON_TEXTSIZE; i++)
			con.text[i] = (ColorIndex (COLOR_WHITE) << 8) | ' ';

		for (i = 0; i < numlines; i++) {
			for (j = 0; j < numchars; j++) {
				con.text[(con.totalLines - 1 - i) * con.lineWidth + j] =
					tbuf[((con.current - i + oldtotalLines) %
					oldtotalLines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con.current = con.totalLines - 1;
	con.display = con.current;
}


/*
================
Con_Init
================
*/
void Con_Init (void) {
	con.lineWidth = -1;

	Con_CheckResize ();

	Com_Printf (S_COLOR_YELLOW"===== Quake2xp Console initialized =====\n\n");

	//
	// register our commands
	//
	con_notifytime = Cvar_Get ("con_notifytime", "3", 0);

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);
	Cmd_AddCommand("cls", Con_Clear_f);
	Cmd_AddCommand ("condump", Con_Dump_f);


	con.initialized = qtrue;
}


/*
===============
Con_Linefeed
===============
*/

static void Con_Linefeed (qboolean skipNotify) {
	int		i;

	// mark time for transparent overlay
	if (con.current >= 0)
		con.times[con.current % NUM_CON_TIMES] = skipNotify ? 0 : cls.realtime;

	con.x = 0;
	if (con.display == con.current)
		con.display++;

	con.current++;

	for (i = 0; i < con.lineWidth; i++)
		con.text[(con.current % con.totalLines)*con.lineWidth + i] = (ColorIndex (COLOR_WHITE) << 8) | ' ';
}


/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/

void Con_Print (char *txt) {
	int		y;
	int		c, l;
	int		color;
	//	static qboolean	cr;
	qboolean	skipNotify = qfalse;

	if (!Q_strnicmp (txt, "[skipnotify]", 12)) {
		skipNotify = qtrue;
		txt += 12;
	}

	if (!con.initialized) {
		con.lineWidth = -1;
		Con_CheckResize ();
		con.initialized = qtrue;
	}

	color = ColorIndex (COLOR_WHITE);

	while ((c = *txt)) {
		if (IsColorString (txt)) {
			color = ColorIndex (*(txt + 1));
			txt += 2;
			continue;
		}

		// count word length
		for (l = 0; l< con.lineWidth; l++) {
			if (txt[l] <= ' ')
				break;
		}

		// word wrap
		if (l != con.lineWidth && (con.x + l > con.lineWidth))
			Con_Linefeed (skipNotify);

		txt++;

		switch (c) {
			case '\n':
				Con_Linefeed (skipNotify);
				break;
			case '\r':
				con.x = 0;
				break;
			default:	// display character and advance
				y = con.current % con.totalLines;
				con.text[y*con.lineWidth + con.x] = (color << 8) | c;
				con.x++;
				if (con.x >= con.lineWidth)
					Con_Linefeed (skipNotify);
				break;
		}

	}
}

/*
==============
Con_CenteredPrint
==============
*/
void Con_CenteredPrint (char *text) {
	int l;
	char buffer[1024];

	l = strlen (text);
	l = (con.lineWidth - l) * 0.5;
	if (l < 0)
		l = 0;
	memset (buffer, ' ', l);
	strcpy (buffer + l, text);
	strcat (buffer, "\n");
	Con_Print (buffer);
}

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/

void Con_DrawInput (void) {
	char	*text;
	int		i;
	float	fontscale = cl_fontScale->value;
	float	intervalScale = 0.75;

	if (cls.key_dest == key_menu)
		return;

	if (cls.key_dest != key_console && cls.state == ca_active)
		return;					// don't draw anything (always draw if not active)

	text = key_lines[edit_line];

	// add the cursor frame
	text[key_linepos] = 10 + ((int)(cls.realtime >> 8) & 1);

	// fill out remainder with spaces
	for (i = key_linepos + 1; i < con.lineWidth; i++)
		text[i] = ' ';

	// prestep if horizontally scrolling
	if (key_linepos >= con.lineWidth)
		text += 1 + key_linepos - con.lineWidth;

	// draw it
	RE_SetColor (colorWhite);

	for (i = 0; i < con.lineWidth; i++)
		//	Draw_Char ((i + 1) << 3, con.vislines - 15, text[i]);
		Draw_CharScaled ((i*fontscale + 1) * (8 * intervalScale), con.vislines - 15 * fontscale, fontscale, fontscale, text[i]);

	// remove cursor
	key_lines[edit_line][key_linepos] = 0;
}



/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/

void Con_DrawNotify (void) {
	int		x, v;
	short	*text;
	int		i;
	int		time;
	char	*s;
	int		skip;
	int		currentColor;
	float	intervalScale = 0.75;
	float	fontscale = cl_fontScale->value;

	currentColor = 7;
	RE_SetColor (ColorTable[currentColor]);

	Set_FontShader (qtrue);

	v = 0;

	for (i = con.current - NUM_CON_TIMES + 1; i <= con.current; i++) {
		
		if (i < 0)
			continue;

		time = con.times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;

		time = cls.realtime - time;
		if (time > con_notifytime->value * 1000)
			continue;

		text = con.text + (i % con.totalLines)*con.lineWidth;

		for (x = 0; x < con.lineWidth; x++) {
			if ((text[x] & 0xff) == ' ')
				continue;

			if (((text[x] >> 8) & 7) != currentColor) {
				currentColor = (text[x] >> 8) & 7;
				RE_SetColor (ColorTable[currentColor]);
			}
			Draw_CharScaled ((x*fontscale + 1) * (8 * intervalScale), v, fontscale, fontscale, text[x] & 0xff);
		}

		v += 8 * fontscale;
	}

	RE_SetColor (colorYellow);

	if (cls.key_dest == key_message) {
		if (chat_team) {
			Draw_StringScaled (8 * fontscale, v, fontscale, fontscale, "say_team:");
			skip = 11;
		}
		else {
			Draw_StringScaled (8 * fontscale, v, fontscale, fontscale, "say:");
			skip = 5;
		}

		s = chat_buffer;

		if (chat_bufferlen > ((viddef.width / fontscale) / 8) - (skip + 1))
			s += chat_bufferlen - (int)(((viddef.width / fontscale) / 8) - (skip + 1));
	
		Draw_StringScaled (skip*fontscale * 8, v, fontscale, fontscale, s);
		Draw_CharScaled ((strlen (s) + skip) * fontscale * 8, v, fontscale, fontscale, 10 + ((cls.realtime >> 8) & 1));

		v += 8;
	}
	Set_FontShader	(qfalse);
	RE_SetColor		(colorWhite);
}



/*
================
Con_DrawConsole

Draws the console with the solid background
================
*/
void Con_DrawConsole (float frac) {
	int			i, j, x, y, n;
	int			rows;
	short		*text;
	int			row;
	int			lines;
	char		version[64];
	char		dlbar[1024];
	int			currentColor;
	float		intervalScale = 0.75;
	float		fontscale = cl_fontScale->value;

	if (frac == 1.0)
		lines = viddef.height * frac;
	else
		lines = viddef.height * frac - 8 * fontscale; // download bar fix

	if (lines <= 0)
		return;

	if (lines > viddef.height)
		lines = viddef.height;

	// draw the background
	Draw_StretchPic2 (0, lines - viddef.height, viddef.width, viddef.height, i_conback);
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width - 1, lines - 1);

	Set_FontShader (qtrue);

	Com_sprintf (version, sizeof(version), "q2xp 1.26.9 (%s)", __DATE__);
	for (x = 0; x < strlen (version); x++)
		version[x] += 128;
	int len = strlen(version);

	Draw_StringScaled (viddef.width - len * 6 * fontscale, lines - 12 * fontscale, fontscale, fontscale, version);

	// draw the text
	con.vislines = lines;

#if 0
	rows = (lines - 8) >> 3;	// rows of text to draw

	y = lines - 24;
#else
	rows = (lines - 22 * fontscale) / 8;	// rows of text to draw

	y = lines - 30 * fontscale;
#endif

	// draw from the bottom up
	if (con.display != con.current) {
		// draw arrows to show the buffer is backscrolled
		RE_SetColor (colorCyan);
		for (x = 0; x < con.lineWidth; x += 4)
			Draw_CharScaled ((x*fontscale + 1) * 8, y, fontscale, fontscale, '^');

		RE_SetColor (colorWhite);
		y -= 8 * fontscale;
		rows--;
	}

	currentColor = 7;
	RE_SetColor (ColorTable[currentColor]);

	row = con.display;
	for (i = 0; i < rows; i++, y -= 8 * fontscale, row--) {
		if (row < 0)
			break;
		if (con.current - row >= con.totalLines)
			break;				// past scrollback wrap point

		text = con.text + (row % con.totalLines) * con.lineWidth;


		for (x = 0; x < con.lineWidth; x++) {
			if ((text[x] & 0xFF) == ' ')
				continue;

			if (((text[x] >> 8) & 7) != currentColor) {
				currentColor = (text[x] >> 8) & 7;
				RE_SetColor (ColorTable[currentColor]);
			}

			//Reset Current font color
			RE_SetColor (ColorTable[currentColor]);
			Draw_CharScaled ((x*fontscale + 1) * (8 * intervalScale), y, fontscale, fontscale, text[x] & 0xFF);
		}
	}

	//ZOID
	// draw the download bar
	// figure out width
	if (cls.download) {
		// avoid warnings of using a short* instead of char* in strrchr/strlen
		char *textch = (char*)text;

		if ((textch = strrchr (cls.downloadname, '/')) != NULL)
			textch++;
		else
			textch = cls.downloadname;

		x = con.lineWidth - ((con.lineWidth * 7) * 0.025);
		y = x - strlen (textch) - 8;
		i = con.lineWidth * 0.3333333333;
		if (strlen (textch) > i) {
			y = x - i - 11;
			strncpy (dlbar, textch, i);
			dlbar[i] = 0;
			strcat (dlbar, "...");
		}
		else
			strcpy (dlbar, textch);
		strcat (dlbar, ": ");
		i = strlen (dlbar);
		dlbar[i++] = '\x80';
		// where's the dot go?
		if (cls.downloadpercent == 0)
			n = 0;
		else
			n = y * cls.downloadpercent / 100;

		for (j = 0; j < y; j++)
		if (j == n)
			dlbar[i++] = '\x83';
		else
			dlbar[i++] = '\x81';
		dlbar[i++] = '\x82';
		dlbar[i] = 0;

		sprintf (dlbar + strlen (dlbar), " %02d%%", cls.downloadpercent);

		// draw it
		y = con.vislines - 12;
		for (i = 0; i < strlen (dlbar); i++)
			//	Draw_Char((i + 1) << 3, y, dlbar[i]);
			Draw_CharScaled ((i*fontscale + 1) * 8, y, fontscale, fontscale, dlbar[i]);
	}
	//ZOID

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput ();

	RE_SetColor (colorWhite);

	Set_FontShader (qfalse);

}

