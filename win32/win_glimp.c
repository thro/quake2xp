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
/*
** GLW_IMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/

#include "../ref_gl/r_local.h"

// Enable High Performance Graphics while using Integrated Graphics.
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;        // Nvidia
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;  // AMD

#define	WINDOWBORDERLESS_STYLE	(WS_VISIBLE | WS_POPUP)

#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE)

typedef struct {
	qboolean		accelerated;
	qboolean		drawToWindow;
	qboolean		supportOpenGL;
	qboolean		doubleBuffer;
	qboolean		rgba;

	int				colorBits;
	int				alphaBits;
	int				depthBits;
	int				stencilBits;
	int				samples;
} glwPixelFormatDescriptor_t;

static LRESULT CALLBACK FakeWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
#define	WINDOW_CLASS_FAKE		"quake2xp Fake Window"
#define	WINDOW_NAME				"quake2xp"

qboolean GLW_InitDriver(void);

glwstate_t glw_state;

/*
** VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"quake2xp"

qboolean VID_CreateWindow( int width, int height, qboolean fullscreen )
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos, *vid_BorderlessWindow;
	int				stylebits;
	int				x, y;
	int				exstyle;
	DEVMODE			dm;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)glw_state.wndproc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = glw_state.hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc) )
		VID_Error (ERR_FATAL, "Couldn't register window class");

	// compute width and height
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if (glw_state.desktopName[0])
	{
		if (!EnumDisplaySettings(glw_state.desktopName, ENUM_CURRENT_SETTINGS, &dm))
		{
			memset(&dm, 0, sizeof(dm));
			dm.dmSize = sizeof(dm);
		}
	}
	/// save real monitor position in the virtual monitor
	glw_state.desktopPosX = dm.dmPosition.x;
	glw_state.desktopPosY = dm.dmPosition.y;
	
	vid_BorderlessWindow = Cvar_Get("vid_BorderlessWindow", "0", CVAR_ARCHIVE);

	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE;
	}
	else
	{
		exstyle = 0;
		if(!vid_BorderlessWindow->integer)
			stylebits = WINDOW_STYLE;
		else
			stylebits = WINDOWBORDERLESS_STYLE;
	}

	r.left = glw_state.desktopPosX;
	r.top = glw_state.desktopPosY;
	r.right = width + glw_state.desktopPosX;
	r.bottom = height + glw_state.desktopPosY;
	
	glw_state.virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	glw_state.virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	glw_state.virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	glw_state.virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	glw_state.borderWidth = GetSystemMetrics(SM_CXBORDER) * 3;
	glw_state.borderHeight = GetSystemMetrics(SM_CYBORDER) * 3 + GetSystemMetrics(SM_CYCAPTION);
	vid_xpos = Cvar_Get("vid_xpos", "0", CVAR_ARCHIVE);
	vid_ypos = Cvar_Get("vid_ypos", "0", CVAR_ARCHIVE);

	AdjustWindowRect (&r, stylebits, FALSE);

	if (fullscreen)
	{
		x = glw_state.desktopPosX;
		y = glw_state.desktopPosY;
	}
	else
	{
		x = vid_xpos->integer;
		y = vid_ypos->integer;

		// adjust window coordinates if necessary
		// so that the window is completely on screen
		if (x < glw_state.virtualX)
			x = glw_state.virtualX;
		if (x > glw_state.virtualX + glw_state.virtualWidth - 64)
			x = glw_state.virtualX + glw_state.virtualWidth - 64;
		if (y < glw_state.virtualY)
			y = glw_state.virtualY;
		if (y > glw_state.virtualY + glw_state.virtualHeight - 64)
			y = glw_state.virtualY + glw_state.virtualHeight - 64;
	}

	glw_state.hWnd = CreateWindowEx (
		 exstyle, 
		 WINDOW_CLASS_NAME,
		 "quake2xp",
		 stylebits,
		 x, y, width, height,
		 NULL,
		 NULL,
		 glw_state.hInstance,
		 NULL);

	if (!glw_state.hWnd)
		VID_Error (ERR_FATAL, "Couldn't create window");

	UINT(WINAPI *GetDpiForWindow)(HWND hwnd) = NULL;
	HINSTANCE u32DLL = LoadLibrary("user32.dll"); // Windows 10, version 1607 [desktop apps only]

	if(u32DLL)
		GetDpiForWindow = (UINT(WINAPI *)(HWND hwnd)) GetProcAddress(u32DLL, "GetDpiForWindow");

	if (GetDpiForWindow) {
		glw_state.dpi = GetDpiForWindow(glw_state.hWnd);
		Com_Printf("...desktop dpi is: "S_COLOR_GREEN"%i"S_COLOR_WHITE"dpi\n", glw_state.dpi);
		if (glw_state.dpi > 96)
			Com_Printf(S_COLOR_YELLOW"...force dpi awareness:"S_COLOR_GREEN" ok\n");
	}

	ShowWindow( glw_state.hWnd, SW_SHOW );
	UpdateWindow( glw_state.hWnd );

	// init all the gl stuff for the window
	if (!GLW_InitDriver())
	{
		Com_Printf(S_COLOR_RED"...destroying window\n");
		Com_Printf(S_COLOR_RED "VID_CreateWindow() - GLimp_InitGL failed\n");

		ShowWindow(glw_state.hWnd, SW_HIDE);
		DestroyWindow(glw_state.hWnd);
		glw_state.hWnd = NULL;

		UnregisterClass(WINDOW_CLASS_NAME, glw_state.hInstance);
		return qfalse;
	}

	SetForegroundWindow( glw_state.hWnd );
	SetFocus( glw_state.hWnd );

	// let the sound and input subsystems know about the new window
	VID_NewWindow (width, height);

	return qtrue;
}

#define MAX_SUPPORTED_MONITORS  16
int monitorCounter;
MONITORINFO monitorInfos[MAX_SUPPORTED_MONITORS];
char        monitorNames[MAX_SUPPORTED_MONITORS][16];

BOOL GetDisplayMonitorInfo(char *monitorName, char *monitorModel)
{
	DISPLAY_DEVICE  dd;
	int             i = 0;
	BOOL            bRet = FALSE;

	monitorModel[0] = 0;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);

	while (EnumDisplayDevices(glw_state.desktopName, i, &dd, EDD_GET_DEVICE_INTERFACE_NAME))
	{
		if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE)
		{
			char *p, *s;
			char deviceID[128];
			char regPath[128];
			byte edid[128];
			HKEY hKey;
			int j = 0;
			lstrcpy(deviceID, dd.DeviceID);
			p = strstr(deviceID, "DISPLAY");
			if (p)
			{
				s = p;
				while (1)
				{
					if (*s == 0)
					{
						j = -1; // not found
						break;
					}
					if (*s == '#')
					{
						j++;
						if (j == 3)
						{
							*s = 0;
							break;
						}
						else
							*s = '\\';
					}
					s++;
				}
				if (j != -1)
				{
					LSTATUS err;
					Com_sprintf(regPath, sizeof(regPath), "SYSTEM\\CurrentControlSet\\Enum\\%s\\Device Parameters\\", p);
					err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey);
					if (err == ERROR_SUCCESS)
					{
						DWORD buflen = sizeof(edid);
						err = RegQueryValueEx(hKey, "EDID", NULL, NULL, edid, &buflen);
						RegCloseKey(hKey);
						if (err == ERROR_SUCCESS)
						{
							int k, m, n, descOffs[4] = { 54, 72, 90, 108 };
							for (k = 0; k < 4; k++)
							{
								byte *desc = &edid[descOffs[k]];
								if (desc[0] == 0 && desc[1] == 0 && desc[2] == 0 && desc[3] == 0xFC)
								{
									Q_strncpyz(monitorModel, &desc[5], 13);
									n = strlen(monitorModel);
									for (m = 0; m < n; m++)
										if (monitorModel[m] == '\n')
											monitorModel[m] = 0;
									break;
								}
							}
						}
					}
				}
			}

			lstrcpy(monitorName, dd.DeviceString);
			bRet = TRUE;
			break;
		}
		i++;
	}

	return bRet;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	monitorInfos[monitorCounter].cbSize = sizeof(monitorInfos[monitorCounter]);
	if (GetMonitorInfo(hMonitor, &monitorInfos[monitorCounter]))
	{
		monitorCounter++;
		if (monitorCounter == MAX_SUPPORTED_MONITORS)
			return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK MonitorEnumProc2(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	monitorInfos[monitorCounter].cbSize = sizeof(monitorInfos[monitorCounter]);
	if (GetMonitorInfo(hMonitor, &monitorInfos[monitorCounter]))
	{
		Com_Printf("   " S_COLOR_GREEN "%i" S_COLOR_WHITE ": %i " S_COLOR_GREEN "x" S_COLOR_WHITE " %i", monitorCounter + 1,
			abs(monitorInfos[monitorCounter].rcMonitor.left - monitorInfos[monitorCounter].rcMonitor.right),
			abs(monitorInfos[monitorCounter].rcMonitor.top - monitorInfos[monitorCounter].rcMonitor.bottom));
		if (monitorNames[monitorCounter][0])
			Com_Printf(S_COLOR_YELLOW" %s", monitorNames[monitorCounter]);
		else
			Com_Printf(S_COLOR_YELLOW" Unknown model");
		if (monitorInfos[monitorCounter].dwFlags & MONITORINFOF_PRIMARY)
			Com_Printf(" (" S_COLOR_YELLOW "primary" S_COLOR_WHITE ")");
		Com_Printf("\n");
		monitorCounter++;
		if (monitorCounter == MAX_SUPPORTED_MONITORS)
			return FALSE;
	}
	return TRUE;
}
void GLimp_InitADL();
void GLimp_InitNvApi();
extern qboolean adlInit;

/*
** GLimp_SetMode
*/

