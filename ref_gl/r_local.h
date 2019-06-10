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

#ifndef R_LOCAL_H
#define R_LOCAL_H


#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glu.h>
#include "glcorearb.h"

#include <stdio.h>
#include <math.h>

#include "../client/ref.h"
#include "r_md3.h"

#ifdef _WIN32

#include "wglext.h"
#include "imagelib/ilut_config.h"
#include "imagelib/il.h"
#include "imagelib/ilu.h"
#include "imagelib/ilut.h"
typedef void ILvoid;
#define _inline inline

#else

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

typedef void ILvoid;
#define _inline inline
#endif

#include "qgl.h"

#ifdef _WIN32
	#include "../win32/adl/adl_sdk.h"
	#include "../win32/nvapi/nvapi.h"
#endif

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


//#ifndef __VIDDEF_T
//#define __VIDDEF_T
#ifndef VIDDEF_LOCK
#define VIDDEF_LOCK
typedef struct {
	unsigned width, height;		// coordinates from main game
} viddef_t;
#endif

extern viddef_t vid;

#define	TEXNUM_LIGHTMAPS	32768
#define	MAX_GLTEXTURES		32768 

#define BUFFER_OFFSET(i) ((byte *)NULL + (i))

#define MAX_DRAW_STRING_LENGTH  512
#define MAX_IDX 65536
// ===================================================================

