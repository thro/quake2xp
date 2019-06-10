/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2004-2007 Quake2xp Team.

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
// cl_particles.c -- client side particle effects

#include "client.h"
#include "../ref_gl/r_local.h"	

static vec3_t avelocities[NUM_VERTEX_NORMALS];

/*
==============================================================

PARTICLE MANAGEMENT

==============================================================
*/
// original Q2 palette
static const byte r_originalPalette[] = {
	0, 0, 0, 15, 15, 15, 31, 31, 31, 47, 47, 47,
	63, 63, 63, 75, 75, 75, 91, 91, 91, 107, 107, 107,
	123, 123, 123, 139, 139, 139, 155, 155, 155, 171, 171, 171,
	187, 187, 187, 203, 203, 203, 219, 219, 219, 235, 235, 235,
	99, 75, 35, 91, 67, 31, 83, 63, 31, 79, 59, 27,
	71, 55, 27, 63, 47, 23, 59, 43, 23, 51, 39, 19,
	47, 35, 19, 43, 31, 19, 39, 27, 15, 35, 23, 15,
	27, 19, 11, 23, 15, 11, 19, 15, 7, 15, 11, 7,
	95, 95, 111, 91, 91, 103, 91, 83, 95, 87, 79, 91,
	83, 75, 83, 79, 71, 75, 71, 63, 67, 63, 59, 59,
	59, 55, 55, 51, 47, 47, 47, 43, 43, 39, 39, 39,
	35, 35, 35, 27, 27, 27, 23, 23, 23, 19, 19, 19,
	143, 119, 83, 123, 99, 67, 115, 91, 59, 103, 79, 47,
	207, 151, 75, 167, 123, 59, 139, 103, 47, 111, 83, 39,
	235, 159, 39, 203, 139, 35, 175, 119, 31, 147, 99, 27,
	119, 79, 23, 91, 59, 15, 63, 39, 11, 35, 23, 7,
	167, 59, 43, 159, 47, 35, 151, 43, 27, 139, 39, 19,
	127, 31, 15, 115, 23, 11, 103, 23, 7, 87, 19, 0,
	75, 15, 0, 67, 15, 0, 59, 15, 0, 51, 11, 0,
	43, 11, 0, 35, 11, 0, 27, 7, 0, 19, 7, 0,
	123, 95, 75, 115, 87, 67, 107, 83, 63, 103, 79, 59,
	95, 71, 55, 87, 67, 51, 83, 63, 47, 75, 55, 43,
	67, 51, 39, 63, 47, 35, 55, 39, 27, 47, 35, 23,
	39, 27, 19, 31, 23, 15, 23, 15, 11, 15, 11, 7,
	111, 59, 23, 95, 55, 23, 83, 47, 23, 67, 43, 23,
	55, 35, 19, 39, 27, 15, 27, 19, 11, 15, 11, 7,
	179, 91, 79, 191, 123, 111, 203, 155, 147, 215, 187, 183,
	203, 215, 223, 179, 199, 211, 159, 183, 195, 135, 167, 183,
	115, 151, 167, 91, 135, 155, 71, 119, 139, 47, 103, 127,
	23, 83, 111, 19, 75, 103, 15, 67, 91, 11, 63, 83,
	7, 55, 75, 7, 47, 63, 7, 39, 51, 0, 31, 43,
	0, 23, 31, 0, 15, 19, 0, 7, 11, 0, 0, 0,
	139, 87, 87, 131, 79, 79, 123, 71, 71, 115, 67, 67,
	107, 59, 59, 99, 51, 51, 91, 47, 47, 87, 43, 43,
	75, 35, 35, 63, 31, 31, 51, 27, 27, 43, 19, 19,
	31, 15, 15, 19, 11, 11, 11, 7, 7, 0, 0, 0,
	151, 159, 123, 143, 151, 115, 135, 139, 107, 127, 131, 99,
	119, 123, 95, 115, 115, 87, 107, 107, 79, 99, 99, 71,
	91, 91, 67, 79, 79, 59, 67, 67, 51, 55, 55, 43,
	47, 47, 35, 35, 35, 27, 23, 23, 19, 15, 15, 11,
	159, 75, 63, 147, 67, 55, 139, 59, 47, 127, 55, 39,
	119, 47, 35, 107, 43, 27, 99, 35, 23, 87, 31, 19,
	79, 27, 15, 67, 23, 11, 55, 19, 11, 43, 15, 7,
	31, 11, 7, 23, 7, 0, 11, 0, 0, 0, 0, 0,
	119, 123, 207, 111, 115, 195, 103, 107, 183, 99, 99, 167,
	91, 91, 155, 83, 87, 143, 75, 79, 127, 71, 71, 115,
	63, 63, 103, 55, 55, 87, 47, 47, 75, 39, 39, 63,
	35, 31, 47, 27, 23, 35, 19, 15, 23, 11, 7, 7,
	155, 171, 123, 143, 159, 111, 135, 151, 99, 123, 139, 87,
	115, 131, 75, 103, 119, 67, 95, 111, 59, 87, 103, 51,
	75, 91, 39, 63, 79, 27, 55, 67, 19, 47, 59, 11,
	35, 47, 7, 27, 35, 0, 19, 23, 0, 11, 15, 0,
	0, 255, 0, 35, 231, 15, 63, 211, 27, 83, 187, 39,
	95, 167, 47, 95, 143, 51, 95, 123, 51, 255, 255, 255,
	255, 255, 211, 255, 255, 167, 255, 255, 127, 255, 255, 83,
	255, 255, 39, 255, 235, 31, 255, 215, 23, 255, 191, 15,
	255, 171, 7, 255, 147, 0, 239, 127, 0, 227, 107, 0,
	211, 87, 0, 199, 71, 0, 183, 59, 0, 171, 43, 0,
	155, 31, 0, 143, 23, 0, 127, 15, 0, 115, 7, 0,
	95, 0, 0, 71, 0, 0, 47, 0, 0, 27, 0, 0,
	239, 0, 0, 55, 55, 255, 255, 0, 0, 0, 0, 255,
	43, 43, 35, 27, 27, 23, 19, 19, 15, 235, 151, 127,
	195, 115, 83, 159, 87, 51, 123, 63, 27, 235, 211, 199,
	199, 171, 155, 167, 139, 119, 135, 107, 87, 159, 91, 83
};


cparticle_t *active_particles, *free_particles;

cparticle_t particles[MAX_PARTICLES];
int cl_numparticles = MAX_PARTICLES;
void CL_ParticleSmoke2 (vec3_t org, vec3_t dir, float r, float g, float b,
	int count, qboolean add);

/*
===============
CL_ClearParticles
===============
*/
void CL_ClearParticles (void) {
	int i;

	free_particles = &particles[0];
	active_particles = NULL;

	for (i = 0; i < cl_numparticles; i++)
		particles[i].next = &particles[i + 1];
	particles[cl_numparticles - 1].next = NULL;

	for (i = 0; i < 256; i++) {
		cl_indexPalette[i][0] = r_originalPalette[i * 3 + 0] * (1.0 / 255);
		cl_indexPalette[i][1] = r_originalPalette[i * 3 + 1] * (1.0 / 255);
		cl_indexPalette[i][2] = r_originalPalette[i * 3 + 2] * (1.0 / 255);
	}


}


/*
===============
CL_AddParticles
===============
*/


void VectorReflect (const vec3_t v, const vec3_t normal, vec3_t out) {
	float d;

	d = 2.0 * (v[0] * normal[0] + v[1] * normal[1] + v[2] * normal[2]);

	out[0] = v[0] - normal[0] * d;
	out[1] = v[1] - normal[1] * d;
	out[2] = v[2] - normal[2] * d;
}

trace_t CL_PMTrace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);


