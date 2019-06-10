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
// cl_view.c -- player rendering positioning

#include "client.h"
#include "../ref_gl/r_local.h"
//=============
//
// development tools for weapons
//
int gun_frame;
struct model_s *gun_model;

//=============

cvar_t *crosshair;
cvar_t *crosshairScale;
cvar_t *cl_stats;


int r_numdlights;
dlight_t r_dlights[MAX_DLIGHTS];

int r_numentities;
entity_t r_entities[MAX_ENTITIES];

int r_numparticles;
particle_t r_particles[MAX_PARTICLES];

lightstyle_t r_lightstyles[MAX_LIGHTSTYLES];

char cl_weaponmodels[MAX_CLIENTWEAPONMODELS][MAX_QPATH];
int num_cl_weaponmodels;


/*
====================
V_ClearScene

Specifies the model that will be used as the world
====================
*/

void V_ClearScene (void) {
	r_numdlights = 0;
	r_numentities = 0;
	r_numparticles = 0;
}

/*
=====================
V_AddEntity

=====================
*/
void V_AddEntity (entity_t * ent) {

	if (ent->flags & RF_VIEWERMODEL) {
		int i;

		for (i = 0; i < 3; i++)
			ent->oldorigin[i] = ent->origin[i] = cl.predicted_origin[i];

		if (cl_thirdPerson->integer)
			ent->flags &= ~RF_VIEWERMODEL;
	}

	if (r_numentities >= MAX_ENTITIES)
		return;
	r_entities[r_numentities++] = *ent;
}

/*
=====================
V_AddParticle

=====================
*/
void V_AddParticle (vec3_t org, vec3_t length, vec3_t color, float alpha,
	int type, float size, int sFactor, int dFactor,
	int flags, int time, float orient, float len,
	vec3_t oldOrg, vec3_t dir) {
	particle_t *p;
	vec3_t lm;

	if (r_numparticles >= MAX_PARTICLES)
		return;

	p = &r_particles[r_numparticles++];
	VectorCopy (org, p->origin);
	VectorCopy (length, p->length);
	VectorCopy (color, p->color);
	VectorCopy (dir, p->dir);
	p->alpha = alpha;
	p->type = type;
	p->size = size;
	p->sFactor = sFactor;
	p->dFactor = dFactor;
	p->flags = flags;
	p->time = (float)time * 0.001f;
	p->orient = orient;
	p->len = len;
	VectorCopy (oldOrg, p->oldOrg);

	if (p->flags & PARTICLE_VERTEXLIGHT) {
		R_LightColor (org, lm);
		VectorMul (p->color, lm, p->color);
	}
}


/*
=====================
V_AddLight

=====================
*/
void V_AddLight (vec3_t org, float intensity, float r, float g, float b, vec3_t ang, float cone, int filter) {
	dlight_t *dl;

	if (r_numdlights >= MAX_DLIGHTS)
		return;
	dl = &r_dlights[r_numdlights++];
	VectorCopy (org, dl->origin);
	VectorCopy (ang, dl->angles);
	dl->intensity = intensity;
	dl->_cone = cone;
	dl->filter = filter;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
}


/*
=====================
V_AddLightStyle

=====================
*/
void V_AddLightStyle (int style, float r, float g, float b) {
	lightstyle_t *ls;

	if (style < 0 || style > MAX_LIGHTSTYLES)
		Com_Error (ERR_DROP, "Bad light style %i", style);
	ls = &r_lightstyles[style];

	ls->white = r + g + b;
	ls->rgb[0] = r;
	ls->rgb[1] = g;
	ls->rgb[2] = b;
}


//===================================================================

/*
=================
CL_PrepRefresh

Call before entering a new level, or after changing dlls
=================
*/
qboolean needLoadingPlaque (void);
void R_GenSkyCubeMap (char *name);

#ifdef _WIN32
extern int	xInputActiveController;
void SetRumble(int inputDeviceNum, int rumbleLow, int rumbleHigh);
#endif

void CL_PrepRefresh (void) {
	char		mapname[32];
	int			i, start = 0, stop = 0;
	char		name[MAX_QPATH];
	float		rotate, sec;
	vec3_t		axis;
	qboolean	newPlaque = needLoadingPlaque ();

	if (!cl.configstrings[CS_MODELS + 1][0])
		return;					// no map loaded

	if (newPlaque)
		SCR_BeginLoadingPlaque ();

	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width - 1, viddef.height - 1);

#ifdef _WIN32
	SetRumble(xInputActiveController, 0, 0);
