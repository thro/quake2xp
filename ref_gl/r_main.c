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
// r_main.c
#include "r_local.h"

#ifndef _WIN32
#include <dlfcn.h>
#define qwglGetProcAddress( a ) dlsym( glw_state.hinstOpenGL, a )
#endif

viddef_t vid;

model_t *r_worldmodel;

double gldepthmin, gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;
entity_t *currententity;
model_t *currentmodel;

cplane_t frustum[6];

int r_visframecount;			// bumped when going to a new PVS
int r_framecount;				// used for dlight push checking

float v_blend[4];				// final blending color

void GL_Strings_f(void);

//
// view origin
//
vec3_t vup;
vec3_t vpn;
vec3_t vright;
vec3_t r_origin;

//
// screen size info
//
refdef_t r_newrefdef;

glwstate_t glw_state;

int r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;
int	occ_framecount;

/*=================
GL_ARB_Debug_output
From https://sites.google.com/site/opengltutorialsbyaks/introduction-to-opengl-4-1---tutorial-05
=================*/

void DebugOutput(unsigned source, unsigned type, unsigned id, unsigned severity, const char* message)
{
	char debSource[64], debType[64], debSev[64];

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	if (source == GL_DEBUG_SOURCE_API_ARB)
		strcpy(debSource, "OpenGL");
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
		strcpy(debSource, "Windows");
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
		strcpy(debSource, "Shader Compiler");
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
		strcpy(debSource, "Third Party");
	else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)
		strcpy(debSource, "Application");
	else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
		strcpy(debSource, "Other");
	else
		strcpy(debSource, va("Source 0x%X", source));

	if (type == GL_DEBUG_TYPE_ERROR_ARB)
		strcpy(debType, "Error");
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
		strcpy(debType, "Deprecated behavior");
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
		strcpy(debType, "Undefined behavior");
	else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB)
		strcpy(debType, "Portability");
	else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
		strcpy(debType, "Performance");
	else if (type == GL_DEBUG_TYPE_OTHER_ARB)
		strcpy(debType, "Other");
	else
		strcpy(debType, va("Type 0x%X", type));

	if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		strcpy(debSev, "High");
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		strcpy(debSev, "Medium");
	else if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
		strcpy(debSev, "Low");
	else
		strcpy(debSev, va("0x%X", severity));

	Com_Printf("GL_DEBUG: %s %s\nSeverity '%s': '%s'\nID: '%d'\n", debSource, debType, debSev, message, id);
}

void GL_CheckError(const char *fileName, int line, const char *subr)
{
	int         err;
	char        s[128];

#ifdef _WIN32
	if (!r_glDebugOutput->integer)
		return;

	err = qglGetError();
	if (err == GL_NO_ERROR)
		return;

		#define			MAX_GL_DEBUG_MESSAGES   16
		unsigned        sources[MAX_GL_DEBUG_MESSAGES];
		unsigned        types[MAX_GL_DEBUG_MESSAGES];
		unsigned        ids[MAX_GL_DEBUG_MESSAGES];
		unsigned        severities[MAX_GL_DEBUG_MESSAGES];
		int             lengths[MAX_GL_DEBUG_MESSAGES];
		char            messageLog[2048];
		unsigned        count = MAX_GL_DEBUG_MESSAGES;
		int             bufsize = 2048;

		unsigned retVal = glGetDebugMessageLogARB(count, bufsize, sources, types, ids, severities, lengths, messageLog);
		if (retVal > 0)
		{
			unsigned pos = 0;
			for (unsigned i = 0; i < retVal; i++)
			{
				DebugOutput(sources[i], types[i], ids[i], severities[i], &messageLog[pos]);
				pos += lengths[i];
			}
		}


	switch (err)
	{
	case GL_INVALID_ENUM:
		strcpy(s, "GL_INVALID_ENUM");
		break;
	case GL_INVALID_VALUE:
		strcpy(s, "GL_INVALID_VALUE");
		break;
	case GL_INVALID_OPERATION:
		strcpy(s, "GL_INVALID_OPERATION");
		break;
	case GL_STACK_OVERFLOW:
		strcpy(s, "GL_STACK_OVERFLOW");
		break;
	case GL_STACK_UNDERFLOW:
		strcpy(s, "GL_STACK_UNDERFLOW");
		break;
	case GL_OUT_OF_MEMORY:
		strcpy(s, "GL_OUT_OF_MEMORY");
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		strcpy(s, "GL_INVALID_FRAMEBUFFER_OPERATION_EXT");
		break;
	default:
		Com_sprintf(s, sizeof(s), "0x%X", err);
		break;
	}

	Com_Printf("GL_CheckErrors: %s in file '%s' subroutine '%s' line %i\n", s, fileName, subr, line);
#endif
}



/*
=============================================================

  SPRITE MODELS

=============================================================
*/


void R_DrawSpriteModel(entity_t * e)
{
	return;
}


static void R_DrawDistortSpriteModel(entity_t * e)
{
	dsprframe_t *frame;
	float		*up, *right;
	dsprite_t	*psprite;
	int			vert=0;
	int			len, scaled = 1;

	psprite = (dsprite_t *) currentmodel->extraData;
	e->frame %= psprite->numFrames;
	frame = &psprite->frames[e->frame];

	len = frame->width;

	// normal sprite
	up = vup;
	right = vright;

	GL_MBind(GL_TEXTURE1, currentmodel->skins[e->frame]->texnum);

	qglUniform1f(U_REFR_ALPHA, e->alpha);
	qglUniform1f(U_REFR_THICKNESS0, len * 0.5);
	qglUniform1f(U_REFR_THICKNESS1, len * 0.5);

	if (currententity->flags & RF_BFG_SPRITE) {
		GL_MBind(GL_TEXTURE1, r_notexture->texnum);
		scaled = 2;
	}
	else		
		GL_MBind(GL_TEXTURE1, currentmodel->skins[e->frame]->texnum);
	
	VectorMA	(e->origin,				-frame->origin_y * scaled, up, wVertexArray[vert+0]);
	VectorMA	(wVertexArray[vert+0],	-frame->origin_x * scaled, right, wVertexArray[vert+0]);
	VA_SetElem2	(wTexArray[vert+0],		0, 1);
	
	VectorMA	(e->origin,				frame->height * scaled - frame->origin_y * scaled, up, wVertexArray[vert+1]);
	VectorMA	(wVertexArray[vert+1], -frame->origin_x * scaled, right, wVertexArray[vert+1]);
    VA_SetElem2	(wTexArray[vert+1],		0, 0);

	VectorMA	(e->origin,				frame->height * scaled - frame->origin_y * scaled, up, wVertexArray[vert+2]);
	VectorMA	(wVertexArray[vert+2],	frame->width * scaled - frame->origin_x * scaled, right, wVertexArray[vert+2]);
    VA_SetElem2	(wTexArray[vert+2],		1, 0);

	VectorMA (e->origin,				-frame->origin_y * scaled, up, wVertexArray[vert+3]);
	VectorMA (wVertexArray[vert+3],		frame->width * scaled - frame->origin_x * scaled, right, wVertexArray[vert+3]);
    VA_SetElem2(wTexArray[vert+3],		1, 1);

	vert+=4;
	
	if(vert)
		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel(void)
{
	return;
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame(void)
{
	int i;
	mleaf_t *leaf;

	r_framecount++;
	occ_framecount++;

	// build the transformation matrix for the given view angles
	VectorCopy(r_newrefdef.vieworg, r_origin);

	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);
	
	// current viewcluster
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;
		
		// check above and below so crossing solid water doesn't draw
		// wrong
		if (!leaf->contents) {	// look down a bit
			vec3_t temp;

			VectorCopy(r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);
			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
				r_viewcluster2 = leaf->cluster;
		} else {				// look up a bit
			vec3_t temp;

			VectorCopy(r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);
			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
				r_viewcluster2 = leaf->cluster;
		}
	}

	if(CL_PMpointcontents(r_origin) & CONTENTS_SOLID)
		outMap = qtrue;
	else
		outMap = qfalse;

	for (i = 0; i < 4; i++)
		v_blend[i] = r_newrefdef.blend[i];

	// clear out the portion of the screen that the NOWORLDMODEL defines
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) {

			GL_Enable(GL_SCISSOR_TEST);
			GL_Scissor(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);

		if (!(r_newrefdef.rdflags & RDF_NOCLEAR)) {
			qglClearColor(0.0, 0.0, 0.0, 0.0);
			qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		} else
			qglClear(GL_DEPTH_BUFFER_BIT);
		GL_Disable(GL_SCISSOR_TEST);
	}

	
}

/*
=============
R_SetupViewMatrices

=============
*/

// convert from Q2 coordinate system (screen depth is X)
// to OpenGL's coordinate system (screen depth is -Z)
static mat4_t r_flipMatrix = {
	{  0.f, 0.f, -1.f, 0.f },	// world +X -> screen -Z
	{ -1.f, 0.f,  0.f, 0.f },	// world +Y -> screen -X
	{  0.f, 1.f,  0.f, 0.f },	// world +Z -> screen +Y (Y=0 being the bottom of the screen)
	{  0.f, 0.f,  0.f, 1.f }
};

static void R_SetupViewMatrices (void) {
	mat4_t	tmpMatrix;
	int		i;
	// setup perspective projection matrix
	r_newrefdef.projectionMatrix[0][0] = 1.f / tan(DEG2RAD(r_newrefdef.fov_x) * 0.5f);
	r_newrefdef.projectionMatrix[0][1] = 0.f;
	r_newrefdef.projectionMatrix[0][2] = 0.f;
	r_newrefdef.projectionMatrix[0][3] = 0.f;

	r_newrefdef.projectionMatrix[1][0] = 0.f;
	r_newrefdef.projectionMatrix[1][1] = 1.f / tan(DEG2RAD(r_newrefdef.fov_y) * 0.5f);
	r_newrefdef.projectionMatrix[1][2] = 0.f;
	r_newrefdef.projectionMatrix[1][3] = 0.f;

	r_newrefdef.projectionMatrix[2][0] = 0.f;
	r_newrefdef.projectionMatrix[2][1] = 0.f;
	r_newrefdef.projectionMatrix[2][2] = -0.999f; // infinite
	r_newrefdef.projectionMatrix[2][3] = -1.f;

	r_newrefdef.projectionMatrix[3][0] = 0.f;
	r_newrefdef.projectionMatrix[3][1] = 0.f;
	r_newrefdef.projectionMatrix[3][2] = -2.f * r_zNear->value; // infinite
	r_newrefdef.projectionMatrix[3][3] = 0.f;

	r_newrefdef.depthParms[0] = r_zNear->value;
	r_newrefdef.depthParms[1] = 0.9995f;

	// setup view matrix
	AnglesToMat3(r_newrefdef.viewangles, r_newrefdef.axis);
	Mat4_SetupTransform(r_newrefdef.modelViewMatrix, r_newrefdef.axis, r_newrefdef.vieworg);
	Mat4_AffineInvert(r_newrefdef.modelViewMatrix, tmpMatrix);
	Mat4_Multiply(tmpMatrix, r_flipMatrix, r_newrefdef.modelViewMatrix);

	// scissors transform
	Mat4_Multiply	(r_newrefdef.modelViewMatrix, r_newrefdef.projectionMatrix, r_newrefdef.modelViewProjectionMatrix);
	Mat4_Transpose	(r_newrefdef.modelViewProjectionMatrix, r_newrefdef.modelViewProjectionMatrixTranspose);

	// set sky matrix
	Mat4_Copy(r_newrefdef.modelViewMatrix, tmpMatrix);
	Mat4_Translate(tmpMatrix, r_newrefdef.vieworg[0], r_newrefdef.vieworg[1], r_newrefdef.vieworg[2]);
	if (skyrotate)
		Mat4_Rotate(tmpMatrix, r_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);

	Mat4_Multiply(tmpMatrix, r_newrefdef.projectionMatrix, r_newrefdef.skyMatrix);

	float tx, ty;
	mat3_t axis;
	// compute the world-space rays to the far plane corners
	tx = tan(DEG2RAD(r_newrefdef.fov_x * 0.5f));
	ty = tan(DEG2RAD(r_newrefdef.fov_y * 0.5f));

	for (i = 0; i < 3; i++) {
		axis[0][i] = r_newrefdef.axis[0][i];
		axis[1][i] = r_newrefdef.axis[1][i] * tx;
		axis[2][i] = r_newrefdef.axis[2][i] * ty;

		// counter-clockwise order
		r_newrefdef.cornerRays[0][i] = axis[0][i] + axis[1][i] + axis[2][i];	// top left
		r_newrefdef.cornerRays[1][i] = axis[0][i] + axis[1][i] - axis[2][i];	// bottom left
		r_newrefdef.cornerRays[2][i] = axis[0][i] - axis[1][i] - axis[2][i];	// bottom right
		r_newrefdef.cornerRays[3][i] = axis[0][i] - axis[1][i] + axis[2][i];	// top right
	}
}

