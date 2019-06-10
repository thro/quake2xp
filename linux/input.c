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
 * This is the Quake II input system, written in SDL.
 *
 * =======================================================================
 */

#include <SDL.h>
#include "../client/client.h"
#include "../ref_gl/r_local.h"

static qboolean input_started = qfalse;

#define MOUSE_MAX 3000
#define MOUSE_MIN 40

static qboolean	grab_on;
static cvar_t   *in_grab;
static cvar_t	*fullscreen;
static int		mouse_x, mouse_y;
static int		old_mouse_x, old_mouse_y;
static int		mouse_buttonstate;
static int		mouse_oldbuttonstate;

struct {
	int key;
	int down;
} keyq[128];

int keyq_head=0;
int keyq_tail=0;
int mx;
int my; 

extern SDL_Surface	 *surface; 
static unsigned char KeyStates[SDLK_LAST];
static qboolean		 mlooking;

static cvar_t *m_filter;
static cvar_t *exponential_speedup;

inline void
Do_Key_Event (int key, qboolean down)
{
	Key_Event(key, down, Sys_Milliseconds());
}

/*
 * This function translates the SDL keycodes
 * to the internal key representation of the
 * id Tech 2 engine.
 */
int
IN_TranslateSDLtoQ2Key(unsigned int keysym)
{
    int key = 0;

    if( keysym >= SDLK_SPACE && keysym < SDLK_DELETE )
    {
        /* These happen to match the ASCII chars */
        key = (int)keysym;
    }
    else
    {
        switch( keysym )
        {
            case SDLK_PAGEUP:       key = K_PGUP;          break;
            case SDLK_KP9:          key = K_KP_PGUP;       break;
            case SDLK_PAGEDOWN:     key = K_PGDN;          break;
            case SDLK_KP3:          key = K_KP_PGDN;       break;
            case SDLK_KP7:          key = K_KP_HOME;       break;
            case SDLK_HOME:         key = K_HOME;          break;
            case SDLK_KP1:          key = K_KP_END;        break;
            case SDLK_END:          key = K_END;           break;
            case SDLK_KP4:          key = K_KP_LEFTARROW;  break;
            case SDLK_LEFT:         key = K_LEFTARROW;     break;
            case SDLK_KP6:          key = K_KP_RIGHTARROW; break;
            case SDLK_RIGHT:        key = K_RIGHTARROW;    break;
            case SDLK_KP2:          key = K_KP_DOWNARROW;  break;
            case SDLK_DOWN:         key = K_DOWNARROW;     break;
            case SDLK_KP8:          key = K_KP_UPARROW;    break;
            case SDLK_UP:           key = K_UPARROW;       break;
            case SDLK_ESCAPE:       key = K_ESCAPE;        break;
            case SDLK_KP_ENTER:     key = K_KP_ENTER;      break;
            case SDLK_RETURN:       key = K_ENTER;         break;
            case SDLK_TAB:          key = K_TAB;           break;
            case SDLK_F1:           key = K_F1;            break;
            case SDLK_F2:           key = K_F2;            break;
            case SDLK_F3:           key = K_F3;            break;
            case SDLK_F4:           key = K_F4;            break;
            case SDLK_F5:           key = K_F5;            break;
            case SDLK_F6:           key = K_F6;            break;
            case SDLK_F7:           key = K_F7;            break;
            case SDLK_F8:           key = K_F8;            break;
            case SDLK_F9:           key = K_F9;            break;
            case SDLK_F10:          key = K_F10;           break;
            case SDLK_F11:          key = K_F11;           break;
            case SDLK_F12:          key = K_F12;           break;

            case SDLK_BACKSPACE:    key = K_BACKSPACE;     break;
            case SDLK_KP_PERIOD:    key = K_KP_DEL;        break;
            case SDLK_DELETE:       key = K_DEL;           break;
            case SDLK_PAUSE:        key = K_PAUSE;         break;

            case SDLK_LSHIFT:
            case SDLK_RSHIFT:       key = K_SHIFT;         break;

            case SDLK_LCTRL:
            case SDLK_RCTRL:        key = K_CTRL;          break;

            case SDLK_RALT:
            case SDLK_LALT:         key = K_ALT;           break;

            case SDLK_KP5:          key = K_KP_5;          break;
            case SDLK_INSERT:       key = K_INS;           break;
            case SDLK_KP0:          key = K_KP_INS;        break;
            case SDLK_KP_PLUS:      key = K_KP_PLUS;       break;
            case SDLK_KP_MINUS:     key = K_KP_MINUS;      break;
            case SDLK_KP_DIVIDE:    key = K_KP_SLASH;      break;

            default:
                break;
        }
    }

    return key;
}

/*
 * Input event processing
 */
