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
// Main windowed and fullscreen graphics interface module. This module
// is used for both the software and OpenGL rendering versions of the
// Quake refresh engine.
//#include <assert.h>
#include <float.h>

#include "..\client\client.h"
#include "winquake.h"
#include "../client/snd_loc.h"			//for experimental OpenAL suspend feature.
//#include "zmouse.h"

#include "xinput.h"

cvar_t *win_noalttab;

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported by the OS
#endif

static UINT MSH_MOUSEWHEEL;

// Console variables that we need to access from this module
cvar_t		*r_brightness;
cvar_t		*vid_ref;			// Name of Refresh DLL loaded
cvar_t		*vid_xpos;			// X coordinate of window position
cvar_t		*vid_ypos;			// Y coordinate of window position
cvar_t		*r_fullScreen;
cvar_t		*r_customWidth;
cvar_t		*r_customHeight;



// Global variables used internally by this module
viddef_t	viddef;				// global video state; used by other modules
//extern viddef_t	vid;				// global video state; used by other modules

qboolean	reflib_active = 0;

HWND        cl_hwnd;            // Main window handle for life of program

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static qboolean s_alttab_disabled;

extern	unsigned	sys_msg_time;

/*
** WIN32 helper functions
*/
extern qboolean s_win95;

static void WIN_DisableAltTab (void) {
	BOOL old;

	if (s_alttab_disabled)
		return;

	SystemParametersInfo (SPI_SCREENSAVERRUNNING, 1, &old, 0);
	
	s_alttab_disabled = qtrue;
}

static void WIN_EnableAltTab (void) {
	if (s_alttab_disabled) {
			BOOL old;
			SystemParametersInfo (SPI_SCREENSAVERRUNNING, 0, &old, 0);
			s_alttab_disabled = qfalse;
	}
}

/*
==========================================================================

DLL GLUE

==========================================================================
*/

//#define	MAXPRINTMSG	4096
void Con_Printf (int print_level, char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if (print_level == PRINT_ALL) {
		Com_Printf ("%s", msg);
	}
	else if (print_level == PRINT_DEVELOPER) {
		Com_DPrintf ("%s", msg);
	}
	else if (print_level == PRINT_ALERT) {
		MessageBox (0, msg, "PRINT_ALERT", MB_ICONWARNING);
		OutputDebugString (msg);
	}

}

void VID_Error (int err_level, char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Com_Error (err_level, "%s", msg);
}

//==========================================================================

byte        scantokey[128] =
{
	//  0           1       2       3       4       5       6       7
	//  8           9       A       B       C       D       E       F
	0, 27, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', K_BACKSPACE, 9, // 0
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', 13, K_CTRL, 'a', 's',      // 1
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', K_SHIFT, '\\', 'z', 'x', 'c', 'v',      // 2
	'b', 'n', 'm', ',', '.', '/', K_SHIFT, '*',
	K_ALT, ' ', 0, K_F1, K_F2, K_F3, K_F4, K_F5,   // 3
	K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE, 0, K_HOME,
	K_UPARROW, K_PGUP, K_KP_MINUS, K_LEFTARROW, K_KP_5, K_RIGHTARROW, K_KP_PLUS, K_END, //4
	K_DOWNARROW, K_PGDN, K_INS, K_DEL, 0, 0, 0, K_F11,
	K_F12, 0, 0, 0, 0, 0, 0, 0,        // 5
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,        // 6
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0         // 7
};

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key) {
	int result;
	int modified = (key >> 16) & 255;
	qboolean is_extended = qfalse;

	if (modified > 127)
		return 0;

	if (key & (1 << 24))
		is_extended = qtrue;

	result = scantokey[modified];

	if (!is_extended) {
		switch (result) {
			case K_HOME:
				return K_KP_HOME;
			case K_UPARROW:
				return K_KP_UPARROW;
			case K_PGUP:
				return K_KP_PGUP;
			case K_LEFTARROW:
				return K_KP_LEFTARROW;
			case K_RIGHTARROW:
				return K_KP_RIGHTARROW;
			case K_END:
				return K_KP_END;
			case K_DOWNARROW:
				return K_KP_DOWNARROW;
			case K_PGDN:
				return K_KP_PGDN;
			case K_INS:
				return K_KP_INS;
			case K_DEL:
				return K_KP_DEL;
			default:
				return result;
		}
	}
	else {
		switch (result) {
			case 0x0D:
				return K_KP_ENTER;
			case 0x2F:
				return K_KP_SLASH;
			case 0xAF:
				return K_KP_PLUS;
		}
		return result;
	}
}

