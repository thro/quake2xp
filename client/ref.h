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



#ifndef __REF_H
#define __REF_H

#include "../qcommon/qcommon.h"

#define PT_DEFAULT 0
#define PT_BUBBLE  1
#define PT_FLY     2
#define PT_BLOOD   3
#define PT_BLOOD2  4
#define PT_BLASTER 5
#define PT_SMOKE   6
#define PT_SPLASH  7
#define PT_SPARK   8
#define PT_BEAM    9
#define PT_SPIRAL  10
#define PT_FLAME   11
#define PT_BLOODSPRAY	12
#define PT_xBLOODSPRAY	13
#define	PT_EXPLODE		14
#define	PT_WATERPULME	15
#define	PT_WATERCIRCLE	16
#define PT_BLOODDRIP	17
#define PT_BLOODMIST	18
#define PT_BLOOD_SPLAT  19
#define PT_BLASTER_BOLT 20
#define PT_BFG_BALL		21
#define PT_BFG_REFR		22
#define PT_BFG_EXPL		23
#define PT_BFG_EXPL2	24
#define PT_MAX			25


#define DECAL_BULLET		0
#define DECAL_BLASTER		1
#define DECAL_EXPLODE		2
#define DECAL_RAIL			3
#define DECAL_BLOOD1			4
#define DECAL_BLOOD2			5
#define DECAL_BLOOD3			6
#define DECAL_BLOOD4			7
#define DECAL_BLOOD5			8
#define DECAL_BLOOD6			9
#define DECAL_BLOOD7			10
#define DECAL_BLOOD8			11
#define DECAL_BLOOD9			12
#define DECAL_ACIDMARK			13
#define DECAL_BFG				14


#define DECAL_MAX			15

#define DF_SHADE		0x00000400	// 1024
#define DF_NOTIMESCALE	0x00000800	// 2048
#define INSTANT_DECAL	-10000.0
#define DF_OVERBRIGHT	1
#define DF_VERTEXLIGHT	2
#define DF_AREADECAL	4

#define	MAX_DLIGHTS		32
#define	MAX_ENTITIES	128
#define	MAX_PARTICLES	4096
#define	MAX_LIGHTSTYLES	256

extern vec3_t r_origin;

typedef vec_t vec2_t[2];

typedef vec3_t	mat3_t[3];		// column-major (axis)
typedef vec4_t	mat4_t[4];		// row-major

#define DEG2RAD(v) ((v) * (M_PI / 180.0f))
#define RAD2DEG(v) ((v) * (180.0f / M_PI))

//
// msurface_t->flags
//
#define	MSURF_PLANEBACK		0x2
#define	MSURF_DRAWSKY		0x4
#define MSURF_DRAWTURB		0x10
//#define MSURF_DRAWBACKGROUND	0x40
#define MSURF_UNDERWATER		0x80
#define MSURF_ENVMAP		0x100
#define MSURF_SSS			0x200
#define MSURF_WATER      	0x400
#define MSURF_SLIME      	0x800
#define MSURF_LAVA       	0x1000


#define POWERSUIT_SCALE		4.0F

#define SHELL_RED_COLOR		0xF2
#define SHELL_GREEN_COLOR	0xD0
#define SHELL_BLUE_COLOR	0xF3

#define SHELL_RG_COLOR		0xDC
//#define SHELL_RB_COLOR        0x86
#define SHELL_RB_COLOR		0x68
#define SHELL_BG_COLOR		0x78

//ROGUE
#define SHELL_DOUBLE_COLOR	0xDF	// 223
#define	SHELL_HALF_DAM_COLOR	0x90
#define SHELL_CYAN_COLOR	0x72
//ROGUE

#define SHELL_WHITE_COLOR	0xD7

#define	GL_INDEX_TYPE		GL_UNSIGNED_SHORT
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef ushort				index_t;
typedef uint				index32_t;
typedef const char			cchar;

void Set_FontShader (qboolean enable);