void 
IN_GetEvent(SDL_Event *event)
{
	unsigned int key;
	
	switch(event->type) 
	{
		/* The mouse wheel */
		case SDL_MOUSEBUTTONDOWN:
			if (event->button.button == 4) 
			{
				keyq[keyq_head].key = K_MWHEELUP;
				keyq[keyq_head].down = qtrue;
				keyq_head = (keyq_head + 1) & 127;
				keyq[keyq_head].key = K_MWHEELUP;
				keyq[keyq_head].down = qfalse;
				keyq_head = (keyq_head + 1) & 127;
			} 
			else if (event->button.button == 5) 
			{
				keyq[keyq_head].key = K_MWHEELDOWN;
				keyq[keyq_head].down = qtrue;
				keyq_head = (keyq_head + 1) & 127;
				keyq[keyq_head].key = K_MWHEELDOWN;
				keyq[keyq_head].down = qfalse;
				keyq_head = (keyq_head + 1) & 127;
			} 
			break;

		case SDL_MOUSEBUTTONUP:
			break;

		/* The user pressed a button */
		case SDL_KEYDOWN:
			/* Fullscreen switch via Alt-Return */
			if ( (KeyStates[SDLK_LALT] || KeyStates[SDLK_RALT]) && (event->key.keysym.sym == SDLK_RETURN) )
			{

				SDL_WM_ToggleFullScreen(surface);

				if (surface->flags & SDL_FULLSCREEN) 
					Cvar_SetValue( "r_fullScreen", 1 );
				else 
					Cvar_SetValue( "r_fullScreen", 0 );

				fullscreen->modified = qfalse; 
				gl_state.fullscreen = fullscreen->value;
				break;
			}

			KeyStates[event->key.keysym.sym] = 1;

			/* Get the pressed key and add it to the key list */
			key = IN_TranslateSDLtoQ2Key(event->key.keysym.sym);
			if (key)
			{
				keyq[keyq_head].key = key;
				keyq[keyq_head].down = qtrue;
				keyq_head = (keyq_head + 1) & 127;
			}
			break;

		/* The user released a key */
		case SDL_KEYUP:
			if (KeyStates[event->key.keysym.sym]) 
			{
				KeyStates[event->key.keysym.sym] = 0;

				/* Get the pressed key and remove it from the key list */
				key = IN_TranslateSDLtoQ2Key(event->key.keysym.sym);
				if (key) 
				{
					keyq[keyq_head].key = key;
					keyq[keyq_head].down = qfalse;
					keyq_head = (keyq_head + 1) & 127;
				}
			}
			break;
	}
}

/*
 * Updates the state of the input queue
 */
void IN_Update(void)
{
SDL_Event event;
	int bstate;

	while (SDL_PollEvent(&event))
		IN_GetEvent(&event);

	/* Mouse button processing. Button 4 and 5 are the mousewheel and thus not processed here. */

	if (!mx && !my)
		SDL_GetRelativeMouseState(&mx, &my);

	mouse_buttonstate = 0;
	bstate = SDL_GetMouseState(NULL, NULL);

	if (SDL_BUTTON(1) & bstate)
	mouse_buttonstate |= (1 << 0);
  
	if (SDL_BUTTON(3) & bstate) 
		mouse_buttonstate |= (1 << 1);
  
	if (SDL_BUTTON(2) & bstate) 
		mouse_buttonstate |= (1 << 2);
  
	if (SDL_BUTTON(6) & bstate)
		mouse_buttonstate |= (1 << 3);
  
	if (SDL_BUTTON(7) & bstate)
		mouse_buttonstate |= (1 << 4);

	/* Grab and ungrab the mouse if the console is opened */
	switch ((int)in_grab->value) {
	case 0:
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		break;
	case 1:
		SDL_WM_GrabInput(SDL_GRAB_ON);
		break;
	case 2:
		if (!grab_on && cl_paused->value == 0) {
			SDL_WM_GrabInput(SDL_GRAB_ON);
			grab_on = qtrue;
		} else if (grab_on && cl_paused->value != 0) {
			SDL_WM_GrabInput(SDL_GRAB_OFF);
			grab_on = qfalse;
		}
		break;
	default:
		Cvar_SetValue("in_grab", 2);
		break;
	}
    
    if (cl_paused->value == 0)
        Music_Resume();
    else
        Music_Pause();

	/* Process the key events */
	while (keyq_head != keyq_tail) {
		Do_Key_Event(keyq[keyq_tail].key, keyq[keyq_tail].down);
		keyq_tail = (keyq_tail + 1) & 127;
	}
}

/*
 * Gets the mouse state
 */
void IN_GetMouseState(int *x, int *y, int *state) {
	*x = mx;
	*y = my;
	*state = mouse_buttonstate;
} 

/*
 * Cleares the mouse state
 */
void IN_ClearMouseState() 
{
	mx = my = 0;
}
 
/*
 * Look up
 */
static void
IN_MLookDown ( void )
{
	mlooking = qtrue;
}

/*
 * Look down
 */
static void
IN_MLookUp ( void )
{
	mlooking = qfalse;
	IN_CenterView();
}

/*
 * Initializes the backend
 */
