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

//
// these are the key numbers that should be passed to Key_Event
//
//
// these are the key numbers that should be passed to Key_Event
//
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_KP_HOME		160
#define K_KP_UPARROW	161
#define K_KP_PGUP		162
#define	K_KP_LEFTARROW	163
#define K_KP_5			164
#define K_KP_RIGHTARROW	165
#define K_KP_END		166
#define K_KP_DOWNARROW	167
#define K_KP_PGDN		168
#define	K_KP_ENTER		169
#define K_KP_INS   		170
#define	K_KP_DEL		171
#define K_KP_SLASH		172
#define K_KP_MINUS		173
#define K_KP_PLUS		174

//
// mouse buttons generate virtual keys
//
#define	K_MOUSE1				200
#define	K_MOUSE2				201
#define	K_MOUSE3				202
#define	K_MOUSE4				203
#define	K_MOUSE5				204

// xBox controller buttons
#define	K_XPAD_START			205
#define	K_XPAD_BACK				206	
#define	K_XPAD_LEFT_THUMB		207
#define	K_XPAD_RIGHT_THUMB		208
#define	K_XPAD_LEFT_SHOULDER	209
#define	K_XPAD_RIGHT_SHOULDER	210
#define	K_XPAD_A				211
#define	K_XPAD_B				212
#define	K_XPAD_X				213
#define	K_XPAD_Y				214
#define	K_XPAD_DPAD_UP			215
#define	K_XPAD_DPAD_DOWN		216
#define	K_XPAD_DPAD_LEFT		217
#define	K_XPAD_DPAD_RIGHT		218
#define	K_XPAD_LEFT_TRIGGER		219
#define	K_XPAD_RIGHT_TRIGGER	220

#define	K_MWHEELDOWN			221
#define	K_MWHEELUP				222

#define K_PAUSE					255

extern char *keybindings[256];
extern int key_repeats[256];

extern int anykeydown;
extern char chat_buffer[];
extern int chat_bufferlen;
extern qboolean chat_team;

void Key_Event (int key, qboolean down, unsigned time);
void Key_Init (void);
void Key_WriteBindings (FILE * f);
void Key_SetBinding (int keynum, char *binding);
void Key_ClearStates (void);
int Key_GetKey (void);
