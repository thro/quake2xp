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
// GL_RSURF.C: surface-related refresh code
//#include <assert.h>

#include "r_local.h"

static vec3_t modelorg;			// relative to viewpoint

vec4_t	color_black = { 0.f, 0.f, 0.f, 0.f };

vec3_t	wVertexArray[MAX_BATCH_SURFS];
float	wTexArray[MAX_BATCH_SURFS][2];
float	wLMArray[MAX_BATCH_SURFS][2];
vec4_t	wColorArray[MAX_BATCH_SURFS];

float   wTmu0Array[MAX_BATCH_SURFS][2];
float   wTmu1Array[MAX_BATCH_SURFS][2];
float   wTmu2Array[MAX_BATCH_SURFS][2];

uint	indexArray[MAX_MAP_VERTS * 3];
vec3_t	nTexArray[MAX_BATCH_SURFS];
vec3_t	tTexArray[MAX_BATCH_SURFS];
vec3_t	bTexArray[MAX_BATCH_SURFS];

/*
=============================================================

	BRUSH MODELS

=============================================================
*/
/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
image_t *R_TextureAnimation(mtexInfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->image;

	c = currententity->frame % tex->numFrames;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->image;
}


image_t *R_TextureAnimationFx(mtexInfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->addTexture;

	c = currententity->frame % tex->numFrames;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->addTexture;
}

image_t *R_TextureAnimationNormal(mtexInfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->normalmap;

	c = currententity->frame % tex->numFrames;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->normalmap;
}

image_t *R_TextureAnimationEnv(mtexInfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->envTexture;

	c = currententity->frame % tex->numFrames;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->envTexture;
}

image_t *R_TextureAnimationRgh(mtexInfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->rghMap;

	c = currententity->frame % tex->numFrames;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->rghMap;
}

/*
================
DrawGLPoly

================
*/
void DrawGLPoly (msurface_t * fa, qboolean scrolling) {
	int i;
	float *v;
	float alpha, scroll;
	glpoly_t *p;
	int nv = fa->polys->numVerts;
	uint numIndices = 0, numVertixes = 0;

	if (fa->texInfo->flags & SURF_TRANS33)
		alpha = 0.33f;
	else
		alpha = 0.66f;		

	qglUniform1f(U_REFR_ALPHA, alpha);

		if (scrolling)
			GL_MBind(GL_TEXTURE0, r_DSTTex->texnum);
		else
			GL_MBind(GL_TEXTURE0, fa->texInfo->normalmap->texnum);

		GL_MBind(GL_TEXTURE1, fa->texInfo->image->texnum);
		GL_MBindRect(GL_TEXTURE2, ScreenMap->texnum);
		GL_MBindRect(GL_TEXTURE3, depthMap->texnum);

	if (scrolling)
		scroll = (r_newrefdef.time * 0.15f) - (int)(r_newrefdef.time * 0.15f);
	 else
		scroll = 0;

	p = fa->polys;
	v = p->verts[0];
	
	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = numVertixes;
		indexArray[numIndices++] = numVertixes + i + 1;
		indexArray[numIndices++] = numVertixes + i + 2;
	}

	for (i = 0; i < p->numVerts; i++, v += VERTEXSIZE) {
		
		VectorCopy(v, wVertexArray[i]);
			
		wTexArray[i][0] = v[3] - scroll;
		wTexArray[i][1] = v[4];
	}

	qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
	c_brush_polys += numIndices / 3;

	
}


void R_DrawChainsRA (qboolean bmodel) {
	
	msurface_t	*s;
	float		colorScale = max(r_lightmapScale->value, 0.33), ambient;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	GL_BindProgram(waterProgram);

	GL_MBind(GL_TEXTURE1, r_waterNormals[((int)(r_newrefdef.time * 15)) & (MAX_WATER_NORMALS - 1)]->texnum);
	GL_MBindRect(GL_TEXTURE2, ScreenMap->texnum);
	GL_MBindRect(GL_TEXTURE3, depthMap->texnum);

	ambient = max(r_lightmapScale->value, 0.15f); // sRGB clamp fix

	qglUniform1f(U_WATER_DEFORM_MUL, 1.0);
	qglUniform1f(U_WATHER_THICKNESS, 150.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform1f(U_COLOR_MUL, 1.0);
	qglUniform1f(U_AMBIENT_LEVEL, ambient);

	if (!bmodel)
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	else
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

	if (r_newrefdef.rdflags & RDF_UNDERWATER)
		qglUniform1i(U_WATER_MIRROR, 0);
	else
		qglUniform1i(U_WATER_MIRROR, 1);

	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, qfalse, (const float *)r_newrefdef.projectionMatrix);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_NORMAL);
	qglEnableVertexAttribArray(ATT_TANGENT);
	qglEnableVertexAttribArray(ATT_BINORMAL);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, wVertexArray);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, wTexArray);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, wColorArray);
	qglVertexAttribPointer(ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, nTexArray);
	qglVertexAttribPointer(ATT_TANGENT, 3, GL_FLOAT, qfalse, 0, tTexArray);
	qglVertexAttribPointer(ATT_BINORMAL, 3, GL_FLOAT, qfalse, 0, bTexArray);

	for (s = r_reflective_surfaces; s; s = s->texturechain) {

		if (s->flags & MSURF_LAVA)
			continue;

		R_DrawWaterPolygons(s, bmodel);
	}


	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_NORMAL);
	qglDisableVertexAttribArray(ATT_TANGENT);
	qglDisableVertexAttribArray(ATT_BINORMAL);
	qglDisableVertexAttribArray(ATT_COLOR);
