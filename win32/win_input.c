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
// win_input.c -- windows xp raw mouse code

#include "../client/client.h"
#include "winquake.h"

extern	unsigned	sys_msg_time;

cvar_t	*m_inversion;
cvar_t	*v_centermove;
cvar_t	*v_centerspeed;
qboolean	in_appactive;


/*
============================================================

MOUSE CONTROL

============================================================
*/

qboolean	mlooking;

void IN_MLookDown (void) {
	mlooking = qtrue;
}

void IN_MLookUp (void) {
	mlooking = qfalse;
	if (!freelook->value && lookspring->value)
		IN_CenterView ();
}

int			mouse_buttons;
int			mouse_oldbuttonstate;
POINT		current_pos;
int			mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;

int			old_x, old_y;

qboolean	mouseactive;	// qfalse when not focus app

qboolean	restore_spi;
qboolean	mouseinitialized;
qboolean	mouseparmsvalid;

int			originalmouseparms[3], newmouseparms[3] = { 0, 0, 1 };
int			window_center_x, window_center_y;
RECT		window_rect;


/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse (void) {
	int		width, height;

	if (mouseactive)
		return;

	/// Berserker's fix
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(cl_hwnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width - 1;
	if (window_rect.bottom >= height - 1)
		window_rect.bottom = height - 1;

	window_center_x = (window_rect.right + window_rect.left) * 0.5;
	window_center_y = (window_rect.top + window_rect.bottom) * 0.5;

	old_x = window_center_x;
	old_y = window_center_y;

	SetCursorPos(window_center_x, window_center_y);
	SetCapture(cl_hwnd);
	ClipCursor(&window_rect);

	mouseactive = qtrue;
	while (ShowCursor(FALSE) >= 0);
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse (void) {
	
	if (!mouseactive)
		return;

	ClipCursor (NULL);
	ReleaseCapture ();

	mouseactive = qfalse;
	
	while (ShowCursor(TRUE) < 0);
}



/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void) {
	cvar_t		*cv;

	cv = Cvar_Get ("in_initmouse", "1", CVAR_NOSET);

	if (!cv->integer)
		return;

	mouseinitialized = qtrue;
	mouseparmsvalid = SystemParametersInfo (SPI_GETMOUSE, 0, originalmouseparms, 0);
	mouse_buttons = 5;
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate) {
	int		i;

	if (!mouseinitialized)
		return;

	// perform button actions
	for (i = 0; i < mouse_buttons; i++) {
		if ((mstate & (1 << i)) &&
			!(mouse_oldbuttonstate & (1 << i))) {
			Key_Event (K_MOUSE1 + i, qtrue, sys_msg_time);
		}

		if (!(mstate & (1 << i)) &&
			(mouse_oldbuttonstate & (1 << i))) {
			Key_Event (K_MOUSE1 + i, qfalse, sys_msg_time);
		}
	}

	mouse_oldbuttonstate = mstate;
}


/*
===========
IN_MouseMove
===========
*/

qboolean FindRawDevices()
{
	PRAWINPUTDEVICELIST g_pRawInputDeviceList;
	UINT nDevices;

	Com_Printf("====== Init RAW Input Devices ======\n\n");

	// Get Number of devices attached
	if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
	{
		Com_Printf("\n" S_COLOR_RED "No RawInput devices attached\n");
		return qfalse;
	}
	else
		Com_Printf("" S_COLOR_YELLOW "... Found " S_COLOR_GREEN "%i" S_COLOR_YELLOW " RAW input devices.\n", nDevices);

	// Create list large enough to hold all RAWINPUTDEVICE structs
	if ((g_pRawInputDeviceList = (PRAWINPUTDEVICELIST)Z_Malloc(sizeof(RAWINPUTDEVICELIST) * nDevices)) == NULL)
	{
		Com_Printf("" S_COLOR_RED "Error mallocing RAWINPUTDEVICELIST\n");
		return qfalse;
	}
	// Now get the data on the attached devices
	if (GetRawInputDeviceList(g_pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST)) == -1)
	{
		Com_Printf("" S_COLOR_RED "1Error from GetRawInputDeviceList\n");
		Z_Free(g_pRawInputDeviceList);
		return qfalse;
	}

	PRAWINPUTDEVICE g_pRawInputDevices = (PRAWINPUTDEVICE)Z_Malloc(nDevices * sizeof(RAWINPUTDEVICE));

	for (UINT i = 0; i<nDevices; i++)
	{
		if (g_pRawInputDeviceList[i].dwType == RIM_TYPEMOUSE)
		{
			UINT nchars = 300;
			TCHAR deviceName[300];
			if (GetRawInputDeviceInfo(g_pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, deviceName, &nchars) >= 0)
				Com_DPrintf("Device[%d]:\n handle=0x%x\n name = %s\n\n", i, g_pRawInputDeviceList[i].hDevice, deviceName);
			RID_DEVICE_INFO dinfo;
			UINT sizeofdinfo = sizeof(dinfo);
			dinfo.cbSize = sizeofdinfo;
			if (GetRawInputDeviceInfo(g_pRawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &dinfo, &sizeofdinfo) >= 0)
			{
				if (dinfo.dwType == RIM_TYPEMOUSE)
				{
					RID_DEVICE_INFO_MOUSE *pMouseInfo = &dinfo.mouse;
					Com_DPrintf("ID = 0x%x\n", pMouseInfo->dwId);
					Com_DPrintf("Number of buttons = %i\n", pMouseInfo->dwNumberOfButtons);
					Com_DPrintf("Sample Rate = %i\n", pMouseInfo->dwSampleRate);
					Com_DPrintf("Has Horizontal Wheel: %s\n", pMouseInfo->fHasHorizontalWheel ? "Yes" : "No");
				}
			}
		}
	}
	Z_Free(g_pRawInputDevices);
	Z_Free(g_pRawInputDeviceList);

	Com_Printf("\n------------------------------------\n");
	
	return qtrue;
}

/*
===========
IN_Init
===========
*/
void IN_Init (void) {
	// mouse variables
	m_inversion = Cvar_Get ("m_inversion", "0", CVAR_ARCHIVE);

	// centering
	v_centermove = Cvar_Get ("v_centermove", "0.15", 0);
	v_centerspeed = Cvar_Get ("v_centerspeed", "500", 0);

	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

	FindRawDevices();
	IN_StartupXInput();
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void) {
	IN_DeactivateMouse ();
	IN_ShutDownXinput();
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active) {
	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
extern int bind_grab;

void IN_Frame (void) {

	if (!in_appactive)
	{
		IN_DeactivateMouse();
		return;
	}

	/*
	if (!cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu) {
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableInteger ("r_fullScreen") == 0) {
			IN_DeactivateMouse ();
			return;
		}
	}
	*/
	IN_ActivateMouse ();
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd) {
	
	IN_ToggleXInput();

	if (ActiveApp) {
		if (xInputActive) {
			IN_ControllerMove(cmd);
		}
	}
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void) {
	mx_accum = 0;
	my_accum = 0;
	mouse_oldbuttonstate = 0;
}