typedef enum {
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "r_model.h"

void GL_SetDefaultState (void);
void GL_UpdateSwapInterval (void);

extern double gldepthmin, gldepthmax;


typedef struct {
	float x, y, z;
	float s, t;
	float r, g, b;
} glvert_t;



#define BACKFACE_EPSILON	0.01


//====================================================

#define		MAX_CAUSTICS		32
image_t *r_caustic[MAX_CAUSTICS];

#define		MAX_WATER_NORMALS		32
image_t *r_waterNormals[MAX_WATER_NORMALS];

#define		MAX_FLY		2
image_t *fly[MAX_FLY];

#define		MAX_FLAMEANIM		5
image_t *flameanim[MAX_FLAMEANIM];

#define		MAX_BLOOD 6
image_t *r_blood[MAX_BLOOD];

#define		MAX_xBLOOD 6
image_t *r_xblood[MAX_BLOOD];

#define	MAX_SHELLS 6
image_t	*r_texshell[MAX_SHELLS];

#define		MAX_EXPLODE 8
image_t *r_explode[MAX_EXPLODE];

#define		MAX_BFG_EXPL		32
image_t *r_bfg_expl[MAX_BFG_EXPL];

extern qboolean drawFlares;
extern image_t gltextures[MAX_GLTEXTURES];
extern int numgltextures;

image_t *r_notexture;
image_t *r_distort;
image_t *r_predator;
image_t *depthMap;

image_t *r_particletexture[PT_MAX];
image_t *r_decaltexture[DECAL_MAX];


image_t *r_flare;

image_t *draw_chars;
image_t *r_DSTTex;

image_t	*r_defBump;
image_t	*ScreenMap;
image_t	*Screen2D;
image_t	*r_envTex;
image_t	*r_randomNormalTex;
image_t	*shadowMask;
image_t	*r_conBump;
image_t	*weaponHack;
image_t *fxaaMap;
image_t *fboScreen;
image_t *fboScreenCopy;

image_t	*r_whiteMap;
image_t *skinBump;

#define MAX_FILTERS 256
image_t	*r_lightCubeMap[MAX_FILTERS];
#define	MAX_GLOBAL_FILTERS	37
#define MAX_LUTS 10
image_t *r_3dLut[MAX_LUTS];


image_t *fboDN;
image_t *fboColor[2];
image_t *ScreenCapture;

uint bloomtex;
uint thermaltex;
uint fxaatex;
uint fovCorrTex;
int	skyCube;

uint fboId;
byte fboColorIndex;
uint fboDps;

image_t	*skyMask;
uint fbo_skyMask;

extern entity_t *currententity;
extern model_t *currentmodel;
extern int r_visframecount;
extern int r_framecount;
extern cplane_t frustum[6];

extern	int gl_filter_min, gl_filter_max;

int lutCount;

//
// view origin
//
extern vec3_t vup;
extern vec3_t vpn;
extern vec3_t vright;
extern vec3_t r_origin;
extern entity_t r_worldentity;

//
// screen size info
//
extern refdef_t r_newrefdef;
extern int r_viewcluster, r_viewcluster2, r_oldviewcluster,
r_oldviewcluster2;

cvar_t *r_noRefresh;
cvar_t *r_drawEntities;
cvar_t *r_drawWorld;
cvar_t *r_speeds;
cvar_t *r_noVis;
cvar_t *r_noCull;
cvar_t *r_leftHand;
cvar_t *r_lightLevel;
cvar_t *r_mode;
cvar_t *r_noBind;
cvar_t *r_cull;
cvar_t *r_vsync;
cvar_t *r_textureMode;

cvar_t *r_imageAutoBump;
cvar_t *r_imageAutoBumpScale;
cvar_t *r_imageAutoSpecularScale;

cvar_t *r_lockPvs;
cvar_t *r_fullScreen;

cvar_t *r_brightness;
cvar_t *r_contrast;
cvar_t *r_saturation;
cvar_t *r_gamma;

cvar_t	*r_colorVibrance;
cvar_t	*r_colorBalanceRed;
cvar_t	*r_colorBalanceGreen;
cvar_t	*r_colorBalanceBlue;

cvar_t *vid_ref;

cvar_t	*r_causticIntens;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;
cvar_t	*r_screenShotJpegQuality;

cvar_t	*r_textureColorScale;
cvar_t	*r_textureCompression;
cvar_t	*r_anisotropic;
cvar_t	*r_maxAnisotropy;
cvar_t	*r_textureLodBias;

cvar_t	*r_shadows;
cvar_t	*r_playerShadow;

cvar_t	*r_multiSamples;
cvar_t	*r_fxaa;
cvar_t	*deathmatch;

cvar_t	*r_drawFlares;
cvar_t	*r_scaleAutoLightColor;
cvar_t	*r_lightWeldThreshold;

cvar_t	*r_customWidth;
cvar_t	*r_customHeight;

cvar_t	*r_bloom;
cvar_t	*r_bloomThreshold;
cvar_t	*r_bloomIntens;
cvar_t	*r_bloomWidth;

cvar_t	*r_ssao;
cvar_t	*r_ssaoIntensity;
cvar_t	*r_ssaoScale;
cvar_t	*r_ssaoBlur;

cvar_t	*r_useBlinnPhongLighting;
cvar_t	*r_skipStaticLights;
cvar_t	*r_lightmapScale;
cvar_t	*r_lightsWeldThreshold;
cvar_t	*r_debugLights;
cvar_t	*r_useLightScissors;
cvar_t	*r_useDepthBounds;
cvar_t	*r_specularScale;
cvar_t	*r_ambientSpecularScale;
cvar_t	*r_useRadiosityBump;
cvar_t	*r_zNear;
cvar_t	*r_zFar;
cvar_t	*r_useLightOcclusions;

cvar_t	*hunk_bsp;
cvar_t	*hunk_md2;
cvar_t	*hunk_md3;
cvar_t	*hunk_sprite;

cvar_t	*r_maxTextureSize;

cvar_t	*r_reliefMapping;
cvar_t	*r_reliefScale;
cvar_t	*r_reliefMappingSelfShadow;
cvar_t	*r_reliefMappingSelfShadowOffset;

cvar_t	*r_dof;
cvar_t	*r_dofBias;
cvar_t	*r_dofFocus;

cvar_t	*r_motionBlur;
cvar_t	*r_motionBlurSamples;
cvar_t	*r_motionBlurFrameLerp;

cvar_t	*r_radialBlur;
cvar_t	*r_radialBlurFov;

cvar_t	*r_globalFog;
cvar_t	*r_globalFogDensity;
cvar_t	*r_globalFogRed;
cvar_t	*r_globalFogGreen;
cvar_t	*r_globalFogBlue;
cvar_t	*r_globalFogBias;

cvar_t	*r_globalSkyFogRed;
cvar_t	*r_globalSkyFogGreen;
cvar_t	*r_globalSkyFogBlue;
cvar_t	*r_globalSkyFogDensity;
cvar_t	*r_globalSkyFogBias;

cvar_t	*r_tbnSmoothAngle;

cvar_t	*r_glDebugOutput;
cvar_t	*r_glMinorVersion;
cvar_t	*r_glMajorVersion;
cvar_t	*r_glCoreProfile;

cvar_t	*r_lightEditor;
cvar_t	*r_cameraSpaceLightMove;

cvar_t	*r_hudLighting;
cvar_t	*r_bump2D;

cvar_t	*r_filmFilter;
cvar_t	*r_filmFilterNoiseIntens;
cvar_t	*r_filmFilterScratchIntens;
cvar_t	*r_filmFilterVignetIntens;

cvar_t	*r_fixFovStrength; // 0.0 = no hi-fov perspective correction
cvar_t	*r_fixFovDistroctionRatio; // 0.0 = cylindrical distortion ratio. 1.0 = spherical

cvar_t	*r_screenBlend;

cvar_t	*r_useShaderCache;
cvar_t	*r_particlesOverdraw;

cvar_t	*r_lutId;
cvar_t	*r_colorTempK;

int CL_PMpointcontents (vec3_t point);
qboolean outMap;

extern float ref_realtime;

extern int r_visframecount;

qboolean xhargar2hack;
qboolean RA_Frame;
msurface_t *r_alpha_surfaces;		// all non-entity BSP surfaces with TRANS33/66
msurface_t *r_reflective_surfaces;	// all non-entity BSP surfaces with WARP

void GL_Bind (int texnum);
void GL_MBind (GLenum target, int texnum);
void GL_SelectTexture (GLenum);
void GL_MBindCube (GLenum target, int texnum);

void R_LightPoint (vec3_t p, vec3_t color);

void R_InitLightgrid (void);

void R_GenEnvCubeMap();
void R_Capture2dColorBuffer();

worldShadowLight_t *R_AddNewWorldLight (vec3_t origin, vec3_t color, float radius[3], int style, int filter, vec3_t angles, vec3_t speed,
	qboolean isStatic, int isShadow, int isAmbient, float cone, qboolean ingame, int flare, vec3_t flareOrg,
	float flareSize, char target[MAX_QPATH], int start_off, int fog, float fogDensity);
void R_DrawParticles ();
void R_RenderDecals (void);
void R_LightColor (vec3_t org, vec3_t color);
qboolean R_CullAliasModel (vec3_t bbox[8], entity_t *e);
int CL_PMpointcontents2 (vec3_t point, struct model_s * ignore);
void VID_MenuInit (void);
void AnglesToMat3 (const vec3_t angles, mat3_t m);
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out);
void R_ShutdownPrograms (void);
void GL_BindRect (int texnum);
void GL_MBindRect (GLenum target, int texnum);
void R_Bloom (void);
void R_ThermalVision (void);
void R_RadialBlur (void);
void R_DofBlur (void);
void R_FXAA (void);
void R_FilmFilter (void);
void R_FixFov(void);
void R_lutCorrection(void);
void R_ListPrograms_f (void);
void R_InitPrograms (void);
void R_ClearWorldLights (void);
qboolean R_CullSphere (const vec3_t centre, const float radius);
void R_CastBspShadowVolumes (void);
void R_CastAliasShadowVolumes (qboolean player);
void R_DrawAliasModelLightPass (qboolean weapon_model);
void R_SetupEntityMatrix (entity_t * e);
void GL_MBind3d (GLenum target, int texnum);
void R_SSAO(void);
void R_DrawDepthScene(void);
void R_DownsampleDepth(void);
void R_ScreenBlend(void);
void R_GlobalFog();