void CL_AddParticles (void) {

	cparticle_t *p, *next;
	vec3_t org, mins, maxs, kls, old;
	vec3_t color, lcol;
	cparticle_t *active, *tail;
	float alpha, lightradius;
	float time, time2, grav = Cvar_VariableValue ("sv_gravity");
	float size, len, lerp, endLerp;
	float orient, backup;
	int sFactor, dFactor, flags;
	int cont;
	qboolean ground;

	if (!grav)
		grav = 1;
	else
		grav /= 800;

	active = NULL;
	tail = NULL;

	for (p = active_particles; p; p = next) {
		next = p->next;

		endLerp = (float)(cl.time - p->time) / (p->endTime - p->time);
		lerp = 1.0 - endLerp;

		time = (cl.time - p->time) * 0.001;
		alpha = p->alpha + time * p->alphavel;
		len = p->len * lerp + p->endLen * endLerp;

		if (alpha <= 0) {		// faded out
			p->next = free_particles;
			free_particles = p;
			continue;
		}
		size = p->size + time * p->sizeVel;
		if (size <= 0) {		// kill
			p->next = free_particles;
			free_particles = p;
			continue;
		}

		if (p->flags & PARTICLE_CLAMP) {
			if (size >= p->sizeVel)
				size = p->sizeVel; //clamp!
		}

		if (p->endTime <= cl.time) {	// kill
			p->next = free_particles;
			free_particles = p;
			continue;
		}

		if (len <= 0.0 && p->len > p->endLen) {	// kill
			p->next = free_particles;
			free_particles = p;
			continue;
		}

		if (alpha > 1.0)
			alpha = 1;

		color[0] = p->color[0] + p->colorVel[0] * time;
		color[1] = p->color[1] + p->colorVel[1] * time;
		color[2] = p->color[2] + p->colorVel[2] * time;

		time2 = time * time;

		org[0] = p->org[0] + p->vel[0] * time + p->accel[0] * time2;
		org[1] = p->org[1] + p->vel[1] * time + p->accel[1] * time2;
		backup = p->org[2] + p->vel[2] * time + p->accel[2] * time2 * grav;

		org[2] = backup - 5;
		cont = CL_PMpointcontents (org);
		org[2] = backup;
		ground = cont & MASK_SOLID;

		VectorCopy (p->org, old);

		sFactor = p->sFactor;
		dFactor = p->dFactor;
		flags = p->flags;
		orient = p->orient;

		// = Particle dlight
		lightradius = p->lightradius;
		VectorCopy (p->lcolor, lcol);
		if (p->flags & PARTICLE_LIGHTING) {
			V_AddLight (org, lightradius, lcol[0], lcol[1], lcol[2], vec3_origin, 0, 0);
		}
		VectorSet (kls, org[0], org[1], org[2] + size * 2); //killed particle origin -  in air, water

		if (p->flags & PARTICLE_UNDERWATER) {

			if (!(CL_PMpointcontents (kls) & MASK_WATER)) { //in water
				p->next = free_particles;
				free_particles = p;
				continue;
			}
		}

		if (p->flags & PARTICLE_NONSOLID) {

			if (CL_PMpointcontents (kls) & MASK_SHOT) {
				p->next = free_particles;
				free_particles = p;
				continue;
			}
		}

		if (p->flags & PARTICLE_AIRONLY) { // on air :-)

			if (CL_PMpointcontents (kls) & MASK_WATER) {
				p->next = free_particles;
				free_particles = p;
				continue;
			}
		}

		if (!ground) {

			if (p->flags & PARTICLE_STOPED) {

				p->flags &= ~PARTICLE_STOPED;
				p->flags |= PARTICLE_BOUNCE;
				p->accel[2] = -PARTICLE_GRAVITY * 15;
				//Reset
				p->time = cl.time;
				VectorCopy (org, p->org);
				p->len = p->endLen = len = 1.0;
				VectorCopy (color, p->color);
				p->flags &= ~PARTICLE_DIRECTIONAL;
				p->flags &= ~PARTICLE_STRETCH;
			}
		}

		p->next = NULL;
		if (!tail)
			active = tail = p;
		else {
			tail->next = p;
			tail = p;
		}

		if (p->flags & PARTICLE_FRICTION) {
			// Water friction affected particle
			int contents = CL_PMpointcontents (org);
			if (contents & MASK_WATER) {
				// Add friction
				if (contents & CONTENTS_WATER) {
					VectorScale (p->vel, 0.25, p->vel);
					VectorScale (p->accel, 0.25, p->accel);
				}
				else if (contents & CONTENTS_SLIME) {
					VectorScale (p->vel, 0.20, p->vel);
					VectorScale (p->accel, 0.20, p->accel);
				}
				else if (contents & CONTENTS_LAVA) {
					VectorScale (p->vel, 0.10, p->vel);
					VectorScale (p->accel, 0.10, p->accel);
				}
				else if (contents & MASK_SOLID) {
					p->alpha = 0;	// kill the particle
					continue;	// don't add the particle
				}
				// Don't add friction again
				p->flags &= ~PARTICLE_FRICTION;

				// Reset
				p->time = cl.time;
				p->len = p->endLen = len = 1.0;
				VectorCopy (org, p->org);
				VectorCopy (color, p->color);
			}
		}


		if (p->flags & PARTICLE_BOUNCE) {
			trace_t trace;
			VectorSet (mins, -size, -size, -size);
			VectorSet (maxs, size, size, size);
			VectorScale (mins, 2, mins);
			VectorScale (maxs, 2, maxs);

			trace = CL_PMTraceWorld (p->oldOrg, mins, maxs, org, MASK_SOLID, qfalse);

			if (trace.fraction > 0 && trace.fraction < 1) {
				vec3_t	vel;
				float time = cl.time - (cls.frametime + cls.frametime * trace.fraction) * 1000;
				time = (time - p->time) * 0.001;

				VectorSet (vel, p->vel[0], p->vel[1], p->vel[2] + p->accel[2] * time * grav);
				VectorReflect (vel, trace.plane.normal, p->vel);
				VectorScale (p->vel, 0.3, p->vel);

				// Check for stop or slide along the plane
				if (trace.plane.normal[2] > 0 && p->vel[2] < 1) {
					if (trace.plane.normal[2] > 0.9) {
						VectorClear (p->vel);
						VectorClear (p->accel);
						p->flags &= ~PARTICLE_BOUNCE;
						p->flags |= PARTICLE_STOPED;
					}
					else {
						// FIXME: check for new plane or free fall
						float dot = DotProduct (p->vel, trace.plane.normal);
						VectorMA (p->vel, -dot, trace.plane.normal, p->vel);
						dot = DotProduct (p->accel, trace.plane.normal);
						VectorMA (p->accel, -dot, trace.plane.normal, p->accel);
					}
				}

				VectorCopy (trace.endpos, org);

				// Reset
				p->time = cl.time;
				VectorCopy (org, p->org);
				p->len = p->endLen = len = 1.0;
				VectorCopy (color, p->color);
				p->flags &= ~PARTICLE_DIRECTIONAL;
				p->flags &= ~PARTICLE_STRETCH;
			}
		}

		if (cl_blood->integer) {

			if (p->type == PT_BLOODDRIP || p->type == PT_xBLOODSPRAY) {
				trace_t trace;
				trace = CL_Trace (p->oldOrg, org, p->size * 2, MASK_SOLID);

				if (trace.fraction != 1.0) {
					p->alpha = 0;	// kill the particle after marking

					if (p->type == PT_BLOODDRIP) {

						CL_AddDecalToScene (org, trace.plane.normal,
							1, 1, 1, 1,
							1, 1, 1, 1,
							7, 12000,
							DECAL_BLOOD1 + (rand () & 9), 0,
							frand () * 360, GL_ZERO,
							GL_ONE_MINUS_SRC_COLOR);

					}
					else if (p->type == PT_xBLOODSPRAY) {

						CL_AddDecalToScene (org, trace.plane.normal,
							1, 1, 0, 1,
							1, 1, 0, 1,
							17, 12000,
							DECAL_ACIDMARK, DF_OVERBRIGHT,
							frand () * 360, GL_ONE, GL_ONE);

					}


					continue;
				}

			}
		}

		V_AddParticle (org, p->length, color, alpha, p->type, size,
			p->sFactor, p->dFactor, p->flags, p->time,
			p->orient, len, old, p->dir);

	}


	active_particles = active;
}


/*
===============
CL_ParticleEffect

Wall impact puffs
===============
*/
void CL_ParticleEffect (vec3_t org, vec3_t dir, float r, float b, float g,
	int count) {
	int i, j;
	cparticle_t *p;
	float d;


	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->flags = PARTICLE_DEFAULT;
		p->orient = 0;
		p->len = 0;
		p->endLen = 0;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = r;
		p->color[1] = g;
		p->color[2] = b;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 0;
		d = rand () & 31;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = crand () * 20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);

	}
}


void CL_ParticleBlood (vec3_t org, vec3_t dir, int count) {
	int i, j;
	cparticle_t *p;
	float d;

	if (!cl_blood->integer) 
		return;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->type = PT_BLOODDRIP;
		p->orient = 0;
		p->flags = PARTICLE_DIRECTIONAL;
		p->flags |= PARTICLE_NONSOLID;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->size = 1.3;
		p->sizeVel = 0;
		p->time = cl.time;
		p->endTime = cl.time + 10000;
		p->color[0] = 0.1;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->len = 10;
		p->endLen = 90;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = dir[j] * 30 + crand () * 60;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 4;
		p->alpha = 1;

		p->alphavel = 0;
		VectorCopy (p->org, p->oldOrg);
	}

		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->vel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->flags |= PARTICLE_NONSOLID;
		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.1;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_BLOODMIST;
		p->size = 0.5;
		p->sizeVel = 20;

		d = rand () & 7;
		for (j = 0; j < 3; j++)
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];


		p->accel[0] = p->accel[1] = p->accel[2] = 0;
		p->alpha = 1;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);

}

void CL_ParticleBlood2 (vec3_t org, vec3_t dir, int count) {
	int i, j;
	cparticle_t *p;

	if (!cl_blood->integer) 
		return;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->type = PT_xBLOODSPRAY;
		p->orient = 0;
		p->flags = PARTICLE_DIRECTIONAL;
		p->flags |= PARTICLE_NONSOLID;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->size = 1.3;
		p->sizeVel = 0;
		p->time = cl.time;
		p->endTime = cl.time + 10000;
		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->len = 5;
		p->endLen = 80;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = dir[j] * 30 + crand () * 40;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 4;
		p->alpha = 1;

		p->alphavel = 0;
		VectorCopy (p->org, p->oldOrg);
	}
}


/*
===============
CL_LaserParticle
===============
*/
void CL_LaserParticle (vec3_t org, vec3_t dir, int count) {
	int i, j;
	cparticle_t *p;
	float d;


	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->flags = PARTICLE_BOUNCE;
		p->flags |= PARTICLE_FRICTION;
		p->orient = 0;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = -1;
		p->colorVel[2] = 0;

		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 0;
		d = rand () & 7;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = crand () * 20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
		VectorCopy (p->org, p->oldOrg);
	}
}

void CL_LaserParticle2 (vec3_t org, vec3_t dir, int color, int count) {
	int i, j;
	cparticle_t *p;
	float d;

	color &= 0xff;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->flags = PARTICLE_BOUNCE;
		p->flags |= PARTICLE_FRICTION;
		p->orient = 0;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = cl_indexPalette[color][0];
		p->color[1] = cl_indexPalette[color][1];
		p->color[2] = cl_indexPalette[color][2];


		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_BLASTER;
		p->size = 1;
		p->sizeVel = 0;
		d = rand () & 7;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = crand () * 20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
		VectorCopy (p->org, p->oldOrg);
	}
}



