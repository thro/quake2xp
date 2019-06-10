/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
==============================

SHADING LANGUAGE INTERFACE

==============================
*/
#include "r_local.h"

#define MAX_INFO_LOG		4096

#define	PROGRAM_HASH_SIZE	MAX_PROGRAMS

static glslProgram_t		*programHashTable[PROGRAM_HASH_SIZE];
int r_numPrograms;
static glslProgram_t	r_nullProgram;

#ifdef GLSLS_330 // work with out FXAA glsl 400 required for textureGather and textureGatherOffset.
static const char *glslExt =
"#version 330 core\n"
"#extension GL_ARB_explicit_uniform_location : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"#extension GL_ARB_gpu_shader5 : enable\n"
"precision mediump float;\n"
"precision mediump int;\n"
"out vec4 fragData;\n";	// out fragColor

#else

static const char *glslExt =
"#version 450 core\n"
"precision mediump float;\n"
"precision mediump int;\n"
"out vec4 fragData;\n";	// out fragColor
#endif

static const char *mathDefs =
"#define	CUTOFF_EPSILON	1.0 / 255.0\n"
"#define	PI				3.14159265358979323846\n"
"#define	HALF_PI			1.57079632679489661923\n"
"#define	SQRT_PI			1.77245385090551602729\n"
"#define	SQRT_THREE		1.73205080756887729352\n"
"#define	INV_PI			(1.0 / PI)\n";

static const char *glslUniforms =
"#define	U_MVP_MATRIX			0\n"
"#define	U_MODELVIEW_MATRIX		1\n"
"#define	U_PROJ_MATRIX			2\n"
"#define	U_ORTHO_MATRIX			3\n"

"#define	U_TEXTURE0_MATRIX		4\n"
"#define	U_TEXTURE1_MATRIX		5\n"
"#define	U_TEXTURE2_MATRIX		6\n"
"#define	U_TEXTURE3_MATRIX		7\n"
"#define	U_TEXTURE4_MATRIX		8\n"
"#define	U_TEXTURE5_MATRIX		9\n"
"#define	U_TEXTURE6_MATRIX		10\n"

"#define	U_ATTEN_MATRIX			11\n"
"#define	U_SPOT_MATRIX			12\n"
"#define	U_CUBE_MATRIX			13\n"

"#define	U_SCREEN_SIZE			14\n"
"#define	U_DEPTH_PARAMS			15\n"
"#define	U_COLOR					16\n"
"#define	U_COLOR_OFFSET			17\n"	// glow shift
"#define	U_COLOR_MUL				18\n"	// color multipler

"#define	U_SCROLL				19\n"
"#define	U_AMBIENT_LEVEL			20\n"
"#define	U_LM_TYPE				21\n"
"#define	U_PARALLAX_TYPE			22\n"
"#define	U_PARALLAX_PARAMS		23\n"
"#define	U_USE_SSAO				24\n"
"#define	U_LAVA_PASS				25\n"
"#define	U_SHELL_PASS			26\n"
"#define	U_SHELL_PARAMS			27\n"
"#define	U_ENV_PASS				28\n"
"#define	U_ENV_SCALE				29\n"

"#define	U_LIGHT_POS				30\n"
"#define	U_VIEW_POS				31\n"
"#define	U_USE_FOG				32\n"
"#define	U_FOG_DENSITY			33\n"
"#define	U_USE_CAUSTICS			34\n"
"#define	U_CAUSTICS_SCALE		35\n"
"#define	U_AMBIENT_LIGHT			36\n"
"#define	U_SPOT_LIGHT			37\n"
"#define	U_SPOT_PARAMS			38\n"
"#define	U_USE_AUTOBUMP			39\n"
"#define	U_AUTOBUMP_PARAMS		40\n"
"#define	U_USE_RGH_MAP			41\n"
"#define	U_RGH_SCALE				42\n"
"#define	U_SPECULAR_SCALE		43\n"

"#define	U_TRANS_PASS			44\n"

"#define	U_COLOR_PARAMS			45\n"
"#define	U_COLOR_VIBRANCE		46\n"

"#define	U_PARTICLE_THICKNESS	47\n"	
"#define	U_PARTICLE_MASK			48\n"
"#define	U_TEXCOORD_OFFSET		49\n"
"#define	U_PARTICLE_ANIM			50\n"