void CreateFboBuffer(void);
void R_CopyFboColorBuffer();

void R_SaveLights_f (void);
void R_Light_Spawn_f (void);
void R_Light_Delete_f (void);
void R_EditSelectedLight_f (void);
void R_MoveLightToRight_f (void);
void R_MoveLightForward_f (void);
void R_MoveLightUpDown_f (void);
void R_Light_SpawnToCamera_f (void);
void R_ChangeLightRadius_f (void);
void R_Light_Clone_f (void);
void R_ChangeLightCone_f (void);
void R_Light_UnSelect_f (void);
void R_FlareEdit_f (void);
void R_ResetFlarePos_f (void);
void R_Copy_Light_Properties_f (void);
void R_Paste_Light_Properties_f (void);

void R_SaveFogScript_f(void);
void R_RemoveFogScript_f(void);

extern qboolean flareEdit;

void R_CalcCubeMapMatrix (qboolean model);
void DeleteShadowVertexBuffers (void);
void MakeFrustum4Light (worldShadowLight_t *light, qboolean ingame);
qboolean R_CullConeLight (vec3_t mins, vec3_t maxs, cplane_t *frust);
void GL_DrawAliasFrameLerpLight (dmdl_t *paliashdr);
qboolean SurfInFrustum (msurface_t *s);
qboolean HasSharedLeafs (byte *v1, byte *v2);
qboolean InLightVISEntity ();
void R_DrawLightBrushModel ();
void UpdateLightEditor (void);
void Load_LightFile ();
qboolean BoundsIntersectsPoint (vec3_t mins, vec3_t maxs, vec3_t p);
extern int lightsQueries[MAX_WORLD_SHADOW_LIHGTS];
extern int numLightQ;
extern int numFlareOcc;
extern qboolean FoundReLight;
qboolean PF_inPVS (vec3_t p1, vec3_t p2);
void R_SetFrustum (void);
void R_SetViewLightScreenBounds ();
qboolean BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
void R_DrawLightFlare ();
void R_DrawLightBounds(void);