rserr_t GLimp_SetMode( unsigned *pwidth, unsigned *pheight, int mode, qboolean fullscreen )
{
	int width, height, i, idx, cvm, cdsRet, j;
	const char *win_fs[] = { "Window", "Full Screen" };
	cvar_t	*vid_monitor = Cvar_Get("vid_monitor", "0", CVAR_ARCHIVE);
	char	monitorName[128], monitorModel[16];
	HDC		hDC;
	DEVMODE dm;

	GLimp_InitNvApi();
	GLimp_InitADL();
	
	Com_Printf("\n==================================\n\n");

	Com_Printf(S_COLOR_YELLOW"...Initializing OpenGL display\n");
	
	Com_Printf("\n==================================\n");

	monitorCounter = 0;
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

	monitorName[0] = 0;
	if (vid_monitor->integer <= 0 || vid_monitor->integer > MAX_SUPPORTED_MONITORS)
		cvm = 0;    // лишь бы что-то было корректное...
	else
		cvm = vid_monitor->integer - 1;
	idx = -1;
	for (j = 0; j < MAX_SUPPORTED_MONITORS; j++)
	{
		monitorNames[j][0] = 0;
		for (i = 1; i < 256; i++)   // много?
		{
			Com_sprintf(glw_state.desktopName, sizeof(glw_state.desktopName), "\\\\.\\Display%i", i);
			memset(&dm, 0, sizeof(dm));
			dm.dmSize = sizeof(dm);
			if (EnumDisplaySettings(glw_state.desktopName, ENUM_CURRENT_SETTINGS, &dm))
			{
				char    tempMonitorName[128];
				glw_state.desktopPosX = dm.dmPosition.x;
				glw_state.desktopPosY = dm.dmPosition.y;
				if (GetDisplayMonitorInfo(tempMonitorName, monitorModel))
				{
					hDC = CreateDC(glw_state.desktopName, tempMonitorName, NULL, NULL);
					if (hDC)
					{   /// monitor found, so compare positions in virtual desktop
						glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
						glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
						if (monitorInfos[j].rcMonitor.left == glw_state.desktopPosX &&
							monitorInfos[j].rcMonitor.top == glw_state.desktopPosY &&
							abs(monitorInfos[j].rcMonitor.left - monitorInfos[j].rcMonitor.right) == glw_state.desktopWidth &&
							abs(monitorInfos[j].rcMonitor.top - monitorInfos[j].rcMonitor.bottom) == glw_state.desktopHeight)
						{
							lstrcpy(monitorNames[j], monitorModel);
							if (j == cvm)
							{
								lstrcpy(monitorName, tempMonitorName);
								idx = i;
								i = 256;    /// break
							}
						}
					}
					DeleteDC(hDC);
				}
			}
		}
	}
	if (idx == -1 || vid_monitor->integer <= 0 || vid_monitor->integer > MAX_SUPPORTED_MONITORS)    /// not found :(
	{
		glw_state.desktopName[0] = 0;
		Com_Printf("\n...using " S_COLOR_YELLOW "primary" S_COLOR_WHITE " monitor\n");
		glw_state.desktopPosX = 0;
		glw_state.desktopPosY = 0;
		hDC = GetDC(GetDesktopWindow());
		glw_state.desktopBitPixel = GetDeviceCaps(hDC, BITSPIXEL);
		glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
		glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
		ReleaseDC(GetDesktopWindow(), hDC);
	}
	else
	{
		Com_sprintf(glw_state.desktopName, sizeof(glw_state.desktopName), "\\\\.\\Display%i", idx);
		Com_Printf("\n...calling " S_COLOR_YELLOW "CreateDC" S_COLOR_WHITE "('" S_COLOR_GREEN "%s" S_COLOR_WHITE "','" S_COLOR_GREEN "%s" S_COLOR_WHITE "')\n", glw_state.desktopName, monitorName);
		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		EnumDisplaySettings(glw_state.desktopName, ENUM_CURRENT_SETTINGS, &dm);
		glw_state.desktopPosX = dm.dmPosition.x;
		glw_state.desktopPosY = dm.dmPosition.y;
		hDC = CreateDC(glw_state.desktopName, monitorName, NULL, NULL);
		glw_state.desktopBitPixel = GetDeviceCaps(hDC, BITSPIXEL);
		glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
		glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
		DeleteDC(hDC);

		if (monitorNames[cvm][0])
			Com_Printf("...using monitor " S_COLOR_GREEN "%i " S_COLOR_WHITE "(" S_COLOR_GREEN "%s" S_COLOR_WHITE ")\n", vid_monitor->integer, monitorNames[cvm]);
		else
			Com_Printf("...using monitor " S_COLOR_GREEN "%i\n", vid_monitor->integer);

	}
		Com_Printf(S_COLOR_YELLOW"\n...Available monitors:\n\n");
		monitorCounter = 0;
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc2, 0);


	Com_Printf("\n==================================\n\n");

	if ( !VID_GetModeInfo( &width, &height, mode ) )
	{
		Com_Printf(S_COLOR_RED " invalid mode\n" );
		return rserr_invalid_mode;
	}


	if(mode == 0){
	width = glw_state.desktopWidth;
	height = glw_state.desktopHeight;
	}

	Com_Printf ("...setting mode "S_COLOR_YELLOW"%d"S_COLOR_WHITE":"S_COLOR_YELLOW"[%ix%i]", mode , width, height);

	if(width > glw_state.desktopWidth || height > glw_state.desktopHeight){
		width = glw_state.desktopWidth;
		height = glw_state.desktopHeight;
		Com_Printf(S_COLOR_RED "\n!!!Invalid Resolution!!!\n"S_COLOR_MAGENTA"Set Current Desktop Resolution\n"S_COLOR_WHITE"%i"S_COLOR_GREEN"x"S_COLOR_WHITE"%i "S_COLOR_WHITE"%s\n", 
					width, height, win_fs[fullscreen]);
		
	} else
	Con_Printf( PRINT_ALL, " "S_COLOR_WHITE"%s\n", win_fs[fullscreen] );


	// destroy the existing window
	if (glw_state.hWnd)
	{
		GLimp_Shutdown ();
	}

	// do a CDS if needed
	if ( fullscreen )
	{
		Com_Printf("...attempting fullscreen\n" );

		memset( &dm, 0, sizeof( dm ) );
		dm.dmSize = sizeof( dm );

		dm.dmPelsWidth  = width;
		dm.dmPelsHeight = height;
		dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

		gl_state.monitorWidth = GetSystemMetrics(SM_CXSCREEN);
		gl_state.monitorHeight = GetSystemMetrics(SM_CYSCREEN);

		/* display frequency */
		if (r_displayRefresh->integer != 0){
	        gl_state.displayrefresh	= r_displayRefresh->integer;
			dm.dmDisplayFrequency	= r_displayRefresh->integer;
			dm.dmFields				|= DM_DISPLAYFREQUENCY;
			Com_Printf("...display frequency is "S_COLOR_GREEN"%d"S_COLOR_WHITE" hz\n", gl_state.displayrefresh);
		}
		else {
			
			int displayref = GetDeviceCaps (hDC, VREFRESH);
            dm.dmDisplayFrequency	= displayref;
			dm.dmFields				|= DM_DISPLAYFREQUENCY;
			Com_Printf("...using desktop frequency.\n");
		}
     
			// force set 32-bit color depth
			dm.dmBitsPerPel = 32;
			dm.dmFields |= DM_BITSPERPEL;

			
		Con_Printf( PRINT_ALL, "...calling CDS: " );
		
		if (glw_state.desktopName[0])
			cdsRet = ChangeDisplaySettingsEx(glw_state.desktopName, &dm, NULL, CDS_FULLSCREEN, NULL);
		else
			cdsRet = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

		if (cdsRet == DISP_CHANGE_SUCCESSFUL )
		{
			*pwidth = width;
			*pheight = height;

			gl_state.fullscreen = qtrue;

			Com_Printf(S_COLOR_GREEN"ok\n" );

			if ( !VID_CreateWindow (width, height, qtrue) )
				return rserr_invalid_mode;

			return rserr_ok;
		}
		else
		{
			*pwidth = width;
			*pheight = height;

			Com_Printf(S_COLOR_RED"failed\n" );

			Com_Printf("...calling CDS assuming dual monitors:" );

			dm.dmPelsWidth = width * 2;
			dm.dmPelsHeight = height;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		
			/*
			** our first CDS failed, so maybe we're running on some weird dual monitor
			** system 
			*/
			if (glw_state.desktopName[0])
				cdsRet = ChangeDisplaySettingsEx(glw_state.desktopName, &dm, NULL, CDS_FULLSCREEN, NULL);
			else
				cdsRet = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

			if (cdsRet != DISP_CHANGE_SUCCESSFUL )
			{
				Com_Printf(S_COLOR_RED" failed\n" );

				Com_Printf(S_COLOR_YELLOW"...setting windowed mode\n" );

				ChangeDisplaySettings( 0, 0 );

				*pwidth = width;
				*pheight = height;
				gl_state.fullscreen = qfalse;
				if ( !VID_CreateWindow (width, height, qfalse) )
					return rserr_invalid_mode;
				return rserr_invalid_fullscreen;
			}
			else
			{
				Com_Printf(S_COLOR_GREEN" ok\n" );
				if ( !VID_CreateWindow (width, height, qtrue) )
					return rserr_invalid_mode;
				gl_state.fullscreen = qtrue;
				return rserr_ok;
			}
		}
	}
	else
	{
		Com_Printf("...setting windowed mode\n" );

		ChangeDisplaySettings( 0, 0 );

		*pwidth = width;
		*pheight = height;
		gl_state.fullscreen = qfalse;
		if ( !VID_CreateWindow (width, height, qfalse) )
			return rserr_invalid_mode;
	}
	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/

