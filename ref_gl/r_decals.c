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
//Decals based on EGL code. 

#include "r_local.h"

vec4_t shadelight_surface;

int R_GetClippedFragments(vec3_t origin, float radius, mat3_t axis,
						  int maxfverts, vec3_t * fverts, int maxfragments,
						  fragment_t * fragments);

/*
=================
PlaneTypeForNormal
=================
*/

int PlaneTypeForNormal(const vec3_t normal)
{
	vec_t ax, ay, az;

	// NOTE: should these have an epsilon around 1.0? 
	if (normal[0] >= 1.0)
		return PLANE_X;
	if (normal[1] >= 1.0)
		return PLANE_Y;
	if (normal[2] >= 1.0)
		return PLANE_Z;

	ax = fabs(normal[0]);
	ay = fabs(normal[1]);
	az = fabs(normal[2]);

	if (ax >= ay && ax >= az)
		return PLANE_ANYX;
	if (ay >= ax && ay >= az)
		return PLANE_ANYY;
	return PLANE_ANYZ;
}

/*
===============
R_RenderDecals
===============
*/
extern decals_t	decals[MAX_DECALS];
extern decals_t	active_decals, *free_decals;
extern cvar_t *cl_decals;

void CL_FreeDecal(decals_t * dl);

#define MAX_DECAL_ARRAY_VERTS 4096
#define MAX_DECAL_INDICES     8192

vec4_t	DecalColorArray		[MAX_DECAL_ARRAY_VERTS];
vec2_t	DecalTexCoordArray	[MAX_DECAL_ARRAY_VERTS];
vec3_t	DecalVertexArray	[MAX_DECAL_ARRAY_VERTS];
index_t	DecalIdxArray		[MAX_DECAL_INDICES];

void R_RenderDecals(void)
{
    decals_t    *dl, *next, *active; 
    vec3_t		decalColor;
    unsigned    tex, texture = 0;
    int			x, i;
    int			numIndices = 0, numVertices = 0;
    float		endLerp, decalAlpha;
	
	if (!cl_decals->integer)
		return;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

    qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

    qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, DecalVertexArray);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, DecalTexCoordArray);
    qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, DecalColorArray);
     

	GL_BindProgram(colorProgram);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniform1i(U_PARAM_INT_0, 1); // textured pass

	GL_Enable(GL_POLYGON_OFFSET_FILL);
    GL_PolygonOffset(-1, -1);
    GL_DepthMask(0);
    GL_Enable(GL_BLEND);

	active = &active_decals;

    for (dl = active->next; dl != active; dl = next) {
	
		 next = dl->next;

		 if (!dl->node || dl->node->visframe != r_visframecount)
               continue;

		 // look if it has a bad type
          if (dl->type < 0 || dl->type >= DECAL_MAX)
		  {
			  CL_FreeDecal(dl);
              continue;
		  }
       	
		  if( R_CullSphere(dl->org, dl->size * 1.3) )
				continue;

        endLerp = (float)(r_newrefdef.time - dl->time) / (float)(dl->endTime - dl->time);	
		endLerp *= 250.0;

		for (i = 0; i < 3; i++) {
			decalColor[i] = dl->color[i] + (dl->endColor[i] - dl->color[i]) * endLerp;
		
			if (decalColor[i] < dl->endColor[i])
				decalColor[i] = dl->endColor[i];
		}

		decalAlpha = dl->alpha + (dl->endAlpha - dl->alpha) * endLerp;

		tex = r_decaltexture[dl->type]->texnum;
          
        if (texture != tex) {
        // flush array if new texture/blend
        if (numIndices) {
			qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, DecalIdxArray);
			c_decal_tris += numIndices/3;
			numVertices = 0;
			numIndices = 0;
          }

		texture = tex;
        GL_MBind(GL_TEXTURE0, texture);
        GL_BlendFunc(dl->sFactor, dl->dFactor);

		if (dl->flags == DF_OVERBRIGHT)
			qglUniform1f(U_COLOR_MUL, 2.0);
		else
			qglUniform1f(U_COLOR_MUL, 1.0);

          }

     //
     // array is full, flush to screen
     //
     if ((numIndices >= MAX_DECAL_INDICES - (dl->numverts - 2) * 3) || 
		 (numVertices >= MAX_DECAL_ARRAY_VERTS - dl->numverts)) {
          
		 qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, DecalIdxArray);
         c_decal_tris = numIndices/3;
		 numVertices = 0;
         numIndices = 0;
     }

     // set vertices
          for (x = 0; x < dl->numverts; x++) {
               DecalColorArray      [x + numVertices][0] = decalColor[0];
               DecalColorArray      [x + numVertices][1] = decalColor[1];
               DecalColorArray      [x + numVertices][2] = decalColor[2];
               DecalColorArray      [x + numVertices][3] = decalAlpha;

               DecalTexCoordArray   [x + numVertices][0] = dl->stcoords[x][0];
               DecalTexCoordArray   [x + numVertices][1] = dl->stcoords[x][1];

               DecalVertexArray     [x + numVertices][0] = dl->verts[x][0];
               DecalVertexArray     [x + numVertices][1] = dl->verts[x][1];
               DecalVertexArray     [x + numVertices][2] = dl->verts[x][2];
          }

     // set indices
     for (x = 0; x < dl->numverts - 2; x++) {
		 DecalIdxArray[numIndices+x*3+0] = numVertices;
		 DecalIdxArray[numIndices+x*3+1] = numVertices + x + 1;
		 DecalIdxArray[numIndices+x*3+2] = numVertices + x + 2;
     }
     numVertices += dl->numverts;
     numIndices += (dl->numverts - 2) * 3;
     }     

     // draw the rest
	 if (numIndices){
		 qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, DecalIdxArray);
		c_decal_tris += numIndices/3;
	 }

    GL_Disable(GL_BLEND);
    qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
    qglDisableVertexAttribArray(ATT_COLOR);
    GL_DepthMask(1);
    GL_Disable(GL_POLYGON_OFFSET_FILL);
}