typedef struct entity_s {
	struct model_s *model;		// opaque type outside refresh
	float angles[3];
	mat3_t	axis;			// entity -> world space
	mat4_t	orMatrix, matrix;

	qboolean angleMod;
	/*
	 ** most recent data
	 */
	float	origin[3];				// also used as RF_BEAM's "from"
	int		frame;					// also used as RF_BEAM's diameter
	int		framecount;				// for vis calc

	/*
	 ** previous data for lerping
	 */
	float oldorigin[3];			// also used as RF_BEAM's "to"
	int oldframe;
	
	// iqm
	int iqmFrameTime;
	/*
	 ** misc
	 */
	float backlerp;				// 0.0 = current, 1.0 = old
	int skinnum;				// also used as RF_BEAM's palette index

	int lightstyle;				// for flashing entities
	float alpha;				// ignore if RF_TRANSLUCENT isn't set

	struct image_s *skin;		// NULL for inline skin
	int flags;
	vec3_t color;

	vec3_t lightvector;
	struct	image_s	*bump;
	float shadelight[3];
	float minmax[6];
	vec3_t mins;
	vec3_t maxs;
	qboolean lightVised;
	byte vis[MAX_MAP_LEAFS / 8];

} entity_t;

#define ENTITY_FLAGS  68

typedef struct {
	vec3_t origin, color, angles;
	float intensity, _cone;
	int filter;
	qboolean spotlight;

} dlight_t;

typedef struct {
	vec3_t origin;
	vec3_t mins;
	vec3_t maxs;
	vec3_t color;
	vec3_t length;
	vec3_t angle;
	vec3_t oldOrg;
	vec3_t dir;
	float alpha;
	int type;
	float orient;
	float len;
	int flags;
	float size;
	int sFactor;
	int dFactor;
	float time;
} particle_t;

#define PARTICLE_BOUNCE					1
#define PARTICLE_FRICTION				2
#define PARTICLE_DIRECTIONAL			4
#define PARTICLE_VERTEXLIGHT			8
#define PARTICLE_STRETCH				16
#define PARTICLE_UNDERWATER				32
#define PARTICLE_OVERBRIGHT				64
#define PARTICLE_SPIRAL					128
#define PARTICLE_AIRONLY				256
#define PARTICLE_LIGHTING				512
#define PARTICLE_ALIGNED				1024
#define PARTICLE_NONSOLID				2048
#define PARTICLE_STOPED					4096
#define PARTICLE_CLAMP					8192
#define PARTICLE_NOFADE					16384
#define PARTICLE_DEFAULT				32768
#define PARTICLE_ROTATE					65536
#define PARTICLE_SOFT_MIDLE				131072

#define CLM_BOUNCE			1
#define CLM_FRICTION		2
#define CLM_DIRECTIONAL		4
#define CLM_ROTATE			8
#define CLM_STOPPED			16
#define CLM_STRETCH			32
#define CLM_MSHELL			64
#define CLM_NOSHADOW		128

typedef struct {
	float rgb[3];				// 0.0 - 2.0
	float white;				// highest of rgb
} lightstyle_t;

typedef struct {
	float strength;
	vec3_t direction;
	vec3_t color;
} m_dlight_t;


/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

  */

typedef enum {
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky,
	it_autobump,
	it_parallax,
	it_wall2,
	it_bump,
} imagetype_t;

typedef unsigned long int uint64;

typedef struct image_s {

	char		name[MAX_QPATH];		// game path, including extension
	char		bare_name[MAX_QPATH];	// filename only, as called when searching
	imagetype_t type;
	int			width, height;			// source image
	int			upload_width, upload_height;	// after power of two and picmip
	int			registration_sequence;	// 0 = free
	struct		msurface_s *texturechain;	// for sort-by-texture world
	// drawing
	int			texnum;					// gl texture binding
	float		sl, tl, sh, th;		// 0,0 - 1,1 unless part of the scrap
	qboolean	has_alpha;
	qboolean	autobump;
	qboolean	paletted;
	qboolean	is_cin;
	qboolean	envMap;

	float		picScale_w;
	float		picScale_h;
	index_t		*index;
	uint		target, id;
	float		parallaxScale,
				specularScale,
				SpecularExp,
				envScale, 
				rghScale;

	//bindless graphics
	uint64		handle;
	qboolean	hasHandle;
	
	//lut description
	float		lutSize;
	char		*lutName;
} image_t;


typedef struct mtexInfo_s {
	float vecs[2][4];
	int flags;
	int numFrames;
	struct mtexInfo_s *next;	// animation chain

	image_t *image;
	image_t *normalmap;
	image_t *addTexture;
	image_t *envTexture;
	image_t *rghMap;

	int value;

} mtexInfo_t;

