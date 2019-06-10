/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2004-2011 Quake2xp Team.

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
//Decals, base code from EGL rewrite to q2xp specifics

#include "client.h"

decals_t	decals[MAX_DECALS];
decals_t	active_decals, *free_decals;

float Clamp_Value (float value, float min, float max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

/*
=================
CL_ClearDecals
=================
*/

void CL_ClearDecals (void) {
	int		i;

	memset (decals, 0, sizeof(decals));

	/* link decals */
	free_decals = decals;
	active_decals.prev = &active_decals;
	active_decals.next = &active_decals;
	for (i = 0; i < MAX_DECALS - 1; i++)
		decals[i].next = &decals[i + 1];
}

decals_t *CL_AllocDecal (void) {
	decals_t       *dl;

	if (free_decals) {	/* take a free decal if possible */
		dl = free_decals;
		free_decals = dl->next;
	}
	else {		/* grab the oldest one otherwise */
		dl = active_decals.prev;
		dl->prev->next = dl->next;
		dl->next->prev = dl->prev;
	}

	/* put the decal at the start of the list */
	dl->prev = &active_decals;
	dl->next = active_decals.next;
	dl->next->prev = dl;
	dl->prev->next = dl;
	return dl;
}

void CL_FreeDecal (decals_t * dl) {
	if (!dl->prev)
		return;

	/* remove from linked active list */
	dl->prev->next = dl->next;
	dl->next->prev = dl->prev;

	/* insert into linked free list */
	dl->next = free_decals;
	free_decals = dl;
}


/*
===============
CL_AddDecalToScene
===============
*/

void CL_AddDecalToScene (vec3_t origin, vec3_t dir,
	float red, float green, float blue, float alpha,
	float endRed, float endGreen, float endBlue,
	float endAlpha, float size,
	float endTime, int type, int flags, float angle,
	int sFactor, int dFactor) {
	int i, j, numfragments;
	vec3_t verts[MAX_DECAL_VERTS];
	fragment_t *fr, fragments[MAX_DECAL_FRAGMENTS];
	mat3_t axis;
	decals_t *d = NULL;

	if (!cl_decals->integer)
		return;

	// invalid decal
	if (size <= 0)
		return;

	// a hack to produce decals from explosions etc.
	if (VectorCompare (dir, vec3_origin)) {

		float	scale = 1.5 * size;
		trace_t	trace;
		vec3_t	end,
			dirs[6] = {
				{ 1.0, 0.0, 0.0 },
				{ -1.0, 0.0, 0.0 },
				{ 0.0, 1.0, 0.0 },
				{ 0.0, -1.0, 0.0 },
				{ 0.0, 0.0, 1.0 },
				{ 0.0, 0.0, -1.0 }
		};

		for (i = 0; i < 6; i++) {

			VectorMA (origin, scale, dirs[i], end);
			trace = CL_PMTraceWorld (origin, vec3_origin, vec3_origin, end, MASK_SOLID, qfalse);
			if (trace.fraction != 1.0)
				CL_AddDecalToScene (origin, trace.plane.normal,
				red, green, blue, alpha,
				endRed, endGreen, endBlue,
				endAlpha, size,
				endTime, type, flags, angle,
				sFactor, dFactor);
		}
		return;
	}

	// calculate orientation matrix
	VectorNormalize2 (dir, axis[0]);
	PerpendicularVector (axis[1], axis[0]);
	RotatePointAroundVector (axis[2], axis[0], axis[1], angle);
	CrossProduct (axis[0], axis[2], axis[1]);

	numfragments = R_GetClippedFragments (origin, size, axis, MAX_DECAL_VERTS, verts, MAX_DECAL_FRAGMENTS, fragments);

	// no valid fragments
	if (!numfragments)
		return;

	VectorScale (axis[1], 0.5f / size, axis[1]);
	VectorScale (axis[2], 0.5f / size, axis[2]);

	for (i = 0, fr = fragments; i < numfragments; i++, fr++) {

		if (fr->numverts > MAX_DECAL_VERTS)
			fr->numverts = MAX_DECAL_VERTS;
		else if (fr->numverts <= 0)
			continue;

		d = CL_AllocDecal ();
		d->numverts = fr->numverts;
		d->node = fr->node;
		d->time = cl.refdef.time;
		d->endTime = cl.refdef.time + endTime;
		d->alpha = alpha;
		d->endAlpha = endAlpha;
		d->size = size;
		d->type = type;
		d->flags = flags;
		d->sFactor = sFactor;
		d->dFactor = dFactor;

		VectorCopy (origin, d->org);
		VectorSet (d->color, red, green, blue);
		VectorSet (d->endColor, endRed, endGreen, endBlue);

		for (j = 0; j < fr->numverts; j++) {
			vec3_t v;

			VectorCopy (verts[fr->firstvert + j], d->verts[j]);
			VectorSubtract (d->verts[j], origin, v);
			d->stcoords[j][0] = DotProduct (v, axis[1]) + 0.5f;
			d->stcoords[j][1] = DotProduct (v, axis[2]) + 0.5f;
		}
	}
}

