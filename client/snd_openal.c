/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2007 willow.

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
// snd_dma.c -- main control for any streaming and OpenAL I/O

#include "client.h"
#include "snd_loc.h"

// =======================================================================
// Internal sound data & structures
// =======================================================================
ALuint source_name[MAX_CHANNELS + 1];

// during registration it is possible to have more sounds
// than could actually be referenced during gameplay,
// because we don't want to free anything until we are
// sure we won't need it.
int num_sfx;
char known_sfx_name[MAX_SFX][MAX_QPATH];
ALuint known_sfx_bufferNum[MAX_SFX];

#define		MAX_PLAYSOUNDS	8
playsound_t s_playsounds[MAX_PLAYSOUNDS];
playsound_t s_freeplays;
playsound_t s_pendingplays;

openal_channel_t s_openal_channels[MAX_CHANNELS];
unsigned s_openal_numChannels;

static struct rbtree *knownsounds;

// =======================================================================
// Video & Music streaming
// =======================================================================

static __inline void sq_add (ALuint x) {
	int tail = (streaming.bFirst + streaming.bNumAvail) % NUM_STRBUF;

	// assuming non-full queue
	assert (streaming.bNumAvail < NUM_STRBUF);

	streaming.buffers[tail] = x;
	streaming.bNumAvail++;
}

static __inline ALuint sq_remove (void) {
	ALuint r;

	// assuming non-empty queue
	assert (streaming.bNumAvail > 0);

	r = streaming.buffers[streaming.bFirst];
	streaming.bFirst = (streaming.bFirst + 1) % NUM_STRBUF;
	streaming.bNumAvail--;

	return r;
}

static void S_Streaming_RecycleBuffers (void);

/*
===============================================================================
console functions
===============================================================================
*/

void S_Play (void) {
	int i = 1;
	char name[256];

	while (i < Cmd_Argc ()) {
		if (!strrchr (Cmd_Argv (i), '.')) {
			strcpy (name, Cmd_Argv (i));
			strcat (name, ".wav");
		}
		else
			strcpy (name, Cmd_Argv (i));

		// TO DO - willow: do not cache this data to onboard memory!
		// this seems to be just any random file, we do no need to store
		// it in valuable memory.
		S_StartLocalSound (S_FindName (name, qtrue));
		i++;
	}
}

void S_SoundInfo_f (void) {
	if (!alConfig.hALC) {
		Com_Printf (S_COLOR_RED"Cannot provide OpenAL information\n");
		return;
	}

	Com_Printf ("\n");
	Com_Printf ("AL_VENDOR:     "S_COLOR_GREEN"%s\n", alGetString (AL_VENDOR));
	Com_Printf ("AL_RENDERER:   "S_COLOR_GREEN"%s\n", alGetString (AL_RENDERER));
	Com_Printf ("AL_VERSION:    "S_COLOR_GREEN"%s\n", alGetString (AL_VERSION));
	
	const char *alext;
	Com_Printf("AL_EXTENSIONS:\n");
	alext = alGetString(AL_EXTENSIONS);
	if (alext)
	{
		unsigned l = strlen(alext), ll;
		if (l > 0)
		{
			char buf[128], c;
			char *ptr = (char *)alext;
			char *p;
			while (1)
			{
				ll = 0;
				p = (char*)ptr;
				while (1)
				{
					c = *p;
					if (!c || c == ' ')
						break;
					ll++;
					p++;
				}
				if (ll >= sizeof(buf))
				{
					Com_Printf(S_COLOR_RED"*** extension too long: %i bytes ***\n", ll);
					break;
				}
				if (ll)
				{
					memcpy(buf, ptr, ll);
					buf[ll] = 0;
					Com_Printf(S_COLOR_GREEN"%s\n", buf);
				}
				if (!c)
					break;
				ptr = p + 1;
			}
		}
		else
			Com_Printf(S_COLOR_RED"*** strlen(exts) = 0 ***\n");
	}
	else
		Com_Printf(S_COLOR_RED"*** exts = NULL ***\n");

	Com_Printf ("\n");
	Com_Printf ("DEVICE: "S_COLOR_GREEN"%s\n",
		alcGetString (alConfig.hDevice, ALC_DEVICE_SPECIFIER));
	Com_Printf ("\n");
}

static void AllocChannels (void) {
	s_openal_numChannels = MAX_CHANNELS + 1;	// +1 streaming channel

	while (s_openal_numChannels > MIN_CHANNELS) {
		alGenSources (s_openal_numChannels, source_name);
		--s_openal_numChannels;
		if (alGetError () == AL_NO_ERROR) {
			Com_Printf ("%i mix channels allocated.\n", s_openal_numChannels);
			Com_Printf ("streaming channel allocated.\n");
			return;
		}
	}

	Com_Printf ("Not enough mix channels!\n");
	s_openal_numChannels = 0;
}

/*
================
S_Init
================
*/
void CL_fast_sound_init (void);
void S_Music_f (void);
void S_Play (void);