void CL_ParticleSmoke (vec3_t org, vec3_t dir, int count) {
	int i, j;
	cparticle_t *p;
	float d;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_VERTEXLIGHT;
		p->flags |= PARTICLE_AIRONLY;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.4;
		p->color[1] = 0.4;
		p->color[2] = 0.4;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_SMOKE;
		p->size = 4;
		p->sizeVel = 7;

		d = rand () & 7;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = crand () * 20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
	}
}


void CL_ParticleSmoke2 (vec3_t org, vec3_t dir, float r, float g, float b,
	int count, qboolean add) {
	int i, j;
	cparticle_t *p;
	float d;


	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->type = PT_SMOKE;
		p->flags = PARTICLE_AIRONLY;
		p->orient = frand () * 360;
		if (add) {
			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE;
		}
		else {
			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		}

		p->size = 3;
		p->sizeVel = 20;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->color[0] = r;
		p->color[1] = g;
		p->color[2] = b;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		d = rand () & 15;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + d * dir[j];
			p->vel[j] = dir[j] * 30;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = 9;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (0.5 + frand () * 0.5);

	}
}

void CL_ParticleGunSmoke (vec3_t org, vec3_t dir, int count) {
	int i, j;
	cparticle_t *p;
	float d;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_VERTEXLIGHT;
		p->flags |= PARTICLE_AIRONLY;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.45;
		p->color[1] = 0.45;
		p->color[2] = 0.45;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_SMOKE;
		p->size = 1;
		p->sizeVel = 15;

		d = rand () & 7;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = crand () * 20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = PARTICLE_GRAVITY*0.5;
		p->alpha = 0.45;

		p->alphavel = -1.0 / (0.5 + frand () * 5.3);
	}
}



void CL_ParticleSpark (vec3_t org, vec3_t dir, int count) {
	int i, j;
	cparticle_t *p;
	float d;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_OVERBRIGHT;
		p->flags |= PARTICLE_DIRECTIONAL;
		p->time = cl.time;
		p->endTime = cl.time + 1000;
		p->sFactor = GL_ONE;
		p->dFactor = GL_ONE;
		p->len = 4;
		p->endLen = 9;
		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 0.7;

		p->colorVel[0] = 0;
		p->colorVel[1] = -0.40;
		p->colorVel[2] = -1.0;

		p->type = PT_SPARK;
		p->size = 0.4;
		p->sizeVel = 0;
		d = rand () & 7;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = crand () * 20;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 5;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (1.5 + frand () * 1.666);
		VectorCopy (p->org, p->oldOrg);
	}

	for (i = 0; i < 50; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->len = 0;
		p->endLen = 0;
		p->orient = 0;
		p->flags = PARTICLE_OVERBRIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 1000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->color[0] = 1;
		p->color[1] = 0.5;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = -0.40;
		p->colorVel[2] = 0;

		p->type = PT_DEFAULT;
		p->size = 0.5;
		p->sizeVel = 0;
		d = rand () & 19;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = crand () * 70;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 5;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (1.5 + frand () * 1.666);
		VectorCopy (p->org, p->oldOrg);
	}
}

void CL_ParticleArmorSpark (vec3_t org, vec3_t dir, int count,
	qboolean power) {
	int i, j;
	cparticle_t *p;
	float d;

	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_OVERBRIGHT;
		p->flags |= PARTICLE_DIRECTIONAL;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_ONE;
		p->dFactor = GL_ONE;
		p->len = 1;
		p->endLen = 500;
		if (power) {
			p->color[0] = 0;
			p->color[1] = 1;
			p->color[2] = 0;

			p->colorVel[0] = 0;
			p->colorVel[1] = -1;
			p->colorVel[2] = 0;
		}
		else if (!power) {
			p->color[0] = 1;
			p->color[1] = 1;
			p->color[2] = 0.7;

			p->colorVel[0] = 0;
			p->colorVel[1] = -1;
			p->colorVel[2] = -1.5;
		}

		p->type = PT_SPARK;
		p->size = 0.4;
		p->sizeVel = 0;
		d = rand () & 9;
		for (j = 0; j < 3; j++) {
			// p->org[j] = org[j]+ d*dir[j];
			// p->vel[j] = dir[j] * 30;
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = dir[j] * 30 + crand () * 30;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 8;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (1.5 + frand () * 1.666);
	}
}


void CL_ParticleTracer (vec3_t start, vec3_t end) {
	int j;
	cparticle_t *p;
	float d;
	vec3_t dir;
	float dist = 5000;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;

	VectorClear (p->accel);
	VectorSubtract (end, start, dir);
	VectorNormalize (dir);

	active_particles = p;
	p->orient = 0;
	p->flags = PARTICLE_DIRECTIONAL;
	p->flags |= PARTICLE_NONSOLID;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
	p->len = 100;
	p->endLen = 200;
	p->color[0] = 1;
	p->color[1] = 0.35;
	p->color[2] = 0;

	p->colorVel[0] = 0;
	p->colorVel[1] = 0;
	p->colorVel[2] = 0;

	p->type = PT_BLASTER;
	p->size = 0.3;
	p->sizeVel = 0;
	d = rand () & 7;
	for (j = 0; j < 3; j++) {
		p->org[j] = start[j];
		p->vel[j] = dir[j] * dist;
	}


	p->alpha = 1.0;

	p->alphavel = -1.0 / (1.5 + frand () * 1.666);

}

void CL_ParticleSplash (vec3_t org, vec3_t dir, float r, float g, float b) {

	float d;
	int j, i;
	cparticle_t *p;


	for (i = 0; i < 40; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_AIRONLY;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = r;
		p->color[1] = g;
		p->color[2] = b;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_DEFAULT;
		p->size = 0.85;
		p->sizeVel = 0;

		d = rand () & 9;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = dir[j] * 60 + crand () * 90;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 4.5;
		p->alpha = 1;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
	}


	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	VectorClear (p->accel);
	VectorClear (p->vel);
	p->orient = 0;
	p->flags = PARTICLE_DIRECTIONAL;
	p->flags |= PARTICLE_AIRONLY;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE;

	p->color[0] = r;
	p->color[1] = g;
	p->color[2] = b;

	p->colorVel[0] = 0;
	p->colorVel[1] = 0;
	p->colorVel[2] = 0;

	p->alpha = 0.5;
	p->alphavel = -1.0 / (0.5 + frand () * 0.5);

	p->type = PT_WATERPULME;
	p->size = 7;
	p->sizeVel = 0;
	p->len = 10;
	p->endLen = 2000;


	for (j = 0; j < 3; j++) {
		p->org[j] = org[j];
		p->vel[j] = dir[j];
	}

	for (i = 0; i < 3; i++) {

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		VectorClear (p->vel);
		VectorCopy (dir, p->dir);
		VectorNormalize (p->dir);
		p->orient = 0;
		p->flags = PARTICLE_ALIGNED;
		p->flags |= PARTICLE_AIRONLY;
		p->flags |= PARTICLE_NOFADE;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = r;
		p->color[1] = b;
		p->color[2] = g;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->alpha = 1;
		p->alphavel = -1.0 / (0.5 + frand () * 0.5);

		p->type = PT_WATERCIRCLE;
		p->size = 10 + frand () * 6;
		p->sizeVel = 30;
		p->len = 0;
		p->endLen = 0;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = p->dir[j];
		}
	}


}

void CL_ParticleSplashSlime (vec3_t org, vec3_t dir) {

	float d;
	int j, i;
	cparticle_t *p;


	for (i = 0; i < 40; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_AIRONLY;
		p->flags |= PARTICLE_OVERBRIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 0.2;
		p->color[1] = 1.0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_SPLASH;
		p->size = 3;
		p->sizeVel = -6;

		d = rand () & 9;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = dir[j] * 60 + crand () * 90;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 2.5;
		p->alpha = 1;

		p->alphavel = -1.0 / (0.5 + frand () * 0.5);
	}


	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	VectorClear (p->accel);
	VectorClear (p->vel);
	p->orient = 0;
	p->flags = PARTICLE_DIRECTIONAL;
	p->flags |= PARTICLE_AIRONLY;
	p->flags |= PARTICLE_OVERBRIGHT;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE;

	p->color[0] = 0.2;
	p->color[1] = 1.0;
	p->color[2] = 0;

	p->colorVel[0] = 0;
	p->colorVel[1] = 0;
	p->colorVel[2] = 0;

	p->alpha = 0.5;
	p->alphavel = -1.0 / (0.5 + frand () * 0.5);

	p->type = PT_WATERPULME;
	p->size = 7;
	p->sizeVel = 0;
	p->len = 10;
	p->endLen = 2000;


	for (j = 0; j < 3; j++) {
		p->org[j] = org[j];
		p->vel[j] = dir[j];
	}

	for (i = 0; i < 4; i++) {

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		VectorClear (p->vel);
		VectorCopy (dir, p->dir);
		VectorNormalize (p->dir);
		p->orient = 0;
		p->flags = PARTICLE_ALIGNED;
		p->flags |= PARTICLE_AIRONLY;
		p->flags |= PARTICLE_OVERBRIGHT;
		p->flags |= PARTICLE_NOFADE;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 0.2;
		p->color[1] = 1.0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->alpha = 1;
		p->alphavel = -1.0 / (0.5 + frand () * 0.5);

		p->type = PT_WATERCIRCLE;
		p->size = 10 + frand () * 6;
		p->sizeVel = 30;
		p->len = 0;
		p->endLen = 0;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = p->dir[j];
		}
	}


}


