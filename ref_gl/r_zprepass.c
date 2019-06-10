/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"

extern msurface_t	*scene_surfaces[MAX_MAP_FACES];
static int			num_depth_surfaces;
static vec3_t		modelorg;			// relative to viewpoint

qboolean R_FillDepthBatch (msurface_t *surf, unsigned *vertices, unsigned *indeces) {
	unsigned	numVertices, numIndices;
	int			i, nv = surf->polys->numVerts;

	numVertices = *vertices;
	numIndices = *indeces;

	if (numVertices + nv > MAX_BATCH_SURFS)
		return qfalse;

	// create indexes
	if (numIndices == 0xffffffff)
		numIndices = 0;

	for (i = 0; i < nv - 2; i++)
	{
		indexArray[numIndices++] = surf->baseIndex;
		indexArray[numIndices++] = surf->baseIndex + i + 1;
		indexArray[numIndices++] = surf->baseIndex + i + 2;
	}
	
	*vertices = numVertices;
	*indeces = numIndices;

	return qtrue;
}

static void GL_DrawDepthPoly () {
	msurface_t	*s;
	int			i;
	unsigned	numIndices = 0xffffffff;
	unsigned	numVertices = 0;

	for (i = 0; i < num_depth_surfaces; i++) {
		s = scene_surfaces[i];

		if (!R_FillDepthBatch (s, &numVertices, &numIndices)) {
			if (numIndices != 0xFFFFFFFF) {
				qglDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numVertices = 0;
				numIndices = 0xFFFFFFFF;
			}
		}
	}

	// draw the rest
	if (numIndices != 0xFFFFFFFF) {
		qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brush_polys += numIndices / 3;
	}
}

static void R_RecursiveDepthWorldNode (mnode_t * node) {
	int c, side, sidebit;
	cplane_t *plane;
	msurface_t *surf, **mark;
	mleaf_t *pleaf;
	float dot;

	if (node->contents == CONTENTS_SOLID)
		return;					// solid

	if (node->visframe != r_visframecount)
		return;

	if (R_CullBox (node->minmaxs, node->minmaxs + 3))
		return;

	// if a leaf node, draw stuff
	if (node->contents != -1) {
		pleaf = (mleaf_t *)node;

		// check for door connected areas
		if (r_newrefdef.areabits) {
			if (!(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
				return;			// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->numMarkSurfaces;

		if (c) {
			do {
				if (SurfInFrustum (*mark))
					(*mark)->visframe = r_framecount;
				(*mark)->ent = NULL;
				mark++;
			} while (--c);
		}

		return;
	}
	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	plane = node->plane;

	switch (plane->type) {
		case PLANE_X:
			dot = modelorg[0] - plane->dist;
			break;
		case PLANE_Y:
			dot = modelorg[1] - plane->dist;
			break;
		case PLANE_Z:
			dot = modelorg[2] - plane->dist;
			break;
		default:
			dot = DotProduct (modelorg, plane->normal) - plane->dist;
			break;
	}

	if (dot >= 0) {
		side = 0;
		sidebit = 0;
	}
	else {
		side = 1;
		sidebit = MSURF_PLANEBACK;
	}

	// recurse down the children, front side first
	R_RecursiveDepthWorldNode (node->children[side]);

	// draw stuff
	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c; c--, surf++) {

		if (surf->visframe != r_framecount)
			continue;

		if ((surf->flags & MSURF_PLANEBACK) != sidebit)
			continue;			// wrong side

		if (surf->texInfo->flags & SURF_SKY) {	// just adds to visible sky bounds
			R_AddSkySurface (surf);
		}
		else if (surf->texInfo->flags & SURF_NODRAW)
			continue;
		else if (surf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66))
			continue;
		else
			scene_surfaces[num_depth_surfaces++] = surf;
	}

	// recurse down the back side
	R_RecursiveDepthWorldNode (node->children[!side]);
}

static void R_AddBModelDepthPolys (void) {
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;

	psurf = &currentmodel->surfaces[currentmodel->firstModelSurface];

	for (i = 0; i < currentmodel->numModelSurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		if (pplane->type < 3)
			dot = modelorg[pplane->type] - pplane->dist;
		else
			dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & MSURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & MSURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {

			if (psurf->visframe == r_framecount)	// reckless fix
				continue;

			if (psurf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {
				continue;
			}
			scene_surfaces[num_depth_surfaces++] = psurf;
		}
	}
}
void R_DrawDepthBrushModel (void) {
	vec3_t		mins, maxs;
	int			i;
	qboolean	rotated;
	mat4_t		mvp;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!r_drawEntities->integer)
		return;

	if (currentmodel->numModelSurfaces == 0)
		return;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = qtrue;
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		rotated = qfalse;
		VectorAdd (currententity->origin, currentmodel->mins, mins);
		VectorAdd (currententity->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox (mins, maxs))
		return;

	VectorSubtract (r_newrefdef.vieworg, currententity->origin, modelorg);

	if (rotated) {
		vec3_t temp;
		vec3_t forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (currententity->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

	R_SetupEntityMatrix (currententity);

	Mat4_TransposeMultiply(currententity->matrix, r_newrefdef.modelViewProjectionMatrix, mvp);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)mvp);

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_BSP);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, BUFFER_OFFSET(vbo.xyz_offset));

	num_depth_surfaces = 0;
	R_AddBModelDepthPolys ();
	GL_DrawDepthPoly();

	qglDisableVertexAttribArray (ATT_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
}