void S_Init (int hardreset) {

	if (hardreset) {
		s_fxVolume = Cvar_Get ("s_fxVolume", "1", CVAR_ARCHIVE);
		s_show = Cvar_Get ("s_show", "0", 0);
		s_musicVolume = Cvar_Get ("s_musicVolume", "0.8", CVAR_ARCHIVE);
		s_musicSrc = Cvar_Get ("s_musicSrc", "1", CVAR_ARCHIVE);
		s_musicRandom = Cvar_Get ("s_musicRandom", "0", CVAR_ARCHIVE);
		s_device = Cvar_Get ("s_device", "OpenAL Soft", CVAR_ARCHIVE);
		s_useEfx = Cvar_Get ("s_useEfx", "1", CVAR_ARCHIVE);
		s_initSound = Cvar_Get ("s_initSound", "1", CVAR_NOSET);
		s_dynamicReverberation = Cvar_Get("s_dynamicReverberation", "1", CVAR_ARCHIVE);
		s_useHRTF = Cvar_Get("s_useHRTF", "1", CVAR_ARCHIVE);
		s_resamplerQuality = Cvar_Get("s_resamplerQuality", "1", CVAR_ARCHIVE);
		s_resamplerQuality->help = "0- low quality, 4- high quality.";
	}

	if (!s_initSound->value || openalStop) {
		Com_Printf ("\n");
		Com_Printf (S_COLOR_YELLOW"=======Sound not initializing.=======\n");
		Com_Printf ("\n");
		return;
	}
	else {
		if (AL_Init (hardreset)) {
			AllocChannels ();
			if (s_openal_numChannels) {
				S_SoundInfo_f ();

		//		alListenerf (AL_GAIN, /*clamp (s_volume->value, 0, 1)*/ 1.0);

				if (hardreset) {
					Cmd_AddCommand ("play", S_Play);
					Cmd_AddCommand ("stopsound", S_StopAllSounds);
					Cmd_AddCommand ("music", S_Music_f);
					Cmd_AddCommand ("s_info", S_SoundInfo_f);

					CL_fast_sound_init ();

					// Generate some AL Buffers for streaming
					alGenBuffers (NUM_STRBUF, streaming.buffers);

					if (alConfig.efx)
						EFX_RvbInit ();
				}

				S_StopAllSounds ();	// inits freeplays
			}
			else
				AL_Shutdown ();
		}

		if (!s_openal_numChannels) {
			Com_Printf (S_COLOR_RED"OpenAL failed to initialize; no sound available\n");
			Cvar_ForceSet ("s_initSound", "0");	/// Berserker's FIX: устранён крэш если OpenAL не может запуститься
			AL_Shutdown ();
		}
	}

	Com_Printf ("-------------------------------------\n");
}




static void FreeChannels (void) {
	alDeleteSources (s_openal_numChannels + 1, source_name);
	memset (s_openal_channels, 0, sizeof(openal_channel_t)* MAX_CHANNELS);

	s_openal_numChannels = 0;
}


void FreeSounds (void) {
	// Stop all sounds
	S_StopAllSounds ();

	// Free all sounds
	alDeleteBuffers (num_sfx, known_sfx_bufferNum);

	// Clean up
	memset (known_sfx_name, 0, sizeof(known_sfx_name));
	memset (known_sfx_bufferNum, 0, sizeof(known_sfx_bufferNum));
	num_sfx = 0;
}




// =======================================================================
// Shutdown sound engine
// =======================================================================
void CL_fast_sound_close (void);

void S_Shutdown (void) {
	if (s_openal_numChannels) {
		CL_fast_sound_close ();
		FreeSounds ();

		Cmd_RemoveCommand ("play");
		Cmd_RemoveCommand ("stopsound");
		Cmd_RemoveCommand ("music");
		Cmd_RemoveCommand ("s_info");

		if (alConfig.efx)
			EFX_RvbShutdown ();

		FreeChannels ();
		AL_Shutdown ();
	}
}

// =======================================================================
// Soft reset sound engine
// based on the few ideas:
// 1) Once loaded OpenAL library should be not freed on just context or device change matter.
// 2) Unlike source and listener objects, buffer objects can be shared among AL contexts.
// Buffers are referenced by sources. A single buffer can be referred to by multiple sources.
// This separation allows drivers and hardware to optimize storage and processing where applicable.
// =======================================================================
void S_Restart (void) {
	S_Shutdown ();
	S_Init (1);
}


ALuint S_RegisterSound (const char *name) {
	if (name[0] == '*')
		return S_RegisterSexedSound (&cl_entities[cl.playernum + 1].current, name);
	else
		return S_FindName ((char*)name, s_openal_numChannels);
}


playsound_t *S_AllocPlaysound (void) {
	playsound_t *ps = s_freeplays.next;

	if (ps == &s_freeplays)
		return NULL;			// no free playsounds

	// unlink from freelist
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	return ps;
}


/*
=================
S_FreePlaysound
=================
*/
void S_FreePlaysound (playsound_t * ps) {
	// unlink from channel
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	// add to free list
	ps->next = s_freeplays.next;
	s_freeplays.next->prev = ps;
	ps->prev = &s_freeplays;
	s_freeplays.next = ps;
}


/*
=================
S_RegisterSexedSound
=================
*/
extern cvar_t *gender;

ALuint S_RegisterSexedSound (entity_state_t * ent, const char *base) {
	int n;
	char *p;
	char model[MAX_QPATH];
	char sexedFilename[MAX_QPATH];

	// determine what model the client is using
	model[0] = 0;
	n = CS_PLAYERSKINS + ent->number - 1;
	if (cl.configstrings[n][0]) {
		p = strchr (cl.configstrings[n], '\\');
		if (p) {
			p += 1;
			strcpy (model, p);
			p = strchr (model, '/');
			if (p)
				*p = 0;
		}
	}

	// if we can't figure it out, they're male
	if (!model[0]) {
		if (!strcmp (cl.clientinfo->sex, "cyborg"))
			strcpy (model, "cyborg");
		else if (!strcmp (cl.clientinfo->sex, "female"))
			strcpy (model, "female");
		else if (!strcmp (cl.clientinfo->sex, "male"))
			strcpy (model, "male");
		else
			strcpy (model, gender->string);

	}

	// see if we already know of the model specific sound
	Com_sprintf (sexedFilename, sizeof(sexedFilename), "#players/%s/%s", model, base + 1);

	return S_FindName (sexedFilename, qtrue);

}


