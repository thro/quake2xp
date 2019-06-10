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
// r_calcAlias.c: calc triangles for alias models


#include "r_local.h"

vec3_t	tempVertexArray	[MAX_VERTICES  * 4];

static vec3_t	vertexArray		[MAX_TRIANGLES * 3];
static vec3_t	normalArray		[MAX_TRIANGLES * 3];
static vec3_t	tangentArray	[MAX_TRIANGLES * 3];
static vec3_t	binormalArray	[MAX_TRIANGLES * 3];
static vec4_t	colorArray		[MAX_TRIANGLES * 4];

extern float	*shadedots;

void R_CalcAliasFrameLerp (dmdl_t *paliashdr, float shellScale) {
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	float	frontlerp;
	vec3_t	move, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	float	*lerp;
	float	backlerp;

	if (currentmodel->numFrames < 1)
		return;

	backlerp = currententity->backlerp;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;
	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	frontlerp = 1.0 - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, move);

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		vec3_t	temp;
		VectorCopy (move, temp);
		AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);
		move[0] = DotProduct (temp, vectors[0]);
		move[1] = -DotProduct (temp, vectors[1]);
		move[2] = DotProduct (temp, vectors[2]);
	}

	VectorAdd (move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];
		frontv[i] = frontlerp*frame->scale[i];
		backv[i] = backlerp*oldframe->scale[i];
	}

	lerp = tempVertexArray[0];
	
	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) {
		for (i = 0; i < paliashdr->num_xyz; i++, v++, ov++, lerp += 3) {
			float *normal = q_byteDirs[verts[i].lightnormalindex];
			lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0] + normal[0] * shellScale;
			lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1] + normal[1] * shellScale;
			lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2] + normal[2] * shellScale;
		}
	}
	else {
		for (i = 0; i < paliashdr->num_xyz; i++, v++, ov++, lerp += 3) {

			lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0];
			lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1];
			lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2];
		}
	}
}

int CL_PMpointcontents (vec3_t point);

void GL_DrawAliasFrameLerp (dmdl_t *paliashdr, vec3_t lightColor) {
	int				index_xyz;
	int				i, j, jj = 0;
	dtriangle_t		*tris;
	image_t			*skin, *skinNormalmap, *glowskin;
	float			alphaShift, alpha;
	float			backlerp, frontlerp;
	int				index2, oldindex2;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*verts, *oldverts;

	alphaShift = sin (ref_realtime * currentmodel->glowCfg[2]);
	alphaShift = (alphaShift + 1) * 0.5f;
	alphaShift = clamp (alphaShift, currentmodel->glowCfg[0], currentmodel->glowCfg[1]);

	if (currententity->flags & RF_TRANSLUCENT) {
		alpha = currententity->alpha;
	}
	else
		alpha = 1.0;

	if (currententity->flags & RF_NOCULL) {
		GL_Disable(GL_CULL_FACE);
		GL_DepthMask(0);
	}

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (r_skipStaticLights->integer) {

		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(lightColor, 0.5, 0.5, 0.5);
	}
	else {
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(lightColor, 0.1, 0.1, 0.1);
	}

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		VectorSet(lightColor, 1, 1, 1);

	// select skin
	if (currententity->skin)
		skin = currententity->skin;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skin = currentmodel->skins[0];
			currententity->skinnum = 0;
		}
		else {
			skin = currentmodel->skins[currententity->skinnum];
			if (!skin) {
				skin = currentmodel->skins[0];
				currententity->skinnum = 0;
			}
		}
	}

	if (!skin)
		skin = r_notexture;

	// select skin
	if (currententity->bump)
		skinNormalmap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skinNormalmap = currentmodel->skins_normal[0];
			currententity->skinnum = 0;
		}
		else {
			skinNormalmap = currentmodel->skins_normal[currententity->skinnum];
			if (!skin) {
				skinNormalmap = currentmodel->skins_normal[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!skinNormalmap)
		skinNormalmap = r_defBump;

	glowskin = currentmodel->glowtexture[currententity->skinnum];

	if (!glowskin)
		glowskin = r_notexture;

	if (!skin)
		skin = r_notexture;

	R_CalcAliasFrameLerp (paliashdr, 0);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);

	qglEnableVertexAttribArray (ATT_NORMAL);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);

	qglEnableVertexAttribArray (ATT_COLOR);
	qglVertexAttribPointer (ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorArray);