#define	ON_EPSILON			0.1	// point on plane side epsilon
#define BACKFACE_EPSILON	0.01

static int numFragmentVerts;
static int maxFragmentVerts;
static vec3_t *fragmentVerts;

static int numClippedFragments;
static int maxClippedFragments;
static fragment_t *clippedFragments;

static int fragmentFrame;
static cplane_t fragmentPlanes[6];

/*
=================
R_ClipPoly
=================
*/

static void R_ClipPoly(int nump, vec4_t vecs, int stage, fragment_t * fr)
{
	cplane_t *plane;
	qboolean front, back;
	vec4_t newv[MAX_DECAL_VERTS];
	float *v, d, dists[MAX_DECAL_VERTS];
	int newc, i, j, sides[MAX_DECAL_VERTS];

	if (nump > MAX_DECAL_VERTS - 2)
		Com_Printf("R_ClipPoly: MAX_DECAL_VERTS");
	if (stage == 6) {			// fully clipped
		if (nump > 2) {
			fr->numverts = nump;
			fr->firstvert = numFragmentVerts;

			if (numFragmentVerts + nump >= maxFragmentVerts)
				nump = maxFragmentVerts - numFragmentVerts;

			for (i = 0, v = vecs; i < nump; i++, v += 4)
				VectorCopy(v, fragmentVerts[numFragmentVerts + i]);

			numFragmentVerts += nump;
		}

		return;
	}

	front = back = qfalse;
	plane = &fragmentPlanes[stage];
	for (i = 0, v = vecs; i < nump; i++, v += 4) {
		d = PlaneDiff(v, plane);
		if (d > ON_EPSILON) {
			front = qtrue;
			sides[i] = SIDE_FRONT;
		} else if (d < -ON_EPSILON) {
			back = qtrue;
			sides[i] = SIDE_BACK;
		} else {
			sides[i] = SIDE_ON;
		}

		dists[i] = d;
	}

	if (!front)
		return;

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy(vecs, (vecs + (i * 4)));
	newc = 0;

	for (i = 0, v = vecs; i < nump; i++, v += 4) {
		switch (sides[i]) {
		case SIDE_FRONT:
			VectorCopy(v, newv[newc]);
			newc++;
			break;
		case SIDE_BACK:
			break;
		case SIDE_ON:
			VectorCopy(v, newv[newc]);
			newc++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i + 1] == SIDE_ON
			|| sides[i + 1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i + 1]);
		for (j = 0; j < 3; j++)
			newv[newc][j] = v[j] + d * (v[j + 4] - v[j]);
		newc++;
	}

	// continue
	R_ClipPoly(newc, newv[0], stage + 1, fr);
}

/*
=================
R_PlanarSurfClipFragment
=================
*/