//=============================================================================
#define AL_TASK_MANAGER__IS_LOOP_ACTIVE		1
#define AL_TASK_MANAGER__IS_SOURCE_RELATIVE 2
#define AL_TASK_MANAGER__TERMINATE			4
#define AL_TASK_MANAGER__EXECUTE			8

#define AL_FLAGS_FLAT2D						1
#define AL_FLAGS_AL_LOOPING					2
#define AL_FLAGS_FIXED_POSITION				4	// Use position instead of fetching entity's origin

typedef struct {
	ALfloat TASK_AL_REFERENCE_DISTANCE;
	ALfloat TASK_AL_ROLLOFF_FACTOR;
	ALfloat TASK_AL_GAIN;
	vec3_t TASK_AL_VELOCITY;
	unsigned long flags;
} channel_task_t;

void Flag_set (channel_task_t * task, unsigned long flags_collection) {
	task->flags |= flags_collection;
}

void Flag_clear (channel_task_t * task, unsigned long flags_collection) {
	task->flags &= ~flags_collection;
}

qboolean Flag_check (channel_task_t * task, unsigned long flags_collection) {
	return task->flags & flags_collection;
}

ALuint Flag_checkAL (channel_task_t * task, unsigned long flags_collection) {
	return task->flags & flags_collection ? AL_TRUE : AL_FALSE;
}

void FlagAL_set (openal_channel_t * ch, unsigned long flags_collection) {
	ch->flags |= flags_collection;
}

void FlagAL_clear (openal_channel_t * ch, unsigned long flags_collection) {
	ch->flags &= ~flags_collection;
}

qboolean FlagAL_check (openal_channel_t * ch,
	unsigned long flags_collection) {
	return ch->flags & flags_collection;
}

ALuint FlagAL_checkAL (openal_channel_t * ch,
	unsigned long flags_collection) {
	return ch->flags & flags_collection ? AL_TRUE : AL_FALSE;
}

void TASK_TerminateChannel (channel_task_t * task, openal_channel_t * ch) {
	Flag_set (task, AL_TASK_MANAGER__TERMINATE);
	ch->bufferNum = 0;			// ch->sfx = NULL;

	//  alSourceStop(ch->sourceNum);
	//  alSourcei(ch->sourceNum, AL_BUFFER, 0);
}

// picks a channel based on priorities, empty slots, number of channels
openal_channel_t *PickChannel_NEW (unsigned int entNum,
	unsigned int entChannel, vec3_t new_vec,
	ALuint * return_index) {
	openal_channel_t *ch;
	int i;
	int firstToDie = -1;
	//  int                 oldestTime = cl.time;
	qboolean is_terminate = qtrue;

	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		i++, ch++) {
		// Don't let game sounds override streaming sounds
		/* if (!i && streaming) continue; */

		// Check if this channel is active
		/*		if (!ch->sfx)							// found free channel
				{
				firstToDie = i;
				is_terminate = qfalse;
				break;
				} else {*/
		ALuint SourceState;

		alGetSourcei (source_name[i], AL_SOURCE_STATE, &SourceState);

		if (SourceState == AL_STOPPED || SourceState == AL_INITIAL)	// The
			// source
			// already
			// out
			// of
			// processing.
		{
			ch->bufferNum = 0;	// ch->sfx = NULL;
			firstToDie = i;
			is_terminate = qfalse;
			break;
		}
	}

	// Emergency channel re-caption.
	// Override sound from same entity willow???????
	/* if (firstToDie == -1) { for (i = 0, ch = s_openal_channels; i <
	   s_openal_numChannels; i++, ch++) if (ch->entNum == entNum &&
	   ch->entChannel == entChannel) { firstToDie = i; break; } } */

	// Emergency channel re-caption. Issue 2
	// Terminate the most distant entity
	if (firstToDie == -1) {
		float len, max;
		vec3_t delta;

		VectorSubtract (cl.refdef.vieworg, new_vec, delta);
		max = VectorLength_Squared (delta);

		for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
			i++, ch++)
		if (!FlagAL_check (ch, AL_FLAGS_FLAT2D)	// Don't touch FLAT 2D
			// samples!
			&& FlagAL_check (ch, AL_FLAGS_AL_LOOPING)
			) {
			// ch->_AL_POSITION must have same data:
			// alGetSourcefv(ch->sourceNum, AL_POSITION, (ALfloat
			// *)&al_position);
			// willow: i assume cl.refdef.vieworg always have the last
			// listener position
			VectorSubtract (cl.refdef.vieworg, ch->_AL_POSITION, delta);
			// willow:
			// actually we need vector length here, but squared length
			// can do the same job faster.
			len = VectorLength_Squared (delta);
			if (len > max) {
				firstToDie = i;
				max = len;
			}
		}

		if (firstToDie == -1) {
			return NULL;
		}
	}

	ch = &s_openal_channels[firstToDie];

	memcpy (ch->_AL_POSITION, new_vec, sizeof(vec3_t));
	FlagAL_clear (ch, AL_FLAGS_FLAT2D);
	// Flag_clear
	// (&Channels_TODO[firstToDie],AL_TASK_MANAGER__IS_SOURCE_RELATIVE);

	ch->entNum = entNum;
	ch->entChannel = entChannel;
	// ch->startTime = cl.time;

	*return_index = source_name[firstToDie];

	// TERMINATE CHANNEL (stop working channel)
	// TASK_TerminateChannel (&Channels_TODO[firstToDie], ch);
	if (is_terminate) {
		alSourceStop (source_name[firstToDie]);
		// alSourceStop(ch->sourceNum);
		// alSourcei(ch->sourceNum, AL_BUFFER, 0);
	}

	return ch;
}

