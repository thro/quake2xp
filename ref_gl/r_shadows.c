/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2004-2013 Quake2xp Team, Berserker.

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
// shadows.c: shadow functions



#include "r_local.h"

int			num_shadow_surfaces, shadowTimeStamp;
vec4_t		s_lerped[MAX_VERTS];
vec3_t		vcache[MAX_MAP_TEXINFO * MAX_POLY_VERT];
vec4_t		vcache4[MAX_INDICES];
uint		icache[MAX_MAP_TEXINFO * MAX_POLY_VERT];
msurface_t	*shadow_surfaces[MAX_MAP_FACES];
char		triangleFacingLight[MAX_INDICES];


vec4_t	extrudedVerts[MD3_MAX_TRIANGLES * MD3_MAX_MESHES];
float	shadowVerts[MD3_MAX_TRIANGLES * MD3_MAX_MESHES];


/*
=====================
Alias Shadow Volumes
=====================
*/

void R_MarkShadowTriangles (dmdl_t *paliashdr, dtriangle_t *tris, vec3_t lightOrg) {

	vec3_t	temp, dir0, dir1, r_triNormals;
	int		i;
	float	f;
	float	*v0, *v1, *v2;

	for (i = 0; i < paliashdr->num_tris; i++, tris++) {

		v0 = (float*)s_lerped[tris->index_xyz[0]];
		v1 = (float*)s_lerped[tris->index_xyz[1]];
		v2 = (float*)s_lerped[tris->index_xyz[2]];

		//Calculate shadow volume triangle normals
		VectorSubtract (v0, v1, dir0);
		VectorSubtract (v2, v1, dir1);

		CrossProduct (dir0, dir1, r_triNormals);

		// Find front facing triangles
		VectorSubtract (lightOrg, v0, temp);
		f = DotProduct (temp, r_triNormals);

		triangleFacingLight[i] = f > 0;
	}
}

