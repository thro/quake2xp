/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"

void CalcTangent4MD3(index_t *index, md3Vertex_t *vertices, md3ST_t *texcos, vec3_t Tangent, vec3_t Binormal)
{
	float *v0, *v1, *v2;
	float *st0, *st1, *st2;
	vec3_t vec1, vec2;
	vec3_t planes[3];
	int i;

	v0 = vertices[index[0]].xyz;
	v1 = vertices[index[1]].xyz;
	v2 = vertices[index[2]].xyz;
	st0 = texcos[index[0]].st;
	st1 = texcos[index[1]].st;
	st2 = texcos[index[2]].st;

	for (i = 0; i<3; i++)
	{
		vec1[0] = v1[i] - v0[i];
		vec1[1] = st1[0] - st0[0];
		vec1[2] = st1[1] - st0[1];
		vec2[0] = v2[i] - v0[i];
		vec2[1] = st2[0] - st0[0];
		vec2[2] = st2[1] - st0[1];
		VectorNormalize(vec1);
		VectorNormalize(vec2);
		CrossProduct(vec1, vec2, planes[i]);
	}

	Tangent[0] = -planes[0][1] / planes[0][0];
	Tangent[1] = -planes[1][1] / planes[1][0];
	Tangent[2] = -planes[2][1] / planes[2][0];
	Binormal[0] = -planes[0][2] / planes[0][0];
	Binormal[1] = -planes[1][2] / planes[1][0];
	Binormal[2] = -planes[2][2] / planes[2][0];
	VectorNormalize(Tangent); //is this needed?
	VectorNormalize(Binormal);
}


int R_FindTriangleWithEdge(index_t *indexes, int numtris, index_t start, index_t end, int ignore)
{
	int i;
	int match, count;

	count = 0;
	match = -1;

	for (i = 0; i < numtris; i++, indexes += 3)
	{
		if ((indexes[0] == start && indexes[1] == end)
			|| (indexes[1] == start && indexes[2] == end)
			|| (indexes[2] == start && indexes[0] == end))
		{
			if (i != ignore)
				match = i;
			count++;
		}
		else if ((indexes[1] == start && indexes[0] == end)
			|| (indexes[2] == start && indexes[1] == end)
			|| (indexes[0] == start && indexes[2] == end))
		{
			count++;
		}
	}

	// detect edges shared by three triangles and make them seams
	if (count > 2)
		match = -1;

	return match;
}


/*
===============
R_BuildTriangleNeighbors
===============
*/
void R_BuildTriangleNeighbors(neighbours_t *neighbors, index_t *indexes, int numtris)
{
	int				i;
	neighbours_t	*n;
	index_t			*index;

	for (i = 0, index = indexes, n = neighbors; i < numtris; i++, index += 3, n++)
	{
		n->neighbours[0] = R_FindTriangleWithEdge(indexes, numtris, index[1], index[0], i);
		n->neighbours[1] = R_FindTriangleWithEdge(indexes, numtris, index[2], index[1], i);
		n->neighbours[2] = R_FindTriangleWithEdge(indexes, numtris, index[0], index[2], i);
	}
}

