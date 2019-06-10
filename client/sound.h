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

#ifdef _WIN32
//#include "AL/alc.h"
//#include "AL/altypes.h"
#include "AL/al.h"
#else
#include <AL/al.h>
#endif

#define FAST_SOUNDS_TOTAL 40
extern ALuint fastsound_descriptor[FAST_SOUNDS_TOTAL];

void S_Init (int hardreset);
void S_Shutdown (void);
void S_Restart (void);			// Soft audio hardware changes.


// if origin is NULL, the sound will be dynamically sourced from the entity
#define S_StartSound S_fastsound_queue
//void S_StartSound (vec3_t origin, int entnum, int entchannel, struct sfx_s *sfx, float fvol,  float attenuation, unsigned timeofs);
void S_fastsound_queue (vec3_t origin, int entnum, int entchannel,
	ALuint bufferNum, float fvol, float attenuation,
	unsigned timeofs);
void S_StartLocalSound (ALuint bufferNum);

void S_StopAllSounds (void);
void S_Update (vec3_t listener_position, vec3_t velocity,
	float orientation[6]);

ALuint S_RegisterSound (const char *sample);
ALuint S_RegisterSexedSound (entity_state_t * ent, const char *base);

ALuint S_FindName (char *name, qboolean create);
ALuint S_FindName_lite (char *input_name);

// the sound code makes callbacks to the client for entitiy position
// information, so entities can be dynamically re-spatialized
void CL_GetEntitySoundOrigin (int ent, vec3_t org);