#endif

	start = Sys_Milliseconds ();

	loadScreenColorFade = 0.1;

	loadingMessage = qtrue;
	Com_sprintf (loadingMessages[0], sizeof(loadingMessages[0]),
		"Loading Map...");
	Com_sprintf (loadingMessages[1], sizeof(loadingMessages[1]),
		"Loading Models...");
	Com_sprintf (loadingMessages[2], sizeof(loadingMessages[2]),
		"Loading Pics...");
	Com_sprintf (loadingMessages[3], sizeof(loadingMessages[3]),
		"Loading Clients...");
	loadingPercent = 1;

	// let the render dll load the map
	strcpy (mapname, cl.configstrings[CS_MODELS + 1] + 5);	// skip "maps/"
	mapname[strlen (mapname) - 4] = 0;	// cut off ".bsp"

	// register models, pics, and skins
	Com_Printf ("Map: %s\r", mapname);
	SCR_UpdateScreen ();
	R_BeginRegistration (mapname);

	Com_Printf ("                                     \r");
	Com_sprintf (loadingMessages[0], sizeof(loadingMessages[0]),
		"Loading Map...done");
	loadingPercent = 35;
	loadScreenColorFade = 0.35;

	// precache status bar pics
	Com_Printf ("pics\r");
	SCR_UpdateScreen ();
	SCR_TouchPics ();
	Com_Printf ("                                     \r");

	CL_RegisterTEntModels ();

	num_cl_weaponmodels = 1;
	strcpy (cl_weaponmodels[0], "weapon.md2");

	for (i = 1; i < MAX_MODELS && cl.configstrings[CS_MODELS + i][0]; i++) {
		strcpy (name, cl.configstrings[CS_MODELS + i]);
		name[37] = 0;			// never go beyond one line
		if (name[0] != '*')
			Com_Printf ("%s\r", name);
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
		if (name[0] == '#') {
			// special player weapon model
			if (num_cl_weaponmodels < MAX_CLIENTWEAPONMODELS) {
				strncpy (cl_weaponmodels[num_cl_weaponmodels],
					cl.configstrings[CS_MODELS + i] + 1,
					sizeof(cl_weaponmodels[num_cl_weaponmodels]) - 1);
				num_cl_weaponmodels++;
			}
		}
		else {

			Com_sprintf (loadingMessages[1], sizeof(loadingMessages[1]),
				"Loading Models...%s", cl.configstrings[CS_MODELS + i]);

			cl.model_draw[i] = R_RegisterModel (cl.configstrings[CS_MODELS + i]);
			if (name[0] == '*')
				cl.model_clip[i] = CM_InlineModel (cl.configstrings[CS_MODELS + i]);
			else
				cl.model_clip[i] = NULL;
		}
		if (name[0] != '*')
			Com_Printf ("                                     \r");
	}
	Com_sprintf (loadingMessages[1], sizeof(loadingMessages[1]),
		"Loading Models...done");
	loadingPercent = 55;
	loadScreenColorFade = 0.55;

	Com_Printf ("images\r", i);
	SCR_UpdateScreen ();
	for (i = 1; i < MAX_IMAGES && cl.configstrings[CS_IMAGES + i][0]; i++) {

		cl.image_precache[i] =
			Draw_FindPic (cl.configstrings[CS_IMAGES + i]);
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
	}

	Com_Printf ("                                     \r");
	Com_sprintf (loadingMessages[2], sizeof(loadingMessages[2]),
		"Loading Pics...done");
	loadingPercent = 75;

	loadScreenColorFade = 0.75;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (!cl.configstrings[CS_PLAYERSKINS + i][0])
			continue;

		Com_sprintf (loadingMessages[3], sizeof(loadingMessages[3]),
			"Loading Clients...%i", i);

		Com_Printf ("client %i\r", i);

		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
		CL_ParseClientinfo (i);
		Com_Printf ("                                     \r");
		Com_sprintf (loadingMessages[3], sizeof(loadingMessages[3]),
			"Loading Clients...done");
		loadingPercent = 85;

		loadScreenColorFade = 1.0;
	}
	loadingPercent = 100;

	CL_LoadClientinfo (&cl.baseclientinfo, "unnamed\\male/grunt");

	// set sky textures and speed
	Com_Printf ("sky\r", i);
	SCR_UpdateScreen ();
	rotate = atof (cl.configstrings[CS_SKYROTATE]);
	sscanf (cl.configstrings[CS_SKYAXIS], "%f %f %f",
		&axis[0], &axis[1], &axis[2]);
	R_SetSky (cl.configstrings[CS_SKY], rotate, axis);
	R_GenSkyCubeMap (cl.configstrings[CS_SKY]);
	Com_Printf ("                                     \r");


	// clear any lines of console text
	Con_ClearNotify ();

	SCR_UpdateScreen ();
	cl.refresh_prepped = qtrue;
	cl.force_refdef = qtrue;		// make sure we have a valid refdef

	Com_sprintf (loadingMessages[0], sizeof(loadingMessages[0]), "");
	Com_sprintf (loadingMessages[1], sizeof(loadingMessages[1]), "");
	Com_sprintf (loadingMessages[2], sizeof(loadingMessages[2]), "");
	Com_sprintf (loadingMessages[3], sizeof(loadingMessages[3]), "");

	// start the cd track
	Music_Play ();

	loadingMessage = qfalse;
	stop = Sys_Milliseconds ();
	sec = (float)stop - (float)start;
	sec *= 0.001;
	Com_Printf ("level loading time = %5.4f sec\n", sec);

	// the renderer can now free unneeded stuff
	R_EndRegistration ();

	if (newPlaque)
		SCR_EndLoadingPlaque ();
	else
		Cvar_Set ("paused", "0");

	cl.minFps = cl.maxFps = 0;
}