void R_ShutDownVertexBuffers();

extern const mat3_t	mat3_identity;
extern const mat4_t	mat4_identity;

void Mat3_Identity (mat3_t m);
void Mat3_Copy (const mat3_t in, mat3_t out);

void Mat4_Multiply (const mat4_t a, const mat4_t b, mat4_t out);
void Mat4_Copy (const mat4_t in, mat4_t out);
void Mat4_Transpose (const mat4_t in, mat4_t out);
void Mat4_MultiplyVector (const mat4_t m, const vec3_t in, vec3_t out);
void Mat4_Translate (mat4_t m, float x, float y, float z);
void Mat4_Scale (mat4_t m, float x, float y, float z);
qboolean Mat4_Invert (const mat4_t in, mat4_t out);
void Mat4_TransposeMultiply (const mat4_t a, const mat4_t b, mat4_t out);
void Mat4_SetOrientation (mat4_t m, const mat3_t rotation, const vec3_t translation);
void Mat4_Identity (mat4_t mat);
void Mat4_Rotate (mat4_t m, float angle, float x, float y, float z);
void Mat4_AffineInvert(const mat4_t in, mat4_t out);
void Mat4_SetupTransform(mat4_t m, const mat3_t rotation, const vec3_t translation);
void Mat3_Set(mat3_t mat, vec3_t x, vec3_t y, vec3_t z);
void Mat4_Set(mat4_t mat, vec4_t x, vec4_t y, vec4_t z, vec4_t w);
void VectorLerp(const vec3_t from, const vec3_t to, float frac, vec3_t out);

qboolean Frustum_CullBoundsProjection(const vec3_t mins, const vec3_t maxs, const vec3_t projOrigin, const int planeBits);
qboolean Frustum_CullLocalBoundsProjection(const vec3_t mins, const vec3_t maxs, const vec3_t origin, const mat3_t axis, const vec3_t projOrigin, const int planeBits);

qboolean Mat3_IsIdentity(const mat3_t mat);
void Mat3_MultiplyVector(const mat3_t m, const vec3_t in, vec3_t out);

void SetPlaneType (cplane_t *plane);
void SetPlaneSignBits (cplane_t *plane);

trace_t CL_PMTraceWorld (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask, qboolean checkAliases);
void AddBoundsToBounds(const vec3_t mins1, const vec3_t maxs1, vec3_t mins2, vec3_t maxs2);

void R_DrawChainsRA(qboolean bmodel);
void R_DrawBrushModelRA(void);

void R_DrawMD3Mesh(qboolean weapon);
void R_DrawMD3MeshLight(qboolean weapon);
void R_DrawMD3ShellMesh(qboolean weapon);
void CheckEntityFrameMD3(md3Model_t *paliashdr);
qboolean R_CullMD3Model(vec3_t bbox[8], entity_t *e);

qboolean R_AliasInLightBound();
void R_UpdateLightAliasUniforms();

void R_InitVertexBuffers();

void SetModelsLight();
extern float shadelight[3];
byte Normal2Index(const vec3_t vec);
extern int	occ_framecount;
void R_ColorTemperatureCorrection(void);

//====================================================================

#define MAX_POLY_VERT		128
#define	MAX_BATCH_SURFS		21845

extern vec3_t	wVertexArray[MAX_BATCH_SURFS];

extern float	wTexArray[MAX_BATCH_SURFS][2];
extern float	wLMArray[MAX_BATCH_SURFS][2];
extern vec4_t   wColorArray[MAX_BATCH_SURFS];


extern vec3_t	nTexArray[MAX_BATCH_SURFS];
extern vec3_t	tTexArray[MAX_BATCH_SURFS];
extern vec3_t	bTexArray[MAX_BATCH_SURFS];

extern float   wTmu0Array[MAX_BATCH_SURFS][2];
extern float   wTmu1Array[MAX_BATCH_SURFS][2];
extern float   wTmu2Array[MAX_BATCH_SURFS][2];

extern uint		indexArray[MAX_MAP_VERTS * 3];

extern model_t *r_worldmodel;

extern unsigned d_8to24table[256];
extern float	d_8to24tablef[256][3];

extern int registration_sequence;

extern float skyrotate;
extern vec3_t skyaxis;

int R_Init (void *hinstance, void *hWnd);
void R_Shutdown (void);
void GL_CheckError(const char *fileName, int line, const char *subr);