void R_SetupEntityMatrix(entity_t * e) {

	AnglesToMat3(e->angles, e->axis);
	Mat4_SetOrientation(e->matrix, e->axis, e->origin);
	Mat4_TransposeMultiply(e->matrix, r_newrefdef.modelViewProjectionMatrix, e->orMatrix);

	if ((e->flags & RF_WEAPONMODEL) && (r_leftHand->integer == 1)) { // Flip player weapon
		Mat4_Scale(e->orMatrix, 1.0, -1.0, 1.0);
		GL_CullFace(GL_FRONT);
	} else
		GL_CullFace(GL_BACK);
}

void R_SetupOrthoMatrix(void) {
	// set 2D virtual screen size
	qglViewport(0, 0, vid.width, vid.height);

	// setup orthographic projection
	r_newrefdef.orthoMatrix[0][0] = 2.f / (float)vid.width;
	r_newrefdef.orthoMatrix[0][1] = 0.f;
	r_newrefdef.orthoMatrix[0][2] = 0.f;
	r_newrefdef.orthoMatrix[0][3] = 0.f;
	r_newrefdef.orthoMatrix[1][0] = 0.f;
	r_newrefdef.orthoMatrix[1][1] = -2.f / (float)vid.height;
	r_newrefdef.orthoMatrix[1][2] = 0.f;
	r_newrefdef.orthoMatrix[1][3] = 0.f;
	r_newrefdef.orthoMatrix[2][0] = 0.f;
	r_newrefdef.orthoMatrix[2][1] = 0.f;
	r_newrefdef.orthoMatrix[2][2] = -1.f;
	r_newrefdef.orthoMatrix[2][3] = 0.f;
	r_newrefdef.orthoMatrix[3][0] = -1.f;
	r_newrefdef.orthoMatrix[3][1] = 1.f;
	r_newrefdef.orthoMatrix[3][2] = 0.f;
	r_newrefdef.orthoMatrix[3][3] = 1.f;

	GL_Disable(GL_DEPTH_TEST);
	GL_Disable(GL_CULL_FACE);
}

/*
=============
R_SetupGL
=============
*/

void R_SetupGL(void)
{
	// set drawing parms
	GL_Enable(GL_CULL_FACE);
	GL_Disable(GL_BLEND);
	GL_Enable(GL_DEPTH_TEST);
	GL_DepthMask(1);
}

/*
=============
R_Clear
=============
*/

void R_Clear(void)
{
	qglClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	gldepthmin = 0.0;
	gldepthmax = 1.0;

	GL_DepthFunc(GL_LEQUAL); 
	GL_DepthRange(gldepthmin, gldepthmax);
}

void R_DrawPlayerWeaponLightPass(void)
{
	int i;

	if (!r_drawEntities->integer)
		return;

	GL_DepthFunc(GL_LEQUAL);
	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);

		for (i = 0; i < r_newrefdef.num_entities; i++)	// weapon model
		{
			currententity = &r_newrefdef.entities[i];
			currentmodel = currententity->model;
			if (currententity->flags & RF_TRANSLUCENT)
				continue;

			if (!currentmodel)
				continue;
			if (!(currententity->flags & RF_WEAPONMODEL))
				continue;

			if (currentmodel->type == mod_alias)
				R_DrawAliasModelLightPass(qtrue);

			if (currentmodel->type == mod_alias_md3)
				R_DrawMD3MeshLight(qtrue);	
		}

}

void R_DrawPlayerWeaponAmbient(void)
{
	int i;

	if (!r_drawEntities->integer)
		return;

	// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		currentmodel = currententity->model;

		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;

		if (currentmodel->type == mod_alias)
			R_DrawAliasModel(currententity);

		if(currentmodel->type == mod_alias_md3)
			R_DrawMD3Mesh(qtrue);
	}

	// draw transluscent shells
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);
	GL_DepthMask(0);
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;

		currentmodel = currententity->model;

		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;

		if (currentmodel->type == mod_alias)
			R_DrawAliasModel(currententity);

		if (currentmodel->type == mod_alias_md3) {
			if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
				R_DrawMD3ShellMesh(qtrue);
		}
	}

	GL_Disable(GL_BLEND);
	GL_DepthMask(1);
}

qboolean R_GetLightOcclusionResult();
void R_LightOcclusionTest();

void R_DrawLightScene (void)
{
	int i;

	num_visLights = 0;

	GL_DepthMask(0);
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE /*GL_DST_COLOR, GL_ZERO*/);

	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {

		if (r_useLightScissors->integer)
			GL_Enable(GL_SCISSOR_TEST);

		if (gl_state.depthBoundsTest && r_useDepthBounds->integer)
			GL_Enable(GL_DEPTH_BOUNDS_TEST_EXT);

		if (r_shadows->integer)
			GL_Enable(GL_STENCIL_TEST);

		GL_Enable(GL_POLYGON_OFFSET_FILL);
	}

	R_PrepareShadowLightFrame(qfalse);
	
	if(shadowLight_frame) {

	for(currentShadowLight = shadowLight_frame; currentShadowLight; currentShadowLight = currentShadowLight->next) {

	if (r_skipStaticLights->integer && currentShadowLight->isStatic)
		continue;
	
	if ((r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !currentShadowLight->isNoWorldModel)
		continue;

	UpdateLightEditor();
	
	R_SetViewLightScreenBounds();

	if(r_useLightScissors->integer)
		GL_Scissor(currentShadowLight->scissor[0], currentShadowLight->scissor[1], currentShadowLight->scissor[2], currentShadowLight->scissor[3]);
	
	if(gl_state.depthBoundsTest && r_useDepthBounds->integer)
		GL_DepthBoundsTest(currentShadowLight->depthBounds[0], currentShadowLight->depthBounds[1]);

	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
		qglClearStencil(128);
		GL_StencilMask(255);
		qglClear(GL_STENCIL_BUFFER_BIT);
	}

	if (!R_GetLightOcclusionResult())
		continue;

	R_CastBspShadowVolumes();			// bsp and bmodels shadows
	R_CastAliasShadowVolumes(qtrue);	// player shadow

	for (i = 0; i < r_newrefdef.num_entities; i++) { 
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		if ((currententity->flags & RF_TRANSLUCENT) && (currentmodel->type == mod_alias))
			continue;

		if (currententity->flags & RF_DISTORT)
			continue;

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}
		
		if (currentmodel->type == mod_alias)
			R_DrawAliasModelLightPass(qfalse);
		
		if (currentmodel->type == mod_alias_md3)
			R_DrawMD3MeshLight(qfalse);
	}

	R_CastAliasShadowVolumes(qfalse);   // alias shadows with out player model
	R_DrawLightWorld();					// light world
	//brush models light pass
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;			

		if (currententity->flags & RF_DISTORT)
				continue;

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}
		if (currentmodel->type == mod_brush) 
			R_DrawLightBrushModel();
		}
	
	R_DrawLightFlare();				// light flare
	R_DrawLightBounds();			// debug stuff

	}
	}
	
	GL_DepthMask(1);
	GL_Disable(GL_STENCIL_TEST);
	GL_Disable(GL_SCISSOR_TEST);
	if(gl_state.depthBoundsTest && r_useDepthBounds->integer)
		GL_Disable(GL_DEPTH_BOUNDS_TEST_EXT);
	GL_Disable(GL_BLEND);

	GL_Disable(GL_POLYGON_OFFSET_FILL);
	GL_PolygonOffset(0.0, 0.0);
}

void R_DrawPlayerWeapon(void)
{
	if (!r_drawEntities->integer)
		return;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	R_DrawPlayerWeaponAmbient();

	GL_DepthMask(0);
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);

	if (r_useLightScissors->integer)
		GL_Enable(GL_SCISSOR_TEST);

	if (gl_state.depthBoundsTest && r_useDepthBounds->integer)
		GL_Enable(GL_DEPTH_BOUNDS_TEST_EXT);

	if (r_shadows->integer)
		GL_Enable(GL_STENCIL_TEST);

	R_PrepareShadowLightFrame(qtrue);

	if (shadowLight_frame) {

		for (currentShadowLight = shadowLight_frame; currentShadowLight; currentShadowLight = currentShadowLight->next) {

			if (r_skipStaticLights->integer && currentShadowLight->isStatic)
				continue;

			if (currentShadowLight->filter == 33) // flashlight cut off
				continue;

			R_SetViewLightScreenBounds();

			if (r_useLightScissors->integer)
				GL_Scissor(currentShadowLight->scissor[0], currentShadowLight->scissor[1], currentShadowLight->scissor[2], currentShadowLight->scissor[3]);

			if (gl_state.depthBoundsTest && r_useDepthBounds->integer)
				GL_DepthBoundsTest(currentShadowLight->depthBounds[0], currentShadowLight->depthBounds[1]);

			qglClearStencil(128);
			GL_StencilMask(255);
			qglClear(GL_STENCIL_BUFFER_BIT);
			
			R_CastBspShadowVolumes();
			R_DrawPlayerWeaponLightPass();
		}
	}

	GL_DepthMask(1);
	GL_Disable(GL_STENCIL_TEST);
	GL_Disable(GL_SCISSOR_TEST);
	if (gl_state.depthBoundsTest && r_useDepthBounds->integer)
		GL_Disable(GL_DEPTH_BOUNDS_TEST_EXT);
	GL_Disable(GL_BLEND);
}