//============================================================================

// gun frame debugging functions
void V_Gun_Next_f (void) {
	gun_frame++;
	Com_Printf ("frame %i\n", gun_frame);
}

void V_Gun_Prev_f (void) {
	gun_frame--;
	if (gun_frame < 0)
		gun_frame = 0;
	Com_Printf ("frame %i\n", gun_frame);
}

void V_Gun_Model_f (void) {
	char name[MAX_QPATH];

	if (Cmd_Argc () != 2) {
		gun_model = NULL;
		return;
	}
	Com_sprintf (name, sizeof(name), "models/%s/tris.md2", Cmd_Argv (1));
	gun_model = R_RegisterModel (name);
}

//============================================================================


/*
=================
SCR_DrawCrosshair
=================
*/
void SCR_DrawCrosshair (void) {
	int		size_x, size_y;


	if (!crosshair->integer)
		return;

	if (crosshair->modified) {
		crosshair->modified = qfalse;
		SCR_TouchPics ();
	}

	if (!crosshair_pic[0])
		return;

	size_x = crosshair_width * crosshairScale->value;
	size_y = crosshair_height * crosshairScale->value;

	//	Com_Printf("width %i height %i syze_x %i size_y %i cross width %i cross height %i\n",viddef.width, viddef.height, size_x, size_y, crosshair_width, crosshair_height);

	Draw_PicScaled ((viddef.width * 0.5) - (size_x * 0.5), (viddef.height * 0.5) - (size_y * 0.5),
		crosshairScale->value, crosshairScale->value, crosshair_pic);

}

/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height) // fov for noWorldModels
{
	float a;
	float x;

	if (fov_x < 1 || fov_x > 179)
		Com_Error (ERR_DROP, "Bad fov: %f", fov_x);

	x = width / tan (fov_x * (0.002777777777778 * M_PI));

	a = atan (height / x);

	a = a * 114.59165581759554875079179651068;

	return a;
}

#define AR_4x3	4.0f / 3.0f 

void CalcFovForScreen (float ingameFOV) {
	float	x, y, ratio_x, ratio_y;
	float	screenAspect = (float)vid.width / (float)vid.height;

	if (ingameFOV < 1 || ingameFOV > 179)
		ingameFOV = 91;

	// calc FOV for 640x480 view (4x3 aspect ratio)
	x = 640.0f / tan (ingameFOV / 360.0f * M_PI);
	y = atan2 (480.0f, x);
	cl.refdef.fov_y = y * 360.0f / M_PI;

	if (screenAspect == AR_4x3) {
		cl.refdef.fov_x = ingameFOV;
		return;
	}
	// calc FOV for widescreen
	ratio_x = (float)vid.width;
	ratio_y = (float)vid.height;

	y = ratio_y / tan (cl.refdef.fov_y / 360.0f * M_PI);
	cl.refdef.fov_x = atan2 (ratio_x, y) * 360.0f / M_PI;

	if (cl.refdef.fov_x < ingameFOV) {
		cl.refdef.fov_x = ingameFOV;
		x = ratio_x / tan (cl.refdef.fov_x / 360.0f * M_PI);
		cl.refdef.fov_y = atan2 (ratio_y, x) * 360.0f / M_PI;
	}

}


/*
==================
V_RenderView

==================
*/

