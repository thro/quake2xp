/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2006-2011 Quake2xp Team

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

/*
====================
GLSL Full Screen
Post Process Effects
====================
*/

void R_DrawFullScreenQuad () {

	glBindVertexArray(vao.fullscreenQuad);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
}

void R_DrawHalfScreenQuad () {

	glBindVertexArray(vao.halfScreenQuad);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
}

void R_DrawQuarterScreenQuad () {
	
	glBindVertexArray(vao.quaterScreenQuad);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	
	glBindVertexArray(0);
}

void R_Bloom (void) 
{
	if (!r_bloom->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// downsample and cut color
	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram (bloomdsProgram);
	qglUniform1f(U_PARAM_FLOAT_0, r_bloomThreshold->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// create bloom texture (set to zero in default state)
	if (!bloomtex) {
		qglGenTextures (1, &bloomtex);
		GL_BindRect (bloomtex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE, 0, GL_SRGB8, 0, 0, vid.width*0.25, vid.height*0.25, 0);
	}

	// generate star shape
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (glareProgram);
	qglUniform1f(U_PARAM_FLOAT_0, r_bloomWidth->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	// blur x
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussXProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// blur y
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussYProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	//final pass
	GL_BindProgram (bloomfpProgram);
	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	GL_MBindRect (GL_TEXTURE1, bloomtex);
	qglUniform1f(U_PARAM_FLOAT_0, r_bloomIntens->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();
}



void R_ThermalVision (void) 
{
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!(r_newrefdef.rdflags & RDF_IRGOGGLES))
		return;

	if (!thermaltex) {
		qglGenTextures (1, &thermaltex);
		GL_MBindRect(GL_TEXTURE0, thermaltex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE, 0, GL_SRGB8, 0, 0, vid.width, vid.height, 0);
	}
	else {
		GL_MBindRect(GL_TEXTURE0, thermaltex);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	}

	// setup program
	GL_BindProgram (thermalProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// blur x
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussXProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// blur y
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussYProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	//final pass
	GL_BindProgram (thermalfpProgram);

	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();
}



void R_RadialBlur (void) 
{
	float	blur;

	if (!r_radialBlur->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (r_newrefdef.fov_x <= r_radialBlurFov->value)
		goto hack;

	if (r_newrefdef.rdflags & (RDF_UNDERWATER | RDF_PAIN)) {

	hack:

	
		GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

		// setup program
		GL_BindProgram (radialProgram);

		if (r_newrefdef.rdflags & RDF_UNDERWATER)
			blur = 0.0065;
		else
			blur = 0.01;

		// xy = radial center screen space position, z = radius attenuation, w = blur strength
		qglUniform4f(U_PARAM_VEC4_0, vid.width*0.5, vid.height*0.5, 1.0 / vid.height, blur);
		qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		R_DrawFullScreenQuad ();
	}
}

extern float v_blend[4];

void R_ScreenBlend(void)
{

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (!v_blend[3] || !r_screenBlend->integer)
		return;

		// setup program
		GL_BindProgram(genericProgram);
		qglUniform1i(U_2D_PICS, 0);
		qglUniform1i(U_CONSOLE_BACK, 0);
		qglUniform1i(U_FRAG_COLOR, 1);
		qglUniform4f(U_COLOR, v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
		qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		GL_Disable(GL_ALPHA_TEST);
		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		R_DrawFullScreenQuad();

		GL_Disable(GL_BLEND);
}

void R_DofBlur (void) 
{
	float			tmpDist[5], tmpMins[3];
	vec2_t          dofParams;
	trace_t			trace;
	vec3_t			end_trace, v_f, v_r, v_up, tmp, left, right, up, dn;

	if (!r_dof->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	//dof autofocus
	if (!r_dofFocus->integer) {

		AngleVectors (r_newrefdef.viewangles, v_f, v_r, v_up);
		VectorMA (r_newrefdef.vieworg, 4096, v_f, end_trace);

		VectorMA (end_trace, 96, v_r, right);
		VectorMA (end_trace, -96, v_r, left);
		VectorMA (end_trace, 96, v_up, up);
		VectorMA (end_trace, -96, v_up, dn);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, right, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[0] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, left, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[1] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, up, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[2] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, dn, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[3] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, end_trace, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[4] = VectorLength (tmp);

		tmpMins[0] = min (tmpDist[0], tmpDist[1]);
		tmpMins[1] = min (tmpDist[2], tmpDist[3]);
		tmpMins[2] = min (tmpMins[0], tmpMins[1]);

		dofParams[0] = min (tmpMins[2], tmpDist[4]);
		dofParams[1] = r_dofBias->value;
	}
	else {
		dofParams[0] = r_dofFocus->value;
		dofParams[1] = r_dofBias->value;
	}

	// setup program
	GL_BindProgram (dofProgram);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform4f(U_PARAM_VEC4_0, dofParams[0], dofParams[1], r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	GL_MBindRect			(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	GL_MBindRect			(GL_TEXTURE1, depthMap->texnum);

	R_DrawFullScreenQuad ();
}

void R_FXAA (void) {

	if (!r_fxaa->integer)
		return;

	// setup program
	GL_BindProgram (fxaaProgram);

	if (!fxaatex) {
		qglGenTextures (1, &fxaatex);
		GL_MBind (GL_TEXTURE0, fxaatex);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_2D, 0, GL_SRGB8, 0, 0, vid.width, vid.height, 0);
	}
	GL_MBind (GL_TEXTURE0, fxaatex);
	qglCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();
}

void R_FilmFilter (void) 
{

	if (!r_filmFilter->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	 
	// setup program
	GL_BindProgram (filmGrainProgram);

	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform2f (U_SCREEN_SIZE,	vid.width, vid.height);
	qglUniform1f (U_PARAM_FLOAT_0,	crand());
	qglUniform1i (U_PARAM_INT_0,	r_framecount);
	qglUniform3f (U_PARAM_VEC3_0,	r_filmFilterNoiseIntens->value, r_filmFilterScratchIntens->value, r_filmFilterVignetIntens->value);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();
}

void R_GammaRamp (void) 
{
	
	GL_BindProgram (gammaProgram);

	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform4f (U_COLOR_PARAMS,	r_brightness->value, 
									r_contrast->value, 
									r_saturation->value, 
									1.0 / r_gamma->value);

	qglUniform3f (U_COLOR_VIBRANCE, r_colorBalanceRed->value	* r_colorVibrance->value,
									r_colorBalanceGreen->value	* r_colorVibrance->value, 
									r_colorBalanceBlue->value	* r_colorVibrance->value);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

}

static int ClampCvarInteger(int min, int max, int value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

void R_ColorTemperatureCorrection(void){

	if (r_colorTempK->value < 1000.0)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	GL_BindProgram(lutProgram);

	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1f(U_PARAM_FLOAT_1, r_colorTempK->value);
	qglUniform1i(U_PARAM_INT_0, 1);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

}

void R_lutCorrection(void)
{
	if (!lutCount)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	r_lutId->integer = ClampCvarInteger(0, lutCount - 1, r_lutId->integer);

	int lutID = r_lutId->integer;

	GL_BindProgram(lutProgram);

	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	GL_MBind3d(GL_TEXTURE1, r_3dLut[lutID]->texnum);
	qglUniform3f(U_PARAM_VEC3_0, r_3dLut[lutID]->lutSize, r_3dLut[lutID]->lutSize, r_3dLut[lutID]->lutSize);
	qglUniform1i(U_PARAM_INT_0, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

}

void R_MotionBlur (void) 
{
	vec2_t	angles, delta;
	vec3_t velocity;
	float blur;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_motionBlur->integer)
		return;
	// go to 2d
	R_SetupOrthoMatrix();
	GL_DepthMask(0);

	// calc camera offsets
	angles[0] = r_newrefdef.viewanglesOld[1] - r_newrefdef.viewangles[1]; //YAW left-right
	angles[1] = r_newrefdef.viewanglesOld[0] - r_newrefdef.viewangles[0]; //PITCH up-down
	
	blur = r_motionBlurFrameLerp->value;
	delta[0] = (angles[0] / r_newrefdef.fov_x) * blur;
	delta[1] = (angles[1] / r_newrefdef.fov_y) * blur;
	
	VectorSet(velocity, delta[0], delta[1], 1.0);
	VectorNormalize(velocity);

	// setup program
	GL_BindProgram(motionBlurProgram);

	qglUniform3f(U_PARAM_VEC3_0, velocity[0], velocity[1], r_motionBlurSamples->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	R_DrawFullScreenQuad();

	// restore 3d
	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);
	GL_DepthMask(1);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
		r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_DownsampleDepth(void) 
{
	if (!r_ssao->integer)
		return;

	GL_DepthRange(0.0, 1.0);
	// downsample the depth buffer
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT2);

	GL_BindProgram(depthDownsampleProgram);
	GL_MBindRect(GL_TEXTURE0, depthMap->texnum);

	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// restore settings
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void R_SSAO (void) 
{
	int i, j, numSamples;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

	if (!r_ssao->integer)
		return;
	
	R_SetupOrthoMatrix();
	GL_DepthMask(0);

	R_DownsampleDepth();

	// process
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_BindProgram (ssaoProgram);
	GL_MBindRect(GL_TEXTURE0, fboDN->texnum);
	GL_MBind(GL_TEXTURE1, r_randomNormalTex->texnum);

	qglUniform2f (U_PARAM_VEC2_0, max(r_ssaoIntensity->value, 0.f), r_ssaoScale->value);
	qglUniform2f (U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// blur
	fboColorIndex = 0;

	if (r_ssaoBlur->integer) {
		qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
		GL_MBindRect(GL_TEXTURE1, fboDN->texnum);

		GL_BindProgram(ssaoBlurProgram);

		qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		numSamples = (int)rintf(4.f * vid.height / 1080.f);
		qglUniform1i(U_PARAM_INT_0, max(numSamples, 1));

		for (i = 0; i < r_ssaoBlur->integer; i++) {
#if 1
			// two-pass shader
			for (j = 0; j < 2; j++) {
				GL_MBindRect(GL_TEXTURE0, fboColor[j]->texnum);
				qglDrawBuffer(GL_COLOR_ATTACHMENT0 + (j ^ 1));
				qglUniform2f(U_PARAM_VEC2_0, j ? 0.f : 1.f, j ? 1.f : 0.f);
				R_DrawHalfScreenQuad();
			}
#else
			// single-pass shader
			GL_MBindRect(GL_TEXTURE0, fboColor[fboColorIndex]);
			fboColorIndex ^= 1;
			qglDrawBuffer(GL_COLOR_ATTACHMENT0 + fboColorIndex);
			R_DrawHalfScreenQuad();
#endif
		}
	}

	// restore
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);
	GL_DepthMask(1);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
				r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

/*
===========================================
 Based on Giliam de Carpentier work
 http://www.decarpentier.nl/lens-distortion
===========================================
*/

void R_FixFov(void) {

	vec4_t params;

	if (!r_fixFovStrength->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram(fixFovProgram);

	if (!fovCorrTex) {
		qglGenTextures(1, &fovCorrTex);
		GL_MBind(GL_TEXTURE0, fovCorrTex);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 0, 0, vid.width, vid.height, 0);
	}
	GL_MBind(GL_TEXTURE0, fovCorrTex);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);

	params[0] = r_fixFovStrength->value;
	params[1] = tan(DEG2RAD(r_newrefdef.fov_x) / 2.0) / (vid.width / vid.height);
	params[2] = vid.width / vid.height;
	params[3] = r_fixFovDistroctionRatio->value;

	qglUniform4fv(U_PARAM_VEC4_0, 1, params);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();
}

void R_MenuBackGround() {
	
	GL_Disable(GL_BLEND);
	GL_BindProgram(menuProgram);
	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();
	GL_Enable(GL_BLEND);
}


void R_GlobalFog() {

	if (!r_globalFog->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;
	
	if (!r_worldmodel)
		return;

	GL_DepthMask(0);
	R_SetupOrthoMatrix();

	GL_BindProgram(globalFogProgram);
	
	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	GL_MBindRect(GL_TEXTURE1, depthMap->texnum);
	GL_MBindRect(GL_TEXTURE2, skyMask->texnum);

	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	if (r_worldmodel->useFogFile) {
		qglUniform1i(U_PARAM_INT_0, r_worldmodel->fogType);
		qglUniform4f(U_PARAM_VEC4_0, r_worldmodel->fogColor[0], r_worldmodel->fogColor[1], r_worldmodel->fogColor[2], r_worldmodel->fogDensity);
		qglUniform4f(U_PARAM_VEC4_1, r_worldmodel->fogSkyColor[0], r_worldmodel->fogSkyColor[1], r_worldmodel->fogSkyColor[2], r_worldmodel->fogSkyDensity);
		qglUniform2f(U_PARAM_VEC2_0, r_worldmodel->fogBias, r_worldmodel->fogSkyBias);
	}
	else {
		qglUniform1i(U_PARAM_INT_0, r_globalFog->integer);
		qglUniform4f(U_PARAM_VEC4_0, r_globalFogRed->value, r_globalFogGreen->value, r_globalFogBlue->value, r_globalFogDensity->value);
		qglUniform4f(U_PARAM_VEC4_1, r_globalSkyFogRed->value, r_globalSkyFogGreen->value, r_globalSkyFogBlue->value, r_globalSkyFogDensity->value);
		qglUniform2f(U_PARAM_VEC2_0, r_globalFogBias->value, r_globalSkyFogBias->value);
	}
	
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	// restore 3d
	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);

	GL_DepthMask(1);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
		r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_SaveFogScript_f(void) {

	char	name[MAX_QPATH], path[MAX_QPATH];
	FILE	*f;

	if (!r_globalFog->integer) {
		Com_Printf("Type r_globalFog 1 or 2 to save or remove fog script.\n");
		return;
	}
	if (!r_worldmodel)
		return;

	FS_StripExtension(r_worldmodel->name, name, sizeof(name));
	Com_sprintf(path, sizeof(path), "%s/%s.fog", FS_Gamedir(), name);

	f = fopen(path, "w");
	if (!f) {
		Com_Printf("Could not open %s.\n", path);
		return;
	}

	fprintf(f, "//Fog Script for %s\n//Generated by quake2xp\n\n", r_worldmodel->name);
	fprintf(f, "fogType %i\n",					r_globalFog->integer);
	fprintf(f, "fogColor %.3f %.3f %.3f\n",		r_globalFogRed->value,		r_globalFogGreen->value,	r_globalFogBlue->value);
	fprintf(f, "fogSkyColor %.3f %.3f %.3f\n",	r_globalSkyFogRed->value,	r_globalSkyFogGreen->value, r_globalSkyFogBlue->value);
	fprintf(f, "fogDensity %.5f\n",				r_globalFogDensity->value);
	fprintf(f, "fogSkyDensity %.5f\n",			r_globalSkyFogDensity->value);
	fprintf(f, "fogBias %.3f\n",				r_globalFogBias->value);
	fprintf(f, "fogSkyBias %.3f\n",				r_globalSkyFogBias->value);
	fclose(f);

	Com_Printf(""S_COLOR_MAGENTA"R_SaveFogScript_f: "S_COLOR_WHITE"Save Fog Script To "S_COLOR_GREEN"%s.fog\n", name);
}

void R_RemoveFogScript_f(void) {

	char	name[MAX_QPATH], path[MAX_QPATH];

	if (!r_globalFog->integer) {
		Com_Printf("Type r_globalFog 1 or 2 to save or remove fog script.\n");
		return;
	}
	if (!r_worldmodel)
		return;

	FS_StripExtension(r_worldmodel->name, name, sizeof(name));
	Com_sprintf(path, sizeof(path), "%s/%s.fog", FS_Gamedir(), name);
	remove(path); //remove it!
	Com_Printf(""S_COLOR_MAGENTA"R_RemoveFogScript_f: "S_COLOR_WHITE"Remove Fog Script To "S_COLOR_GREEN"%s.fog\n", name);
}
