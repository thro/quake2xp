/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc., 2004-2013 Quake2xp Team.

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

#include "r_local.h"

// this is the slow, general version
int BoxOnPlaneSide22 (vec3_t emins, vec3_t emaxs, struct cplane_s *p) {
	int		i;
	float	dist1, dist2;
	int		sides;
	vec3_t	corners[2];

	for (i = 0; i < 3; i++) {
		if (p->normal[i] < 0) {
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else {
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct (p->normal, corners[0]) - p->dist;
	dist2 = DotProduct (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}


/*
 =================
 BoundsAndSphereIntersect
 =================
 */
qboolean BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius) {

	if (r_noCull->integer)
		return qfalse;

	if (mins[0] > origin[0] + radius || mins[1] > origin[1] + radius || mins[2] > origin[2] + radius)
		return qfalse;
	if (maxs[0] < origin[0] - radius || maxs[1] < origin[1] - radius || maxs[2] < origin[2] - radius)
		return qfalse;

	return qtrue;
}

/*
===========
BoundsIntersect

===========
*/
qboolean BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2) {
	
	if (r_noCull->integer)
		return qfalse;

	if (mins1[0] > maxs2[0] || mins1[1] > maxs2[1] || mins1[2] > maxs2[2])
		return qfalse;
	if (maxs1[0] < mins2[0] || maxs1[1] < mins2[1] || maxs1[2] < mins2[2])
		return qfalse;

	return qtrue;
}
/*
=================
R_CullBox

Returns qtrue if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs) {
	int i;

	if (r_noCull->integer)
		return qfalse;

	for (i = 0; i < 6; i++)
	if (BOX_ON_PLANE_SIDE (mins, maxs, &frustum[i]) == 2)
		return qtrue;
	return qfalse;
}

qboolean R_CullConeLight (vec3_t mins, vec3_t maxs, cplane_t *frust) {
	int		i;

	if (r_noCull->integer)
		return qfalse;

	for (i = 0; i < 4; i++) {
		if (BoxOnPlaneSide22(mins, maxs, &frust[i]) == 2)
			return qtrue;
	}

	return qfalse;
}

/*
=================
R_CullOrigin

Returns qtrue if the origin is completely outside the frustom
=================
*/
qboolean R_CullOrigin (vec3_t origin) {
	int i;

	if (r_noCull->integer)
		return qfalse;

	for (i = 0; i < 6; i++)
	if (BOX_ON_PLANE_SIDE (origin, origin, &frustum[i]) == 2)
		return qtrue;
	return qfalse;
}


qboolean R_CullPoint (vec3_t org) {
	int i;

	if (r_noCull->integer)
		return qfalse;

	for (i = 0; i < 6; i++)
	if (DotProduct (org, frustum[i].normal) > frustum[i].dist)
		return qtrue;

	return qfalse;
}

qboolean R_CullSphere (const vec3_t centre, const float radius) {
	int		i;
	cplane_t *p;

	if (r_noCull->integer)
		return qfalse;

	for (i = 0, p = frustum; i < 6; i++, p++) {
		if (DotProduct (centre, p->normal) - p->dist <= -radius)
			return qtrue;
	}

	return qfalse;
}

qboolean BoundsIntersectsPoint (vec3_t mins, vec3_t maxs, vec3_t p) {
	
	if (r_noCull->integer)
		return qfalse;

	if (p[0] > maxs[0]) return qfalse;
	if (p[1] > maxs[1]) return qfalse;
	if (p[2] > maxs[2]) return qfalse;

	if (p[0] < mins[0]) return qfalse;
	if (p[1] < mins[1]) return qfalse;
	if (p[2] < mins[2]) return qfalse;

	return qtrue;
}

/*
=========================
Frustum_CullHexProjection

=========================
*/
qboolean Frustum_CullHexProjection(const vec3_t points[8], const vec3_t projOrigin, const int planeBits) {
	const cplane_t	*plane;
	vec3_t		projPoints[8];
	uint		side;
	int			i, j;

	if (!planeBits)
		return qfalse;

	for (i = 0; i < 8; i++)
		VectorSubtract(points[i], projOrigin, projPoints[i]);

	for (i = 0, plane = frustum; i < 6; i++, plane++) {
		if (!(planeBits & BIT(i)))
			continue;

		side = 0;

		for (j = 0; j < 8; j++) {
			if (DotProduct(points[j], plane->normal) > plane->dist)
				side |= PLANE_FRONT;
			else
				side |= PLANE_BACK;

			if (side == PLANE_CLIP)
				break;

			if (DotProduct(projPoints[j], plane->normal) > 0.f)
				side |= PLANE_FRONT;
			else
				side |= PLANE_BACK;

			if (side == PLANE_CLIP)
				break;
		}

		if (side == PLANE_BACK)
			return qtrue;
	}

	return qfalse;
}

/*
============================
Frustum_CullBoundsProjection

============================
*/
qboolean Frustum_CullBoundsProjection(const vec3_t mins, const vec3_t maxs, const vec3_t projOrigin, const int planeBits) {
	vec3_t	points[8];
	int		i;

	for (i = 0; i < 8; i++) {
		points[i][0] = (i & 1) ? mins[0] : maxs[0];
		points[i][1] = (i & 2) ? mins[1] : maxs[1];
		points[i][2] = (i & 4) ? mins[2] : maxs[2];
	}

	return Frustum_CullHexProjection(points, projOrigin, planeBits);
}

/*
=================================
Frustum_CullLocalBoundsProjection

=================================
*/
qboolean Frustum_CullLocalBoundsProjection(const vec3_t mins, const vec3_t maxs, const vec3_t origin, const mat3_t axis, const vec3_t projOrigin, const int planeBits) {
	vec3_t	tMins, tMaxs;
	vec3_t	tmp;
	vec3_t	points[8];
	int		i;

	if (Mat3_IsIdentity(axis)) {
		VectorAdd(origin, mins, tMins);
		VectorAdd(origin, maxs, tMaxs);

		return Frustum_CullBoundsProjection(tMins, tMaxs, projOrigin, planeBits);
	}

	for (i = 0; i < 8; i++) {
		tmp[0] = (i & 1) ? mins[0] : maxs[0];
		tmp[1] = (i & 2) ? mins[1] : maxs[1];
		tmp[2] = (i & 4) ? mins[2] : maxs[2];

		Mat3_MultiplyVector(axis, tmp, points[i]);
		VectorAdd(points[i], origin, points[i]);
	}

	return Frustum_CullHexProjection(points, projOrigin, planeBits);
}

int SignbitsForPlane (cplane_t * out) {
	int bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j = 0; j < 3; j++) {
		if (out->normal[j] < 0)
			bits |= 1 << j;
	}
	return bits;
}


void R_SetFrustum (void) {
	int i;

	RotatePointAroundVector (frustum[0].normal, vup, vpn,    -(90 - r_newrefdef.fov_x * 0.5));
	RotatePointAroundVector (frustum[1].normal, vup, vpn, 	   90 - r_newrefdef.fov_x * 0.5);
	RotatePointAroundVector (frustum[2].normal, vright, vpn,   90 - r_newrefdef.fov_y * 0.5);
	RotatePointAroundVector (frustum[3].normal, vright, vpn, -(90 - r_newrefdef.fov_y * 0.5));
	VectorCopy	(vpn, frustum[4].normal); 
	VectorNegate(vpn, frustum[5].normal); 

	for (i = 0; i < 6; i++) {
		VectorNormalize(frustum[i].normal);

		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}

	frustum[4].dist += r_zNear->value;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		frustum[5].dist -= 128.0;
	else
		frustum[5].dist -= r_zFar->value;
}