void GLimp_Shutdown( void )
{

	if ( qwglMakeCurrent && !qwglMakeCurrent( NULL, NULL ) )
		Com_Printf(S_COLOR_RED"ref_gl::R_Shutdown() - wglMakeCurrent failed\n");
	if ( glw_state.hGLRC )
	{
		if (  qwglDeleteContext && !qwglDeleteContext( glw_state.hGLRC ) )
			Com_Printf(S_COLOR_RED"ref_gl::R_Shutdown() - wglDeleteContext failed\n");
		glw_state.hGLRC = NULL;
	}
	if (glw_state.hDC)
	{
		if ( !ReleaseDC( glw_state.hWnd, glw_state.hDC ) )
			Com_Printf(S_COLOR_RED"ref_gl::R_Shutdown() - ReleaseDC failed\n" );
		glw_state.hDC   = NULL;
	}
	if (glw_state.hWnd)
	{
		DestroyWindow (	glw_state.hWnd );
		glw_state.hWnd = NULL;
	}

	UnregisterClass (WINDOW_CLASS_NAME, glw_state.hInstance);

	if ( gl_state.fullscreen )
	{
		ChangeDisplaySettings( 0, 0 );
		gl_state.fullscreen = qfalse;
	}
	
	if(nvApiInit)
		NvAPI_Unload();

	if(adlInit)
		ADL_Shutdown();
}