//	qglBindBufferARB (GL_ARRAY_BUFFER_ARB, currentmodel->vboId);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, currentmodel->st);


	c_alias_polys += paliashdr->num_tris;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	jj = 0;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	oldverts = oldframe->verts;
	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy (tempVertexArray[index_xyz], vertexArray[jj]);

			VA_SetElem4 (colorArray[jj], lightColor[0], lightColor[1], lightColor[2], 1.0);

			if (currentmodel->envMap) {
				index2 = verts[index_xyz].lightnormalindex;
				oldindex2 = oldverts[index_xyz].lightnormalindex;
				normalArray[jj][0] = q_byteDirs[oldindex2][0] * backlerp + q_byteDirs[index2][0] * frontlerp;
				normalArray[jj][1] = q_byteDirs[oldindex2][1] * backlerp + q_byteDirs[index2][1] * frontlerp;
				normalArray[jj][2] = q_byteDirs[oldindex2][2] * backlerp + q_byteDirs[index2][2] * frontlerp;
			}
		}
	}

	// setup program
	GL_BindProgram (aliasAmbientProgram);

	if (currentmodel->envMap)
		qglUniform1i (U_ENV_PASS, 1);
	else
		qglUniform1i (U_ENV_PASS, 0);

	qglUniform1i (U_SHELL_PASS, 0);

	qglUniform1f (U_COLOR_MUL, r_textureColorScale->value);
	qglUniform1f (U_COLOR_OFFSET, alphaShift);

	GL_MBind (GL_TEXTURE0, skin->texnum);
	GL_MBind (GL_TEXTURE1, glowskin->texnum);
	GL_MBind (GL_TEXTURE2, r_envTex->texnum);
	GL_MBind (GL_TEXTURE3, skinNormalmap->texnum);

	qglUniform1f(U_ENV_SCALE, currentmodel->envScale);

	if (r_ssao->integer && !(currententity->flags & RF_WEAPONMODEL) && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !(r_newrefdef.rdflags & RDF_IRGOGGLES)) {
		GL_MBindRect (GL_TEXTURE4, fboColor[fboColorIndex]->texnum);
		qglUniform1i(U_USE_SSAO, 1);
	}
	else
		qglUniform1i(U_USE_SSAO, 0);

	qglUniform3fv(U_VIEW_POS, 1, r_origin);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	qglDrawArrays (GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_COLOR);
	qglDisableVertexAttribArray (ATT_TEX0);
//	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	if (currententity->flags & RF_NOCULL) {
		GL_Enable(GL_CULL_FACE);
		GL_DepthMask(1);
	}
}

void GL_DrawAliasFrameLerpShell (dmdl_t *paliashdr) {
	int			index_xyz, i, j, jj = 0;
	dtriangle_t	*tris;
	unsigned	defBits = 0;
	float		backlerp, frontlerp;
	int			index2, oldindex2;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*verts, *oldverts;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		R_CalcAliasFrameLerp (paliashdr, 0.1);
	else if (currententity->flags & RF_CAMERAMODEL2)
		R_CalcAliasFrameLerp (paliashdr, 0.0);
	else
		R_CalcAliasFrameLerp (paliashdr, 0.5);

	c_alias_polys += paliashdr->num_tris;

	jj = 0;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	oldverts = oldframe->verts;
	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy (tempVertexArray[index_xyz], vertexArray[jj]);

			index2 = verts[index_xyz].lightnormalindex;
			oldindex2 = oldverts[index_xyz].lightnormalindex;

			normalArray[jj][0] = q_byteDirs[oldindex2][0] * backlerp + q_byteDirs[index2][0] * frontlerp;
			normalArray[jj][1] = q_byteDirs[oldindex2][1] * backlerp + q_byteDirs[index2][1] * frontlerp;
			normalArray[jj][2] = q_byteDirs[oldindex2][2] * backlerp + q_byteDirs[index2][2] * frontlerp;

		}
	}

	// setup program
	GL_BindProgram (aliasAmbientProgram);
	
	vec2_t shellParams = { r_newrefdef.time * 0.45, 0.0f };

	qglUniform1i (U_SHELL_PASS, 1);
	qglUniform1i (U_ENV_PASS, 0);
	qglUniform1f (U_COLOR_MUL, r_textureColorScale->value);
	qglUniform2fv(U_SHELL_PARAMS, 1, shellParams);
	qglUniform3fv(U_VIEW_POS, 1, r_origin);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	if (currententity->flags & RF_SHELL_BLUE)
		GL_MBind (GL_TEXTURE0, r_texshell[0]->texnum);
	if (currententity->flags & RF_SHELL_RED)
		GL_MBind (GL_TEXTURE0, r_texshell[1]->texnum);
	if (currententity->flags & RF_SHELL_GREEN)
		GL_MBind (GL_TEXTURE0, r_texshell[2]->texnum);
	if (currententity->flags & RF_SHELL_GOD)
		GL_MBind (GL_TEXTURE0, r_texshell[3]->texnum);
	if (currententity->flags & RF_SHELL_HALF_DAM)
		GL_MBind (GL_TEXTURE0, r_texshell[4]->texnum);
	if (currententity->flags & RF_SHELL_DOUBLE)
		GL_MBind (GL_TEXTURE0, r_texshell[5]->texnum);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);

	qglEnableVertexAttribArray (ATT_NORMAL);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);

