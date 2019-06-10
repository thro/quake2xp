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

#include "r_local.h"

extern model_t *loadmodel;

//
// 3-vector radiosity basis for normal mapping
// need high precision to lower error on further bounces
//
const vec3_t r_xplmBasisVecs[XPLM_NUMVECS] = {
	{ 0.81649658092772603273242802490196f, 0.f, 0.57735026918962576450914878050195f },
	{ -0.40824829046386301636621401245098f, 0.70710678118654752440084436210485f, 0.57735026918962576450914878050195f },
	{ -0.40824829046386301636621401245098f, -0.70710678118654752440084436210485f, 0.57735026918962576450914878050195f }
};

/*
=============================================================================

LIGHTMAP ALLOCATION

=============================================================================
*/
/*
===============
R_SetCacheState

===============
*/
void R_SetCacheState (msurface_t * surf) {
	int i;

	for (i = 0; i < MAXLIGHTMAPS && surf->styles[i] != 255; i++)
		surf->cached_light[i] = r_newrefdef.lightstyles[surf->styles[i]].white;
}

/*
===============
R_BuildLightMap

Combine & scale multiple lightmaps into the floating-point format in s_blocklights,
then normalize into GL format in gl_lms.lightmap_buffer.
===============
*/
//extern int *mod_xplmOffsets;		// face light offsets from .xplm file, freed after level is loaded
void R_BuildLightMap (msurface_t *surf, int stride) {
	static float s_blocklights[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];		// intermediate RGB buffer for combining multiple lightmaps
	const int numVecs = loadmodel->useXPLM ? 3 : 1;
	int smax, tmax;
	int r, g, b, cmax;
	int i, j, k, size;
	vec3_t scale;
	float *bl;
	byte *lm, *dest;
	int numMaps;

	if (surf->texInfo->flags & (SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP))
		VID_Error (ERR_DROP, "R_BuildLightMap(): called for non-lit surface.");

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	// no more dynamic lightmaps, so only loadmodel is used
	smax = (surf->extents[0] / (int)loadmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / (int)loadmodel->lightmap_scale) + 1;

	size = smax * tmax;
	stride -= smax * 3;

	if (size > (sizeof(s_blocklights) / (int)loadmodel->lightmap_scale))
		VID_Error (ERR_DROP, "R_BuildLightMap(): bad s_blocklights size.");

	// count maps
	if (surf->samples)
	for (numMaps = 0; numMaps < MAXLIGHTMAPS && surf->styles[numMaps] != 255; numMaps++);

	for (k = 0; k < numVecs; k++) {
		if (surf->samples) {
			lm = surf->samples + k * size * 3;

			// KRIGS:
			// we don't refresh lightmaps at runtime anymore, so load only the first map
			// use real-time lights for styling instead
			// 0 or 1
			if (1 || numMaps == 1) {
				// copy the lightmap
				bl = s_blocklights;

				for (i = 0; i < 3; i++)
					scale[i] = r_newrefdef.lightstyles[surf->styles[0]].rgb[i];

				if (scale[0] == 1.f && scale[1] == 1.f && scale[2] == 1.f) {
					for (i = 0; i < size; i++, bl += 3, lm += 3) {
						bl[0] = lm[0];
						bl[1] = lm[1];
						bl[2] = lm[2];
					}
				}
				else {
					for (i = 0; i < size; i++, bl += 3, lm += 3) {
						bl[0] = lm[0] * scale[0];
						bl[1] = lm[1] * scale[1];
						bl[2] = lm[2] * scale[2];
					}
				}
			}
			else {
				// add all the lightmaps
				memset (s_blocklights, 0, sizeof(s_blocklights[0]) * size * 3);

				for (j = 0; j < numMaps; j++) {
					bl = s_blocklights;

					for (i = 0; i < 3; i++)
						scale[i] = r_newrefdef.lightstyles[surf->styles[j]].rgb[i];

					if (scale[0] == 1.f && scale[1] == 1.f && scale[2] == 1.f) {
						for (i = 0; i < size; i++, bl += 3, lm += 3) {
							bl[0] += lm[0];
							bl[1] += lm[1];
							bl[2] += lm[2];
						}
					}
					else {
						for (i = 0; i < size; i++, bl += 3, lm += 3) {
							bl[0] += lm[0] * scale[0];
							bl[1] += lm[1] * scale[1];
							bl[2] += lm[2] * scale[2];
						}
					}

					if (loadmodel->useXPLM)
						lm += size * 6;	// skip the rest 6/9 to the next style's lightmap
				}
			}
		}
		else {
			// set to full bright if no light data
			for (i = 0; i < size * 3; i++)
				s_blocklights[i] = 255.f;
		}

		//
		// put into texture format
		//

		bl = s_blocklights;

		dest = gl_lms.lightmap_buffer[k];
		dest += (surf->light_t * LIGHTMAP_SIZE + surf->light_s) * 3;

		for (i = 0; i < tmax; i++, dest += stride) {
			for (j = 0; j < smax; j++, bl += 3, dest += 3) {
				r = Q_ftol (bl[0]);
				g = Q_ftol (bl[1]);
				b = Q_ftol (bl[2]);

				// catch negative lights
				r = max (r, 0);
				g = max (g, 0);
				b = max (b, 0);

				// determine the brightest of the three color components
				cmax = max (r, max (g, b));

				// rescale all the color components if the intensity of the greatest channel exceeds 1.0
				if (cmax > 255) {
					float t = 255.f / cmax;

					r *= t;
					g *= t;
					b *= t;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
			}
		}
	}	// for k

}

static void LM_InitBlock (void) {
	memset (gl_lms.allocated, 0, sizeof(gl_lms.allocated));
}

// FIXME: remove dynamic completely
static void LM_UploadBlock (qboolean dynamic) {

	const int	numVecs = loadmodel->useXPLM ? 3 : 1;
	int			texture = gl_lms.current_lightmap_texture;
	int			i;

	// upload the finished atlas
	for (i = 0; i < numVecs; i++) {

		GL_Bind (gl_state.lightmap_textures + texture + i * MAX_LIGHTMAPS);
		qglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		qglTexImage2D (GL_TEXTURE_2D,
				0,
				gl_lms.internal_format,
				LIGHTMAP_SIZE, LIGHTMAP_SIZE,
				0,
				GL_LIGHTMAP_FORMAT,
				GL_UNSIGNED_BYTE,
				gl_lms.lightmap_buffer[i]);
	}

	// start new one
	//	if (!dynamic) {
	// check if atlas limit is exceeded
	if (++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS)
		VID_Error (ERR_DROP, "LM_UploadBlock(): MAX_LIGHTMAPS (%i) exceeded.\n", MAX_LIGHTMAPS);
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock (int w, int h, int *x, int *y) {
	int i, j;
	int best, best2;

	best = LIGHTMAP_SIZE;

	for (i = 0; i < LIGHTMAP_SIZE - w; i++) {
		best2 = 0;

		for (j = 0; j < w; j++) {
			if (gl_lms.allocated[i + j] >= best)
				break;
			if (gl_lms.allocated[i + j] > best2)
				best2 = gl_lms.allocated[i + j];
		}

		if (j == w) {			// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > LIGHTMAP_SIZE)
		return qfalse;

	for (i = 0; i < w; i++)
		gl_lms.allocated[*x + i] = best + h;

	return qtrue;
}

/*
================
GL_BuildPolygonFromSurface
================
*/
void GL_BuildPolygonFromSurface (msurface_t * fa) {
	int			i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
	int			vertpage;
	float		*vec;
	float		s, t;
	glpoly_t	*poly;
	vec3_t		total;
	temp_connect_t *tempEdge;

	fa->numVertices = fa->numEdges;
	fa->numIndices = (fa->numVertices - 2) * 3;

	// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numEdges;
	vertpage = 0;

	VectorClear (total);
	//
	// draw texture
	//
	poly = (glpoly_t*)Hunk_Alloc (sizeof(glpoly_t)+(lnumverts - 4) * VERTEXSIZE * sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numVerts = lnumverts;

	currentmodel->memorySize += sizeof(glpoly_t)+(lnumverts - 4) * VERTEXSIZE * sizeof(float);

	// reserve space for neighbour pointers
	// FIXME: pointers don't need to be 4 bytes
	poly->neighbours = (glpoly_t **)Hunk_Alloc (lnumverts * 4);

	for (i = 0; i < lnumverts; i++) {
		lindex = currentmodel->surfEdges[fa->firstedge + i];

		if (lindex > 0) {
			r_pedge = &pedges[lindex];
			vec = currentmodel->vertexes[r_pedge->v[0]].position;
		}
		else {
			r_pedge = &pedges[-lindex];
			vec = currentmodel->vertexes[r_pedge->v[1]].position;
		}
		s = DotProduct (vec,
			fa->texInfo->vecs[0]) + fa->texInfo->vecs[0][3];
		s /= fa->texInfo->image->width;

		t = DotProduct (vec,
			fa->texInfo->vecs[1]) + fa->texInfo->vecs[1][3];
		t /= fa->texInfo->image->height;

		VectorAdd (total, vec, total);
		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct (vec, fa->texInfo->vecs[0]) + fa->texInfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s * (float)loadmodel->lightmap_scale;
		s += (float)loadmodel->lightmap_scale / 2.0;
		s /= LIGHTMAP_SIZE * (float)loadmodel->lightmap_scale;

		t = DotProduct (vec, fa->texInfo->vecs[1]) + fa->texInfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t * (float)loadmodel->lightmap_scale;
		t += (float)loadmodel->lightmap_scale / 2.0;
		t /= LIGHTMAP_SIZE * (float)loadmodel->lightmap_scale;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;

		// Store edge data for shadow volumes
		tempEdge = tempEdges + abs (lindex);
		if (tempEdge->used < 2) {
			tempEdge->poly[tempEdge->used] = poly;
			tempEdge->used++;
		}
		else
			Com_DPrintf ("GL_BuildPolygonFromSurface: Edge used by more than 2 surfaces\n");

	}

	poly->numVerts = lnumverts;

	VectorScale (total, 1.0f / (float)lnumverts, total);

	fa->c_s =
		(DotProduct (total, fa->texInfo->vecs[0]) + fa->texInfo->vecs[0][3])
		/ fa->texInfo->image->width;
	fa->c_t =
		(DotProduct (total, fa->texInfo->vecs[1]) + fa->texInfo->vecs[1][3])
		/ fa->texInfo->image->height;
}

/*
========================
GL_CreateSurfaceLightmap

========================
*/
void GL_CreateSurfaceLightmap (msurface_t * surf) {
	int smax, tmax;

	if (surf->flags & (MSURF_DRAWSKY | MSURF_DRAWTURB))
		return;

	smax = (surf->extents[0] / loadmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / loadmodel->lightmap_scale) + 1;

	if (!LM_AllocBlock (smax, tmax, &surf->light_s, &surf->light_t)) {
		LM_UploadBlock (qfalse);
		LM_InitBlock ();

		if (!LM_AllocBlock (smax, tmax, &surf->light_s, &surf->light_t))
			VID_Error (ERR_FATAL, "GL_CreateSurfaceLightmap(): consecutive calls to LM_AllocBlock(%d, %d) failed.\n", smax, tmax);
	}

	R_SetCacheState (surf);
	R_BuildLightMap (surf, LIGHTMAP_SIZE * 3);
}

extern int occ_framecount;

/*
==================
GL_BeginBuildingLightmaps

==================
*/
void GL_BeginBuildingLightmaps (model_t *m) {
	static lightstyle_t lightstyles[MAX_LIGHTSTYLES];
	int i;

	memset (gl_lms.allocated, 0, sizeof(gl_lms.allocated));

	occ_framecount = r_framecount = 1;

	/*
	 ** setup the base lightstyles so the lightmaps won't have to be regenerated
	 ** the first time they're seen
	 */
	for (i = 0; i < MAX_LIGHTSTYLES; i++) {
		lightstyles[i].rgb[0] = 1;
		lightstyles[i].rgb[1] = 1;
		lightstyles[i].rgb[2] = 1;
		lightstyles[i].white = 3;
	}
	r_newrefdef.lightstyles = lightstyles;

	if (!gl_state.lightmap_textures)
		gl_state.lightmap_textures = TEXNUM_LIGHTMAPS;

	gl_lms.current_lightmap_texture = 1;
	gl_lms.internal_format = GL_RGB8;//gl_tex_solid_format;

}

/*
=======================
GL_EndBuildingLightmaps
=======================
*/
void GL_EndBuildingLightmaps (void) {
	LM_UploadBlock (qfalse);
}