void CL_ParticleSplashLava (vec3_t org, vec3_t dir) {

	float d;
	int j, i;
	cparticle_t *p;


	for (i = 0; i < 80; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_AIRONLY;
		p->flags |= PARTICLE_OVERBRIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 1;
		p->color[1] = 0.3;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_SPLASH;
		p->size = 3;
		p->sizeVel = -5;

		d = rand () & 9;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = dir[j] * 60 + crand () * 90;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 2.5;
		p->alpha = 1;

		p->alphavel = -1.0 / (0.5 + frand () * 0.5);
	}


	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	VectorClear (p->accel);
	VectorClear (p->vel);
	p->orient = 0;
	p->flags = PARTICLE_DIRECTIONAL;
	p->flags |= PARTICLE_AIRONLY;
	p->flags |= PARTICLE_OVERBRIGHT;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE;

	p->color[0] = 1;
	p->color[1] = 0.3;
	p->color[2] = 0;

	p->colorVel[0] = 0;
	p->colorVel[1] = 0;
	p->colorVel[2] = 0;

	p->alpha = 1;
	p->alphavel = -1.0 / (0.5 + frand () * 0.5);

	p->type = PT_WATERPULME;
	p->size = 7;
	p->sizeVel = 0;
	p->len = 10;
	p->endLen = 800;


	for (j = 0; j < 3; j++) {
		p->org[j] = org[j];
		p->vel[j] = dir[j];
	}

	for (i = 0; i < 15; i++) {

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		VectorClear (p->vel);
		VectorCopy (dir, p->dir);
		VectorNormalize (p->dir);
		p->orient = 0;
		p->flags = PARTICLE_ALIGNED;
		p->flags |= PARTICLE_AIRONLY;
		p->flags |= PARTICLE_OVERBRIGHT;
		p->flags |= PARTICLE_NOFADE;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 1;
		p->color[1] = 0.3;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->alpha = 1;
		p->alphavel = -1.0 / (0.5 + frand () * 0.5);

		p->type = PT_WATERCIRCLE;
		p->size = 10 + frand () * 6;
		p->sizeVel = 20;
		p->len = 0;
		p->endLen = 0;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = p->dir[j];
		}
	}


}


void CL_ParticleHeadBlood (vec3_t org) {
	int j, i;
	cparticle_t *p;
	vec3_t end, tmp;
	float d;

	if (!cl_blood->integer) 
		return;

	VectorCopy (org, end);
	end[2] += 150;
	VectorSubtract (end, org, tmp);
	VectorNormalize (tmp);

	for (i = 0; i < 15; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);

		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->flags = PARTICLE_DIRECTIONAL;
		p->flags |= PARTICLE_NONSOLID;
		p->orient = 0;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->type = PT_BLOODDRIP;
		p->size = 1.3;
		p->sizeVel = 0;
		p->alpha = 1;
		p->alphavel = 0;

		p->color[0] = 0.1;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		p->len = 7;
		p->endLen = 0;
		d = rand () & 5;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 2) - 7) + d * tmp[j];
			p->vel[j] = tmp[j] * 150 + crand () * 190;
		}
		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 6.5;

		VectorCopy (p->org, p->oldOrg);

	}

	for (i = 0; i < 3; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->vel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.2;
		p->color[1] = 0.2;
		p->color[2] = 0.2;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_BLOODMIST;
		p->size = 7;
		p->sizeVel = 30;
		p->org[2] += 10;
		for (j = 0; j < 3; j++)
			p->org[j] = org[j];


		p->accel[0] = p->accel[1] = p->accel[2] = 0;
		p->alpha = 0.7;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
	}


}

void CL_ParticleGibBlood (vec3_t org) {
	int j, i;
	cparticle_t *p;
	vec3_t end, tmp;
	float d;

	if (!cl_blood->integer) 
		return;

	VectorCopy (org, end);
	end[2] += 150;
	VectorSubtract (end, org, tmp);
	VectorNormalize (tmp);

	for (i = 0; i < 5; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;


		VectorClear (p->accel);

		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->flags = PARTICLE_DIRECTIONAL;
		p->flags |= PARTICLE_NONSOLID;
		p->orient = 0;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->type = PT_BLOODDRIP;
		p->size = 1;
		p->sizeVel = 0;
		p->alpha = 1;
		p->alphavel = 0;

		p->color[0] = 0.1;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		p->len = 7;
		p->endLen = 0;
		d = rand () & 5;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 2) - 7) + d * tmp[j];
			p->vel[j] = tmp[j] * 150 + crand () * 190;
		}
		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 6.5;

		VectorCopy (p->org, p->oldOrg);

	}

	for (i = 0; i < 4; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->vel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.1;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_BLOODMIST;
		p->size = 10 + (1.5 * crand ());
		p->sizeVel = 25 - (0.5 * crand ());
		org[2] -= 20;
		d = rand () & 7;
		for (j = 0; j < 3; j++)
			p->org[j] = org[j] + ((rand () & 7) - 4) + d;


		p->accel[0] = p->accel[1] = p->accel[2] = 0;
		p->alpha = 1;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
	}

}

void CL_ParticleGibBlood2 (vec3_t org) {
	int j, i;
	cparticle_t *p;
	vec3_t end, tmp;
	float d;

	if (!cl_blood->integer) 
		return;

	VectorCopy (org, end);
	end[2] += 150;
	VectorSubtract (end, org, tmp);
	VectorNormalize (tmp);

	for (i = 0; i < 5; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;


		VectorClear (p->accel);

		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->orient = 0;
		p->flags = PARTICLE_DIRECTIONAL;
		p->flags |= PARTICLE_NONSOLID;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->type = PT_xBLOODSPRAY;
		p->size = 1;
		p->sizeVel = 0;
		p->alpha = 1;
		p->alphavel = 0;

		p->color[0] = 0.5;
		p->color[1] = 1;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		p->len = 7;
		p->endLen = 0;
		d = rand () & 5;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 2) - 7) + d * tmp[j];
			p->vel[j] = tmp[j] * 150 + crand () * 190;
		}
		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 6.5;

		VectorCopy (p->org, p->oldOrg);

	}

	for (i = 0; i < 4; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->vel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 5000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.1;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_BLOODMIST;
		p->size = 10 + (1.5 * crand ());
		p->sizeVel = 25 - (0.5 * crand ());
		org[2] -= 20;
		d = rand () & 7;
		for (j = 0; j < 3; j++)
			p->org[j] = org[j] + ((rand () & 7) - 4) + d;


		p->accel[0] = p->accel[1] = p->accel[2] = 0;
		p->alpha = 1;

		p->alphavel = -1.0 / (0.5 + frand () * 0.3);
	}

}


void CL_Explosion (vec3_t org) {
	cparticle_t *p;
	int i, j;
	float d;
	int cont;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->orient = 0;
	p->flags = PARTICLE_OVERBRIGHT;
	p->flags |= PARTICLE_LIGHTING;
	p->time = cl.time;
	p->endTime = cl.time + 400;
	p->size = 50;
	p->sizeVel = 0;
	p->alpha = 1;
	p->alphavel = -1;

	p->lightradius = 350;
	p->lcolor[0] = 1.0;
	p->lcolor[1] = 0.5;
	p->lcolor[2] = 0.5;


	p->sFactor = GL_ONE;
	p->dFactor = GL_ONE;

	p->color[0] = 1;
	p->color[1] = 1;
	p->color[2] = 1;

	p->colorVel[0] = 0;
	p->colorVel[1] = 0;
	p->colorVel[2] = 0;
	p->type = PT_EXPLODE;

	VectorCopy (org, p->org);
	VectorClear (p->vel);
	VectorClear (p->accel);
	//Sparks
	for (i = 0; i < 99; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		cont = (CL_PMpointcontents (p->org) & MASK_WATER);
		if (cont) {
			p->orient = 0;
			p->flags = PARTICLE_UNDERWATER;
			p->type = PT_BUBBLE;
			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
			p->size = 0.8;
			p->sizeVel = 0;
			p->color[0] = 1;
			p->color[1] = 1;
			p->color[2] = 1;

			p->colorVel[0] = 0;
			p->colorVel[1] = 0;
			p->colorVel[2] = 0;
		}
		else {
			p->flags = PARTICLE_DIRECTIONAL;
			p->orient = 0;
			p->type = PT_DEFAULT;

			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE;
			p->size = 0.30;
			p->sizeVel = 0.1;
			p->len = 10;
			p->endLen = 30;
			p->color[0] = 1;
			p->color[1] = 1;
			p->color[2] = 0;

			p->colorVel[0] = -1;
			p->colorVel[1] = -2;
			p->colorVel[2] = 0;
		}
		p->time = cl.time;
		p->endTime = cl.time + 700;



		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () % 32) - 16);
			p->vel[j] = (rand () % 384) - 192;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -0.8 / (0.5 + frand () * 0.1);
	}


	//Smoke     
	for (i = 0; i < 4; i++) {
		cont = (CL_PMpointcontents (p->org) & MASK_WATER);
		if (cont)
			return;

		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_AIRONLY;
		p->flags |= PARTICLE_VERTEXLIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.35;
		p->color[1] = 0.35;
		p->color[2] = 0.35;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_SMOKE;
		p->size = 3;
		p->sizeVel = 50;
		d = rand () & 7;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j];
			p->vel[j] = crand ();
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = 20;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (1.5 + frand () * 0.5);

	}


}