/*
=================
Mod_LoadAliasMD3Model
=================
*/
void Mod_LoadMD3(model_t *mod, void *buffer)
{
	int					version, i, j, k, l, m;
	dmd3_t				*inModel;
	dmd3frame_t			*inFrame;
	dmd3tag_t			*inTag;
	dmd3mesh_t			*inMesh;
	dmd3skin_t			*inSkin;
	dmd3coord_t			*inCoord;
	dmd3vertex_t		*inVerts;
	uint				*inIndex;

	index_t				*outIndex;
	md3Vertex_t			*outVerts;
	md3ST_t				*outCoord;
	md3Mesh_t			*outMesh;
	md3Tag_t			*outTag;
	md3Frame_t			*outFrame;
	md3Model_t			*outModel;
	static vec3_t		tangents[MD3_MAX_VERTS], binormals[MD3_MAX_VERTS];
	vec3_t				scaleMD3;
	char				name[MD3_MAX_PATH];
	float				lat, lng;

	VectorSet(scaleMD3, MD3_XYZ_SCALE, MD3_XYZ_SCALE, MD3_XYZ_SCALE);

	inModel = (dmd3_t *)buffer;
	version = LittleLong(inModel->version);

	if (version != MD3_ALIAS_VERSION)
	{
		VID_Error(ERR_DROP, "%s has wrong version number (%i should be %i)",
			mod->name, version, MD3_ALIAS_VERSION);
	}

	outModel = Hunk_Alloc(sizeof(md3Model_t));

	// byte swap the header fields and sanity check
	outModel->num_frames = LittleLong(inModel->num_frames);
	outModel->num_tags = LittleLong(inModel->num_tags);
	outModel->num_meshes = LittleLong(inModel->num_meshes);

	if (outModel->num_frames <= 0)
		VID_Error(ERR_DROP, "model %s has no frames", mod->name);
	else if (outModel->num_frames > MD3_MAX_FRAMES)
		VID_Error(ERR_DROP, "model %s has too many frames", mod->name);

	if (outModel->num_tags > MD3_MAX_TAGS)
		VID_Error(ERR_DROP, "model %s has too many tags", mod->name);
	else if (outModel->num_tags < 0)
		VID_Error(ERR_DROP, "model %s has invalid number of tags", mod->name);

	if (outModel->num_meshes <= 0)
		VID_Error(ERR_DROP, "model %s has no meshes", mod->name);
	else if (outModel->num_meshes > MD3_MAX_MESHES)
		VID_Error(ERR_DROP, "model %s has too many meshes", mod->name);

	//
	// load the frames
	//
	inFrame = (dmd3frame_t *)((byte *)inModel + LittleLong(inModel->ofs_frames));
	outFrame = outModel->frames = Hunk_Alloc(sizeof(md3Frame_t) * outModel->num_frames);

	for (i = 0; i < outModel->num_frames; i++, inFrame++, outFrame++)
	{
		for (j = 0; j < 3; j++)
		{
			outFrame->translate[j] = LittleFloat(inFrame->translate[j]);
			outFrame->mins[j] = LittleFloat(inFrame->mins[j]) + outFrame->translate[j];
			outFrame->maxs[j] = LittleFloat(inFrame->maxs[j]) + outFrame->translate[j];
		}

		outFrame->radius = LittleFloat(inFrame->radius);
	}

	//
	// load the tags
	//
	inTag = (dmd3tag_t *)((byte *)inModel + LittleLong(inModel->ofs_tags));
	outTag = outModel->tags = Hunk_Alloc(sizeof(md3Tag_t) * outModel->num_frames * outModel->num_tags);

	for (i = 0; i < outModel->num_frames; i++)
	{
		for (l = 0; l < outModel->num_tags; l++, inTag++, outTag++)
		{
			memcpy(outTag->name, inTag->name, MD3_MAX_PATH);
			for (j = 0; j < 3; j++) {
				outTag->orient.origin[j] = LittleFloat(inTag->orient.origin[j]);
				outTag->orient.axis[0][j] = LittleFloat(inTag->orient.axis[0][j]);
				outTag->orient.axis[1][j] = LittleFloat(inTag->orient.axis[1][j]);
				outTag->orient.axis[2][j] = LittleFloat(inTag->orient.axis[2][j]);
			}
		}
	}

	//
	// load the meshes
	//
	ClearBounds(mod->mins, mod->maxs);
	mod->flags = 0;

	inMesh = (dmd3mesh_t *)((byte *)inModel + LittleLong(inModel->ofs_meshes));
	outMesh = outModel->meshes = Hunk_Alloc(sizeof(md3Mesh_t)*outModel->num_meshes);

	for (i = 0; i < outModel->num_meshes; i++, outMesh++)
	{
		memcpy(outMesh->name, inMesh->name, MD3_MAX_PATH);

		if (strncmp((const char *)inMesh->id, "IDP3", 4))
		{
			VID_Error(ERR_DROP, "mesh %s in model %s has wrong id (%i should be %i)",
				outMesh->name, mod->name, LittleLong((int)inMesh->id), IDMD3HEADER);
		}

		outMesh->num_tris = LittleLong(inMesh->num_tris);
		outMesh->num_skins = LittleLong(inMesh->num_skins);
		outMesh->num_verts = LittleLong(inMesh->num_verts);

		if (outMesh->num_skins <= 0)
			VID_Error(ERR_DROP, "mesh %i in model %s has no skins", i, mod->name);
		else if (outMesh->num_skins > MD3_MAX_SHADERS)
			VID_Error(ERR_DROP, "mesh %i in model %s has too many skins", i, mod->name);

		if (outMesh->num_tris <= 0)
			VID_Error(ERR_DROP, "mesh %i in model %s has no triangles", i, mod->name);
		else if (outMesh->num_tris > MD3_MAX_TRIANGLES)
			VID_Error(ERR_DROP, "mesh %i in model %s has too many triangles", i, mod->name);

		if (outMesh->num_verts <= 0)
			VID_Error(ERR_DROP, "mesh %i in model %s has no vertices", i, mod->name);
		else if (outMesh->num_verts > MD3_MAX_VERTS)
			VID_Error(ERR_DROP, "mesh %i in model %s has too many vertices, %i (4096 max)", i, mod->name, outMesh->num_verts);

		//
		// register all skins
		//
		inSkin = (dmd3skin_t *)((byte *)inMesh + LittleLong(inMesh->ofs_skins));

		for (j = 0; j < outMesh->num_skins; j++, inSkin++)
		{
			if (!inSkin->name[0])
			{
				outMesh->skinsAlbedo[j] = 
				outMesh->skinsNormal[j] = outMesh->skinsLight[j] =
				outMesh->skinsEnv[j]	= outMesh->skinsRgh[j] = r_notexture;
				continue;
			}

			char tex[128];
			memcpy(name, inSkin->name, MD3_MAX_PATH);
			outMesh->skinsAlbedo[j] = GL_FindImage(name, it_skin);

			// GlowMaps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_light.tga");
			outMesh->skinsLight[j] = GL_FindImage(tex, it_skin);
			if (!outMesh->skinsLight[j])
				outMesh->skinsLight[j] = r_notexture;

			// Normal maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_bump.tga");
			outMesh->skinsNormal[j] = GL_FindImage(tex, it_bump);
			if (!outMesh->skinsNormal[j])
				outMesh->skinsNormal[j] = r_notexture;

			// Roughness maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_rgh.tga");
			outMesh->skinsRgh[j] = GL_FindImage(tex, it_skin);
			if (!outMesh->skinsRgh[j])
				outMesh->skinsRgh[j] = r_notexture;

			// Env maps loading
			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_env.tga");
			outMesh->skinsEnv[j] = GL_FindImage(tex, it_skin);
			if (!outMesh->skinsEnv[j])
				outMesh->skinsEnv[j] = r_notexture;

			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_ao.tga");
			outMesh->skinsAO[j] = GL_FindImage(tex, it_skin);
			if (!outMesh->skinsAO[j])
				outMesh->skinsAO[j] = r_whiteMap;

			strcpy(tex, name);
			tex[strlen(tex) - 4] = 0;
			strcat(tex, "_local.tga");
			outMesh->skinsSkinLocal[j] = GL_FindImage(tex, it_bump);
			if (!outMesh->skinsSkinLocal[j])
				outMesh->skinsSkinLocal[j] = r_defBump;
		}

		//
		// load the indexes
		//
		inIndex = (unsigned *)((byte *)inMesh + LittleLong(inMesh->ofs_tris));
		outIndex = outMesh->indexes = (index_t*)Hunk_Alloc(sizeof(index_t) * outMesh->num_tris * 3);

		for (j = 0; j < outMesh->num_tris; j++, inIndex += 3, outIndex += 3)
		{
			outIndex[0] = (index_t)LittleLong(inIndex[0]);
			outIndex[1] = (index_t)LittleLong(inIndex[1]);
			outIndex[2] = (index_t)LittleLong(inIndex[2]);
		}

		//
		// load the texture coordinates
		//
		inCoord = (dmd3coord_t *)((byte *)inMesh + LittleLong(inMesh->ofs_tcs));
		outCoord = outMesh->stcoords = Hunk_Alloc(sizeof(md3ST_t) * outMesh->num_verts);

		for (j = 0; j < outMesh->num_verts; j++, inCoord++, outCoord++)
		{
			outCoord->st[0] = LittleFloat(inCoord->st[0]);
			outCoord->st[1] = LittleFloat(inCoord->st[1]);
		}

		//
		// load all vertexes and calc TBN
		//
		inVerts = (dmd3vertex_t *)((byte *)inMesh + LittleLong(inMesh->ofs_verts));
		outVerts = outMesh->vertexes = Hunk_Alloc(outModel->num_frames * outMesh->num_verts * sizeof(md3Vertex_t));

		for (l = 0; l < outModel->num_frames; l++){

			// for all frames
			memset(tangents, 0, outMesh->num_verts * sizeof(vec3_t));
			memset(binormals, 0, outMesh->num_verts * sizeof(vec3_t));

			outVerts = outMesh->vertexes + l * outMesh->num_verts;
			
			for (j = 0; j < outMesh->num_verts; j++, inVerts++, outVerts++){

				vec3_t	boundsPoints, norm, translate;
				int		x;

				for (x = 0; x < 3; x++){

					translate[x] = LittleFloat(inFrame->translate[x]);
					outVerts->xyz[x] = (float)LittleShort(inVerts->point[x]) * scaleMD3[x] + translate[x];
					boundsPoints[x] = outVerts->xyz[x] + outModel->frames[l].translate[x];
				}

				AddPointToBounds(boundsPoints, mod->mins, mod->maxs); // add points for bound box

				lat = (float)((inVerts->norm >> 8) & 0xFF) * M_PI / 128.0;
				lng = (float)((inVerts->norm >> 0) & 0xFF) * M_PI / 128.0;
				
				norm[0] = sin(lng) * cos(lat);
				norm[1] = sin(lng) * sin(lat);
				norm[2] = cos(lng);
				
				VectorNormalize(norm);
				VectorCopy(norm, outVerts->normal);
			}

			//for all tris
			outVerts = outMesh->vertexes + l * outMesh->num_verts;
			for (j = 0; j<outMesh->num_tris; j++)
			{
				static vec3_t tangent;
				static vec3_t binormal;

				CalcTangent4MD3(&outMesh->indexes[j * 3], outVerts, outMesh->stcoords, tangent, binormal);
				// for all vertices in the tri
				for (k = 0; k<3; k++){

					m = outMesh->indexes[j * 3 + k];
					VectorAdd(tangents[m], tangent, tangents[m]);
					VectorAdd(binormals[m], binormal, binormals[m]);
				}
			}

			// normalize averages
			for (j = 0; j<outMesh->num_verts; j++)
			{
				VectorNormalize(tangents[j]);
				VectorNormalize(binormals[j]);
				VectorCopy(tangents[j], outVerts[j].tangent);
				VectorCopy(binormals[j], outVerts[j].binormal);
				}
		}

		//
		// build triangle neighbours
		//
		inMesh = (dmd3mesh_t *)((byte *)inMesh + LittleLong(inMesh->meshsize));
		outMesh->triangles = (neighbours_t*)Hunk_Alloc(sizeof(neighbours_t) * outMesh->num_tris);
		R_BuildTriangleNeighbors(outMesh->triangles, outMesh->indexes, outMesh->num_tris);

		if (!Q_strcasecmp(outMesh->name, "MF"))
			outMesh->muzzle = qtrue;
		else
			outMesh->muzzle = qfalse;
		
		if (!Q_strcasecmp(outMesh->name, "ALPHATEST")) {
			outMesh->skinAlphatest = qtrue;
		}
		else
			outMesh->skinAlphatest = qfalse;

		outMesh->flags = MESH_OPAQUE;

		if (!Q_strcasecmp(outMesh->name, "TRANSLUS")) 
			outMesh->flags = MESH_TRANSLUSCENT;

		if (!Q_strcasecmp(outMesh->name, "MESH_SSS"))
			outMesh->flags = MESH_SSS;
}

	mod->type = mod_alias_md3;

	/// Calc md3 bounds and radius...
	vec3_t	tempr, tempv;
	tempr[0] = mod->maxs[0] - mod->mins[0];
	tempr[1] = mod->maxs[1] - mod->mins[1];
	tempr[2] = 0;
	tempv[0] = 0;
	tempv[1] = 0;
	tempv[2] = mod->maxs[2] - mod->mins[2];
	mod->radius = max(VectorLength(tempr), VectorLength(tempv));

	for (i = 0; i<3; i++)
		mod->center[i] = (mod->maxs[i] + mod->mins[i]) * 0.5;
}


