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
// r_light.c

#include "r_local.h"

int r_dlightframecount;

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/
vec3_t pointcolor;
cplane_t *lightplane;			// used as shadow plane
vec3_t lightspot;

/*
======================
RecursiveLightPoint

======================
*/
int RecursiveLightPoint (mnode_t * node, vec3_t start, vec3_t end) {
	const float ambientScale = max(r_lightmapScale->value, 0.f) * (1.f / 255.f);
	float front, back, f;
	int side, size;
	int smax, tmax;
	cplane_t *plane;
	vec3_t mid;
	msurface_t *surf;
	int s, t, ds, dt;
	int i, j, r;
	mtexInfo_t *tex;
	byte *lm;

	if (node->contents != -1)
		return -1;	// didn't hit anything

	// calculate mid point

	// FIXME: optimize for axial

	plane = node->plane;
	front = DotProduct(start, plane->normal) - plane->dist;
	back = DotProduct(end, plane->normal) - plane->dist;
	side = front < 0.f;

	if ((back < 0.f) == side)
		return RecursiveLightPoint(node->children[side], start, end);

	f = front / (front - back);

	mid[0] = start[0] + (end[0] - start[0]) * f;
	mid[1] = start[1] + (end[1] - start[1]) * f;
	mid[2] = start[2] + (end[2] - start[2]) * f;

	// go down front side   
	r = RecursiveLightPoint(node->children[side], start, mid);
	if (r >= 0)
		return r;	// hit something

	if ((back < 0.f) == side)
		return -1;	// didn't hit anuthing

	// check for impact on this node
	VectorCopy(mid, lightspot);
	lightplane = plane;

	for (i = 0, surf = &r_worldmodel->surfaces[node->firstsurface]; i < node->numsurfaces; i++, surf++) {
		if (surf->flags & (MSURF_DRAWTURB | MSURF_DRAWSKY))
			continue;	// no lightmaps

		tex = surf->texInfo;

		s = DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;

		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;
		if (!surf->samples)
			return 0;

		ds /= r_worldmodel->lightmap_scale;
		dt /= r_worldmodel->lightmap_scale;

		smax = (surf->extents[0] / (int)r_worldmodel->lightmap_scale) + 1;

		lm = surf->samples + (dt * smax + ds) * 3;

		if (r_worldmodel->useXPLM) {
			tmax = (surf->extents[1] / (int)r_worldmodel->lightmap_scale) + 1;
			size = smax * tmax * 3;

			VectorClear(pointcolor);

			for (j = 0; j < XPLM_NUMVECS; j++, lm += size) {
				pointcolor[0] += lm[0] * r_xplmBasisVecs[j][2];
				pointcolor[1] += lm[1] * r_xplmBasisVecs[j][2];
				pointcolor[2] += lm[2] * r_xplmBasisVecs[j][2];
			}
		}
		else {
			pointcolor[0] = lm[0];
			pointcolor[1] = lm[1];
			pointcolor[2] = lm[2];
		}

		VectorScale(pointcolor, ambientScale, pointcolor);

		return 1;
	}

	// go down back side
	return RecursiveLightPoint(node->children[!side], mid, end);
}

/*
===============
R_LightPoint

===============
*/
void R_LightPoint (vec3_t p, vec3_t color) {
	vec3_t end;
	float r;
	int i;
//	trace_t trace;
	
	if ((r_worldmodel && !r_worldmodel->lightData) || !r_worldmodel) {
		color[0] = color[1] = color[2] = 1.0;
		return;
	}
	
	if (!r_newrefdef.areabits)
		return;

	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048.0f;

/*	trace = CL_PMTraceWorld(p, vec3_origin, vec3_origin, end, MASK_SOLID, qfalse);

	if (trace.fraction != 1.0) {
		vec3_t tmp;
		VectorCopy(trace.endpos, tmp);
		tmp[2] -= 2.0;

		r = RecursiveLightPoint(r_worldmodel->nodes, p, tmp);
	}
	else*/
		r = RecursiveLightPoint(r_worldmodel->nodes, p, end);

	if (r == -1)
		VectorClear(color);
	else
		VectorCopy(pointcolor, color);

	// this catches too bright modulated color
	for (i = 0; i < 3; i++)
		if (color[i] > 1)
			color[i] = 1;
}