static void R_PlanarSurfClipFragment(mnode_t * node, msurface_t * surf,
									 vec3_t normal)
{
	int i;
	float *v, *v2, *v3;
	fragment_t *fr;
	vec4_t verts[MAX_DECAL_VERTS];

	// bogus face
	if (surf->numEdges < 3)
		return;

	// greater than 60 degrees
	if (surf->flags & MSURF_PLANEBACK) {
		if (-DotProduct(normal, surf->plane->normal) < 0.5)
			return;
	} else {
		if (DotProduct(normal, surf->plane->normal) < 0.5)
			return;
	}

	v = surf->polys->verts[0];
	// copy vertex data and clip to each triangle
	for (i = 0; i < surf->polys->numVerts - 2; i++) {
		fr = &clippedFragments[numClippedFragments];
		fr->numverts = 0;
		fr->node = node;
		fr->surf = surf;

		v2 = surf->polys->verts[0] + (i + 1) * VERTEXSIZE;
		v3 = surf->polys->verts[0] + (i + 2) * VERTEXSIZE;

		VectorCopy(v, verts[0]);
		VectorCopy(v2, verts[1]);
		VectorCopy(v3, verts[2]);
		R_ClipPoly(3, verts[0], 0, fr);

		if (fr->numverts) {
			numClippedFragments++;

			if ((numFragmentVerts >= maxFragmentVerts)
				|| (numClippedFragments >= maxClippedFragments)) {
				return;
			}
		}
	}
}

/*
=================
R_RecursiveFragmentNode
=================
*/

static void R_RecursiveFragmentNode(mnode_t * node, vec3_t origin,
									float radius, vec3_t normal)
{
	float dist;
	cplane_t *plane;

  mark0:
	if ((numFragmentVerts >= maxFragmentVerts)
		|| (numClippedFragments >= maxClippedFragments))
		return;					// already reached the limit somewhere
								// else

	if (node->contents == CONTENTS_SOLID)
		return;

	if (node->contents != CONTENTS_NODE) {
		// leaf
		int c;
		mleaf_t *leaf;
		msurface_t *surf, **mark;

		leaf = (mleaf_t *) node;
		if (!(c = leaf->numMarkSurfaces))
			return;

		mark = leaf->firstmarksurface;
		do {
			if ((numFragmentVerts >= maxFragmentVerts)
				|| (numClippedFragments >= maxClippedFragments))
				return;
			
			surf = *mark++;
			if (surf->fragmentframe == fragmentFrame)
				continue;

			surf->fragmentframe = fragmentFrame;
			if ((surf->texInfo->
				 flags & (SURF_SKY | SURF_WARP | SURF_NODRAW)))
				continue;

			R_PlanarSurfClipFragment(node, surf, normal);

		} while (--c);

		return;
	}

	plane = node->plane;
	dist = PlaneDiff(origin, plane);

	if (dist > radius) {
		node = node->children[0];
		goto mark0;
	}
	if (dist < -radius) {
		node = node->children[1];
		goto mark0;
	}

	R_RecursiveFragmentNode(node->children[0], origin, radius, normal);
	R_RecursiveFragmentNode(node->children[1], origin, radius, normal);
}

/*
=================
R_GetClippedFragments
=================
*/

int R_GetClippedFragments(vec3_t origin, float radius, mat3_t axis,
						  int maxfverts, vec3_t * fverts, int maxfragments,
						  fragment_t * fragments)
{
	int i;
	float d;

	fragmentFrame++;

	// initialize fragments
	numFragmentVerts = 0;
	maxFragmentVerts = maxfverts;
	fragmentVerts = fverts;

	numClippedFragments = 0;
	maxClippedFragments = maxfragments;
	clippedFragments = fragments;

	// calculate clipping planes
	for (i = 0; i < 3; i++) {
		d = DotProduct(origin, axis[i]);

		VectorCopy(axis[i], fragmentPlanes[i * 2].normal);
		fragmentPlanes[i * 2].dist = d - radius;
		fragmentPlanes[i * 2].type =
			PlaneTypeForNormal(fragmentPlanes[i * 2].normal);

		VectorNegate(axis[i], fragmentPlanes[i * 2 + 1].normal);
		fragmentPlanes[i * 2 + 1].dist = -d - radius;
		fragmentPlanes[i * 2 + 1].type =
			PlaneTypeForNormal(fragmentPlanes[i * 2 + 1].normal);
	}

	R_RecursiveFragmentNode(r_worldmodel->nodes, origin, radius, axis[0]);

	return numClippedFragments;
}