void CL_BfgBall(vec3_t org) {
	cparticle_t *p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->orient = 0;
	p->flags = PARTICLE_LIGHTING;
	p->flags |= PARTICLE_ROTATE;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->size = 30;
	p->sizeVel = 30;
	p->alpha = 1;
	p->alphavel = INSTANT_PARTICLE;

	p->lightradius = 100;
	p->lcolor[0] = 0.0;
	p->lcolor[1] = 1.0;
	p->lcolor[2] = 0.0;


	p->sFactor = GL_ONE;
	p->dFactor = GL_ONE;

	p->color[0] = 1;
	p->color[1] = 1;
	p->color[2] = 1;

	p->colorVel[0] = 1;
	p->colorVel[1] = 1;
	p->colorVel[2] = 1;
	p->type = PT_BFG_BALL;

	VectorCopy(org, p->org);
	VectorClear(p->vel);
	VectorClear(p->accel);
}


void CL_BfgExplosion(vec3_t org) {
	cparticle_t *p;
		
	// blast circle
	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->orient = 0;
	p->flags = 0;
	p->time = cl.time;
	p->endTime = cl.time + 1600;
	p->size = 10;
	p->sizeVel = 30;
	p->alpha = 1;
	p->alphavel = 0;

	p->sFactor = GL_ONE;
	p->dFactor = GL_ONE;

	p->color[0] = 0.2;
	p->color[1] = 1.0;
	p->color[2] = 0.2;

	p->colorVel[0] = 0.0001;
	p->colorVel[1] = 0.01;
	p->colorVel[2] = 0.0001;
	p->type = PT_BFG_EXPL2;

	VectorCopy(org, p->org);
	VectorClear(p->vel);
	VectorClear(p->accel);

	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->orient = 0;
	p->flags = PARTICLE_LIGHTING | PARTICLE_SOFT_MIDLE;
	p->time = cl.time;
	p->endTime = cl.time + 1600;
	p->size = 40;
	p->sizeVel = 40;
	p->alpha = 1;
	p->alphavel = 0;

	p->lightradius = 350;
	p->lcolor[0] = 0.0;
	p->lcolor[1] = 1.0;
	p->lcolor[2] = 0.0;


	p->sFactor = GL_ONE;
	p->dFactor = GL_ONE;

	p->color[0] = 1;
	p->color[1] = 1;
	p->color[2] = 1;

	p->colorVel[0] = 1;
	p->colorVel[1] = 1;
	p->colorVel[2] = 1;
	p->type = PT_BFG_EXPL;

	VectorCopy(org, p->org);
	VectorClear(p->vel);
	VectorClear(p->accel);
}

/*
===============
CL_TeleporterParticles
===============
*/
void CL_TeleporterParticles (entity_state_t * ent) {
	int i, j;
	cparticle_t *p;

	for (i = 0; i < 8; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 0.7;
		p->color[1] = 0.0;
		p->color[2] = 0.5;

		p->colorVel[0] = -0.4;
		p->colorVel[1] = 1;
		p->colorVel[2] = -0.5;
		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 0;
		for (j = 0; j < 2; j++) {
			p->org[j] = ent->origin[j] - 16 + (rand () & 31);
			p->vel[j] = crand () * 14;
		}

		p->org[2] = ent->origin[2] - 8 + (rand () & 7);
		p->vel[2] = 80 + (rand () & 7);

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -0.5;
	}
}


/*
===============
CL_LogoutEffect

===============
*/
void CL_LogoutEffect (vec3_t org, int type) {
	int i, j;
	cparticle_t *p;

	for (i = 0; i < 500; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		if (type == MZ_LOGIN) {
			p->color[0] = 0;
			p->color[1] = 1;
			p->color[2] = 0;
		}


		else if (type == MZ_LOGOUT) {
			p->color[0] = 1;
			p->color[1] = 0;
			p->color[2] = 0;
		}
		else {
			p->color[0] = 1;
			p->color[1] = 1;
			p->color[2] = 0;
		}

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 0;
		p->org[0] = org[0] - 16 + frand () * 32;
		p->org[1] = org[1] - 16 + frand () * 32;
		p->org[2] = org[2] - 24 + frand () * 56;

		for (j = 0; j < 3; j++)
			p->vel[j] = crand () * 20;

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -1.0 / (1.0 + frand () * 0.3);
	}
}


/*
===============
CL_ItemRespawnParticles

===============
*/
void CL_ItemRespawnParticles (vec3_t org) {
	int i, j;
	cparticle_t *p;

	for (i = 0; i < 15; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->color[0] = 0;
		p->color[1] = 1;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->org[0] = org[0] + crand () * 15;
		p->org[1] = org[1] + crand () * 15;
		p->org[2] = org[2] + crand () * 15;
		p->type = PT_SMOKE;
		p->size = 0.7;
		p->sizeVel = 80;
		for (j = 0; j < 3; j++)
			p->vel[j] = crand () * 15;

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = PARTICLE_GRAVITY*1.2;
		p->alpha = 0.5;

		p->alphavel = -1.0 / (1.0 + frand () * 0.3);
	}
}


/*
===============
CL_ExplosionParticles
===============
*/
void CL_ExplosionParticles (vec3_t org) {
	int i, j;
	cparticle_t *p;


	for (i = 0; i < 256; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_OVERBRIGHT;
		p->type = PT_BLASTER;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->time = cl.time;
		p->endTime = cl.time + 500;
		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 0;

		p->colorVel[0] = -0.3;
		p->colorVel[1] = -1;
		p->colorVel[2] = 0;

		p->size = 0.7;
		p->sizeVel = 0.1;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () % 32) - 16);
			p->vel[j] = (rand () % 384) - 192;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;

		p->alphavel = -0.8 / (0.5 + frand () * 0.3);


	}

}


/*
===============
CL_BigTeleportParticles
===============
*/
void CL_BigTeleportParticles (vec3_t org) {
	int i;
	cparticle_t *p;
	float angle, dist, color;

	for (i = 0; i < 4096; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_OVERBRIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		color = 0.1 + (0.2 * frand ());

		p->color[0] = color;
		p->color[1] = color;
		p->color[2] = color;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		angle = M_PI * 2 * (rand () & 1023) / 1023.0;
		dist = rand () & 31;
		p->org[0] = org[0] + cos (angle) * dist;
		p->vel[0] = cos (angle) * (70 + (rand () & 63));
		p->accel[0] = -cos (angle) * 100;

		p->org[1] = org[1] + sin (angle) * dist;
		p->vel[1] = sin (angle) * (70 + (rand () & 63));
		p->accel[1] = -sin (angle) * 100;

		p->org[2] = org[2] + 8 + (rand () % 90);
		p->vel[2] = -100 + (rand () & 31);
		p->accel[2] = PARTICLE_GRAVITY * 4;
		p->alpha = 1.0;
		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 1;
		p->alphavel = -0.3 / (0.5 + frand () * 0.3);
	}
}


/*
===============
CL_BlasterParticles

Wall impact puffs
===============
*/
void CL_BlasterParticles (vec3_t org, vec3_t dir) {
	int i, j;
	cparticle_t *p;
	float d;
	int count;

	count = 15;
	for (i = 0; i < count; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->type = PT_BLASTER;
		p->orient = 0;
		p->flags = PARTICLE_BOUNCE;
		p->flags |= PARTICLE_FRICTION;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->size = 1;
		p->sizeVel = 0;
		p->time = cl.time;
		p->endTime = cl.time + 20000;

		p->color[0] = 0.97;
		p->color[1] = 0.46;
		p->color[2] = 0.14;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->alpha = 1;
		p->alphavel = -0.25 / (0.3 + frand () * 0.2);

		p->len = 2;
		p->endLen = 60;

		d = rand () & 15;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d*dir[j];
			p->vel[j] = dir[j] * 30 + crand () * 40;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY*1.5;

		VectorCopy (p->org, p->oldOrg);
	}
}


/*
===============
CL_BlasterTrail

===============
*/
void CL_BlasterTrail (vec3_t start, vec3_t end) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	int dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 2;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->orient = 0;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;
		p->flags = PARTICLE_DEFAULT;
		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.3 + frand () * 0.2);
		p->color[0] = 0.97;
		p->color[1] = 0.46;
		p->color[2] = 0.14;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_BLASTER;
		p->size = 2.6 + (1.2 * crand ());
		p->sizeVel = -2.4 + (1.2 * crand ());

		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand ();
			p->vel[j] = crand () * 5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_QuadTrail

===============
*/
void CL_QuadTrail (vec3_t start, vec3_t end) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	int dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->type = PT_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->size = 1;
		p->sizeVel = 1;
		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.8 + frand () * 0.2);
		p->color[0] = 0;
		p->color[1] = 0;
		p->color[2] = 1;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand () * 16;
			p->vel[j] = crand () * 5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_FlagTrail

===============
*/
void CL_FlagTrail (vec3_t start, vec3_t end, float r, float g, float b) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	int dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		VectorClear (p->accel);
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 1;
		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.8 + frand () * 0.2);
		p->color[0] = r;
		p->color[1] = g;
		p->color[2] = b;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand () * 16;
			p->vel[j] = crand () * 5;
			p->accel[j] = 0;
		}

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_DiminishingTrail