void BuildShadowVolumeTriangles(dmdl_t * hdr, vec3_t lightOrg) {
	dtriangle_t		*oldTris, *tris;
	neighbors_t		*neighbours;
	vec4_t			v0, v1, v2;
	daliasframe_t	*frame;
	dtrivertx_t		*verts;
	int				i, j, numVerts = 0, id = 0;

	frame = (daliasframe_t *)((byte *)hdr + hdr->ofs_frames + currententity->frame * hdr->framesize);
	verts = frame->verts;
	oldTris = tris = (dtriangle_t *)((unsigned char *)hdr + hdr->ofs_tris);

	R_MarkShadowTriangles(hdr, tris, lightOrg);

	for (i = 0, tris = oldTris, neighbours = currentmodel->neighbours; i < hdr->num_tris; i++, tris++, neighbours++) {

		if (!triangleFacingLight[i])
			continue;

		if (neighbours->n[0] < 0 || !triangleFacingLight[neighbours->n[0]]) {

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[1]][j];
				v1[j] = s_lerped[tris->index_xyz[0]][j];
			}

			//  transforms points with w = 1 normally and sends points with w = 0 to infinity away from the light.
			VA_SetElem4(vcache4[numVerts + 0], v0[0], v0[1], v0[2], v0[3] = 1.0);
			VA_SetElem4(vcache4[numVerts + 1], v1[0], v1[1], v1[2], v1[3] = 1.0);
			VA_SetElem4(vcache4[numVerts + 2], v1[0], v1[1], v1[2], v1[3] = 0.0);
			VA_SetElem4(vcache4[numVerts + 3], v0[0], v0[1], v0[2], v0[3] = 0.0);
			
			icache[id++] = numVerts + 0;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 2;
			numVerts += 4;
		}

		if (neighbours->n[1] < 0 || !triangleFacingLight[neighbours->n[1]]) {

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[2]][j];
				v1[j] = s_lerped[tris->index_xyz[1]][j];
			}

			VA_SetElem4(vcache4[numVerts + 0], v0[0], v0[1], v0[2], v0[3] = 1.0);
			VA_SetElem4(vcache4[numVerts + 1], v1[0], v1[1], v1[2], v1[3] = 1.0);
			VA_SetElem4(vcache4[numVerts + 2], v1[0], v1[1], v1[2], v1[3] = 0.0);
			VA_SetElem4(vcache4[numVerts + 3], v0[0], v0[1], v0[2], v0[3] = 0.0);

			icache[id++] = numVerts + 0;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 2;
			numVerts += 4;
		}

		if (neighbours->n[2] < 0 || !triangleFacingLight[neighbours->n[2]]) {

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[0]][j];
				v1[j] = s_lerped[tris->index_xyz[2]][j];
			}

			VA_SetElem4(vcache4[numVerts + 0], v0[0], v0[1], v0[2], v0[3] = 1.0);
			VA_SetElem4(vcache4[numVerts + 1], v1[0], v1[1], v1[2], v1[3] = 1.0);
			VA_SetElem4(vcache4[numVerts + 2], v1[0], v1[1], v1[2], v1[3] = 0.0);
			VA_SetElem4(vcache4[numVerts + 3], v0[0], v0[1], v0[2], v0[3] = 0.0);

			icache[id++] = numVerts + 0;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 2;
			numVerts += 4;
		}
	}

	// build shadows caps
	for (i = 0, tris = oldTris; i < hdr->num_tris; i++, tris++) {
		if (!triangleFacingLight[i])
			continue;

		for (j = 0; j < 3; j++) {
			v0[j] = s_lerped[tris->index_xyz[0]][j];
			v1[j] = s_lerped[tris->index_xyz[1]][j];
			v2[j] = s_lerped[tris->index_xyz[2]][j];
		}

		VA_SetElem4(vcache4[numVerts + 0], v0[0], v0[1], v0[2], v0[3] = 1.0);
		VA_SetElem4(vcache4[numVerts + 1], v1[0], v1[1], v1[2], v1[3] = 1.0);
		VA_SetElem4(vcache4[numVerts + 2], v2[0], v2[1], v2[2], v2[3] = 1.0);


		icache[id++] = numVerts + 0;
		icache[id++] = numVerts + 1;
		icache[id++] = numVerts + 2;
		numVerts += 3;

		// rear cap (with flipped winding order)
		for (j = 0; j < 3; j++) {
			v0[j] = s_lerped[tris->index_xyz[0]][j];
			v1[j] = s_lerped[tris->index_xyz[1]][j];
			v2[j] = s_lerped[tris->index_xyz[2]][j];
		}

		VA_SetElem4(vcache4[numVerts + 0], v0[0], v0[1], v0[2], v0[3] = 0.0);
		VA_SetElem4(vcache4[numVerts + 1], v1[0], v1[1], v1[2], v1[3] = 0.0);
		VA_SetElem4(vcache4[numVerts + 2], v2[0], v2[1], v2[2], v2[3] = 0.0);

		icache[id++] = numVerts + 2;
		icache[id++] = numVerts + 1;
		icache[id++] = numVerts + 0;
		numVerts += 3;
	}

	qglBufferSubData(GL_ARRAY_BUFFER, 0, numVerts * sizeof(vec4_t), vcache4);
	qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, id * sizeof(uint), icache);

	qglDrawElements (GL_TRIANGLES, id, GL_UNSIGNED_INT, NULL);

	c_shadow_tris += id / 3;
	c_shadow_volumes++;
}