void S_fastsound (vec3_t origin, int entnum, int entchannel,
	ALuint bufferNum, ALfloat gain, ALfloat rolloff_factor) {
	openal_channel_t *ch;
	ALuint sourceNum;

	if (!s_openal_numChannels || !bufferNum)
		return;					// safety check.

	// Pick a channel and start the sound effect
	if (origin)
		ch = PickChannel_NEW (entnum, entchannel, origin, &sourceNum);
	else {
		vec3_t position;
		CL_GetEntityOrigin (entnum, position);
		ch = PickChannel_NEW (entnum, entchannel, position, &sourceNum);
	}

	if (ch) {
		//      VectorCopy(ps->origin, ch->position);
		ch->bufferNum = bufferNum;	// ch->sfx = sfx;

		// Update min/max distance
		alSourcef (sourceNum, AL_REFERENCE_DISTANCE, 28);
		// Set max distance really far away (default)
		// alSourcef(ch->sourceNum, AL_MAX_DISTANCE, 65536);

		alSourcefv (sourceNum, AL_POSITION, ch->_AL_POSITION);

		if (origin) {
			FlagAL_clear (ch, AL_FLAGS_AL_LOOPING);
			FlagAL_set (ch, AL_FLAGS_FIXED_POSITION);
			alSource3f (sourceNum, AL_VELOCITY, 0, 0, 0);
		}
		else {
			FlagAL_clear (ch,
				AL_FLAGS_FIXED_POSITION | AL_FLAGS_AL_LOOPING);
			// willow: TO DO.
			// alSourcefv(sourceNum, AL_VELOCITY,
			// current_task->TASK_AL_VELOCITY);
			alSource3f (sourceNum, AL_VELOCITY, 0, 0, 0);
		}

		alSourcei (sourceNum, AL_BUFFER, bufferNum);
		alSourcei (sourceNum, AL_SOURCE_RELATIVE, AL_FALSE);
		alSourcei (sourceNum, AL_LOOPING,
			FlagAL_checkAL (ch, AL_FLAGS_AL_LOOPING));
		alSourcef (sourceNum, AL_ROLLOFF_FACTOR, rolloff_factor);
		alSourcef (sourceNum, AL_GAIN, /*gain*/ s_fxVolume->value);

		alSourcei(sourceNum, AL_SOURCE_RESAMPLER_SOFT, s_resamplerQuality->integer);

		if (alConfig.efx)
			EFX_RvbProcSrc (ch, sourceNum, qtrue);

		alSourcePlay (sourceNum);
	}
}

void S_fastsound_queue (vec3_t origin, int entnum, int entchannel,
	ALuint bufferNum, float fvol, float attenuation,
	unsigned timeofs) {
	playsound_t *ps, *sort;

	if (!s_openal_numChannels || !bufferNum)
		return;

	// willow: Immediate sound effect start in case zero time delay
	// provided.
	// This is shortcut, just call "S_fastsound" call instead
	if (!timeofs) {
		S_fastsound (origin, entnum, entchannel, bufferNum, fvol,
			attenuation);
		return;
	}
	// Allocate a playSound
	ps = S_AllocPlaysound ();
	if (!ps) {
		Com_Printf ("S_StartSound: active-task queue fault\n");
		return;
	}

	ps->bufferNum = bufferNum;
	ps->entnum = entnum;
	ps->entchannel = entchannel;

	//  ps->flat = is_flat;

	if (origin) {
		ps->fixed_origin = qtrue;
		VectorCopy (origin, ps->origin);
	}
	else
		ps->fixed_origin = qfalse;

	ps->volume = fvol;
	ps->attenuation = attenuation;
	ps->begin = cl.time + timeofs;

	// Sort into the pending playSounds list
	for (sort = s_pendingplays.next;
		sort != &s_pendingplays && sort->begin < ps->begin;
		sort = sort->next);

	ps->next = sort;
	ps->prev = sort->prev;

	ps->next->prev = ps;
	ps->prev->next = ps;
}

// picks a channel based on priorities, empty slots, number of channels
openal_channel_t *PickChannel_lite (ALuint * sourceNum) {
	openal_channel_t *ch;
	int i;
	int firstToDie = -1;
	qboolean terminate = qtrue;

	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		i++, ch++) {
		// Check if this channel is active
		/*		if (!ch->sfx)							// found free channel
				{
				firstToDie = i;
				terminate = qfalse;
				break;
				} else {*/
		ALuint SourceState;

		alGetSourcei (source_name[i], AL_SOURCE_STATE, &SourceState);

		// The source already out of processing.
		if (SourceState == AL_STOPPED || SourceState == AL_INITIAL) {
			firstToDie = i;
			terminate = qfalse;
			break;
		}
		/*		}*/
	}

	// Emergency channel re-caption. Issue 2
	// Terminate the most distant entity
	if (firstToDie == -1) {
		// vec3_t listener;
		float len, max = 0;
		vec3_t delta;
		// alGetListenerfv(AL_POSITION, (ALfloat *)&listener);

		for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
			i++, ch++)
		if (!FlagAL_check (ch, AL_FLAGS_FLAT2D)	// Don't touch FLAT 2D
			// samples!
			&& FlagAL_check (ch, AL_FLAGS_AL_LOOPING)
			) {
			// ch->_AL_POSITION must have same data:
			// alGetSourcefv(ch->sourceNum, AL_POSITION, (ALfloat
			// *)&al_position);
			// willow: i am assume cl.refdef.vieworg always have the
			// last listener position
			VectorSubtract (cl.refdef.vieworg, ch->_AL_POSITION, delta);
			// willow:
			// actually we need vector length here, but squared length
			// can do the same job faster.
			len = VectorLength_Squared (delta);
			if (len > max) {
				firstToDie = i;
				max = len;
			}
		}

		if (firstToDie == -1) {
			return NULL;
		}
	}

	ch = &s_openal_channels[firstToDie];

	memset (ch->_AL_POSITION, 0, sizeof(vec3_t));

	ch->entNum = -1;
	ch->entChannel = 0;
	// ch->startTime = cl.time;

	*sourceNum = source_name[firstToDie];

	// TERMINATE CHANNEL (stop working channel)
	if (terminate) {
		alSourceStop (*sourceNum);
	}

	return ch;
}