void R_RenderView (refdef_t * fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t * e);
void R_DrawBrushModel ();
void R_DrawSpriteModel (entity_t * e);
void R_DrawBeam ();
void R_DrawBSP (void);
void R_InitEngineTextures (void);
void R_LoadFont (void);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_MarkLeaves (void);
void R_DrawWaterPolygons (msurface_t * fa, qboolean bmodel);
void R_AddSkySurface (msurface_t * fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (qboolean color);

void COM_StripExtension (char *in, char *out);

void Draw_GetPicSize (int *w, int *h, char *name);
void Draw_Pic (int x, int y, char *name);
void Draw_StretchPic (int x, int y, int w, int h, char *name);
void Draw_TileClear (int x, int y, int w, int h, char *name);
void Draw_Fill (int x, int y, int w, int h, float r, float g, float b, float a);
void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows,
	byte * data);

void R_BeginFrame ();
void R_SetPalette (const unsigned char *palette);

int Draw_GetPalette (void);

struct image_s *R_RegisterSkin (char *name);

image_t *GL_LoadPic (char *name, byte * pic, int width, int height,
	imagetype_t type, int bits);

image_t *GL_FindImage (char *name, imagetype_t type);

void GL_TextureMode (char *string);
void GL_ImageList_f (void);

void GL_InitImages (void);
void GL_ShutdownImages (void);

void GL_FreeUnusedImages (void);
qboolean R_CullOrigin (vec3_t origin);
qboolean IsExtensionSupported(const char *name);

/*
** GL extension emulation functions
*/

void	CreateSSAOBuffer();
void CreateFboBuffer (void);

/*
** GL config stuff
*/


typedef struct {
	int renderer;
	const char	*renderer_string;
	const char	*vendor_string;
	const char	*version_string;
	const char	*extensions3_string;

	int			screenTextureSize;
	const char	*wglExtensionsString;

	const char	*shadingLanguageVersionString;
	int			maxVertexUniformComponents;		// GLSL info
	int			maxVaryingFloats;
	int			maxVertexTextureImageUnits;
	int			maxCombinedTextureImageUnits;
	int			maxFragmentUniformComponents;
	int			maxVertexAttribs;
	int			maxTextureImageUnits;
	int			maxUniformLocations;

	int			glMajorVersion;
	int			glMinorVersion;

	int			colorBits;
	int			alphaBits;
	int			depthBits;
	int			stencilBits;
	int			samples;
	int			maxSamples;
} glconfig_t;


typedef struct {
	qboolean fullscreen;

	int prev_mode;

	int lightmap_textures;
	int currenttextures[32]; // max gl_texturesXX
	int currenttmu;

	qboolean	texture_compression_bptc;
	int			displayrefresh;
	int			monitorWidth, monitorHeight;

	qboolean	wgl_no_error;
	qboolean	wgl_swap_control_tear;
	qboolean	depthBoundsTest;

	int			numFormats, binaryFormats;
	int			programId;
	GLenum		matrixMode;
	
	int			vaoBuffer, vboBuffer;

	mat4_t		projectionMatrix;
	mat4_t		modelViewMatrix;		// ready to load

	// frame buffer
	int			maxRenderBufferSize;
	int			maxColorAttachments;
	int			maxSamples;
	int			maxDrawBuffers;

	// gl state cache
	qboolean		cullFace;
	GLenum			cullMode;
	GLenum			frontFace;

	qboolean		blend;
	GLenum			blendSrc;
	GLenum			blendDst;
	GLenum			alphaFunc;
	GLclampf		alphaRef;

	GLboolean		colorMask[4];

	qboolean		depthTest;
	GLenum			depthFunc;
	GLboolean		depthMask;
	GLclampd		depthRange[2];

	qboolean		polygonOffsetFill;
	GLfloat			polygonOffsetFactor;
	GLfloat			polygonOffsetUnits;

	qboolean		lineSmooth;

	qboolean		depthClamp;
	
	qboolean		alphaTest;

	qboolean		stencilTest;
	GLenum			stencilFunc;
	GLenum			stencilFace;
	GLuint			stencilMask;
	GLint			stencilRef;
	GLuint			stencilRefMask;
	GLenum			stencilFail;
	GLenum			stencilZFail;
	GLenum			stencilZPass;

	qboolean		scissorTest;
	GLint			scissor[4];

	qboolean		glDepthBoundsTest;
	GLfloat			depthBoundsMins;
	GLfloat			depthBoundsMax;

	vec4_t			fontColor;
} glstate_t;