//differs from the test for mesh in light bound
qboolean R_EntityCastShadow() { 

	int		i;
	vec3_t	mins, maxs;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return qfalse;

	if (currententity->flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED |
		RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_GOD |
		RF_TRANSLUCENT | RF_BEAM | RF_WEAPONMODEL | RF_NOSHADOW | RF_DISTORT))
		return qfalse;

	if (!r_playerShadow->integer && (currententity->flags & RF_VIEWERMODEL))
		return qfalse;

	if ((currententity->flags & RF_VIEWERMODEL) && (currentShadowLight->filter == 33))
		return qfalse;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		VectorAdd (currententity->origin, currententity->model->maxs, maxs);
		VectorAdd (currententity->origin, currententity->model->mins, mins);
	}

	if (currentShadowLight->_cone) {

		if(R_CullConeLight(mins, maxs, currentShadowLight->frust))
			return qfalse;

	}else if (currentShadowLight->spherical) {

		if (!BoundsAndSphereIntersect (mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return qfalse;
	}
	else {

		if (!BoundsIntersect (mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return qfalse;
	}

	if (VectorCompare(currententity->origin, currentShadowLight->origin)) // skip shadows from shell lights
		return qfalse;
	
	// cull shadow volume out of view frustum
	if(Frustum_CullLocalBoundsProjection(currententity->model->mins, currententity->model->maxs, currententity->origin, currententity->axis, currentShadowLight->origin, 63)) 
		return qfalse;

	if (!InLightVISEntity())
		return qfalse;

	return qtrue;
}


void GL_LerpShadowVerts (int numVerts, dtrivertx_t *v, dtrivertx_t *ov, float *lerp, float move[3], float frontv[3], float backv[3]) {
	int i;

	for (i = 0; i < numVerts; i++, v++, ov++, lerp += 4) {
		lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0];
		lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1];
		lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2];
	}
}

void R_DrawMD2ShadowVolume () {
	dmdl_t			*paliashdr;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*v, *ov, *verts;
	int				*order, i;
	float			frontlerp;
	vec3_t			move, delta, vectors[3], frontv, backv, light, temp;

	if (!R_EntityCastShadow())
		return;

	paliashdr = (dmdl_t *)currentmodel->extraData;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
		+ currententity->frame * paliashdr->framesize);

	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames +
		currententity->oldframe * paliashdr->framesize);

	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	frontlerp = 1.0 - currententity->backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, delta);
	AngleVectors (currententity->angles, vectors[0], vectors[1],
		vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = currententity->backlerp * move[i] + frontlerp * frame->translate[i];
		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = currententity->backlerp * oldframe->scale[i];
	}

	GL_LerpShadowVerts (paliashdr->num_xyz, v, ov, s_lerped[0], move, frontv, backv);

	R_SetupEntityMatrix (currententity);

	VectorSubtract (currentShadowLight->origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector (currententity->axis, temp, light);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);
	qglUniform3fv(U_LIGHT_POS, 1, light);

	BuildShadowVolumeTriangles (paliashdr, light);
}

