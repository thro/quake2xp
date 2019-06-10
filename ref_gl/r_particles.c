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

#include "r_local.h"

vec4_t shadelight_surface;
image_t *r_particletexture[PT_MAX];

int CL_PMpointcontents (vec3_t point);

/*
===============
R_DrawParticles
===============
*/
void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up);

void ClampVertexColor (vec4_t color) {
	if (color[0] < 0.15)
		color[0] = 0.15;
	if (color[1] < 0.15)
		color[1] = 0.15;
	if (color[2] < 0.15)
		color[2] = 0.15;
}

#define MAX_PARTICLE_VERT 16384

vec4_t		ParticleColor[MAX_PARTICLE_VERT];
vec3_t		ParticleVert[MAX_PARTICLE_VERT];
vec2_t		ParticleTextCoord[MAX_PARTICLE_VERT];

int SortPart (particle_t *a, particle_t *b) {
	return (a->type + a->flags) - (b->type + b->flags);
}

void R_DrawParticles (void) {
	particle_t *p;
	unsigned	texId, texture = -1, flagId, flags = -1;
	index_t		ParticleIndex[MAX_INDICES];
	int			i, len, loc, partVert = 0, index = 0;
	vec3_t		point, width;
	vec3_t		move, vec, dir1, dir2, dir3, spdir;
	vec3_t		up, right;
	vec3_t		axis[3];
	vec3_t		oldOrigin;
	float		scale, r, g, b, a;
	float		c, d, s;
	mat4_t		m;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (particlesProgram);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglEnableVertexAttribArray (ATT_COLOR);

	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, ParticleVert);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, ParticleTextCoord);
	qglVertexAttribPointer (ATT_COLOR, 4, GL_FLOAT, qfalse, 0, ParticleColor);

	GL_MBindRect (GL_TEXTURE1, depthMap->texnum);

	qglUniform2f (U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewMatrix);

	GL_DepthMask (0);		// no z buffering
	GL_Enable (GL_BLEND);

	qsort (r_newrefdef.particles, r_newrefdef.num_particles, sizeof(particle_t), (int (*)(const void *, const void *))SortPart);

	Mat4_Identity(m);

	for (p = r_newrefdef.particles, i = 0; i < r_newrefdef.num_particles; i++, p++) {

		switch (p->type) {

			case PT_BUBBLE:
				texId = r_particletexture[PT_BUBBLE]->texnum;
				break;

			case PT_FLY:
				texId = fly[((int)(r_newrefdef.time * 10)) & (MAX_FLY - 1)]->texnum;
				break;

			case PT_BLOOD:
				texId = r_particletexture[PT_BLOOD]->texnum;
				break;

			case PT_BLOOD2:
				texId = r_particletexture[PT_BLOOD2]->texnum;
				break;

			case PT_BLASTER:
				texId = r_particletexture[PT_BLASTER]->texnum;
				break;

			case PT_SMOKE:
				texId = r_particletexture[PT_SMOKE]->texnum;
				break;

			case PT_SPLASH:
				texId = r_particletexture[PT_SPLASH]->texnum;
				break;

			case PT_SPARK:
				texId = r_particletexture[PT_SPARK]->texnum;
				break;

			case PT_BEAM:
				texId = r_particletexture[PT_BEAM]->texnum;
				break;

			case PT_SPIRAL:
				texId = r_particletexture[PT_SPIRAL]->texnum;
				break;


			case PT_FLAME:
				texId = flameanim[((int)((r_newrefdef.time - p->time) * 10)) % MAX_FLAMEANIM]->texnum;
				break;

			case PT_BLOODSPRAY:
				texId = r_blood[((int)((r_newrefdef.time - p->time) * 15)) % MAX_BLOOD]->texnum;
				break;

			case PT_xBLOODSPRAY:
				texId = r_xblood[((int)((r_newrefdef.time - p->time) * 15)) % MAX_BLOOD]->texnum;
				break;

			case PT_EXPLODE:
				texId = r_explode[((int)((r_newrefdef.time - p->time) * 20)) % MAX_EXPLODE]->texnum;
				break;

			case PT_WATERPULME:
				texId = r_particletexture[PT_WATERPULME]->texnum;
				break;

			case PT_WATERCIRCLE:
				texId = r_particletexture[PT_WATERCIRCLE]->texnum;
				break;

			case PT_BLOODDRIP:
				texId = r_particletexture[PT_BLOODDRIP]->texnum;
				break;

			case PT_BLOODMIST:
				texId = r_particletexture[PT_BLOODMIST]->texnum;
				break;

			case PT_BLASTER_BOLT:
				texId = r_particletexture[PT_BLASTER_BOLT]->texnum;
				break;

			case PT_BFG_BALL:
				texId = r_particletexture[PT_BFG_BALL]->texnum;
				break;

			case PT_BFG_EXPL:
				texId = r_bfg_expl[((int)((r_newrefdef.time - p->time) * 20)) % MAX_BFG_EXPL]->texnum;
				break;
			
			case PT_BFG_EXPL2:
				texId = r_particletexture[PT_BFG_EXPL2]->texnum;
				break;

			default:
				texId = r_particletexture[PT_DEFAULT]->texnum;

		}


		scale = p->size;
		flagId = p->flags;

		if (texture != texId || flags != flagId) {

			if (partVert) {
				qglDrawElements	(GL_TRIANGLES, index, GL_UNSIGNED_SHORT, ParticleIndex);
				c_part_tris += index / 3;
			}
			texture = texId;
			flags = flagId;
			partVert = 0;
			index = 0;

			GL_MBind (GL_TEXTURE0, texId);
			GL_BlendFunc (p->sFactor, p->dFactor);

			if (p->sFactor == GL_ONE && p->dFactor == GL_ONE)
				qglUniform2f (U_PARTICLE_MASK, 1.0, 0.0); //color
			else
				qglUniform2f (U_PARTICLE_MASK, 0.0, 1.0); //alpha
			
			if (p->flags & PARTICLE_NOFADE)
				qglUniform1f (U_PARTICLE_THICKNESS, 0.0);
			else
				if (p->flags & PARTICLE_SOFT_MIDLE)
					qglUniform1f(U_PARTICLE_THICKNESS, scale * 0.35);
				else
				qglUniform1f (U_PARTICLE_THICKNESS, scale * 0.75); // soft blend scale

			if (p->flags & PARTICLE_OVERBRIGHT)
				qglUniform1f (U_COLOR_MUL, 2.0);
			else
				qglUniform1f (U_COLOR_MUL, 1.0);

			if (p->flags & PARTICLE_ROTATE) {

				float rot = r_newrefdef.time * 70;

				Mat4_Translate	(m, 0.5f, 0.5f, 0.5f);

				Mat4_Rotate		(m, 0.f,	1.f, 0.f, 0.f);
				Mat4_Rotate		(m, 0.f,	0.f, 1.f, 0.f);
				Mat4_Rotate		(m, rot,	0.f, 0.f, 1.f);

				Mat4_Translate	(m, -0.5f, -0.5f, -0.5f);

				qglUniformMatrix4fv(U_TEXTURE0_MATRIX, 1, qfalse, (const float *)m);
			}else
				qglUniformMatrix4fv(U_TEXTURE0_MATRIX, 1, qfalse, (const float *)m);

		}

		r = p->color[0];
		g = p->color[1];
		b = p->color[2];
		a = p->alpha;

		if (p->flags & PARTICLE_STRETCH) {

			VectorSubtract (p->origin, r_newrefdef.vieworg, point);
			CrossProduct (point, p->length, width);
			VectorNormalizeFast(width);
			VectorScale (width, scale, width);

			VA_SetElem2 (ParticleTextCoord[partVert + 0], 1, 1);
			VA_SetElem3 (ParticleVert[partVert + 0], p->origin[0] + width[0],
				p->origin[1] + width[1],
				p->origin[2] + width[2]);
			VA_SetElem4 (ParticleColor[partVert + 0], r, g, b, a);


			VA_SetElem2 (ParticleTextCoord[partVert + 1], 0, 0);
			VA_SetElem3 (ParticleVert[partVert + 1], p->origin[0] - width[0],
				p->origin[1] - width[1],
				p->origin[2] - width[2]);
			VA_SetElem4 (ParticleColor[partVert + 1], r, g, b, a);

			VectorAdd (point, p->length, point);
			CrossProduct (point, p->length, width);
			VectorNormalizeFast(width);
			VectorScale (width, scale, width);

			VA_SetElem2 (ParticleTextCoord[partVert + 2], 0, 0);
			VA_SetElem3 (ParticleVert[partVert + 2], p->origin[0] + p->length[0] - width[0],
				p->origin[1] + p->length[1] - width[1],
				p->origin[2] + p->length[2] - width[2]);
			VA_SetElem4 (ParticleColor[partVert + 2], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 3], 1, 1);
			VA_SetElem3 (ParticleVert[partVert + 3], p->origin[0] + p->length[0] + width[0],
				p->origin[1] + p->length[1] + width[1],
				p->origin[2] + p->length[2] + width[2]);
			VA_SetElem4 (ParticleColor[partVert + 3], r, g, b, a);

			ParticleIndex[index++] = partVert + 0;
			ParticleIndex[index++] = partVert + 1;
			ParticleIndex[index++] = partVert + 3;
			ParticleIndex[index++] = partVert + 3;
			ParticleIndex[index++] = partVert + 1;
			ParticleIndex[index++] = partVert + 2;

			partVert += 4;


		}

		if (p->flags & PARTICLE_SPIRAL) {

			VectorCopy (p->origin, move);
			VectorCopy (p->length, vec);
			len = VectorNormalize(vec);
			MakeNormalVectors (vec, right, up);

			for (loc = 0; loc < len; loc++) {
				d = loc * 0.1f;
				c = cos (d);
				s = sin (d);

				VectorScale (right, c * 5.0f, dir1);
				VectorMA (dir1, s * 5.0f, up, dir1);

				d = (loc + 1) * 0.1f;
				c = cos (d);
				s = sin (d);

				VectorScale (right, c * 5.0f, dir2);
				VectorMA (dir2, s * 5.0f, up, dir2);
				VectorAdd (dir2, vec, dir2);

				d = (loc + 2) * 0.1f;
				c = cos (d);
				s = sin (d);

				VectorScale (right, c * 5.0f, dir3);
				VectorMA (dir3, s * 5.0f, up, dir3);
				VectorMA (dir3, 2.0f, vec, dir3);

				VectorAdd (move, dir1, point);
				VectorSubtract (dir2, dir1, spdir);

				VectorSubtract (point, r_origin, point);
				CrossProduct (point, spdir, width);

				if (VectorLength (width))
					VectorNormalize(width);
				else
					VectorCopy (vup, width);


				VA_SetElem2 (ParticleTextCoord[partVert + 0], 0.5, 1);
				VA_SetElem3 (ParticleVert[partVert + 0], point[0] + width[0] + r_origin[0],
					point[1] + width[1] + r_origin[1],
					point[2] + width[2] + r_origin[2]);
				VA_SetElem4 (ParticleColor[partVert + 0], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 1], 0.5, 0);
				VA_SetElem3 (ParticleVert[partVert + 1], point[0] - width[0] + r_origin[0],
					point[1] - width[1] + r_origin[1],
					point[2] - width[2] + r_origin[2]);
				VA_SetElem4 (ParticleColor[partVert + 1], r, g, b, a);
				
				VectorAdd (move, dir2, point);
				VectorSubtract (dir3, dir2, spdir);

				VectorSubtract (point, r_origin, point);
				CrossProduct (point, spdir, width);

				if (VectorLength (width))
					VectorNormalize(width);
				else
					VectorCopy (vup, width);

				VA_SetElem2 (ParticleTextCoord[partVert + 2], 0.5, 0);
				VA_SetElem3 (ParticleVert[partVert + 2], point[0] - width[0] + r_origin[0],
					point[1] - width[1] + r_origin[1],
					point[2] - width[2] + r_origin[2]);
				VA_SetElem4 (ParticleColor[partVert + 2], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 3], 0.5, 1);
				VA_SetElem3 (ParticleVert[partVert + 3], point[0] + width[0] + r_origin[0],
					point[1] + width[1] + r_origin[1],
					point[2] + width[2] + r_origin[2]);
				VA_SetElem4 (ParticleColor[partVert + 3], r, g, b, a);

				ParticleIndex[index++] = partVert + 0;
				ParticleIndex[index++] = partVert + 1;
				ParticleIndex[index++] = partVert + 3;
				ParticleIndex[index++] = partVert + 3;
				ParticleIndex[index++] = partVert + 1;
				ParticleIndex[index++] = partVert + 2;

				partVert += 4;

				VectorAdd (move, vec, move);
			}
		}

		if (p->flags & PARTICLE_DIRECTIONAL) {
			// find orientation vectors
			VectorSubtract (r_newrefdef.vieworg, p->origin, axis[0]);
			VectorSubtract (p->oldOrg, p->origin, axis[1]);
			CrossProduct (axis[0], axis[1], axis[2]);
			VectorNormalizeFast(axis[1]);
			VectorNormalizeFast(axis[2]);

			// find normal
			CrossProduct (axis[1], axis[2], axis[0]);
			VectorNormalizeFast(axis[0]);

			VectorMA (p->origin, -p->len, axis[1], oldOrigin);
			VectorScale (axis[2], p->size, axis[2]);


			VA_SetElem2 (ParticleTextCoord[partVert + 0], 1, 1);
			VA_SetElem3 (ParticleVert[partVert + 0], oldOrigin[0] + axis[2][0],
				oldOrigin[1] + axis[2][1],
				oldOrigin[2] + axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 0], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 1], 0, 1);
			VA_SetElem3 (ParticleVert[partVert + 1], p->origin[0] + axis[2][0],
				p->origin[1] + axis[2][1],
				p->origin[2] + axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 1], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 2], 0, 0);
			VA_SetElem3 (ParticleVert[partVert + 2], p->origin[0] - axis[2][0],
				p->origin[1] - axis[2][1],
				p->origin[2] - axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 2], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 3], 1, 0);
			VA_SetElem3 (ParticleVert[partVert + 3], oldOrigin[0] - axis[2][0],
				oldOrigin[1] - axis[2][1],
				oldOrigin[2] - axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 3], r, g, b, a);

			ParticleIndex[index++] = partVert + 0;
			ParticleIndex[index++] = partVert + 1;
			ParticleIndex[index++] = partVert + 3;
			ParticleIndex[index++] = partVert + 3;
			ParticleIndex[index++] = partVert + 1;
			ParticleIndex[index++] = partVert + 2;

			partVert += 4;
		}

		if (p->flags & PARTICLE_ALIGNED) {
			// Find axes
			VectorCopy (p->dir, axis[0]);
			MakeNormalVectors (axis[0], axis[1], axis[2]);

			// Scale the axes by radius
			VectorScale (axis[1], p->size, axis[1]);
			VectorScale (axis[2], p->size, axis[2]);


			VA_SetElem2 (ParticleTextCoord[partVert + 0], 0, 1);
			VA_SetElem3 (ParticleVert[partVert + 0], p->origin[0] + axis[1][0] + axis[2][0],
				p->origin[1] + axis[1][1] + axis[2][1],
				p->origin[2] + axis[1][2] + axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 0], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 1], 0, 0);
			VA_SetElem3 (ParticleVert[partVert + 1], p->origin[0] - axis[1][0] + axis[2][0],
				p->origin[1] - axis[1][1] + axis[2][1],
				p->origin[2] - axis[1][2] + axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 1], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 2], 1, 0);
			VA_SetElem3 (ParticleVert[partVert + 2], p->origin[0] - axis[1][0] - axis[2][0],
				p->origin[1] - axis[1][1] - axis[2][1],
				p->origin[2] - axis[1][2] - axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 2], r, g, b, a);

			VA_SetElem2 (ParticleTextCoord[partVert + 3], 1, 1);
			VA_SetElem3 (ParticleVert[partVert + 3], p->origin[0] + axis[1][0] - axis[2][0],
				p->origin[1] + axis[1][1] - axis[2][1],
				p->origin[2] + axis[1][2] - axis[2][2]);
			VA_SetElem4 (ParticleColor[partVert + 3], r, g, b, a);

			ParticleIndex[index++] = partVert + 0;
			ParticleIndex[index++] = partVert + 1;
			ParticleIndex[index++] = partVert + 3;
			ParticleIndex[index++] = partVert + 3;
			ParticleIndex[index++] = partVert + 1;
			ParticleIndex[index++] = partVert + 2;

			partVert += 4;
		}

		if (!(p->flags & PARTICLE_ALIGNED) && !(p->flags & PARTICLE_DIRECTIONAL) &&
			!(p->flags & PARTICLE_SPIRAL) && !(p->flags & PARTICLE_STRETCH)) {
			//  standart particles 
			VectorScale (vup, scale, up);
			VectorScale (vright, scale, right);

			// hack a scale up to keep particles from disapearing
			scale = (p->origin[0] - r_origin[0]) * vpn[0] +
					(p->origin[1] - r_origin[1]) * vpn[1] +
					(p->origin[2] - r_origin[2]) * vpn[2];

			scale = (scale < 20) ? 1 : 1 + scale * 0.0004;

			if (p->orient) {

				float c = (float)cos (DEG2RAD (p->orient)) * scale;
				float s = (float)sin (DEG2RAD (p->orient)) * scale;

				VA_SetElem2 (ParticleTextCoord[partVert + 0], 0, 1);
				VA_SetElem3 (ParticleVert[partVert + 0], p->origin[0] - right[0] * c - up[0] * s,
					p->origin[1] - right[1] * c - up[1] * s,
					p->origin[2] - right[2] * c - up[2] * s);
				VA_SetElem4 (ParticleColor[partVert + 0], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 1], 0, 0);
				VA_SetElem3 (ParticleVert[partVert + 1], p->origin[0] - right[0] * s + up[0] * c,
					p->origin[1] - right[1] * s + up[1] * c,
					p->origin[2] - right[2] * s + up[2] * c);
				VA_SetElem4 (ParticleColor[partVert + 1], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 2], 1, 0);
				VA_SetElem3 (ParticleVert[partVert + 2], p->origin[0] + right[0] * c + up[0] * s,
					p->origin[1] + right[1] * c + up[1] * s,
					p->origin[2] + right[2] * c + up[2] * s);
				VA_SetElem4 (ParticleColor[partVert + 2], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 3], 1, 1);
				VA_SetElem3 (ParticleVert[partVert + 3], p->origin[0] + right[0] * s - up[0] * c,
					p->origin[1] + right[1] * s - up[1] * c,
					p->origin[2] + right[2] * s - up[2] * c);
				VA_SetElem4 (ParticleColor[partVert + 3], r, g, b, a);

				ParticleIndex[index++] = partVert + 0;
				ParticleIndex[index++] = partVert + 1;
				ParticleIndex[index++] = partVert + 3;
				ParticleIndex[index++] = partVert + 3;
				ParticleIndex[index++] = partVert + 1;
				ParticleIndex[index++] = partVert + 2;

				partVert += 4;
			}
			else {

				VA_SetElem2 (ParticleTextCoord[partVert + 0], 0, 1);
				VA_SetElem3 (ParticleVert[partVert + 0], p->origin[0] - right[0] * scale - up[0] * scale,
					p->origin[1] - right[1] * scale - up[1] * scale,
					p->origin[2] - right[2] * scale - up[2] * scale);
				VA_SetElem4 (ParticleColor[partVert + 0], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 1], 0, 0);
				VA_SetElem3 (ParticleVert[partVert + 1], p->origin[0] - right[0] * scale + up[0] * scale,
					p->origin[1] - right[1] * scale + up[1] * scale,
					p->origin[2] - right[2] * scale + up[2] * scale);
				VA_SetElem4 (ParticleColor[partVert + 1], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 2], 1, 0);
				VA_SetElem3 (ParticleVert[partVert + 2], p->origin[0] + right[0] * scale + up[0] * scale,
					p->origin[1] + right[1] * scale + up[1] * scale,
					p->origin[2] + right[2] * scale + up[2] * scale);
				VA_SetElem4 (ParticleColor[partVert + 2], r, g, b, a);

				VA_SetElem2 (ParticleTextCoord[partVert + 3], 1, 1);
				VA_SetElem3 (ParticleVert[partVert + 3], p->origin[0] + right[0] * scale - up[0] * scale,
					p->origin[1] + right[1] * scale - up[1] * scale,
					p->origin[2] + right[2] * scale - up[2] * scale);
				VA_SetElem4 (ParticleColor[partVert + 3], r, g, b, a);

				ParticleIndex[index++] = partVert + 0;
				ParticleIndex[index++] = partVert + 1;
				ParticleIndex[index++] = partVert + 3;
				ParticleIndex[index++] = partVert + 3;
				ParticleIndex[index++] = partVert + 1;
				ParticleIndex[index++] = partVert + 2;

				partVert += 4;
			}
		}
	}

	if (partVert) {
		qglDrawElements	(GL_TRIANGLES, index, GL_UNSIGNED_SHORT, ParticleIndex);
		c_part_tris += index / 3;
	}

	GL_Disable (GL_BLEND);
	GL_DepthMask (1);			// back to normal Z buffering

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_TEX0);
	qglDisableVertexAttribArray (ATT_COLOR);

}
