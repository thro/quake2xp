/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
 * Copyright (C) 2001 Robert Bäuml
 * Copyright (C) 2002 W. P. va Paassen
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
 * Audio-CD backend in SDL
 *
 * =======================================================================
 */

#include <SDL.h>
#include "../qcommon/qcommon.h"
#include "../client/cdaudio.h"

static qboolean cdValid = qfalse;
static qboolean initialized = qfalse;
static qboolean enabled = qtrue;
static qboolean playLooping = qfalse;
static SDL_CD *cd_id;
static int lastTrack = 0;
static int loopcounter = 0;

cvar_t *cd_nocd;
cvar_t *cd_dev;
cvar_t *cd_volume;
cvar_t *cd_loopcount;
cvar_t *cd_looptrack;

static void CD_f ();

static void
CDAudio_Eject ()
{
	if ( !cd_id || !enabled )
		return;

	if ( SDL_CDEject( cd_id ) )
		Com_DPrintf( "Unable to eject CD-ROM tray.\n" );
}

void
CDAudio_Play ( int track, qboolean looping )
{
	CDstatus cd_stat;

	lastTrack = track + 1;

	if ( !cd_id || !enabled )
		return;

	cd_stat = SDL_CDStatus( cd_id );

	if ( !cdValid )
	{
		if ( !CD_INDRIVE( cd_stat ) || ( !cd_id->numtracks ) )
		{
			return;
		}

		cdValid = qtrue;
	}

	if ( ( track < 1 ) || ( track >= cd_id->numtracks ) )
	{
		Com_DPrintf( "CDAudio: Bad track number: %d\n", track );
		return;
	}

	track--;

	if ( cd_stat == CD_PLAYING )
	{
		if ( cd_id->cur_track == track )
			return;

		CDAudio_Stop();
	}

	if ( SDL_CDPlay( cd_id, cd_id->track [ track ].offset,
				 cd_id->track [ track ].length ) )
	{
		Com_DPrintf( "CDAudio_Play: Unable to play track: %d (%s)\n", track + 1, SDL_GetError() );
		return;
	}

	playLooping = looping;
}

void
CDAudio_Stop ()
{
	int cdstate;

	if ( !cd_id || !enabled )
		return;

	cdstate = SDL_CDStatus( cd_id );

	if ( ( cdstate != CD_PLAYING ) && ( cdstate != CD_PAUSED ) )
		return;

	if ( SDL_CDStop( cd_id ) )
		Com_DPrintf( "CDAudio_Stop: Failed to stop track.\n" );

	playLooping = 0;
}

void
CDAudio_Pause ()
{
	if ( !cd_id || !enabled )
		return;

	if ( SDL_CDStatus( cd_id ) != CD_PLAYING )
		return;

	if ( SDL_CDPause( cd_id ) )
		Com_DPrintf( "CDAudio_Pause: Failed to pause track.\n" );
}

void
CDAudio_Resume ()
{
	if ( !cd_id || !enabled )
		return;

	if ( SDL_CDStatus( cd_id ) != CD_PAUSED )
		return;

	if ( SDL_CDResume( cd_id ) )
		Com_DPrintf( "CDAudio_Resume: Failed to resume track.\n" );
}

void
CDAudio_Update ()
{
	static int cnt = 0;

	if ( !cd_id || !enabled )
		return;

	/* this causes too much overhead to be executed every frame */
	if ( ++cnt == 16 )
	{
		cnt = 0;

		if ( cd_nocd->value )
		{
			CDAudio_Stop();
			return;
		}

		if ( playLooping &&
			 ( SDL_CDStatus( cd_id ) != CD_PLAYING ) &&
			 ( SDL_CDStatus( cd_id ) != CD_PAUSED ) )
		{
			if (loopcounter >= cd_loopcount->value) {
				CDAudio_Play( cd_looptrack->value, qtrue );
			} else {
				CDAudio_Play( lastTrack, qtrue );
				loopcounter++;
			}
		}
	}
}