void R_DrawMD3ShadowVolume(){

	int				i, j, k, numTris, numVerts, id = 0;
	float			frontlerp, backlerp;
	md3Model_t		*paliashdr;
	md3Frame_t		*frame, *oldframe;
	md3Mesh_t		*mesh;
	md3Vertex_t		*v, *ov;
	vec3_t			move, v1, v2, normal, trinormal, temp,
					delta, vectors[3], lightOrg;
	index_t			*idx, *index0, *index1;

	if (!R_EntityCastShadow())
		return;

	currentmodel = currententity->model;
	paliashdr = (md3Model_t *)currentmodel->extraData;

	R_SetupEntityMatrix(currententity);

	VectorSubtract(currentShadowLight->origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector(currententity->axis, temp, lightOrg);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);
	qglUniform3fv(U_LIGHT_POS, 1, lightOrg);

	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);
	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	backlerp = currententity->backlerp;
	frontlerp = 1.0 - backlerp;
	frame = paliashdr->frames + currententity->frame;
	oldframe = paliashdr->frames + currententity->oldframe;

	VectorAdd(move, oldframe->translate, move);

	for (j = 0; j<3; j++)
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	for (i = 0; i < paliashdr->num_meshes; i++)
	{
		mesh = &paliashdr->meshes[i];

		if (mesh->flags & MESH_TRANSLUSCENT)
			continue;

		v = mesh->vertexes + currententity->frame * mesh->num_verts;
		ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;

		for (j = 0; j < mesh->num_verts; j++, v++, ov++)
		{
			md3VertexCache[j][0] = move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp;
			md3VertexCache[j][1] = move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp;
			md3VertexCache[j][2] = move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp;

			VectorCopy(md3VertexCache[j], extrudedVerts[j]);
		}

		idx = mesh->indexes;
		for (j = 0; j < mesh->num_tris; j++){

			//Calculate shadow volume triangle normals
			VectorSubtract(md3VertexCache[*idx], md3VertexCache[*(idx + 1)], v1);
			VectorSubtract(md3VertexCache[*(idx + 2)], md3VertexCache[*(idx + 1)], v2);
			CrossProduct(v2, v1, normal);
			VectorScale(normal, 1 / VectorLength(normal), trinormal);

			// Find front facing triangles
			if (DotProduct(trinormal, lightOrg) <= DotProduct(md3VertexCache[*idx], trinormal))
				triangleFacingLight[j] = 0;	
			else
				triangleFacingLight[j] = 1;

			idx += 3;

		}

		idx = mesh->indexes;
		numTris = numVerts = 0;
		for (j = 0; j < mesh->num_tris; j++){

			if (triangleFacingLight[j])
			{
				for (k = 0; k<3; k++)
				{
					qboolean shadow = qfalse;
					if (mesh->triangles[j].neighbours[k] == -1)
						shadow = qtrue;
					else
						if (!triangleFacingLight[mesh->triangles[j].neighbours[k]])
							shadow = qtrue;

					if (shadow)
					{
						index0 = idx + k;
						index1 = idx + ((k + 1) % 3);

						//  transforms points with w = 1 normally and sends points with w = 0 to infinity away from the light.
						shadowVerts[numVerts++] = md3VertexCache[*index0][0];
						shadowVerts[numVerts++] = md3VertexCache[*index0][1];
						shadowVerts[numVerts++] = md3VertexCache[*index0][2];
						shadowVerts[numVerts++] = 1.0;

						shadowVerts[numVerts++] = extrudedVerts[*index0][0];
						shadowVerts[numVerts++] = extrudedVerts[*index0][1];
						shadowVerts[numVerts++] = extrudedVerts[*index0][2];
						shadowVerts[numVerts++] = 0.0;

						shadowVerts[numVerts++] = extrudedVerts[*index1][0];
						shadowVerts[numVerts++] = extrudedVerts[*index1][1];
						shadowVerts[numVerts++] = extrudedVerts[*index1][2];
						shadowVerts[numVerts++] = 0.0;

						shadowVerts[numVerts++] = extrudedVerts[*index1][0];
						shadowVerts[numVerts++] = extrudedVerts[*index1][1];
						shadowVerts[numVerts++] = extrudedVerts[*index1][2];
						shadowVerts[numVerts++] = 0.0;

						shadowVerts[numVerts++] = md3VertexCache[*index1][0];
						shadowVerts[numVerts++] = md3VertexCache[*index1][1];
						shadowVerts[numVerts++] = md3VertexCache[*index1][2];
						shadowVerts[numVerts++] = 1.0;

						shadowVerts[numVerts++] = md3VertexCache[*index0][0];
						shadowVerts[numVerts++] = md3VertexCache[*index0][1];
						shadowVerts[numVerts++] = md3VertexCache[*index0][2];
						shadowVerts[numVerts++] = 1.0;

						numTris += 6;
					}
				}
			}

			idx += 3;
			c_shadow_tris += numTris;
		}

		idx = mesh->indexes;
		for (j = 0; j < mesh->num_tris; j++)
		{
			if (triangleFacingLight[j])
			{
				for (k = 0; k<3; k++)
				{
					index0 = idx + k;
					 shadowVerts[numVerts++] = md3VertexCache[*index0][0];
					 shadowVerts[numVerts++] = md3VertexCache[*index0][1];
					 shadowVerts[numVerts++] = md3VertexCache[*index0][2];
					 shadowVerts[numVerts++] = 1.0;
				}

				for (k = 2; k >= 0; k--)
				{
					index0 = idx + k;
					 shadowVerts[numVerts++] = extrudedVerts[*index0][0];
					 shadowVerts[numVerts++] = extrudedVerts[*index0][1];
					 shadowVerts[numVerts++] = extrudedVerts[*index0][2];
					 shadowVerts[numVerts++] = 0.0;
				}

				numTris += 6;
				c_shadow_tris += numTris;
			}

			idx += 3;
		}
		qglBufferSubData(GL_ARRAY_BUFFER, 0, numVerts * sizeof(float), shadowVerts);
		qglDrawElements(GL_TRIANGLES, numVerts / 4, GL_UNSIGNED_INT, NULL);
	}

	c_shadow_volumes++;
}