"#define	U_PARAM_VEC2_0			51\n"
"#define	U_PARAM_VEC2_1			52\n"
"#define	U_PARAM_VEC2_2			53\n"
"#define	U_PARAM_VEC2_3			54\n"
"#define	U_PARAM_VEC2_4			55\n"
"#define	U_PARAM_VEC2_5			56\n"

"#define	U_PARAM_VEC3_0			57\n"
"#define	U_PARAM_VEC3_1			58\n"
"#define	U_PARAM_VEC3_2			59\n"
"#define	U_PARAM_VEC3_3			60\n"
"#define	U_PARAM_VEC3_4			61\n"
"#define	U_PARAM_VEC3_5			62\n"

"#define	U_PARAM_VEC4_0			63\n"
"#define	U_PARAM_VEC4_1			64\n"
"#define	U_PARAM_VEC4_2			65\n"
"#define	U_PARAM_VEC4_3			66\n"
"#define	U_PARAM_VEC4_4			67\n"
"#define	U_PARAM_VEC4_5			68\n"

"#define	U_PARAM_FLOAT_0			69\n"
"#define	U_PARAM_FLOAT_1			70\n"
"#define	U_PARAM_FLOAT_2			71\n"
"#define	U_PARAM_FLOAT_3			72\n"
"#define	U_PARAM_FLOAT_4			73\n"
"#define	U_PARAM_FLOAT_5			74\n"

"#define	U_PARAM_INT_0			75\n"
"#define	U_PARAM_INT_1			76\n"
"#define	U_PARAM_INT_2			77\n"
"#define	U_PARAM_INT_3			78\n"
"#define	U_PARAM_INT_4			79\n"
"#define	U_PARAM_INT_5			80\n"

"#define	U_REFR_ALPHA			81\n"
"#define	U_REFR_DEFORM_MUL		82\n"
"#define	U_REFR_THICKNESS0		83\n"
"#define	U_REFR_THICKNESS1		84\n"
"#define	U_REFR_ALPHA_MASK		85\n"
"#define	U_REFR_MASK				86\n"

"#define	U_WATER_DEFORM_MUL		87\n"
"#define	U_WATER_ALPHA			88\n"
"#define	U_WATHER_THICKNESS		89\n"
"#define	U_WATER_TRANS			90\n"
"#define	U_WATER_MIRROR			91\n"

"#define	U_CONSOLE_BACK			92\n"
"#define	U_2D_PICS				93\n"
"#define	U_FRAG_COLOR			94\n"
;

static const char *unbindTextures =
"#define	TMU0			200\n"
"#define	TMU1			201\n"
"#define	TMU2			202\n"
"#define	TMU3			203\n"
"#define	TMU4			204\n"
"#define	TMU5			205\n"
"#define	TMU6			206\n"
"#define	TMU7			207\n"
"#define	TMU8			208\n"
"#define	TMU9			209\n"
;

typedef enum {
	S_DEFAULT		= 1,
	S_TESSCONTROL	= 2,
	S_TESSEVAL		= 4,
	S_GEO			= 8,
}shaderType;


/*
=================
Com_HashKey

=================
*/
unsigned Com_HashKey (const char *string, unsigned size) {
	int			i;
	unsigned	hash = 0;
	char		letter;

	for (i = 0; string[i]; i++) {
		letter = tolower (string[i]);

		if (letter == '.') break;				// don't include extension
		if (letter == '\\') letter = '/';		// damn path names

		hash += (unsigned)letter * (i + 119);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash %= size;

	return hash;
}

/*
===============
Q_IsLiteral

===============
*/
qboolean Q_IsLiteral (const char *text) {
	int		i, c, len;

	len = strlen (text);

	for (i = 0; i < len; i++) {
		c = text[i];

		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != '_')
			return qfalse;
	}

	return qtrue;
}

/*
==========================================

MISCELLANEOUS

==========================================
*/

/*
==============
R_ProgramForName

==============
*/
static glslProgram_t *R_ProgramForName (const char *name) {
	glslProgram_t	*program;
	unsigned	hash;

	hash = Com_HashKey (name, PROGRAM_HASH_SIZE);

	for (program = programHashTable[hash]; program; program = program->nextHash) {
		if (!Q_stricmp (program->name, name))
			return program;
	}

	return NULL;
}