void
IN_Init ( void )
{
	if (input_started)
		return;

	m_filter = Cvar_Get( "m_filter", "0", CVAR_ARCHIVE );

	exponential_speedup = Cvar_Get( "exponential_speedup", "0", CVAR_ARCHIVE );

	Cmd_AddCommand( "+mlook", IN_MLookDown );
	Cmd_AddCommand( "-mlook", IN_MLookUp );

	mouse_x = mouse_y = 0.0;

	/* SDL stuff */
	SDL_EnableUNICODE( 0 );
    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

	in_grab = Cvar_Get ("in_grab", "2", CVAR_ARCHIVE);
	fullscreen = Cvar_Get ("r_fullScreen", "1", CVAR_ARCHIVE);
	grab_on = qfalse;
	SDL_WM_GrabInput(SDL_GRAB_OFF);

	Com_Printf( "Input initialized.\n" );
	input_started = qtrue;
}

/*
 * Shuts the backend down
 */
void
IN_Shutdown ( void )
{
	if (!input_started)
		return;

    // clears the input queue
	keyq_head = 0;
	keyq_tail = 0;
	memset(keyq, 0, sizeof(keyq));

	Cmd_RemoveCommand("+mlook");
	Cmd_RemoveCommand("-mlook");
	Com_Printf("Input shut down.\n");
	input_started = qfalse;
}

/*
 * Mouse button handling
 */
void
IN_MouseButtons ( void )
{
	int i;

	IN_GetMouseState( &mouse_x, &mouse_y, &mouse_buttonstate );

	for ( i = 0; i < 3; i++ )
	{
		if ( ( mouse_buttonstate & ( 1 << i ) ) && !( mouse_oldbuttonstate & ( 1 << i ) ) )
			Do_Key_Event( K_MOUSE1 + i, qtrue );

		if ( !( mouse_buttonstate & ( 1 << i ) ) && ( mouse_oldbuttonstate & ( 1 << i ) ) )
			Do_Key_Event( K_MOUSE1 + i, qfalse );
	}

	if ( ( mouse_buttonstate & ( 1 << 3 ) ) && !( mouse_oldbuttonstate & ( 1 << 3 ) ) )
		Do_Key_Event( K_MOUSE4, qtrue );

	if ( !( mouse_buttonstate & ( 1 << 3 ) ) && ( mouse_oldbuttonstate & ( 1 << 3 ) ) )
		Do_Key_Event( K_MOUSE4, qfalse );

	if ( ( mouse_buttonstate & ( 1 << 4 ) ) && !( mouse_oldbuttonstate & ( 1 << 4 ) ) )
		Do_Key_Event( K_MOUSE5, qtrue );

	if ( !( mouse_buttonstate & ( 1 << 4 ) ) && ( mouse_oldbuttonstate & ( 1 << 4 ) ) )
		Do_Key_Event( K_MOUSE5, qfalse );

	mouse_oldbuttonstate = mouse_buttonstate;
}

/*
 * Move handling
 */
void
IN_Move ( usercmd_t *cmd )
{
	IN_GetMouseState( &mouse_x, &mouse_y, &mouse_buttonstate );
	
	if ( m_filter->value )
	{
		if ( ( mouse_x > 1 ) || ( mouse_x < -1) )
		{
			mouse_x = ( mouse_x + old_mouse_x ) * 0.5;
		}
		if ( ( mouse_y > 1 ) || ( mouse_y < -1) )
		{
			mouse_y = ( mouse_y + old_mouse_y ) * 0.5;
		}
	}

	old_mouse_x = mouse_x;
	old_mouse_y = mouse_y;

	if ( mouse_x || mouse_y )
	{
		if ( !exponential_speedup->value )
		{
			mouse_x *= sensitivity->value;
			mouse_y *= sensitivity->value;
		}
		else
		{
			if ( ( mouse_x > MOUSE_MIN ) || ( mouse_y > MOUSE_MIN ) ||
					( mouse_x < -MOUSE_MIN ) || ( mouse_y < -MOUSE_MIN ) )
			{
				mouse_x = ( mouse_x * mouse_x * mouse_x ) / 4;
				mouse_y = ( mouse_y * mouse_y * mouse_y ) / 4;

				if ( mouse_x > MOUSE_MAX )
				{
					mouse_x = MOUSE_MAX;
				}
				else if ( mouse_x < -MOUSE_MAX )
				{
					mouse_x = -MOUSE_MAX;
				}

				if ( mouse_y > MOUSE_MAX )
				{
					mouse_y = MOUSE_MAX;
				}
				else if ( mouse_y < -MOUSE_MAX )
				{
					mouse_y = -MOUSE_MAX;
				}
			}
		}

		/* add mouse X/Y movement to cmd */
		if ( ( in_strafe.state & 1 ) ||
				( lookstrafe->value && mlooking ) )
		{
			cmd->sidemove += m_side->value * mouse_x;
		}
		else
		{
			cl.viewangles [ YAW ] -= m_yaw->value * mouse_x;
		}

		if ( ( mlooking || freelook->value ) &&
				!( in_strafe.state & 1 ) )
		{
			cl.viewangles [ PITCH ] += m_pitch->value * mouse_y;
		}
		else
		{
			cmd->forwardmove -= m_forward->value * mouse_y;
		}

		IN_ClearMouseState();
	}
}

void IN_Frame(void)
{
    IN_MouseButtons();
}