void R_CalcAliasFrameLerp (dmdl_t *paliashdr, float shellScale);
extern vec3_t	tempVertexArray[MAX_VERTICES * 4];

void GL_DrawAliasFrameLerpDepth(dmdl_t *paliashdr) {
	vec3_t		vertexArray[3 * MAX_TRIANGLES];
	int			index_xyz;
	int			i, j, jj = 0;
	dtriangle_t	*tris;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;


	R_CalcAliasFrameLerp(paliashdr, 0);			/// Просто сюда переместили вычисления Lerp...
	
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_Dynamic);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);
	Mat4_TransposeMultiply(currententity->matrix, r_newrefdef.modelViewProjectionMatrix, currententity->orMatrix);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	c_alias_polys += paliashdr->num_tris;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	jj = 0;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy(tempVertexArray[index_xyz], vertexArray[jj]);
		}
	}
	qglBufferSubData(GL_ARRAY_BUFFER, 0, jj * sizeof(vec3_t), vertexArray);
	qglDrawArrays(GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
}

void R_DrawDepthAliasModel(void){

	dmdl_t		*paliashdr;
	vec3_t		bbox[8];
	
	if (!r_drawEntities->integer)
		return;
	
	if (R_CullAliasModel(bbox, currententity))
		return;

	paliashdr = (dmdl_t *)currentmodel->extraData;

	if ((currententity->frame >= paliashdr->num_frames)
		|| (currententity->frame < 0)) {
		Com_Printf("R_DrawAliasModel %s: no such frame %d\n",
			currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ((currententity->oldframe >= paliashdr->num_frames)
		|| (currententity->oldframe < 0)) {
		Com_Printf("R_DrawAliasModel %s: no such oldframe %d\n",
			currentmodel->name, currententity->oldframe);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	R_SetupEntityMatrix(currententity);

	GL_DrawAliasFrameLerpDepth(paliashdr);
}

void R_DrawDepthMD3Model(void) {

	md3Model_t	*md3Hdr;
	vec3_t		bbox[8];
	int			i, j;
	float		frontlerp, backlerp;
	md3Frame_t	*frame, *oldframe;
	vec3_t		move, delta, vectors[3];
	md3Vertex_t	*v, *ov;

	if (!r_drawEntities->integer)
		return;

	if (R_CullMD3Model(bbox, currententity))
		return;

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	CheckEntityFrameMD3(md3Hdr);

	backlerp = currententity->backlerp;
	frontlerp = 1.0 - backlerp;
	frame = md3Hdr->frames + currententity->frame;
	oldframe = md3Hdr->frames + currententity->oldframe;

	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);
	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldframe->translate, move);

	for (j = 0; j<3; j++)
		move[j] = backlerp*move[j] + frontlerp*frame->translate[j];

	currentmodel->max_meshes = md3Hdr->num_meshes;

	R_SetupEntityMatrix(currententity);

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_Dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_Dynamic);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);

	Mat4_TransposeMultiply(currententity->matrix, r_newrefdef.modelViewProjectionMatrix, currententity->orMatrix);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	for (i = 0; i < md3Hdr->num_meshes; i++){
		
		md3Mesh_t *mesh = &md3Hdr->meshes[i];

		if (mesh->flags & MESH_TRANSLUSCENT)
			continue;

		if (mesh->skinAlphatest)
			continue;

		v = mesh->vertexes + currententity->frame * mesh->num_verts;
		ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;

		for (j = 0; j < mesh->num_verts; j++, v++, ov++)
		{
			md3VertexCache[j][0] = move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp;
			md3VertexCache[j][1] = move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp;
			md3VertexCache[j][2] = move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp;
		}

		qglBufferSubData(GL_ARRAY_BUFFER, 0, mesh->num_verts * sizeof(vec3_t), md3VertexCache);
		qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->num_tris * 3 * sizeof(uint), mesh->indexes);
		qglDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, 0);
	}

	qglDisableVertexAttribArray(ATT_POSITION);

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_DrawSkyMask() {

	int i;

	qglBindFramebuffer(GL_FRAMEBUFFER, fbo_skyMask);
	qglClear(GL_COLOR_BUFFER_BIT);

	qglClearColor(1.0, 0.0, 0.0, 0.0);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_BSP);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, BUFFER_OFFSET(vbo.xyz_offset));
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);

	num_depth_surfaces = 0;
	R_RecursiveDepthWorldNode(r_worldmodel->nodes);
	GL_DrawDepthPoly();

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;
		if (currententity->flags & RF_WEAPONMODEL)
			continue;
		if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
			continue;
		if (currententity->flags & RF_DISTORT)
			continue;

		if (currentmodel->type == mod_brush)
			R_DrawDepthBrushModel();

		if (currentmodel->type == mod_alias)
			R_DrawDepthAliasModel();

		if (currentmodel->type == mod_alias_md3)
			R_DrawDepthMD3Model();
	}
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void R_DrawDepthScene (void) {

	int i;

	if (!r_drawWorld->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	currentmodel = r_worldmodel;

	VectorCopy (r_newrefdef.vieworg, modelorg);
	
	GL_DepthFunc(GL_LESS);
	GL_DepthMask(1);

	R_ClearSkyBox ();

	GL_BindProgram (nullProgram);

//	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //debug tool

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_BSP);
	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, BUFFER_OFFSET(vbo.xyz_offset));
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);

	num_depth_surfaces = 0;
	R_RecursiveDepthWorldNode (r_worldmodel->nodes);
	GL_DrawDepthPoly ();
	
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray (ATT_POSITION);

	R_DrawSkyBox (qfalse);
	
	if(r_globalFog->integer)
		R_DrawSkyMask();

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;
		if (currententity->flags & RF_WEAPONMODEL)
			continue;
		if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
			continue;
		if (currententity->flags & RF_DISTORT)
			continue;

		if (currentmodel->type == mod_brush)
			R_DrawDepthBrushModel ();

		if (currentmodel->type == mod_alias)
			R_DrawDepthAliasModel ();

		if (currentmodel->type == mod_alias_md3)
			R_DrawDepthMD3Model();
	}
	GL_DepthFunc(GL_LEQUAL);
	GL_DepthMask(0);
//	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