#define LIGHTGRID_STEP 128
#define LIGHTGRID_NUM_STEPS (8192/LIGHTGRID_STEP)	// 64

byte r_lightgrid[LIGHTGRID_NUM_STEPS * LIGHTGRID_NUM_STEPS * LIGHTGRID_NUM_STEPS][3];


void R_InitLightgrid (void) {
	int i, x, y, z;
	vec3_t p, end;
	float r;
	byte *b;

	memset(r_lightgrid, 0, sizeof(r_lightgrid));

	if (!r_worldmodel->lightData)
		return;

	b = &r_lightgrid[0][0];

	// Huh ?
	for (x = 0; x < 8192; x += LIGHTGRID_STEP)
		for (y = 0; y < 8192; y += LIGHTGRID_STEP)
			for (z = 0; z < 8192; z += LIGHTGRID_STEP) {
				end[0] = p[0] = x - 4096;
				end[1] = p[1] = y - 4096;
				end[2] = (p[2] = z - 4096) - 2048;
				r = RecursiveLightPoint(r_worldmodel->nodes, p, end);
				if (r != -1) {
					for (i = 0; i < 3; i++) {
						float mu = pointcolor[i];
						float f = mu * (2 * (1 - mu) + mu);
						*b++ = 255.0 * f;
					}
				} else {
					b += 3;
				}
			}
}

//___________________________________________________________
void R_LightColor (vec3_t org, vec3_t color) {
	byte *b[8];
	int i;
	float f;
	float x = (4096.f + org[0]) / LIGHTGRID_STEP;
	float y = (4096.f + org[1]) / LIGHTGRID_STEP;
	float z = (4096.f + org[2]) / LIGHTGRID_STEP;
	int s = x;
	int t = y;
	int r = z;

	x = x - s;
	y = y - t;
	z = z - r;

	b[0] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r][0];
	b[1] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r + 1][0];
	b[2] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r][0];
	b[3] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r + 1][0];
	b[4] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r][0];
	b[5] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r + 1][0];
	b[6] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r][0];
	b[7] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r + 1][0];

	f = 4.f / 255.f;

	if (!(b[0][0] && b[0][1] && b[0][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (1 - y) * (1 - z);
	if (!(b[1][0] && b[1][1] && b[1][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (1 - y) * (z);
	if (!(b[2][0] && b[2][1] && b[2][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (y) * (1 - z);
	if (!(b[3][0] && b[3][1] && b[3][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (y) * (z);
	if (!(b[4][0] && b[4][1] && b[4][2]))
		f -= (1.0 / (510.0)) * (x) * (1 - y) * (1 - z);
	if (!(b[5][0] && b[5][1] && b[5][2]))
		f -= (1.0 / (510.0)) * (x) * (1 - y) * (z);
	if (!(b[6][0] && b[6][1] && b[6][2]))
		f -= (1.0 / (510.0)) * (x) * (y) * (1 - z);
	if (!(b[7][0] && b[7][1] && b[7][2]))
		f -= (1.0 / (510.0)) * (x) * (y) * (z);

	for (i = 0; i < 3; i++) {	// hahaha slap me silly
		color[i] =
			(1 - x) * ((1 - y) * ((1 - z) * b[0][i] + (z) * b[1][i]) +
		y * ((1 - z) * b[2][i] + (z) * b[3][i]))
	  + x * ((1 - y) * ((1 - z) * b[4][i] + (z) * b[5][i]) +
	    y * ((1 - z) * b[6][i] + (z) * b[7][i]));
		color[i] *= f;
	}
}