//------------------------------

	r_reflective_surfaces = NULL;

	R_CaptureColorBuffer();
	R_Capture2dColorBuffer();

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, wVertexArray);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, wTexArray);

	// setup program
	GL_BindProgram(refractProgram);

	qglUniform1f(U_REFR_DEFORM_MUL, 1.0);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, qfalse, (const float *)r_newrefdef.projectionMatrix);

	qglUniform1f(U_REFR_THICKNESS0, 150.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform1f(U_COLOR_MUL, colorScale);
	qglUniform1i(U_REFR_ALPHA_MASK, 0);
	
	float blurScale= 18.88 * ((float)vid.width / 1024.0);
	qglUniform1i(U_PARAM_FLOAT_0, blurScale);

	for (s = r_alpha_surfaces; s; s = s->texturechain) {
		
		if (s->flags & MSURF_LAVA)
			continue;

		if (s->texInfo->flags & SURF_FLOWING)
			DrawGLPoly(s, qtrue);
		else
			DrawGLPoly(s, qfalse);
	}

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);

	r_alpha_surfaces = NULL;

}


/*
===============
SORT AND BATCH 
BSP SURFACES

===============
*/

qboolean R_FillAmbientBatch (msurface_t *surf, qboolean newBatch, unsigned *indeces, qboolean bmodel) {
	unsigned	numIndices;
	int			i, nv = surf->numEdges;
	float		scroll = 0.0, scale[2];

	numIndices	= *indeces;

	if ((nv-2) * 3 >= MAX_IDX)
		return qfalse;	// force the start new batch

	if (newBatch) {
		image_t	*image, *fx, *normal, *rgh;

		image	= R_TextureAnimation(surf->texInfo);
		fx		= R_TextureAnimationFx(surf->texInfo);
		normal	= R_TextureAnimationNormal(surf->texInfo);
		rgh		= R_TextureAnimationRgh(surf->texInfo);

		qglUniform1f(U_SPECULAR_SCALE, image->specularScale ? image->specularScale : r_ambientSpecularScale->value);
		
		if (surf->flags & MSURF_ENVMAP)
			qglUniform1i(U_PARAM_INT_0, 1);
		else
			qglUniform1i(U_PARAM_INT_0, 0);

		if (!r_skipStaticLights->integer) 
		{
			if (surf->flags & MSURF_LAVA)
				qglUniform1f(U_AMBIENT_LEVEL, 0.5);
			else
				qglUniform1f(U_AMBIENT_LEVEL, r_lightmapScale->value);
		}

		if (!image->parallaxScale){
			scale[0] = r_reliefScale->value / image->width;
			scale[1] = r_reliefScale->value / image->height;
		}
		else
		{
			scale[0] = image->parallaxScale / image->width;
			scale[1] = image->parallaxScale / image->height;
		}
			qglUniform4f(U_PARALLAX_PARAMS, scale[0], scale[1], image->upload_width, image->upload_height);
	
		if (surf->flags & MSURF_LAVA)
			qglUniform1i(U_LAVA_PASS, 1);
		else
			qglUniform1i(U_LAVA_PASS, 0);

		GL_MBind(GL_TEXTURE0, image->texnum);
		GL_MBind(GL_TEXTURE2, fx->texnum);
		GL_MBind(GL_TEXTURE3, normal->texnum);
		
		if (surf->texInfo->flags & SURF_FLOWING)
		{
			scroll = -64 * ((r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0));
			if (scroll == 0.0)
				scroll = -64.0;
		}

		if(!bmodel)
			qglUniform1f(U_SCROLL, scroll);
		else
			qglUniform1f(U_SCROLL, 0.0);
	}

	// create indexes
	if (numIndices == 0xffffffff)
		numIndices = 0;

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = surf->baseIndex;
		indexArray[numIndices++] = surf->baseIndex + i + 1;
		indexArray[numIndices++] = surf->baseIndex + i + 2;
		}

	*indeces	= numIndices;

	return qtrue;	
}

int SurfSort( const msurface_t **a, const msurface_t **b )
{
	return	(( (*a)->lightmaptexturenum<<26) +( (*a)->texInfo->image->texnum<<13)) - 
			(((*b)->lightmaptexturenum<<26)+((*b)->texInfo->image->texnum<<13));
}

vec3_t		BmodelViewOrg;
int			num_scene_surfaces;
msurface_t	*scene_surfaces[MAX_MAP_FACES];

