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
// cl_fx.c -- entity effects parsing and management

#include "client.h"



vec3_t cl_indexPalette[256];
game_export_t *ge;
void CL_BrassShells (vec3_t org, vec3_t dir, int count, qboolean mshell);
void CL_LogoutEffect (vec3_t org, int type);
void CL_ItemRespawnParticles (vec3_t org);



extern struct model_s *cl_mod_smoke;
extern struct model_s *cl_mod_flash;

void CL_ParticleSmoke (vec3_t org, vec3_t dir, int count);

//=================================================


/*
==============================================================

LIGHT STYLE MANAGEMENT

==============================================================
*/

typedef struct {
	int length;
	float value[3];
	float map[MAX_QPATH];
} clightstyle_t;

clightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
int lastofs;

/*
================
CL_ClearLightStyles
================
*/
void CL_ClearLightStyles (void) {
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	lastofs = -1;
}

/*
================
CL_RunLightStyles
================
*/
void CL_RunLightStyles (void) {
	int ofs;
	int i;
	clightstyle_t *ls;

	ofs = cl.time / 100;
	if (ofs == lastofs)
		return;
	lastofs = ofs;

	for (i = 0, ls = cl_lightstyle; i < MAX_LIGHTSTYLES; i++, ls++) {
		if (!ls->length) {
			ls->value[0] = ls->value[1] = ls->value[2] = 1.0;
			continue;
		}
		if (ls->length == 1)
			ls->value[0] = ls->value[1] = ls->value[2] = ls->map[0];
		else
			ls->value[0] = ls->value[1] = ls->value[2] = ls->map[ofs % ls->length];
	}
}


void CL_SetLightstyle (int i) {
	int j, k;
	char *s = cl.configstrings[i + CS_LIGHTS];

	j = strlen (s);
	if (j >= MAX_QPATH)
		Com_Error (ERR_DROP, "svc_lightstyle length=%i", j);

	cl_lightstyle[i].length = j;

	for (k = 0; k < j; k++)
		cl_lightstyle[i].map[k] =
		(float)(s[k] - 'a') / (float)('m' - 'a');
}

/*
================
CL_AddLightStyles
================
*/
void CL_AddLightStyles (void) {
	int i;
	clightstyle_t *ls;

	for (i = 0, ls = cl_lightstyle; i < MAX_LIGHTSTYLES; i++, ls++)
		V_AddLightStyle (i, ls->value[0], ls->value[1], ls->value[2]);
}

/*
==============================================================

DLIGHT MANAGEMENT

==============================================================
*/

cdlight_t cl_dlights[MAX_DLIGHTS];

/*
================
CL_ClearDlights
================
*/
void CL_ClearDlights (void) {
	memset (cl_dlights, 0, sizeof(cl_dlights));
}