void R_CastAliasShadowVolumes(qboolean player) {
	int	i;

	if (!r_shadows->integer || !r_drawEntities->integer)
		return;

	if (!currentShadowLight->isShadow || currentShadowLight->isAmbient || currentShadowLight->isFog)
		return;

	// turboshadows
	GL_BindProgram(shadowProgram);

	GL_StencilMask(255);
	GL_StencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 128, 255);
	GL_StencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	GL_StencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);

	GL_Disable(GL_CULL_FACE);
	GL_DepthFunc(GL_LESS);
	GL_ColorMask(0, 0, 0, 0);

	GL_PolygonOffset(0.1, 1);

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_Dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_Dynamic);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	if (player) {
		for (i = 0; i < r_newrefdef.num_entities; i++) {
			currententity = &r_newrefdef.entities[i];
			currentmodel = currententity->model;

			if (!(currententity->flags & RF_VIEWERMODEL)) 
				continue;

				if (!currentmodel)
					continue;

				if (currentmodel->type == mod_alias)
					R_DrawMD2ShadowVolume();

		}
	}
	else {
		for (i = 0; i < r_newrefdef.num_entities; i++) {
			currententity = &r_newrefdef.entities[i];
			currentmodel = currententity->model;

			if (currententity->flags & RF_VIEWERMODEL)
				continue;

			if (!currentmodel)
				continue;

			if (currentmodel->type == mod_alias)
				R_DrawMD2ShadowVolume();

		}
	}
	/*===============
	DRAW MD3 SHADOWS
	================*/

	GL_FrontFace(GL_CCW); // flip cull face order vs stencil re-setup
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);
	if (player) {
		for (i = 0; i < r_newrefdef.num_entities; i++) {
			currententity = &r_newrefdef.entities[i];
			currentmodel = currententity->model;

			if (!(currententity->flags & RF_VIEWERMODEL))
				continue;

			if (!currentmodel)
				continue;

			if (currentmodel->type == mod_alias_md3)
				R_DrawMD3ShadowVolume();

		}
	}
	else
	{
		for (i = 0; i < r_newrefdef.num_entities; i++) {
			currententity = &r_newrefdef.entities[i];
			currentmodel = currententity->model;

			if (currententity->flags & RF_VIEWERMODEL)
				continue;

			if (!currentmodel)
				continue;

			if (currentmodel->type == mod_alias_md3)
				R_DrawMD3ShadowVolume();

		}
	}

	GL_FrontFace(GL_CW);
	qglDisableVertexAttribArray(ATT_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GL_Enable(GL_CULL_FACE);
	GL_ColorMask(1, 1, 1, 1);
}


/*
======================================
BSP SHADOW VOLUMES
EASY VERSION FROM TENEBRAE AND BERS@Q2
======================================
*/

qboolean R_MarkShadowSurf (msurface_t *surf) {
	cplane_t	*plane;
	glpoly_t	*poly;
	float		dist, lbbox[6], pbbox[6];

	if (surf->texInfo->flags & (SURF_NODRAW)) // rogue hack
		return qfalse;

	// add sky surfaces to shadow marking
	if (surf->texInfo->flags & (SURF_SKY))
		goto hack;

	if ((surf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66 | SURF_WARP)) || (surf->flags & MSURF_DRAWTURB))
		return qfalse;
