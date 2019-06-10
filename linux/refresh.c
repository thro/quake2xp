/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
 * Copyright (C) 2010 Yamagi Burmeister
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * This file implements an OpenGL context via SDL
 *
 * =======================================================================
 */ 

#include <SDL.h>

#include "../ref_gl/r_local.h"
#include "../client/ref.h"

/* The window icon */
#include "q2icon.xbm"
 
SDL_Surface		*surface = NULL;

/*
 * Initialzes the SDL OpenGL context
 */
qboolean GLimp_Init(void *hinstance, void *wndproc)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
		char driverName[ 64 ];

        if (SDL_Init(SDL_INIT_VIDEO) == -1)
        {
			Com_Printf(S_COLOR_RED "Couldn't init SDL video: %s.\n", SDL_GetError());
            return qfalse;
        }
		SDL_VideoDriverName( driverName, sizeof( driverName ) - 1 );
        Com_Printf(S_COLOR_GREEN "\nInitialized SDL video, driver is \"%s\".\n\n", driverName );
	}

	return qtrue;
}

/*
 * Sets the window icon
 */
static void SetSDLIcon()
{
	SDL_Surface *icon;
	SDL_Color	color;
	Uint8		*ptr;
	int			i;
	int			mask;

	icon = SDL_CreateRGBSurface(SDL_SWSURFACE, q2icon_width, q2icon_height, 8, 0, 0, 0, 0);

	if (icon == NULL)
	{
		return; 
	}
	
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 0);

	color.r = 255;
	color.g = 255;
	color.b = 255;
	
	SDL_SetColors(icon, &color, 0, 1);
	
	color.r = 0;
	color.g = 16;
	color.b = 0;
	
	SDL_SetColors(icon, &color, 1, 1);

	ptr = (Uint8 *)icon->pixels;
	
	for (i = 0; i < sizeof(q2icon_bits); i++) 
	{
		for (mask = 1; mask != 0x100; mask <<= 1) {
			*ptr = (q2icon_bits[i] & mask) ? 1 : 0;
			ptr++;
		}		
	}

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}

void GL_UpdateSwapInterval()
{
	// XXX: needs restarting video mode
}

/*
 * Swaps the buffers to show the new frame
 */
 
 extern float ref_realtime;
 
void GLimp_EndFrame (void)
{
	SDL_GL_SwapBuffers(); 
  ref_realtime=Sys_Milliseconds()		* 0.0005f;
}

/*
 * Changes the video mode, and initializes the OpenGL window
 */
rserr_t GLimp_SetMode(unsigned *pwidth, unsigned *pheight, int mode, qboolean fullscreen)
{
    int width, height;
	int flags;
	int stencil_bits;

    if (!VID_GetModeInfo( &width, &height, mode))
	{
		Com_Printf(S_COLOR_RED "GLimp_SetMode: invalid mode\n" );
		return rserr_invalid_mode;
	}

	if (surface && (surface->w == width) && (surface->h == height))
	{
		/* Are we running fullscreen? */
		int isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;

		/* We should, but we don't */
		if (fullscreen != isfullscreen)
			SDL_WM_ToggleFullScreen(surface);

		/* Do we now? */
		isfullscreen = (surface->flags & SDL_FULLSCREEN) ? 1 : 0;
		if (fullscreen == isfullscreen) {
            gl_state.fullscreen = fullscreen;
			return rserr_ok;
        }
        else if (fullscreen)
            return rserr_invalid_fullscreen;
	}
	
	/* Is the surface used? */
	if (surface)
		SDL_FreeSurface(surface);

	/* Create the window */
	VID_NewWindow(width, height);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, r_vsync->integer ? 1 : 0);
	  
  // sdl2 
  /*
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, r_glMajorVersion->value);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, r_glMinorVersion->value);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
  */
	if (r_multiSamples->integer > 1) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, r_multiSamples->integer);
	}
	
	/* Initiate the flags */
	flags = SDL_OPENGL;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	
	/* Set the icon */
	SetSDLIcon();
	
    if ((surface = SDL_SetVideoMode(width, height, 0, flags)) == NULL) {
			Com_Printf("SDL SetVideoMode failed: %s\n", SDL_GetError());
            return rserr_invalid_mode;
	}
	Com_Printf("setting mode "S_COLOR_YELLOW"%d"S_COLOR_WHITE":"S_COLOR_YELLOW"[%ix%i]\n", mode , width, height);
	// Print information
//	if (!SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil_bits))
//		Com_Printf("Got %d bits of stencil.\n", stencil_bits);

	if (r_multiSamples->value > 1){
		qglEnable(GL_MULTISAMPLE);
		Com_Printf("Use multisampling %ix samples per pixel.\n", (int)r_multiSamples->value);
	} 
	

	/* Window title */
	SDL_WM_SetCaption("quake2xp", "quake2xp");

	/* No cursor */
	SDL_ShowCursor(0);

    /* Notify of current values */
    *pwidth = width;
    *pheight = height;
    gl_state.fullscreen = fullscreen;

	return rserr_ok;
}

/*
 * Shuts the SDL render backend down
 */
void GLimp_Shutdown( void )
{
    Com_Printf("GLimp shut down\n");

	if (surface)
	{
		SDL_FreeSurface(surface);
	}

	surface = NULL;
	
	if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
	{
		SDL_Quit();
	}
	else
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}