extern int	c_brush_polys,
c_alias_polys,
c_visible_textures,
c_visible_lightmaps,
c_flares,
c_shadow_volumes,
c_decals,
c_shadow_tris,
c_part_tris,
c_decal_tris;

extern int num_visLights;

#define	VERTEXSIZE	16

typedef struct glpoly_s {

	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	struct	glpoly_s	**neighbours;

	vec3_t	normal;
	vec3_t	center;
	int		lightTimestamp;
	int		shadowTimestamp;
	int		ShadowedFace;
	int		numVerts;
	int		flags;
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;

//temporaly storage for polygons that use an edge
typedef struct {
	byte		used;		//how many polygons use this edge
	glpoly_t	*poly[2];	//pointer to the polygons who use this edge
} temp_connect_t;

temp_connect_t	*tempEdges;

typedef struct msurface_s {
	int visframe;				// should be drawn when node is crossed

	cplane_t *plane;
	int flags;

	int firstedge;				// look up in model->surfEdges[], negative
	// numbers
	int numEdges;				// are backwards edges

	short texturemins[2];
	short extents[2];

	int light_s, light_t;		// gl lightmap coordinates
	int dlight_s, dlight_t;		// gl lightmap coordinates for dynamic
	// lightmaps

	glpoly_t *polys;			// multiple if warped

	struct msurface_s *texturechain;
	struct msurface_s *lightmapchain;

	mtexInfo_t *texInfo;

	float c_s, c_t;

	// lighting info
	int dlightframe;
	int dlightbits;

	int lightmaptexturenum;

	byte styles[MAXLIGHTMAPS];
	float cached_light[MAXLIGHTMAPS];	// values currently used in
	// lightmap
	byte *samples;				// [numstyles * surfsize * 3] for vanilla or [numstyles * 3 * surfsize * 3] for XP lightmaps

	int checkCount;
	vec3_t center;

	struct msurface_s *fogchain;
	int fragmentframe;
	entity_t *ent;
	vec3_t		mins, maxs;

	int	numIndices;
	int	numVertices;
	index_t	*indices;
	unsigned int sort;

	//vbo
	size_t vbo_pos;
	int	xyz_size;
	int st_size;
	int lm_size;

	int	baseIndex;
	int numIdx;

} msurface_t;

typedef struct mnode_s {
	// common with leaf
	int contents;				// -1, to differentiate from leafs
	int visframe;				// node needs to be traversed if current

	float minmaxs[6];			// for bounding box culling

	struct mnode_s *parent;

	// node specific
	cplane_t *plane;
	struct mnode_s *children[2];

	unsigned short firstsurface;
	unsigned short numsurfaces;
} mnode_t;

/*
=====================
DECALS

=====================
*/

#define MAX_DECALS				8192
#define MAX_DECAL_VERTS			384
#define MAX_DECAL_FRAGMENTS		256

typedef struct decals_t {
	struct decals_t *prev, *next;
	mnode_t *node;

	float time, endTime;

	int numverts;
	int numIndices;
	index_t	*indices;

	vec3_t verts[MAX_DECAL_VERTS];
	vec2_t stcoords[MAX_DECAL_VERTS];
	vec3_t color;
	vec3_t endColor;
	float alpha;
	float endAlpha;
	float size;
	vec3_t org;
	int type;
	int flags;
	int sFactor;
	int dFactor;
} decals_t;



typedef struct {
	mnode_t *node;
	msurface_t *surf;

	int firstvert;
	int numverts;
} fragment_t;

//================
// end decals
//================


#define	MAX_FRAME_BUFFERS	32

typedef struct {
	unsigned int	format;
	unsigned int	id;
	int				width;
	int				height;

} rbo_t;

typedef struct {
	char			name[MAX_QPATH];
	int				index;	// in rg.fbs
	unsigned int	id;

} fbo_t;

typedef struct {
	int		x, y, width, height;	// in virtual screen coordinates
	float	fov_x, fov_y;
	float	vieworg[3];
	float	vieworg_old[3];
	float	viewangles[3];
	float	viewanglesOld[3];

	float blend[4];				// rgba 0-1 full screen blend
	float time;					// time is uesed to auto animate
	int rdflags;				// RDF_UNDERWATER, etc
	qboolean mirrorView;
	byte *areabits;				// if not NULL, only areas with set bits
	// will be drawn

	// world bounds
	vec3_t			visMins;
	vec3_t			visMaxs;

	lightstyle_t *lightstyles;	// [MAX_LIGHTSTYLES]

	// viewport
	int		viewport[4];
	vec3_t	cornerRays[4];
	vec2_t	depthParms;
	mat3_t	axis;
	mat4_t	projectionMatrix;
	mat4_t	orthoMatrix;
	mat4_t	modelViewMatrix;
	mat4_t	modelViewProjectionMatrix;
	mat4_t	modelViewProjectionMatrixTranspose;
	mat4_t	skyMatrix;

	int num_entities;
	entity_t *entities;

	int num_dlights;
	dlight_t *dlights;

	int num_particles;
	particle_t *particles;

	int numDecals;
	decals_t *decals;

	int			numFBs;
	fbo_t		*fbs[MAX_FRAME_BUFFERS];
	fbo_t		*screenFB;
	fbo_t		*hdrFB;
	image_t		*depthBufferImage;	// depth24-stencil8 format
	image_t		*colorBufferImage; //  screen texture

} refdef_t;

extern float loadScreenColorFade;
extern char *sInf;

//sul
#define MAX_RADAR_ENTS 512
typedef struct RadarEnt_s {
	float color[3];
	vec3_t org;
} RadarEnt_t;


extern int numRadarEnts;
extern RadarEnt_t RadarEnts[MAX_RADAR_ENTS];

#define	API_VERSION		3

//
// these are the functions exported by the refresh module
//

int R_GetClippedFragments (vec3_t origin, float radius, mat3_t axis,
	int maxfverts, vec3_t * fverts, int maxfragments,
	fragment_t * fragments);
void Draw_Pic (int x, int y, char *name);
void Draw_Pic2 (int x, int y, image_t * gl);
void Draw_StretchPic2 (int x, int y, int w, int h, image_t * gl);
void Draw_StretchPic (int x, int y, int w, int h, char *name);


void Draw_PicScaled (int x, int y, float scale_x, float scale_y, char *pic);
void Draw_PicBumpScaled(int x, int y, float scale_x, float scale_y, char *pic, char *pic2);
void Draw_ScaledPic (int x, int y, float scale_x, float scale_y, image_t * gl);

void Draw_CharScaled (int x, int y, float scale_x, float scale_y, unsigned char num);
void Draw_StringScaled (int x, int y, float scale_x, float scale_y, const char *str);

void Draw_TileClear (int x, int y, int w, int h, char *name);
void Draw_TileClear2 (int x, int y, int w, int h, image_t * image);
void Draw_Fill (int x, int y, int w, int h, float r, float g, float b, float a);
void R_BeginRegistration (char *map);
void R_SetSky (char *name, float rotate, vec3_t axis);
void R_EndRegistration (void);
void R_RenderFrame (refdef_t * fd);
void Draw_GetPicSize (int *w, int *h, char *name);	// will return 0 0 if
// not found
void R_ModelBounds (struct model_s * model, vec3_t mins, vec3_t maxs);
void R_ModelRadius (struct model_s * model, vec3_t rad);
void R_ModelCenter (struct model_s * model, vec3_t center);


void R_Shutdown (void);
qboolean R_CullPoint (vec3_t org);
int R_Init (void *hinstance, void *wndproc);
image_t *Draw_FindPic (char *name);
struct model_s *R_RegisterModel (char *name);
struct image_s *R_RegisterSkin (char *name);
image_t *Draw_FindPic (char *name);
void R_SetPalette (const unsigned char *palette);
void R_BeginFrame ();
void GLimp_EndFrame (void);
void GLimp_AppActivate (qboolean active);
void VID_NewWindow (int width, int height);
qboolean VID_GetModeInfo (int *width, int *height, int mode);
void VectorNormalizeFast(vec3_t v);

struct sfx_s;

//
// these are the functions imported by the refresh module
//

void Con_Printf (int print_level, char *str, ...);
cvar_t *Cvar_Get (char *name, char *value, int flags);
void Cvar_SetValue (char *name, float value);
cvar_t *Cvar_Set (char *name, char *value);
void Cmd_AddCommand (char *name, void (*cmd) (void));
void Cmd_RemoveCommand (char *name);
int Cmd_Argc (void);
char *Cmd_Argv (int i);
void VID_Error (int err_level, char *str, ...);
void Cbuf_ExecuteText (int exec_when, char *text);
int FS_LoadFile (const char *name, void **buf);
void FS_FreeFile (void *buf);
char *FS_Gamedir (void);

#endif							// __REF_H