/*
==================
S_StartLocalSound
==================
*/
void S_StartLocalSound (ALuint bufferNum) {
	if (s_openal_numChannels && bufferNum) {
		ALuint sourceNum;
		// Pick a channel and start the sound effect
		openal_channel_t *ch = PickChannel_lite (&sourceNum);

		if (ch) {
			FlagAL_clear (ch, AL_FLAGS_AL_LOOPING);
			ch->bufferNum = bufferNum;
			FlagAL_set (ch, AL_FLAGS_FLAT2D | AL_FLAGS_FIXED_POSITION);
			alSourcef (sourceNum, AL_PITCH, 1);
			alSource3f (sourceNum, AL_DIRECTION, 0, 0, 0);
			alSourcef (sourceNum, AL_REFERENCE_DISTANCE, 0);
			alSourcef (sourceNum, AL_ROLLOFF_FACTOR, 0);
			alSource3f (sourceNum, AL_POSITION, 0, 0, 0);
			alSource3f (sourceNum, AL_VELOCITY, 0, 0, 0);
			alSourcei (sourceNum, AL_BUFFER, bufferNum);
			alSourcei (sourceNum, AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcei (sourceNum, AL_LOOPING, AL_FALSE);
			alSourcef (sourceNum, AL_GAIN, /*0.47*/ s_fxVolume->value);
			
			alSourcei(sourceNum, AL_SOURCE_RESAMPLER_SOFT, s_resamplerQuality->integer);

			if (alConfig.efx)
				EFX_RvbProcSrc (ch, sourceNum, qfalse);

			alSourcePlay (sourceNum);
		}
		else {
			Com_Printf ("S_StartLocalSound: Dropped sound\n");
		}
	}
}


/*
==================
S_StopAllSounds
==================
*/
void S_StopAllSounds (void) {
	unsigned i;
	openal_channel_t *ch;

	if (!s_openal_numChannels)
		return;

	// clear all the playsounds
	memset (s_playsounds, 0, sizeof(s_playsounds));
	s_freeplays.next = s_freeplays.prev = &s_freeplays;
	s_pendingplays.next = s_pendingplays.prev = &s_pendingplays;

	for (i = 0; i < MAX_PLAYSOUNDS; i++) {
		s_playsounds[i].prev = &s_freeplays;
		s_playsounds[i].next = s_freeplays.next;
		s_playsounds[i].prev->next = &s_playsounds[i];
		s_playsounds[i].next->prev = &s_playsounds[i];
	}

	// Stop all the channels
	alSourceStopv (s_openal_numChannels, source_name);

	// Mark all the channels free
	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		i++, ch++) {
		ch->bufferNum = AL_NONE;
		alSourcei (source_name[i], AL_BUFFER, AL_NONE);
	}


	// clear all the channels
	// memset(s_openal_channels, 0, sizeof(s_openal_channels));

	S_Streaming_Stop ();
}