typedef enum XP_PROCESS_DPI_AWARENESS { //with out win header, some rename....
	XP_PROCESS_DPI_UNAWARE = 0,
	XP_PROCESS_SYSTEM_DPI_AWARE = 1,
	XP_PROCESS_PER_MONITOR_DPI_AWARE = 2
} XP_PROCESS_DPI_AWARENESS;

void VID_SetProcessDpiAwareness(void) {

	HRESULT(WINAPI *SetProcessDpiAwareness)(XP_PROCESS_DPI_AWARENESS dpiAwareness) = NULL;
	BOOL(WINAPI *SetProcessDPIAware)(void) = NULL;

	HINSTANCE u32DLL	= LoadLibrary("user32.dll"); //win 7-8.0
	HINSTANCE shDLL		= LoadLibrary("shcore.dll"); //win 8.1-10
	
	if (shDLL)
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(XP_PROCESS_DPI_AWARENESS))GetProcAddress(shDLL, "SetProcessDpiAwareness");
	else 
		if(u32DLL)
		SetProcessDPIAware = (BOOL(WINAPI *)(void)) GetProcAddress(u32DLL, "SetProcessDPIAware");

	if (SetProcessDpiAwareness) {
		SetProcessDpiAwareness(XP_PROCESS_PER_MONITOR_DPI_AWARE); 
	}
	else
		SetProcessDPIAware(); 
	
	if (shDLL)
		FreeLibrary(shDLL);
	if (u32DLL)
		FreeLibrary(u32DLL);
}