//	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, currentmodel->vboId);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, currentmodel->st);

	qglDrawArrays (GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_TEX0);
//	qglBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
}

void GL_DrawAliasFrameLerpLight (dmdl_t *paliashdr) {
	int				i, j, jj = 0;
	int				index_xyz;
	byte			*binormals, *oldbinormals;
	byte			*tangents, *oldtangents;
	dtriangle_t		*tris;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*verts, *oldverts;
	float			backlerp, frontlerp;
	unsigned		offs, offs2;
	vec3_t			maxs;
	image_t			*skin, *skinNormalmap, *rgh;
	int				index2, oldindex2;
	qboolean		inWater;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (currentmodel->noSelfShadow && r_shadows->integer)
		GL_Disable(GL_STENCIL_TEST);

	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	offs = paliashdr->num_xyz;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	oldverts = oldframe->verts;
	offs2 = offs*currententity->oldframe;
	oldbinormals = currentmodel->binormals + offs2;
	oldtangents = currentmodel->tangents + offs2;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	offs2 = offs*currententity->frame;
	binormals = currentmodel->binormals + offs2;
	tangents = currentmodel->tangents + offs2;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);

	// select skin
	if (currententity->skin)
		skin = currententity->skin;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skin = currentmodel->skins[0];
			currententity->skinnum = 0;
		}
		else {
			skin = currentmodel->skins[currententity->skinnum];
			if (!skin) {
				skin = currentmodel->skins[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!skin)
		skin = r_notexture;

//	// select skin
	if (currententity->bump)
		skinNormalmap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skinNormalmap = currentmodel->skins_normal[0];
			currententity->skinnum = 0;
		}
		else {
			skinNormalmap = currentmodel->skins_normal[currententity->skinnum];
			if (!skin) {
				skinNormalmap = currentmodel->skins_normal[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!skinNormalmap)
		skinNormalmap = r_defBump;
	
	rgh = currentmodel->skins_roughness[currententity->skinnum];
	if (!rgh)
		rgh = r_notexture;

	R_CalcAliasFrameLerp(paliashdr, 0);			/// Просто сюда переместили вычисления Lerp...
	
	c_alias_polys += paliashdr->num_tris;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			index2 = verts[index_xyz].lightnormalindex;
			oldindex2 = oldverts[index_xyz].lightnormalindex;

			normalArray[jj][0] = q_byteDirs[oldindex2][0] * backlerp + q_byteDirs[index2][0] * frontlerp;
			normalArray[jj][1] = q_byteDirs[oldindex2][1] * backlerp + q_byteDirs[index2][1] * frontlerp;
			normalArray[jj][2] = q_byteDirs[oldindex2][2] * backlerp + q_byteDirs[index2][2] * frontlerp;

			tangentArray[jj][0] = q_byteDirs[oldtangents[index_xyz]][0] * backlerp + q_byteDirs[tangents[index_xyz]][0] * frontlerp;
			tangentArray[jj][1] = q_byteDirs[oldtangents[index_xyz]][1] * backlerp + q_byteDirs[tangents[index_xyz]][1] * frontlerp;
			tangentArray[jj][2] = q_byteDirs[oldtangents[index_xyz]][2] * backlerp + q_byteDirs[tangents[index_xyz]][2] * frontlerp;

			binormalArray[jj][0] = q_byteDirs[oldbinormals[index_xyz]][0] * backlerp + q_byteDirs[binormals[index_xyz]][0] * frontlerp;
			binormalArray[jj][1] = q_byteDirs[oldbinormals[index_xyz]][1] * backlerp + q_byteDirs[binormals[index_xyz]][1] * frontlerp;
			binormalArray[jj][2] = q_byteDirs[oldbinormals[index_xyz]][2] * backlerp + q_byteDirs[binormals[index_xyz]][2] * frontlerp;

			VectorCopy(tempVertexArray[index_xyz], vertexArray[jj]);

		}
	}

	// setup program
	GL_BindProgram (aliasBumpProgram);

	VectorAdd (currententity->origin, currententity->model->maxs, maxs);
	if (CL_PMpointcontents (maxs) & MASK_WATER)
		inWater = qtrue;
	else
		inWater = qfalse;

	R_UpdateLightAliasUniforms();
	
	if (r_imageAutoBump->integer && skinNormalmap == r_defBump) {
		qglUniform1i(U_USE_AUTOBUMP, 1);
		qglUniform2f(U_AUTOBUMP_PARAMS, r_imageAutoBumpScale->value, r_imageAutoSpecularScale->value);
	}
	else
		qglUniform1i(U_USE_AUTOBUMP, 0);

	if (inWater && currentShadowLight->castCaustics && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		qglUniform1i(U_USE_CAUSTICS, 1);
	else
		qglUniform1i(U_USE_CAUSTICS, 0);

	GL_MBind (GL_TEXTURE0, skinNormalmap->texnum);
	GL_MBind (GL_TEXTURE1, skin->texnum);
	GL_MBind (GL_TEXTURE2, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
	GL_MBindCube (GL_TEXTURE3, r_lightCubeMap[currentShadowLight->filter]->texnum);

	if (rgh == r_notexture)
		qglUniform1i(U_USE_RGH_MAP, 0);
	else {
		qglUniform1i(U_USE_RGH_MAP, 1);
		GL_MBind(GL_TEXTURE4, rgh->texnum);
	}
	
	GL_MBind(GL_TEXTURE5, skinBump->texnum);
	
	qglUniform1i(U_PARAM_INT_1, 0);
	qglUniform1i(U_PARAM_INT_2, 0);
	qglUniform1i(U_PARAM_INT_3, 0);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TANGENT);
	qglEnableVertexAttribArray(ATT_BINORMAL);
	qglEnableVertexAttribArray(ATT_NORMAL);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);
	qglVertexAttribPointer (ATT_TANGENT, 3, GL_FLOAT, qfalse, 0, tangentArray);
	qglVertexAttribPointer (ATT_BINORMAL, 3, GL_FLOAT, qfalse, 0, binormalArray);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, currentmodel->st);

	qglDrawArrays (GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_TANGENT);
	qglDisableVertexAttribArray (ATT_BINORMAL);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_TEX0);
}