hack:
	plane = surf->plane;
	poly = surf->polys;

	if (poly->shadowTimestamp == shadowTimeStamp)
		return qfalse;

	switch (plane->type) {
		case PLANE_X:
			dist = currentShadowLight->origin[0] - plane->dist;
			break;
		case PLANE_Y:
			dist = currentShadowLight->origin[1] - plane->dist;
			break;
		case PLANE_Z:
			dist = currentShadowLight->origin[2] - plane->dist;
			break;
		default:
			dist = DotProduct (currentShadowLight->origin, plane->normal) - plane->dist;
			break;
	}

	//the normals are flipped when surf_planeback is 1
	if (((surf->flags & MSURF_PLANEBACK) && (dist > 0)) ||
		(!(surf->flags & MSURF_PLANEBACK) && (dist < 0)))
		return qfalse;

	//the normals are flipped when surf_planeback is 1
	if (fabsf (dist) > currentShadowLight->maxRad)
		return qfalse;

	lbbox[0] = currentShadowLight->origin[0] - currentShadowLight->radius[0];
	lbbox[1] = currentShadowLight->origin[1] - currentShadowLight->radius[1];
	lbbox[2] = currentShadowLight->origin[2] - currentShadowLight->radius[2];
	lbbox[3] = currentShadowLight->origin[0] + currentShadowLight->radius[0];
	lbbox[4] = currentShadowLight->origin[1] + currentShadowLight->radius[1];
	lbbox[5] = currentShadowLight->origin[2] + currentShadowLight->radius[2];

	// surface bounding box
	pbbox[0] = surf->mins[0];
	pbbox[1] = surf->mins[1];
	pbbox[2] = surf->mins[2];
	pbbox[3] = surf->maxs[0];
	pbbox[4] = surf->maxs[1];
	pbbox[5] = surf->maxs[2];

	if (currentShadowLight->_cone && R_CullConeLight(&pbbox[0], &pbbox[3], currentShadowLight->frust))
		return qfalse;

	if (!BoundsIntersect (&lbbox[0], &lbbox[3], &pbbox[0], &pbbox[3]))
		return qfalse;

	poly->shadowTimestamp = shadowTimeStamp;

	return qtrue;
}

void R_MarkBrushModelShadowSurfaces () {
	int			i;
	msurface_t	*psurf;
	model_t		*clmodel;

	clmodel = currententity->model;
	psurf = &clmodel->surfaces[clmodel->firstModelSurface];

	for (i = 0; i < clmodel->numModelSurfaces; i++, psurf++) {

		if (R_MarkShadowSurf (psurf))
			shadow_surfaces[num_shadow_surfaces++] = psurf;
	}
}