/*
=================
R_CullAliasModel
=================
*/
qboolean R_CullMD3Model(vec3_t bbox[8], entity_t *e)
{
	int			i, j;
	vec3_t		mins, maxs, tmp; //angles;
	vec3_t		vectors[3];
	md3Model_t	*md3Hdr;
	md3Frame_t	*currFrame, *oldFrame;
	int			mask, aggregatemask = ~0;

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	if ((e->frame >= md3Hdr->num_frames) || (e->frame < 0))
	{
		Com_DPrintf("R_Cullmd3Model %s: no such frame %d\n", currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ((e->oldframe >= md3Hdr->num_frames) || (e->oldframe < 0))
	{
		Com_DPrintf("R_Cullmd3Model %s: no such oldframe %d\n", currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	currFrame = md3Hdr->frames + e->frame;
	oldFrame = md3Hdr->frames + e->oldframe;

	// compute axially aligned mins and maxs
	if (currFrame == oldFrame)
	{
		VectorCopy(currFrame->mins, mins);
		VectorCopy(currFrame->maxs, maxs);
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			if (currFrame->mins[i] < oldFrame->mins[i])
				mins[i] = currFrame->mins[i];
			else
				mins[i] = oldFrame->mins[i];

			if (currFrame->maxs[i] > oldFrame->maxs[i])
				maxs[i] = currFrame->maxs[i];
			else
				maxs[i] = oldFrame->maxs[i];
		}
	}

	// Compute and rotate bonding box
	AngleVectors(e->angles, vectors[0], vectors[1], vectors[2]);
	VectorSubtract(vec3_origin, vectors[1], vectors[1]); // AngleVectors returns "right" instead of "left"

	for (i = 0; i < 8; i++) {
		tmp[0] = ((i & 1) ? mins[0] : maxs[0]);
		tmp[1] = ((i & 2) ? mins[1] : maxs[1]);
		tmp[2] = ((i & 4) ? mins[2] : maxs[2]);

		bbox[i][0] = vectors[0][0] * tmp[0] + vectors[1][0] * tmp[1] + vectors[2][0] * tmp[2] + e->origin[0];
		bbox[i][1] = vectors[0][1] * tmp[0] + vectors[1][1] * tmp[1] + vectors[2][1] * tmp[2] + e->origin[1];
		bbox[i][2] = vectors[0][2] * tmp[0] + vectors[1][2] * tmp[1] + vectors[2][2] * tmp[2] + e->origin[2];
	}

	// cull
	for (i = 0; i< 8; i++)
	{
		mask = 0;
		for (j = 0; j<6; j++)
		{
			float dp = DotProduct(frustum[j].normal, bbox[i]);
			if ((dp - frustum[j].dist) < 0)
				mask |= (1 << j);
		}

		aggregatemask &= mask;
	}

	if (aggregatemask)
		return qtrue;

	return qfalse;
}

void CheckEntityFrameMD3(md3Model_t *paliashdr)
{
	if ((currententity->frame >= paliashdr->num_frames) || (currententity->frame < 0))
	{
		Com_Printf("^3CheckEntityFrameMD3, %s: no such frame %d\n", currentmodel->name, currententity->frame);
		currententity->frame = 0;
	}

	if ((currententity->oldframe >= paliashdr->num_frames) || (currententity->oldframe < 0))
	{
		Com_Printf("^3CheckEntityFrameMD3, %s: no such oldframe %d\n", currentmodel->name, currententity->oldframe);
		currententity->oldframe = 0;
	}

}
static vec3_t	vertexArray		[MD3_MAX_TRIANGLES * 3];
static vec3_t	normalArray		[MD3_MAX_TRIANGLES * 3];
static vec3_t	tangentArray	[MD3_MAX_TRIANGLES * 3];
static vec3_t	binormalArray	[MD3_MAX_TRIANGLES * 3];
static vec4_t	colorArray		[MD3_MAX_TRIANGLES * 4];

void R_DrawMD3Mesh(qboolean weapon) {

	md3Model_t	*md3Hdr;
	vec3_t		bbox[8], temp, viewOrg;
	int			i, j, k;
	float		frontlerp, backlerp, lum;
	md3Frame_t	*frame, *oldframe;
	vec3_t		move, delta, vectors[3];
	md3Vertex_t	*v, *ov, *verts, *oldVerts;
	vec3_t		luminance = { 0.2125, 0.7154, 0.0721 };
	image_t     *skin, *light, *normal, *ao, *rgh;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		if (!weapon || r_leftHand->integer == 2)
			return;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (R_CullMD3Model(bbox, currententity))
		return;

	if (currententity->flags & RF_NOCULL)
		GL_Disable(GL_CULL_FACE);

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	SetModelsLight();

	if (r_skipStaticLights->integer) {

		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(shadelight, 0.5, 0.5, 0.5);
	}
	else {
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(shadelight, 0.1, 0.1, 0.1);
	}

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		VectorSet(shadelight, 1, 1, 1);
		
	CheckEntityFrameMD3(md3Hdr);

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

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
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	R_SetupEntityMatrix(currententity);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	// setup program
	GL_BindProgram(md3AmbientProgram);

	qglUniform1i(U_ENV_PASS, 0);
	qglUniform1i(U_SHELL_PASS, 0);
	qglUniform1i(U_TRANS_PASS, 0);
	qglUniform1f(U_COLOR_MUL, 1.0);

	float alphaShift = sin(ref_realtime * 5.666);
	alphaShift = (alphaShift + 1) * 0.5f;
	alphaShift = clamp(alphaShift, 0.01, 1.0);

	qglUniform1f(U_COLOR_OFFSET, alphaShift);
	qglUniform1f(U_ENV_SCALE, 0.1);
	
	VectorSubtract(r_origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector(currententity->axis, temp, viewOrg);

	qglUniform3fv(U_VIEW_POS, 1, viewOrg);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	for (i = 0; i < md3Hdr->num_meshes; i++) {

		md3Mesh_t *mesh = &md3Hdr->meshes[i];

		if (!(mesh->flags & MESH_OPAQUE)) 
			continue;

		v = mesh->vertexes + currententity->frame * mesh->num_verts;
		ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;
		
		c_alias_polys += md3Hdr->meshes[i].num_tris;

		if (mesh->muzzle) {
			GL_Enable(GL_BLEND);
			GL_DepthMask(0);
			GL_Disable(GL_CULL_FACE);
			qglUniform1f(U_COLOR_OFFSET, 1.0);
			qglUniform1f(U_COLOR_MUL, 2.0);
			GL_BlendFunc(GL_ONE, GL_ONE);
		}

		if (mesh->skinAlphatest) {
			qglUniform1i(U_PARAM_INT_0, 1);
			rgh = mesh->skinsRgh[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
			if (!rgh)
				rgh = r_notexture;
			GL_MBind(GL_TEXTURE5, rgh->texnum);
		}
		else 
			qglUniform1i(U_PARAM_INT_0, 0);

		skin = mesh->skinsAlbedo[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!skin || skin == r_notexture)
		{
			if (currententity->skin)
			{
				skin = currententity->skin;	// custom player skin
			}
		}
		if (!skin)
			skin = r_notexture;

		light = mesh->skinsLight[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!light)
			light = r_notexture;

		normal = mesh->skinsNormal[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!normal)
			normal = r_defBump;
		
		ao = mesh->skinsAO[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!ao)
			ao = r_whiteMap;

		for (j = 0; j < mesh->num_verts; j++, v++, ov++) {

			if (mesh->muzzle)
				Vector4Set(md3ColorCache[j], 1.0, 1.0, 1.0, 1.0);
			else
				Vector4Set(md3ColorCache[j], shadelight[0], shadelight[1], shadelight[2], 1.0);

			md3VertexCache[j][0] = move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp;
			md3VertexCache[j][1] = move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp;
			md3VertexCache[j][2] = move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp;
		}

		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, md3VertexCache);
		qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, mesh->stcoords);
		qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, md3ColorCache);

		GL_MBind(GL_TEXTURE0, skin->texnum);
		GL_MBind(GL_TEXTURE1, light->texnum);
		GL_MBind(GL_TEXTURE2, r_envTex->texnum);
		GL_MBind(GL_TEXTURE3, normal->texnum);
		GL_MBind(GL_TEXTURE4, ao->texnum);

		qglDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, mesh->indexes);
	
		if (mesh->muzzle) {
			GL_Disable(GL_BLEND);
			GL_DepthMask(1);
			GL_Enable(GL_CULL_FACE);
			GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

		// Draw Transluscent meshes

		qglEnableVertexAttribArray(ATT_NORMAL);
		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_DepthMask(0);
		qglUniform1i(U_ENV_PASS, 1);
		qglUniform1i(U_TRANS_PASS, 1);

		lum = DotProduct(luminance, shadelight);
		qglUniform1f(U_ENV_SCALE, lum);

		for (i = 0; i < md3Hdr->num_meshes; i++) {

			md3Mesh_t *mesh = &md3Hdr->meshes[i];
			v = mesh->vertexes + currententity->frame * mesh->num_verts;
			ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;

			c_alias_polys += md3Hdr->meshes[i].num_tris;

			if (mesh->muzzle)
				continue;

			if (!(mesh->flags & MESH_TRANSLUSCENT))
				continue;

			skin = mesh->skinsAlbedo[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
			if (!skin || skin == r_notexture)
			{
				if (currententity->skin)
				{
					skin = currententity->skin;	// custom player skin
				}
			}
			if (!skin)
				skin = r_notexture;

			normal = mesh->skinsNormal[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
			if (!normal)
				normal = r_defBump;

			for (j = 0; j < mesh->num_verts; j++, v++, ov++) {
				if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
					Vector4Set(md3ColorCache[j], 0.33, 0.33, 0.33, 0.5);
				else
					Vector4Set(md3ColorCache[j], shadelight[0], shadelight[1], shadelight[2], 0.5);
				md3VertexCache[j][0] = move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp;
				md3VertexCache[j][1] = move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp;
				md3VertexCache[j][2] = move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp;
			}

			verts = mesh->vertexes + currententity->frame * mesh->num_verts;
			oldVerts = mesh->vertexes + currententity->oldframe * mesh->num_verts;

			for (k = 0; k< mesh->num_verts; k++) {

				normalArray[k][0] = verts[k].normal[0] * frontlerp + oldVerts[k].normal[0] * backlerp;
				normalArray[k][1] = verts[k].normal[1] * frontlerp + oldVerts[k].normal[1] * backlerp;
				normalArray[k][2] = verts[k].normal[2] * frontlerp + oldVerts[k].normal[2] * backlerp;
			}

			qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, md3VertexCache);
			qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, mesh->stcoords);
			qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, md3ColorCache);
			qglVertexAttribPointer(ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);
						
			GL_MBind(GL_TEXTURE0, skin->texnum);
			GL_MBind(GL_TEXTURE1, r_notexture->texnum);
			GL_MBind(GL_TEXTURE2, r_envTex->texnum);
			GL_MBind(GL_TEXTURE3, normal->texnum);


			qglDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, mesh->indexes);
		}

	GL_DepthMask(1);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
	qglDisableVertexAttribArray(ATT_NORMAL);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

	if (currententity->flags & RF_NOCULL)
		GL_Enable(GL_CULL_FACE);
}

qboolean R_Md3InLightBound() {

	vec3_t mins, maxs;
	int i;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else
	{
		VectorAdd(currententity->origin, currententity->model->maxs, maxs);
		VectorAdd(currententity->origin, currententity->model->mins, mins);
	}

	if (currentShadowLight->_cone) {

		if (R_CullConeLight(mins, maxs, currentShadowLight->frust))
			return qfalse;

	}
	else if (currentShadowLight->spherical) {

		if (!BoundsAndSphereIntersect(mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return qfalse;
	}
	else {

		if (!BoundsIntersect(mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return qfalse;
	}

	if (!InLightVISEntity())
		return qfalse;

	return qtrue;

}

void R_UpdateLightAliasUniforms();

void R_DrawMD3MeshLight(qboolean weapon) {

	md3Model_t	*md3Hdr;
	vec3_t		bbox[8];
	int			i, j, k;
	float		frontlerp, backlerp;
	md3Frame_t	*frame, *oldframe;
	vec3_t		move, delta, vectors[3], maxs;
	md3Vertex_t	*v, *ov;
	md3Vertex_t	*verts, *oldVerts;
	image_t     *skin, *rgh, *normal;
	qboolean inWater;
	vec3_t tmp, oldLight, oldView;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		if (!weapon || r_leftHand->integer == 2)
			return;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (!(currententity->flags & RF_WEAPONMODEL)) {
		if (R_CullMD3Model(bbox, currententity))
			return;
	}

	if (!R_Md3InLightBound())
		return;

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));


	R_SetupEntityMatrix(currententity);

	VectorCopy(currentShadowLight->origin, oldLight);
	VectorCopy(r_origin, oldView);

	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, r_origin);


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
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);

	GL_PolygonOffset(-1.0, -1.0);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TANGENT);
	qglEnableVertexAttribArray(ATT_BINORMAL);
	qglEnableVertexAttribArray(ATT_NORMAL);
	qglEnableVertexAttribArray(ATT_TEX0);

	// setup program
	GL_BindProgram(aliasBumpProgram);

	VectorAdd(currententity->origin, currententity->model->maxs, maxs);
	if (CL_PMpointcontents(maxs) & MASK_WATER)
		inWater = qtrue;
	else
		inWater = qfalse;

	R_UpdateLightAliasUniforms();

	qglUniform1i(U_USE_AUTOBUMP, 0);
	
	if (inWater && currentShadowLight->castCaustics && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		qglUniform1i(U_USE_CAUSTICS, 1);
	else
		qglUniform1i(U_USE_CAUSTICS, 0);

	for (i = 0; i < md3Hdr->num_meshes; i++) {

		md3Mesh_t *mesh = &md3Hdr->meshes[i];
		v = mesh->vertexes + currententity->frame * mesh->num_verts;
		ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;
		
		if (mesh->muzzle)
			continue;
		
	//	if (mesh->flags & MESH_TRANSLUSCENT)
	//		continue;

		if (mesh->flags & MESH_SSS)
			qglUniform1i(U_PARAM_INT_2, 1);
		else
			qglUniform1i(U_PARAM_INT_2, 0);

		if (mesh->skinAlphatest)
			qglUniform1i(U_PARAM_INT_1, 1);
		else
			qglUniform1i(U_PARAM_INT_1, 0);

		if (weapon) {
			trace_t trace;
			vec3_t ray;
			VectorCopy(r_newrefdef.vieworg, ray);
			ray[2] = r_origin[2] + 4096.0;
			trace = CL_PMTraceWorld(r_newrefdef.vieworg, vec3_origin, vec3_origin, ray, MASK_SOLID, qfalse);

			if (trace.fraction != 1.0) {

				if (trace.surface->flags & SURF_SKY) {
					qglUniform1i(U_PARAM_INT_3, 1);
					mat4_t tmpMatrix, m;

					Mat4_Identity(m);
					Mat4_Copy(r_newrefdef.skyMatrix, tmpMatrix);

					Mat4_Identity(m);
					
					if (skyrotate)
						Mat4_Rotate(m, r_newrefdef.time* -skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);

					if (currententity->angles[1])
						Mat4_Rotate(m, currententity->angles[1], 0, 0, 1);
					if (currententity->angles[0])
						Mat4_Rotate(m, currententity->angles[0], 0, 1, 0);
					if (currententity->angles[2])
						Mat4_Rotate(m, currententity->angles[2], 1, 0, 0);

					Mat4_Translate(m, -currententity->origin[0], -currententity->origin[1], -currententity->origin[2]);
					Mat4_Copy(m, tmpMatrix);

					qglUniformMatrix4fv(U_TEXTURE0_MATRIX, 1, qfalse, (const float*)tmpMatrix);
				}
				else
					qglUniform1i(U_PARAM_INT_3, 0);
			}
		}
		else
			qglUniform1i(U_PARAM_INT_3, 0);

		c_alias_polys += md3Hdr->meshes[i].num_tris;

		skin = mesh->skinsAlbedo[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!skin || skin == r_notexture)
		{
			if (currententity->skin)
			{
				skin = currententity->skin;	// custom player skin
			}
		}
		if (!skin)
			skin = r_notexture;

		normal = mesh->skinsNormal[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!normal)
			normal = r_defBump;

		rgh = mesh->skinsRgh[min(currententity->skinnum, MD3_MAX_SKINS - 1)];
		if (!rgh)
			rgh = r_notexture;

		for (j = 0; j < mesh->num_verts; j++, v++, ov++) {

			md3VertexCache[j][0] = move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp;
			md3VertexCache[j][1] = move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp;
			md3VertexCache[j][2] = move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp;
		}

		verts = mesh->vertexes + currententity->frame * mesh->num_verts;
		oldVerts = mesh->vertexes + currententity->oldframe * mesh->num_verts;

		for (k = 0; k< mesh->num_verts; k++) {

				tangentArray[k][0] = verts[k].tangent[0] * frontlerp + oldVerts[k].tangent[0] * backlerp;
				tangentArray[k][1] = verts[k].tangent[1] * frontlerp + oldVerts[k].tangent[1] * backlerp;
				tangentArray[k][2] = verts[k].tangent[2] * frontlerp + oldVerts[k].tangent[2] * backlerp;

				binormalArray[k][0] = verts[k].binormal[0] * frontlerp + oldVerts[k].binormal[0] * backlerp;
				binormalArray[k][1] = verts[k].binormal[1] * frontlerp + oldVerts[k].binormal[1] * backlerp;
				binormalArray[k][2] = verts[k].binormal[2] * frontlerp + oldVerts[k].binormal[2] * backlerp;

				normalArray[k][0] = verts[k].normal[0] * frontlerp + oldVerts[k].normal[0] * backlerp;
				normalArray[k][1] = verts[k].normal[1] * frontlerp + oldVerts[k].normal[1] * backlerp;
				normalArray[k][2] = verts[k].normal[2] * frontlerp + oldVerts[k].normal[2] * backlerp;
		}

		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, md3VertexCache);
		qglVertexAttribPointer(ATT_TANGENT, 3, GL_FLOAT, qfalse, 0, tangentArray);
		qglVertexAttribPointer(ATT_BINORMAL, 3, GL_FLOAT, qfalse, 0, binormalArray);
		qglVertexAttribPointer(ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);
		qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, mesh->stcoords);

		GL_MBind(GL_TEXTURE0, normal->texnum);
		GL_MBind(GL_TEXTURE1, skin->texnum);
		GL_MBind(GL_TEXTURE2, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
		GL_MBindCube(GL_TEXTURE3, r_lightCubeMap[currentShadowLight->filter]->texnum);

		if (rgh == r_notexture)
			qglUniform1i(U_USE_RGH_MAP, 0);
		else {
			qglUniform1i(U_USE_RGH_MAP, 1);
			GL_MBind(GL_TEXTURE4, rgh->texnum);
		}

		GL_MBind(GL_TEXTURE5, skinBump->texnum);
		
		GL_MBindCube(GL_TEXTURE6, skyCube);

		qglDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, mesh->indexes);
	}

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TANGENT);
	qglDisableVertexAttribArray(ATT_BINORMAL);
	qglDisableVertexAttribArray(ATT_NORMAL);
	qglDisableVertexAttribArray(ATT_TEX0);

	VectorCopy(oldLight, currentShadowLight->origin);
	VectorCopy(oldView, r_origin);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);
}