int
CDAudio_Init ()
{
	cvar_t *cv;

	if ( initialized )
		return 0;

	cd_nocd = Cvar_Get( "cd_nocd", "0", CVAR_ARCHIVE );
	cd_dev = Cvar_Get( "cd_dev", "0", CVAR_ARCHIVE );

	// XXX: we can't adjust CD volume with SDL, so it is ignored
	cd_volume = Cvar_Get( "cd_volume", "1", CVAR_ARCHIVE );

	cd_loopcount = Cvar_Get ("cd_loopcount", "25", CVAR_ARCHIVE); //was 4
	cd_looptrack = Cvar_Get ("cd_looptrack", "11", 0);

	if ( cd_nocd->value )
		return -1;

	if ( SDL_WasInit( SDL_INIT_EVERYTHING ) == 0 )
	{
		if ( SDL_Init( SDL_INIT_CDROM ) < 0 )
		{
			Com_Printf( "Couldn't init SDL cdrom: %s\n", SDL_GetError() );
			return ( -1 );
		}
	}
	else if ( SDL_WasInit( SDL_INIT_CDROM ) == 0 )
	{
		if ( SDL_InitSubSystem( SDL_INIT_CDROM ) < 0 )
		{
			Com_Printf( "Couldn't init SDL cdrom: %s\n", SDL_GetError() );
			return ( -1 );
		}
	}

	cd_id = SDL_CDOpen( cd_dev->value );

	if ( !cd_id )
	{
		Com_Printf( "CDAudio_Init: Unable to open default CD-ROM drive: %s\n",
				SDL_GetError() );
		return ( -1 );
	}

	initialized = qtrue;
	enabled = qtrue;
	cdValid = qtrue;

	if ( !CD_INDRIVE( SDL_CDStatus( cd_id ) ) )
	{
		Com_Printf( "CDAudio_Init: No CD in drive.\n" );
		cdValid = qfalse;
	}

	if ( !cd_id->numtracks )
	{
		Com_Printf( "CDAudio_Init: CD contains no audio tracks.\n" );
		cdValid = qfalse;
	}

	Cmd_AddCommand( "cd", CD_f );
	Com_Printf( "CD Audio Initialized.\n" );
	return ( 0 );
}

void
CDAudio_Shutdown ()
{
	if ( !cd_id )
		return;

	CDAudio_Stop();
	SDL_CDClose( cd_id );
	cd_id = NULL;

	if ( SDL_WasInit( SDL_INIT_EVERYTHING ) == SDL_INIT_CDROM ) {
		SDL_Quit();
	} else {
		SDL_QuitSubSystem( SDL_INIT_CDROM );
	}

	initialized = qfalse;
}

static void
CD_f ()
{
	char *command;
	int cdstate;

	if ( Cmd_Argc() < 2 )
		return;

	command = Cmd_Argv( 1 );

	if ( !Q_strcasecmp( command, "on" ) )
		enabled = qtrue;

	if ( !Q_strcasecmp( command, "off" ) )
	{
		if ( !cd_id )
			return;

		cdstate = SDL_CDStatus( cd_id );

		if ( ( cdstate == CD_PLAYING ) || ( cdstate == CD_PAUSED ) )
			CDAudio_Stop();

		enabled = qfalse;
		return;
	}

	if ( !Q_strcasecmp( command, "play" ) )
	{
		CDAudio_Play( (byte) atoi( Cmd_Argv( 2 ) ), qfalse );
		return;
	}

	if ( !Q_strcasecmp( command, "loop" ) )
	{
		CDAudio_Play( (byte) atoi( Cmd_Argv( 2 ) ), qtrue );
		return;
	}

	if ( !Q_strcasecmp( command, "stop" ) )
	{
		CDAudio_Stop();
		return;
	}

	if ( !Q_strcasecmp( command, "pause" ) )
	{
		CDAudio_Pause();
		return;
	}

	if ( !Q_strcasecmp( command, "resume" ) )
	{
		CDAudio_Resume();
		return;
	}

	if ( !Q_strcasecmp( command, "eject" ) )
	{
		CDAudio_Eject();
		return;
	}

	if ( !Q_strcasecmp( command, "info" ) )
	{
		if ( !cd_id )
		{
			return;
		}

		cdstate = SDL_CDStatus( cd_id );
		Com_Printf( "%d tracks\n", cd_id->numtracks );

		if ( cdstate == CD_PLAYING )
		{
			Com_Printf( "Currently %s track %d\n",
					playLooping ? "looping" : "playing",
					cd_id->cur_track + 1 );
		}
		else if ( cdstate == CD_PAUSED )
		{
			Com_Printf( "Paused %s track %d\n",
					playLooping ? "looping" : "playing",
					cd_id->cur_track + 1 );
		}

		return;
	}
}

void
CDAudio_Activate ( qboolean active )
{
	if ( active )
		CDAudio_Resume();
	else
		CDAudio_Pause();
}

