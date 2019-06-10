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
// cl_tent.c -- client side temporary entities

#include "client.h"
#include <math.h>

void CL_ParticleGibBlood2 (vec3_t org);

#define	MAX_EXPLOSIONS	32
explosion_t cl_explosions[MAX_EXPLOSIONS];

void CL_BfgBall(vec3_t org);
void CL_BfgExplosion(vec3_t org);

#define	MAX_BEAMS	32
typedef struct {
	int entity;
	int dest_entity;
	struct model_s *model;
	int endtime;
	vec3_t offset;
	vec3_t start, end;
} beam_t;
beam_t cl_beams[MAX_BEAMS];
//PMM - added this for player-linked beams.  Currently only used by the plasma beam
beam_t cl_playerbeams[MAX_BEAMS];

void CL_ClientGibs (vec3_t org, vec3_t velocity);

//ROGUE
cl_sustain_t cl_sustains[MAX_SUSTAINS];
//ROGUE

//PGM
extern void CL_TeleportParticles (vec3_t org);
//PGM

void CL_BlasterParticles (vec3_t org, vec3_t dir);
void CL_ExplosionParticles (vec3_t org);
void CL_BFGExplosionParticles (vec3_t org);
// RAFAEL
void CL_BlueBlasterParticles (vec3_t org, vec3_t dir);

ALuint cl_sfx_lashit;
ALuint cl_sfx_railg;
ALuint cl_sfx_rockexp;
ALuint cl_sfx_grenexp;
ALuint cl_sfx_watrexp;
ALuint cl_sfx_plasexp;

struct model_s *cl_mod_explode;
struct model_s *cl_mod_smoke;
struct model_s *cl_mod_flash;
struct model_s *cl_mod_parasite_segment;
struct model_s *cl_mod_grapple_cable;
struct model_s *cl_mod_parasite_tip;
struct model_s *cl_mod_explo4;
struct model_s *cl_mod_bfg_explo;
struct model_s *cl_mod_powerscreen;
struct model_s *cl_mod_distort;

// RAFAEL
struct model_s *cl_mod_plasmaexplo;

//ROGUE
ALuint cl_sfx_lightning;
ALuint cl_sfx_disrexp;
struct model_s *cl_mod_lightning;
struct model_s *cl_mod_heatbeam;
struct model_s *cl_mod_monster_heatbeam;
struct model_s *cl_mod_explo4_big;

//ROGUE
/*
=================
CL_RegisterTEntModels
=================
*/
void CL_RegisterTEntModels (void) {
	cl_mod_explode = R_RegisterModel ("models/objects/explode/tris.md2");
	cl_mod_smoke = R_RegisterModel ("models/objects/smoke/tris.md2");
	cl_mod_flash = R_RegisterModel ("models/objects/flash/tris.md2");
	cl_mod_parasite_segment = R_RegisterModel ("models/monsters/parasite/segment/tris.md2");
	cl_mod_grapple_cable = R_RegisterModel ("models/ctf/segment/tris.md2");
	cl_mod_parasite_tip = R_RegisterModel ("models/monsters/parasite/tip/tris.md2");

	// q2xp
	cl_mod_explo4 = R_RegisterModel ("models/misc/dst_model.md2");
	cl_mod_mshell = R_RegisterModel ("models/shells/m_shell.md2");
	cl_mod_sshell = R_RegisterModel ("models/shells/s_shell.md2");

	cl_mod_debris1 = R_RegisterModel ("models/objects/debris1/tris.md2");
	cl_mod_debris2 = R_RegisterModel ("models/objects/debris2/tris.md2");
	cl_mod_debris3 = R_RegisterModel ("models/objects/debris3/tris.md2");

	cl_mod_gib0 = R_RegisterModel ("models/objects/gibs/sm_meat/tris.md2");
	cl_mod_gib1 = R_RegisterModel ("models/objects/gibs/sm_metal/tris.md2");

	cl_mod_gib2 = R_RegisterModel ("models/objects/gibs/bone/tris.md2");
	cl_mod_gib3 = R_RegisterModel ("models/objects/gibs/bone2/tris.md2");
	cl_mod_gib4 = R_RegisterModel ("models/objects/gibs/chest/tris.md2");
	cl_mod_gib5 = R_RegisterModel ("models/objects/gibs/head2/tris.md2");

	cl_mod_distort = R_RegisterModel ("sprites/distort.sp2");
	cl_mod_bfg_explo = R_RegisterModel ("sprites/s_bfg2.sp2");
	cl_mod_powerscreen = R_RegisterModel ("models/items/armor/effect/tris.md2");

	R_RegisterModel ("models/objects/laser/tris.md2");
	R_RegisterModel ("models/objects/grenade2/tris.md2");

	//Precache all first person player guns
	//All items and world guns precache in 3d had

	R_RegisterModel ("models/weapons/v_bfg/tris.md2");
	R_RegisterModel ("models/weapons/v_blast/tris.md2");
	R_RegisterModel ("models/weapons/v_chain/tris.md2");
	R_RegisterModel ("models/weapons/v_handgr/tris.md2");
	R_RegisterModel ("models/weapons/v_hyperb/tris.md2");
	R_RegisterModel ("models/weapons/v_launch/tris.md2");
	R_RegisterModel ("models/weapons/v_machn/tris.md2");
	R_RegisterModel ("models/weapons/v_rail/tris.md2");
	R_RegisterModel ("models/weapons/v_rocket/tris.md2");
	R_RegisterModel ("models/weapons/v_shotg/tris.md2");
	R_RegisterModel ("models/weapons/v_shotg2/tris.md2");

	//Precache gibs
	R_RegisterModel ("models/objects/gibs/bone/tris.md2");
	R_RegisterModel ("models/objects/gibs/sm_meat/tris.md2");
	R_RegisterModel ("models/objects/gibs/bone2/tris.md2");

	Draw_FindPic ("w_machinegun");
	Draw_FindPic ("a_bullets");
	Draw_FindPic ("i_health");
	Draw_FindPic ("a_grenades");

	//ROGUE
	cl_mod_explo4_big = R_RegisterModel ("models/objects/r_explode2/tris.md2");
	cl_mod_lightning = R_RegisterModel ("models/proj/lightning/tris.md2");
	cl_mod_heatbeam = R_RegisterModel ("models/proj/beam/tris.md2");
	cl_mod_monster_heatbeam = R_RegisterModel ("models/proj/widowbeam/tris.md2");
	//ROGUE

	LoadHudEnts ();
}

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void) {
	memset (cl_beams, 0, sizeof(cl_beams));
	memset (cl_explosions, 0, sizeof(cl_explosions));
	memset (cl_lasers, 0, sizeof(cl_lasers));

	//ROGUE
	memset (cl_playerbeams, 0, sizeof(cl_playerbeams));
	memset (cl_sustains, 0, sizeof(cl_sustains));
	//ROGUE
}