void R_RenderSprites(void)
{
	int i;

	GL_DepthMask(0);

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, wVertexArray);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, wTexArray);

	GL_MBind	(GL_TEXTURE0, r_distort->texnum);
	GL_MBind	(GL_TEXTURE2, ScreenMap->texnum);
	GL_MBindRect(GL_TEXTURE3, depthMap->texnum);

	// setup program
	GL_BindProgram(refractProgram);

	qglUniform1f(U_REFR_DEFORM_MUL, 2.5);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, qfalse, (const float *)r_newrefdef.projectionMatrix);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform2f(U_REFR_MASK, 0.0, 1.0);
	qglUniform1i(U_REFR_ALPHA_MASK, 1);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currentmodel->type == mod_sprite)
			R_DrawDistortSpriteModel(currententity);
	}

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GL_DepthMask(1);
	
}

// draws ambient opaque entities
static void R_DrawEntitiesOnList (void) {
	int i;

	if (!r_drawEntities->integer)
		return;

	// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL) || ((currententity->flags & RF_DISTORT) && !(r_newrefdef.rdflags & RDF_IRGOGGLES)))
			continue;

		if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
			continue;
		
		if (currententity->flags & RF_BEAM) {
			R_DrawBeam();
			continue;
		}

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}

		switch (currentmodel->type) {
			case mod_alias:
				R_DrawAliasModel(currententity);
				break;
			case mod_brush:
				R_DrawBrushModel();
				break;
			case mod_sprite:
				R_DrawSpriteModel(currententity);
				break;
			case mod_alias_md3:
				R_DrawMD3Mesh(qfalse);
				break;
			default:
				VID_Error(ERR_DROP, "Bad modeltype");
				break;
		}
	}
}

static void R_DrawTransEntities(void) {
	int i;

	if (!r_drawEntities->integer)
		return;

	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);
	GL_DepthMask(0); // wtf??? 

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;

		if (currententity->flags & (RF_WEAPONMODEL))
			continue;

		if (currententity->flags & RF_BEAM) {
			R_DrawBeam();
			continue;
		}

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}

		if (currentmodel->type == mod_alias)
			R_DrawAliasModel(currententity);

		if (currentmodel->type == mod_alias_md3) {
			if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
				R_DrawMD3ShellMesh(qfalse);
		}
	}
	GL_Disable(GL_BLEND);
	GL_DepthMask(1);
}

// draw all opaque, non-reflective stuff
// fills reflective & alpha chains from world model
static void R_DrawAmbientScene (void) {

	R_DrawBSP();

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		GL_DepthMask(1);

	R_DrawEntitiesOnList();

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		GL_DepthMask(0);
}

// draws reflective & alpha chains from world model
// draws reflective surfaces from brush models

static void R_DrawRAScene (void) {
	int i;
	
	GL_DepthMask(0);
	GL_PolygonOffset(-1.0, 1.0);

	RA_Frame = qfalse;

	if (r_reflective_surfaces || r_alpha_surfaces)
		RA_Frame = qtrue;

	R_DrawChainsRA(qfalse);

	GL_DepthMask(1);
	GL_PolygonOffset(0.0, 1.0);

	R_CaptureColorBuffer();

	if (!r_drawEntities->integer)
		return;

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		
		if (currentmodel && currentmodel->type == mod_brush) {
			R_DrawBrushModelRA();
			continue;
		}
	}
}

/*
================
R_RenderView

r_newrefdef must be set before the first call.
================
*/
void R_MotionBlur(void);
extern uint fboDps;

void R_RenderView (refdef_t *fd) {
	
	if (r_noRefresh->integer)
		return;

	r_newrefdef = *fd;
	r_newrefdef.viewport[0] = fd->x;
	r_newrefdef.viewport[1] = vid.height - fd->height - fd->y;
	r_newrefdef.viewport[2] = fd->width;
	r_newrefdef.viewport[3] = fd->height;

	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);

	if (!r_worldmodel && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		VID_Error(ERR_DROP, "R_RenderView: NULL worldmodel.");

	R_SetupFrame();
	R_SetFrustum();
	R_SetupViewMatrices();
	R_SetupGL();
	R_MarkLeaves();				// done here so we know if we're in water
	
	R_DrawDepthScene();
	R_LightOcclusionTest();
	R_CaptureDepthBuffer();
	
	R_SSAO();
	R_DrawAmbientScene();
	R_DrawLightScene();
	R_RenderDecals();
	
	R_DrawParticles();

	R_CaptureColorBuffer();

	R_DrawRAScene();
	
	if (RA_Frame && r_particlesOverdraw->integer) { // overdraw particles if we have trans or reflective surfaces in frame
		R_DrawParticles();
		RA_Frame = qfalse;
	}
	R_DrawTransEntities();

	R_CaptureColorBuffer();
	R_RenderSprites();

	R_MotionBlur();
	R_GlobalFog();
	R_DrawPlayerWeapon();
}


/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel (void) {
	vec3_t amb;
	float mid;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)
	R_LightPoint(r_newrefdef.vieworg, amb);

	mid = max(max(amb[0], amb[1]), amb[2]);
	if (mid <= 0.1)
		mid = 0.15;

	mid *= 2.0;
	r_lightLevel->value = 150 * mid; // convert to byte
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
extern char buff0[128];
extern char buff1[128];
extern char buff2[128];
extern char buff3[128];
extern char buff4[128];
extern char buff5[128];
extern char buff6[128];
extern char buff7[128];
extern char buff8[128];
extern char buff9[128];
extern char buff10[128];
extern char buff11[128];
extern char buff12[128];
extern char buff13[128];
extern char buff14[128];
extern char buff15[128];

extern worldShadowLight_t *selectedShadowLight;

void R_FixFov(void);

void R_RenderFrame(refdef_t * fd) {

	R_RenderView(fd);
	R_SetupOrthoMatrix();
	R_SetLightLevel();

	// post processing - cut off if player camera is out of map bounds
	if (!outMap) {
		R_FixFov();
		R_FXAA();
		R_RadialBlur();
		R_ThermalVision();
		R_DofBlur();
		R_Bloom();

		R_FilmFilter();
		R_ScreenBlend();
	}
	R_ColorTemperatureCorrection();
	R_lutCorrection();

	// set alpha blend for 2D mode
	GL_Enable(GL_BLEND); 
	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (selectedShadowLight && r_lightEditor->integer){
		Set_FontShader(qtrue);
		RE_SetColor(colorCyan);
		Draw_StringScaled(0, vid.height*0.5,     2, 2, buff0);
		Draw_StringScaled(0, vid.height*0.5+25,  2, 2, buff1);
		Draw_StringScaled(0, vid.height*0.5+45,  2, 2, buff2);
		Draw_StringScaled(0, vid.height*0.5+65,  2, 2, buff3);
		Draw_StringScaled(0, vid.height*0.5+85,  2, 2, buff4);
		Draw_StringScaled(0, vid.height*0.5+105, 2, 2, buff5);
		Draw_StringScaled(0, vid.height*0.5+125, 2, 2, buff6);
		Draw_StringScaled(0, vid.height*0.5+145, 2, 2, buff7);
		Draw_StringScaled(0, vid.height*0.5+165, 2, 2, buff8);
		Draw_StringScaled(0, vid.height*0.5+185, 2, 2, buff9);
		Draw_StringScaled(0, vid.height*0.5+205, 2, 2, buff12);
		Draw_StringScaled(0, vid.height*0.5+225, 2, 2, buff13);
		Draw_StringScaled(0, vid.height*0.5+245, 2, 2, buff10);
		Draw_StringScaled(0, vid.height*0.5+265, 2, 2, buff11);
		Draw_StringScaled(0, vid.height*0.5+285, 2, 2, buff14);
		Draw_StringScaled(0, vid.height*0.5+305, 2, 2, buff15);
		RE_SetColor(colorWhite);
		Set_FontShader(qfalse);
	}
}

void AutoLightsStatsList_f(void){

	Com_Printf("%i raw auto lights\n", r_numAutoLights);
	Com_Printf("%i clean auto lights\n", r_numIgnoreAutoLights);
	Com_Printf("%i total auto lights\n", r_numAutoLights - r_numIgnoreAutoLights);
}


void Dump_EntityString(void){

	char *buf;
	FILE *f;
	char name[MAX_OSPATH];
	
	if(!r_worldmodel->name[0]){
		Com_Printf(S_COLOR_RED"You must be in a game to dump entity string\n");
			return;
	}

	Com_sprintf(name, sizeof(name), "%s/%s", FS_Gamedir(), r_worldmodel->name);
	
	name[strlen(name) - 4] = 0;
	strcat(name, ".ent");
	
	Com_Printf("Dump entity string to "S_COLOR_GREEN"%s\n", name);
	FS_CreatePath(name);

	buf = CM_EntityString();
	
	f = fopen(name, "w");
	if (!f) {
		Com_Printf(S_COLOR_RED"ERROR: couldn't open.\n");
		return;
	}

	fputs(buf, f);
	fclose(f);


}
/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine(char **data) {
	char    *p;
	int     c;

	p = *data;
	while ((c = *p++) != 0) {
		if (c == '\n') {
			//	com_lines++;
			break;
		}
	}

	*data = p;
}

void Cube2Lut_f(void)
{
	char *buf, *buf0;
	FILE *out;
	char name[MAX_OSPATH], outName[MAX_OSPATH];
	int i, len, lutSize = 0;
	char *token, *titlePtr = NULL;
	float x, y, z;

	if (Cmd_Argc() != 2) {
		Com_Printf(S_COLOR_YELLOW"Usage: %s <adobe cube lut filename>\n", Cmd_Argv(0));
		return;
	}

	Com_sprintf(name, sizeof(name), "gfx/lut/%s.cube", Cmd_Argv(1));
	len = FS_LoadFile(name, (void**)&buf);
	if (!buf || len <= 0)
	{
		if (buf)
			FS_FreeFile(buf);
		Com_Printf(S_COLOR_RED"ERROR: couldn't open %s\n", name);
		return;
	}

	Com_sprintf(outName, sizeof(outName), "%s/gfx/lut/%s.lut", FS_Gamedir(), Cmd_Argv(1));
	out = fopen(outName, "wb");
	if (!out) {
		FS_FreeFile(buf);
		Com_Printf(S_COLOR_RED"ERROR: couldn't open %s\n", outName);
		return;
	}

	buf0 = buf;
	while (buf)
	{
		if (sscanf(buf, "%f %f %f", &x, &y, &z) == 3) {

			if (lutSize <= 0) {
				FS_FreeFile(buf0);
				fclose(out);
				Com_Printf(S_COLOR_RED"ERROR: lut file %s is corrupted\n", name);
				return;
			}
			// save lut size
			fwrite(&lutSize, 1, sizeof(lutSize), out);
			// read lut block
			for (i = 0; i < lutSize * lutSize * lutSize; i++)
			{
				if (sscanf(buf, "%f %f %f", &x, &y, &z) != 3) {
					FS_FreeFile(buf0);
					fclose(out);
					Com_Printf(S_COLOR_RED"ERROR: lut file %s is truncated\n", name);
					return;
				}
				fwrite(&x, 1, sizeof(x), out);
				fwrite(&y, 1, sizeof(y), out);
				fwrite(&z, 1, sizeof(z), out);
				SkipRestOfLine(&buf);
			}
			break;
		}

		token = COM_Parse(&buf);
		if (token[0] == '#')
		{   // comment
			SkipRestOfLine(&buf);
			continue;
		}
		if (!Q_strcasecmp(token, "LUT_3D_SIZE"))
		{   // lut size
			token = COM_Parse(&buf);
			lutSize = atoi(token);
			continue;
		}
		if (!Q_strcasecmp(token, "TITLE"))
		{   // name of lut
			titlePtr = buf;
			SkipRestOfLine(&buf);
			continue;
		}
		SkipRestOfLine(&buf);
	}

	if (titlePtr)
	{
		token = COM_Parse(&titlePtr);
		fwrite(token, 1, strlen(token), out);
	}
	FS_FreeFile(buf0);
	fclose(out);
	Com_Printf(S_COLOR_YELLOW"wrote: %s\n", outName);
}