===============
*/
void CL_DiminishingTrail (vec3_t start, vec3_t end, centity_t * old,
	int flags) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j, i;
	cparticle_t *p;
	float dec;
	float orgscale;
	float velscale;
	int cont;
	trace_t trace;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	trace = CL_Trace (start, end, 8.0, MASK_SOLID);
	VectorNormalize (trace.plane.normal);
	dec = 20;

	VectorScale (vec, dec, vec);

	if (old->trailcount > 900) {
		orgscale = 4;
		velscale = 15;
	}
	else if (old->trailcount > 800) {
		orgscale = 2;
		velscale = 10;
	}
	else {
		orgscale = 1;
		velscale = 5;
	}

	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;

		// drop less particles as it flies
		if ((rand () & 1023) < old->trailcount) {
			p = free_particles;
			free_particles = p->next;
			p->next = active_particles;
			active_particles = p;
			p->orient = frand () * 360;
			p->flags = PARTICLE_DEFAULT;
			VectorClear (p->accel);

			p->time = cl.time;
			p->endTime = cl.time + 20000;
			if (flags & EF_GIB) {
				p->orient = frand () * 360;
				p->alpha = 1;
				p->alphavel = -3.0 / (1 + frand () * 1.7);
				if (!cl_blood->integer)
					p->alpha = 0;
				p->color[0] = 0.1;
				p->color[1] = 0.0;
				p->color[2] = 0.0;
				p->sFactor = GL_SRC_ALPHA;
				p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

				p->colorVel[0] = 0;
				p->colorVel[1] = 0;
				p->colorVel[2] = 0;
				p->type = PT_BLOODMIST;
				p->size = 5 / (0.5 + frand () * 0.7);
				p->sizeVel = -5;

				for (j = 0; j < 3; j++) {
					p->org[j] = move[j] + crand () * orgscale;
					p->vel[j] = crand () * velscale;
					p->accel[j] = 0;

				}
				p->vel[2] -= PARTICLE_GRAVITY;
				VectorCopy (p->org, p->oldOrg);

				p->alpha = 0;
				if (cl_blood->integer) {

					if (trace.fraction != 1.0) {

						for (i = 0; i < 1; i++)
							CL_AddDecalToScene (trace.endpos, trace.plane.normal,
							1, 1, 1, 1,
							1, 1, 1, 1,
							20, 12000,
							DECAL_BLOOD9, 0, frand () * 360,
							GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
						VectorClear (trace.plane.normal);
						return;
					}
				}
			}
			else if (flags & EF_GREENGIB) {

				p->alpha = 0.55;
				p->alphavel = -1.0 / (1 + frand () * 1.7);

				if (!cl_blood->integer)
					p->alpha = 0;

				p->sFactor = GL_SRC_ALPHA;
				p->dFactor = GL_ONE;
				p->color[0] = 1;
				p->color[1] = 1;
				p->color[2] = 0;

				p->colorVel[0] = 0;
				p->colorVel[1] = 0;
				p->colorVel[2] = 0;

				p->size = 10;
				p->sizeVel = -5;

				for (j = 0; j < 3; j++) {
					p->org[j] = move[j] + crand () * orgscale;
					p->vel[j] = crand () * velscale;
					p->accel[j] = 0;
					p->type = PT_BLOOD2;

				}
				p->vel[2] -= PARTICLE_GRAVITY;

				if (cl_blood->integer) {

					if (trace.fraction > 0 && trace.fraction < 1) {
						CL_AddDecalToScene (trace.endpos, trace.plane.normal,
							1, 1, 0, 1,
							1, 1, 0, 1,
							25, 12000,
							DECAL_ACIDMARK, DF_OVERBRIGHT,
							frand () * 360, GL_ONE, GL_ONE);
						CL_ParticleSmoke2 (trace.endpos, trace.plane.normal, 1,
							1, 0, 8, qtrue);
						VectorClear (trace.plane.normal);
						p->alpha = 0;
						return;
					}
				}
			}
			else {
				p->alpha = 1.0;
				p->alphavel = -1.0 / (1 + frand () * 0.2);
				p->color[0] = 0.5;
				p->color[1] = 0.5;
				p->color[2] = 0.5;
				p->sFactor = GL_SRC_ALPHA;
				p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
				p->colorVel[0] = 0;
				p->colorVel[1] = 0;
				p->colorVel[2] = 0;
				for (j = 0; j < 3; j++) {
					p->org[j] = move[j] + crand () * orgscale;
					p->vel[j] = crand () * velscale;
				}
				p->accel[2] = 20;

				cont = (CL_PMpointcontents (p->org) & MASK_WATER);
				if (cont) {
					p->flags = PARTICLE_UNDERWATER;
					p->type = PT_BUBBLE;
					p->size = 0.65;
					p->sizeVel = 0;
				}
				else if (!cont) {
					p->flags = PARTICLE_VERTEXLIGHT;
					p->type = PT_SMOKE;
					p->size = 2;
					p->sizeVel = 12;
				}
			}
		}

		old->trailcount -= 5;
		if (old->trailcount < 100)
			old->trailcount = 100;
		VectorAdd (move, vec, move);
	}
}






void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up) {
	float d;

	// this rotate and negat guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct (right, forward);
	VectorMA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}

void CL_GunFire (vec3_t start, vec3_t end) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	int dec;


	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 1;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;

		p->time = cl.time;
		p->endTime = cl.time + 14000;
		p->sFactor = GL_ONE;
		p->dFactor = GL_ONE;
		p->alpha = 1.0;
		p->alphavel = -2.0 / (0.2 + frand () * 0.1);

		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 1;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_FLAME;
		p->size = 4;
		p->sizeVel = -35;

		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand ();
			p->accel[j] = 0;
			p->vel[j] = crand () * 5;
		}

		VectorAdd (move, vec, move);

	}

}

/*
===============
CL_RocketTrail

===============
*/

void CL_RocketFire (vec3_t start, vec3_t end) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	int dec, cont;


	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 1;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;

		p->time = cl.time;
		p->endTime = cl.time + 14000;
		p->sFactor = GL_ONE;
		p->dFactor = GL_ONE;
		p->alpha = 1.0;
		p->alphavel = -2.0 / (0.2 + frand () * 0.1);

		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 1;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->type = PT_FLAME;
		p->size = 2;
		p->sizeVel = -35;

		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand ();
			p->accel[j] = 0;
			p->vel[j] = crand () * 5;
		}

		cont = (CL_PMpointcontents (p->org) & MASK_WATER);


		if (cont) {
			CL_BubbleTrail (start, end);
		}

		VectorAdd (move, vec, move);

	}

}
void CL_RocketTrail (vec3_t start, vec3_t end, centity_t * old) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	float dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 0.3;
	VectorScale (vec, dec, vec);

	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;

		if ((rand () & 7) == 0) {
			p = free_particles;
			free_particles = p->next;
			p->next = active_particles;
			active_particles = p;


			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
			p->flags = PARTICLE_VERTEXLIGHT;
			p->type = PT_SMOKE;
			p->orient = frand () * 360;
			p->size = 2;
			p->sizeVel = 40;
			VectorClear (p->accel);
			p->time = cl.time;
			p->endTime = cl.time + 20000;
			p->alpha = 0.5;
			p->alphavel = -1.0 / (0.1 + frand () * 0.9);
			p->color[0] = 0.4;
			p->color[1] = 0.4;
			p->color[2] = 0.4;

			p->colorVel[0] = 0.15;
			p->colorVel[1] = 0.15;
			p->colorVel[2] = 0.15;
			for (j = 0; j < 3; j++) {
				p->org[j] = move[j] + crand ();
				p->vel[j] = crand ();
			}

			p->accel[0] = p->accel[1] = 0;
			p->accel[2] = 0;

		}
		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_RailTrail

===============
*/

void CL_RailTrail (vec3_t start, vec3_t end) {
	vec3_t vec;
	int j;
	cparticle_t *p;

	VectorSubtract (end, start, vec);

	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->orient = 0;
	p->flags = PARTICLE_SPIRAL;
	p->flags |= PARTICLE_OVERBRIGHT;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	VectorClear (p->accel);
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
	p->alpha = 1.0;
	p->alphavel = -0.50 / (0.3 + frand () * 0.3);
	p->color[0] = cl_railspiral_red->value;
	p->color[1] = cl_railspiral_green->value;
	p->color[2] = cl_railspiral_blue->value;

	p->colorVel[0] = -0.5;
	p->colorVel[1] = -0.75;
	p->colorVel[2] = -1.0;
	p->type = PT_BLASTER;
	p->size = 2.8;
	p->sizeVel = 1;

	for (j = 0; j < 3; j++) {
		p->org[j] = start[j];
		p->length[j] = vec[j];
		p->vel[j] = 0;
		p->accel[j] = 0;
	}


	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->time = cl.time;
	p->endTime = cl.time + 20000;
	VectorClear (p->accel);
	p->orient = 0;
	p->flags = PARTICLE_STRETCH;
	p->flags |= PARTICLE_OVERBRIGHT;
	p->alpha = 1.0;
	p->alphavel = -0.5 / (0.3 + frand () * 0.3);
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
	p->color[0] = cl_railcore_red->value;
	p->color[1] = cl_railcore_green->value;
	p->color[2] = cl_railcore_blue->value;

	p->colorVel[0] = -0.5;
	p->colorVel[1] = -0.75;
	p->colorVel[2] = -1.0;
	p->type = PT_BLASTER;
	p->size = 3;
	p->sizeVel = 1;

	for (j = 0; j < 3; j++) {
		p->org[j] = start[j];
		p->length[j] = vec[j];
		p->vel[j] = 0;
		p->accel[j] = 0;
	}

}