void AppActivate (BOOL fActive, BOOL minimize) {
	extern alConfig_t alConfig;

	Minimized = minimize;

	Key_ClearStates ();

	// we don't want to act like we're active if we're minimized
	// minimize/restore mouse-capture on demand
	if (!fActive || Minimized) {
		ActiveApp = qfalse;
		IN_Activate (qfalse);
		Music_Pause ();
		if (win_noalttab->integer) {
			WIN_EnableAltTab ();
		}
		if (alConfig.hALC) alcSuspendContext (alConfig.hALC); //willow: Have no success??
	}
	else {
		ActiveApp = qtrue;
		IN_Activate (qtrue);
		Music_Resume ();
		if (win_noalttab->integer) {
			WIN_DisableAltTab ();
		}
		if (alConfig.hALC) alcProcessContext (alConfig.hALC); //willow: Have no success??
	}
}

#ifndef MK_XBUTTON3
# define MK_XBUTTON3         0x0080
# define MK_XBUTTON4         0x0100
#endif

#ifndef MK_XBUTTON5
# define MK_XBUTTON5         0x0200
#endif

/// Added by Willow: new mouse support
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC	((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE	((USHORT) 0x02)
#endif
///#define mouse_buttons	5

LONG CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern cvar_t	*m_inversion;

extern qboolean mouseactive;

/*
====================
MainWndProc

main window procedure
====================
*/
LONG WINAPI MainWndProc(HWND    hWnd, UINT    uMsg, WPARAM  wParam, LPARAM  lParam)
{
//willow:
//ГЛАВНАЯ идея оптимизации обработки сообщений это упорядочить сообщения по интенсинсивности их появления
//и критичности к времени обработки. Первым в данном списке идёт мышь, до 200 (PS/2) или 500 (USB) сообщений в секунду.
//Профессиональные игровые мыши рапортуют до 1000 сообщений в секунду.
	if (mouseactive)
	{
		if (uMsg == WM_INPUT)
			{
				UINT dwSize = 40;
				static BYTE lpb[40];

				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != -1)
				{
					PRAWINPUT raw = (RAWINPUT*)lpb;

					if (raw->header.dwType == RIM_TYPEMOUSE)
					{
						if (raw->data.mouse.lLastX || raw->data.mouse.lLastY)
						{
							//willow:
							//Делать поддержку какого-либо искусственного ускорения баллистической траектории (по примеру m_accel)
							//мыши я не рекомендую. Подобные технологии признаны безперспективными всеми профессиональными игроками.
							///Berserker: коррекция чуствительности от FOV
							///уменьшим чуствительность в 10 раз (чтоб примерно соответствовало чуствительности старой мыши)
							///float sens = sensitivity->value * cl.refdef.fov_x / 90.0f * 0.1f;
							//willow: оптимизируем эту формулу. Корректирующий фактор поправлен c 0.1f на 0.111111f * 0.66f,
							//что в общем-то не имеет никакого принципиального значения. Иначе у меня слайдер настройки едва
							//за границу не вышел. Заодно получаем оптимизацию - гарантированная замена деления на умножение.
							float sens = sensitivity->value * cl.refdef.fov_x * 0.00066f;

							if (raw->data.mouse.lLastX)
								cl.viewangles_YAW -= sens * raw->data.mouse.lLastX;

							if (raw->data.mouse.lLastY)
							{
								if (m_inversion->integer)
									cl.viewangles_PITCH -= sens * raw->data.mouse.lLastY;
								else
									cl.viewangles_PITCH += sens * raw->data.mouse.lLastY;
							}

							//willow, Berserker:
							//Хак курсор не убегал за пределы окна
							//willow: P.S. всё-же только для оконного режима
							if (!(r_fullScreen && r_fullScreen->value))
								SetCursorPos(window_center_x, window_center_y);
						}

						// perform button actions
						if (raw->data.mouse.usButtonFlags)
						{
							//RI_MOUSE_LEFT_BUTTON
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) Key_Event(K_MOUSE1, qtrue, sys_msg_time);
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) Key_Event(K_MOUSE1, qfalse, sys_msg_time);
							//RI_MOUSE_RIGHT_BUTTON
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) Key_Event(K_MOUSE2, qtrue, sys_msg_time);
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) Key_Event(K_MOUSE2, qfalse, sys_msg_time);
							//RI_MOUSE_MIDDLE_BUTTON
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) Key_Event(K_MOUSE3, qtrue, sys_msg_time);
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) Key_Event(K_MOUSE3, qfalse, sys_msg_time);
							//RI_MOUSE_BUTTON_4
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) Key_Event(K_MOUSE4, qtrue, sys_msg_time);
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) Key_Event(K_MOUSE4, qfalse, sys_msg_time);
							//RI_MOUSE_BUTTON_5
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) Key_Event(K_MOUSE5, qtrue, sys_msg_time);
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) Key_Event(K_MOUSE5, qfalse, sys_msg_time);
							//RI_MOUSE_WHEEL
							if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
							{
								int i = (short)raw->data.mouse.usButtonData; //wheel delta, signed
								if (i < 0)
								{
									Key_Event(K_MWHEELDOWN, qtrue, sys_msg_time);
									Key_Event(K_MWHEELDOWN, qfalse, sys_msg_time);
								}
								else if (i > 0)
								{
									Key_Event(K_MWHEELUP, qtrue, sys_msg_time);
									Key_Event(K_MWHEELUP, qfalse, sys_msg_time);
								}
							}
						}
						return 0;
						//willow: DefWindowProc? DefRawInputProc? то и другое?
					}
					else
					{
						//willow:
						//Мы обнаружили неизвестное устройство - сообщим в компетентные органы по вопросам RAW INPUT.
						//Ставить ли в известность DefWindowProc?
						return DefRawInputProc(&raw, 1, sizeof(RAWINPUTHEADER)) ? 1 : 0;
					}
				}
				//willow:
				//Поправьте меня, но если мы не смогли прочитать WM_INPUT сообщение, то никто не сможет
				return 1;
			} //WM_INPUT
	
	}

	//Willow: Относительно некритичные или редкие сообщения
	switch (uMsg)
	{
	case WM_SYSKEYDOWN:
		if (wParam == 13)		// Alt+Enter
		{
			if (r_fullScreen)
			{
				Cvar_SetValue("r_fullScreen", !r_fullScreen->value);
			}
			return 0;
		}
		// fall through
	case WM_KEYDOWN:
		Key_Event(MapKey(lParam), qtrue, sys_msg_time);
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		Key_Event(MapKey(lParam), qfalse, sys_msg_time);
		break;
	case WM_HOTKEY:
		return 0;
	case WM_PAINT:
		SCR_DirtyScreen();	// force entire screen to update next frame
		break;
	case WM_DESTROY:
		// let sound and input know about this?
		cl_hwnd = NULL;
		break;
	case WM_ACTIVATE:
	{
		// KJB: Watch this for problems in fullscreen modes with Alt-tabbing.
		qboolean fActive = LOWORD(wParam);
		qboolean fMinimized = (BOOL)HIWORD(wParam);
		AppActivate(fActive != WA_INACTIVE, fMinimized);
		if (reflib_active)
			GLimp_AppActivate(!(fActive == WA_INACTIVE));
	}
	break;
	case WM_MOVE:
		if (!r_fullScreen->integer)
		{
			RECT r;
			int xPos = (short)LOWORD(lParam);    // horizontal position 
			int yPos = (short)HIWORD(lParam);    // vertical position 

			r.left = 0;
			r.top = 0;
			r.right = 1;
			r.bottom = 1;

			int style = GetWindowLong(hWnd, GWL_STYLE);
			AdjustWindowRect(&r, style, FALSE);

			Cvar_SetValue("vid_xpos", xPos + r.left);
			Cvar_SetValue("vid_ypos", yPos + r.top);
			vid_xpos->modified = qfalse;
			vid_ypos->modified = qfalse;
			if (ActiveApp)
				IN_Activate(qtrue);
		}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)
			return 0;
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case MM_MCINOTIFY:
	//	LONG CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		CDAudio_MessageHandler(hWnd, uMsg, wParam, lParam);
		break;
	case WM_CREATE:
		cl_hwnd = hWnd;

		//Разрешить события WM_INPUT
			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			Rid[0].dwFlags =
				(r_fullScreen && r_fullScreen->value) ?
				//If set, the mouse button click does not activate the other window
				RIDEV_CAPTUREMOUSE
				//If set, this enables the caller to receive the input even when the caller is not in the foreground.
				//Note that hwndTarget must be specified
				| RIDEV_INPUTSINK
				//If set, this prevents any devices specified by usUsagePage or usUsage from generating legacy messages.
				//This is only for the mouse and keyboard.
				| RIDEV_NOLEGACY
				:
			0;
			Rid[0].hwndTarget = cl_hwnd;
			RegisterRawInputDevices(Rid, 1, sizeof(RAWINPUTDEVICE));
	
		break;

	case WM_CLOSE:
		///		if(MessageBox (NULL, "Are you sure you want to quit?", "Confirm Exit", MB_YESNO | MB_ICONQUESTION) == IDYES)
		///			PostQuitMessage( 0 );	// Exit program when the user will close program
	
		return 0;	// Внимание! Если вместо нуля возвращать DefWindowProc(), то программа остается висеть в памяти!
					// Так мы запрещаем Alt+F4, вместо этого срабатывает просто F4	;)

					///	default:	// pass all unhandled messages to DefWindowProc
					///		no_jmp = false;
					///		return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}

	/* return 0 if handled message, 1 if not */

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void VID_Restart_f (void) {
	vid_ref->modified = qtrue;
}