openal_channel_t *PickChannel (channel_task_t * Channels_TODO,
	unsigned int entNum, unsigned int entChannel,
	vec3_t new_vec, int *return_index,
	vec3_t listener) {
	openal_channel_t *ch;
	int i;
	int firstToDie = -1;
	qboolean terminate = qfalse;

	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels; i++, ch++) {

		// Don't let game sounds override streaming sounds
		/* if (!i && streaming) continue; */

		// Check if this channel is active
		if (!ch->bufferNum)		// (!ch->sfx)
		{
			// Free channel
			firstToDie = i;
			break;
		}
		/*		if (Flag_checkAL (&Channels_TODO[i], AL_TASK_MANAGER__TERMINATE))
				{
				firstToDie = i;
				terminate = qfalse;
				break;
				}*/
		else {
			ALuint SourceState;

			alGetSourcei (source_name[i], AL_SOURCE_STATE, &SourceState);

			if (SourceState == AL_STOPPED || SourceState == AL_INITIAL)	// The
				// source
				// already
				// out
				// of
				// processing.
			{
				firstToDie = i;
				break;
			}
		}

	}

	// Emergency channel re-caption.
	// Override sound from same entity willow???????
	/* if (firstToDie == -1) { for (i = 0, ch = s_openal_channels; i <
	   s_openal_numChannels; i++, ch++) if (ch->entNum == entNum &&
	   ch->entChannel == entChannel) { firstToDie = i; break; } } */

	// Emergency channel re-caption. Issue 2
	// Terminate the most distant entity
	if (firstToDie == -1) {
		float len, max;
		vec3_t delta;

		terminate = qtrue;		// we need to eat weakest now :)
		if (new_vec) {
			VectorSubtract (listener, new_vec, delta);
			max = VectorLength_Squared (delta);
		}
		else
			max = 0;

		for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
			i++, ch++)
		if (!FlagAL_check (ch, AL_FLAGS_FLAT2D)	// Don't touch FLAT 2D
			// samples!
			&& FlagAL_check (ch, AL_FLAGS_AL_LOOPING)
			) {
			// ch->_AL_POSITION must have same data:
			// alGetSourcefv(ch->sourceNum, AL_POSITION, (ALfloat
			// *)&al_position);
			VectorSubtract (listener, ch->_AL_POSITION, delta);
			// willow:
			// actually we need vector length here, but squared length
			// can do the same job faster.
			len = VectorLength_Squared (delta);
			if (len > max) {
				firstToDie = i;
				max = len;
			}
		}

		if (firstToDie == -1) {
			return NULL;
		}
	}

	ch = &s_openal_channels[firstToDie];

	if (new_vec) {
		memcpy (ch->_AL_POSITION, new_vec, sizeof(vec3_t));
		FlagAL_clear (ch, AL_FLAGS_FLAT2D);
		//      Flag_clear (&Channels_TODO[firstToDie],AL_TASK_MANAGER__IS_SOURCE_RELATIVE);
	}
	else {
		memset (ch->_AL_POSITION, 0, sizeof(vec3_t));
		FlagAL_set (ch, AL_FLAGS_FLAT2D);
		//      Flag_set (&Channels_TODO[firstToDie],AL_TASK_MANAGER__IS_SOURCE_RELATIVE);
	}

	ch->entNum = entNum;
	ch->entChannel = entChannel;
	// ch->startTime = cl.time;
	if (return_index)
		*return_index = firstToDie;

	// TERMINATE CHANNEL (stop working channel)
	if (terminate)
		TASK_TerminateChannel (&Channels_TODO[firstToDie], ch);
	// alSourceStop(ch->sourceNum);
	// alSourcei(ch->sourceNum, AL_BUFFER, 0);

	return ch;
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update (vec3_t listener_position, vec3_t velocity,
	float orientation[6]) {
	entity_state_t *ent;
	int i, j;
	byte cl_parse_entities_goodjob[MAX_PARSE_ENTITIES];
	playsound_t *ps;
	openal_channel_t *ch;
	channel_task_t Channels_TODO[MAX_CHANNELS];
	int logical_channel_index;

	if (!s_openal_numChannels)
		return;

	s_resamplerQuality->integer = ClampCvarInteger(0, alConfig.numResamplers, s_resamplerQuality->integer); // clamp

	// Set up listener
	alListenerfv (AL_POSITION, listener_position);
	alListenerfv (AL_VELOCITY, velocity);
	alListenerfv (AL_ORIENTATION, orientation);

//	if ((CL_PMpointcontents (listener_position) & MASK_WATER))
//		alSpeedOfSound (47197);
//	else
//		alSpeedOfSound (10976);


	if (alConfig.efx)
		EFX_RvbUpdate (listener_position);

	memset (Channels_TODO, 0, sizeof(Channels_TODO));

	// Add looping sounds
	if (!(cls.state != ca_active || cl_paused->value)) {
		memset (cl_parse_entities_goodjob, 0, MAX_PARSE_ENTITIES);

		for (i = cl.frame.parse_entities;
			i < cl.frame.num_entities + cl.frame.parse_entities; i++) {
			int idx = i & (MAX_PARSE_ENTITIES - 1);
			ALuint sfx_numm;
			ent = &cl_parse_entities[idx];
			if (!ent->sound) {
				cl_parse_entities_goodjob[idx] = 1;
				continue;		// No sound effect
			}

			sfx_numm = cl.sound_precache[ent->sound];
			if (!sfx_numm) {
				cl_parse_entities_goodjob[idx] = 1;
				continue;		// Bad sound effect
			}
			// If this entity is already playing the same sound effect on
			// an
			// active channel, then simply update it
			for (j = 0, ch = s_openal_channels; j < s_openal_numChannels;
				j++, ch++) {
				//              if (ch->sfx != sfx) continue;
				if (ch->bufferNum != sfx_numm)
					continue;

				if (ch->entNum != ent->number)
					continue;

				if (!FlagAL_check (ch, AL_FLAGS_AL_LOOPING))
					continue;

				Flag_set (&Channels_TODO[j],
					AL_TASK_MANAGER__IS_LOOP_ACTIVE);

				{
					vec3_t position;
					vec3_t delta;
					CL_GetEntityOrigin (ch->entNum, position);

					VectorSubtract (position, ch->_AL_POSITION, delta);
					VectorScale (delta, 1 / 30, delta);

					alSourcefv (source_name[j], AL_VELOCITY, delta);
					alSourcefv (source_name[j], AL_POSITION, position);
					// memcpy(Channels_TODO[j].TASK_AL_VELOCITY, delta,
					// sizeof(vec3_t));
					memcpy (ch->_AL_POSITION, position, sizeof(vec3_t));
				}

				cl_parse_entities_goodjob[idx] = 1;

				break;			// keep channel alive.
			}
		}
	}
	// Check for stop, no-loops spatialization
	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		i++, ch++) {
		if (ch->bufferNum)		// (ch->sfx)
		{
			ALuint SourceState;

			alGetSourcei (source_name[i], AL_SOURCE_STATE, &SourceState);

			if (SourceState == AL_STOPPED || SourceState == AL_INITIAL)	// The
				// source
				// already
				// out
				// of
				// processing.
			{
				ch->bufferNum = 0;
				// alSourcei(ch->sourceNum, AL_BUFFER, 0);
			}
			else {
				// Kill some loops
				if (FlagAL_check (ch, AL_FLAGS_AL_LOOPING)) {
					if (!Flag_check
						(&Channels_TODO[i],
						AL_TASK_MANAGER__IS_LOOP_ACTIVE)) {
						TASK_TerminateChannel (&Channels_TODO[i], ch);
					}
				}
				else if (!FlagAL_check (ch, AL_FLAGS_FIXED_POSITION)) {
					vec3_t velocity, position;

					CL_GetEntityOrigin (ch->entNum, position);
					VectorSubtract (position, ch->_AL_POSITION, velocity);
					VectorScale (velocity, 1 / 30, velocity);

					alSourcefv (source_name[i], AL_VELOCITY, velocity);
					alSourcefv (source_name[i], AL_POSITION, position);
					memcpy (ch->_AL_POSITION, position, sizeof(vec3_t));
					// memcpy(Channels_TODO[i].TASK_AL_VELOCITY, velocity,
					// sizeof(vec3_t));
					// alSourcefv(ch->sourceNum, AL_DIRECTION, direction);
				}
			}
		}
	}

	if (!(cls.state != ca_active || cl_paused->value))
	for (i = cl.frame.parse_entities;
		i < cl.frame.num_entities + cl.frame.parse_entities; i++) {
		if (!cl_parse_entities_goodjob[i & (MAX_PARSE_ENTITIES - 1)]) {
			vec3_t position;

			ent = &cl_parse_entities[i & (MAX_PARSE_ENTITIES - 1)];

			// Pick a channel and start the sound effect
			CL_GetEntityOrigin (ent->number, position);


			//          ????? ps->entnum, ps->entchannel ?????
			if ((ch = PickChannel (Channels_TODO, ent->number, 0, position,
				&logical_channel_index, listener_position))) {
				channel_task_t *current_task = &Channels_TODO[logical_channel_index];

				ch->bufferNum = cl.sound_precache[ent->sound];
				ch->entNum = ent->number;	// loopNum

				// ch->loopSound = AL_TRUE;
				FlagAL_set (ch, AL_FLAGS_AL_LOOPING);
				FlagAL_clear (ch, AL_FLAGS_FIXED_POSITION);

				// ch->distanceMult = 0.3f;
				// Update min/max distance
				current_task->TASK_AL_REFERENCE_DISTANCE = 27;	// willow:
				// Player
				// unit
				// width
				// * 3
				// /
				// 4;
				// Set max distance really far away (default)
				// alSourcef(ch->sourceNum, AL_MAX_DISTANCE, 65536);

				// Update volume and rolloff factor from hacking
				// database, the only decent source :(
				current_task->TASK_AL_ROLLOFF_FACTOR =
					cl.sound_precache_rolloff_factor[ent->sound];
				current_task->TASK_AL_GAIN =
					cl.sound_precache_gain[ent->sound];

				//              alSourcefv(ch->sourceNum, AL_DIRECTION, ent->angles);
				{
					vec3_t delta, delta2;
					VectorSubtract (position, ent->old_origin, delta2);
					VectorScale (delta2, 1 / 30, delta);
					memcpy (current_task->TASK_AL_VELOCITY, delta,
						sizeof(vec3_t));
					// memcpy(current_task->TASK_AL_VELOCITY,
					// ent->delta, sizeof(vec3_t));
				}

				//              current_task->TASK_AL_SOURCE_RELATIVE = AL_FALSE;
				Flag_set (current_task, AL_TASK_MANAGER__EXECUTE);
			}
		}
	}
	// Issue playSounds
	for (;;) {
		ps = s_pendingplays.next;

		if (ps == &s_pendingplays)
			break;				// No more pending playSounds

		if (ps->begin > cl.time)
			break;				// No more pending playSounds this frame

		if (ps->fixed_origin)
			ch = PickChannel (Channels_TODO, ps->entnum, ps->entchannel,
			ps->origin, &logical_channel_index,
			listener_position);
		else {
			vec3_t position;
			CL_GetEntityOrigin (ps->entnum, position);
			ch = PickChannel (Channels_TODO, ps->entnum, ps->entchannel,
				position, &logical_channel_index,
				listener_position);
		}


		if (ch) {
			channel_task_t *current_task =
				&Channels_TODO[logical_channel_index];

			// VectorCopy(ps->origin, ch->position);
			ch->bufferNum = ps->bufferNum;	// ch->sfx = ps->sfx;


			{
				// Update min/max distance
				current_task->TASK_AL_REFERENCE_DISTANCE = 28;	// 34;
				// Set max distance really far away (default)
				// alSourcef(ch->sourceNum, AL_MAX_DISTANCE, 65536);

				current_task->TASK_AL_ROLLOFF_FACTOR = ps->attenuation;
				current_task->TASK_AL_GAIN = ps->volume;

				// SpatializeChannel(ch, ps->origin);
				// memcpy(ch->_AL_POSITION, ps->origin, sizeof(vec3_t));
				// //alSourcefv(ch->sourceNum, AL_POSITION, ps->origin);
				// //ch->position);
				// alSource3f(ch->sourceNum, AL_VELOCITY, 0, 0, 0);

				if (ps->fixed_origin) {
					FlagAL_clear (ch, AL_FLAGS_AL_LOOPING);
					FlagAL_set (ch, AL_FLAGS_FIXED_POSITION);
					memset (current_task->TASK_AL_VELOCITY, 0,
						sizeof(vec3_t));
				}
				else {
					FlagAL_clear (ch,
						AL_FLAGS_FIXED_POSITION |
						AL_FLAGS_AL_LOOPING);
					// willow: TO DO.
					memset (current_task->TASK_AL_VELOCITY, 0,
						sizeof(vec3_t));
				}

				//              current_task->TASK_AL_SOURCE_RELATIVE = AL_FALSE;
			}
			Flag_set (current_task, AL_TASK_MANAGER__EXECUTE);
		}
		// Free the playSound
		S_FreePlaysound (ps);
	}

	// Direct access to OpenAL layer
	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		i++, ch++) {
		channel_task_t *current_task = &Channels_TODO[i];

		if (Flag_check (current_task, AL_TASK_MANAGER__TERMINATE)) {
			alSourceStop (source_name[i]);
		}

		if (Flag_check (current_task, AL_TASK_MANAGER__EXECUTE)) {
			ALuint sourceNum = source_name[i];
			alSourcef (sourceNum, AL_PITCH, 1);
			alSource3f (sourceNum, AL_DIRECTION, 0, 0, 0);
			alSourcef (sourceNum, AL_REFERENCE_DISTANCE, current_task->TASK_AL_REFERENCE_DISTANCE);
			alSourcef (sourceNum, AL_ROLLOFF_FACTOR, current_task->TASK_AL_ROLLOFF_FACTOR);
			alSourcefv (sourceNum, AL_POSITION, ch->_AL_POSITION);
			alSourcefv (sourceNum, AL_VELOCITY,	current_task->TASK_AL_VELOCITY);
			alSourcei (sourceNum, AL_BUFFER, ch->bufferNum);
			alSourcei (sourceNum, AL_SOURCE_RELATIVE, FlagAL_checkAL (ch, AL_FLAGS_FLAT2D));	// Flag_checkAL
			// (current_task,
			// AL_TASK_MANAGER__IS_SOURCE_RELATIVE));
			alSourcei (sourceNum, AL_LOOPING, FlagAL_checkAL (ch, AL_FLAGS_AL_LOOPING));	// ch->loopSound);
			alSourcef (sourceNum, AL_GAIN, /*current_task->TASK_AL_GAIN*/ s_fxVolume->value);
			alSourcei(sourceNum, AL_SOURCE_RESAMPLER_SOFT, s_resamplerQuality->integer);
			
			if (alConfig.efx)
				EFX_RvbProcSrc (ch, sourceNum, qtrue);

			alSourcePlay (sourceNum);
		}
	}

	S_Streaming_RecycleBuffers ();
}

