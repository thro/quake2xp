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

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/
/*
==============================================================================

WORLD LIGHTS

==============================================================================
*/

typedef struct worldShadowLight_s {
	vec3_t		origin;
	vec3_t		angles;
	vec3_t		speed;
	vec3_t		color, startColor;
	vec3_t		mins, maxs;
	vec3_t		corners[8];
	vec2_t		fov;
	mat3_t		axis;
	mat4_t		attenMatrix;
	mat4_t		spotMatrix;
	mat4_t		cubeMapMatrix;

	vec3_t		flareOrigin;
	float		flareSize;
	int			flare;

	float		radius[3];
	float		_cone;
	float		coneExp;
	float		hotSpot;
	float		distance;
	float		depthBounds[2];
	float		maxRad;
	float		fogDensity;

	int			filter, style, area;
	int			isShadow;
	int			isStatic;
	int			isNoWorldModel;
	int			isAmbient;
	int			isFog;
	int			isCone;
	int			scissor[4];
	int			start_off;

	qboolean	spherical;
	qboolean	castCaustics;

	cplane_t	frust[6];
	msurface_t	*interaction[MAX_MAP_FACES];
	int			numInteractionSurfs;

	char		targetname[MAX_QPATH];

	byte		vis[MAX_MAP_LEAFS / 8];

	GLuint		vboId;
	GLuint		iboId;
	int			iboNumIndices;

	GLuint		vboBoxId;
	GLuint		vaoBoxId;

	int		occId;

	struct worldShadowLight_s *next;
	struct worldShadowLight_s *s_next;
	
} worldShadowLight_t;

#define		Q_INFINITY	1e30f
#define		MAX_WORLD_SHADOW_LIHGTS	2048
#define		EQUAL_EPSILON		0.000001f
int			r_numWorlsShadowLights;
extern		worldShadowLight_t *currentShadowLight;
extern int	numPreCachedLights;

typedef struct {
	vec3_t origin;
	vec3_t color;
	float outcolor[4];
	float size;
	float sizefull;

	int style;

	float lightIntens;
	msurface_t *surf;
	vec3_t lightsurf_origin;
	qboolean ignore;

} autoLight_t;

int r_numAutoLights;
int r_numIgnoreAutoLights;
autoLight_t r_lightSpawnSurf[MAX_WORLD_SHADOW_LIHGTS];

#define MAX_FLARES_VERTEX 4096


byte	viewvis[MAX_MAP_LEAFS / 8];

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/

//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	vec3_t position;
} mvertex_t;

typedef struct {
	vec3_t mins, maxs;
	vec3_t origin;				// for sounds or lights
	float radius;
	int headnode;
	int visleafs;				// not including the solid leaf 0
	int firstface, numfaces;
} mmodel_t;


#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	unsigned short v[2];
	unsigned int cachededgeoffset;
} medge_t;

#define CONTENTS_NODE -1

typedef struct mleaf_s {
	// common with node
	int contents;				// wil be a negative contents number
	int visframe;				// node needs to be traversed if current

	float minmaxs[6];			// for bounding box culling

	struct mnode_s *parent;

	// leaf specific
	int cluster;
	int area;

	msurface_t **firstmarksurface;
	int numMarkSurfaces;
} mleaf_t;


//===================================================================

//
// Whole model
//


typedef enum {
	mod_bad, mod_brush, mod_sprite, 
	mod_alias, mod_alias_md3
} modtype_t;


// typedef enum {mod_bad, mod_brush, mod_sprite, mod_alias } modtype_t;

typedef struct {
	float s, t;
} fstvert_t;


typedef struct {
	int n[3];
} neighbors_t;

typedef struct model_s {

	char		name[MAX_QPATH];
	int			registration_sequence;
	int			max_meshes;
	modtype_t	type;
	int			numFrames;
	int			flags;

	//
	// volume occupied by the model graphics
	//
	vec3_t		mins, maxs, center;
	vec3_t		bbox[8];
	float		radius;
	//
	// solid volume for clipping
	//
	qboolean	clipbox;
	vec3_t		clipmins, clipmaxs;
	//
	// brush model
	//
	int			firstModelSurface,
		numModelSurfaces,
		lightmap;				// only for subModels

	int			numSubModels;
	mmodel_t	*subModels;

	int			numPlanes;
	cplane_t	*planes;

	int			numLeafs;				// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numVertexes;
	mvertex_t	*vertexes;

	int			numEdges;
	medge_t		*edges;

	int			numNodes;
	int			firstNode;
	mnode_t		*nodes;

	int			numTexInfo;
	mtexInfo_t	*texInfo;

	int			numSurfaces;
	msurface_t	*surfaces;

	int			numSurfEdges;
	int			*surfEdges;

	int			numMarkSurfaces;
	msurface_t	**markSurfaces;

	dvis_t		*vis;

	int			lightmap_scale;
	byte		*lightData;
	qboolean	useXPLM;	// 3-vector basis lightmap
	
	vec3_t		fogColor, fogSkyColor;
	float		fogDensity, fogSkyDensity, fogBias, fogSkyBias;
	int			fogType;
	qboolean	useFogFile;

	// for alias models and skins
	image_t		*skins				[MAX_MD2SKINS];
	image_t		*skins_normal		[MAX_MD2SKINS];
	image_t		*skins_specular		[MAX_MD2SKINS];
	image_t		*skins_roughness	[MAX_MD2SKINS];
	image_t		*glowtexture		[MAX_MD2SKINS];
	image_t		*skin_env			[MAX_MD2SKINS];

	image_t      *skinsMD3				[MD3_MAX_MESHES][MD3_MAX_SKINS];
	image_t      *skinsMD3_normal		[MD3_MAX_MESHES][MD3_MAX_SKINS];
	image_t      *skinsMD3_roughness	[MD3_MAX_MESHES][MD3_MAX_SKINS];
	image_t      *skinsMD3_glow			[MD3_MAX_MESHES][MD3_MAX_SKINS];
	image_t      *skinsMD3_env			[MD3_MAX_MESHES][MD3_MAX_SKINS];

	int			extraDataSize;
	void		*extraData;
	int			triangles[MAX_TRIANGLES];
	float		*st;
	neighbors_t *neighbours;

	float		ambient;
	float		diffuse;
	float		specular;
	float		alphaShift;
	float		glowCfg[3];
	float		envScale;
	float		modelScale;
	qboolean	noSelfShadow;
	qboolean	envMap;

	byte		*normals;
	byte		*binormals;
	byte		*tangents;

	int			*indexArray;
	int			numIndices;

	GLuint		vboId;
	int			memorySize;

	mat3_t		axis;
} model_t;


#define SHELL_SCALE		        0.5F
#define WEAPON_SHELL_SCALE		0.2F

//============================================================================

void Mod_Init (void);
model_t *Mod_ForName (char *name, qboolean crash);
mleaf_t *Mod_PointInLeaf (float *p, model_t * model);
byte *Mod_ClusterPVS (int cluster, model_t * model);

void Mod_Modellist_f (void);

void *Hunk_Begin (int maxsize, char *name);
void *Hunk_Alloc (int size);
int Hunk_End (void);
void Hunk_Free (void *base);

void Mod_FreeAll (void);
void Mod_Free (model_t * mod);