qboolean GLimp_Init( void *hinstance, void *wndproc )
{
	Con_Printf (PRINT_ALL, "\n");
	Com_Printf ("========"S_COLOR_YELLOW"System Information"S_COLOR_WHITE"========\n");
	Con_Printf (PRINT_ALL, "\n");
	
	Sys_CpuID();
	Sys_GetMemorySize();

	if(!Sys_CheckWindowsVersion()){
		Com_Printf( S_COLOR_RED "GLimp_CheckWindowsVersion() - Unsupported windows version.\nWindows 7 and above required\n" );
		QGL_Shutdown();
		Com_Error(ERR_FATAL, "GLimp_CheckWindowsVersion() - Unsupported windows version.\nWindows 7 and above required\n");
	}

	Sys_WindowsInfo();

	VID_SetProcessDpiAwareness();

	glw_state.hInstance = ( HINSTANCE ) hinstance;
	glw_state.wndproc = wndproc;
	
	return qtrue;
	

}

/*
==================
GLW_InitExtensions

==================
*/
qboolean ext_sRGB, arb_sRGB;

void GLW_InitExtensions() {

	qwglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)qwglGetProcAddress("wglGetExtensionsStringARB");

	if (!qwglGetExtensionsStringARB)
	{
		Com_Printf(S_COLOR_RED  "WGL extension string not found!");
		VID_Error(ERR_FATAL, "WGL extension string not found!");
	}

	glw_state.wglExtsString = qwglGetExtensionsStringARB(glw_state.hDCFake);

	if (glw_state.wglExtsString == NULL)
		Com_Printf(S_COLOR_RED "WGL_EXTENSION not found!\n");

	Com_Printf("\n");

	Com_Printf("=============================\n");
	Com_Printf(S_COLOR_GREEN"Checking Basic WGL Extensions\n");
	Com_Printf("=============================\n\n");

	if (strstr(glw_state.wglExtsString, "WGL_ARB_pixel_format"))
	{
		Com_Printf("...using WGL_ARB_pixel_format\n");
		qwglGetPixelFormatAttribivARB	= (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)	qwglGetProcAddress	("wglGetPixelFormatAttribivARB");
		qwglGetPixelFormatAttribfvARB	= (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)	qwglGetProcAddress	("wglGetPixelFormatAttribfvARB");
		qwglChoosePixelFormatARB		= (PFNWGLCHOOSEPIXELFORMATARBPROC)		qwglGetProcAddress	("wglChoosePixelFormatARB");

	}
	else {
		Com_Printf(S_COLOR_RED"WARNING!!! WGL_ARB_pixel_format not found\nOpenGL subsystem not initiation\n");
		VID_Error(ERR_FATAL, "WGL_ARB_pixel_format not found!");
	}

	if (strstr(glw_state.wglExtsString, "WGL_EXT_swap_control")) {
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)qwglGetProcAddress("wglSwapIntervalEXT");
		Com_Printf("...using WGL_EXT_swap_control\n");
	}
	else
		Com_Printf(S_COLOR_RED"...WGL_EXT_swap_control not found\n");

	gl_state.wgl_swap_control_tear = qfalse;
	if (strstr(glw_state.wglExtsString, "WGL_EXT_swap_control_tear")) {
		Com_Printf("...using WGL_EXT_swap_control_tear\n");
		gl_state.wgl_swap_control_tear = qtrue;
	}
	else {
		Com_Printf(S_COLOR_RED"WGL_EXT_swap_control_tear not found\n");
	}

	if (strstr(glw_state.wglExtsString, "WGL_ARB_multisample"))
		if (r_multiSamples->integer < 2)
			Com_Printf("" S_COLOR_YELLOW "...ignoring WGL_ARB_multisample\n");
		else
			Com_Printf("...using WGL_ARB_multisample\n");
		
	if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context")) {
		qwglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)qwglGetProcAddress("wglCreateContextAttribsARB");

		if (qwglCreateContextAttribsARB)
			Com_Printf("...using WGL_ARB_create_context\n");
			else{
				Com_Printf(S_COLOR_RED"WARNING!!! WGL_ARB_create_context not found\nOpenGL subsystem not initiation\n");
				VID_Error(ERR_FATAL, "WGL_ARB_create_context not found!");
			}
		}

	ext_sRGB = qfalse;
	arb_sRGB = qfalse;

		if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context_profile"))
			Com_Printf("...using WGL_ARB_create_context_profile\n");