void V_RenderView () {
	extern int entitycmpfnc (const entity_t *, const entity_t *);

	if (cls.state != ca_active)
		return;

	if (!cl.refresh_prepped)
		return;					// still loading

	if (cl_timedemo->integer) {
		if (!cl.timedemo_start)
			cl.timedemo_start = Sys_Milliseconds ();
		cl.timedemo_frames++;
	}
	// an invalid frame will just use the exact previous refdef
	// we can't use the old frame if the video mode has changed, though...
	if (cl.frame.valid && (cl.force_refdef || !cl_paused->value)) {
		cl.force_refdef = qfalse;

		V_ClearScene ();

		// build a refresh entity list and calc cl.sim*
		// this also calls CL_CalcViewValues which loads
		// v_forward, etc.
		CL_AddEntities ();

		// never let it sit exactly on a node line, because a water plane
		// can
		// dissapear when viewed with the eye exactly on it.
		// the server protocol only specifies to 1/8 pixel, so add 1/16 in 
		// each axis
		cl.refdef.vieworg[0] += 1.0 / 16;
		cl.refdef.vieworg[1] += 1.0 / 16;
		cl.refdef.vieworg[2] += 1.0 / 16;

		cl.refdef.x = scr_vrect.x;
		cl.refdef.y = scr_vrect.y;
		cl.refdef.width = scr_vrect.width;
		cl.refdef.height = scr_vrect.height;

		CalcFovForScreen (cl.refdef.fov_x);

		cl.refdef.time = cl.time * 0.001;


		// Warp if underwater ala q3a :-)
		if (cl.refdef.rdflags & RDF_UNDERWATER) {
			float f = sin (cl.time * 0.001 * 0.4 * (M_PI * 2.7));

			cl.refdef.fov_x += f;
			cl.refdef.fov_y -= f;
		}

#ifdef _WIN32
		// add xBox controller vibration 
		int value = cl.frame.playerstate.stats[STAT_HEALTH];

		if ((cl.refdef.rdflags & RDF_PAIN) && (value > 0)) 
			SetRumble(xInputActiveController, 4096, 65535);
		else
			SetRumble(xInputActiveController, 0, 0);
#endif
		
		cl.refdef.areabits = cl.frame.areabits;

		if (!cl_add_entities->integer)
			r_numentities = 0;

		if (!cl_add_particles->integer)
			r_numparticles = 0;
		if (!cl_add_lights->integer)
			r_numdlights = 0;
		if (!cl_add_blend->integer)
			VectorClear (cl.refdef.blend);

		pst.playerHealth = cl.frame.playerstate.stats[STAT_HEALTH];
		pst.playerArmor = cl.frame.playerstate.stats[STAT_ARMOR];

		cl.refdef.num_entities = r_numentities;
		cl.refdef.entities = r_entities;

		cl.refdef.num_particles = r_numparticles;
		cl.refdef.particles = r_particles;

		cl.refdef.num_dlights = r_numdlights;
		cl.refdef.dlights = r_dlights;

		cl.refdef.lightstyles = r_lightstyles;

		cl.refdef.rdflags = cl.frame.playerstate.rdflags;

		// sort entities for better cache locality
	//	qsort (cl.refdef.entities, cl.refdef.num_entities,
	//		sizeof(cl.refdef.entities[0]),
	//		(int (*)(const void *, const void *)) entitycmpfnc);



	}

	c_brush_polys = 0;
	c_alias_polys = 0;
	c_part_tris = 0;
	c_shadow_tris = 0;
	c_flares = 0;
	c_shadow_volumes = 0;
	c_decal_tris = 0;

	R_RenderFrame (&cl.refdef);


	if (cl_stats->integer)
		Com_Printf ("ent:%i  dlights:%i  part:%i \n", r_numentities,
		r_numdlights, r_numparticles);
	if (log_stats->integer && (log_stats_file != 0))
		fprintf (log_stats_file, "%i,%i,%i,", r_numentities,
		r_numdlights, r_numparticles);


	SCR_AddDirtyPoint (scr_vrect.x, scr_vrect.y);
	SCR_AddDirtyPoint (scr_vrect.x + scr_vrect.width - 1,
		scr_vrect.y + scr_vrect.height - 1);

	SCR_DrawCrosshair ();
}


/*
=============
V_Viewpos_f
=============
*/
void V_Viewpos_f (void) {
	Com_Printf ("(%i %i %i) : %i\n", (int)cl.refdef.vieworg[0],
		(int)cl.refdef.vieworg[1], (int)cl.refdef.vieworg[2],
		(int)cl.refdef.viewangles[YAW]);
}

/*
=============
V_Init
=============
*/
void V_Init (void) {
	Cmd_AddCommand ("gun_next", V_Gun_Next_f);
	Cmd_AddCommand ("gun_prev", V_Gun_Prev_f);
	Cmd_AddCommand ("gun_model", V_Gun_Model_f);

	Cmd_AddCommand ("viewpos", V_Viewpos_f);

	crosshair = Cvar_Get ("crosshair", "0", CVAR_ARCHIVE);
	crosshairScale = Cvar_Get ("crosshairScale", "0.666", CVAR_ARCHIVE);
	cl_stats = Cvar_Get ("cl_stats", "0", 0);
}