static void GL_DrawLightmappedPoly(qboolean bmodel)
{
	msurface_t	*s;
	int			i;
	qboolean	newBatch;
	unsigned	oldTex		= 0xffffffff;
	unsigned	oldFlag		= 0xffffffff;
	unsigned	numIndices  = 0xffffffff;

	// setup program
	GL_BindProgram(ambientWorldProgram);

	qglUniform1f(U_COLOR_MUL, r_textureColorScale->value);
	qglUniform3fv(U_VIEW_POS, 1, bmodel ? BmodelViewOrg : r_origin);
	qglUniform1i(U_PARALLAX_TYPE, clamp(r_reliefMapping->integer, 0, 1));
	qglUniform1f(U_AMBIENT_LEVEL, r_lightmapScale->value);

	qglUniform1i(U_LM_TYPE, (r_worldmodel->useXPLM && r_useRadiosityBump->integer) ? 1 : 0);

	if (!bmodel){
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	}
	else{
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);
	}

	if (r_ssao->integer && !(r_newrefdef.rdflags & RDF_IRGOGGLES) && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
		GL_MBindRect(GL_TEXTURE4, fboColor[fboColorIndex]->texnum);
		qglUniform1i(U_USE_SSAO, 1);
	}
	else
		qglUniform1i(U_USE_SSAO, 0);

	qsort(scene_surfaces, num_scene_surfaces, sizeof(msurface_t*), (int(*)(const void *, const void *))SurfSort);

	for (i = 0; i < num_scene_surfaces; i++){
		s = scene_surfaces[i];

		// update lightmaps
		if (gl_state.currenttextures[1] != gl_state.lightmap_textures + s->lightmaptexturenum)
		{
			GL_MBind(GL_TEXTURE1, gl_state.lightmap_textures + s->lightmaptexturenum);

			if (r_worldmodel->useXPLM && r_useRadiosityBump->integer) {
				GL_MBind(GL_TEXTURE5, gl_state.lightmap_textures + s->lightmaptexturenum + MAX_LIGHTMAPS);
				GL_MBind(GL_TEXTURE6, gl_state.lightmap_textures + s->lightmaptexturenum + MAX_LIGHTMAPS * 2);
			}
			
			if (numIndices != 0xFFFFFFFF) {
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				numIndices = 0xFFFFFFFF;
			}
		}
		
		// flush batch (new texture)
		if (s->texInfo->image->texnum != oldTex)
		{
			if (numIndices != 0xFFFFFFFF){
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0xFFFFFFFF;
			}

			oldTex = s->texInfo->image->texnum;
			newBatch = qtrue;
		}
	else
		newBatch = qfalse;
	
	// fill new batch
		if (!R_FillAmbientBatch(s, newBatch, &numIndices, bmodel))
		{
			if (numIndices != 0xFFFFFFFF){
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
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


int	r_lightTimestamp;

qboolean R_FillLightBatch(msurface_t *surf, qboolean newBatch, unsigned *indeces, qboolean bmodel, qboolean caustics)
{
	unsigned	numIndices;
	int			i, nv = surf->numEdges;
	float		scroll = 0.0, scale[2];

	numIndices = *indeces;

	if ((nv - 2) * 3 >= MAX_IDX)
		return qfalse;	// force the start new batch

	if (newBatch)
	{
		image_t		*image, *normalMap, *rghMap;
		image		= R_TextureAnimation		(surf->texInfo);
		normalMap	= R_TextureAnimationNormal	(surf->texInfo);
		rghMap		= R_TextureAnimationRgh		(surf->texInfo);

		qglUniform1f(U_SPECULAR_SCALE,	image->specularScale ?	image->specularScale	: r_specularScale->value);
		qglUniform1f(U_RGH_SCALE, image->rghScale ?		image->rghScale			: 0.666); // HAIL SATAN!

		if (rghMap == r_notexture) {
			qglUniform1i(U_USE_RGH_MAP, 0);
		}
		else {
			qglUniform1i(U_USE_RGH_MAP, 1);
			GL_MBind(GL_TEXTURE4, rghMap->texnum);
		//	glUniformHandleui64ARB(104, rghMap->handle);
			GL_CheckError("glUniformHandleui64ARB(104, rghMap->handle);", 553, "tst");
		}

		if (bmodel){
			if (caustics && currentShadowLight->castCaustics){
				qglUniform1i(U_USE_CAUSTICS, 1);
				GL_MBind(GL_TEXTURE3, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
			//	glUniformHandleui64ARB(103, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->handle);
			}
			else
				qglUniform1i(U_USE_CAUSTICS, 0);
		}
		else{
			if ((surf->flags & MSURF_WATER) && currentShadowLight->castCaustics) {
				qglUniform1i(U_USE_CAUSTICS, 1);
				GL_MBind(GL_TEXTURE3, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
			//	glUniformHandleui64ARB(103, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->handle);
			}
			else
				qglUniform1i(U_USE_CAUSTICS, 0);
		}

		if (!image->parallaxScale){
			scale[0] = r_reliefScale->value / image->width;
			scale[1] = r_reliefScale->value / image->height;
		}
	else
		{
			scale[0] = image->parallaxScale / image->width;
			scale[1] = image->parallaxScale / image->height;
		}

		qglUniform4f(U_PARALLAX_PARAMS, scale[0], scale[1], image->upload_width, image->upload_height);


		if (surf->flags & MSURF_SSS)
			qglUniform1i(U_PARAM_INT_0, 1);
		else
			qglUniform1i(U_PARAM_INT_0, 0);

		GL_MBind		(GL_TEXTURE0, image->texnum);
	//	glUniformHandleui64ARB(100, image->handle);
		GL_MBind		(GL_TEXTURE1, normalMap->texnum);
	//	glUniformHandleui64ARB(101, normalMap->handle);
		GL_MBindCube	(GL_TEXTURE2, r_lightCubeMap[currentShadowLight->filter]->texnum);
	//	glUniformHandleui64ARB(102, r_lightCubeMap[currentShadowLight->filter]->handle);

		if (r_imageAutoBump->integer && normalMap == r_defBump) {
			qglUniform1i(U_USE_AUTOBUMP, 1);
			qglUniform2f(U_AUTOBUMP_PARAMS, r_imageAutoBumpScale->value, r_imageAutoSpecularScale->value);
		}
		else
			qglUniform1i(U_USE_AUTOBUMP, 0);

		if (surf->texInfo->flags & SURF_FLOWING){

			scroll = -64 * ((r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0));
			if (scroll == 0.0)
				scroll = -64.0;
		}		
		
		if(!bmodel)
			qglUniform1f(U_SCROLL, scroll);
		else
			qglUniform1f(U_SCROLL, 0.0);
	}

	// create indexes
	if (numIndices == 0xffffffff)
		numIndices = 0;

	for (i = 0; i < nv - 2; i++)
	{
		indexArray[numIndices++] = surf->baseIndex;
		indexArray[numIndices++] = surf->baseIndex + i + 1;
		indexArray[numIndices++] = surf->baseIndex + i + 2;
	}

	*indeces = numIndices;

	
	return qtrue;
}

 int lightSurfSort( const msurface_t **a, const msurface_t **b )
{
	return	(((*a)->texInfo->image->texnum) + ((*a)->flags)) -
			(((*b)->texInfo->image->texnum) + ((*b)->flags));
}


 void R_UpdateLightUniforms(qboolean bModel)
 {
	 mat4_t	entAttenMatrix, entSpotMatrix;

	qglUniform1f(U_COLOR_MUL, r_textureColorScale->value);
	qglUniform1i(U_AMBIENT_LIGHT, (int)currentShadowLight->isAmbient);
	qglUniform3fv(U_LIGHT_POS, 1, currentShadowLight->origin);
	qglUniform4f(U_COLOR, currentShadowLight->color[0], currentShadowLight->color[1], currentShadowLight->color[2], 1.0);
	qglUniform1i(U_USE_FOG, (int)currentShadowLight->isFog);
	qglUniform1f(U_FOG_DENSITY, currentShadowLight->fogDensity);
	qglUniform1i(U_PARALLAX_TYPE, clamp(r_reliefMapping->integer, 0, 1));
	qglUniform1f(U_CAUSTICS_SCALE, r_causticIntens->value);
	qglUniform1f(U_PARAM_FLOAT_0, r_reliefMappingSelfShadowOffset->value);
	
	 if (bModel)
		qglUniform3fv(U_VIEW_POS, 1, BmodelViewOrg);
	 else
		 qglUniform3fv(U_VIEW_POS, 1, r_origin);

	 if (!bModel){
		 qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
		 qglUniformMatrix4fv(U_ATTEN_MATRIX, 1, qfalse, (const float *)currentShadowLight->attenMatrix);
		 qglUniformMatrix4fv(U_SPOT_MATRIX, 1, qfalse, (const float *)currentShadowLight->spotMatrix);
	 }
	 else{
		 qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)currententity->orMatrix);

		 Mat4_TransposeMultiply(currententity->matrix, currentShadowLight->attenMatrix, entAttenMatrix);
		 qglUniformMatrix4fv(U_ATTEN_MATRIX, 1, qfalse, (const float *)entAttenMatrix);

		 Mat4_TransposeMultiply(currententity->matrix, currentShadowLight->spotMatrix, entSpotMatrix);
		 qglUniformMatrix4fv(U_SPOT_MATRIX, 1, qfalse, (const float *)entSpotMatrix);
	 }
	 
	 qglUniform3f(U_SPOT_PARAMS, currentShadowLight->hotSpot, 1.f / (1.f - currentShadowLight->hotSpot), currentShadowLight->coneExp);

	 if (currentShadowLight->isCone) 
		 qglUniform1i(U_SPOT_LIGHT, 1);
	 else
		 qglUniform1i(U_SPOT_LIGHT, 0);

	 if (r_reliefMappingSelfShadow->integer)
		 qglUniform1i(U_PARAM_INT_1, 1);
	 else
		 qglUniform1i(U_PARAM_INT_1, 0);

	 if (r_useBlinnPhongLighting->integer)
		 qglUniform1i(U_PARAM_INT_2, 1);
	 else
		 qglUniform1i(U_PARAM_INT_2, 0);

	 R_CalcCubeMapMatrix(bModel);
	 qglUniformMatrix4fv(U_CUBE_MATRIX, 1, qfalse, (const float *)currentShadowLight->cubeMapMatrix);

 }

 msurface_t		*interaction[MAX_MAP_FACES];
 int			numInteractionSurfs;

static void GL_DrawDynamicLightPass(qboolean bmodel, qboolean caustics)
{
	msurface_t	*s;
	int			i;
	glpoly_t	*poly;
	qboolean	newBatch, oldCaust;
	unsigned	oldTex		= 0xffffffff;
	unsigned	oldFlag		= 0xffffffff;
	unsigned	numIndices	= 0xffffffff;

	R_UpdateLightUniforms(bmodel);

	qsort(interaction, numInteractionSurfs, sizeof(msurface_t*), (int (*)(const void *, const void *))lightSurfSort);

	for (i = 0; i < numInteractionSurfs; i++){
		s = interaction[i];
		poly = s->polys;

		if ((poly->lightTimestamp != r_lightTimestamp) || (s->visframe != r_framecount))
			continue;

		// flush batch (new texture or flag)
		if (s->texInfo->image->texnum != oldTex || s->flags != oldFlag || caustics != oldCaust)
		{
			if (numIndices != 0xffffffff){
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0xffffffff;
			}
			oldTex = s->texInfo->image->texnum;
			oldFlag = s->flags;
			oldCaust = caustics;
			newBatch = qtrue;
		}
		else
			newBatch = qfalse;

	// fill new batch
		if (!R_FillLightBatch(s, newBatch, &numIndices, bmodel, caustics))
		{
			if (numIndices != 0xffffffff){
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0xffffffff;
			}
		}
	}
	// draw the rest
	if (numIndices != 0xffffffff) {
		qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brush_polys += numIndices / 3;
	}
}

static void GL_DrawStaticLightPass()
{
	msurface_t	*s;
	int			i;
	qboolean	newBatch;
	unsigned	oldTex = 0xffffffff;
	unsigned	oldFlag = 0xffffffff;
	unsigned	numIndices = 0xffffffff;

	R_UpdateLightUniforms(qfalse);

	for (i = 0; i < currentShadowLight->numInteractionSurfs; i++) {
		
		s = currentShadowLight->interaction[i];

		if ((s->visframe != r_framecount) || (s->ent))
			continue;

		// flush batch (new texture or flag)
		if (s->texInfo->image->texnum != oldTex || s->flags != oldFlag)
		{
			if (numIndices != 0xffffffff) {
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0xffffffff;
			}

			oldTex = s->texInfo->image->texnum;
			oldFlag = s->flags;
			newBatch = qtrue;
		}
		else
			newBatch = qfalse;

		// fill new batch
		if (!R_FillLightBatch(s, newBatch, &numIndices, qfalse, qfalse))
		{
			if (numIndices != 0xffffffff) {
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0xffffffff;
			}
		}
	}
	// draw the rest
	if (numIndices != 0xffffffff) {
		qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brush_polys += numIndices / 3;
	}
}


qboolean SurfInFrustum (msurface_t *s) {
	if (s->polys)
		return !R_CullBox(s->mins, s->maxs);

	return qtrue;
}

/*
================
R_RecursiveWorldNode

================
*/
static void R_RecursiveWorldNode (mnode_t * node) {
	int c, side, sidebit;
	cplane_t *plane;
	msurface_t *surf, **mark;
	mleaf_t *pleaf;
	float dot;

	if (node->contents == CONTENTS_SOLID)
		return;					// solid

	if (node->visframe != r_visframecount)
		return;

	if (R_CullBox(node->minmaxs, node->minmaxs + 3))
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
				if (SurfInFrustum(*mark))
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
		dot = DotProduct(modelorg, plane->normal) - plane->dist;
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
	R_RecursiveWorldNode(node->children[side]);

	// draw stuff
	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c; c--, surf++) {
		if (surf->visframe != r_framecount)
			continue;
		if ((surf->flags & MSURF_PLANEBACK) != sidebit)
			continue;			// wrong side

		if (surf->texInfo->flags & SURF_SKY)	// just adds to visible sky bounds
			R_AddSkySurface(surf);
		else if (surf->texInfo->flags & SURF_NODRAW)
			continue;
		else if (surf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66) && !(surf->flags & MSURF_LAVA) && !(surf->flags & MSURF_DRAWTURB) ) {
			// add to the translucent chain
			surf->texturechain = r_alpha_surfaces;
			r_alpha_surfaces = surf;
		}
		else {
			if ((surf->flags & MSURF_DRAWTURB) && !(surf->flags & MSURF_LAVA)) {
				// add to the reflective chain
				surf->texturechain = r_reflective_surfaces;
				r_reflective_surfaces = surf;
			}
			else {
				// add to the ambient batch
				scene_surfaces[num_scene_surfaces++] = surf;
			}
		}
	}

	// recurse down the back side
	R_RecursiveWorldNode(node->children[!side]);
}

qboolean R_MarkLightSurf (msurface_t *surf, qboolean world, worldShadowLight_t *light) {
	cplane_t	*plane;
	float		dist;
	glpoly_t	*poly;


	if (surf->flags & MSURF_LAVA)
		goto hack;

	if ((surf->texInfo->flags & (SURF_TRANS33|SURF_TRANS66|SURF_SKY|SURF_WARP|SURF_NODRAW)) || (surf->flags & MSURF_DRAWTURB))
		return qfalse;
hack:

	plane = surf->plane;
	poly = surf->polys;
	
	if (poly->lightTimestamp == r_lightTimestamp)
		return qfalse;

	switch (plane->type)
	{
	case PLANE_X:
		dist = light->origin[0] - plane->dist;
		break;
	case PLANE_Y:
		dist = light->origin[1] - plane->dist;
		break;
	case PLANE_Z:
		dist = light->origin[2] - plane->dist;
		break;
	default:
		dist = DotProduct (light->origin, plane->normal) - plane->dist;
		break;
	}
	
	if (light->isFog && !light->isShadow)
		goto next;

		//the normals are flipped when surf_planeback is 1
		if (((surf->flags & MSURF_PLANEBACK) && (dist > 0)) ||
			(!(surf->flags & MSURF_PLANEBACK) && (dist < 0)))
			return qfalse;
next:

	if (fabsf(dist) > light->maxRad)
		return qfalse;

	if(world)
	{
		float	lbbox[6], pbbox[6];

		lbbox[0] = light->origin[0] - light->radius[0];
		lbbox[1] = light->origin[1] - light->radius[1];
		lbbox[2] = light->origin[2] - light->radius[2];
		lbbox[3] = light->origin[0] + light->radius[0];
		lbbox[4] = light->origin[1] + light->radius[1];
		lbbox[5] = light->origin[2] + light->radius[2];

		// surface bounding box
		pbbox[0] = surf->mins[0];
		pbbox[1] = surf->mins[1];
		pbbox[2] = surf->mins[2];
		pbbox[3] = surf->maxs[0];
		pbbox[4] = surf->maxs[1];
		pbbox[5] = surf->maxs[2];

		if (light->_cone && R_CullConeLight(&pbbox[0], &pbbox[3], light->frust))
			return qfalse;

		if(!BoundsIntersect(&lbbox[0], &lbbox[3], &pbbox[0], &pbbox[3]))
			return qfalse;
	}

	poly->lightTimestamp = r_lightTimestamp;

	return qtrue;
}

void R_MarkLightCasting (mnode_t *node, qboolean precalc, worldShadowLight_t *light)
{
	cplane_t			*plane;
	float				dist;
	msurface_t			**surf;
	mleaf_t				*leaf;
	int					c, cluster;

	if (light->isNoWorldModel)
		return;

	if (node->contents != -1)
	{
		//we are in a leaf
		leaf = (mleaf_t *)node;
		cluster = leaf->cluster;

		if (!(light->vis[cluster>>3] & (1<<(cluster&7))))
			return;

		surf = leaf->firstmarksurface;

		for (c = 0; c < leaf->numMarkSurfaces; c++, surf++){

			if (R_MarkLightSurf((*surf), qtrue, light)) {
				if(!precalc)
					interaction[numInteractionSurfs++] = (*surf);
				else
					light->interaction[light->numInteractionSurfs++] = (*surf);
			}
		}
		return;
	}

	plane = node->plane;
	dist = DotProduct (light->origin, plane->normal) - plane->dist;

	if (dist > light->maxRad)
	{
		R_MarkLightCasting (node->children[0], precalc, light);
		return;
	}
	if (dist < -light->maxRad)
	{
		R_MarkLightCasting (node->children[1], precalc, light);
		return;
	}

	R_MarkLightCasting (node->children[0], precalc, light);
	R_MarkLightCasting (node->children[1], precalc, light);
}

void R_DrawLightWorld(void)
{

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);

	GL_PolygonOffset(-1.0, -1.0);

	GL_BindProgram(lightWorldProgram);

	glBindVertexArray(vao.bsp_l);

	if (!currentShadowLight->isStatic) {
		r_lightTimestamp++;
		numInteractionSurfs = 0;
		R_MarkLightCasting(r_worldmodel->nodes, qfalse, currentShadowLight);
		if (numInteractionSurfs > 0)
			GL_DrawDynamicLightPass(qfalse, qfalse);
	}
	else
		GL_DrawStaticLightPass();

	glBindVertexArray(0);
}