/*
===============================================================================
Music Streaming
===============================================================================
*/

qboolean S_Streaming_Start (int num_bits, int num_channels, ALsizei rate, float volume) {
	
	if (!s_initSound->integer)
		return qfalse;

	if (streaming.enabled) {
		Com_Printf (S_COLOR_YELLOW "S_Streaming_Start: interrupting active stream\n");
		S_Streaming_Stop ();
	}

	if (num_bits == 8 && num_channels == 1)
		streaming.sound_format = AL_FORMAT_MONO8;
	else if (num_bits == 8 && num_channels == 2)
		streaming.sound_format = AL_FORMAT_STEREO8;
	else if (num_bits == 16 && num_channels == 1)
		streaming.sound_format = AL_FORMAT_MONO16;
	else if (num_bits == 16 && num_channels == 2)
		streaming.sound_format = AL_FORMAT_STEREO16;
	else {
		Com_Printf (S_COLOR_RED "S_StreamingStart: unsupported format (%d bits and %d channels)\n", num_bits, num_channels);
		return qfalse;
	}

	alSourcef (source_name[CH_STREAMING], AL_GAIN, volume);
	streaming.sound_rate = rate;
	streaming.bFirst = 0;
	streaming.bNumAvail = NUM_STRBUF;
	streaming.enabled = qtrue;

	return qtrue;
}