/*
===============
CL_AllocDlight

===============
*/
cdlight_t *CL_AllocDlight (int key) {
	int i;
	cdlight_t *dl;

	// first look for an exact key match
	if (key) {
		dl = cl_dlights;
		for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
			if (dl->key == key) {
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}
	// then look for anything else
	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
		if (dl->die < cl.time) {
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}

/*
===============
CL_NewDlight
===============
*/



void CL_NewDlight (int key, vec3_t org, float r, float g, float b,
	float radius, float time) {
	cdlight_t *dl;

	dl = CL_AllocDlight (key);
	dl->origin[0] = org[0];
	dl->origin[1] = org[1];
	dl->origin[2] = org[2];
	dl->color[0] = r;
	dl->color[0] = g;
	dl->color[0] = b;
	dl->radius = radius;
	dl->die = cl.time + time;
}


/*
===============
CL_RunDLights

===============
*/
void CL_RunDLights (void) {
	int i;
	cdlight_t *dl;

	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
		if (!dl->radius)
			continue;

		if (dl->die < cl.time) {
			dl->radius = 0;
			return;
		}
		dl->radius -= cls.frametime * dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}

/*
==============
CL_ParseMuzzleFlash
==============
*/

// willow: Fast sound sfx library!
// Resource names
char *fastsound_name[FAST_SOUNDS_TOTAL] = {
	"weapons/blastf1a.wav",		// 0
	"weapons/hyprbf1a.wav",		// 1
	"weapons/machgf1b.wav",		// 2
	"weapons/machgf2b.wav",		// 3
	"weapons/machgf3b.wav",		// 4
	"weapons/machgf4b.wav",		// 5
	"weapons/machgf5b.wav",		// 6
	"weapons/shotgf1b.wav",		// 7
	"weapons/shotgr1b.wav",		// 8
	"weapons/sshotf1b.wav",		// 9
	"weapons/railgf1a.wav",		// 10
	"weapons/rocklf1a.wav",		// 11
	"weapons/rocklr1b.wav",		// 12
	"weapons/grenlf1a.wav",		// 13
	"weapons/grenlr1b.wav",		// 14
	"weapons/bfg__f1y.wav",		// 15
	"misc/talk.wav",			// 16
	"misc/menu1.wav",			// 17 (menu_in_sound)
	"misc/menu2.wav",			// 18 (menu_move_sound)
	"misc/menu3.wav",			// 19 (menu_out_sound)
	"world/ric1.wav",			// 20
	"world/ric2.wav",			// 21
	"world/ric3.wav",			// 22
	"weapons/lashit.wav",		// 23
	"world/spark5.wav",			// 24
	"world/spark6.wav",			// 25
	"world/spark7.wav",			// 26
	"weapons/railgr1a.wav",		// 27
	"weapons/rocklx1a.wav",		// 28
	"weapons/grenlx1a.wav",		// 29
	"weapons/xpld_wat.wav",		// 30
	"world/lava1.wav",			// 31
	"misc/brass_shell.wav",		// 32
	"misc/debris.wav",			// 33
	"player/step1.wav",			// 34
	"player/step2.wav",			// 35
	"player/step3.wav",			// 36
	"player/step4.wav",			// 37
	"weapons/tesla.wav",		// 38
	"weapons/disrupthit.wav"	// 39
};

// Resource descriptors
ALuint fastsound_descriptor[FAST_SOUNDS_TOTAL];

void S_fastsound_load (char *input_name, ALuint bufferNum);
void S_fastsound_get_descriptors_pool (unsigned count,
	ALuint * descriptors_pool);
void S_fastsound_kill_descriptors_pool (unsigned count,
	ALuint * descriptors_pool);
void S_fastsound_queue (vec3_t origin, int entnum, int entchannel,
	ALuint bufferNum, float fvol, float attenuation,
	unsigned timeofs);
void S_fastsound (vec3_t origin, int entnum, int entchannel,
	ALuint bufferNum, float fvol, float attenuation);

ALuint cl_sfx_idlogo;

void CL_fast_sound_init (void) {
	extern ALuint cl_sfx_lashit, cl_sfx_railg, cl_sfx_rockexp,
		cl_sfx_grenexp, cl_sfx_watrexp, cl_sfx_lightning, cl_sfx_disrexp;

	int i = FAST_SOUNDS_TOTAL;
	// Register waves
	S_fastsound_get_descriptors_pool (FAST_SOUNDS_TOTAL,
		fastsound_descriptor);
	while (i--) {
		// Actually load waves them from disk
		S_fastsound_load (fastsound_name[i], fastsound_descriptor[i]);
	}

	// PMM - version stuff
	// Com_Printf ("%s\n", ROGUE_VERSION_STRING);

	// individual access short pathes
	// PMM
	cl_sfx_lashit = fastsound_descriptor[id_cl_sfx_lashit];
	cl_sfx_railg = fastsound_descriptor[id_cl_sfx_railg];
	cl_sfx_rockexp = fastsound_descriptor[id_cl_sfx_rockexp];
	cl_sfx_grenexp = fastsound_descriptor[id_cl_sfx_grenexp];

	// q2xp
	cl_sfx_lava = fastsound_descriptor[id_cl_sfx_lava];
	cl_sfx_shell = fastsound_descriptor[id_cl_sfx_shell];
	cl_sfx_debris = fastsound_descriptor[id_cl_sfx_debris];

	// RAFAEL
	// cl_sfx_plasexp = S_RegisterSound ("weapons/plasexpl.wav");
	// willow: i have no solution for sexual flowered sounds now...
	// S_RegisterSound ("player/land1.wav");
	// S_RegisterSound ("player/fall2.wav");
	// S_RegisterSound ("player/fall1.wav");

	//PGM
	cl_sfx_lightning = fastsound_descriptor[id_cl_sfx_lightning];
	cl_sfx_disrexp = fastsound_descriptor[id_cl_sfx_disrexp];
	// version stuff - willow: some ID's garbage? Should i just clean up this? I bet i can.
	//  sprintf (name, "weapons/sound%d.wav", ROGUE_VERSION_ID);
	//  if (name[0] == 'w')
	//      name[0] = 'W';
	//PGM
}

void CL_fast_sound_close (void) {
	// Kill and free the waves
	S_fastsound_kill_descriptors_pool (FAST_SOUNDS_TOTAL,
		fastsound_descriptor);
}

#define		MONSTERS_ATTACK_ATTENUATION		0.1

void CL_ParseMuzzleFlash (void) {
	vec3_t fv, rv, smoke_origin, shell_brass, dir;
	cdlight_t *dl;
	int i, weapon, j;
	vec3_t _forward, _right, _up;

	centity_t *pl;
	int silenced;
	float volume;
	vec3_t up;
	extern cvar_t *hand;

	i = MSG_ReadShort (&net_message);

	if (i < 1 || i >= MAX_EDICTS)
		// Com_Error (ERR_DROP, "CL_ParseMuzzleFlash: bad entity");
		Com_Error (ERR_DROP, "CL_ParseMuzzleFlash: bad entity - %i", i);

	weapon = MSG_ReadByte (&net_message);

	silenced = weapon & MZ_SILENCED;
	weapon &= ~MZ_SILENCED;

	pl = &cl_entities[i];

	dl = CL_AllocDlight (i);
	
	AngleVectors(cl.refdef.viewangles, _forward, _right, _up);

	if (cl.playernum == i - 1 && !cl_thirdPerson->value) { //local player w/o third person view
		VectorCopy(smoke_puff, dl->origin);
		VectorCopy (smoke_puff, smoke_origin);
		VectorCopy (cl.refdef.vieworg, shell_brass);
		VectorMA(shell_brass, -3, _up, shell_brass);
		
		if(hand->integer == 1)
			VectorMA(shell_brass, -23, _right, shell_brass); // left
		else
			VectorMA(shell_brass, 23, _right, shell_brass); // right and center

		VectorMA(shell_brass, 21, _forward, shell_brass);
	}
	else {
		VectorMA (pl->current.origin, 10, fv, shell_brass);
		VectorMA (shell_brass, 6, rv, shell_brass);
		VectorMA (shell_brass, 21, up, shell_brass);

		VectorMA (pl->current.origin, 20, fv, smoke_origin);
		VectorMA (smoke_origin, 5, rv, smoke_origin);
		VectorMA (smoke_origin, 18, up, smoke_origin);
	}

	if (silenced)
		dl->radius = 100 + (rand () & 31);
	else
		dl->radius = 200 + (rand () & 31);
	dl->minlight = 32;
	dl->die = cl.time;			// + 0.1;

	if (silenced)
		volume = 0.2;
	else
		volume = 1;

	if (hand->integer == 1) {
		for (j = 0; j < 3; j++)
			dir[j] = -_forward[j] * 0.5 + -_right[j] + _up[j] * 2.5; // left
	}
	else {
		for (j = 0; j < 3; j++)
			dir[j] = -_forward[j] * 0.5 + _right[j] + _up[j] * 2.5; // right and centre
	}

	switch (weapon) {
		case MZ_BLASTER:
			dl->color[0] = 1;
			dl->color[1] = 0.7;
			dl->color[2] = 0;

			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_blastf1a], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_BLUEHYPERBLASTER:
			dl->color[0] = 0;
			dl->color[1] = 0;
			dl->color[2] = 1;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_hyprbf1a], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_HYPERBLASTER:
			dl->color[0] = 1;
			dl->color[1] = 0.7;
			dl->color[2] = 0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_hyprbf1a], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_MACHINEGUN:
			dl->color[0] = 1;
			dl->color[1] = 0.6;
			dl->color[2] = 0.4;
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 1);
			CL_BrassShells (shell_brass, dir, 1, qtrue);
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b + (rand () % 5)], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_SHOTGUN:
			dl->color[0] = 1;
			dl->color[1] = 0.6;
			dl->color[2] = 0.4;
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 4);
			CL_BrassShells (shell_brass, dir, 1, qfalse);
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_shotgf1b], volume, ATTN_WEAPON_LIGHT);
			S_fastsound_queue (smoke_origin, i, CHAN_AUTO,
				fastsound_descriptor[weapons_shotgr1b],
				volume * 0.5, ATTN_WEAPON_LIGHT, 100);
			break;
		case MZ_SSHOTGUN:
			dl->color[0] = 1;
			dl->color[1] = 0.6;
			dl->color[2] = 0.4;
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 6);
			CL_BrassShells (shell_brass, dir, 2, qfalse);
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_sshotf1b], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_CHAINGUN1:
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 1);
			CL_BrassShells (shell_brass, dir, 1, qtrue);
			dl->radius = 200 + (rand () & 31);
			dl->color[0] = 1;
			dl->color[1] = 0.7;
			dl->color[2] = 0.5;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b + (rand () % 5)], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_CHAINGUN2:
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 1);
			CL_BrassShells (shell_brass, dir, 1, qtrue);
			dl->radius = 225 + (rand () & 31);
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.4;
			dl->die = cl.time + 0.1;	// long delay
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b + (rand () % 5)], volume, ATTN_WEAPON_LIGHT);
			S_fastsound_queue (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b +(rand () % 5)], volume,	ATTN_WEAPON_LIGHT, 50);
			break;
		case MZ_CHAINGUN3:
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 1);
			CL_BrassShells (shell_brass, dir, 1, qtrue);
			dl->radius = 250 + (rand () & 31);
			dl->color[0] = 1;
			dl->color[1] = 0.6;
			dl->color[2] = 0.4;
			dl->die = cl.time + 0.1;	// long delay
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b + (rand () % 5)], volume, ATTN_WEAPON_LIGHT);
			S_fastsound_queue (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b +(rand () % 5)], volume, ATTN_WEAPON_LIGHT, 33);
			S_fastsound_queue (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_machgf1b + (rand () % 5)], volume, ATTN_WEAPON_LIGHT, 66);
			break;
		case MZ_RAILGUN:
			dl->color[0] = 0.5;
			dl->color[1] = 0.5;
			dl->color[2] = 1.0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_railgf1a], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_ROCKET:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.2;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_rocklf1a], volume, ATTN_WEAPON_HEAVY);
			S_fastsound_queue (NULL, i, CHAN_AUTO, fastsound_descriptor[weapons_rocklr1b], volume, ATTN_WEAPON_HEAVY, 150);
			CL_ParticleGunSmoke (smoke_origin, vec3_origin, 8);
			break;
		case MZ_GRENADE:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_grenlf1a], volume, ATTN_WEAPON_LIGHT);
			S_fastsound_queue (NULL, i, CHAN_AUTO, fastsound_descriptor[weapons_grenlr1b], volume, ATTN_WEAPON_LIGHT, 100);
			break;
		case MZ_BFG:
			dl->color[0] = 0;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_bfg__f1y], volume, ATTN_WEAPON_HEAVY);
			break;

		case MZ_LOGIN:
			dl->color[0] = 0;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = cl.time + 1.0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_grenlf1a], 1.0, ATTN_MEDIUM);
			CL_LogoutEffect (pl->current.origin, weapon);
			break;
		case MZ_LOGOUT:
			dl->color[0] = 1;
			dl->color[1] = 0;
			dl->color[2] = 0;
			dl->die = cl.time + 1.0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_grenlf1a], 1.0, ATTN_MEDIUM);
			CL_LogoutEffect (pl->current.origin, weapon);
			break;
		case MZ_RESPAWN:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = cl.time + 1.0;
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_grenlf1a], 1.0, ATTN_MEDIUM);
			CL_LogoutEffect (pl->current.origin, weapon);
			break;
			// RAFAEL
		case MZ_PHALANX:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.5;
			S_fastsound (NULL, i, CHAN_WEAPON, S_RegisterSound ("weapons/plasshot.wav"), volume, ATTN_WEAPON_HEAVY);
			break;
			// RAFAEL
		case MZ_IONRIPPER:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.5;
			S_fastsound (NULL, i, CHAN_WEAPON, S_RegisterSound ("weapons/rippfire.wav"), volume, ATTN_WEAPON_LIGHT);
			break;

			// ======================
			// PGM
		case MZ_ETF_RIFLE:
			dl->color[0] = 0.9;
			dl->color[1] = 0.7;
			dl->color[2] = 0;
			S_fastsound (NULL, i, CHAN_WEAPON, S_RegisterSound ("weapons/nail1.wav"), volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_SHOTGUN2:
			dl->color[0] = 1;
			dl->color[1] = 0.7;
			dl->color[2] = 0.5;
			S_fastsound (NULL, i, CHAN_WEAPON,	S_RegisterSound ("weapons/shotg2.wav"), volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_HEATBEAM:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = cl.time + 100;
			 S_fastsound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/bfg__l1a.wav"), volume, ATTN_NORM);
			break;
		case MZ_BLASTER2:
			dl->color[0] = 0;
			dl->color[1] = 1;
			dl->color[2] = 0;
			// FIXME - different sound for blaster2 ??
			// FIXME - willow: volume 0.75
			S_fastsound (NULL, i, CHAN_WEAPON, fastsound_descriptor[weapons_blastf1a], volume, ATTN_WEAPON_LIGHT);
			break;
		case MZ_TRACKER:
			// negative flashes handled the same in gl/soft until
			// CL_AddDLights
			dl->color[0] = -1;
			dl->color[1] = -1;
			dl->color[2] = -1;
			S_fastsound (NULL, i, CHAN_WEAPON, S_RegisterSound ("weapons/disint2.wav"), volume, ATTN_MEDIUM);
			break;
		case MZ_NUKE1:
			dl->color[0] = 1;
			dl->color[1] = 0;
			dl->color[2] = 0;
			dl->die = cl.time + 100;
			break;
		case MZ_NUKE2:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = cl.time + 100;
			break;
		case MZ_NUKE4:
			dl->color[0] = 0;
			dl->color[1] = 0;
			dl->color[2] = 1;
			dl->die = cl.time + 100;
			break;
		case MZ_NUKE8:
			dl->color[0] = 0;
			dl->color[1] = 1;
			dl->color[2] = 1;
			dl->die = cl.time + 100;
			break;
			// PGM
			// ======================
	}
}