void R_DrawBrushModelVolumes () {
	int			i, j, vb = 0, ib = 0, surfBase = 0;
	float		scale;
	msurface_t	*surf;
	model_t		*clmodel;
	glpoly_t	*poly;
	vec3_t		v1, temp, oldLight;
	qboolean	shadow;

	clmodel = currententity->model;
	surf = &clmodel->surfaces[clmodel->firstModelSurface];

	if (!R_EntityCastShadow())
		return;

	R_SetupEntityMatrix (currententity);

	VectorCopy (currentShadowLight->origin, oldLight);
	VectorSubtract (currentShadowLight->origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector (currententity->axis, temp, currentShadowLight->origin);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	shadowTimeStamp++;
	num_shadow_surfaces = 0;
	R_MarkBrushModelShadowSurfaces ();

	scale = currentShadowLight->maxRad * 2;

	// generate vertex buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {

			VectorCopy (poly->verts[j], vcache[vb * 2 + 0]);
			VectorSubtract (poly->verts[j], currentShadowLight->origin, v1);
			vcache[vb * 2 + 1][0] = v1[0] * scale + poly->verts[j][0];
			vcache[vb * 2 + 1][1] = v1[1] * scale + poly->verts[j][1];
			vcache[vb * 2 + 1][2] = v1[2] * scale + poly->verts[j][2];
			vb++;
		}
	}

	// generate index buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {
			shadow = qfalse;

			if (poly->neighbours[j] != NULL) {

				if (poly->neighbours[j]->shadowTimestamp != poly->shadowTimestamp)
					shadow = qtrue;
			}
			else
				shadow = qtrue;

			if (shadow) {
				int jj = (j + 1) % poly->numVerts;

				icache[ib++] = j  * 2 + 0 + surfBase;
				icache[ib++] = j  * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;

				icache[ib++] = j  * 2 + 0 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 0 + surfBase;
			}
		}

		//Draw near light cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 0 + surfBase;
			icache[ib++] = (j + 1) * 2 + 0 + surfBase;
			icache[ib++] = (j + 2) * 2 + 0 + surfBase;
		}

		//Draw extruded cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 1 + surfBase;
			icache[ib++] = (j + 2) * 2 + 1 + surfBase;
			icache[ib++] = (j + 1) * 2 + 1 + surfBase;
		}
		surfBase += surf->numEdges * 2;
	}

	if (ib) {
		qglBufferSubData(GL_ARRAY_BUFFER, 0, surfBase * sizeof(vec3_t), vcache);
		qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, ib * sizeof(uint), icache);

		qglDrawElements	(GL_TRIANGLES, ib, GL_UNSIGNED_INT, NULL);
	}

	c_shadow_volumes++;
	c_shadow_tris += ib / 3;

	VectorCopy(oldLight, currentShadowLight->origin);
}


void R_MarkShadowCasting (mnode_t *node) {
	cplane_t	*plane;
	int			c, cluster;
	float		dist;
	msurface_t	**surf;
	mleaf_t		*leaf;

	if (node->contents != -1) {

		//we are in a leaf
		leaf = (mleaf_t *)node;
		cluster = leaf->cluster;

		if (!(currentShadowLight->vis[cluster >> 3] & (1 << (cluster & 7))))
			return;

		surf = leaf->firstmarksurface;

		for (c = 0; c < leaf->numMarkSurfaces; c++, surf++) {

			if (R_MarkShadowSurf ((*surf))) {

				shadow_surfaces[num_shadow_surfaces++] = (*surf);
			}
		}
		return;
	}

	plane = node->plane;
	dist = DotProduct (currentShadowLight->origin, plane->normal) - plane->dist;

	if (dist > currentShadowLight->maxRad) {
		R_MarkShadowCasting (node->children[0]);
		return;
	}

	if (dist < -currentShadowLight->maxRad) {
		R_MarkShadowCasting (node->children[1]);
		return;
	}

	R_MarkShadowCasting (node->children[0]);
	R_MarkShadowCasting (node->children[1]);
}

model_t *loadmodel;
int numPreCachedLights;