/*
=================
CL_AllocExplosion
=================
*/
explosion_t *CL_AllocExplosion (void) {
	int i;
	int time;
	int index;

	for (i = 0; i < MAX_EXPLOSIONS; i++) {
		if (cl_explosions[i].type == ex_free) {
			memset (&cl_explosions[i], 0, sizeof(cl_explosions[i]));
			return &cl_explosions[i];
		}
	}
	// find the oldest explosion
	time = cl.time;
	index = 0;

	for (i = 0; i < MAX_EXPLOSIONS; i++)
	if (cl_explosions[i].start < time) {
		time = cl_explosions[i].start;
		index = i;
	}
	memset (&cl_explosions[index], 0, sizeof(cl_explosions[index]));
	return &cl_explosions[index];
}

/*
=================
CL_ParseBeam
=================
*/
int CL_ParseBeam (struct model_s *model) {
	int ent;
	vec3_t start, end;
	beam_t *b;
	entity_t ents;
	int i;

	ent = MSG_ReadShort (&net_message);

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

	// override any beam with the same entity
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++)
	if (b->entity == ent) {
		b->entity = ent;
		b->model = model;
		b->endtime = cl.time + 200;
		ents.model = b->model;
		ents.flags |= RF_NOSHADOW;
		VectorCopy (start, b->start);
		VectorCopy (end, b->end);
		VectorClear (b->offset);
		return ent;
	}
	// find a free beam
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++) {
		if (!b->model || b->endtime < cl.time) {
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			ents.model = b->model;
			ents.flags |= RF_NOSHADOW;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return ent;
		}
	}
	Com_Printf ("beam list overflow!\n");
	return ent;
}

/*
=================
CL_ParseBeam2
=================
*/
int CL_ParseBeam2 (struct model_s *model) {
	int ent;
	vec3_t start, end, offset;
	beam_t *b;
	int i;
	entity_t ents;
	ent = MSG_ReadShort (&net_message);

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);
	MSG_ReadPos (&net_message, offset);

	//  Com_Printf ("end- %f %f %f\n", end[0], end[1], end[2]);

	// override any beam with the same entity

	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++)
	if (b->entity == ent) {
		b->entity = ent;
		b->model = model;
		ents.model = b->model;
		ents.flags |= RF_NOSHADOW;
		b->endtime = cl.time + 200;
		VectorCopy (start, b->start);
		VectorCopy (end, b->end);
		VectorCopy (offset, b->offset);
		return ent;
	}
	// find a free beam
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++) {
		if (!b->model || b->endtime < cl.time) {
			b->entity = ent;
			b->model = model;
			ents.model = b->model;
			ents.flags |= RF_NOSHADOW;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}
	Com_Printf ("beam list overflow!\n");
	return ent;
}

// ROGUE
/*
=================
CL_ParsePlayerBeam
- adds to the cl_playerbeam array instead of the cl_beams array
=================
*/
int CL_ParsePlayerBeam (struct model_s *model) {
	int ent;
	vec3_t start, end, offset;
	beam_t *b;
	int i;

	ent = MSG_ReadShort (&net_message);

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);
	// PMM - network optimization
	if (model == cl_mod_heatbeam)
		VectorSet (offset, 2, 7, -3);
	else if (model == cl_mod_monster_heatbeam) {
		model = cl_mod_heatbeam;
		VectorSet (offset, 0, 0, 0);
	}
	else
		MSG_ReadPos (&net_message, offset);

	//  Com_Printf ("end- %f %f %f\n", end[0], end[1], end[2]);

	// override any beam with the same entity
	// PMM - For player beams, we only want one per player (entity) so..
	for (i = 0, b = cl_playerbeams; i < MAX_BEAMS; i++, b++) {
		if (b->entity == ent) {
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}

	// find a free beam
	for (i = 0, b = cl_playerbeams; i < MAX_BEAMS; i++, b++) {
		if (!b->model || b->endtime < cl.time) {
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 100;	// PMM - this needs to be 100 to
			// prevent multiple heatbeams
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}
	Com_Printf ("beam list overflow!\n");
	return ent;
}

//rogue

/*
=================
CL_ParseLightning
=================
*/
int CL_ParseLightning (struct model_s *model) {
	int srcEnt, destEnt;
	vec3_t start, end;
	beam_t *b;
	int i;

	srcEnt = MSG_ReadShort (&net_message);
	destEnt = MSG_ReadShort (&net_message);

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

	// override any beam with the same source AND destination entities
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++)
	if (b->entity == srcEnt && b->dest_entity == destEnt) {
		//          Com_Printf("%d: OVERRIDE  %d -> %d\n", cl.time, srcEnt, destEnt);
		b->entity = srcEnt;
		b->dest_entity = destEnt;
		b->model = model;
		b->endtime = cl.time + 200;
		VectorCopy (start, b->start);
		VectorCopy (end, b->end);
		VectorClear (b->offset);
		return srcEnt;
	}
	// find a free beam
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++) {
		if (!b->model || b->endtime < cl.time) {
			//          Com_Printf("%d: NORMAL  %d -> %d\n", cl.time, srcEnt, destEnt);
			b->entity = srcEnt;
			b->dest_entity = destEnt;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return srcEnt;
		}
	}
	Com_Printf ("beam list overflow!\n");
	return srcEnt;
}

/*
=================
CL_ParseLaser
=================
*/
void CL_ParseLaser (int colors) {
	vec3_t start;
	vec3_t end;
	laser_t *l;
	int i;

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

	for (i = 0, l = cl_lasers; i < MAX_LASERS; i++, l++) {
		if (l->endtime < cl.time) {
			l->ent.flags = RF_TRANSLUCENT | RF_BEAM;
			VectorCopy (start, l->ent.origin);
			VectorCopy (end, l->ent.oldorigin);
			l->ent.alpha = 0.30;
			l->ent.skinnum = (colors >> ((rand () % 4) * 8)) & 0xff;
			l->ent.model = NULL;
			l->ent.frame = 4;
			l->endtime = cl.time + 100;
			return;
		}
	}
}