void CL_GunFire (vec3_t start, vec3_t end);

/*
==============
CL_ParseMuzzleFlash2
Monsters
==============
*/
void CL_ParseMuzzleFlash2 (void) {
	int ent, j;
	vec3_t origin, dlorg;
	int flash_number;
	cdlight_t *dl;
	vec3_t forward, right, up, dir, shell, end;
	char soundname[64];

	ent = MSG_ReadShort (&net_message);
	if (ent < 1 || ent >= MAX_EDICTS)
		Com_Error (ERR_DROP, "CL_ParseMuzzleFlash2: bad entity");

	flash_number = MSG_ReadByte (&net_message);

	// locate the origin
	AngleVectors (cl_entities[ent].current.angles, forward, right, up);
	origin[0] =
		cl_entities[ent].current.origin[0] +
		forward[0] * monster_flash_offset[flash_number][0] +
		right[0] * monster_flash_offset[flash_number][1];
	origin[1] =
		cl_entities[ent].current.origin[1] +
		forward[1] * monster_flash_offset[flash_number][0] +
		right[1] * monster_flash_offset[flash_number][1];
	origin[2] =
		cl_entities[ent].current.origin[2] +
		forward[2] * monster_flash_offset[flash_number][0] +
		right[2] * monster_flash_offset[flash_number][1] +
		monster_flash_offset[flash_number][2];

	VectorMA (origin, -5, forward, shell);
	VectorMA (origin, 15, forward, dlorg);
	VectorMA (dlorg, 35, up, dlorg);
	VectorMA (origin, 15, forward, end);

	for (j = 0; j < 3; j++)
		dir[j] = forward[j] + right[j] + up[j] * 3;


	dl = CL_AllocDlight (ent);
	VectorCopy (dlorg, dl->origin);
	dl->radius = 200 + (rand () & 31);
	dl->minlight = 32;
	dl->die = cl.time + 0.1;			// + 0.1;

	switch (flash_number) {
		case MZ2_INFANTRY_MACHINEGUN_1:
		case MZ2_INFANTRY_MACHINEGUN_2:
		case MZ2_INFANTRY_MACHINEGUN_3:
		case MZ2_INFANTRY_MACHINEGUN_4:
		case MZ2_INFANTRY_MACHINEGUN_5:
		case MZ2_INFANTRY_MACHINEGUN_6:
		case MZ2_INFANTRY_MACHINEGUN_7:
		case MZ2_INFANTRY_MACHINEGUN_8:
		case MZ2_INFANTRY_MACHINEGUN_9:
		case MZ2_INFANTRY_MACHINEGUN_10:
		case MZ2_INFANTRY_MACHINEGUN_11:
		case MZ2_INFANTRY_MACHINEGUN_12:
		case MZ2_INFANTRY_MACHINEGUN_13:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			CL_BrassShells (shell, dir, 1, qtrue);
			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);

			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("infantry/infatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_SOLDIER_MACHINEGUN_1:
		case MZ2_SOLDIER_MACHINEGUN_2:
		case MZ2_SOLDIER_MACHINEGUN_3:
		case MZ2_SOLDIER_MACHINEGUN_4:
		case MZ2_SOLDIER_MACHINEGUN_5:
		case MZ2_SOLDIER_MACHINEGUN_6:
		case MZ2_SOLDIER_MACHINEGUN_7:
		case MZ2_SOLDIER_MACHINEGUN_8:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			CL_BrassShells (shell, dir, 1, qtrue);
			CL_ParticleSmoke (origin, vec3_origin, 1);
			CL_GunFire (origin, end);
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("soldier/solatck3.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_GUNNER_MACHINEGUN_1:
		case MZ2_GUNNER_MACHINEGUN_2:
		case MZ2_GUNNER_MACHINEGUN_3:
		case MZ2_GUNNER_MACHINEGUN_4:
		case MZ2_GUNNER_MACHINEGUN_5:
		case MZ2_GUNNER_MACHINEGUN_6:
		case MZ2_GUNNER_MACHINEGUN_7:
		case MZ2_GUNNER_MACHINEGUN_8:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			CL_BrassShells (shell, dir, 1, qtrue);
			// CL_ParticleEffect (origin, vec3_origin, 0, 40);
			// CL_SmokeAndFlash(origin);

			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("gunner/gunatck2.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_ACTOR_MACHINEGUN_1:
		case MZ2_SUPERTANK_MACHINEGUN_1:
		case MZ2_SUPERTANK_MACHINEGUN_2:
		case MZ2_SUPERTANK_MACHINEGUN_3:
		case MZ2_SUPERTANK_MACHINEGUN_4:
		case MZ2_SUPERTANK_MACHINEGUN_5:
		case MZ2_SUPERTANK_MACHINEGUN_6:
		case MZ2_TURRET_MACHINEGUN:	// PGM
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			CL_BrassShells (shell, dir, 1, qtrue);
			// CL_ParticleEffect (origin, vec3_origin, 0, 40);
			// CL_SmokeAndFlash(origin);

			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);

			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("infantry/infatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_BOSS2_MACHINEGUN_L1:
		case MZ2_BOSS2_MACHINEGUN_L2:
		case MZ2_BOSS2_MACHINEGUN_L3:
		case MZ2_BOSS2_MACHINEGUN_L4:
		case MZ2_BOSS2_MACHINEGUN_L5:
		case MZ2_CARRIER_MACHINEGUN_L1:	// PMM
		case MZ2_CARRIER_MACHINEGUN_L2:	// PMM
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			CL_BrassShells (shell, dir, 1, qtrue);
			// CL_ParticleEffect (origin, vec3_origin, 0, 40);
			// CL_SmokeAndFlash(origin);

			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("infantry/infatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_SOLDIER_BLASTER_1:
		case MZ2_SOLDIER_BLASTER_2:
		case MZ2_SOLDIER_BLASTER_3:
		case MZ2_SOLDIER_BLASTER_4:
		case MZ2_SOLDIER_BLASTER_5:
		case MZ2_SOLDIER_BLASTER_6:
		case MZ2_SOLDIER_BLASTER_7:
		case MZ2_SOLDIER_BLASTER_8:
		case MZ2_TURRET_BLASTER:	// PGM
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("soldier/solatck2.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_FLYER_BLASTER_1:
		case MZ2_FLYER_BLASTER_2:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("flyer/flyatck3.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_MEDIC_BLASTER_1:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("medic/medatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_HOVER_BLASTER_1:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("hover/hovatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_FLOAT_BLASTER_1:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("floater/fltatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_SOLDIER_SHOTGUN_1:
		case MZ2_SOLDIER_SHOTGUN_2:
		case MZ2_SOLDIER_SHOTGUN_3:
		case MZ2_SOLDIER_SHOTGUN_4:
		case MZ2_SOLDIER_SHOTGUN_5:
		case MZ2_SOLDIER_SHOTGUN_6:
		case MZ2_SOLDIER_SHOTGUN_7:
		case MZ2_SOLDIER_SHOTGUN_8:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			// CL_SmokeAndFlash(origin);
			CL_BrassShells (shell, dir, 1, qfalse);
			CL_ParticleSmoke (origin, vec3_origin, 2);
			CL_GunFire (origin, end);
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("soldier/solatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_TANK_BLASTER_1:
		case MZ2_TANK_BLASTER_2:
		case MZ2_TANK_BLASTER_3:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("tank/tnkatck3.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_TANK_MACHINEGUN_1:
		case MZ2_TANK_MACHINEGUN_2:
		case MZ2_TANK_MACHINEGUN_3:
		case MZ2_TANK_MACHINEGUN_4:
		case MZ2_TANK_MACHINEGUN_5:
		case MZ2_TANK_MACHINEGUN_6:
		case MZ2_TANK_MACHINEGUN_7:
		case MZ2_TANK_MACHINEGUN_8:
		case MZ2_TANK_MACHINEGUN_9:
		case MZ2_TANK_MACHINEGUN_10:
		case MZ2_TANK_MACHINEGUN_11:
		case MZ2_TANK_MACHINEGUN_12:
		case MZ2_TANK_MACHINEGUN_13:
		case MZ2_TANK_MACHINEGUN_14:
		case MZ2_TANK_MACHINEGUN_15:
		case MZ2_TANK_MACHINEGUN_16:
		case MZ2_TANK_MACHINEGUN_17:
		case MZ2_TANK_MACHINEGUN_18:
		case MZ2_TANK_MACHINEGUN_19:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			// CL_ParticleEffect (origin, vec3_origin, 0, 40);
			// CL_SmokeAndFlash(origin);
			CL_BrassShells (shell, dir, 1, qtrue);
			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);
			Com_sprintf (soundname, sizeof(soundname), "tank/tnkatk2%c.wav",
				'a' + rand () % 5);
			S_fastsound (NULL, ent, CHAN_WEAPON, S_RegisterSound (soundname), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_CHICK_ROCKET_1:
		case MZ2_TURRET_ROCKET:	// PGM
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.2;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("chick/chkatck2.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_TANK_ROCKET_1:
		case MZ2_TANK_ROCKET_2:
		case MZ2_TANK_ROCKET_3:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.2;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("tank/tnkatck1.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_SUPERTANK_ROCKET_1:
		case MZ2_SUPERTANK_ROCKET_2:
		case MZ2_SUPERTANK_ROCKET_3:
		case MZ2_BOSS2_ROCKET_1:
		case MZ2_BOSS2_ROCKET_2:
		case MZ2_BOSS2_ROCKET_3:
		case MZ2_BOSS2_ROCKET_4:
		case MZ2_CARRIER_ROCKET_1:
			//  case MZ2_CARRIER_ROCKET_2:
			//  case MZ2_CARRIER_ROCKET_3:
			//  case MZ2_CARRIER_ROCKET_4:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.2;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("tank/rocket.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_GUNNER_GRENADE_1:
		case MZ2_GUNNER_GRENADE_2:
		case MZ2_GUNNER_GRENADE_3:
		case MZ2_GUNNER_GRENADE_4:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("gunner/gunatck3.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_GLADIATOR_RAILGUN_1:
			// PMM
		case MZ2_CARRIER_RAILGUN:
		case MZ2_WIDOW_RAIL:

			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("weapons/railgf1a.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			// pmm
			dl->color[0] = 0.5;
			dl->color[1] = 0.5;
			dl->color[2] = 1.0;
			break;

			// --- Xian's shit starts ---
		case MZ2_MAKRON_BFG:
			dl->color[0] = 0.5;
			dl->color[1] = 1;
			dl->color[2] = 0.5;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("makron/bfg_fire.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_MAKRON_BLASTER_1:
		case MZ2_MAKRON_BLASTER_2:
		case MZ2_MAKRON_BLASTER_3:
		case MZ2_MAKRON_BLASTER_4:
		case MZ2_MAKRON_BLASTER_5:
		case MZ2_MAKRON_BLASTER_6:
		case MZ2_MAKRON_BLASTER_7:
		case MZ2_MAKRON_BLASTER_8:
		case MZ2_MAKRON_BLASTER_9:
		case MZ2_MAKRON_BLASTER_10:
		case MZ2_MAKRON_BLASTER_11:
		case MZ2_MAKRON_BLASTER_12:
		case MZ2_MAKRON_BLASTER_13:
		case MZ2_MAKRON_BLASTER_14:
		case MZ2_MAKRON_BLASTER_15:
		case MZ2_MAKRON_BLASTER_16:
		case MZ2_MAKRON_BLASTER_17:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("makron/blaster.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_JORG_MACHINEGUN_L1:
		case MZ2_JORG_MACHINEGUN_L2:
		case MZ2_JORG_MACHINEGUN_L3:
		case MZ2_JORG_MACHINEGUN_L4:
		case MZ2_JORG_MACHINEGUN_L5:
		case MZ2_JORG_MACHINEGUN_L6:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			//      CL_ParticleEffect (origin, vec3_origin, 0, 40);
			//      CL_SmokeAndFlash(origin);
			CL_BrassShells (shell, dir, 1, qtrue);
			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("boss3/xfire.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_JORG_MACHINEGUN_R1:
		case MZ2_JORG_MACHINEGUN_R2:
		case MZ2_JORG_MACHINEGUN_R3:
		case MZ2_JORG_MACHINEGUN_R4:
		case MZ2_JORG_MACHINEGUN_R5:
		case MZ2_JORG_MACHINEGUN_R6:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			//      CL_ParticleEffect (origin, vec3_origin, 0, 40);
			//      CL_SmokeAndFlash(origin);
			CL_BrassShells (shell, dir, 1, qtrue);
			CL_ParticleSmoke (origin, vec3_origin, 3);
			CL_GunFire (origin, end);
			break;

		case MZ2_JORG_BFG_1:
			dl->color[0] = 0.5;
			dl->color[1] = 1;
			dl->color[2] = 0.5;
			break;

		case MZ2_BOSS2_MACHINEGUN_R1:
		case MZ2_BOSS2_MACHINEGUN_R2:
		case MZ2_BOSS2_MACHINEGUN_R3:
		case MZ2_BOSS2_MACHINEGUN_R4:
		case MZ2_BOSS2_MACHINEGUN_R5:
		case MZ2_CARRIER_MACHINEGUN_R1:	// PMM
		case MZ2_CARRIER_MACHINEGUN_R2:	// PMM

			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			CL_BrassShells (shell, dir, 1, qtrue);
			// CL_ParticleEffect (origin, vec3_origin, 0, 40);
			// CL_SmokeAndFlash(origin);

			CL_ParticleSmoke (origin, vec3_origin, 3);

			break;

			// ======
			// ROGUE
		case MZ2_STALKER_BLASTER:
		case MZ2_DAEDALUS_BLASTER:
		case MZ2_MEDIC_BLASTER_2:
		case MZ2_WIDOW_BLASTER:
		case MZ2_WIDOW_BLASTER_SWEEP1:
		case MZ2_WIDOW_BLASTER_SWEEP2:
		case MZ2_WIDOW_BLASTER_SWEEP3:
		case MZ2_WIDOW_BLASTER_SWEEP4:
		case MZ2_WIDOW_BLASTER_SWEEP5:
		case MZ2_WIDOW_BLASTER_SWEEP6:
		case MZ2_WIDOW_BLASTER_SWEEP7:
		case MZ2_WIDOW_BLASTER_SWEEP8:
		case MZ2_WIDOW_BLASTER_SWEEP9:
		case MZ2_WIDOW_BLASTER_100:
		case MZ2_WIDOW_BLASTER_90:
		case MZ2_WIDOW_BLASTER_80:
		case MZ2_WIDOW_BLASTER_70:
		case MZ2_WIDOW_BLASTER_60:
		case MZ2_WIDOW_BLASTER_50:
		case MZ2_WIDOW_BLASTER_40:
		case MZ2_WIDOW_BLASTER_30:
		case MZ2_WIDOW_BLASTER_20:
		case MZ2_WIDOW_BLASTER_10:
		case MZ2_WIDOW_BLASTER_0:
		case MZ2_WIDOW_BLASTER_10L:
		case MZ2_WIDOW_BLASTER_20L:
		case MZ2_WIDOW_BLASTER_30L:
		case MZ2_WIDOW_BLASTER_40L:
		case MZ2_WIDOW_BLASTER_50L:
		case MZ2_WIDOW_BLASTER_60L:
		case MZ2_WIDOW_BLASTER_70L:
		case MZ2_WIDOW_RUN_1:
		case MZ2_WIDOW_RUN_2:
		case MZ2_WIDOW_RUN_3:
		case MZ2_WIDOW_RUN_4:
		case MZ2_WIDOW_RUN_5:
		case MZ2_WIDOW_RUN_6:
		case MZ2_WIDOW_RUN_7:
		case MZ2_WIDOW_RUN_8:
			dl->color[0] = 0;
			dl->color[1] = 1;
			dl->color[2] = 0;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("tank/tnkatck3.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_WIDOW_DISRUPTOR:
			dl->color[0] = -1;
			dl->color[1] = -1;
			dl->color[2] = -1;
			S_fastsound (NULL, ent, CHAN_WEAPON,
				S_RegisterSound ("weapons/disint2.wav"), 1,
				MONSTERS_ATTACK_ATTENUATION);
			break;

		case MZ2_WIDOW_PLASMABEAM:
		case MZ2_WIDOW2_BEAMER_1:
		case MZ2_WIDOW2_BEAMER_2:
		case MZ2_WIDOW2_BEAMER_3:
		case MZ2_WIDOW2_BEAMER_4:
		case MZ2_WIDOW2_BEAMER_5:
		case MZ2_WIDOW2_BEAM_SWEEP_1:
		case MZ2_WIDOW2_BEAM_SWEEP_2:
		case MZ2_WIDOW2_BEAM_SWEEP_3:
		case MZ2_WIDOW2_BEAM_SWEEP_4:
		case MZ2_WIDOW2_BEAM_SWEEP_5:
		case MZ2_WIDOW2_BEAM_SWEEP_6:
		case MZ2_WIDOW2_BEAM_SWEEP_7:
		case MZ2_WIDOW2_BEAM_SWEEP_8:
		case MZ2_WIDOW2_BEAM_SWEEP_9:
		case MZ2_WIDOW2_BEAM_SWEEP_10:
		case MZ2_WIDOW2_BEAM_SWEEP_11:
			dl->radius = 300 + (rand () & 100);
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = cl.time + 200;
			break;
			// ROGUE
			// ======

			// --- Xian's shit ends ---

	}
}


/*
===============
CL_AddDLights

===============
*/
void CL_AddDLights (void) {
	int i;
	cdlight_t *dl;

	dl = cl_dlights;

	for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
		if (!dl->radius)
			continue;
		V_AddLight (dl->origin, dl->radius, dl->color[0], dl->color[1], dl->color[2], vec3_origin, 0, 0);
	}
}



/*
==============
CL_EntityEvent

An entity has just been parsed that has an event value

the female events are there for backwards compatability
==============
*/
void CL_EntityEvent (entity_state_t * ent) {
	switch (ent->event) {
		case EV_ITEM_RESPAWN:
			S_StartSound (NULL, ent->number, CHAN_WEAPON,
				S_RegisterSound ("items/respawn1.wav"), 1, ATTN_IDLE,
				0);
			CL_ItemRespawnParticles (ent->origin);
			break;
		case EV_PLAYER_TELEPORT:
			S_StartSound (NULL, ent->number, CHAN_WEAPON,
				S_RegisterSound ("misc/tele1.wav"), 1, ATTN_IDLE, 0);
			CL_TeleportParticles (ent->origin);
			break;
		case EV_FOOTSTEP:
			if (cl_footsteps->integer)
				S_fastsound (NULL, ent->number, CHAN_BODY,
				fastsound_descriptor[id_cl_sfx_footsteps_0 +
				(rand () & 3)], 0.5,
				ATTN_NORM);
			break;
		case EV_FALLSHORT:
			S_StartSound (NULL, ent->number, CHAN_AUTO,
				S_RegisterSound ("player/land1.wav"), 1, ATTN_NORM, 0);
			break;
		case EV_FALL:
			S_StartSound (NULL, ent->number, CHAN_AUTO,
				S_RegisterSexedSound (&cl_entities[ent->number].current, "*fall2.wav"), 1, ATTN_NORM, 0);
			//					S_RegisterSound("*fall2.wav"), 1, ATTN_NORM, 0);
			break;
		case EV_FALLFAR:
			S_StartSound (NULL, ent->number, CHAN_AUTO,
				S_RegisterSexedSound (&cl_entities[ent->number].current, "*fall1.wav"), 1, ATTN_NORM, 0);
			//					S_RegisterSound("*fall1.wav"), 1, ATTN_NORM, 0);
			break;
	}
}




/*
==============
CL_ClearEffects

==============
*/
void CL_ClearEffects (void) {
	CL_ClearParticles ();
	CL_ClearDecals ();
	CL_ClearDlights ();
	CL_ClearLightStyles ();
	CL_ClearClEntities ();
}