//==========================================================
		if (strstr(glw_state.wglExtsString, "WGL_ARB_framebuffer_sRGB")) {
			Com_Printf("...using WGL_ARB_framebuffer_sRGB\n");
			arb_sRGB = qtrue;
		}
		else {
			if (strstr(glw_state.wglExtsString, "WGL_EXT_framebuffer_sRGB"))
				Com_Printf("...using WGL_EXT_framebuffer_sRGB\n");
			ext_sRGB = qtrue;
		}

		if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context_no_error"))
			Com_Printf("...using WGL_ARB_create_context_no_error\n");
}

/*
==================
GLW_ShutdownFakeOpenGL

==================
*/

static void GLW_ShutdownFakeOpenGL(void) {

	if (glw_state.hGLRCFake) {
		if (qwglMakeCurrent)
			qwglMakeCurrent(NULL, NULL);
		if (qwglDeleteContext)
			qwglDeleteContext(glw_state.hGLRCFake);

		glw_state.hGLRCFake = NULL;
	}

	if (glw_state.hDCFake) {
		ReleaseDC(glw_state.hWndFake, glw_state.hDCFake);
		glw_state.hDCFake = NULL;
	}

	if (glw_state.hWndFake) {
		DestroyWindow(glw_state.hWndFake);
		glw_state.hWndFake = NULL;

		UnregisterClass(WINDOW_CLASS_FAKE, glw_state.hInstance);
	}
}