//=============
//ROGUE
void CL_ParseSteam (void) {
	vec3_t pos, dir;
	int id, i;
	int r;
	int cnt;
	int color;
	int magnitude;
	cl_sustain_t *s, *free_sustain;

	id = MSG_ReadShort (&net_message);	// an id of -1 is an instant
	// effect
	if (id != -1)				// sustains
	{
		//          Com_Printf ("Sustain effect id %d\n", id);
		free_sustain = NULL;
		for (i = 0, s = cl_sustains; i < MAX_SUSTAINS; i++, s++) {
			if (s->id == 0) {
				free_sustain = s;
				break;
			}
		}
		if (free_sustain) {
			s->id = id;
			s->count = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, s->org);
			MSG_ReadDir (&net_message, s->dir);
			r = MSG_ReadByte (&net_message);
			s->color = r & 0xff;
			s->magnitude = MSG_ReadShort (&net_message);
			s->endtime = cl.time + MSG_ReadLong (&net_message);
			s->think = CL_ParticleSteamEffect2;
			s->thinkinterval = 100;
			s->nextthink = cl.time;
		}
		else {
			//              Com_Printf ("No free sustains!\n");
			// FIXME - read the stuff anyway
			cnt = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			r = MSG_ReadByte (&net_message);
			magnitude = MSG_ReadShort (&net_message);
			magnitude = MSG_ReadLong (&net_message);	// really interval
		}
	}
	else						// instant
	{
		cnt = MSG_ReadByte (&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		r = MSG_ReadByte (&net_message);
		magnitude = MSG_ReadShort (&net_message);
		color = r & 0xff;
		CL_ParticleSteamEffect (pos, dir, color, cnt, magnitude);
		//      S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
	}
}