/*
=============
R_DrawBSP

=============
*/
void R_DrawBSP (void) {
	entity_t ent;

	if (!r_drawWorld->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	currentmodel = r_worldmodel;

	VectorCopy(r_newrefdef.vieworg, modelorg);

	// auto cycle the world frame for texture animation
	memset(&ent, 0, sizeof(ent));
	ent.frame = (int) (r_newrefdef.time * 2);
	Mat3_Identity(ent.axis);
	currententity = &ent;

	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	memset(gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
		
	R_ClearSkyBox();	

	glBindVertexArray(vao.bsp_a);

	num_scene_surfaces = 0;
	R_RecursiveWorldNode(r_worldmodel->nodes);
	GL_DrawLightmappedPoly(qfalse);

	glBindVertexArray(0);

	R_DrawSkyBox(qtrue);
}

/*
=================
R_DrawInlineBModel

=================
*/
extern qboolean bmodelcaust = qfalse;

void R_DrawBrushModel();

static void R_DrawInlineBModel (void) {
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;

	psurf = &currentmodel->surfaces[currentmodel->firstModelSurface];

	//
	// draw texture
	//

	for (i = 0; i < currentmodel->numModelSurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;

		if (pplane->type < 3)
			dot = modelorg[pplane->type] - pplane->dist;
		else
			dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & MSURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & MSURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {

			if (psurf->visframe == r_framecount)	// reckless fix
				continue;

			/*===============================
			berserker - flares for brushmodels
			=================================*/
			psurf->visframe = r_framecount;
			psurf->ent = currententity;
			// ================================

			if (psurf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) 
				continue;
			
			if (!(psurf->texInfo->flags & MSURF_DRAWTURB))
				scene_surfaces[num_scene_surfaces++] = psurf;
		}
	}

}

static void R_DrawInlineBModel2 (void) {
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;
		
	psurf = &currentmodel->surfaces[currentmodel->firstModelSurface];

	//
	// draw texture
	//

	for (i = 0; i < currentmodel->numModelSurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & MSURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) || (!(psurf->flags & MSURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {
			
			if (psurf->visframe == r_framecount) // reckless fix
				continue;

			if (psurf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66))
				continue;

			if (psurf->flags & MSURF_DRAWTURB)
				R_DrawWaterPolygons(psurf, qtrue);
		}
	}
}

static void R_DrawInlineBModel3 (void) {
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;

	psurf = &currentmodel->surfaces[currentmodel->firstModelSurface];

	//
	// draw texture
	//

	for (i = 0; i < currentmodel->numModelSurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & MSURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) || (!(psurf->flags & MSURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {
			if (psurf->visframe != r_framecount)
				continue;

			if (psurf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {
				psurf->texturechain = r_alpha_surfaces;
				r_alpha_surfaces = psurf;
			}
			else if (psurf->texInfo->flags & SURF_WARP) {
				psurf->texturechain = r_reflective_surfaces;
				r_reflective_surfaces = psurf;
			}
		}
	}
}

/*
=================
R_DrawBrushModel

=================
*/
int CL_PMpointcontents(vec3_t point);

void R_DrawBrushModel (void) {
	vec3_t		mins, maxs, tmp;
	int			i;
	qboolean	rotated;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	if (currentmodel->numModelSurfaces == 0)
		return;
	
	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = qtrue;

		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		rotated = qfalse;
		VectorAdd(currententity->origin, currentmodel->mins, mins);
		VectorAdd(currententity->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox(mins, maxs))
		return;

	memset(gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));


	VectorSubtract(r_newrefdef.vieworg, currententity->origin, modelorg);

	if (rotated) {
		vec3_t temp;
		vec3_t forward, right, up;

		VectorCopy(modelorg, temp);
		AngleVectors(currententity->angles, forward, right, up);
		modelorg[0] = DotProduct(temp, forward);
		modelorg[1] = -DotProduct(temp, right);
		modelorg[2] = DotProduct(temp, up);
	}

	R_SetupEntityMatrix(currententity);

	//Put camera into model space view angle for bmodels parallax
	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, BmodelViewOrg);
	GL_DepthMask(0);
	R_DrawInlineBModel2();

	glBindVertexArray(vao.bsp_a);

	num_scene_surfaces = 0;
	R_DrawInlineBModel();
	GL_DrawLightmappedPoly(qtrue);
	
	glBindVertexArray(0);

	GL_DepthMask(1);
}

/*
=================
R_DrawBrushModelRA

=================
*/
void R_DrawBrushModelRA (void) {
	vec3_t		mins, maxs, tmp;
	int			i;
	qboolean	rotated;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	if (currentmodel->numModelSurfaces == 0)
		return;
	
	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = qtrue;

		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		rotated = qfalse;
		VectorAdd(currententity->origin, currentmodel->mins, mins);
		VectorAdd(currententity->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox(mins, maxs))
		return;

	VectorSubtract(r_newrefdef.vieworg, currententity->origin, modelorg);

	if (rotated) {
		vec3_t temp;
		vec3_t forward, right, up;

		VectorCopy(modelorg, temp);
		AngleVectors(currententity->angles, forward, right, up);

		modelorg[0] = DotProduct(temp, forward);
		modelorg[1] = -DotProduct(temp, right);
		modelorg[2] = DotProduct(temp, up);
	}

	R_SetupEntityMatrix(currententity);

	// put camera into model space view angle for bmodels parallax
	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, BmodelViewOrg);

	R_DrawInlineBModel3();
	R_DrawChainsRA(qtrue);
}

void R_MarkBrushModelSurfaces (void) {
	int			i;
	msurface_t	*psurf;
	model_t		*clmodel;

	clmodel = currententity->model;
	psurf = &clmodel->surfaces[clmodel->firstModelSurface];


	for (i=0 ; i<clmodel->numModelSurfaces ; i++, psurf++){

		if (R_MarkLightSurf (psurf, qfalse, currentShadowLight))
			interaction[numInteractionSurfs++] = psurf;
	}
}

void R_DrawLightBrushModel (void) {
	vec3_t		mins, maxs, org;
	int			i;
	qboolean	rotated;
	vec3_t		tmp, oldLight;
	qboolean	caustics;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (currentmodel->numModelSurfaces == 0)
		return;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = qtrue;
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	} else {
		rotated = qfalse;
		VectorAdd(currententity->origin, currentmodel->mins, mins);
		VectorAdd(currententity->origin, currentmodel->maxs, maxs);
	}

	if(currentShadowLight->spherical){
		if(!BoundsAndSphereIntersect(mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return;
	}else{
		if(!BoundsIntersect(mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return;
	}

	R_SetupEntityMatrix(currententity);
	
	//Put camera into model space view angle for bmodels parallax
	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, BmodelViewOrg);

	VectorCopy(currentShadowLight->origin, oldLight);
	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	caustics = qfalse;
	currententity->minmax[0] = mins[0];
	currententity->minmax[1] = mins[1];
	currententity->minmax[2] = mins[2];
	currententity->minmax[3] = maxs[0];
	currententity->minmax[4] = maxs[1];
	currententity->minmax[5] = maxs[2];

	VectorSet(org, currententity->minmax[0], currententity->minmax[1], currententity->minmax[5]);
	if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
		caustics = qtrue;
	else
	{
		VectorSet(org, currententity->minmax[3], currententity->minmax[1], currententity->minmax[5]);
		if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
			caustics = qtrue;
		else
		{
			VectorSet(org, currententity->minmax[0], currententity->minmax[4], currententity->minmax[5]);
			if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
				caustics = qtrue;
			else
			{
				VectorSet(org, currententity->minmax[3], currententity->minmax[4], currententity->minmax[5]);
				if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
					caustics = qtrue;
			}
		}
	}
	
	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);

	GL_PolygonOffset(-1.0, -1.0);

	GL_BindProgram(lightWorldProgram);

	glBindVertexArray(vao.bsp_l);

	r_lightTimestamp++;
	numInteractionSurfs = 0;
	R_MarkBrushModelSurfaces();
	if(numInteractionSurfs > 0)
		GL_DrawDynamicLightPass(qtrue, caustics);
	
	glBindVertexArray(0);

	VectorCopy(oldLight, currentShadowLight->origin);
}



/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves (void) {
	byte	*vis;
	byte	fatvis[MAX_MAP_LEAFS/8];
	mnode_t	*node;
	int		i, c;
	mleaf_t	*leaf;
	int		cluster;

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 /*&& !r_novis->value*/ && r_viewcluster != -1)
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if (/*r_novis->value ||*/ r_viewcluster == -1 || !r_worldmodel->vis)
	{
		// mark everything
		for (i=0 ; i<r_worldmodel->numLeafs ; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;
		for (i=0 ; i<r_worldmodel->numNodes ; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;
		memset(&viewvis, 0xff, (r_worldmodel->numLeafs+7)>>3);	// all visible
		return;
	}

	vis = Mod_ClusterPVS (r_viewcluster, r_worldmodel);
	// may have to combine two clusters because of solid water boundaries
	if (r_viewcluster2 != r_viewcluster)
	{
		memcpy (fatvis, vis, (r_worldmodel->numLeafs+7)>>3);
		vis = Mod_ClusterPVS (r_viewcluster2, r_worldmodel);
		c = (r_worldmodel->numLeafs+31)/32;
		for (i=0 ; i<c ; i++)
			((int *)fatvis)[i] |= ((int *)vis)[i];
		vis = fatvis;
	}

	memcpy(&viewvis, vis, (r_worldmodel->numLeafs+7)>>3);

	for (i=0,leaf=r_worldmodel->leafs ; i<r_worldmodel->numLeafs ; i++, leaf++)
	{
		cluster = leaf->cluster;
		if (cluster == -1)
			continue;
		if (vis[cluster>>3] & (1<<(cluster&7)))
		{
			node = (mnode_t *)leaf;
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
}