/*
==================
GLW_InitFakeOpenGL

==================
*/
static qboolean GLW_InitFakeOpenGL(void) {
	WNDCLASSEX				wndClass;
	PIXELFORMATDESCRIPTOR	PFD;
	int						pixelFormat;

	// register the frame class
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = 0;
	wndClass.lpfnWndProc = FakeWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = glw_state.hInstance;
	wndClass.hIcon = 0;
	wndClass.hIconSm = 0;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
	wndClass.lpszMenuName = 0;
	wndClass.lpszClassName = WINDOW_CLASS_FAKE;

	if (!RegisterClassEx(&wndClass))
		return qfalse;

	// create the fake window
	glw_state.hWndFake = CreateWindowEx(0, WINDOW_CLASS_FAKE, WINDOW_NAME, WINDOWBORDERLESS_STYLE, 0, 0, 320, 240, NULL, NULL, glw_state.hInstance, NULL);
	if (!glw_state.hWndFake) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	glw_state.hDCFake = GetDC(glw_state.hWndFake);
	if (!glw_state.hDCFake) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	// choose a pixel format
	memset(&PFD, 0, sizeof(PIXELFORMATDESCRIPTOR));

	PFD.cColorBits = 32;
	PFD.cDepthBits = 24;
	PFD.cStencilBits = 8;
	PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	PFD.iLayerType = PFD_MAIN_PLANE;
	PFD.iPixelType = PFD_TYPE_RGBA;
	PFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	PFD.nVersion = 1;

	pixelFormat = ChoosePixelFormat(glw_state.hDCFake, &PFD);

	if (!pixelFormat) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	// set the pixel format
	DescribePixelFormat(glw_state.hDCFake, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &PFD);

	if (!SetPixelFormat(glw_state.hDCFake, pixelFormat, &PFD)) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	// create the fake GL context and make it current
	glw_state.hGLRCFake = qwglCreateContext(glw_state.hDCFake);

	if (!glw_state.hGLRCFake) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	if (!qwglMakeCurrent(glw_state.hDCFake, glw_state.hGLRCFake)) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	return qtrue;
}

static qboolean GLW_ChoosePixelFormat() {
	PIXELFORMATDESCRIPTOR	PFD;
	int pixelFormat, samples;
	uint numFormats, sRGBformat;
	qboolean sRGB;

	sRGB = arb_sRGB|ext_sRGB;
	if (arb_sRGB)
		sRGBformat = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
	else if (ext_sRGB)
		sRGBformat = WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT;

	qglGetIntegerv(GL_MAX_SAMPLES, &gl_config.maxSamples);

	if (r_multiSamples->integer > gl_config.maxSamples)
		Cvar_SetInteger(r_multiSamples, gl_config.maxSamples);

	if (r_multiSamples->integer <= 1)
		samples = 0;
	else
		samples = r_multiSamples->integer;

	const int pAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, samples ? GL_TRUE: GL_FALSE,
		WGL_SAMPLES_ARB, samples,
		sRGBformat, sRGB ? GL_TRUE: GL_FALSE,
		0
	};

	Com_Printf(S_COLOR_YELLOW"\n...Attempting PIXELFORMAT:\n\n");

	if (!qwglChoosePixelFormatARB(glw_state.hDC, pAttribs, NULL, 1, &pixelFormat, &numFormats)) {
		Com_Printf(S_COLOR_RED "...qwglChoosePixelFormatARB() failed.");
		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;

		return qfalse;
	}

	int numPixelFormats = WGL_NUMBER_PIXEL_FORMATS_ARB;
	qwglGetPixelFormatAttribivARB(glw_state.hDC, 0, 0, 1, &numPixelFormats, &numPixelFormats);

	Com_Printf("..." S_COLOR_GREEN "%i " S_COLOR_WHITE "pixel formats found\n", numPixelFormats);
	Com_Printf("...Selected " S_COLOR_GREEN "%i " S_COLOR_WHITE "PIXELFORMAT\n", pixelFormat);
	
	Com_Printf("...setting pixel format: ");
	DescribePixelFormat(glw_state.hDC, pixelFormat, sizeof(PFD), &PFD);
	SetPixelFormat(glw_state.hDC, pixelFormat, &PFD);

	Com_Printf(S_COLOR_GREEN "ok\n");

	gl_config.colorBits = 32;
	gl_config.alphaBits = 8;
	gl_config.depthBits = 24;
	gl_config.stencilBits = 8;
	gl_config.samples = samples;

	Com_Printf("\nPIXELFORMAT: Color "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits, Depth "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits, Alpha "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits,\n             Stencil "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits, MSAA [" S_COLOR_GREEN "%i" S_COLOR_WHITE " max] [" S_COLOR_GREEN "%i"S_COLOR_WHITE" selected]\n\n",
		gl_config.colorBits, gl_config.depthBits, gl_config.alphaBits, gl_config.stencilBits, gl_config.maxSamples, gl_config.samples);

	return qtrue;
}

