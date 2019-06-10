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
// winquake.h: Win32-specific Quake header file

#include <windows.h>

extern	HINSTANCE	global_hInstance;

extern HWND			cl_hwnd;
extern qboolean		ActiveApp, Minimized;

void IN_Activate(qboolean active);
void IN_MouseEvent(int mstate);

extern int		window_center_x, window_center_y;
extern RECT		window_rect;

cvar_t	*in_useXInput;
cvar_t	*x360_useControllerID;
cvar_t	*x360_sensX;
cvar_t	*x360_sensY;
cvar_t	*x360_pitchInversion;
cvar_t	*x360_swapSticks;
cvar_t	*x360_triggerTreshold;
cvar_t	*x360_deadZone;
cvar_t	*x360_vibration;

extern qboolean xInputActive;

void	IN_StartupXInput(void);
void	IN_ToggleXInput();
void	IN_ControllerMove(usercmd_t *cmd);
void	IN_ShutDownXinput();

extern qboolean	mlooking;