/*
===============
R_GetInfoLog

===============
*/
static void R_GetInfoLog (int id, char *log, qboolean isProgram) {
	int		length, dummy;

	if (isProgram)
		qglGetProgramiv (id, GL_INFO_LOG_LENGTH, &length);
	else
		qglGetShaderiv (id, GL_INFO_LOG_LENGTH, &length);

	if (length < 1) {
		log[0] = 0;
		return;
	}

	if (length >= MAX_INFO_LOG)
		length = MAX_INFO_LOG - 1;

	if (isProgram)
		qglGetProgramInfoLog (id, length, &dummy, log);
	else
		qglGetShaderInfoLog (id, length, &dummy, log);

	log[length] = 0;
}


/*
==============
R_LoadIncludes

Search shader texts for '#include' directives
and insert included file contents.
==============
*/
void *Z_Malloc(int size);

char *R_LoadIncludes (char *glsl) {
	char filename[MAX_QPATH];
	char *token, *p, *oldp, *oldglsl;
	int l, limit = 64;          // limit for prevent infinity recursion

	/// calculate size of glsl with includes
	l = strlen (glsl);
	p = glsl;
	while (1) {
		oldp = p;
		token = Com_ParseExt (&p, qtrue);
		if (!token[0])
			break;

		if (!strcmp (token, "#include")) {
			int	li;
			char	*buf;

			if (limit < 0)
				Com_Error (ERR_FATAL, "R_LoadIncludes: more than 64 includes");

			token = Com_ParseExt (&p, qfalse);
			Com_sprintf (filename, sizeof(filename), "glsl/include/%s", token);
			li = FS_LoadFile (filename, (void **)&buf);
			if (!buf)
				Com_Error (ERR_FATAL, "Couldn't load %s", filename);

			oldglsl = glsl;
			glsl = (char*)Q_malloc(l + li + 2);
			memset (glsl, 0, l + li + 2);
			Q_memcpy (glsl, oldglsl, oldp - oldglsl);
			Q_strcat (glsl, "\n", l + li + 1);
			Q_strcat (glsl, buf, l + li + 1);
			Q_strcat (glsl, p, l + li + 1);
			p = oldp - oldglsl + glsl;
			l = strlen (glsl);
			FS_FreeFile (buf);
			limit--;
		}
	}

	return glsl;
}

/*
===============================
Try To Load Precompiled Shaders
===============================
*/

qboolean R_LoadBinaryShader(char *shaderName, int shaderId) {

	char			name[MAX_QPATH];
	GLint			binLength;
	GLvoid*			bin;
	GLint			success;
	FILE*			binFile;

	if (!r_useShaderCache->integer)
		return qfalse;

	Com_sprintf(name, sizeof(name), "%s/shadercache/%s.shader", FS_Gamedir(), shaderName);
	FS_CreatePath(name);

	binFile = fopen(name, "rb");
	if (!binFile) {
		return qfalse;
	}
	else {
		fseek(binFile, 0, SEEK_END);
		binLength = (GLint)ftell(binFile);
		bin = (GLvoid*)malloc(binLength);
		fseek(binFile, 0, SEEK_SET);
		fread(bin, binLength, 1, binFile);
		fclose(binFile);

		glProgramBinary(shaderId, gl_state.binaryFormats, bin, binLength);
		qglGetProgramiv(shaderId, GL_LINK_STATUS, &success);
		free(bin);

		if (success) {
			Com_DPrintf(S_COLOR_GREEN">bin\n");
			return qtrue;
		}
	}
	return qfalse;
}

/*
==============
R_CreateProgram

==============
*/

