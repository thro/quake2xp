#define IDMD3HEADER		(('3'<<24)+('P'<<16)+('D'<<8)+'I')

#define MD3_ALIAS_VERSION	15
#define MD3_ALIAS_MAX_LODS	4

#define	MD3_MAX_TRIANGLES	8192	// per mesh
#define MD3_MAX_VERTS		4096	// per mesh
#define MD3_MAX_SHADERS		256		// per mesh
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_MESHES		32		// per model
#define MD3_MAX_TAGS		16		// per frame
#define MD3_MAX_PATH		64
#define MD3_MAX_SKINS		32

#ifndef M_TWOPI
#define M_TWOPI		6.28318530717958647692
#endif

// vertex scales
#define	MD3_XYZ_SCALE		(1.0f / 64.0f)


typedef struct
{
	float			st[2];
} dmd3coord_t;

typedef struct
{
	short			point[3];
	short			norm;
} dmd3vertex_t;

typedef struct
{
	vec3_t			mins;
	vec3_t			maxs;
	vec3_t			translate;
	float			radius;
	char			creator[16];
} dmd3frame_t;

typedef struct
{
	vec3_t			origin;
	float			axis[3][3];
} dorientation_t;

typedef struct
{
	char			name[MD3_MAX_PATH];		// tag name
	dorientation_t	orient;
} dmd3tag_t;

typedef struct
{
	char			name[MD3_MAX_PATH];
	int				unused;					// shader
} dmd3skin_t;

typedef struct
{
	char			id[4];

	char			name[MD3_MAX_PATH];

	int				flags;

	int				num_frames;
	int				num_skins;
	int				num_verts;
	int				num_tris;

	int				ofs_tris;
	int				ofs_skins;
	int				ofs_tcs;
	int				ofs_verts;

	int				meshsize;
} dmd3mesh_t;

typedef struct
{
	int				id;
	int				version;

	char			filename[MD3_MAX_PATH];

	int				flags;

	int				num_frames;
	int				num_tags;
	int				num_meshes;
	int				num_skins;

	int				ofs_frames;
	int				ofs_tags;
	int				ofs_meshes;
	int				ofs_end;
} dmd3_t;


////////// MD3 Support ///////////////////////

typedef struct md3ST_s
{
	vec2_t			st;
} md3ST_t;

typedef struct md3Vertex_s
{
	vec3_t			xyz;
	vec3_t			normal,
					tangent, 
					binormal;
} md3Vertex_t;

typedef struct
{
	vec3_t			mins;
	vec3_t			maxs;
	vec3_t			translate;
	float			radius;
} md3Frame_t;

typedef struct
{
	char			name[MD3_MAX_PATH];
	dorientation_t	orient;
} md3Tag_t;


typedef struct
{
	char			name[MD3_MAX_PATH];

} md3Skin_t;

typedef struct mtriangle_s
{
	int		neighbours[3];
} neighbours_t;

typedef struct
{
	int				num_verts;
	char			name[MD3_MAX_PATH];
	md3Vertex_t		*vertexes;
	md3ST_t			*stcoords;

	int				num_tris;
	index_t			*indexes;
	neighbours_t	*triangles;

	int				num_skins;
	int				flags;

	image_t			*skinsAlbedo[MD3_MAX_SKINS];
	image_t			*skinsNormal[MD3_MAX_SKINS];
	image_t			*skinsLight[MD3_MAX_SKINS];
	image_t			*skinsEnv[MD3_MAX_SKINS];
	image_t			*skinsRgh[MD3_MAX_SKINS];
	image_t			*skinsAO[MD3_MAX_SKINS];
	image_t			*skinsSkinLocal[MD3_MAX_SKINS];
	qboolean		muzzle;
	qboolean		skinAlphatest;
} md3Mesh_t;

typedef struct md3Model_s
{
	int				num_frames;
	md3Frame_t		*frames;

	int				num_tags;
	md3Tag_t		*tags;

	int				num_meshes;
	md3Mesh_t		*meshes;
} md3Model_t;

vec3_t	md3VertexCache[MD3_MAX_VERTS];
vec4_t	md3ColorCache[MD3_MAX_VERTS * 4];

#define MESH_OPAQUE 1
#define MESH_TRANSLUSCENT 2
#define MESH_NOSHADOW 4
#define MESH_TWOSIDE 8
#define MESH_SSS 16