void S_Streaming_Stop (void) {
	if (streaming.enabled) {
		// Stop the Source and clear the Queue
		alSourceStop (source_name[CH_STREAMING]);
		alSourcei (source_name[CH_STREAMING], AL_BUFFER, 0);

		streaming.enabled = qfalse;
	}
}

int S_Streaming_NumFreeBufs (void) {
	return streaming.bNumAvail;
}

static void S_Streaming_RecycleBuffers (void) {
	// Reclaim buffers that have been played, and keep them in a queue
	if (streaming.bNumAvail < NUM_STRBUF) {
		int recycled, i;
		alGetSourcei (source_name[CH_STREAMING], AL_BUFFERS_PROCESSED, &recycled);

		for (i = 0; i < recycled; i++) {
			ALuint buf;
			alSourceUnqueueBuffers (source_name[CH_STREAMING], 1, &buf);
			sq_add (buf);
		}
	}


}

int S_Streaming_Add (const byte *buffer, int num_bytes) {
	// TODO: add comments

	ALint iState;
	int readacc = 0;

	if (!streaming.enabled) {
		Com_DPrintf ("S_Streaming_Add: called with %d bytes when disabled\n", num_bytes);
		return 0;
	}

	if (num_bytes == 0) {
		Com_DPrintf ("S_Streaming_Add: called with empty buffer\n");
		return 0;
	}

	// If there is data to play and free buffers,
	// copy the data and enqueue them.
	while (num_bytes > 0 && streaming.bNumAvail > 0) {
		int readcur = MIN (MAX_STRBUF_SIZE, num_bytes);
		const ALuint buf = sq_remove ();

		alBufferData (buf, streaming.sound_format,
			buffer + readacc, readcur, streaming.sound_rate);
		alSourceQueueBuffers (source_name[CH_STREAMING], 1, &buf);

		num_bytes -= readcur;
		readacc += readcur;
	}

	// Check if we have stopped or starved and should restart
	// (the latter includes the case right afer initialization)
	alGetSourcei (source_name[CH_STREAMING], AL_SOURCE_STATE, &iState);
	if (iState != AL_PLAYING) {
		ALint iQueuedBuffers;
		alGetSourcei (source_name[CH_STREAMING], AL_BUFFERS_QUEUED, &iQueuedBuffers);

		if (iQueuedBuffers > 0) {
			// If we are not playing but there are still enqueued buffers,
			// restart the audio because it starved of data to play
			alSourcePlay (source_name[CH_STREAMING]);
			Com_DPrintf ("S_Streaming_Add: restarting stream, starved\n");
		}
		else {
			// Finished playing
			Com_DPrintf ("S_Streaming_Add: no more buffers to play, stopping\n");
			S_Streaming_Stop ();
		}
	}

	if (readacc == 0)
		Com_DPrintf ("S_Streaming_Add: added 0 samples to queue but %d given\n", num_bytes);

	return readacc;
}