void R_DrawBspModelVolumes (qboolean precalc, worldShadowLight_t *light) {
	int			i, j, vb = 0, ib = 0, surfBase = 0;
	float		scale;
	msurface_t	*surf;
	glpoly_t	*poly;
	vec3_t		v1;
	qboolean	shadow;

	if (precalc)
		currentShadowLight = light;
	else
		light = NULL;

	shadowTimeStamp++;
	num_shadow_surfaces = 0;
	R_MarkShadowCasting (r_worldmodel->nodes);

	scale = currentShadowLight->maxRad * 10;

	// generate vertex buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {

			VectorCopy (poly->verts[j], vcache[vb * 2 + 0]);
			VectorSubtract (poly->verts[j], currentShadowLight->origin, v1);

			vcache[vb * 2 + 1][0] = v1[0] * scale + poly->verts[j][0];
			vcache[vb * 2 + 1][1] = v1[1] * scale + poly->verts[j][1];
			vcache[vb * 2 + 1][2] = v1[2] * scale + poly->verts[j][2];
			vb++;
		}
	}

	// generate index buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {
			shadow = qfalse;

			if (poly->neighbours[j] != NULL) {

				if (poly->neighbours[j]->shadowTimestamp != poly->shadowTimestamp)
					shadow = qtrue;
			}
			else
				shadow = qtrue;

			if (shadow) {
				int jj = (j + 1) % poly->numVerts;

				icache[ib++] = j  * 2 + 0 + surfBase;
				icache[ib++] = j  * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;

				icache[ib++] = j  * 2 + 0 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 0 + surfBase;
			}
		}

		//Draw near light cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 0 + surfBase;
			icache[ib++] = (j + 1) * 2 + 0 + surfBase;
			icache[ib++] = (j + 2) * 2 + 0 + surfBase;
		}

		//Draw extruded cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 1 + surfBase;
			icache[ib++] = (j + 2) * 2 + 1 + surfBase;
			icache[ib++] = (j + 1) * 2 + 1 + surfBase;
		}
		surfBase += surf->numEdges * 2;
	}

	if (precalc) {

		if (currentShadowLight->vboId)
			qglDeleteBuffers(1, &currentShadowLight->vboId);

		qglGenBuffers(1, &currentShadowLight->vboId);
		qglBindBuffer(GL_ARRAY_BUFFER, currentShadowLight->vboId);
		qglBufferData(GL_ARRAY_BUFFER, surfBase * sizeof(vec3_t), vcache, GL_STATIC_DRAW);
		qglBindBuffer(GL_ARRAY_BUFFER, 0);

		if (currentShadowLight->iboId)
			qglDeleteBuffers(1, &currentShadowLight->iboId);

		qglGenBuffers(1, &currentShadowLight->iboId);
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentShadowLight->iboId);
		qglBufferData(GL_ELEMENT_ARRAY_BUFFER, ib * sizeof(uint), icache, GL_STATIC_DRAW);
		currentShadowLight->iboNumIndices = ib;
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		numPreCachedLights++;
	}
	else {
		if (ib) {
			qglBufferSubData(GL_ARRAY_BUFFER, 0, surfBase * sizeof(vec3_t), vcache);
			qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, ib * sizeof(uint), icache);
			
			qglDrawElements	(GL_TRIANGLES, ib, GL_UNSIGNED_INT, NULL);
		}
	}
	c_shadow_tris += ib / 3;
}


void R_CastBspShadowVolumes (void) {
	int	i;

	if (!r_shadows->integer)
		return;

	if (!currentShadowLight->isShadow || currentShadowLight->isAmbient)
		return;

	// setup program
	GL_BindProgram (nullProgram);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);

	GL_StencilMask (255);
	GL_StencilFuncSeparate (GL_FRONT_AND_BACK, GL_ALWAYS, 128, 255);
	GL_StencilOpSeparate (GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	GL_StencilOpSeparate (GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);

	GL_Disable (GL_CULL_FACE);
	GL_DepthFunc (GL_LESS);
	GL_ColorMask (0, 0, 0, 0);

	GL_PolygonOffset(0.1, 1);

	qglEnableVertexAttribArray (ATT_POSITION);

	if (currentShadowLight->vboId && currentShadowLight->iboId && currentShadowLight->isStatic) { // draw vbo shadow

		qglBindBuffer(GL_ARRAY_BUFFER, currentShadowLight->vboId);
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentShadowLight->iboId);
		
		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);
		qglDrawElements	(GL_TRIANGLES, currentShadowLight->iboNumIndices, GL_UNSIGNED_INT, NULL);
	}

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_Dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_Dynamic);
	
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);

	if (!currentShadowLight->isStatic)	
		R_DrawBspModelVolumes(qfalse, NULL); 
	

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currentmodel->type == mod_brush)
			R_DrawBrushModelVolumes ();
	}

	qglDisableVertexAttribArray (ATT_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GL_Enable (GL_CULL_FACE);
	GL_ColorMask (1, 1, 1, 1);
}