void VID_Front_f (void) {
	SetWindowLong (cl_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	SetForegroundWindow (cl_hwnd);
}

/*
** VID_GetModeInfo
*/
typedef struct vidmode_s {
	const char *description;
	int         width, height;
	int         mode;
} vidmode_t;

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
	{ "3840x2160",  3840, 2160, 18 },   // 16:9 ULTRA HD
	{ "4096x1716",  4096, 1716, 19 },   // 2.39:1 DCI 4K WIDE
	{ "4096x2160",  4096, 2160, 20 },   // 1.89:1 DCI 4K
	{ "Custom",		-1, -1, 21 }		// custom
};



qboolean VID_GetModeInfo (int *width, int *height, int mode) {
	if (mode < 0 || mode >= VID_NUM_MODES)
		return qfalse;

	if (mode == 21) {
		*width = r_customWidth->value;
		*height = r_customHeight->value;
	}
	else {
		*width = vid_modes[mode].width;
		*height = vid_modes[mode].height;
	}

	return qtrue;
}

/*
** VID_UpdateWindowPosAndSize
*/
void VID_UpdateWindowPosAndSize (int x, int y) {
	RECT r;
	int		style;
	int		w, h;

	r.left = 0;
	r.top = 0;
	r.right = viddef.width;
	r.bottom = viddef.height;

	style = GetWindowLong (cl_hwnd, GWL_STYLE);
	AdjustWindowRect (&r, style, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	MoveWindow (cl_hwnd, vid_xpos->value, vid_ypos->value, w, h, TRUE);
}

/*
** VID_NewWindow
*/
void VID_NewWindow (int width, int height) {
	viddef.width = width;
	viddef.height = height;

	cl.force_refdef = qtrue;		// can't use a paused refdef
}

void VID_FreeReflib (void) {
	//	memset (&re, 0, sizeof(re));
	reflib_active = qfalse;
}

/*
==============
VID_StartRefresh
==============
*/

#include "../ref_gl/r_local.h"
qboolean VID_StartRefresh (void) {

	if (reflib_active) {
		R_Shutdown ();
		VID_FreeReflib ();
	}

	Com_Printf ("==== Starting OpenGL Renderer ====\n");

	Swap_Init ();

	if (R_Init (global_hInstance, MainWndProc) == -1) {
		R_Shutdown ();
		VID_FreeReflib ();
		return qfalse;
	}
	reflib_active = qtrue;
	return qtrue;
}

/*
============
VID_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to
update the rendering DLL and/or video mode to match.
============
*/

void VID_CheckChanges (void) {
	if (win_noalttab->modified) {
		if (win_noalttab->integer) {
			WIN_DisableAltTab ();
		}
		else {
			WIN_EnableAltTab ();
		}
		win_noalttab->modified = qfalse;
	}

	/*	if ( vid_ref->modified )
		{
		cl.force_refdef = qtrue;		// can't use a paused refdef
		S_StopAllSounds();
		}
		*/
	while (vid_ref->modified) {
		/*
		** refresh has changed
		*/

		cl.force_refdef = qtrue;		// can't use a paused refdef
		S_StopAllSounds ();

		vid_ref->modified = qfalse;
		r_fullScreen->modified = qtrue;
		cl.refresh_prepped = qfalse;
		cls.disable_screen = qtrue;
		CL_ClearDecals ();

		if (!VID_StartRefresh ())
			Com_Error (ERR_FATAL, "Error during initialization video");

		cls.disable_screen = qfalse;
		CL_InitImages ();
	}

	/*
	** update our window position
	*/
	if (vid_xpos->modified || vid_ypos->modified) {
		if (!r_fullScreen->integer)
			VID_UpdateWindowPosAndSize (vid_xpos->integer, vid_ypos->integer);

		vid_xpos->modified = qfalse;
		vid_ypos->modified = qfalse;
	}
}

/*
============
VID_Init
============
*/
void VID_Init (void) {
	/* Create the video variables so we know how to start the graphics drivers */
	vid_ref = Cvar_Get ("vid_ref", "xpgl", 0);
	vid_xpos = Cvar_Get ("vid_xpos", "0", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get ("vid_ypos", "0", CVAR_ARCHIVE);
	r_fullScreen = Cvar_Get ("r_fullScreen", "0", CVAR_ARCHIVE);
	r_customWidth = Cvar_Get ("r_customWidth", "1024", CVAR_ARCHIVE);
	r_customHeight = Cvar_Get ("r_customHeight", "768", CVAR_ARCHIVE);
	win_noalttab = Cvar_Get ("win_noalttab", "0", CVAR_ARCHIVE);


	/* Add some console commands that we want to handle */
	Cmd_AddCommand ("vid_restart", VID_Restart_f);
	Cmd_AddCommand ("vid_front", VID_Front_f);


	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges ();
}

/*
============
VID_Shutdown
============
*/
void VID_Shutdown (void) {
	if (reflib_active) {
		R_Shutdown ();
		VID_FreeReflib ();
	}
}
