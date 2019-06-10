/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2006 Berserker (original code), Kirk Barnes
q2xp porting and adding new effects.

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
// cl_localents.c -- Berserker's client entities effects

#include "client.h"

/*
-----------------------------
ClipMoveEntitiesWorld
-----------------------------
*/
void CL_ClipMoveToEntitiesWorld (vec3_t start, vec3_t mins, vec3_t maxs,
	vec3_t end, trace_t * tr, int mask) {
	int i;
	trace_t trace;
	int headnode;
	float *angles;
	entity_state_t *ent;
	int num;
	cmodel_t *cmodel;

	for (i = 0; i < cl.frame.num_entities; i++) {
		num = (cl.frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
		ent = &cl_parse_entities[num];

		if (!ent->solid)
			continue;


		if (ent->solid != 31)	// special value for bmodel
			continue;

		cmodel = cl.model_clip[ent->modelindex];
		if (!cmodel)
			continue;
		headnode = cmodel->headnode;
		angles = ent->angles;

		if (tr->allsolid)
			return;

		trace =
			CM_TransformedBoxTrace (start, end, mins, maxs, headnode, mask,
			ent->origin, angles);

		if (trace.allsolid || trace.startsolid
			|| trace.fraction < tr->fraction) {
			trace.ent = (struct edict_s *) ent;
			if (tr->startsolid) {
				*tr = trace;
				tr->startsolid = qtrue;
			}
			else
				*tr = trace;
		}
		else if (trace.startsolid)
			tr->startsolid = qtrue;
	}
}

void CL_ClipMoveToEntities (vec3_t start, vec3_t mins, vec3_t maxs,
	vec3_t end, trace_t * tr);

trace_t CL_PMTraceWorld (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end,
	int mask, qboolean checkAliases) {
	trace_t t;

	// check against world
	t = CM_BoxTrace (start, end, mins, maxs, 0, mask);
	if (t.fraction < 1.0)
		t.ent = (struct edict_s *) 1;

	// check all other solid models
	if (checkAliases)
		CL_ClipMoveToEntities (start, mins, maxs, end, &t);
	else
		CL_ClipMoveToEntitiesWorld (start, mins, maxs, end, &t, mask);

	return t;
}


ALuint cl_sfx_lava;
ALuint cl_sfx_shell;
ALuint cl_sfx_debris;
struct model_s *cl_mod_mshell;
struct model_s *cl_mod_sshell;
struct model_s *cl_mod_debris1;
struct model_s *cl_mod_debris2;
struct model_s *cl_mod_debris3;

struct model_s *cl_mod_gib0;
struct model_s *cl_mod_gib1;
struct model_s *cl_mod_gib2;
struct model_s *cl_mod_gib3;
struct model_s *cl_mod_gib4;
struct model_s *cl_mod_gib5;

struct model_s *cl_mod_debris0;

clentity_t *active_clentities, *free_clentities;
clentity_t clentities[MAX_CLENTITIES];


extern int m_menudepth;

void CL_AddClEntities () {
	entity_t ent;
	clentity_t *le, *next;
	clentity_t *active, *tail;
	vec3_t org, dir;
	trace_t trace2;
	float alpha, bak;
	int contents;
	qboolean onground = (qboolean)qfalse;
	float time, time2, dst, grav = Cvar_VariableValue ("sv_gravity");
	vec3_t tmpSize;
	float entSize;
	dst = cl_brass->value * cl_brass->value;

	if (!grav)
		grav = 1;
	else
		grav /= 800;

	active = NULL;
	tail = NULL;

	memset (&ent, 0, sizeof(ent));


	for (le = active_clentities; le; le = next) {
		next = le->next;

		time = (cl.time - le->time) * 0.001;
		alpha = le->alpha + time * le->alphavel;

		if (alpha <= 0) {		// faded out
			le->next = free_clentities;
			free_clentities = le;
			continue;
		}

		time2 = time * time;
		org[0] = le->org[0] + le->vel[0] * time + le->accel[0] * time2;
		org[1] = le->org[1] + le->vel[1] * time + le->accel[1] * time2;
		org[2] = le->org[2] + le->vel[2] * time + le->accel[2] * time2 * grav;

		bak = le->org[2] + le->vel[2] * time + le->accel[2] * time2 * grav;

		org[2] = bak - 1;
		contents = CL_PMpointcontents (org);
		org[2] = bak;

		if (contents & MASK_SOLID)
			onground = (qboolean)qtrue;

		if (onground) {
			le->flags &= ~CLM_ROTATE;
			VectorSet(le->avel, 0.0, 0.0, 0.0);
		}
		else
		if (le->flags & CLM_STOPPED) {
			le->flags &= ~CLM_STOPPED;
			le->flags |= CLM_BOUNCE;
			le->accel[2] = -15 * PARTICLE_GRAVITY;
			// Reset
			le->alpha = 1;
			le->time = cl.time;
			VectorCopy (org, le->org);
		}

		le->next = NULL;
		if (!tail)
			active = tail = le;
		else {
			tail->next = le;
			tail = le;
		}

		if (alpha > 1.0)
			alpha = 1;

		if (le->flags & CLM_NOSHADOW)
			ent.flags |= RF_NOSHADOW;

		if (le->flags & CLM_FRICTION) {
			// Water friction affected cl model
			if (contents & MASK_WATER) {
				if (contents & CONTENTS_LAVA) {	// kill entity in lava
					VectorSet (dir, 0, 0, 1);
					S_fastsound (org, 0, CHAN_AUTO, cl_sfx_lava, 1, ATTN_NORM);
					CL_ParticleSmoke2 (org, dir, 1, 0.3, 0, 6, qtrue);
					le->alpha = 0;
					continue;
				}
				else {
					// Add friction
					VectorScale (le->vel, 0.25, le->vel);
					VectorScale (le->accel, 0.25, le->accel);

					// Don't add friction again
					le->flags &= ~(CLM_FRICTION | CLM_ROTATE);

					// Reset
					le->time = cl.time;
					VectorCopy (org, le->org);

					// Don't stretch
					le->flags &= ~CLM_STRETCH;

				}
			}
		}
		//calc model radius and bbox
		ent.model = le->model;
		VectorSubtract (ent.maxs, ent.mins, tmpSize);
		entSize = VectorLength (tmpSize);
		
		if (le->flags & CLM_BOUNCE) {
			trace_t trace = CL_PMTraceWorld (le->lastOrg, ent.mins, ent.maxs, org, MASK_SOLID, qfalse);

			if (trace.fraction > 0 && trace.fraction < 1) {
				vec3_t	vel;
				// Reflect velocity
				float time = cl.time - (cls.frametime + cls.frametime * trace.fraction) * 1000;
				time = (time - le->time) * 0.001;

				VectorSet (vel, le->vel[0], le->vel[1], le->vel[2] + le->accel[2] * time * grav);
				VectorReflect (vel, trace.plane.normal, le->vel);

				if ((le->model == cl_mod_debris1)
					|| (le->model == cl_mod_debris2)
					|| (le->model == cl_mod_debris3)
					|| (le->model == cl_mod_debris0))
					VectorScale (le->vel, 0.2, le->vel);
				else
				if ((le->model == cl_mod_gib0)
					|| (le->model == cl_mod_gib1)
					|| (le->model == cl_mod_gib2)
					|| (le->model == cl_mod_gib3)
					|| (le->model == cl_mod_gib4)
					|| (le->model == cl_mod_gib5))
					VectorScale (le->vel, 0.3, le->vel);
				else
					VectorScale (le->vel, 0.8, le->vel);

				// Check for stop or slide along the plane
				if (trace.plane.normal[2] > 0 && le->vel[2] < 1) {
					if (trace.plane.normal[2] > 0.9) {
						VectorClear (le->vel);
						VectorClear (le->accel);
						VectorSet(le->avel, 0.0, 0.0, 0.0);
						le->alpha = 1;
						le->flags &= ~CLM_BOUNCE;
						le->flags |= CLM_STOPPED;
					}
					else {
						// FIXME: check for new plane or free fall
						float dot = DotProduct (le->vel, trace.plane.normal);
						VectorMA (le->vel, -dot, trace.plane.normal, le->vel);
						dot = DotProduct (le->accel, trace.plane.normal);
						VectorMA (le->accel, -dot, trace.plane.normal, le->accel);
					}
				}

				VectorCopy (trace.endpos, org);


				// Reset
				le->time = cl.time;
				VectorCopy (org, le->org);

				// Don't stretch
				le->flags &= ~CLM_STRETCH;

				// Nightmare stuff
				if (cl_blood->integer) {

					if (le->model == cl_mod_gib0
						|| le->model == cl_mod_gib1
						|| le->model == cl_mod_gib2
						|| le->model == cl_mod_gib3
						|| le->model == cl_mod_gib4
						|| le->model == cl_mod_gib5) {

	
						trace2 = CL_Trace (le->lastOrg, org, entSize, MASK_SOLID);
						VectorNormalize (trace.plane.normal);


						if (trace.fraction > 0 && trace.fraction < 1)
							CL_AddDecalToScene (trace.endpos,
							trace.plane.normal, 1, 1, 1, 1,
							1, 1, 1, 1, 20, 12000,
							DECAL_BLOOD9, 0,
							rand () % 360, GL_ZERO,
							GL_ONE_MINUS_SRC_COLOR);

					}

				}

				if (le->model == cl_mod_debris1)
					S_fastsound (org, 0, CHAN_AUTO, cl_sfx_debris, 1, ATTN_NORM);

				else if (le->model == cl_mod_mshell
					|| le->model == cl_mod_sshell
					|| le->model == cl_mod_debris3)
					S_fastsound (org, 0, CHAN_AUTO, cl_sfx_shell, 0.8, ATTN_NORM);

			}
		}
		// Save current origin if needed
		if (le->flags & (CLM_BOUNCE | CLM_STRETCH)) {
			VectorCopy (le->lastOrg, ent.origin);
			VectorCopy (org, le->lastOrg);	// FIXME: pause
		}
		else
			VectorCopy (org, ent.origin);

		if (CL_PMpointcontents (ent.origin) & MASK_SOLID) {

			if (le->model == cl_mod_mshell || le->model == cl_mod_sshell) { // kill gun shells in solid only!
				le->alpha = 0;
				continue;
			}
			else {
				VectorClear (le->vel);
				VectorClear (le->accel);
				VectorSet(le->avel, 0.0, 0.0, 0.0);
				le->flags &= ~PARTICLE_BOUNCE;
				le->flags |= PARTICLE_STOPED;
				le->org[2] += entSize;
			}
		}

		// add model to scene
		if (!ent.model)
			continue;

		if ((le->model == cl_mod_debris1)
			|| (le->model == cl_mod_debris2)
			|| (le->model == cl_mod_debris3)
			|| (le->model == cl_mod_debris0))
			ent.angles[0] = ent.angles[1] = ent.angles[2] = le->ang[2] + ((le->flags & CLM_ROTATE) ? (time * le->avel[2]) : 0);
		else {
			ent.angles[0] = 0;
			ent.angles[2] = 0;
			ent.angles[1] = le->ang[0] + ((le->flags & CLM_ROTATE) ? (time * le->avel[1]) : 0);
		}
		V_AddEntity (&ent);
	}


	active_clentities = active;
}

/*
=======================
Berserker@quake2  shell
brass effect. No change
=======================
*/

void CL_BrassShells (vec3_t org, vec3_t dir, int count, qboolean mshell) {
	int i, j;
	clentity_t *le;
	float d;
	vec3_t tmp;

	if (!cl_brass->value || !count)
		return;
	
	if (cl_brassTimeScale->value <= 0)
		return;

	VectorSubtract (cl.refdef.vieworg, org, tmp);

	if (DotProduct (tmp, tmp) >= (cl_brass->value * cl_brass->value))
		return;

	for (i = 0; i < count; i++) {
		if (!free_clentities)
			return;

		le = free_clentities;
		free_clentities = le->next;
		le->next = active_clentities;
		active_clentities = le;
		le->time = cl.time;
		VectorClear (le->accel);
		VectorClear (le->vel);
		le->accel[0] = le->accel[1] = 0;
		le->accel[2] = -6 * PARTICLE_GRAVITY;
		le->alpha = 1.0;
		if (mshell)
			le->alphavel = (-1.0 - frand() * 0.2) / max(1.0, cl_brassTimeScale->value);
		else
			le->alphavel = (-0.2 - frand() * 0.2) / max(1.0, cl_brassTimeScale->value);

		le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;
		if (mshell)
			le->model = cl_mod_mshell;
		else
			le->model = cl_mod_sshell;		
		
		le->ang[0] = crand() * 360;
		le->ang[1] = crand() * 360;
		le->ang[2] = crand() * 90;

		le->avel[0] = 0;
		le->avel[1] = 0;
		le->avel[2] = 0;

		d = 192 + rand() & 63;
		for (j = 0; j < 3; j++)
		{
			le->lastOrg[j] = le->org[j] = org[j];
			le->vel[j] = crand() * 24 + d * dir[j];
		}
	}
}

/*
=============================
Use client side debris effect
no "get space overflow" error
=============================
*/
void CL_Debris (vec3_t org, vec3_t dir) {
	int i, j;
	clentity_t *le;
	float d;

	for (i = 0; i < 3; i++) {
		if (!free_clentities)
			return;

		le = free_clentities;
		free_clentities = le->next;
		le->next = active_clentities;
		active_clentities = le;
		le->time = cl.time;
		VectorClear (le->accel);
		VectorClear (le->vel);
		le->flags = CLM_BOUNCE;
		le->flags |= CLM_FRICTION | CLM_ROTATE | CLM_NOSHADOW;
		le->model = cl_mod_debris1;


		d = (192 + rand ()) & 63;

		for (j = 0; j < 3; j++) {
			le->lastOrg[j] = le->org[j] = org[j];
			le->vel[j] = crand () * 200 + d * dir[j];
		}

		le->accel[0] = le->accel[1] = 0;
		le->accel[2] = -PARTICLE_GRAVITY * 6;
		le->alpha = 1.0;

		le->alphavel = -1.0 / (1.5 + frand () * 2.666);
		le->ang[0] = crand () * 360;
		le->ang[1] = crand() * 360;
		le->ang[2] = crand() * 360;

		le->avel[0] = crand() * 384 - 192;
		le->avel[1] = crand() * 384 - 192;
		le->avel[2] = crand() * 384 - 192;
	}
	// 2
	for (i = 0; i < 5; i++) {
		if (!free_clentities)
			return;

		le = free_clentities;
		free_clentities = le->next;
		le->next = active_clentities;
		active_clentities = le;
		le->time = cl.time;
		VectorClear (le->accel);
		VectorClear (le->vel);
		le->flags = CLM_BOUNCE;
		le->flags |= CLM_FRICTION | CLM_ROTATE | CLM_NOSHADOW;
		le->model = cl_mod_debris2;


		d = (192 + rand ()) & 63;

		for (j = 0; j < 3; j++) {
			le->lastOrg[j] = le->org[j] = org[j];
			le->vel[j] = crand () * 500 + d * dir[j];
		}

		le->accel[0] = le->accel[1] = 0;
		le->accel[2] = -PARTICLE_GRAVITY * 6;
		le->alpha = 1.0;

		le->alphavel = -1.0 / (1.5 + frand () * 2.666);
		le->ang[0] = crand() * 360;
		le->ang[1] = crand() * 360;
		le->ang[2] = crand() * 360;

		le->avel[0] = crand() * 500;
		le->avel[1] = crand() * 500;
		le->avel[2] = crand() * 500;
	}
	//3
	for (i = 0; i < 3; i++) {
		if (!free_clentities)
			return;

		le = free_clentities;
		free_clentities = le->next;
		le->next = active_clentities;
		active_clentities = le;
		le->time = cl.time;
		VectorClear (le->accel);
		VectorClear (le->vel);
		le->flags = CLM_BOUNCE;
		le->flags |= CLM_FRICTION | CLM_ROTATE | CLM_NOSHADOW;
		le->model = cl_mod_debris3;


		d = (192 + rand ()) & 63;

		for (j = 0; j < 3; j++) {
			le->lastOrg[j] = le->org[j] = org[j];
			le->vel[j] = crand () * 250 + d * dir[j];
		}

		le->accel[0] = le->accel[1] = 0;
		le->accel[2] = -PARTICLE_GRAVITY * 6;
		le->alpha = 1.0;

		le->alphavel = -1.0 / (1.5 + frand () * 2.666);
		le->ang[0] = crand() * 360;
		le->ang[1] = crand() * 360;
		le->ang[2] = crand() * 360;

		le->avel[0] = crand() * 500;
		le->avel[1] = crand() * 500;
		le->avel[2] = crand() * 500;
	}

}




/*
====================
Nightmare gib brass
heh! is very bloody
====================
*/

void CL_GibExplosion (vec3_t org, vec3_t dir) {
	int j;
	clentity_t *le;
	float d;


	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = CLM_BOUNCE;
	le->flags |= CLM_FRICTION | CLM_ROTATE;
	le->model = cl_mod_gib0;


	d = (192 + rand ()) & 63;

	for (j = 0; j < 3; j++) {
		le->lastOrg[j] = le->org[j] = org[j];
		le->vel[j] = crand () * 200 + d * dir[j];
	}

	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 6;
	le->alpha = 1.0;

	le->alphavel = -0.1;
	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 500;
	le->avel[1] = crand() * 500;
	le->avel[2] = crand() * 500;

	// 2

	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = CLM_BOUNCE;
	le->flags |= CLM_FRICTION | CLM_ROTATE;
	le->model = cl_mod_gib1;


	d = (192 + rand ()) & 63;

	for (j = 0; j < 3; j++) {
		le->lastOrg[j] = le->org[j] = org[j];
		le->vel[j] = crand () * 500 + d * dir[j];
	}

	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 6;
	le->alpha = 1.0;

	le->alphavel = -0.08;
	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 500;
	le->avel[1] = crand() * 500;
	le->avel[2] = crand() * 500;

	//3

	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = CLM_BOUNCE;
	le->flags |= CLM_FRICTION | CLM_ROTATE;
	le->model = cl_mod_gib2;


	d = (192 + rand ()) & 63;

	for (j = 0; j < 3; j++) {
		le->lastOrg[j] = le->org[j] = org[j];
		le->vel[j] = crand () * 250 + d * dir[j];
	}

	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 6;
	le->alpha = 1.0;

	le->alphavel = -0.09;
	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 500;
	le->avel[1] = crand() * 500;
	le->avel[2] = crand() * 500;

	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = CLM_BOUNCE;
	le->flags |= CLM_FRICTION | CLM_ROTATE;
	le->model = cl_mod_gib3;


	d = (192 + rand ()) & 63;

	for (j = 0; j < 3; j++) {
		le->lastOrg[j] = le->org[j] = org[j];
		le->vel[j] = crand () * 250 + d * dir[j];
	}

	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 6;
	le->alpha = 1.0;

	le->alphavel = -0.09;
	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 500;
	le->avel[1] = crand() * 500;
	le->avel[2] = crand() * 500;

	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = CLM_BOUNCE;
	le->flags |= CLM_FRICTION | CLM_ROTATE;
	le->model = cl_mod_gib4;


	d = (192 + rand ()) & 63;

	for (j = 0; j < 3; j++) {
		le->lastOrg[j] = le->org[j] = org[j];
		le->vel[j] = crand () * 250 + d * dir[j];
	}

	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 6;
	le->alpha = 1.0;

	le->alphavel = -0.09;
	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 500;
	le->avel[1] = crand() * 500;
	le->avel[2] = crand() * 500;

	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = CLM_BOUNCE;
	le->flags |= CLM_FRICTION | CLM_ROTATE;
	le->model = cl_mod_gib5;


	d = (192 + rand ()) & 63;

	for (j = 0; j < 3; j++) {
		le->lastOrg[j] = le->org[j] = org[j];
		le->vel[j] = crand () * 250 + d * dir[j];
	}

	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 6;
	le->alpha = 1.0;

	le->alphavel = -0.09;
	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 500;
	le->avel[1] = crand() * 500;
	le->avel[2] = crand() * 500;


}

void CL_ClientGibs (vec3_t org, vec3_t velocity) {
	clentity_t     *le;
	int i;

	//0	
	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = 0;

	le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;

	le->model = cl_mod_gib0;

	for (i = 0; i < 3; i++) {
		le->lastOrg[i] = le->org[i] = org[i];
		le->vel[i] = velocity[i] + random () * 64;
	}

	if (le->vel[0] < -300)
		le->vel[0] = -300;
	else if (le->vel[0] > 300)
		le->vel[0] = 300;
	if (le->vel[1] < -300)
		le->vel[1] = -300;
	else if (le->vel[1] > 300)
		le->vel[1] = 300;

	if (le->vel[2] < 200)
		le->vel[2] = 200;     // always some upwards
	else if (le->vel[2] > 500)
		le->vel[2] = 500;


	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 9;

	le->alpha = 1.0;
	le->alphavel = -0.09;

	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 256-128;
	le->avel[1] = crand() * 256-128;
	le->avel[2] = crand() * 256-128;

	//1	
	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = 0;

	le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;

	le->model = cl_mod_gib1;

	for (i = 0; i < 3; i++) {
		le->lastOrg[i] = le->org[i] = org[i];
		le->vel[i] = velocity[i] + random () * 64;
	}

	if (le->vel[0] < -300)
		le->vel[0] = -300;
	else if (le->vel[0] > 300)
		le->vel[0] = 300;
	if (le->vel[1] < -300)
		le->vel[1] = -300;
	else if (le->vel[1] > 300)
		le->vel[1] = 300;

	if (le->vel[2] < 200)
		le->vel[2] = 200;     // always some upwards
	else if (le->vel[2] > 500)
		le->vel[2] = 500;


	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 9;

	le->alpha = 1.0;
	le->alphavel = -0.09;

	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 256 - 128;
	le->avel[1] = crand() * 256 - 128;
	le->avel[2] = crand() * 256 - 128;

	//2	
	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = 0;

	le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;

	le->model = cl_mod_gib2;

	for (i = 0; i < 3; i++) {
		le->lastOrg[i] = le->org[i] = org[i];
		le->vel[i] = velocity[i] + random () * 64;
	}

	if (le->vel[0] < -300)
		le->vel[0] = -300;
	else if (le->vel[0] > 300)
		le->vel[0] = 300;
	if (le->vel[1] < -300)
		le->vel[1] = -300;
	else if (le->vel[1] > 300)
		le->vel[1] = 300;

	if (le->vel[2] < 200)
		le->vel[2] = 200;     // always some upwards
	else if (le->vel[2] > 500)
		le->vel[2] = 500;


	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 9;

	le->alpha = 1.0;
	le->alphavel = -0.09;

	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 256 - 128;
	le->avel[1] = crand() * 256 - 128;
	le->avel[2] = crand() * 256 - 128;


	//3	
	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = 0;

	le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;

	le->model = cl_mod_gib3;

	for (i = 0; i < 3; i++) {
		le->lastOrg[i] = le->org[i] = org[i];
		le->vel[i] = velocity[i] + random () * 64;
	}

	if (le->vel[0] < -300)
		le->vel[0] = -300;
	else if (le->vel[0] > 300)
		le->vel[0] = 300;
	if (le->vel[1] < -300)
		le->vel[1] = -300;
	else if (le->vel[1] > 300)
		le->vel[1] = 300;

	if (le->vel[2] < 200)
		le->vel[2] = 200;     // always some upwards
	else if (le->vel[2] > 500)
		le->vel[2] = 500;


	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 9;

	le->alpha = 1.0;
	le->alphavel = -0.09;

	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 256 - 128;
	le->avel[1] = crand() * 256 - 128;
	le->avel[2] = crand() * 256 - 128;

	//4	
	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = 0;

	le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;

	le->model = cl_mod_gib4;

	for (i = 0; i < 3; i++) {
		le->lastOrg[i] = le->org[i] = org[i];
		le->vel[i] = velocity[i] + random () * 64;
	}

	if (le->vel[0] < -300)
		le->vel[0] = -300;
	else if (le->vel[0] > 300)
		le->vel[0] = 300;
	if (le->vel[1] < -300)
		le->vel[1] = -300;
	else if (le->vel[1] > 300)
		le->vel[1] = 300;

	if (le->vel[2] < 200)
		le->vel[2] = 200;     // always some upwards
	else if (le->vel[2] > 500)
		le->vel[2] = 500;


	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 9;

	le->alpha = 1.0;
	le->alphavel = -0.09;

	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 256 - 128;
	le->avel[1] = crand() * 256 - 128;
	le->avel[2] = crand() * 256 - 128;

	//5	
	if (!free_clentities)
		return;

	le = free_clentities;
	free_clentities = le->next;
	le->next = active_clentities;
	active_clentities = le;
	le->time = cl.time;
	VectorClear (le->accel);
	VectorClear (le->vel);
	le->flags = 0;

	le->flags = CLM_BOUNCE | CLM_FRICTION | CLM_ROTATE;

	le->model = cl_mod_gib5;

	for (i = 0; i < 3; i++) {
		le->lastOrg[i] = le->org[i] = org[i];
		le->vel[i] = velocity[i] + random () * 64;
	}

	if (le->vel[0] < -300)
		le->vel[0] = -300;
	else if (le->vel[0] > 300)
		le->vel[0] = 300;
	if (le->vel[1] < -300)
		le->vel[1] = -300;
	else if (le->vel[1] > 300)
		le->vel[1] = 300;

	if (le->vel[2] < 200)
		le->vel[2] = 200;     // always some upwards
	else if (le->vel[2] > 500)
		le->vel[2] = 500;


	le->accel[0] = le->accel[1] = 0;
	le->accel[2] = -PARTICLE_GRAVITY * 9;

	le->alpha = 1.0;
	le->alphavel = -0.09;

	le->ang[0] = crand() * 360;
	le->ang[1] = crand() * 360;
	le->ang[2] = crand() * 360;

	le->avel[0] = crand() * 256 - 128;
	le->avel[1] = crand() * 256 - 128;
	le->avel[2] = crand() * 256 - 128;
}

void CL_ClearClEntities () {
	int i;

	free_clentities = &clentities[0];
	active_clentities = NULL;

	for (i = 0; i < MAX_CLENTITIES; i++)
		clentities[i].next = &clentities[i + 1];
	clentities[MAX_CLENTITIES - 1].next = NULL;
}