#define		GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define		GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define		GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define		GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define		GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

#define     VBO_FREE_MEMORY_ATI                     0x87FB
#define     TEXTURE_FREE_MEMORY_ATI                 0x87FC
#define     RENDERBUFFER_FREE_MEMORY_ATI            0x87FD

void R_VideoInfo_f(void){

#ifdef _WIN32
	int mem[4];
	
	if (IsExtensionSupported("GL_NVX_gpu_memory_info")) {
						
		Com_Printf("\nNvidia specific memory info:\n");
		Com_Printf("\n");
		qglGetIntegerv ( GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX , mem);
		Com_Printf("dedicated video memory %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX , mem);
		Com_Printf("total available memory %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX , mem);
		Com_Printf("currently unused GPU memory %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_EVICTION_COUNT_NVX , mem);
		Com_Printf("count of total evictions seen by system %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_EVICTED_MEMORY_NVX , mem);
		Com_Printf("total video memory evicted %i MB\n", mem[0] >>10);

	}
	else
#endif
		Com_Printf("MemInfo not availabled for your video card or driver!\n");
}
// 512 mb vram
void R_LowSpecMachine_f(void)
{
Cvar_Set("r_textureCompression", "1");
Cvar_Set("r_maxTextureSize", "512");
Cvar_Set("r_anisotropic", "4");
Cvar_Set("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR");

Cvar_Set("r_shadows", "0");
Cvar_Set("r_drawFlares", "0");
Cvar_Set("r_reliefMapping", "0");
Cvar_Set("r_skipStaticLights", "1");
Cvar_Set("r_lightmapScale", "1.0");
Cvar_Set("r_bloom", "0");
Cvar_Set("r_dof", "0");
Cvar_Set("r_radialBlur", "1");
Cvar_Set("r_softParticles", "1");
Cvar_Set("r_motionBlur", "0");
Cvar_Set("r_ssao", "0");

vid_ref->modified = qtrue;
}
// 1 gb vram
void R_MediumSpecMachine_f(void)
{
Cvar_Set("r_textureCompression", "0");
Cvar_Set("r_maxTextureSize", "512");
Cvar_Set("r_anisotropic", "8");
Cvar_Set("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR");

Cvar_Set("r_shadows", "1");
Cvar_Set("r_drawFlares", "1");
Cvar_Set("r_reliefMapping", "0");
Cvar_Set("r_skipStaticLights", "0");
Cvar_Set("r_lightmapScale", "0.5");
Cvar_Set("r_bloom", "1");
Cvar_Set("r_dof", "0");
Cvar_Set("r_radialBlur", "1");
Cvar_Set("r_softParticles", "1");
Cvar_Set("r_motionBlur", "0");
Cvar_Set("r_ssao", "0");

vid_ref->modified = qtrue;
}

void R_HiSpecMachine_f(void)
{
Cvar_Set("r_textureCompression", "0");
Cvar_Set("r_maxTextureSize", "0");
Cvar_Set("r_anisotropic", "16");
Cvar_Set("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR");

Cvar_Set("r_shadows", "1");
Cvar_Set("r_drawFlares", "1");
Cvar_Set("r_reliefMapping", "1");
Cvar_Set("r_skipStaticLights", "0");
Cvar_Set("r_lightmapScale", "0.5");
Cvar_Set("r_bloom", "1");
Cvar_Set("r_dof", "1");
Cvar_Set("r_radialBlur", "1");
Cvar_Set("r_softParticles", "1");
Cvar_Set("r_fxaa", "1");
Cvar_Set("r_motionBlur", "1");
Cvar_Set("r_ssao", "1");

vid_ref->modified = qtrue;
}

void R_GLSLinfo_f(void);
void GL_LevelShot_f(void);

void R_RegisterCvars(void)
{
	r_leftHand =						Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	r_noRefresh =						Cvar_Get("r_noRefresh", "0", 0);
	r_drawEntities =					Cvar_Get("r_drawEntities", "1", 0);
	r_drawWorld =						Cvar_Get("r_drawWorld", "1", 0);
	r_noVis =							Cvar_Get("r_noVis", "0", 0);
	r_noCull =							Cvar_Get("r_noCull", "0", 0);
	r_speeds =							Cvar_Get("r_speeds", "0", 0);

	r_lightLevel =						Cvar_Get("r_lightLevel", "0", 0);

	r_mode =							Cvar_Get("r_mode", "0", CVAR_ARCHIVE);
	r_noBind =							Cvar_Get("r_noBind", "0", 0);
	r_lockPvs =							Cvar_Get("r_lockPvs", "0", 0);

	r_vsync =							Cvar_Get("r_vsync", "0", CVAR_ARCHIVE);
	
	r_fullScreen =						Cvar_Get("r_fullScreen", "1", CVAR_ARCHIVE);
	
	r_brightness =						Cvar_Get("r_brightness", "1.0", CVAR_ARCHIVE);
	r_contrast	=						Cvar_Get("r_contrast", "1.0", CVAR_ARCHIVE);
	r_saturation =						Cvar_Get("r_saturation", "1.0", CVAR_ARCHIVE);
	r_gamma =							Cvar_Get("r_gamma", "1.0", CVAR_ARCHIVE); 

	r_colorVibrance =					Cvar_Get("r_colorVibrance", "0.0", CVAR_ARCHIVE);
	r_colorBalanceRed =					Cvar_Get("r_colorBalanceRed", "1.0", CVAR_ARCHIVE);;
	r_colorBalanceGreen =				Cvar_Get("r_colorBalanceGreen", "1.0", CVAR_ARCHIVE);;
	r_colorBalanceBlue =				Cvar_Get("r_colorBalanceBlue", "1.0", CVAR_ARCHIVE);;

	vid_ref =							Cvar_Get("vid_ref", "xpgl", CVAR_ARCHIVE);
	r_displayRefresh =					Cvar_Get("r_displayRefresh", "0", CVAR_ARCHIVE);

	r_anisotropic =						Cvar_Get("r_anisotropic", "16", CVAR_ARCHIVE);
	r_maxAnisotropy =					Cvar_Get("r_maxAnisotropy", "0", 0);
	r_maxTextureSize=					Cvar_Get("r_maxTextureSize", "0", CVAR_ARCHIVE);
	r_textureColorScale =				Cvar_Get("r_textureColorScale", "2", CVAR_ARCHIVE);
	r_textureCompression =				Cvar_Get("r_textureCompression", "0", CVAR_ARCHIVE);			
	r_causticIntens =					Cvar_Get("r_causticIntens", "5.0", CVAR_ARCHIVE);
	r_textureMode =						Cvar_Get("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR", CVAR_ARCHIVE);
	r_textureLodBias =					Cvar_Get("r_textureLodBias", "0.0", CVAR_ARCHIVE);
	
	r_imageAutoBump	=					Cvar_Get("r_imageAutoBump", "1", CVAR_ARCHIVE);
	r_imageAutoBumpScale =				Cvar_Get("r_imageAutoBumpScale", "6.0", CVAR_ARCHIVE);
	r_imageAutoSpecularScale =			Cvar_Get("r_imageAutoSpecularScale", "1", CVAR_ARCHIVE);

	r_screenShot =						Cvar_Get("r_screenShot", "jpg", CVAR_ARCHIVE);
	r_screenShotJpegQuality =			Cvar_Get("r_screenShotJpegQuality", "99", CVAR_ARCHIVE);

	r_multiSamples =					Cvar_Get("r_multiSamples", "0", CVAR_ARCHIVE);
	r_fxaa =							Cvar_Get("r_fxaa", "1", CVAR_ARCHIVE);

	deathmatch =						Cvar_Get("deathmatch", "0", CVAR_SERVERINFO);
	
	r_drawFlares =						Cvar_Get("r_drawFlares", "1", CVAR_ARCHIVE);
	r_scaleAutoLightColor =				Cvar_Get("r_scaleAutoLightColor", "3", CVAR_ARCHIVE);
	r_lightWeldThreshold =				Cvar_Get("r_lightWeldThreshold", "32", CVAR_ARCHIVE);

	r_customWidth =						Cvar_Get("r_customWidth", "1024", CVAR_ARCHIVE);
	r_customHeight =					Cvar_Get("r_customHeight", "768", CVAR_ARCHIVE);
		
	hunk_bsp=							Cvar_Get("hunk_bsp", "70", CVAR_ARCHIVE);
	hunk_md2=							Cvar_Get("hunk_md2", "4", CVAR_ARCHIVE);
	hunk_md3=							Cvar_Get("hunk_md3", "20", CVAR_ARCHIVE);
	hunk_sprite=						Cvar_Get("hunk_sprite", "0.08", CVAR_ARCHIVE);

	r_reliefMapping =					Cvar_Get("r_reliefMapping", "1", CVAR_ARCHIVE);
	r_reliefScale =						Cvar_Get("r_reliefScale", "2.0", CVAR_ARCHIVE);
	r_reliefMappingSelfShadow =			Cvar_Get("r_reliefMappingSelfShadow", "1", CVAR_ARCHIVE);
	r_reliefMappingSelfShadowOffset =	Cvar_Get("r_reliefMappingSelfShadowOffset", "0.02", CVAR_ARCHIVE);

	r_shadows =							Cvar_Get("r_shadows", "1", CVAR_DEVELOPER);
	r_playerShadow =					Cvar_Get("r_playerShadow", "1", CVAR_ARCHIVE);

	r_useBlinnPhongLighting =			Cvar_Get("r_useBlinnPhongLighting", "0", CVAR_ARCHIVE);
	r_useBlinnPhongLighting->help =		"use old lighting model";

	r_skipStaticLights =				Cvar_Get("r_skipStaticLights", "0", CVAR_DEVELOPER);
	r_lightmapScale =					Cvar_Get("r_lightmapScale", "0.5", CVAR_ARCHIVE);
	r_useLightScissors = 				Cvar_Get("r_useLightScissors", "1", 0);
	r_useDepthBounds =					Cvar_Get("r_useDepthBounds", "1", 0);
	r_tbnSmoothAngle =					Cvar_Get("r_tbnSmoothAngle", "45", CVAR_ARCHIVE);
	r_lightsWeldThreshold =				Cvar_Get("r_lightsWeldThreshold", "40", CVAR_ARCHIVE);
	r_debugLights =						Cvar_Get("r_debugLights", "0", 0);
	r_specularScale =					Cvar_Get("r_specularScale", "1", CVAR_ARCHIVE);
	r_ambientSpecularScale =			Cvar_Get("r_ambientSpecularScale", "0.3", CVAR_ARCHIVE);
	r_useRadiosityBump =				Cvar_Get("r_useRadiosityBump", "1", CVAR_ARCHIVE);
	r_zNear =							Cvar_Get("r_zNear", "3", CVAR_ARCHIVE);
	r_zFar =							Cvar_Get("r_zFar", "4096", CVAR_ARCHIVE);
	r_useLightOcclusions =				Cvar_Get("r_useLightOcclusions", "1", CVAR_ARCHIVE);

	r_bloom =							Cvar_Get("r_bloom", "1", CVAR_ARCHIVE);
	r_bloomThreshold =					Cvar_Get("r_bloomThreshold", "0.65", CVAR_ARCHIVE);
	r_bloomIntens =						Cvar_Get("r_bloomIntens", "1.0", CVAR_ARCHIVE);
	r_bloomWidth =						Cvar_Get("r_bloomWidth", "3.0", CVAR_ARCHIVE);

	r_ssao =							Cvar_Get ("r_ssao", "1", CVAR_ARCHIVE);
	r_ssaoIntensity =					Cvar_Get ("r_ssaoIntensity", "2.0", CVAR_ARCHIVE);
	r_ssaoScale =						Cvar_Get ("r_ssaoScale", "80.0", CVAR_ARCHIVE);
	r_ssaoBlur	=						Cvar_Get ("r_ssaoBlur", "2", CVAR_ARCHIVE);

	r_dof =								Cvar_Get("r_dof", "1", CVAR_ARCHIVE);
	r_dofBias =							Cvar_Get("r_dofBias", "0.002", CVAR_ARCHIVE);
	r_dofFocus =						Cvar_Get("r_dofFocus", "0.0", CVAR_ARCHIVE);

	r_motionBlur =						Cvar_Get("r_motionBlur", "1", CVAR_ARCHIVE);
	r_motionBlurSamples	=				Cvar_Get("r_motionBlurSamples", "32", CVAR_ARCHIVE);
	r_motionBlurFrameLerp =				Cvar_Get("r_motionBlurFrameLerp", "20", CVAR_ARCHIVE);

	r_radialBlur =						Cvar_Get("r_radialBlur", "1", CVAR_ARCHIVE);
	r_radialBlurFov =                   Cvar_Get("r_radialBlurFov", "30", CVAR_ARCHIVE);
	
	r_filmFilter = 						Cvar_Get("r_filmFilter", "0", CVAR_ARCHIVE);
	r_filmFilterNoiseIntens =			Cvar_Get("r_filmFilterNoiseIntens", "0.03", CVAR_ARCHIVE);
	r_filmFilterScratchIntens =			Cvar_Get("r_filmFilterScratchIntens", "0.4", CVAR_ARCHIVE);
	r_filmFilterVignetIntens =			Cvar_Get("r_filmFilterVignetIntens", "0.35", CVAR_ARCHIVE);

	r_glDebugOutput =					Cvar_Get("r_glDebugOutput", "0", 0);
	r_glMajorVersion =					Cvar_Get("r_glMajorVersion", "4", CVAR_ARCHIVE);
	r_glMinorVersion =					Cvar_Get("r_glMinorVersion", "5", CVAR_ARCHIVE);
	r_glCoreProfile =					Cvar_Get("r_glCoreProfile", "0", 0);

	r_lightEditor =						Cvar_Get("r_lightEditor", "0", 0);
	r_cameraSpaceLightMove =			Cvar_Get("r_cameraSpaceLightMove", "0", CVAR_ARCHIVE);

	r_hudLighting =						Cvar_Get("r_hudLighting", "1.5", CVAR_ARCHIVE);
	r_hudLighting->help =				"intensity of hud light pass";
	r_bump2D =							Cvar_Get("r_bump2D", "1", CVAR_ARCHIVE);
	r_bump2D->help =					"draw 2d bumpmaps";

	r_fixFovStrength =					Cvar_Get("r_fixFovStrength", "0", CVAR_ARCHIVE);
	r_fixFovStrength->help =			"0.0 no perspective correction";
	r_fixFovDistroctionRatio =			Cvar_Get("r_fixFovDistroctionRatio", "5", CVAR_ARCHIVE);
	r_fixFovDistroctionRatio->help =	"cylindrical distortion ratio";

	r_screenBlend =						Cvar_Get("r_screenBlend", "1", CVAR_ARCHIVE);

	r_globalFog =						Cvar_Get("r_globalFog", "1", CVAR_ARCHIVE);
	r_globalFogDensity =				Cvar_Get("r_globalFogDensity", "0.025", CVAR_ARCHIVE);
	r_globalFogRed =					Cvar_Get("r_globalFogRed", "1.0", CVAR_ARCHIVE);
	r_globalFogGreen =					Cvar_Get("r_globalFogGreen", "0.25", CVAR_ARCHIVE);
	r_globalFogBlue =					Cvar_Get("r_globalFogBlue", "0.25", CVAR_ARCHIVE);
	r_globalFogBias =					Cvar_Get("r_globalFogBias", "0.0", CVAR_ARCHIVE);

	r_globalSkyFogDensity =				Cvar_Get("r_globalSkyFogDensity", "0.01", CVAR_ARCHIVE);
	r_globalSkyFogRed =					Cvar_Get("r_globalSkyFogRed", "1.0", CVAR_ARCHIVE);
	r_globalSkyFogGreen =				Cvar_Get("r_globalSkyFogGreen", "0.25", CVAR_ARCHIVE);
	r_globalSkyFogBlue =				Cvar_Get("r_globalSkyFogBlue", "0.25", CVAR_ARCHIVE);
	r_globalSkyFogBias =				Cvar_Get("r_globalSkyFogBias", "0.0", CVAR_ARCHIVE);

	r_useShaderCache =					Cvar_Get("r_useShaderCache", "0", CVAR_ARCHIVE);
	r_particlesOverdraw =				Cvar_Get("r_particlesOverdraw", "1", 0);

	r_lutId =							Cvar_Get("r_lutId", "0", CVAR_ARCHIVE);
	r_colorTempK =						Cvar_Get("r_colorTempK", "6500", CVAR_ARCHIVE);
	r_colorTempK->help =				"Color Temperature in Kelvins (from 1000K to 40000K)";

	Cmd_AddCommand("imagelist",			GL_ImageList_f);
	Cmd_AddCommand("screenshot",		GL_ScreenShot_f);
	Cmd_AddCommand("levelshot",			GL_LevelShot_f);
	Cmd_AddCommand("modellist",			Mod_Modellist_f);
	Cmd_AddCommand("openglInfo",		GL_Strings_f);
	Cmd_AddCommand("autoLightsStats",	AutoLightsStatsList_f);
	Cmd_AddCommand("dumpEntityString",	Dump_EntityString);
	Cmd_AddCommand("glslInfo",			R_ListPrograms_f);
	Cmd_AddCommand("r_meminfo",			R_VideoInfo_f);
	Cmd_AddCommand("low_spec",			R_LowSpecMachine_f);
	Cmd_AddCommand("medium_spec",		R_MediumSpecMachine_f);
	Cmd_AddCommand("hi_spec",			R_HiSpecMachine_f);
	Cmd_AddCommand("glsl",				R_GLSLinfo_f);
	Cmd_AddCommand("saveFogScript",		R_SaveFogScript_f);
	Cmd_AddCommand("removeFogScript",	R_RemoveFogScript_f);
	Cmd_AddCommand("makeLut",			Cube2Lut_f);
	

#ifdef _WIN32
	Cmd_AddCommand("gpuInfo",			R_GpuInfo_f);
#endif

/*
bind INS		"spawnLight"
bind HOME		"spawnLightToCamera"
bind END		"saveLights"
bind DEL		"removeLight"
bind LEFTARROW	"moveLight_right   -1"
bind RIGHTARROW "moveLight_right    1"
bind UPARROW	"moveLight_forward  1"
bind DOWNARROW	"moveLight_forward -1"
bind PGUP		"moveLight_z        1"
bind PGDN		"moveLight_z       -1"
bind KP_MINUS	"changeLightRadius -5"
bind KP_PLUS	"changeLightRadius  5"
bind KP_INS		"copyLight"
bind KP_DEL		"unselectLight"
bind c			"copy"
bind v			"paste"
*/

	Cmd_AddCommand("saveLights",				R_SaveLights_f);
	Cmd_AddCommand("spawnLight",				R_Light_Spawn_f);
	Cmd_AddCommand("spawnLightToCamera",		R_Light_SpawnToCamera_f);
	Cmd_AddCommand("removeLight",				R_Light_Delete_f);
	Cmd_AddCommand("editLight",					R_EditSelectedLight_f);
	Cmd_AddCommand("moveLight_right",			R_MoveLightToRight_f);
	Cmd_AddCommand("moveLight_forward",			R_MoveLightForward_f);
	Cmd_AddCommand("moveLight_z",				R_MoveLightUpDown_f);
	Cmd_AddCommand("changeLightRadius",			R_ChangeLightRadius_f);
	Cmd_AddCommand("cloneLight",				R_Light_Clone_f);
	Cmd_AddCommand("changeLightCone",			R_ChangeLightCone_f);
	Cmd_AddCommand("clearWorldLights",          R_ClearWorldLights);
	Cmd_AddCommand("unselectLight",				R_Light_UnSelect_f);
	Cmd_AddCommand("editFlare",					R_FlareEdit_f);
	Cmd_AddCommand("resetFlarePos",				R_ResetFlarePos_f);
	Cmd_AddCommand("copy",						R_Copy_Light_Properties_f);
	Cmd_AddCommand("paste",						R_Paste_Light_Properties_f);
}

/*
==================
R_SetMode
==================
*/
qboolean R_SetMode(void)
{
	rserr_t err;
	const qboolean fullscreen = (qboolean)r_fullScreen->integer;

	r_fullScreen->modified = qfalse;
	r_mode->modified = qfalse;

    err = GLimp_SetMode(&vid.width, &vid.height, r_mode->integer, fullscreen);

    // success, update variables
	if (err == rserr_ok) {
        Cvar_SetValue("r_fullScreen", gl_state.fullscreen);
        r_fullScreen->modified = qfalse;
		gl_state.prev_mode = r_mode->integer;
        return qtrue;

    // try without fullscreen
	} else if (err == rserr_invalid_fullscreen) {
        Com_Printf(S_COLOR_RED "ref_xpgl::R_SetMode() - fullscreen unavailable in this mode\n");
        if ((err = GLimp_SetMode(&vid.width, &vid.height, r_mode->integer, qfalse)) == rserr_ok) {
            Cvar_SetValue("r_fullScreen", 0);
            r_fullScreen->modified = qfalse;
            gl_state.prev_mode = r_mode->integer;
            return qtrue;
        }

    } else if (err == rserr_invalid_mode) {
        Com_Printf(S_COLOR_RED"ref_xpgl::R_SetMode() - invalid mode\n");
    }

    // revert to previous mode
    if (GLimp_SetMode(&vid.width, &vid.height, gl_state.prev_mode, qfalse) == rserr_ok) {
        Cvar_SetValue("r_mode", gl_state.prev_mode);
        r_mode->modified = qfalse;
        Cvar_SetValue("r_fullScreen", 0);
        r_fullScreen->modified = qfalse;
        return qtrue;
    } else {
        Com_Printf(S_COLOR_RED"ref_xpgl::R_SetMode() - could not revert to safe mode\n");
        return qfalse;
    }
}

static void DevIL_Init() {
    static qboolean init = qfalse;

    if (init)
        return;

	Com_Printf ("\n==="S_COLOR_YELLOW"OpenIL library initiation..."S_COLOR_WHITE"===\n\n");
	
	ilInit();
	iluInit();
	ilutInit();

	ilutRenderer(ILUT_OPENGL);
	ilEnable(IL_ORIGIN_SET);
	ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);

	Com_Printf ("OpenIL VENDOR: "S_COLOR_GREEN" %s\n", ilGetString(IL_VENDOR));
	Com_Printf ("OpenIL Version: "S_COLOR_GREEN"%i\n", ilGetInteger(IL_VERSION_NUM));
	Com_Printf ("\n==================================\n\n");

    init = qtrue;
}


/*
===============
R_Init
===============
*/
qboolean IsExtensionSupported(const char *name)
{
	int			i;
	GLint		n = 0;
	const char	*extension;

	qglGetIntegerv(GL_NUM_EXTENSIONS, &n);
	
	for (i = 0; i<n; i++){
		extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (!strcmp(name, extension))
			return qtrue;		
	}
	return qfalse;
}

void CreateSkyFboMask(void);

void R_InitFboBuffers() {
	
	Com_Printf("Initializing FBOs...\n\n");

	CreateFboBuffer();
	CreateSSAOBuffer();
	CreateSkyFboMask();
	Com_Printf("\n");
}

int R_Init(void *hinstance, void *hWnd)
{
	int	aniso_level, max_aniso;

	Draw_GetPalette();
	R_RegisterCvars();

	// initialize our QGL dynamic bindings
	if (!QGL_Init()) {
		QGL_Shutdown();
		Com_Printf(S_COLOR_RED "quake2xp::R_Init() - could not load opengl32.dll");
		return -1;
	}
	// initialize OS-specific parts of OpenGL
	if (!GLimp_Init(hinstance, hWnd)) {
		QGL_Shutdown();
		return -1;
	}
	// set our "safe" modes
	gl_state.prev_mode = 0;

    // initialize IL library
    DevIL_Init();

	// create the window and set up the context
	if (!R_SetMode()) {
		QGL_Shutdown();
		Com_Printf(S_COLOR_RED "ref_xpgl::R_Init() - could not R_SetMode()\n");
		return -1;
	}

	VID_MenuInit();

	Con_Printf(PRINT_ALL,"\n");

	// Get Extension string
	glGetStringi = (PFNGLGETSTRINGIPROC)			qwglGetProcAddress("glGetStringi");

	// Multitexture stuff
	qglActiveTexture = (PFNGLACTIVETEXTUREPROC)qwglGetProcAddress("glActiveTexture");

	// Separated Stencil 
	qglStencilFuncSeparate	= (PFNGLSTENCILFUNCSEPARATEPROC)	qwglGetProcAddress("glStencilFuncSeparate");
	qglStencilOpSeparate	= (PFNGLSTENCILOPSEPARATEPROC)		qwglGetProcAddress("glStencilOpSeparate");
	qglStencilMaskSeparate	= (PFNGLSTENCILMASKSEPARATEPROC)	qwglGetProcAddress("glStencilMaskSeparate");

	// debug context
	glDebugMessageControlARB	= (PFNGLDEBUGMESSAGECONTROLARBPROC)		qwglGetProcAddress("glDebugMessageControlARB");
	glDebugMessageInsertARB		= (PFNGLDEBUGMESSAGEINSERTARBPROC)		qwglGetProcAddress("glDebugMessageInsertARB");
	glDebugMessageCallbackARB	= (PFNGLDEBUGMESSAGECALLBACKARBPROC)	qwglGetProcAddress("glDebugMessageCallbackARB");
	glGetDebugMessageLogARB		= (PFNGLGETDEBUGMESSAGELOGARBPROC)		qwglGetProcAddress("glGetDebugMessageLogARB");
	
	// vao stuff
	glGenVertexArrays		= (PFNGLGENVERTEXARRAYSPROC)	qwglGetProcAddress("glGenVertexArrays");
	glDeleteVertexArrays	= (PFNGLDELETEVERTEXARRAYSPROC)	qwglGetProcAddress("glDeleteVertexArrays");
	glBindVertexArray		= (PFNGLBINDVERTEXARRAYPROC)	qwglGetProcAddress("glBindVertexArray");

	glMultiDrawElements		= (PFNGLMULTIDRAWELEMENTSPROC)	qwglGetProcAddress("glMultiDrawElements");
	glMultiDrawArrays		= (PFNGLMULTIDRAWARRAYSPROC)	qwglGetProcAddress("glMultiDrawArrays");

	// vbo stuff
	qglBindBuffer		= (PFNGLBINDBUFFERPROC)		qwglGetProcAddress("glBindBuffer");
	qglDeleteBuffers	= (PFNGLDELETEBUFFERSPROC)	qwglGetProcAddress("glDeleteBuffers");
	qglGenBuffers		= (PFNGLGENBUFFERSPROC)		qwglGetProcAddress("glGenBuffers");
	qglBufferData		= (PFNGLBUFFERDATAPROC)		qwglGetProcAddress("glBufferData");
	qglBufferSubData	= (PFNGLBUFFERSUBDATAPROC)	qwglGetProcAddress("glBufferSubData");
	qglMapBuffer		= (PFNGLMAPBUFFERPROC)		qwglGetProcAddress("glMapBuffer");
	qglUnmapBuffer		= (PFNGLUNMAPBUFFERPROC)	qwglGetProcAddress("glUnmapBuffer");
	qglMapBufferRange	= (PFNGLMAPBUFFERRANGEPROC)	qwglGetProcAddress("glMapBufferRange");

	// fbo stuff
	qglIsRenderbuffer						= (PFNGLISRENDERBUFFERPROC)						qwglGetProcAddress("glIsRenderbuffer");
	qglBindRenderbuffer						= (PFNGLBINDRENDERBUFFERPROC)					qwglGetProcAddress("glBindRenderbuffer");
	qglDeleteRenderbuffers					= (PFNGLDELETERENDERBUFFERSPROC)				qwglGetProcAddress("glDeleteRenderbuffers");
	qglGenRenderbuffers						= (PFNGLGENRENDERBUFFERSPROC)					qwglGetProcAddress("glGenRenderbuffers");
	qglRenderbufferStorage					= (PFNGLRENDERBUFFERSTORAGEPROC)				qwglGetProcAddress("glRenderbufferStorage");
	qglGetRenderbufferParameteriv			= (PFNGLGETRENDERBUFFERPARAMETERIVPROC)			qwglGetProcAddress("glGetRenderbufferParameteriv");
	qglRenderbufferStorageMultisample		= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)		qwglGetProcAddress("glRenderbufferStorageMultisample");

	qglIsFramebuffer						= (PFNGLISFRAMEBUFFERPROC)						qwglGetProcAddress("glIsFramebuffer");
	qglBindFramebuffer						= (PFNGLBINDFRAMEBUFFERPROC)					qwglGetProcAddress("glBindFramebuffer");
	qglDeleteFramebuffers					= (PFNGLDELETEFRAMEBUFFERSPROC)					qwglGetProcAddress("glDeleteFramebuffers");
	qglGenFramebuffers						= (PFNGLGENFRAMEBUFFERSPROC)					qwglGetProcAddress("glGenFramebuffers");
	qglCheckFramebufferStatus				= (PFNGLCHECKFRAMEBUFFERSTATUSPROC)				qwglGetProcAddress("glCheckFramebufferStatus");
	qglFramebufferTexture1D					= (PFNGLFRAMEBUFFERTEXTURE1DPROC)				qwglGetProcAddress("glFramebufferTexture1D");
	qglFramebufferTexture2D					= (PFNGLFRAMEBUFFERTEXTURE2DPROC)				qwglGetProcAddress("glFramebufferTexture2D");
	qglFramebufferTexture3D					= (PFNGLFRAMEBUFFERTEXTURE3DPROC)				qwglGetProcAddress("glFramebufferTexture3D");
	qglFramebufferRenderbuffer				= (PFNGLFRAMEBUFFERRENDERBUFFERPROC)			qwglGetProcAddress("glFramebufferRenderbuffer");
	qglGenerateMipmap						= (PFNGLGENERATEMIPMAPPROC)						qwglGetProcAddress("glGenerateMipmap");
	qglBlitFramebuffer						= (PFNGLBLITFRAMEBUFFERPROC)					qwglGetProcAddress("glBlitFramebuffer");
	qglGetFramebufferAttachmentParameteriv	= (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)qwglGetProcAddress("glGetFramebufferAttachmentParameteriv");
	qglDrawBuffers							= (PFNGLDRAWBUFFERSPROC)						qwglGetProcAddress("glDrawBuffers");
	glCopyImageSubData						= (PFNGLCOPYIMAGESUBDATAPROC)					qwglGetProcAddress("glCopyImageSubData");

	// bindless textures stuff
	glGetTextureHandleARB				= (PFNGLGETTEXTUREHANDLEARBPROC)			qwglGetProcAddress("glGetTextureHandleARB");
	glGetTextureSamplerHandleARB		= (PFNGLGETTEXTURESAMPLERHANDLEARBPROC)		qwglGetProcAddress("glGetTextureSamplerHandleARB");
	glMakeTextureHandleResidentARB		= (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)	qwglGetProcAddress("glMakeTextureHandleResidentARB");
	glMakeTextureHandleNonResidentARB	= (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)	qwglGetProcAddress("glMakeTextureHandleNonResidentARB");
	glGetImageHandleARB					= (PFNGLGETIMAGEHANDLEARBPROC)				qwglGetProcAddress("glGetImageHandleARB");
	glMakeImageHandleResidentARB		= (PFNGLMAKEIMAGEHANDLERESIDENTARBPROC)		qwglGetProcAddress("glMakeImageHandleResidentARB");
	glMakeImageHandleNonResidentARB		= (PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC)	qwglGetProcAddress("glMakeImageHandleNonResidentARB");
	glUniformHandleui64ARB				= (PFNGLUNIFORMHANDLEUI64ARBPROC)			qwglGetProcAddress("glUniformHandleui64ARB");
	glUniformHandleui64vARB				= (PFNGLUNIFORMHANDLEUI64VARBPROC)			qwglGetProcAddress("glUniformHandleui64vARB");
	glProgramUniformHandleui64ARB		= (PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC)	qwglGetProcAddress("glProgramUniformHandleui64ARB");
	glProgramUniformHandleui64vARB		= (PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC)	qwglGetProcAddress("glProgramUniformHandleui64vARB");
	glIsTextureHandleResidentARB		= (PFNGLISTEXTUREHANDLERESIDENTARBPROC)		qwglGetProcAddress("glIsTextureHandleResidentARB");
	glIsImageHandleResidentARB			= (PFNGLISIMAGEHANDLERESIDENTARBPROC)		qwglGetProcAddress("glIsImageHandleResidentARB");
	glVertexAttribL1ui64ARB				= (PFNGLVERTEXATTRIBL1UI64ARBPROC)			qwglGetProcAddress("glVertexAttribL1ui64ARB");
	glVertexAttribL1ui64vARB			= (PFNGLVERTEXATTRIBL1UI64VARBPROC)			qwglGetProcAddress("glVertexAttribL1ui64vARB");
	glGetVertexAttribLui64vARB			= (PFNGLGETVERTEXATTRIBLUI64VARBPROC)		qwglGetProcAddress("glGetVertexAttribLui64vARB");

	// glsl stuff
	qglCreateShader =				(PFNGLCREATESHADERPROC)				qwglGetProcAddress("glCreateShader");
	qglCreateProgram =				(PFNGLCREATEPROGRAMPROC)			qwglGetProcAddress("glCreateProgram");
	qglDeleteShader =				(PFNGLDELETESHADERPROC)				qwglGetProcAddress("glDeleteShader");
	qglDeleteProgram =				(PFNGLDELETEPROGRAMPROC)			qwglGetProcAddress("glDeleteProgram");
	qglGetShaderiv =				(PFNGLGETSHADERIVPROC)				qwglGetProcAddress("glGetShaderiv");
	qglGetProgramiv =				(PFNGLGETPROGRAMIVPROC)				qwglGetProcAddress("glGetProgramiv");
	qglGetShaderInfoLog =			(PFNGLGETSHADERINFOLOGPROC)			qwglGetProcAddress("glGetShaderInfoLog");
	qglGetProgramInfoLog =			(PFNGLGETPROGRAMINFOLOGPROC)		qwglGetProcAddress("glGetProgramInfoLog");
	qglShaderSource =				(PFNGLSHADERSOURCEPROC)				qwglGetProcAddress("glShaderSource");
	qglCompileShader =				(PFNGLCOMPILESHADERPROC)			qwglGetProcAddress("glCompileShader");
	qglAttachShader =				(PFNGLATTACHSHADERPROC)				qwglGetProcAddress("glAttachShader");
	qglDetachShader =				(PFNGLDETACHSHADERPROC)				qwglGetProcAddress("glDetachShader");
	qglLinkProgram =				(PFNGLLINKPROGRAMPROC)				qwglGetProcAddress("glLinkProgram");
	qglUseProgram =					(PFNGLUSEPROGRAMPROC)				qwglGetProcAddress("glUseProgram");
	qglVertexAttribPointer =		(PFNGLVERTEXATTRIBPOINTERPROC)				qwglGetProcAddress("glVertexAttribPointer");
	qglEnableVertexAttribArray =	(PFNGLENABLEVERTEXATTRIBARRAYPROC)		qwglGetProcAddress("glEnableVertexAttribArray");
	qglDisableVertexAttribArray =	(PFNGLDISABLEVERTEXATTRIBARRAYPROC)		qwglGetProcAddress("glDisableVertexAttribArray");
	qglBindAttribLocation =			(PFNGLBINDATTRIBLOCATIONPROC)			qwglGetProcAddress("glBindAttribLocation");
	qglGetAttribLocation =			(PFNGLGETATTRIBLOCATIONPROC)				qwglGetProcAddress("glGetAttribLocation");
	qglGetActiveUniform =			(PFNGLGETACTIVEUNIFORMPROC)					qwglGetProcAddress("glGetActiveUniform");
	qglGetUniformLocation =			(PFNGLGETUNIFORMLOCATIONPROC)				qwglGetProcAddress("glGetUniformLocation");
	qglUniform1f =					(PFNGLUNIFORM1FPROC)				qwglGetProcAddress("glUniform1f");
	qglUniform2f =					(PFNGLUNIFORM2FPROC)				qwglGetProcAddress("glUniform2f");
	qglUniform3f =					(PFNGLUNIFORM3FPROC)				qwglGetProcAddress("glUniform3f");
	qglUniform4f =					(PFNGLUNIFORM4FPROC)				qwglGetProcAddress("glUniform4f");
	qglUniform1i =					(PFNGLUNIFORM1IPROC)				qwglGetProcAddress("glUniform1i");
	qglUniform2i =					(PFNGLUNIFORM2IPROC)				qwglGetProcAddress("glUniform2i");
	qglUniform3i =					(PFNGLUNIFORM3IPROC)				qwglGetProcAddress("glUniform3i");
	qglUniform4i =					(PFNGLUNIFORM4IPROC)				qwglGetProcAddress("glUniform4i");
	qglUniform1fv =					(PFNGLUNIFORM1FVPROC)				qwglGetProcAddress("glUniform1fv");
	qglUniform2fv =					(PFNGLUNIFORM2FVPROC)				qwglGetProcAddress("glUniform2fv");
	qglUniform3fv =					(PFNGLUNIFORM3FVPROC)				qwglGetProcAddress("glUniform3fv");
	qglUniform4fv =					(PFNGLUNIFORM4FVPROC)				qwglGetProcAddress("glUniform4fv");
	qglUniform1iv =					(PFNGLUNIFORM1IVPROC)				qwglGetProcAddress("glUniform1iv");
	qglUniform2iv =					(PFNGLUNIFORM2IVPROC)				qwglGetProcAddress("glUniform2iv");
	qglUniform3iv =					(PFNGLUNIFORM3IVPROC)				qwglGetProcAddress("glUniform3iv");
	qglUniform4iv =					(PFNGLUNIFORM4IVPROC)				qwglGetProcAddress("glUniform4iv");
	qglUniformMatrix2fv =			(PFNGLUNIFORMMATRIX2FVPROC)			qwglGetProcAddress("glUniformMatrix2fv");
	qglUniformMatrix3fv =			(PFNGLUNIFORMMATRIX3FVPROC)         qwglGetProcAddress("glUniformMatrix3fv");
	qglUniformMatrix4fv =			(PFNGLUNIFORMMATRIX4FVPROC)			qwglGetProcAddress("glUniformMatrix4fv");

	// DSA stuff glsl block
	glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)					qwglGetProcAddress("glProgramUniform1f");
	glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)					qwglGetProcAddress("glProgramUniform2f");
	glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)					qwglGetProcAddress("glProgramUniform3f");
	glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)					qwglGetProcAddress("glProgramUniform4f");
	glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)					qwglGetProcAddress("glProgramUniform1i");
	glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)					qwglGetProcAddress("glProgramUniform2i");
	glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)					qwglGetProcAddress("glProgramUniform3i");
	glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)					qwglGetProcAddress("glProgramUniform4i");

	glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)					qwglGetProcAddress("glProgramUniform1fv");
	glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)					qwglGetProcAddress("glProgramUniform2fv");
	glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)					qwglGetProcAddress("glProgramUniform3fv");
	glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)					qwglGetProcAddress("glProgramUniform4fv");
	glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)					qwglGetProcAddress("glProgramUniform1iv");
	glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)					qwglGetProcAddress("glProgramUniform2iv");
	glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)					qwglGetProcAddress("glProgramUniform3iv");
	glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)					qwglGetProcAddress("glProgramUniform4iv");

	glProgramUniformMatrix2fv =		(PFNGLPROGRAMUNIFORMMATRIX2FVPROC)	qwglGetProcAddress("glProgramUniformMatrix2fv");
	glProgramUniformMatrix3fv =		(PFNGLPROGRAMUNIFORMMATRIX3FVPROC)	qwglGetProcAddress("glProgramUniformMatrix3fv");
	glProgramUniformMatrix4fv =		(PFNGLPROGRAMUNIFORMMATRIX4FVPROC)	qwglGetProcAddress("glProgramUniformMatrix4fv");

	glProgramUniformMatrix2x3fv =	(PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)	qwglGetProcAddress("glProgramUniformMatrix2x3fv");
	glProgramUniformMatrix3x2fv =	(PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)	qwglGetProcAddress("glProgramUniformMatrix3x2fv");
	glProgramUniformMatrix2x4fv =	(PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)	qwglGetProcAddress("glProgramUniformMatrix2x4fv");
	glProgramUniformMatrix4x2fv =	(PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)	qwglGetProcAddress("glProgramUniformMatrix4x2fv");
	glProgramUniformMatrix3x4fv =	(PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)	qwglGetProcAddress("glProgramUniformMatrix3x4fv");
	glProgramUniformMatrix4x3fv =	(PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)	qwglGetProcAddress("glProgramUniformMatrix4x3fv");
	
	glBindTextureUnit =			(PFNGLBINDTEXTUREUNITPROC)		qwglGetProcAddress("glBindTextureUnit");
	glBindTextures =			(PFNGLBINDTEXTURESPROC)			qwglGetProcAddress("glBindTextures");

	glGenQueries		= (PFNGLGENQUERIESPROC)			qwglGetProcAddress("glGenQueries");
	glDeleteQueries		= (PFNGLDELETEQUERIESPROC)		qwglGetProcAddress("glDeleteQueries");
	glIsQuery			= (PFNGLISQUERYPROC)			qwglGetProcAddress("glIsQuery");
	glBeginQuery		= (PFNGLBEGINQUERYPROC)			qwglGetProcAddress("glBeginQuery");
	glEndQuery			= (PFNGLENDQUERYPROC)			qwglGetProcAddress("glEndQuery");
	glGetQueryiv		= (PFNGLGETQUERYIVPROC)			qwglGetProcAddress("glGetQueryiv");
	glGetQueryObjectiv	= (PFNGLGETQUERYOBJECTIVPROC)	qwglGetProcAddress("glGetQueryObjectiv");
	glGetQueryObjectuiv	= (PFNGLGETQUERYOBJECTUIVPROC)	qwglGetProcAddress("glGetQueryObjectuiv");

	glGetProgramBinary =	(PFNGLGETPROGRAMBINARYPROC)		qwglGetProcAddress("glGetProgramBinary");
	glProgramBinary =		(PFNGLPROGRAMBINARYPROC)		qwglGetProcAddress("glProgramBinary");
	glProgramParameteri =	(PFNGLPROGRAMPARAMETERIPROC)	qwglGetProcAddress("glProgramParameteri");
	qglGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &gl_state.numFormats);
	qglGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &gl_state.binaryFormats);

	qglTexImage3D =	(PFNGLTEXIMAGE3DPROC)	qwglGetProcAddress("glTexImage3D");					

	gl_config.vendor_string					= (const char*)qglGetString(GL_VENDOR);
	gl_config.renderer_string				= (const char*)qglGetString(GL_RENDERER);
	gl_config.version_string				= (const char*)qglGetString(GL_VERSION);
	gl_config.shadingLanguageVersionString	= (const char*)qglGetString(GL_SHADING_LANGUAGE_VERSION);

	Com_Printf("GL_VENDOR:" S_COLOR_GREEN "    %s\n", gl_config.vendor_string);
	Com_Printf("GL_RENDERER:" S_COLOR_GREEN "  %s\n", gl_config.renderer_string);
	Com_Printf("GL_VERSION:" S_COLOR_GREEN "   %s\n", gl_config.version_string);
	Com_Printf("GLSL_VERSION:" S_COLOR_GREEN " %s\n", gl_config.shadingLanguageVersionString);

	
	qglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,	&gl_config.maxFragmentUniformComponents);
	qglGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,	&gl_config.maxVertexUniformComponents);
	qglGetIntegerv(GL_MAX_VARYING_FLOATS,				&gl_config.maxVaryingFloats);
	qglGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,	&gl_config.maxVertexTextureImageUnits);
	qglGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_config.maxCombinedTextureImageUnits);
	qglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,	&gl_config.maxFragmentUniformComponents);
	qglGetIntegerv(GL_MAX_VERTEX_ATTRIBS,				&gl_config.maxVertexAttribs);
	qglGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,			&gl_config.maxTextureImageUnits);
	qglGetIntegerv(GL_MAX_UNIFORM_LOCATIONS,			&gl_config.maxUniformLocations);
	qglGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,			&gl_state.maxRenderBufferSize);
	qglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS,			&gl_state.maxColorAttachments);
	qglGetIntegerv(GL_MAX_SAMPLES,						&gl_state.maxSamples);

	Com_Printf("\n");
	Com_Printf(S_COLOR_YELLOW"Max Fragment Uniform Components:"S_COLOR_GREEN" %i\n", gl_config.maxFragmentUniformComponents);
	Com_Printf(S_COLOR_YELLOW"Max Vertex Uniform Components: "S_COLOR_GREEN"  %i\n", gl_config.maxVertexUniformComponents);
	Com_Printf(S_COLOR_YELLOW"Max Uniform Locations:"S_COLOR_GREEN"           %i\n", gl_config.maxUniformLocations);
	Com_Printf(S_COLOR_YELLOW"Max Vertex Attribs:           "S_COLOR_GREEN"   %i\n", gl_config.maxVertexAttribs);
	Com_Printf(S_COLOR_YELLOW"Max Varying Floats:           "S_COLOR_GREEN"   %i\n", gl_config.maxVaryingFloats);
	Com_Printf(S_COLOR_YELLOW"Max Vertex TextureImageUnits: "S_COLOR_GREEN"   %i\n", gl_config.maxVertexTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"Max Texture ImageUnits:       "S_COLOR_GREEN"   %i\n", gl_config.maxTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"Max Combined TextureImageUnits: "S_COLOR_GREEN" %i\n", gl_config.maxCombinedTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"Max Fragment UniformComponents: "S_COLOR_GREEN" %i\n", gl_config.maxFragmentUniformComponents);

	Com_Printf(S_COLOR_YELLOW"Max Render Buffer Size:  "S_COLOR_GREEN"        %i\n", gl_state.maxRenderBufferSize);
	Com_Printf(S_COLOR_YELLOW"Max Color Attachments:   "S_COLOR_GREEN"        %i\n", gl_state.maxColorAttachments);
	Com_Printf(S_COLOR_YELLOW"Max Buffer Samples:      "S_COLOR_GREEN"        %i\n", gl_state.maxSamples);

	R_InitPrograms();
	R_InitFboBuffers();
	R_InitVertexBuffers();

	qglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);

	Cvar_SetValue("r_maxAnisotropy", max_aniso);
	if (r_anisotropic->integer >= r_maxAnisotropy->integer)
		Cvar_SetValue("r_anisotropic", r_maxAnisotropy->integer);

	aniso_level = r_anisotropic->integer;
	if (r_anisotropic->integer <= 1) {
		r_anisotropic = Cvar_Set("r_anisotropic", "1");
		Com_Printf(S_COLOR_YELLOW"...ignoring GL_ARB_texture_filter_anisotropic\n");
	}
	else {
		Com_Printf("...using GL_ARB_texture_filter_anisotropic\n   ["S_COLOR_GREEN"%i"S_COLOR_WHITE" max] ["S_COLOR_GREEN"%i" S_COLOR_WHITE" selected]\n",
			max_aniso, aniso_level);
	}

	gl_state.texture_compression_bptc = qfalse;
	if (IsExtensionSupported("GL_ARB_texture_compression_bptc"))
		if (!r_textureCompression->integer) {
			Com_Printf(S_COLOR_YELLOW"...ignoring GL_ARB_texture_compression_bptc\n");
			gl_state.texture_compression_bptc = qfalse;
		}
		else {
			Com_Printf("...using GL_ARB_texture_compression_bptc\n");
			gl_state.texture_compression_bptc = qtrue;

		}
	else {
		Com_Printf(S_COLOR_RED"...GL_ARB_texture_compression_bptc not found\n");
		gl_state.texture_compression_bptc = qfalse;
	}

	if (IsExtensionSupported("GL_ARB_seamless_cube_map")) {
		Com_Printf("...using GL_ARB_seamless_cube_map\n");
		qglEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}

	// ===========================================================================================================================

	gl_state.depthBoundsTest = qfalse;
	if (IsExtensionSupported("GL_EXT_depth_bounds_test")) {
		Com_Printf("...using GL_EXT_depth_bounds_test\n");

		glDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)qwglGetProcAddress("glDepthBoundsEXT");
		gl_state.depthBoundsTest = qtrue;
	}
	else {
		Com_Printf(S_COLOR_RED"...GL_EXT_depth_bounds_test not found\n");
		gl_state.depthBoundsTest = qfalse;
	}

	qglEnable(GL_FRAMEBUFFER_SRGB);
	Com_Printf("...using GL_ARB_framebuffer_sRGB\n");

	int queryBits;
	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &queryBits);
	Com_Printf("...found [" S_COLOR_GREEN "%i" S_COLOR_WHITE "] query bits\n", queryBits);

	Com_Printf("=====================================\n");

	GL_SetDefaultState();
	GL_InitImages();
	Mod_Init();
	R_InitEngineTextures();
	R_GenEnvCubeMap();
	R_LoadFont();

	flareEdit = (qboolean)qfalse;
	return 0;
}