void CL_ParticleRick (vec3_t org, vec3_t dir) {
	float d;
	int j, i;
	cparticle_t *p;
	VectorNormalize (dir);

	for (i = 0; i < 24; i++) {

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		VectorClear (p->vel);
		p->orient = 0;
		p->flags = PARTICLE_DIRECTIONAL | PARTICLE_AIRONLY;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;
		p->type = PT_BLASTER;
		p->size = 0.7;
		p->sizeVel = 0;
		p->alpha = 0.3;
		p->alphavel = -1.0 / (0.3 + frand () * 0.2);

		p->color[0] = 1;
		p->color[1] = 0.7;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = -1;
		p->colorVel[2] = 0;
		p->len = 11;
		p->endLen = 36;
		d = rand () & 5;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + d * dir[j];
			p->vel[j] = dir[j] * 30 + crand () * 10;

		}
		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
	}
	
	//SMOKE
	for (i = 0; i < 6; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_AIRONLY | PARTICLE_VERTEXLIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.1;
		p->color[1] = 0.1;
		p->color[2] = 0.1;

		p->colorVel[0] = 0.9;
		p->colorVel[1] = 0.9;
		p->colorVel[2] = 0.9;
		p->alpha = 1;
		p->alphavel = -1.5 / (0.5 + frand () * 0.5);

		p->type = PT_SMOKE;
		p->size = 5;
		p->sizeVel = 20;

		d = rand () & 15;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = dir[j] * 30;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = 10;


	}
	// Wall Aligned	Smoke Puff

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	VectorClear (p->accel);
	VectorClear (p->vel);
	VectorCopy (dir, p->dir);
	VectorNormalize (p->dir);
	p->orient = 0;
	p->flags = PARTICLE_ALIGNED;
	p->flags |= PARTICLE_AIRONLY;
	p->flags |= PARTICLE_NOFADE;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

	p->color[0] = 0.1;
	p->color[1] = 0.1;
	p->color[2] = 0.1;

	p->colorVel[0] = 0.1;
	p->colorVel[1] = 0.1;
	p->colorVel[2] = 0.1;
	p->alpha = 1.0;
	p->alphavel = -1.0 / (0.5 + frand () * 0.5);

	p->type = PT_SMOKE;
	p->size = 5;
	p->sizeVel = 15;

	p->len = 0;
	p->endLen = 0;

	for (j = 0; j < 3; j++) {
		p->org[j] = org[j];
		p->vel[j] = p->dir[j] * 2;
	}
}

void CL_ParticleRailRick (vec3_t org, vec3_t dir) {

	float d;
	int j, i;
	cparticle_t *p;
	VectorNormalize (dir);

	for (i = 0; i < 25; i++) {

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		VectorClear (p->vel);
		p->orient = 0;
		p->flags = PARTICLE_DIRECTIONAL | PARTICLE_AIRONLY;
		p->lightradius = 10.0;
		p->lcolor[0] = 1.0;
		p->lcolor[1] = 0.5;
		p->lcolor[2] = 0.0;

		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;
		p->type = PT_BLASTER;
		p->size = 0.6;
		p->sizeVel = 0;
		p->alpha = 0.7;
		p->alphavel = -1.0 / (0.3 + frand () * 0.2);

		p->color[0] = 1;
		p->color[1] = 0.7;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = -1;
		p->colorVel[2] = 0;
		p->len = 20;
		p->endLen = 200;
		d = rand () & 5;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + d * dir[j];
			p->vel[j] = dir[j] * 30 + crand () * 10;

		}
		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;

	}
	//SMOKE
	for (i = 0; i < 11; i++) {
		if (!free_particles)
			return;
		p = free_particles;

		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = frand () * 360;
		p->flags = PARTICLE_AIRONLY | PARTICLE_VERTEXLIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;

		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

		p->color[0] = 0.1;
		p->color[1] = 0.1;
		p->color[2] = 0.1;

		p->colorVel[0] = 0.2;
		p->colorVel[1] = 0.2;
		p->colorVel[2] = 0.2;
		p->alpha = 1;
		p->alphavel = -1.0 / (0.5 + frand () * 0.5);

		p->type = PT_SMOKE;
		p->size = 5;
		p->sizeVel = 30;

		d = rand () & 15;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d * dir[j];
			p->vel[j] = dir[j] * 30;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = 10;


	}

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	VectorClear (p->accel);
	VectorClear (p->vel);
	VectorCopy (dir, p->dir);
	VectorNormalize (p->dir);
	p->orient = 0;
	p->flags = PARTICLE_ALIGNED | PARTICLE_AIRONLY | PARTICLE_NOFADE | PARTICLE_VERTEXLIGHT;
	p->time = cl.time;
	p->endTime = cl.time + 20000;
	p->sFactor = GL_SRC_ALPHA;
	p->dFactor = GL_ONE_MINUS_SRC_ALPHA;

	p->color[0] = 0.1;
	p->color[1] = 0.1;
	p->color[2] = 0.1;

	p->colorVel[0] = 0.2;
	p->colorVel[1] = 0.2;
	p->colorVel[2] = 0.2;
	p->alpha = 0.7;
	p->alphavel = -1.0 / (0.5 + frand () * 0.5);

	p->type = PT_SMOKE;
	p->size = 5;
	p->sizeVel = 40;

	p->len = 0;
	p->endLen = 0;

	for (j = 0; j < 3; j++) {
		p->org[j] = org[j];
		p->vel[j] = p->dir[j] * 2;
	}

	for (i = 0; i < 35; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->type = PT_BLASTER;
		p->orient = 0;
		p->flags = PARTICLE_BOUNCE;
		p->flags |= PARTICLE_FRICTION;
		p->flags |= PARTICLE_DIRECTIONAL;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;

		p->size = 0.5;
		p->sizeVel = 0;
		p->time = cl.time;
		p->endTime = cl.time + 20000;

		p->color[0] = 0.97;
		p->color[1] = 0.46;
		p->color[2] = 0.14;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->alpha = 1;
		p->alphavel = -0.25 / (0.3 + frand () * 0.2);

		p->len = 1;
		p->endLen = 130;

		d = rand () & 15;
		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () & 7) - 4) + d*dir[j];
			p->vel[j] = dir[j] * 30 + crand () * 40;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY*1.5;

		VectorCopy (p->org, p->oldOrg);
		V_AddLight(p->oldOrg, 10.0, 1.0, 0.5, 1.0, vec3_origin, 0, 0);
	}


}