static glslProgram_t *R_CreateProgram (	const char *name, const char *vertexSource, const char *fragmentSource) {
	char			log[MAX_INFO_LOG];
	unsigned		hash;
	glslProgram_t	*program;
	const char		*strings[MAX_PROGRAM_DEFS * 3 + 2];
	int				numStrings;
	int				numLinked = 0;
	int				id, vertexId, fragmentId;
	int				status;
	int				i;

	if ((vertexSource && strlen (vertexSource) < 17) || (fragmentSource && strlen (fragmentSource) < 17))
		return NULL;

	if (r_numPrograms == MAX_PROGRAMS)
		VID_Error (ERR_DROP, "R_CreateProgram: MAX_PROGRAMS hit");


	for (i = 0, program = r_programs; i < r_numPrograms; i++, program++) {
		if (!r_programs->name[0])
			break;
	}

	if (i == r_numPrograms) {
		if (r_numPrograms == MAX_PROGRAMS)
			VID_Error (ERR_DROP, "MAX_PROGRAMS");
		r_numPrograms++;
	}
	program = &r_programs[i];

	memset (program, 0, sizeof(*program));
	Q_strncpyz (program->name, name, sizeof(program->name));

	id = qglCreateProgram();

	if (!R_LoadBinaryShader(program->name, id)) { // can't load shader from cache - recompile it!

		numStrings = 0;
		vertexId = 0;
		fragmentId = 0;

		strings[numStrings++] = glslExt;
		strings[numStrings++] = mathDefs;
		strings[numStrings++] = glslUniforms;
		strings[numStrings++] = unbindTextures;

		// compile vertex shader
		if (vertexSource) {
			// link includes
			vertexSource = R_LoadIncludes((char*)vertexSource);

			strings[numStrings] = vertexSource;
			vertexId = qglCreateShader(GL_VERTEX_SHADER);
			qglShaderSource(vertexId, numStrings + 1, strings, NULL);
			qglCompileShader(vertexId);
			qglGetShaderiv(vertexId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(vertexId, log, qfalse);
				qglDeleteShader(vertexId);
				Com_Printf("program '%s': error(s) in vertex shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		// compile fragment shader
		if (fragmentSource) {
			// link includes
			fragmentSource = R_LoadIncludes((char*)fragmentSource);
			strings[numStrings] = fragmentSource;
			fragmentId = qglCreateShader(GL_FRAGMENT_SHADER);

			//		Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

			qglShaderSource(fragmentId, numStrings + 1, strings, NULL);
			qglCompileShader(fragmentId);
			qglGetShaderiv(fragmentId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(fragmentId, log, qfalse);
				qglDeleteShader(fragmentId);
				Com_Printf("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		//
		// link the program
		//

		if (vertexId) {
			qglAttachShader(id, vertexId);
			qglDeleteShader(vertexId);
		}

		if (fragmentId) {
			qglAttachShader(id, fragmentId);
			qglDeleteShader(fragmentId);
		}

		qglLinkProgram(id);
		qglGetProgramiv(id, GL_LINK_STATUS, &status);

		R_GetInfoLog(id, log, qtrue);

		if (!status) {
			qglDeleteProgram(id);
			Com_Printf("program '%s': link error(s): %s\n", program->name, log);
			return NULL;
		}

		// don't let it be slow (crap)
		if (strstr(log, "fragment shader will run in software")) {
			qglDeleteProgram(id);
			Com_Printf("program '%s': refusing to perform software emulation\n", program->name);
			return NULL;
		}
		
		if (r_useShaderCache->integer) {// make binary shader
			
			char	binName[MAX_QPATH];
			GLint	binLength;
			GLvoid*	bin;
			FILE*	binFile;

			qglGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binLength);
			bin = (GLvoid*)malloc(binLength);
			glGetProgramBinary(id, binLength, &binLength, &gl_state.binaryFormats, bin);

			Com_sprintf(binName, sizeof(binName), "%s/shadercache/%s.shader", FS_Gamedir(), program->name);
			FS_CreatePath(binName);
			binFile = fopen(binName, "wb");
			fwrite(bin, binLength, 1, binFile);
			fclose(binFile);
			free(bin);
		}
	}

	program->id = id;
	program->valid = qtrue;

	// add to the hash
	hash = Com_HashKey (program->name, PROGRAM_HASH_SIZE);
	program->nextHash = programHashTable[hash];
	programHashTable[hash] = program;

	return program;
}

void Q_snprintfz (char *dst, int dstSize, const char *fmt, ...);
/*
==============
R_FindProgram

==============
*/

glslProgram_t *R_FindProgram (const char *name, int flags) {
	char			filename[MAX_QPATH];
	glslProgram_t	*program;
	char			*vertexSource = NULL, *fragmentSource = NULL;

	if (flags & S_DEFAULT) {
		Q_snprintfz (filename, sizeof(filename), "glsl/%s.vert", name);
		FS_LoadFile (filename, (void **)&vertexSource);

		Q_snprintfz (filename, sizeof(filename), "glsl/%s.frag", name);
		FS_LoadFile (filename, (void **)&fragmentSource);
	}	

	if (!vertexSource | !fragmentSource)
		return &r_nullProgram;		// no appropriate shaders found

	program = R_CreateProgram (name, vertexSource, fragmentSource);

	if (vertexSource)
		FS_FreeFile (vertexSource);
	if (fragmentSource)
		FS_FreeFile (fragmentSource);

	if (!program || !program->valid)
		return &r_nullProgram;

	return program;
}

/*
=============
R_InitPrograms

=============
*/

#define GLSL_LOADING_TIME 1

void R_InitPrograms (void) {
	
	int	missing = 0;

	Com_Printf ("\nInitializing programs...\n\n");

#ifdef GLSL_LOADING_TIME
	int		start = 0, stop = 0;
	float	sec;
	start = Sys_Milliseconds ();
#endif
	 
	memset (programHashTable, 0, sizeof(programHashTable));
	memset (&r_nullProgram, 0, sizeof(glslProgram_t));

	Com_Printf ("Load "S_COLOR_YELLOW"null program"S_COLOR_WHITE" ");
	nullProgram = R_FindProgram ("null", S_DEFAULT);
	if (nullProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient world program"S_COLOR_WHITE" ");
	ambientWorldProgram = R_FindProgram ("ambientWorld", S_DEFAULT);
	if (ambientWorldProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"light world program"S_COLOR_WHITE" ");
	lightWorldProgram = R_FindProgram ("lightWorld", S_DEFAULT);
	if (lightWorldProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient md2 program"S_COLOR_WHITE" ");
	aliasAmbientProgram = R_FindProgram ("ambientMd2", S_DEFAULT);
	if (aliasAmbientProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"ambient md3 program"S_COLOR_WHITE" ");
	md3AmbientProgram = R_FindProgram("ambientMd3", S_DEFAULT);
	if (md3AmbientProgram->valid) {
		Com_Printf("succeeded\n");

	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"light model program"S_COLOR_WHITE" ");
	aliasBumpProgram = R_FindProgram ("lightAlias", S_DEFAULT);

	if (aliasBumpProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"gauss blur program"S_COLOR_WHITE" ");
	gaussXProgram = R_FindProgram ("gaussX", S_DEFAULT);
	gaussYProgram = R_FindProgram ("gaussY", S_DEFAULT);
	

	if (gaussXProgram->valid && gaussYProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"glare program"S_COLOR_WHITE" ");
	glareProgram = R_FindProgram ("glare", S_DEFAULT);

	if (glareProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf ("Load "S_COLOR_YELLOW"radial blur program"S_COLOR_WHITE" ");
	radialProgram = R_FindProgram ("radialBlur", S_DEFAULT);

	if (radialProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"dof blur program"S_COLOR_WHITE" ");
	dofProgram = R_FindProgram ("dof", S_DEFAULT);

	if (dofProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"motion blur program"S_COLOR_WHITE" ");
	motionBlurProgram = R_FindProgram ("mblur", S_DEFAULT);
	
	if (motionBlurProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ssao program"S_COLOR_WHITE" ");
	ssaoProgram = R_FindProgram ("ssao", S_DEFAULT);
	depthDownsampleProgram = R_FindProgram("depthDownsample", S_DEFAULT);
	ssaoBlurProgram = R_FindProgram("ssaoBlur", S_DEFAULT);

	if (ssaoProgram->valid && depthDownsampleProgram->valid && ssaoBlurProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"bloom program"S_COLOR_WHITE" ");
	bloomdsProgram = R_FindProgram ("bloomds", S_DEFAULT);
	bloomfpProgram = R_FindProgram ("bloomfp", S_DEFAULT);

	if (bloomdsProgram->valid && bloomfpProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"refraction program"S_COLOR_WHITE" ");
	refractProgram = R_FindProgram ("refract", S_DEFAULT);

	if (refractProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"thermal vision program"S_COLOR_WHITE" ");
	thermalProgram = R_FindProgram ("thermal", S_DEFAULT);

	thermalfpProgram = R_FindProgram ("thermalfp", S_DEFAULT);

	if (thermalProgram->valid && thermalfpProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"water program"S_COLOR_WHITE" ");
	waterProgram = R_FindProgram ("water", S_DEFAULT);
	if (waterProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"particles program"S_COLOR_WHITE" ");
	particlesProgram = R_FindProgram ("particles", S_DEFAULT);

	if (particlesProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf ("Load "S_COLOR_YELLOW"generic program"S_COLOR_WHITE" ");
	genericProgram = R_FindProgram ("generic", S_DEFAULT);

	if (genericProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"cinematic program"S_COLOR_WHITE" ");
	cinProgram	= R_FindProgram ("cin", S_DEFAULT);

	if (cinProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"load screen program"S_COLOR_WHITE" ");
	loadingProgram = R_FindProgram ("loading", S_DEFAULT);

	if (loadingProgram->valid) {
		Com_Printf ("succeeded\n");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"fxaa program"S_COLOR_WHITE" ");
	fxaaProgram = R_FindProgram ("fxaa", S_DEFAULT);

	if (fxaaProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"film grain program"S_COLOR_WHITE" ");
	filmGrainProgram = R_FindProgram ("filmGrain", S_DEFAULT);

	if (filmGrainProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"color correction program"S_COLOR_WHITE" ");
	gammaProgram = R_FindProgram ("gamma", S_DEFAULT);
	if (gammaProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"lookup color table program"S_COLOR_WHITE" ");
	lutProgram = R_FindProgram("lut", S_DEFAULT);
	if (lutProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"shadow volumes program"S_COLOR_WHITE" ");
	shadowProgram = R_FindProgram("shadow", S_DEFAULT);
	if (shadowProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"light2d program"S_COLOR_WHITE" ");
	light2dProgram = R_FindProgram("light2d", S_DEFAULT);
	if (light2dProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"perspective correction program"S_COLOR_WHITE" ");
	fixFovProgram = R_FindProgram("fixfov", S_DEFAULT);
	if (fixFovProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"menu background program"S_COLOR_WHITE" ");
	menuProgram = R_FindProgram("menu", S_DEFAULT);
	if (menuProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"sky program"S_COLOR_WHITE" ");
	skyProgram = R_FindProgram("sky", S_DEFAULT);
	if (skyProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"color program"S_COLOR_WHITE" ");
	colorProgram = R_FindProgram("color", S_DEFAULT);
	if (colorProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"global fog program"S_COLOR_WHITE" ");
	globalFogProgram = R_FindProgram("globalFog", S_DEFAULT);
	if (globalFogProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
/*
	Com_Printf("Load "S_COLOR_YELLOW"fbo program"S_COLOR_WHITE" ");
	fbo2screenProgram = R_FindProgram("fbo2screen", S_DEFAULT);
	if (fbo2screenProgram->valid) {
		Com_Printf("succeeded\n");
		id = fbo2screenProgram->id;
		fbo2screen_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
*/

#ifdef GLSL_LOADING_TIME
	stop = Sys_Milliseconds ();
	sec = (float)stop - (float)start;
	Com_Printf ("\nGLSL shaders loading time: "S_COLOR_GREEN"%5.4f"S_COLOR_WHITE" sec\n", sec * 0.001);
#endif
	Com_Printf ("\n");
}

/*
=============
R_ShutdownPrograms

=============
*/
void R_ShutdownPrograms (void) {
	glslProgram_t	*program;
	int				i;

	for (i = 0; i < r_numPrograms; i++) {
		program = &r_programs[i];
		qglDeleteProgram (program->id);
		}
	r_numPrograms = 0;
}

/*
=============
R_ListPrograms_f

=============
*/
void R_ListPrograms_f (void) {
	glslProgram_t	*program;
	int			numInvalid = 0;
	int			i;

	Com_Printf ("        permutations name\n");
	Com_Printf ("-------------------------\n");

	for (i = 0; i < r_numPrograms; i++) {
		program = &r_programs[i];
		if (!program->valid)
			numInvalid++;

		Com_Printf ("  %4i: %s%s\n", i, program->name, program->valid ? "" : "(INVALID)");
	}

	Com_Printf ("-------------------\n");
	Com_Printf (" %i programs\n", r_numPrograms);
	Com_Printf ("  %i invalid\n", numInvalid);
}

void R_GLSLinfo_f(void) {
	
	int i;
	GLint j;
	const char *ver;

	ver = (const char*)qglGetString(GL_SHADING_LANGUAGE_VERSION);
	Com_Printf("GLSL Version: "S_COLOR_GREEN"%s\n", ver);

	qglGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &j);
	for (i = 0; i < j; ++i) {
		ver = (const char*)glGetStringi(GL_SHADING_LANGUAGE_VERSION, i);
		if (!ver)
			break;
		Com_Printf(S_COLOR_YELLOW"%s\n", ver);
	}
}

/*
============
GL_BindProgram

============
*/
void GL_BindProgram (glslProgram_t *program) {
	int	id = program->id;

	if (gl_state.programId != id) {
		qglUseProgram (id);
		gl_state.programId = id;
	}
}