static qboolean GLW_CreateContext() {

	const char	*profileName[] = { "core", "compatibility" };

	int	contextFlag = r_glDebugOutput->integer ? WGL_CONTEXT_DEBUG_BIT_ARB : GL_CONTEXT_FLAG_NO_ERROR_BIT;
	int	contextMask = r_glCoreProfile->integer ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;

	int	attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,	r_glMajorVersion->integer,
		WGL_CONTEXT_MINOR_VERSION_ARB,	r_glMinorVersion->integer,
		WGL_CONTEXT_FLAGS_ARB,			contextFlag,
		WGL_CONTEXT_PROFILE_MASK_ARB,	contextMask,
		0
	};

	// create the GL context
	Com_Printf("...creating openGL " S_COLOR_GREEN "%i.%i" S_COLOR_YELLOW "  %s" S_COLOR_WHITE " profile context: ", r_glMajorVersion->integer, r_glMinorVersion->integer, profileName[contextMask == WGL_CONTEXT_CORE_PROFILE_BIT_ARB ? 0 : 1]);

	glw_state.hGLRC = qwglCreateContextAttribsARB(glw_state.hDC, 0, attribs);

	if (!glw_state.hGLRC) {
		if (GetLastError() == ERROR_INVALID_VERSION_ARB)
			Com_Error(ERR_FATAL, "Current video card/driver combination does not support OpenGL %i.%i", r_glMajorVersion->integer, r_glMinorVersion->integer);

		Com_Printf(S_COLOR_RED "failed\n");

		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;

		return qfalse;
	}
	Com_Printf(S_COLOR_GREEN "ok\n");
	
	GLW_ShutdownFakeOpenGL();
	return qtrue;
}

qboolean GLW_InitDriver(void) {

	if (!GLW_InitFakeOpenGL()) {
		Com_Printf(S_COLOR_RED "...failed to initialize fake OpenGL context\n");
		return qfalse;
	}
	// get a DC for the current window
	Com_Printf("...getting DC: ");

	glw_state.hDC = GetDC(glw_state.hWnd);
	if (!glw_state.hDC) {
		Com_Printf(S_COLOR_RED "failed\n");
		return qfalse;
	}

	Com_Printf(S_COLOR_GREEN"ok\n");

	GLW_InitExtensions();
	GLW_ChoosePixelFormat();
	GLW_CreateContext();

	// make it current
	Com_Printf("...making context current: ");

	if (!qwglMakeCurrent(glw_state.hDC, glw_state.hGLRC)) {
		Com_Printf(S_COLOR_RED "...wglMakeCurrent() failed.");
		return qfalse;
	}

	Com_Printf(S_COLOR_GREEN "ok\n");

	gl_config.glMajorVersion = r_glMajorVersion->integer;
	gl_config.glMinorVersion = r_glMinorVersion->integer;

	return qtrue;
}


/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/


void GLimp_EndFrame (void)
{
	if ( !qwglSwapBuffers( glw_state.hDC ) )
			VID_Error( ERR_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n" );

	r_newrefdef.time=Sys_Milliseconds() * 0.001f;
	ref_realtime=Sys_Milliseconds()		* 0.0005f;
}


void GL_UpdateSwapInterval()
{

	if(r_vsync->modified)
	r_vsync->modified = qfalse;

	if(gl_state.wgl_swap_control_tear){
	
	if (wglSwapIntervalEXT){
		if(r_vsync->integer >=2)
			wglSwapIntervalEXT(-1);
	else if(r_vsync->integer >=1)
			wglSwapIntervalEXT(1);	
	else
			wglSwapIntervalEXT(0);
		}
	}
	else
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(r_vsync->integer);
	
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
	if ( active )
	{
		SetForegroundWindow( glw_state.hWnd );
		ShowWindow( glw_state.hWnd, SW_RESTORE );
	}
	else
	{
		if ( r_fullScreen->integer )
			ShowWindow( glw_state.hWnd, SW_MINIMIZE );
	}
}