typedef struct {

GLuint	vbo_fullScreenQuad;
GLuint	vbo_halfScreenQuad;
GLuint	vbo_quarterScreenQuad;
GLuint	ibo_quadTris;
GLuint	vbo_Dynamic;
GLuint	ibo_Dynamic;
GLuint	ibo_cube;
GLuint	ibo_md3Shadow;

GLuint	vbo_BSP;
GLuint	ibo_BSP;

int xyz_offset;
int st_offset;
int lm_offset;
int nm_offset;
int tg_offset;
int bn_offset;
}vbo_t;

vbo_t vbo;

typedef struct {
	GLuint	bsp_a; 
	GLuint	bsp_l;
	GLuint	alias_shadow;
	GLuint	fonts;
	GLuint	fullscreenQuad;
	GLuint	halfScreenQuad;
	GLuint	quaterScreenQuad;
	GLuint	base;
}vao_t;

vao_t vao;

void GL_CullFace (GLenum mode);
void GL_FrontFace (GLenum mode);

void GL_DepthFunc (GLenum func);
void GL_DepthMask (GLboolean flag);
void GL_BlendFunc (GLenum src, GLenum dst);
void GL_ColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

void GL_StencilMask (GLuint mask);
void GL_StencilFunc (GLenum func, GLint ref, GLuint mask);
void GL_StencilOp (GLenum fail, GLenum zFail, GLenum zPass);
void GL_StencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask);
void GL_StencilOpSeparate (GLenum face, GLenum fail, GLenum zFail, GLenum zPass);

void GL_Scissor (GLint x, GLint y, GLint width, GLint height);
void GL_DepthRange (GLclampd n, GLclampd f);
void GL_PolygonOffset (GLfloat factor, GLfloat units);
void GL_DepthBoundsTest (GLfloat mins, GLfloat maxs);

void GL_Enable (GLenum cap);
void GL_Disable (GLenum cap);

#ifndef BIT
#define BIT(num)				(1 << (num))
#endif


extern glconfig_t gl_config;
extern glstate_t gl_state;

extern int	g_numGlLights;

extern	vec3_t	lightspot;

#define VA_SetElem2(v,a,b)		((v)[0]=(a),(v)[1]=(b))
#define VA_SetElem3(v,a,b,c)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c))
#define VA_SetElem4(v,a,b,c,d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))

#define VA_SetElem2v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1])
#define VA_SetElem3v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2])
#define VA_SetElem4v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2],(v)[3]=(a)[3])

#define MAX_VERTICES		16384
#define MAX_INDICES			65536
#define MAX_VERTEX_ARRAY	8192
#define MAX_SHADOW_VERTS	16384

#define MAX_STREAM_VBO_VERTS MD3_MAX_TRIANGLES * MD3_MAX_MESHES
#define MAX_STREAM_IBO_IDX	 MD3_MAX_TRIANGLES * MD3_MAX_MESHES * 3

#define CUBE_INDICES 36

void R_PrepareShadowLightFrame (qboolean weapon);
extern worldShadowLight_t *shadowLight_static, *shadowLight_frame;
qboolean BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius);

#define Vector4Set(v, a, b, c, d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))
#define Vector4Copy(a,b) ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define PlaneDiff(point,plane) (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)

#define Vector4Scale(in,scale,out)		((out)[0]=(in)[0]*scale,(out)[1]=(in)[1]*scale,(out)[2]=(in)[2]*scale,(out)[3]=(in)[3]*scale)
#define Vector4Add(a,b,c)		((c)[0]=(((a[0])+(b[0]))),(c)[1]=(((a[1])+(b[1]))),(c)[2]=(((a[2])+(b[2]))),(c)[3]=(((a[3])+(b[3]))))
#define Vector4Sub(a,b,c)		((c)[0]=(((a[0])-(b[0]))),(c)[1]=(((a[1])-(b[1]))),(c)[2]=(((a[2])-(b[2]))),(c)[3]=(((a[3])-(b[3]))))


#define clamp(a,b,c)	((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

void Q_strncatz (char *dst, int dstSize, const char *src);

#define	MAX_LIGHTMAPS		4		// max number of atlases
#define	LIGHTMAP_SIZE		2048
#define GL_LIGHTMAP_FORMAT	GL_RGB
#define XPLM_NUMVECS		3

typedef struct {
	int internal_format;
	int current_lightmap_texture;

	msurface_t *lightmap_surfaces[MAX_LIGHTMAPS];

	int allocated[LIGHTMAP_SIZE];

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	byte lightmap_buffer[3][LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];
} gllightmapstate_t;

gllightmapstate_t gl_lms;
extern const vec3_t r_xplmBasisVecs[XPLM_NUMVECS];