/*
===============
R_Shutdown
===============
*/

void R_Shutdown(void)
{
	Cmd_RemoveCommand("modellist");
	Cmd_RemoveCommand("screenshot");
	Cmd_RemoveCommand("levelshot");
	Cmd_RemoveCommand("imagelist");
	Cmd_RemoveCommand("autoLightsStats");
	Cmd_RemoveCommand("dumpEntityString");
	Cmd_RemoveCommand("r_meminfo");	
	Cmd_RemoveCommand("low_spec");
	Cmd_RemoveCommand("medium_spec");
	Cmd_RemoveCommand("hi_spec");
	Cmd_RemoveCommand("gpuInfo");
	
	Cmd_RemoveCommand("saveLights");
	Cmd_RemoveCommand("spawnLight");
	Cmd_RemoveCommand("removeLight");
	Cmd_RemoveCommand("editLight");
	Cmd_RemoveCommand("spawnLightToCamera");
	Cmd_RemoveCommand("changeLightRadius");
	Cmd_RemoveCommand("cloneLight");
	Cmd_RemoveCommand("changeLightCone");
	Cmd_RemoveCommand("clearWorldLights");
	Cmd_RemoveCommand("unselectLight");
	Cmd_RemoveCommand("editFlare");
	Cmd_RemoveCommand("resetFlarePos");
	Cmd_RemoveCommand("copy");
	Cmd_RemoveCommand("paste");
	Cmd_RemoveCommand("moveLight_right");
	Cmd_RemoveCommand("moveLight_forward");
	Cmd_RemoveCommand("moveLight_z");

	Cmd_RemoveCommand("glsl");
	Cmd_RemoveCommand("glslInfo");
	Cmd_RemoveCommand("openglInfo");
	Cmd_RemoveCommand("saveFogScript");
	Cmd_RemoveCommand("removeFogScript");
	Cmd_RemoveCommand("Cube2Lut_f");

	qglDeleteFramebuffers (1, &fboId);
	qglDeleteFramebuffers(1, &fbo_skyMask);

	DeleteShadowVertexBuffers();
	R_ShutDownVertexBuffers();

	Mod_FreeAll();
	GL_ShutdownImages();

	R_ClearWorldLights();
	ilShutDown();
	R_ShutdownPrograms();

	GLimp_Shutdown();
	QGL_Shutdown();
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
static float ClampCvar(float min, float max, float value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static int ClampCvarInteger(int min, int max, int value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

void R_BeginFrame()
{
	
#ifndef _WIN32
    // there is no need to restart video mode with SDL
    if (r_fullScreen->modified) {
        R_SetMode();
		r_fullScreen->modified = qfalse;
    }
#endif

	/* 
	 ** change modes if necessary
	 */
	r_lightmapScale->value = ClampCvar(0.0, 1.0, r_lightmapScale->value);
	r_reliefMapping->integer = ClampCvarInteger(0, 1, r_reliefMapping->integer);
	r_reliefScale->integer = ClampCvarInteger(0, 6, r_reliefScale->integer);

	if (r_mode->modified || r_fullScreen->modified)
        vid_ref->modified = qtrue;
	
	if (r_reliefMappingSelfShadow->modified)
		r_reliefMappingSelfShadow->modified = qfalse;

	if(r_dof->modified)
		r_dof->modified = qfalse;

	if(r_lightmapScale->modified)
		r_lightmapScale->modified = qfalse;

	if (r_ssao->modified)
		r_ssao->modified = qfalse;
	
	if (r_reliefMapping->modified)
		r_reliefMapping->modified = qfalse;


	if (r_textureMode->modified || r_anisotropic->modified || r_textureLodBias->modified) {
		GL_TextureMode(r_textureMode->string);

		if (r_textureMode->modified)
			r_textureMode->modified = qfalse;

		if (r_anisotropic->modified)
			r_anisotropic->modified = qfalse;

		if (r_textureLodBias->modified)
			r_textureLodBias->modified = qfalse;
	}

	//go into 2D mode
	R_SetupOrthoMatrix();

	GL_Enable(GL_BLEND); // alpha blend for chars
	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	qglDrawBuffer( GL_BACK );

	GL_UpdateSwapInterval();

	R_Clear();
}

/*
=============
R_SetPalette
=============
*/
unsigned r_rawpalette[256];

void R_SetPalette(const unsigned char *palette)
{
	int i;

	byte *rp = (byte *) r_rawpalette;
	
	if (palette) {
		for (i = 0; i < 256; i++) {
			rp[i * 4 + 0] = palette[i * 3 + 0];
			rp[i * 4 + 1] = palette[i * 3 + 1];
			rp[i * 4 + 2] = palette[i * 3 + 2];
			rp[i * 4 + 3] = 0xff;
		}
	} else {
		for (i = 0; i < 256; i++) {
			rp[i * 4 + 0] = d_8to24table[i] & 0xff;
			rp[i * 4 + 1] = (d_8to24table[i] >> 8) & 0xff;
			rp[i * 4 + 2] = (d_8to24table[i] >> 16) & 0xff;
			rp[i * 4 + 3] = 0xff;
		}
	}
	qglClearColor(0, 0, 0, 0);
	qglClear(GL_COLOR_BUFFER_BIT);
}

/*
** R_DrawBeam
*/
void R_DrawBeam()
{
	return;
}