void R_DrawMD3ShellMesh(qboolean weapon) {

	md3Model_t		*md3Hdr;
	vec3_t			bbox[8];
	int				i, j, k;
	float			frontlerp, backlerp;
	md3Frame_t		*frame, *oldframe;
	vec3_t			move, delta, vectors[3], tmp, viewOrg;
	md3Vertex_t		*v, *ov, *verts, *oldVerts;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		if (!weapon || r_leftHand->integer == 2)
			return;

	if (R_CullMD3Model(bbox, currententity))
		return;

	md3Hdr = (md3Model_t *)currentmodel->extraData;

	CheckEntityFrameMD3(md3Hdr);

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

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
		move[j] = backlerp * move[j] + frontlerp * frame->translate[j];

	R_SetupEntityMatrix(currententity);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_NORMAL);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, viewOrg);

	// setup program
	GL_BindProgram(md3AmbientProgram);
	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE);
	vec2_t shellParams = { r_newrefdef.time * 0.45, 0.1f };

	qglUniform1i(U_SHELL_PASS, 1); // deform in vertex shader
	qglUniform3fv(U_VIEW_POS, 1, viewOrg);
	qglUniform2fv(U_SHELL_PARAMS, 1, shellParams);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	if (currententity->flags & RF_SHELL_BLUE)
		GL_MBind(GL_TEXTURE0, r_texshell[0]->texnum);
	if (currententity->flags & RF_SHELL_RED)
		GL_MBind(GL_TEXTURE0, r_texshell[1]->texnum);
	if (currententity->flags & RF_SHELL_GREEN)
		GL_MBind(GL_TEXTURE0, r_texshell[2]->texnum);
	if (currententity->flags & RF_SHELL_GOD)
		GL_MBind(GL_TEXTURE0, r_texshell[3]->texnum);
	if (currententity->flags & RF_SHELL_HALF_DAM)
		GL_MBind(GL_TEXTURE0, r_texshell[4]->texnum);
	if (currententity->flags & RF_SHELL_DOUBLE)
		GL_MBind(GL_TEXTURE0, r_texshell[5]->texnum);

	for (i = 0; i < md3Hdr->num_meshes; i++) {

		md3Mesh_t *mesh = &md3Hdr->meshes[i];

		if (mesh->muzzle)
			continue;

		verts = mesh->vertexes + currententity->frame * mesh->num_verts;
		oldVerts = mesh->vertexes + currententity->oldframe * mesh->num_verts;

		c_alias_polys += md3Hdr->meshes[i].num_tris;

		for (k = 0; k < mesh->num_verts; k++) {

			normalArray[k][0] = verts[k].normal[0] * frontlerp + oldVerts[k].normal[0] * backlerp;
			normalArray[k][1] = verts[k].normal[1] * frontlerp + oldVerts[k].normal[1] * backlerp;
			normalArray[k][2] = verts[k].normal[2] * frontlerp + oldVerts[k].normal[2] * backlerp;
		}

		v = mesh->vertexes + currententity->frame * mesh->num_verts;
		ov = mesh->vertexes + currententity->oldframe * mesh->num_verts;

		for (j = 0; j < mesh->num_verts; j++, v++, ov++) {

			md3VertexCache[j][0] = move[0] + ov->xyz[0] * backlerp + v->xyz[0] * frontlerp;
			md3VertexCache[j][1] = move[1] + ov->xyz[1] * backlerp + v->xyz[1] * frontlerp;
			md3VertexCache[j][2] = move[2] + ov->xyz[2] * backlerp + v->xyz[2] * frontlerp;
		}

		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, md3VertexCache);
		qglVertexAttribPointer(ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);

		qglDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_SHORT, mesh->indexes);
	}

	qglDisableVertexAttribArray(ATT_POSITION);

	GL_BlendFunc(GL_ONE, GL_ONE);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

}