/*
====================================================================

PROGRAMS

====================================================================
*/
#define	MAX_PROGRAM_UNIFORMS	32
#define	MAX_PROGRAM_DEFS	8					// max permutation defs program can have
#define	MAX_PROGRAM_ID		(1 << MAX_PROGRAM_DEFS)		// max GL indices per program object

#define	MAX_UNIFORM_NAME	64
#define	MAX_DEF_NAME		32
#define	MAX_PROGRAMS		256

typedef struct {
	char			name[MAX_UNIFORM_NAME];
} glslUniform_t;

typedef struct glslProgram_s {

	struct glslProgram_s	*nextHash;

	char			name[MAX_QPATH];
	int				id;
	qboolean		valid;		// qtrue if all permutations linked successfully

} glslProgram_t;

glslProgram_t r_programs[MAX_PROGRAMS];

glslProgram_t		*ambientWorldProgram;
glslProgram_t		*lightWorldProgram;
glslProgram_t		*aliasAmbientProgram;
glslProgram_t		*md3AmbientProgram;
glslProgram_t		*aliasBumpProgram;
glslProgram_t		*gaussXProgram;
glslProgram_t		*gaussYProgram;
glslProgram_t		*glareProgram;
glslProgram_t		*bloomdsProgram;
glslProgram_t		*bloomfpProgram;
glslProgram_t		*motionBlurProgram;
glslProgram_t		*ssaoProgram;
glslProgram_t		*depthDownsampleProgram;
glslProgram_t		*ssaoBlurProgram;
glslProgram_t		*refractProgram;
glslProgram_t		*lightGlassProgram;
glslProgram_t		*thermalProgram;
glslProgram_t		*thermalfpProgram;
glslProgram_t		*waterProgram;
glslProgram_t		*lavaProgram;
glslProgram_t		*radialProgram;
glslProgram_t		*dofProgram;
glslProgram_t		*particlesProgram;
glslProgram_t		*shadowProgram;
glslProgram_t		*ssProgram;
glslProgram_t		*genericProgram;
glslProgram_t		*cinProgram;
glslProgram_t		*loadingProgram;
glslProgram_t		*fxaaProgram;
glslProgram_t		*filmGrainProgram;
glslProgram_t		*nullProgram;
glslProgram_t		*gammaProgram;
glslProgram_t		*lutProgram;
glslProgram_t		*FboProgram;
glslProgram_t		*light2dProgram;
glslProgram_t		*fixFovProgram;
glslProgram_t		*menuProgram;
glslProgram_t		*skyProgram;
glslProgram_t		*colorProgram;
glslProgram_t		*fbo2screenProgram;
glslProgram_t		*globalFogProgram;

void GL_BindProgram (glslProgram_t *program);
void R_CaptureDepthBuffer ();
void R_CaptureColorBuffer ();
void R_DrawLightWorld ();
void R_SetupOrthoMatrix(void);

typedef enum {
	ATT_POSITION = 0,
	ATT_NORMAL = 1,
	ATT_TANGENT = 2,
	ATT_BINORMAL = 3,
	ATT_COLOR = 4,
	ATT_TEX0 = 5,
	ATT_TEX1 = 6,
	ATT_TEX2 = 7,
}
glsl_attrib;