void CL_ParseWidow (void) {
	vec3_t pos;
	int id, i;
	cl_sustain_t *s, *free_sustain;

	id = MSG_ReadShort (&net_message);

	free_sustain = NULL;
	for (i = 0, s = cl_sustains; i < MAX_SUSTAINS; i++, s++) {
		if (s->id == 0) {
			free_sustain = s;
			break;
		}
	}
	if (free_sustain) {
		s->id = id;
		MSG_ReadPos (&net_message, s->org);
		s->endtime = cl.time + 2100;
		s->think = CL_Widowbeamout;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else						// no free sustains
	{
		// FIXME - read the stuff anyway
		MSG_ReadPos (&net_message, pos);
	}
}

void CL_ParseNuke (void) {
	vec3_t pos;
	int i;
	cl_sustain_t *s, *free_sustain;

	free_sustain = NULL;
	for (i = 0, s = cl_sustains; i < MAX_SUSTAINS; i++, s++) {
		if (s->id == 0) {
			free_sustain = s;
			break;
		}
	}
	if (free_sustain) {
		s->id = 21000;
		MSG_ReadPos (&net_message, s->org);
		s->endtime = cl.time + 1000;
		s->think = CL_Nukeblast;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else						// no free sustains
	{
		// FIXME - read the stuff anyway
		MSG_ReadPos (&net_message, pos);
	}
}

//ROGUE
//=============



/*
 =================
 CL_FindTrailPlane

 Disgusting hack
 =================
 */
void CL_FindTrailPlane (vec3_t start, vec3_t end, vec3_t dir) {

	trace_t trace;
	vec3_t vec, point;
	float len;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	VectorMA (start, len + 0.5, vec, point);

	trace =
		CM_BoxTrace (start, point, vec3_origin, vec3_origin, 0, MASK_SOLID);
	if (trace.allsolid || trace.fraction == 1.0) {
		VectorClear (dir);
		return;
	}

	VectorCopy (trace.plane.normal, dir);
}

void CL_FindRailedSurface (vec3_t start, vec3_t end, vec3_t dir) {

	trace_t trace;
	vec3_t vec, point;
	float len;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	VectorMA (start, len + 0.5, vec, point);

	trace = CM_BoxTrace (start, point, vec3_origin, vec3_origin, 0, MASK_SOLID);

	if (!(trace.surface->flags & SURF_SKY)) {
		CL_ParticleRailRick (end, dir);

	}
}


void CL_ParticleSplashLava (vec3_t org, vec3_t dir);
void CL_ParticleSplashSlime (vec3_t org, vec3_t dir);

/*=============================================================
 ReadScaledDir - Berserker's non normalized vector compression
 =============================================================*/

void ReadScaledDir (vec3_t vec) {
	float len;
	MSG_ReadDir (&net_message, vec);
	len = MSG_ReadFloat (&net_message);
	VectorScale (vec, len, vec);
}


/*
=================
CL_ParseTEnt
=================
*/
static byte splash_color[] = { 0x00, 0xe0, 0xb0, 0x50, 0xd0, 0xe0, 0xe8 };

void CL_ParseTEnt (void) {
	int type, cont;
	vec3_t pos, pos2, dir, size, velocity, pos3;
	explosion_t *ex;
	int cnt;
	int color;
	int r;
	int ent;
	int magnitude;

	type = MSG_ReadByte (&net_message);

	switch (type) {

		case TE_BLOOD:				// bullet hitting flesh

			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			CL_ParticleBlood (pos, dir, 3);

			break;


		case TE_SPARKS:
		case TE_BULLET_SPARKS:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);

			if (type == TE_BULLET_SPARKS) {
				// impact sound
				cnt = rand () & 15;
				if (cnt < 3)
					S_fastsound (pos, 0, 0,
					fastsound_descriptor[id_cl_sfx_ric1 + cnt], 1,
					ATTN_NORM);
			}

			break;

		case TE_SCREEN_SPARKS:
		case TE_SHIELD_SPARKS:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			if (type == TE_SCREEN_SPARKS)
				CL_ParticleArmorSpark (pos, dir, 3, qfalse);
			else
				CL_ParticleArmorSpark (pos, dir, 3, qtrue);
			// FIXME : replace or remove this sound
			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;

		case TE_GUNSHOT:
		case TE_SHOTGUN:			// bullet hitting wall

			MSG_ReadPos (&net_message, pos);	// end pos
			if (!net_compatibility->integer)
				MSG_ReadPos (&net_message, pos2);	// start pos
			MSG_ReadDir (&net_message, dir);

			CL_AddDecalToScene (pos, dir,
				1, 1, 1, 1,
				1, 1, 1, 1,
				3, 20000,
				DECAL_BULLET, 0, frand () * 360,
				GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

			cont = (CL_PMpointcontents (pos) & MASK_WATER);
			if (!cont)
				CL_ParticleRick (pos, dir);

			if (type != TE_SHOTGUN && !net_compatibility->value)
				CL_ParticleTracer (pos2, pos);

			if (type == TE_GUNSHOT) {
				// impact sound
				cnt = rand () & 15;
				if (cnt < 3)
					S_fastsound (pos, 0, 0,
					fastsound_descriptor[id_cl_sfx_ric1 + cnt], 1,
					ATTN_NORM);
			}
			break;

		case TE_SPLASH:			// bullet hitting water
			cnt = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			VectorNormalize (dir);
			r = MSG_ReadByte (&net_message);
			if (r > 6)

				color = 0x00;
			else
				color = splash_color[r];
			if (r != SPLASH_SPARKS) {
				if (r == SPLASH_BLUE_WATER || r == SPLASH_BROWN_WATER) {
					CL_ParticleSplash (pos, dir, 0.9, 0.9, 0.9);

				}
				else if (r == SPLASH_LAVA) {
					CL_ParticleSmoke2 (pos, dir, 1, 0.3, 0, 15, qtrue);

				}
				else if (r == SPLASH_SLIME) {
					CL_ParticleSplashSlime (pos, dir);

				}


			}
			if (r == SPLASH_SPARKS) {
				r = rand() & 3;
				if (r == 0)
					S_fastsound(pos, 0, 0, fastsound_descriptor[id_cl_sfx_spark5], 0.5, ATTN_STATIC);
				else if (r == 1)
					S_fastsound(pos, 0, 0, fastsound_descriptor[id_cl_sfx_spark6], 0.5, ATTN_STATIC);
				else
					S_fastsound(pos, 0, 0, fastsound_descriptor[id_cl_sfx_spark7], 0.5, ATTN_STATIC);

				CL_ParticleSmoke (pos, dir, 6);
				CL_ParticleSpark (pos, dir, 25);
			}
			break;

		case TE_LASER_SPARKS:
			cnt = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			color = MSG_ReadByte (&net_message);

			CL_AddDecalToScene (pos, dir,
				1, 1, 0, 1,
				0, -0.1, 0, 1,
				3, 2000,
				DECAL_BLASTER, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			CL_LaserParticle2 (pos, dir, color, cnt);
			break;

			// RAFAEL
		case TE_BLUEHYPERBLASTER:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadPos (&net_message, dir);
			CL_BlasterParticles (pos, dir);
			break;

		case TE_BLASTER:			// blaster hitting wall

			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);

			CL_AddDecalToScene (pos, dir,
				1, 1, 0, 1,
				-0.1, -2, 0, 1,
				3, 15000,
				DECAL_BLASTER, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			CL_ParticleSmoke2 (pos, dir, 0.97, 0.46, 0.14, 16, qtrue);
			CL_BlasterParticles (pos, dir);
			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;

		case TE_RAILTRAIL:			// railgun effect
			MSG_ReadPos (&net_message, pos);
			MSG_ReadPos (&net_message, pos2);

			if (!net_compatibility->integer) {
				MSG_ReadDir (&net_message, dir);

			}
			if (net_compatibility->integer)
				CL_FindTrailPlane (pos, pos2, dir);

			CL_RailTrail (pos, pos2);

			CL_FindRailedSurface (pos, pos2, dir);

			CL_AddDecalToScene (pos2, dir,
				1, 1, 1, 1,
				1, 1, 1, 1,
				3.5, 20000,
				DECAL_RAIL, 0, frand () * 360,
				GL_ZERO, GL_ONE_MINUS_SRC_COLOR);


			S_StartSound (pos2, 0, 0, cl_sfx_railg, 1, ATTN_NORM, 0);
			break;

		case TE_EXPLOSION2:
		case TE_GRENADE_EXPLOSION:
		case TE_GRENADE_EXPLOSION_WATER:
			MSG_ReadPos (&net_message, pos);

			if (!net_compatibility->integer) {
				MSG_ReadDir (&net_message, dir);
				if (type == TE_EXPLOSION2)
					CL_Debris (pos, dir);
			}

			pos[2] += 3.0; // grenade explosion decal hack
			CL_AddDecalToScene (pos, vec3_origin,
				0.5, 0.0, 0.0, 1,
				0.1, 0.0, 0.0, 1,
				40, 10000,
				DECAL_EXPLODE, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			CL_Explosion (pos);

			ex = CL_AllocExplosion ();
			VectorCopy (pos, ex->ent.origin);
			ex->type = ex_poly;
			ex->ent.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DISTORT;
			ex->ent.alpha = 0.0;
			ex->start = cl.frame.servertime - 100;

			ex->ent.model = cl_mod_distort;
			ex->frames = 5;
			ex->baseframe = 0;

			ex->ent.angles[1] = rand () % 360;

			if (type == TE_GRENADE_EXPLOSION_WATER)
				S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_EXPLOSION, 0);
			else
				S_StartSound (pos, 0, 0, cl_sfx_grenexp, 1, ATTN_EXPLOSION, 0);



			break;

			// RAFAEL
		case TE_PLASMA_EXPLOSION:
			MSG_ReadPos (&net_message, pos);

			if (!net_compatibility->integer)
				MSG_ReadDir (&net_message, dir);

			CL_AddDecalToScene (pos, vec3_origin,
				0.5, 0.0, 0.0, 1,
				0.1, 0.0, 0.0, 1,
				40, 10000,
				DECAL_EXPLODE, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			CL_Explosion (pos);

			ex = CL_AllocExplosion ();
			VectorCopy (pos, ex->ent.origin);
			ex->type = ex_poly;
			ex->ent.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DISTORT;
			ex->ent.alpha = 0.0;
			ex->start = cl.frame.servertime - 100;

			ex->ent.model = cl_mod_distort;
			ex->frames = 5;
			ex->baseframe = 0;

			ex->ent.angles[1] = rand () % 360;

			S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1.0, ATTN_EXPLOSION, 0);
			break;

		case TE_EXPLOSION1:
		case TE_EXPLOSION1_BIG:	// PMM
		case TE_ROCKET_EXPLOSION:
		case TE_ROCKET_EXPLOSION_WATER:
		case TE_EXPLOSION1_NP:		// PMM
			MSG_ReadPos (&net_message, pos);

			if (!net_compatibility->integer) {
				if (type == TE_ROCKET_EXPLOSION
					|| type == TE_ROCKET_EXPLOSION_WATER)
					MSG_ReadDir (&net_message, dir);

			}

			CL_AddDecalToScene (pos, vec3_origin,
				0.5, 0.0, 0.0, 1,
				0.1, 0.0, 0.0, 1,
				40, 10000,
				DECAL_EXPLODE, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			CL_Explosion (pos);

			ex = CL_AllocExplosion ();
			VectorCopy (pos, ex->ent.origin);
			ex->type = ex_poly;
			ex->ent.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DISTORT;
			ex->ent.alpha = 0.0;
			ex->start = cl.frame.servertime - 100;

			ex->ent.model = cl_mod_distort;
			ex->frames = 5;
			ex->baseframe = 0;

			ex->ent.angles[1] = rand () % 360;

			if (type == TE_ROCKET_EXPLOSION_WATER)
				S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_EXPLOSION, 0);
			else
				S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_EXPLOSION, 0);

			break;

		case TE_BFG_EXPLOSION:
		MSG_ReadPos (&net_message, pos);
			ex = CL_AllocExplosion ();
			VectorCopy(pos, ex->ent.origin);
			ex->type = ex_poly;
			ex->ent.flags = RF_FULLBRIGHT;
			ex->start = cl.frame.servertime - 100;
			ex->light = 350;
			ex->lightcolor[0] = 0.0;
			ex->lightcolor[1] = 1.0;
			ex->lightcolor[2] = 0.0;
			ex->ent.model = cl_mod_bfg_explo;
			ex->ent.flags |= RF_TRANSLUCENT;
			ex->ent.flags |= RF_DISTORT;
			ex->ent.flags |= RF_BFG_SPRITE;
			ex->ent.alpha = 0.30;
			ex->frames = 4;
			

			break;

		case TE_BFG_BIGEXPLOSION:
			MSG_ReadPos (&net_message, pos);

			if (!net_compatibility->integer)
				MSG_ReadDir (&net_message, dir);

			CL_AddDecalToScene (pos, vec3_origin,
				0.5, 0.0, 0.0, 1,
				0.1, 0.0, 0.0, 1,
				55, 20000,
				DECAL_EXPLODE, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			CL_BfgExplosion(pos);
			CL_BFGExplosionParticles (pos);

			break;

		case TE_BFG_LASER:
			CL_ParseLaser (0xd0d1d2d3);
			break;

		case TE_BUBBLETRAIL:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadPos (&net_message, pos2);
			CL_BubbleTrail (pos, pos2);
			break;

		case TE_PARASITE_ATTACK:
		case TE_MEDIC_CABLE_ATTACK:
			ent = CL_ParseBeam (cl_mod_parasite_segment);	// no shadow from
			// beam now!

			break;

		case TE_BOSSTPORT:			// boss teleporting to station
			MSG_ReadPos (&net_message, pos);
			CL_BigTeleportParticles (pos);
			S_StartSound (pos, 0, 0, S_RegisterSound ("misc/bigtele.wav"), 1,
				ATTN_NONE, 0);
			break;

		case TE_GRAPPLE_CABLE:
			ent = CL_ParseBeam2 (cl_mod_grapple_cable);
			break;

			// RAFAEL
		case TE_WELDING_SPARKS:
			cnt = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			color = MSG_ReadByte (&net_message);

			CL_ParticleSmoke (pos, dir, 3);
			CL_ParticleSpark (pos, dir, 25);
			break;

		case TE_GREENBLOOD:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			CL_ParticleBlood2 (pos, dir, 3);
			break;

			// RAFAEL
		case TE_TUNNEL_SPARKS:
			cnt = MSG_ReadByte (&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			color = MSG_ReadByte (&net_message);

			CL_ParticleSmoke (pos, dir, 3);
			CL_ParticleSpark (pos, dir, 25);
			break;

			//=============
			//PGM
			// PMM -following code integrated for flechette (different color)
		case TE_BLASTER2:			// green blaster hitting wall
		case TE_FLECHETTE:			// flechette
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);

			if (type == TE_BLASTER2) {
				CL_BlasterParticles (pos, dir);
				CL_AddDecalToScene (pos, dir,
					1, 1, 0, 1,
					-0.1, -2, 0, 1,
					6, 20000,
					DECAL_BLASTER, 0, frand () * 360,
					GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (type == TE_FLECHETTE) {

				CL_AddDecalToScene (pos, dir,
					1, 1, 1, 1,
					1, 1, 1, 1,
					3, 20000,
					DECAL_BULLET, 0, frand () * 360,
					GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

				cont = (CL_PMpointcontents (pos) & MASK_WATER);
				if (!cont)
					CL_ParticleRick (pos, dir);

			}


			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;


		case TE_LIGHTNING:
			ent = CL_ParseLightning (cl_mod_lightning);
			S_StartSound (NULL, ent, CHAN_WEAPON, cl_sfx_lightning, 1,
				ATTN_NORM, 0);
			break;


		case TE_PLAIN_EXPLOSION:
			MSG_ReadPos (&net_message, pos);

			if (!net_compatibility->integer) {
				if (type == TE_ROCKET_EXPLOSION
					|| type == TE_ROCKET_EXPLOSION_WATER)
					MSG_ReadDir (&net_message, dir);

			}

			CL_Explosion (pos);
			ex = CL_AllocExplosion ();
			VectorCopy (pos, ex->ent.origin);
			ex->type = ex_poly;
			ex->ent.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DISTORT;
			ex->ent.alpha = 0.0;
			ex->start = cl.frame.servertime - 100;

			ex->ent.model = cl_mod_explo4;
			ex->frames = 6;
			ex->baseframe = 0;

			ex->ent.angles[1] = rand () % 360;

			if (type == TE_ROCKET_EXPLOSION_WATER)
				S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_EXPLOSION, 0);
			else
				S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_EXPLOSION, 0);

			CL_AddDecalToScene (pos, vec3_origin,
				0.5, 0.0, 0.0, 1,
				0.1, 0.0, 0.0, 1,
				40, 10000,
				DECAL_EXPLODE, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


			break;

		case TE_FLASHLIGHT:
			MSG_ReadPos (&net_message, pos);
			ent = MSG_ReadShort (&net_message);
			CL_Flashlight (ent, pos);
			break;

		case TE_FORCEWALL:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadPos (&net_message, pos2);
			color = MSG_ReadByte (&net_message);
			CL_ForceWall (pos, pos2, color);
			break;

		case TE_HEATBEAM:
			ent = CL_ParsePlayerBeam (cl_mod_heatbeam);

			break;

		case TE_MONSTER_HEATBEAM:
			ent = CL_ParsePlayerBeam (cl_mod_monster_heatbeam);
			break;

		case TE_HEATBEAM_SPARKS:

			cnt = 50;
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			r = 8;
			magnitude = 60;
			color = r & 0xff;
			CL_ParticleSteamEffect (pos, dir, color, cnt, magnitude);
			CL_ParticleSmoke2 (pos, dir, 1, 0.5, 0, 6, qtrue);

			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;

		case TE_HEATBEAM_STEAM:
			cnt = 20;
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);

			color = 0xe0;
			magnitude = 60;
			CL_ParticleSmoke2 (pos, dir, 1, 0.5, 0, 6, qtrue);
			CL_BlasterParticles (pos, dir);

			CL_AddDecalToScene (pos, dir,
				1, 1, 0, 1,
				-0.1, -2, 0, 1,
				6, 20000,
				DECAL_BLASTER, 0, frand () * 360,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;

		case TE_STEAM:
			CL_ParseSteam ();
			break;

		case TE_BUBBLETRAIL2:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadPos (&net_message, pos2);
			CL_BubbleTrail (pos, pos2);
			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;

		case TE_MOREBLOOD:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			CL_ParticleBlood (pos, dir, 3);


			break;

		case TE_CHAINFIST_SMOKE:
			dir[0] = 0;
			dir[1] = 0;
			dir[2] = 1;
			MSG_ReadPos (&net_message, pos);
			CL_ParticleSmoke (pos, dir, 4);
			break;

		case TE_ELECTRIC_SPARKS:
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			CL_ParticleSmoke (pos, dir, 3);
			CL_ParticleSpark (pos, dir, 25);
			// FIXME : replace or remove this sound
			S_StartSound (pos, 0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
			break;

		case TE_TRACKER_EXPLOSION:
			MSG_ReadPos (&net_message, pos);
			CL_ColorFlash (pos, 0, 150, -1, -1, -1);

			CL_Explosion (pos);
			S_StartSound (pos, 0, 0, cl_sfx_disrexp, 1, 0.1, 0);
			break;

		case TE_TELEPORT_EFFECT:
		case TE_DBALL_GOAL:
			MSG_ReadPos (&net_message, pos);
			CL_TeleportParticles (pos);
			break;

		case TE_WIDOWBEAMOUT:
			CL_ParseWidow ();
			break;

		case TE_NUKEBLAST:
			CL_ParseNuke ();
			break;

		case TE_WIDOWSPLASH:
			MSG_ReadPos (&net_message, pos);
			CL_WidowSplash (pos);
			break;

		case TE_UNHEAD:

			MSG_ReadPos (&net_message, pos);
			CL_ParticleHeadBlood (pos);

			break;

		case TE_REBORN:

			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			CL_GibExplosion (pos, dir);
			CL_ParticleGibBlood (pos);

			break;

		case TE_GIB:
			MSG_ReadPos (&net_message, pos);
			CL_ParticleGibBlood (pos);
			break;

		case TE_GIB2:
			MSG_ReadPos (&net_message, pos);
			CL_ParticleGibBlood2 (pos);
			break;

		case TE_GIB_CLIENT:
			MSG_ReadPos (&net_message, pos3);
			size[0] = MSG_ReadByte (&net_message);
			size[1] = MSG_ReadByte (&net_message);
			size[2] = MSG_ReadByte (&net_message);
			ReadScaledDir (velocity);          // read the 5 bytes instead 12
			VectorAdd (pos3, size, pos2);
			pos[0] = pos2[0] + crandom () * size[0];
			pos[1] = pos2[1] + crandom () * size[1];
			pos[2] = pos2[2] + crandom () * size[2];

			CL_ClientGibs (pos, velocity);
			CL_ParticleGibBlood (pos);

			break;


			//PGM
			//==============

		default:
			Com_Error (ERR_DROP, "CL_ParseTEnt: bad type");
	}
}

/*
=================
CL_AddBeams
=================
*/
void CL_AddBeams (void) {
	int i, j;
	beam_t *b;
	vec3_t dist, org;
	float d;
	entity_t ent;
	float yaw, pitch;
	float forward;
	float len, steps;
	float model_length;

	// update beams
	for (i = 0, b = cl_beams; i < MAX_BEAMS; i++, b++) {
		if (!b->model || b->endtime < cl.time)
			continue;

		// if coming from the player, update the start position
		if (b->entity == cl.playernum + 1)	// entity 0 is the world
		{
			VectorCopy (cl.refdef.vieworg, b->start);
			b->start[2] -= 22;	// adjust for view height
		}
		VectorAdd (b->start, b->offset, org);

		// calculate pitch and yaw
		VectorSubtract (b->end, org, dist);

		if (dist[1] == 0 && dist[0] == 0) {
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else {
			// PMM - fixed to correct for pitch of 0
			if (dist[0])
				yaw = (atan2 (dist[1], dist[0]) * 180 / M_PI);
			else if (dist[1] > 0)
				yaw = 90;
			else
				yaw = 270;
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (dist[0] * dist[0] + dist[1] * dist[1]);
			pitch = (atan2 (dist[2], forward) * -180.0 / M_PI);
			if (pitch < 0)
				pitch += 360.0;
		}

		// add new entities for the beams
		d = VectorNormalize (dist);

		memset (&ent, 0, sizeof(ent));
		if (b->model == cl_mod_lightning) {
			model_length = 35.0;
			d -= 20.0;			// correction so it doesn't end in middle
			// of tesla
		}
		else {
			model_length = 30.0;
		}
		steps = ceil (d / model_length);
		len = (d - model_length) / (steps - 1);

		// PMM - special case for lightning model .. if the real length is
		// shorter than the model,
		// flip it around & draw it from the end to the start.  This
		// prevents the model from going
		// through the tesla mine (instead it goes through the target)
		if ((b->model == cl_mod_lightning) && (d <= model_length)) {
			//          Com_Printf ("special case\n");
			VectorCopy (b->end, ent.origin);
			// offset to push beam outside of tesla model (negative
			// because dist is from end to start
			// for this beam)
			//          for (j=0 ; j<3 ; j++)
			//              ent.origin[j] -= dist[j]*10.0;

			ent.model = b->model;
			ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
			ent.angles[0] = pitch;
			ent.angles[1] = yaw;
			ent.angles[2] = rand () % 360;
			V_AddEntity (&ent);
			return;
		}
		while (d > 0) {
			VectorCopy (org, ent.origin);
			ent.model = b->model;

			if (b->model == cl_mod_lightning) {
				ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = rand () % 360;
			}
			else {
				ent.angles[0] = pitch;
				ent.angles[1] = yaw;
				ent.angles[2] = rand () % 360;
			}
			ent.flags |= RF_NOSHADOW;
			//          Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
			V_AddEntity (&ent);

			for (j = 0; j < 3; j++)
				org[j] += dist[j] * len;
			d -= model_length;
		}
	}
}


/*
//				Com_Printf ("Endpoint:  %f %f %f\n", b->end[0], b->end[1], b->end[2]);
//				Com_Printf ("Pred View Angles:  %f %f %f\n", cl.predicted_angles[0], cl.predicted_angles[1], cl.predicted_angles[2]);
//				Com_Printf ("Act View Angles: %f %f %f\n", cl.refdef.viewangles[0], cl.refdef.viewangles[1], cl.refdef.viewangles[2]);
//				VectorCopy (cl.predicted_origin, b->start);
//				b->start[2] += 22;	// adjust for view height
//				if (fabs(cl.refdef.vieworg[2] - b->start[2]) >= 10) {
//					b->start[2] = cl.refdef.vieworg[2];
//				}

//				Com_Printf ("Time:  %d %d %f\n", cl.time, cls.realtime, cls.frametime);
*/

extern cvar_t *hand;

/*
=================
ROGUE - draw player locked beams
CL_AddPlayerBeams
=================
*/
void CL_AddPlayerBeams (void) {
	int i, j;
	beam_t *b;
	vec3_t dist, org;
	float d;
	entity_t ent;
	float yaw, pitch;
	float forward;
	float len, steps;
	int framenum;
	float model_length;

	float hand_multiplier;
	frame_t *oldframe;
	player_state_t *ps, *ops;

	//PMM
	if (hand) {
		if (hand->integer == 2)
			hand_multiplier = 0;
		else if (hand->integer == 1)
			hand_multiplier = -1;
		else
			hand_multiplier = 1;
	}
	else {
		hand_multiplier = 1;
	}
	//PMM

	// update beams
	for (i = 0, b = cl_playerbeams; i < MAX_BEAMS; i++, b++) {
		vec3_t f, r, u;
		if (!b->model || b->endtime < cl.time)
			continue;

		if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam)) {
			ent.model = b->model;
			ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
			// if coming from the player, update the start position
			if (b->entity == cl.playernum + 1)	// entity 0 is the world
			{
				// set up gun position
				// code straight out of CL_AddViewWeapon
				ps = &cl.frame.playerstate;
				j = (cl.frame.serverframe - 1) & UPDATE_MASK;
				oldframe = &cl.frames[j];
				if (oldframe->serverframe != cl.frame.serverframe - 1
					|| !oldframe->valid)
					oldframe = &cl.frame;	// previous frame was dropped
				// or involid
				ops = &oldframe->playerstate;
				for (j = 0; j < 3; j++) {
					b->start[j] = cl.refdef.vieworg[j] + ops->gunoffset[j]
						+ cl.lerpfrac * (ps->gunoffset[j] -
						ops->gunoffset[j]);
				}
				VectorMA (b->start, (hand_multiplier * b->offset[0]),
					cl.v_right, org);
				VectorMA (org, b->offset[1], cl.v_forward, org);
				VectorMA (org, b->offset[2], cl.v_up, org);
				if ((hand) && (hand->integer == 2)) {
					VectorMA (org, -1, cl.v_up, org);
				}
				// FIXME - take these out when final
				VectorCopy (cl.v_right, r);
				VectorCopy (cl.v_forward, f);
				VectorCopy (cl.v_up, u);

			}
			else
				VectorCopy (b->start, org);
		}
		else {
			// if coming from the player, update the start position
			if (b->entity == cl.playernum + 1)	// entity 0 is the world
			{
				VectorCopy (cl.refdef.vieworg, b->start);
				b->start[2] -= 22;	// adjust for view height
			}
			VectorAdd (b->start, b->offset, org);
		}

		// calculate pitch and yaw
		VectorSubtract (b->end, org, dist);

		//PMM
		if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam)
			&& (b->entity == cl.playernum + 1)) {
			vec_t len;

			len = VectorLength (dist);
			VectorScale (f, len, dist);
			VectorMA (dist, (hand_multiplier * b->offset[0]), r, dist);
			VectorMA (dist, b->offset[1], f, dist);
			VectorMA (dist, b->offset[2], u, dist);
			if ((hand) && (hand->integer == 2)) {
				VectorMA (org, -1, cl.v_up, org);
			}
		}
		//PMM

		if (dist[1] == 0 && dist[0] == 0) {
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else {
			// PMM - fixed to correct for pitch of 0
			if (dist[0])
				yaw = (atan2 (dist[1], dist[0]) * 180 / M_PI);
			else if (dist[1] > 0)
				yaw = 90;
			else
				yaw = 270;
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (dist[0] * dist[0] + dist[1] * dist[1]);
			pitch = (atan2 (dist[2], forward) * -180.0 / M_PI);
			if (pitch < 0)
				pitch += 360.0;
		}

		if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam)) {
			if (b->entity != cl.playernum + 1) {
				framenum = 2;
				//              Com_Printf ("Third person\n");
				ent.flags = RF_NOSHADOW;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = 0;
				//              Com_Printf ("%f %f - %f %f %f\n", -pitch, yaw+180.0, b->offset[0], b->offset[1], b->offset[2]);
				AngleVectors (ent.angles, f, r, u);

				// if it's a non-origin offset, it's a player, so use the
				// hardcoded player offset
				if (!VectorCompare (b->offset, vec3_origin)) {
					VectorMA (org, -(b->offset[0]) + 1, r, org);
					VectorMA (org, -(b->offset[1]), f, org);
					VectorMA (org, -(b->offset[2]) - 10, u, org);
				}
				else {
					// if it's a monster, do the particle effect
					CL_MonsterPlasma_Shell (b->start);
				}
			}
			else {
				framenum = 1;
			}
		}
		// if it's the heatbeam, draw the particle effect
		if ((cl_mod_heatbeam && (b->model == cl_mod_heatbeam)
			&& (b->entity == cl.playernum + 1))) {
			CL_Heatbeam (org, dist);
			ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
		}
		// add new entities for the beams
		d = VectorNormalize (dist);

		memset (&ent, 0, sizeof(ent));
		if (b->model == cl_mod_heatbeam) {
			model_length = 32.0;
		}
		else if (b->model == cl_mod_lightning) {
			model_length = 35.0;
			d -= 20.0;			// correction so it doesn't end in middle
			// of tesla
		}
		else {
			model_length = 30.0;
		}
		steps = ceil (d / model_length);
		len = (d - model_length) / (steps - 1);

		// PMM - special case for lightning model .. if the real length is
		// shorter than the model,
		// flip it around & draw it from the end to the start.  This
		// prevents the model from going
		// through the tesla mine (instead it goes through the target)
		if ((b->model == cl_mod_lightning) && (d <= model_length)) {
			//          Com_Printf ("special case\n");
			VectorCopy (b->end, ent.origin);
			// offset to push beam outside of tesla model (negative
			// because dist is from end to start
			// for this beam)
			//          for (j=0 ; j<3 ; j++)
			//              ent.origin[j] -= dist[j]*10.0;
			ent.model = b->model;
			ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
			ent.angles[0] = pitch;
			ent.angles[1] = yaw;
			ent.angles[2] = rand () % 360;
			V_AddEntity (&ent);
			return;
		}
		while (d > 0) {
			VectorCopy (org, ent.origin);
			ent.model = b->model;
			if (cl_mod_heatbeam && (b->model == cl_mod_heatbeam)) {
				//              ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
				//              ent.alpha = 0.3;
				ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = (cl.time) % 360;
				//              ent.angles[2] = rand()%360;
				ent.frame = framenum;
			}
			else if (b->model == cl_mod_lightning) {
				ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = rand () % 360;
			}
			else {
				ent.angles[0] = pitch;
				ent.angles[1] = yaw;
				ent.angles[2] = rand () % 360;
			}

			//          Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
			V_AddEntity (&ent);

			for (j = 0; j < 3; j++)
				org[j] += dist[j] * len;
			d -= model_length;
		}
	}
}

/*
=================
CL_AddExplosions
=================
*/
void CL_AddExplosions (void) {
	entity_t *ent;
	int i;
	explosion_t *ex;
	float frac;
	int f;

	memset (&ent, 0, sizeof(ent));

	for (i = 0, ex = cl_explosions; i < MAX_EXPLOSIONS; i++, ex++) {
		if (ex->type == ex_free)
			continue;
		frac = (cl.time - ex->start) / 100.0;
		f = floor (frac);

		ent = &ex->ent;

		switch (ex->type) {
			case ex_mflash:
				if (f >= ex->frames - 1)
					ex->type = ex_free;
				break;
			case ex_misc:
				if (f >= ex->frames - 1) {
					ex->type = ex_free;
					break;
				}
				ent->alpha = 1.0 - frac / (ex->frames - 1);
				break;
			case ex_flash:
				if (f >= 1) {
					ex->type = ex_free;
					break;
				}
				ent->alpha = 1.0;
				break;
			case ex_poly:
				if (f >= ex->frames - 1) {
					ex->type = ex_free;
					break;
				}

				ent->alpha = (16.0 - (float)f) / 16.0;

				if (f < 10) {
					ent->skinnum = (f >> 1);
					if (ent->skinnum < 0)
						ent->skinnum = 0;
				}
				else {
					ent->flags |= RF_TRANSLUCENT;
					if (f < 13)
						ent->skinnum = 5;
					else
						ent->skinnum = 6;
				}
				break;
			case ex_poly2:
				if (f >= ex->frames - 1) {
					ex->type = ex_free;
					break;
				}

				ent->alpha = (5.0 - (float)f) / 5.0;
				ent->skinnum = 0;
				ent->flags |= RF_TRANSLUCENT;
				break;
			case ex_free:
				break;
			case ex_explosion:
				break;
		}

		if (ex->type == ex_free)
			continue;
		if (ex->light) {
			V_AddLight (ent->origin, ex->light * ent->alpha,
				ex->lightcolor[0], ex->lightcolor[1],
				ex->lightcolor[2], vec3_origin, 0, 0);
		}

		VectorCopy (ent->origin, ent->oldorigin);

		if (f < 0)
			f = 0;
		ent->frame = ex->baseframe + f + 1;
		ent->oldframe = ex->baseframe + f;
		ent->backlerp = 1.0 - cl.lerpfrac;

		V_AddEntity (ent);
	}
}


/*
=================
CL_AddLasers
=================
*/
/*
void CL_AddLasers (void)
{
laser_t		*l;
int			i;

for (i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++)
{
if (l->endtime >= cl.time)
V_AddEntity (&l->ent);
}
}
*/


/* PMM - CL_Sustains */
void CL_ProcessSustain () {
	cl_sustain_t *s;
	int i;

	for (i = 0, s = cl_sustains; i < MAX_SUSTAINS; i++, s++) {
		if (s->id) {
			if ((s->endtime >= cl.time) && (cl.time >= s->nextthink)) {
				//              Com_Printf ("think %d %d %d\n", cl.time, s->nextthink, s->thinkinterval);
				s->think (s);
			}
			else if (s->endtime < cl.time)
				s->id = 0;
		}
	}
}

/*
=================
CL_AddTEnts
=================
*/
void CL_AddTEnts (void) {
	CL_AddBeams ();
	// PMM - draw plasma beams
	CL_AddPlayerBeams ();
	CL_AddExplosions ();
	CL_AddLasers ();
	// PMM - set up sustain
	CL_ProcessSustain ();
}