/*
=============================================================
ALIAS MODELS
=============================================================
*/

float	shadelight[3];
float	ref_realtime = 0;

void	GL_DrawAliasFrameLerp(dmdl_t *paliashdr, vec3_t color);
void	GL_DrawAliasFrameLerpShell(dmdl_t *paliashdr);

/*
** R_CullAliasModel
*/
qboolean R_CullAliasModel(vec3_t bbox[8], entity_t *e)
{
	int i;
	vec3_t		mins, maxs;
	dmdl_t		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t *pframe, *poldframe;
	vec3_t tmp;

	paliashdr = (dmdl_t *)currentmodel->extraData;

	if ((e->frame >= paliashdr->num_frames) || (e->frame < 0)) {
		Com_Printf("R_CullAliasModel %s: no such frame %d\n",
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ((e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0)) {
		Com_Printf("R_CullAliasModel %s: no such oldframe %d\n",
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = (daliasframe_t *)((byte *)paliashdr +
		paliashdr->ofs_frames +
		e->frame * paliashdr->framesize);

	poldframe = (daliasframe_t *)((byte *)paliashdr +
		paliashdr->ofs_frames +
		e->oldframe * paliashdr->framesize);

	if (pframe == poldframe) {
		for (i = 0; i < 3; i++) {
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i] * 255;
		}
	}
	else {
		for (i = 0; i < 3; i++) {
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i] * 255;

			oldmins[i] = poldframe->translate[i];
			oldmaxs[i] = oldmins[i] + poldframe->scale[i] * 255;

			if (thismins[i] < oldmins[i])
				mins[i] = thismins[i];
			else
				mins[i] = oldmins[i];

			if (thismaxs[i] > oldmaxs[i])
				maxs[i] = thismaxs[i];
			else
				maxs[i] = oldmaxs[i];
		}
	}

	//=================

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

	//=========================

	{
		int p, f, aggregatemask = ~0;

		for (p = 0; p < 8; p++) {
			int mask = 0;

			for (f = 0; f < 6; f++) {
				float dp = DotProduct(frustum[f].normal, bbox[p]);

				if ((dp - frustum[f].dist) < 0) {
					mask |= (1 << f);
				}
			}

			aggregatemask &= mask;
		}

		if (aggregatemask) {
			return qtrue;
		}

		return qfalse;
	}
}

void SetModelsLight()
{
	int i;
	float mid;

	if (currententity->flags & (RF_FULLBRIGHT | RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE
		| RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
	{
		for (i = 0; i < 3; i++)
			shadelight[i] = 1.0;
	}
	else
		R_LightPoint(currententity->origin, shadelight);

	if (currententity->flags & RF_MINLIGHT) {
		for (i = 0; i < 3; i++)
			if (shadelight[i] > 0.01)
				break;
		if (i == 3) {
			shadelight[0] = 0.005;
			shadelight[1] = 0.005;
			shadelight[2] = 0.005;
		}
	}

	// player lighting hack for communication back to server
	// big hack!
	if (currententity->flags & RF_WEAPONMODEL) {
		mid = max(max(shadelight[0], shadelight[1]), shadelight[2]);

		if (mid <= 0.1)
			mid = 0.15;

		mid *= 2.0;
		r_lightLevel->value = 150 * mid;
	}

	// =================
	// PGM	ir goggles color override
	if (r_newrefdef.rdflags & RDF_IRGOGGLES) {
		shadelight[0] = 1.0;
		shadelight[1] = 1.0;
		shadelight[2] = 1.0;
	}
	// PGM
	// =================
}

/*
=================
R_DrawAliasModel
=================
*/

void R_DrawAliasModel(entity_t *e)
{
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];


	if (!(e->flags & RF_WEAPONMODEL)) {
		if (R_CullAliasModel(bbox, e))
			return;
	}

	if (e->flags & RF_WEAPONMODEL) {
		if (r_leftHand->integer == 2)
			return;
	}

	paliashdr = (dmdl_t *)currentmodel->extraData;

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	SetModelsLight();

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

	R_SetupEntityMatrix(e);

	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
		GL_DrawAliasFrameLerpShell(paliashdr);
	else
		GL_DrawAliasFrameLerp(paliashdr, shadelight);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

}


void R_DrawAliasModelLightPass(qboolean weapon_model)
{
	dmdl_t	*paliashdr;
	vec3_t	bbox[8];
	vec3_t	oldLight, oldView, tmp;

	if (!r_drawEntities->integer)
		return;

	if (currententity->flags & RF_DISTORT)
		return;

	if (currententity->flags & RF_TRANSLUCENT)
		return;

	if (!(currententity->flags & RF_WEAPONMODEL)) {
		if (R_CullAliasModel(bbox, currententity))
			return;
	}

	if (currententity->flags & RF_WEAPONMODEL) {
		if (!weapon_model || r_leftHand->integer == 2)
			return;
	}

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) {
		if (!currentShadowLight->isNoWorldModel)
			return;
		goto visible;
	}

	if (!R_AliasInLightBound())
		return;

visible:

	paliashdr = (dmdl_t *)currentmodel->extraData;


	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

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

	VectorCopy(currentShadowLight->origin, oldLight);
	VectorCopy(r_origin, oldView);

	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, r_origin);

	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);
	
	GL_PolygonOffset(-1.0, -1.0);

	GL_DrawAliasFrameLerpLight(paliashdr);

	VectorCopy(oldLight, currentShadowLight->origin);
	VectorCopy(oldView, r_origin);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);
}