typedef enum {
	U_MVP_MATRIX,
	U_MODELVIEW_MATRIX,
	U_PROJ_MATRIX,
	U_ORTHO_MATRIX,

	U_TEXTURE0_MATRIX,
	U_TEXTURE1_MATRIX,
	U_TEXTURE2_MATRIX,
	U_TEXTURE3_MATRIX,
	U_TEXTURE4_MATRIX,
	U_TEXTURE5_MATRIX,
	U_TEXTURE6_MATRIX,

	U_ATTEN_MATRIX,
	U_SPOT_MATRIX,
	U_CUBE_MATRIX,

	U_SCREEN_SIZE,
	U_DEPTH_PARAMS,
	U_COLOR,
	U_COLOR_OFFSET,	// glow shift
	U_COLOR_MUL,	// color multipler

	U_SCROLL,
	U_AMBIENT_LEVEL,
	U_LM_TYPE,
	U_PARALLAX_TYPE,
	U_PARALLAX_PARAMS,
	U_USE_SSAO,
	U_LAVA_PASS,
	U_SHELL_PASS,
	U_SHELL_PARAMS,
	U_ENV_PASS,
	U_ENV_SCALE,
	
	U_LIGHT_POS,
	U_VIEW_POS,
	U_USE_FOG,
	U_FOG_DENSITY,
	U_USE_CAUSTICS,
	U_CAUSTICS_SCALE,
	U_AMBIENT_LIGHT,
	U_SPOT_LIGHT,
	U_SPOT_PARAMS,
	U_USE_AUTOBUMP,
	U_AUTOBUMP_PARAMS,
	U_USE_RGH_MAP,
	U_RGH_SCALE,
	U_SPECULAR_SCALE,

	U_TRANS_PASS,

	U_COLOR_PARAMS,
	U_COLOR_VIBRANCE,

	U_PARTICLE_THICKNESS,
	U_PARTICLE_MASK,
	U_TEXCOORD_OFFSET,
	U_PARTICLE_ANIM,

	U_PARAM_VEC2_0,
	U_PARAM_VEC2_1,
	U_PARAM_VEC2_2,
	U_PARAM_VEC2_3,
	U_PARAM_VEC2_4,
	U_PARAM_VEC2_5,

	U_PARAM_VEC3_0,
	U_PARAM_VEC3_1,
	U_PARAM_VEC3_2,
	U_PARAM_VEC3_3,
	U_PARAM_VEC3_4,
	U_PARAM_VEC3_5,

	U_PARAM_VEC4_0,
	U_PARAM_VEC4_1,
	U_PARAM_VEC4_2,
	U_PARAM_VEC4_3,
	U_PARAM_VEC4_4,
	U_PARAM_VEC4_5,

	U_PARAM_FLOAT_0,
	U_PARAM_FLOAT_1,
	U_PARAM_FLOAT_2,
	U_PARAM_FLOAT_3,
	U_PARAM_FLOAT_4,
	U_PARAM_FLOAT_5,

	U_PARAM_INT_0,
	U_PARAM_INT_1,
	U_PARAM_INT_2,
	U_PARAM_INT_3,
	U_PARAM_INT_4,
	U_PARAM_INT_5,

	U_REFR_ALPHA,
	U_REFR_DEFORM_MUL,
	U_REFR_THICKNESS0,
	U_REFR_THICKNESS1,
	U_REFR_ALPHA_MASK,
	U_REFR_MASK,

	U_WATER_DEFORM_MUL,
	U_WATER_ALPHA,
	U_WATHER_THICKNESS,
	U_WATER_TRANS,
	U_WATER_MIRROR,

	U_CONSOLE_BACK,
	U_2D_PICS,
	U_FRAG_COLOR,
}
glsl_uniform;

typedef enum {
	TMU0,
	TMU1,
	TMU2,
	TMU3,
	TMU4,
	TMU5,
	TMU6,
	TMU7,
	TMU8,
	TMU9,
}glsl_bindless;

#define	MAX_VERTEX_CACHES	4096

void R_DrawFullScreenQuad();
static GLenum	drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

qboolean nvApiInit;

void R_GpuInfo_f(void);
void ADL_PrintGpuInfo();
void ADL_Shutdown();

typedef enum {
	PLANE_ON,		// used by point check only
	PLANE_FRONT,
	PLANE_BACK,
	PLANE_CLIP
}plane_t;

typedef struct img_s {
	byte* pixels;
	int		width;
	int		height;
} img_t;

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void GLimp_EndFrame (void);
qboolean GLimp_Init (void *hinstance, void *hWnd);
void GLimp_Shutdown (void);
rserr_t GLimp_SetMode (unsigned *pwidth, unsigned *pheight, int mode,
	qboolean fullscreen);
void GLimp_AppActivate (qboolean active);

qboolean Sys_CheckWindowsVersion();
void Sys_CpuID();
void Sys_GetMemorySize();
void Sys_WindowsInfo();

#ifndef __GLW_H__
#define __GLW_H__

typedef struct {
#ifdef _WIN32
	HINSTANCE	hInstance;
	void	*wndproc;

	HDC     hDC;			// handle to device context
	HWND    hWnd;			// handle to window
	HGLRC   hGLRC;			// handle to GL rendering context
	
	HWND	hWndFake;
	HDC		hDCFake;
	HGLRC	hGLRCFake;

	HINSTANCE hinstOpenGL;	// HINSTANCE for the OpenGL library

	const char	*wglExtsString;
	const char	*wglRenderer;
	int desktopWidth, desktopHeight;
	int monitorWidth, monitorHeight;
	int desktopBitPixel;
	int dpi;
	
	int desktopPosX, desktopPosY;
	int virtualX, virtualY;
	int virtualWidth, virtualHeight;
	int borderWidth, borderHeight;

	qboolean pixelFormatSet;
	char	 desktopName[32];		// no monitor specified if empty, drawing on primary display

#else
	void *hinstOpenGL;
#endif
} glwstate_t;

extern glwstate_t glw_state;

#endif

#endif							/* R_LOCAL_H */