// RAFAEL
/*
===============
CL_IonripperTrail
===============
*/
void CL_IonripperTrail (vec3_t start, vec3_t ent) {
	vec3_t move;
	vec3_t vec;
	float len;
	int j;
	cparticle_t *p;
	int dec;
	int left = 0;

	VectorCopy (start, move);
	VectorSubtract (ent, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->orient = 0;
		p->flags = PARTICLE_OVERBRIGHT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;
		p->type = PT_BLASTER;
		p->size = 1.0;
		p->sizeVel = 1.0;
		p->alpha = 0.5;
		p->alphavel = -1.0 / (0.3 + frand () * 0.2);

		p->color[0] = 0.8;
		p->color[1] = 0.4;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		for (j = 0; j < 3; j++) {
			p->org[j] = move[j];
			p->accel[j] = 0;
		}
		if (left) {
			left = 0;
			p->vel[0] = 10;
		}
		else {
			left = 1;
			p->vel[0] = -10;
		}

		p->vel[1] = 0;
		p->vel[2] = 0;

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_BubbleTrail

===============
*/
void CL_BubbleTrail (vec3_t start, vec3_t end) {
	vec3_t move;
	vec3_t vec;
	float len;
	int i, j;
	cparticle_t *p;
	float dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 16;
	VectorScale (vec, dec, vec);

	for (i = 0; i < len; i += dec) {

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->flags = PARTICLE_UNDERWATER;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->alpha = 1.0;
		p->alphavel = -1.0 / (1 + frand () * 0.2);
		p->color[0] = 1;
		p->color[1] = 1;
		p->color[2] = 1;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		p->type = PT_BUBBLE;
		p->size = 0.35;
		p->sizeVel = 0;

		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand () * 2;
			p->vel[j] = crand () * 5;
		}
		p->vel[2] += 6;

		p->accel[0] = p->accel[1] = p->accel[2] = 0;
		VectorAdd (move, vec, move);

	}
}


/*
===============
CL_FlyParticles
===============
*/

#define	BEAMLENGTH			16
void CL_FlyParticles (vec3_t origin, int count) {
	int i;
	cparticle_t *p;
	float angle;
	float sr, sp, sy, cr, cp, cy;
	vec3_t forward;
	float dist = 64;
	float ltime;


	if (count > NUM_VERTEX_NORMALS)
		count = NUM_VERTEX_NORMALS;

	if (!avelocities[0][0]) {
		for (i = 0; i < NUM_VERTEX_NORMALS; i++) {
			avelocities[i][0] = (rand () & 255) * 0.01;
			avelocities[i][1] = (rand () & 255) * 0.01;
			avelocities[i][2] = (rand () & 255) * 0.01;
		}
	}



	ltime = (float)cl.time / 1000.0;
	for (i = 0; i < count; i += 2) {
		angle = ltime * avelocities[i][0];
		sy = sin (angle);
		cy = cos (angle);
		angle = ltime * avelocities[i][1];
		sp = sin (angle);
		cp = cos (angle);
		angle = ltime * avelocities[i][2];
		sr = sin (angle);
		cr = cos (angle);

		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->flags = PARTICLE_DEFAULT;
		p->orient = 0;
		p->time = cl.time;
		p->endTime = cl.time + 60000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		dist = sin (ltime + i) * 64;
		p->org[0] =
			origin[0] + q_byteDirs[i][0] * dist + forward[0] * BEAMLENGTH;
		p->org[1] =
			origin[1] + q_byteDirs[i][1] * dist + forward[1] * BEAMLENGTH;
		p->org[2] =
			origin[2] + q_byteDirs[i][2] * dist + forward[2] * BEAMLENGTH;

		VectorClear (p->vel);
		VectorClear (p->accel);

		p->color[0] = 0;
		p->color[1] = 0;
		p->color[2] = 0;

		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;
		p->size = 0.75;
		p->sizeVel = 0;

		p->alpha = 1;
		p->alphavel = -100;
		p->type = PT_FLY;

	}
}


void CL_FlyEffect (centity_t * ent, vec3_t origin) {
	int n;
	int count;
	int starttime;

	if (ent->fly_stoptime < cl.time) {
		starttime = cl.time;
		ent->fly_stoptime = cl.time + 60000;
	}
	else {
		starttime = ent->fly_stoptime - 60000;
	}

	n = cl.time - starttime;
	if (n < 20000)
		count = n * 162 / 20000.0;
	else {
		n = ent->fly_stoptime - cl.time;
		if (n < 20000)
			count = n * 162 / 20000.0;
		else
			count = 162;
	}

	CL_FlyParticles (origin, count);
}


/*
===============
CL_BfgParticles
===============
*/

#define	BEAMLENGTH			16
void CL_BfgParticles (entity_t * ent) {
	int i;
	cparticle_t *p;
	float angle;
	float sr, sp, sy, cr, cp, cy;
	vec3_t forward;
	float dist = 64;
	vec3_t v;
	float ltime;

	if (!avelocities[0][0]) {
		for (i = 0; i < NUM_VERTEX_NORMALS; i++) {
			avelocities[i][0] = (rand () & 255) * 0.01;
			avelocities[i][1] = (rand () & 255) * 0.01;
			avelocities[i][2] = (rand () & 255) * 0.01;
		}
	}


	ltime = (float)cl.time / 1000.0;
	for (i = 0; i < NUM_VERTEX_NORMALS; i++) {
		angle = ltime * avelocities[i][0];
		sy = sin (angle);
		cy = cos (angle);
		angle = ltime * avelocities[i][1];
		sp = sin (angle);
		cp = cos (angle);
		angle = ltime * avelocities[i][2];
		sr = sin (angle);
		cr = cos (angle);

		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		p->orient = 0.0f;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->flags = PARTICLE_OVERBRIGHT;
		p->type = PT_DEFAULT;
		p->size = 2.0f;
		p->sizeVel = 16.0f;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		dist = sin (ltime + i) * 64;
		p->org[0] =
			ent->origin[0] + q_byteDirs[i][0] * dist +
			forward[0] * BEAMLENGTH;
		p->org[1] =
			ent->origin[1] + q_byteDirs[i][1] * dist +
			forward[1] * BEAMLENGTH;
		p->org[2] =
			ent->origin[2] + q_byteDirs[i][2] * dist +
			forward[2] * BEAMLENGTH;

		VectorClear (p->vel);
		VectorClear (p->accel);

		VectorSubtract (p->org, ent->origin, v);
		dist = VectorLength (v) / 90.0;
		p->color[0] = 0.0 + crand() * 0.5;
		p->color[1] = 1.0;
		p->color[2] = 0.0;
		p->colorVel[0] = 0.0 + crand() * 0.5;
		p->colorVel[1] = 1.0;
		p->colorVel[2] = 0.0;

		p->alpha = 1.0 - dist;
		p->alphavel = -100;

	}
}

/*
===============
CL_TrapParticles
===============
*/
// RAFAEL
void CL_TrapParticles (entity_t * ent) {
	vec3_t move;
	vec3_t vec;
	vec3_t start, end;
	float len;
	int j;
	cparticle_t *p;
	int dec;

	ent->origin[2] -= 14;
	VectorCopy (ent->origin, start);
	VectorCopy (ent->origin, end);
	end[2] += 20;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 1;
	VectorScale (vec, 5, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0) {
		len -= dec;

		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;
		VectorClear (p->accel);
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;
		p->type = PT_BLASTER;
		p->size = 6;
		p->sizeVel = -10;
		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.3 + frand () * 0.2);
		p->color[0] = 1;
		p->color[1] = 0.67;
		p->color[2] = 0.4;
		p->colorVel[0] = 0;
		p->colorVel[1] = -3;
		p->colorVel[2] = -15;
		for (j = 0; j < 3; j++) {
			p->org[j] = move[j] + crand ();
			p->vel[j] = crand () * 15;
			p->accel[j] = 0;
		}
		p->accel[2] = PARTICLE_GRAVITY;

		VectorAdd (move, vec, move);
	}

	{


		int i, j, k;
		cparticle_t *p;
		float vel;
		vec3_t dir;
		vec3_t org;


		ent->origin[2] += 14;
		VectorCopy (ent->origin, org);


		for (i = -2; i <= 2; i += 4)
		for (j = -2; j <= 2; j += 4)
		for (k = -2; k <= 4; k += 4) {
			if (!free_particles)
				return;
			p = free_particles;
			free_particles = p->next;
			p->next = active_particles;
			active_particles = p;
			p->orient = 0;
			p->flags = PARTICLE_DEFAULT;
			p->time = cl.time;
			p->endTime = cl.time + 20000;
			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE;
			p->color[0] = 1;
			p->color[1] = 0.5;
			p->color[2] = 0;
			p->colorVel[0] = 0;
			p->colorVel[1] = -0.5;
			p->colorVel[2] = 0;
			p->type = PT_BLASTER;
			p->size = 1;
			p->sizeVel = -2;
			p->alpha = 1.0;
			p->alphavel = -1.0 / (0.3 + (rand () & 7) * 0.02);

			p->org[0] = org[0] + i + ((rand () & 23) * crand ());
			p->org[1] = org[1] + j + ((rand () & 23) * crand ());
			p->org[2] = org[2] + k + ((rand () & 23) * crand ());

			dir[0] = j * 8;
			dir[1] = i * 8;
			dir[2] = k * 8;

			VectorNormalize (dir);
			vel = 50 + (rand () & 63);
			VectorScale (dir, vel, p->vel);

			p->accel[0] = p->accel[1] = 0;
			p->accel[2] = -PARTICLE_GRAVITY;
		}
	}
}


/*
===============
CL_BFGExplosionParticles
===============
*/
//FIXME combined with CL_ExplosionParticles
void CL_BFGExplosionParticles (vec3_t org) {
	int i, j;
	cparticle_t *p;

	for (i = 0; i < 256; i++) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE_MINUS_SRC_ALPHA;
		p->flags = PARTICLE_OVERBRIGHT;
		p->flags |= PARTICLE_DIRECTIONAL;
		p->orient = 0;
		p->type = PT_BLASTER;
		p->size = 0.30;
		p->sizeVel = 0;
		p->color[0] = 0.24;
		p->color[1] = 0.82;
		p->color[2] = 0.10;
		p->colorVel[0] = -1;
		p->colorVel[1] = 1;
		p->colorVel[2] = -1;
		p->len = 10;
		p->endLen = 30;

		for (j = 0; j < 3; j++) {
			p->org[j] = org[j] + ((rand () % 32) - 16);
			p->vel[j] = (rand () % 384) - 192;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		p->alpha = 1.0;
		p->alphavel = -0.8 / (0.5 + frand () * 0.3);
	}
}


/*
===============
CL_TeleportParticles

===============
*/
void CL_TeleportParticles (vec3_t org) {
	int i, j, k;
	cparticle_t *p;
	float vel, color;
	vec3_t dir;

	color = 0.1 + (0.2 * frand ());
	for (i = -16; i <= 16; i += 4)
	for (j = -16; j <= 16; j += 4)
	for (k = -16; k <= 32; k += 4) {
		if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = cl.time;
		p->endTime = cl.time + 20000;
		p->sFactor = GL_SRC_ALPHA;
		p->dFactor = GL_ONE;
		p->orient = 0;
		p->flags = PARTICLE_DEFAULT;
		p->color[0] = color;
		p->color[1] = color;
		p->color[2] = color;
		p->colorVel[0] = 0;
		p->colorVel[1] = 0;
		p->colorVel[2] = 0;

		p->alpha = 1.0;
		p->alphavel = -1.0 / (0.3 + (rand () & 7) * 0.02);

		p->org[0] = org[0] + i + (rand () & 3);
		p->org[1] = org[1] + j + (rand () & 3);
		p->org[2] = org[2] + k + (rand () & 3);

		dir[0] = j * 8;
		dir[1] = i * 8;
		dir[2] = k * 8;

		VectorNormalize (dir);
		vel = 50 + (rand () & 63);
		VectorScale (dir, vel, p->vel);
		p->type = PT_DEFAULT;
		p->size = 1;
		p->sizeVel = 1;
		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
	}
}

/* BFG LASERS
*/
void CL_AddLasers (void) {
	laser_t *l;
	int i, index;


	for (i = 0, l = cl_lasers; i < MAX_LASERS; i++, l++) {
		if (l->endtime >= cl.time) {

			cparticle_t *p;
			if (!free_particles)
				return;
			p = free_particles;
			index = (l->ent.skinnum >> ((rand () % 4) * 8)) & 0xff;
			free_particles = p->next;
			p->next = active_particles;
			active_particles = p;
			p->orient = 0;
			p->flags = PARTICLE_OVERBRIGHT;
			p->flags |= PARTICLE_STRETCH;
			p->time = cl.time;
			p->endTime = cl.time + 20000;
			p->sFactor = GL_SRC_ALPHA;
			p->dFactor = GL_ONE;
			VectorClear (p->accel);
			VectorClear (p->vel);
			p->alpha = 0.3;
			p->alphavel = INSTANT_PARTICLE;

			p->color[0] = cl_indexPalette[index][0];
			p->color[1] = cl_indexPalette[index][1];
			p->color[2] = cl_indexPalette[index][2];


			p->colorVel[0] = 0;
			p->colorVel[1] = 0;
			p->colorVel[2] = 0;

			p->type = PT_BLASTER;
			p->size = 3;
			p->sizeVel = 3;
			VectorCopy (l->ent.origin, p->org);
			VectorSubtract (l->ent.oldorigin, l->ent.origin, p->length);

		}

	}
}
