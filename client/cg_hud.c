/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C)	1997-2001 Id Software, Inc.,
2004-2013 Quake2xp Team

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
// cl_hud.c -- 3d-2d hud stuff

#include "client.h"

void DrawHUDString (float x, float y, float scale_x, float scale_y, int centerwidth, int xor, char *string);
void SCR_DrawField (int x, int y, float scale_x, float scale_y, int color, int width, int value);
float CalcFov (float fov_x, float width, float height);

extern cvar_t *cl_hudScale;

typedef struct {

	struct model_s *cl_hud_comp;
	struct model_s *cl_hud_predator;

	struct model_s *cl_hud_health_large;
	struct model_s *cl_hud_health_stim;
	struct model_s *cl_hud_health_medium;
	struct model_s *cl_hud_health_mega;

	struct model_s *cl_hud_armor1;
	struct model_s *cl_hud_armor2;
	struct model_s *cl_hud_armor3;
	struct model_s *cl_hud_armor4;

	struct model_s *cl_hud_bullet;
	struct model_s *cl_hud_rockets;
	struct model_s *cl_hud_slugs;
	struct model_s *cl_hud_shells;
	struct model_s *cl_hud_grenades;
	struct model_s *cl_hud_cells;

	struct model_s *cl_hud_adrenaline;
	struct model_s *cl_hud_bandolier;
	struct model_s *cl_hud_envirosuit;
	struct model_s *cl_hud_invulnerability;
	struct model_s *cl_hud_megahealth;
	struct model_s *cl_hud_quad;
	struct model_s *cl_hud_rebreather;
	struct model_s *cl_hud_silencer;
	struct model_s *cl_hud_airstrike;
	struct model_s *cl_hud_carmak;

	struct model_s *cl_hud_blaster;
	struct model_s *cl_hud_shotgun;
	struct model_s *cl_hud_sshotgun;
	struct model_s *cl_hud_machinegun;
	struct model_s *cl_hud_chaingun;
	struct model_s *cl_hud_glauncher;
	struct model_s *cl_hud_rocketl;
	struct model_s *cl_hud_hyperb;
	struct model_s *cl_hud_rail;
	struct model_s *cl_hud_bfg;

	struct model_s *cl_hud_pack;
	struct model_s *cl_hud_powerscreen;
	struct model_s *cl_hud_powershield;

	struct model_s *cl_hud_bluekey;
	struct model_s *cl_hud_comhead;
	struct model_s *cl_hud_datacd;
	struct model_s *cl_hud_dataspin;
	struct model_s *cl_hud_powercube;
	struct model_s *cl_hud_pyramid;
	struct model_s *cl_hud_redkey;
	struct model_s *cl_hud_security;

	//== The Reckoning==//
	struct model_s *cl_hud_mslugs;
	struct model_s *cl_hud_trap;
	struct model_s *cl_hud_greenkey;
	struct model_s *cl_hud_quadfire;
	struct model_s *cl_hud_phallanx;
	struct model_s *cl_hud_ripper;
	//==Ground Zero==//
	struct model_s *cl_hud_heatbeam;
	struct model_s *cl_hud_flech;
	struct model_s *cl_hud_flech_a;
	struct model_s *cl_hud_proxlch;
	struct model_s *cl_hud_proxlch_a;
	struct model_s *cl_hud_chainfist;
	struct model_s *cl_hud_tesla;
	struct model_s *cl_hud_nuke;
	struct model_s *cl_hud_ir;
	struct model_s *cl_hud_double;
	struct model_s *cl_hud_compass;
	struct model_s *cl_hud_vengnce;
	struct model_s *cl_hud_hunter;
	struct model_s *cl_hud_def;
	struct model_s *cl_hud_doppleganger;

} hudmodel_t;

static hudmodel_t hudmodel;

void LoadHudEnts (void) {
	hudmodel.cl_hud_predator =
		R_RegisterModel ("models/items/predator/tris.md2");
	hudmodel.cl_hud_comp =
		R_RegisterModel ("models/items/pda/tris.md2");

	hudmodel.cl_hud_health_large = // and static hud health
		R_RegisterModel ("models/items/healing/large/tris.md2");
	hudmodel.cl_hud_health_stim =
		R_RegisterModel ("models/items/healing/stimpack/tris.md2");
	hudmodel.cl_hud_health_medium =
		R_RegisterModel ("models/items/healing/medium/tris.md2");
	hudmodel.cl_hud_health_mega =
		R_RegisterModel ("models/items/mega_h/tris.md2");

	hudmodel.cl_hud_armor1 =
		R_RegisterModel ("models/items/armor/jacket/tris.md2");
	hudmodel.cl_hud_armor2 =
		R_RegisterModel ("models/items/armor/combat/tris.md2");
	hudmodel.cl_hud_armor3 =
		R_RegisterModel ("models/items/armor/body/tris.md2");
	hudmodel.cl_hud_armor4 =
		R_RegisterModel ("models/items/armor/shard/tris.md2");

	hudmodel.cl_hud_bullet =
		R_RegisterModel ("models/items/ammo/bullets/medium/tris.md2");
	hudmodel.cl_hud_rockets =
		R_RegisterModel ("models/items/ammo/rockets/medium/tris.md2");
	hudmodel.cl_hud_slugs =
		R_RegisterModel ("models/items/ammo/slugs/medium/tris.md2");
	hudmodel.cl_hud_shells =
		R_RegisterModel ("models/items/ammo/shells/medium/tris.md2");
	hudmodel.cl_hud_grenades =
		R_RegisterModel ("models/items/ammo/grenades/medium/tris.md2");
	hudmodel.cl_hud_cells =
		R_RegisterModel ("models/items/ammo/cells/medium/tris.md2");

	hudmodel.cl_hud_adrenaline =
		R_RegisterModel ("models/items/adrenal/tris.md2");
	hudmodel.cl_hud_bandolier =
		R_RegisterModel ("models/items/band/tris.md2");
	hudmodel.cl_hud_envirosuit =
		R_RegisterModel ("models/items/enviro/tris.md2");
	hudmodel.cl_hud_invulnerability =
		R_RegisterModel ("models/items/invulner/tris.md2");
	hudmodel.cl_hud_megahealth =
		R_RegisterModel ("models/items/mega_h/tris.md2");
	hudmodel.cl_hud_quad =
		R_RegisterModel ("models/items/quaddama/tris.md2");
	hudmodel.cl_hud_rebreather =
		R_RegisterModel ("models/items/breather/tris.md2");
	hudmodel.cl_hud_silencer =
		R_RegisterModel ("models/items/silencer/tris.md2");
	hudmodel.cl_hud_pack = R_RegisterModel ("models/items/pack/tris.md2");
	hudmodel.cl_hud_powerscreen =
		R_RegisterModel ("models/items/armor/screen/tris.md2");
	hudmodel.cl_hud_powershield =
		R_RegisterModel ("models/items/armor/shield/tris.md2");
	hudmodel.cl_hud_airstrike =
		R_RegisterModel ("models/items/keys/target/tris.md2");
	hudmodel.cl_hud_carmak =
		R_RegisterModel ("models/items/c_head/tris.md2");

	hudmodel.cl_hud_blaster =
		R_RegisterModel ("models/weapons/g_blast/tris.md2");
	hudmodel.cl_hud_shotgun =
		R_RegisterModel ("models/weapons/g_shotg/tris.md2");
	hudmodel.cl_hud_sshotgun =
		R_RegisterModel ("models/weapons/g_shotg2/tris.md2");
	hudmodel.cl_hud_machinegun =
		R_RegisterModel ("models/weapons/g_machn/tris.md2");
	hudmodel.cl_hud_chaingun =
		R_RegisterModel ("models/weapons/g_chain/tris.md2");
	hudmodel.cl_hud_glauncher =
		R_RegisterModel ("models/weapons/g_launch/tris.md2");
	hudmodel.cl_hud_rocketl =
		R_RegisterModel ("models/weapons/g_rocket/tris.md2");
	hudmodel.cl_hud_hyperb =
		R_RegisterModel ("models/weapons/g_hyperb/tris.md2");
	hudmodel.cl_hud_rail =
		R_RegisterModel ("models/weapons/g_rail/tris.md2");
	hudmodel.cl_hud_bfg =
		R_RegisterModel ("models/weapons/g_bfg/tris.md2");

	hudmodel.cl_hud_bluekey =
		R_RegisterModel ("models/items/keys/key/tris.md2");
	hudmodel.cl_hud_comhead =
		R_RegisterModel ("models/monsters/commandr/head/tris.md2");
	hudmodel.cl_hud_datacd =
		R_RegisterModel ("models/items/keys/data_cd/tris.md2");
	hudmodel.cl_hud_dataspin =
		R_RegisterModel ("models/items/keys/spinner/tris.md2");
	hudmodel.cl_hud_powercube =
		R_RegisterModel ("models/items/keys/power/tris.md2");
	hudmodel.cl_hud_pyramid =
		R_RegisterModel ("models/items/keys/pyramid/tris.md2");
	hudmodel.cl_hud_redkey =
		R_RegisterModel ("models/items/keys/red_key/tris.md2");
	hudmodel.cl_hud_security =
		R_RegisterModel ("models/items/keys/pass/tris.md2");

	//== The Reckoning==//
	hudmodel.cl_hud_mslugs =
		R_RegisterModel ("models/objects/ammo/tris.md2");
	hudmodel.cl_hud_trap =
		R_RegisterModel ("models/weapons/g_trap/tris.md2");
	hudmodel.cl_hud_greenkey =
		R_RegisterModel ("models/items/keys/green_key/tris.md2");
	hudmodel.cl_hud_quadfire =
		R_RegisterModel ("models/items/quadfire/tris.md2");
	hudmodel.cl_hud_phallanx =
		R_RegisterModel ("models/weapons/g_shotx/tris.md2");
	hudmodel.cl_hud_ripper =
		R_RegisterModel ("models/weapons/g_boom/tris.md2");

	//== Ground Zero==//
	hudmodel.cl_hud_heatbeam =
		R_RegisterModel ("models/weapons/g_beamer/tris.md2");
	hudmodel.cl_hud_flech =
		R_RegisterModel ("models/weapons/g_etf_rifle/tris.md2");
	hudmodel.cl_hud_flech_a =
		R_RegisterModel ("models/ammo/am_flechette/fx.md2");
	hudmodel.cl_hud_proxlch =
		R_RegisterModel ("models/weapons/g_plaunch/tris.md2");
	hudmodel.cl_hud_proxlch_a =
		R_RegisterModel ("models/ammo/am_prox/tris.md2");
	hudmodel.cl_hud_chainfist =
		R_RegisterModel ("models/weapons/g_chainf/tris.md2");
	hudmodel.cl_hud_tesla =
		R_RegisterModel ("models/ammo/am_tesl/tris.md2");
	hudmodel.cl_hud_nuke =
		R_RegisterModel ("models/weapons/g_nuke/tris.md2");
	hudmodel.cl_hud_ir =
		R_RegisterModel ("models/items/goggles/tris.md2");
	hudmodel.cl_hud_double =
		R_RegisterModel ("models/items/ddamage/tris.md2");
	hudmodel.cl_hud_compass =
		R_RegisterModel ("models/objects/fire/tris.md2");
	hudmodel.cl_hud_vengnce =
		R_RegisterModel ("models/items/vengnce/tris.md2");
	hudmodel.cl_hud_hunter =
		R_RegisterModel ("models/items/hunter/tris.md2");
	hudmodel.cl_hud_def =
		R_RegisterModel ("models/items/defender/tris.md2");
	hudmodel.cl_hud_doppleganger =
		R_RegisterModel ("models/items/dopple/tris.md2");
}

extern cvar_t	*cl_hudModelScale;
qboolean stopRotation, xatrix;

void SCR_DrawHudModel (float x, float y, struct model_s *model) {
	refdef_t	refdef;
	vec3_t		center, rad;
	float		scale, hud_sx, hud_sy;
	float		screenAspect, scaledHeight;
	entity_t	entity;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!model)
		return;

	screenAspect = (float)viddef.width / (float)viddef.height;
	scaledHeight = 320.0 / screenAspect;

	scale = cl_hudScale->value;

	hud_sx = (float)viddef.width / 320.0 * scale;
	hud_sy = (float)viddef.height / scaledHeight * scale;

	memset (&refdef, 0, sizeof(refdef));
	memset (&entity, 0, sizeof(entity));

	R_ModelRadius (model, rad);
	R_ModelCenter (model, center);

	refdef.x = (int)x + cl_hudModelScale->value;
	refdef.y = (int)y - cl_hudModelScale->value * hud_sy;
	refdef.width = (24 + cl_hudModelScale->value) * hud_sx;
	refdef.height = (24 + cl_hudModelScale->value) * hud_sy;
	refdef.fov_x = 43;
	refdef.fov_y = 43;
	refdef.time = cls.realtime*0.001;
	refdef.viewangles[0] = 30;
	refdef.areabits = 0;
	refdef.num_entities = 1;
	refdef.entities = &entity;
	refdef.lightstyles = 0;
	refdef.rdflags = RDF_NOWORLDMODEL | RDF_NOCLEAR;
	VectorSet (refdef.vieworg, -rad[0] * 1.5, 0, rad[0]*0.8);

	entity.model = model;
	entity.flags = RF_NOSHADOW | RF_DEPTHHACK;
	entity.frame = 0;
	entity.oldframe = 0;
	entity.backlerp = 0.0;
	
	if (net_compatibility->integer) {
		if (stopRotation) {
			entity.angles[1] = 175.0;
		}
		else {
			entity.angles[1] = anglemod(cl.time / 16);
			entity.angleMod = qtrue;
		}
	}
	else {

		entity.angles[1] = anglemod(cl.time / 16);
		entity.angleMod = qtrue;
	}

	VectorNegate (center, entity.origin);

	if (xatrix)
		entity.origin[0] += 8.0;

	// Draw it
	R_RenderFrame (&refdef);
	refdef.num_entities++;
}


void DrawAltStringScaled (int x, int y, float scale_x, float scale_y, char *s) {
	Set_FontShader (qtrue);
	while (*s) {

		Draw_CharScaled (x, y, scale_x, scale_y, *s ^ 0x80);
		x += 8 * scale_x;
		s++;
	}
	Set_FontShader (qfalse);
}


/*
================
SCR_ExecuteLayoutString

================
*/

void SCR_ExecuteLayoutString (char *s) {
	int x, y;
	int value;
	char *token;
	int width;
	int index;
	clientinfo_t *ci;
	float scale, hud_sx, hud_sy;
	float screenAspect, scaledHeight;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!s[0])
		return;

	if (!cl_drawhud->integer)
		return;

	screenAspect = (float)viddef.width / (float)viddef.height;
	scaledHeight = 320.0 / screenAspect;

	scale = cl_hudScale->value;

	hud_sx = (float)viddef.width / 320.0 * scale;
	hud_sy = (float)viddef.height / scaledHeight * scale;

	x = 0;
	y = 0;
	width = 3;


	while (s) {
		token = COM_Parse (&s);
		if (!strcmp (token, "xl")) {
			token = COM_Parse (&s);
			x = atoi (token)*hud_sx;
			continue;
		}
		if (!strcmp (token, "xr")) {
			token = COM_Parse (&s);
			x = viddef.width + atoi (token)*hud_sx;
			continue;
		}
		if (!strcmp (token, "xv")) {
			token = COM_Parse (&s);
			x = viddef.width / 2 - (160 - atoi (token))*hud_sx;
			continue;
		}

		if (!strcmp (token, "yt")) {
			token = COM_Parse (&s);
			y = atoi (token)*hud_sy;
			continue;
		}
		if (!strcmp (token, "yb")) {
			token = COM_Parse (&s);
			y = viddef.height + atoi (token)*hud_sy;
			continue;
		}
		if (!strcmp (token, "yv")) {
			token = COM_Parse (&s);
			y = viddef.height / 2 - (120 - atoi (token))*hud_sy;
			continue;
		}

		if (!strcmp (token, "pic")) {	// draw a pic from a stat number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi (token)];


			if (value >= MAX_IMAGES)
				//	Com_Error(ERR_DROP, "Pic >= MAX_IMAGES");
				continue;
			if (cl.configstrings[CS_IMAGES + value]) {

				SCR_AddDirtyPoint (x, y);
				SCR_AddDirtyPoint (x + 23 * hud_sx, y + 23 * hud_sy);

				if (!cl_3dhud->integer)
					Draw_PicScaled (x, y, hud_sx, hud_sy, cl.configstrings[CS_IMAGES + value]);

			}
			continue;
		}

		if (!strcmp (token, "client")) {	// draw a deathmatch client block
			int score, ping, time;

			token = COM_Parse (&s);
			x = viddef.width / 2 - (160 - atoi (token))*hud_sx;
			token = COM_Parse (&s);
			y = viddef.height / 2 - (120 - atoi (token))*hud_sy;
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x + 159 * hud_sx, y + 31 * hud_sy);


			token = COM_Parse (&s);
			value = atoi (token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi (token);

			token = COM_Parse (&s);
			ping = atoi (token);

			token = COM_Parse (&s);
			time = atoi (token);



			DrawAltStringScaled (x + 32 * hud_sx, y, hud_sx, hud_sy, ci->name);

			Set_FontShader (qtrue);
			Draw_StringScaled (x + 32 * hud_sx, y + 8 * hud_sy, hud_sx, hud_sy, va ("Score:  %i", score));
			Draw_StringScaled (x + 32 * hud_sx, y + 16 * hud_sy, hud_sx, hud_sy, va ("Ping:  %i", ping));
			Draw_StringScaled (x + 32 * hud_sx, y + 24 * hud_sy, hud_sx, hud_sy, va ("Time:  %i", time));
			Set_FontShader (qfalse);

			if (!ci->icon)
				ci = &cl.baseclientinfo;
			Draw_PicScaled (x, y, hud_sx, hud_sy, ci->iconname);
			continue;
		}

		if (!strcmp (token, "ctf")) {	// draw a ctf client block
			int score, ping;
			char block[80];

			token = COM_Parse (&s);
			x = viddef.width * 0.5 - 160 + atoi (token)*hud_sx;
			token = COM_Parse (&s);
			y = viddef.height * 0.5 - 120 + atoi (token)*hud_sy;
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x + 159 * hud_sx, y + 31 * hud_sy);

			token = COM_Parse (&s);
			value = atoi (token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse (&s);
			score = atoi (token);

			token = COM_Parse (&s);
			ping = atoi (token);
			if (ping > 999)
				ping = 999;

			sprintf (block, "%3d %3d %-12.12s", score, ping, ci->name);

			if (value == cl.playernum)
				DrawAltStringScaled (x, y, hud_sx, hud_sy, block);
			else {
				Set_FontShader (qtrue);
				Draw_StringScaled (x, y, hud_sx, hud_sy, block);
				Set_FontShader (qfalse);
				continue;
			}
		}

		if (!strcmp (token, "picn")) {	// draw a pic from a name
			char bump[60];
			token = COM_Parse (&s);
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x + 23 * hud_sx, y + 23 * hud_sy);

			Draw_PicScaled (x, y, hud_sx, hud_sy, token);
			strcpy(bump, token);
			strcat(bump, "_bump");
			Draw_PicBumpScaled(x, y, hud_sx, hud_sy, token, bump);

			continue;
		}

		if (!strcmp (token, "num")) {	// draw a number
			token = COM_Parse (&s);
			width = atoi (token);
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi (token)];
			SCR_DrawField (x, y, hud_sx, hud_sy, 0, width, value);
			continue;
		}

		if (!strcmp (token, "hnum")) {	// health number
			int color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_HEALTH];
			if (value > 25)
				color = 0;		// green
			else if (value > 0)
				color = (cl.frame.serverframe >> 2) & 1;	// flash
			else
				color = 1;

		// ugly yellow rectangle under stat digits - remove it!

		/*	if (cl.frame.playerstate.stats[STAT_FLASHES] & 1)
				Draw_PicScaled (x, y, hud_sx, hud_sy, "field_3"); */

			SCR_DrawField (x, y, hud_sx, hud_sy, color, width, value);
			continue;
		}

		if (!strcmp (token, "anum")) {	// ammo number
			int color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_AMMO];
			if (value > 5)
				color = 0;		// green
			else if (value >= 0)
				color = (cl.frame.serverframe >> 2) & 1;	// flash
			else
				continue;		// negative number = don't show

		/*	if (cl.frame.playerstate.stats[STAT_FLASHES] & 4)
				Draw_PicScaled (x, y, hud_sx, hud_sy, "field_3"); */

			SCR_DrawField (x, y, hud_sx, hud_sy, color, width, value);
			continue;
		}

		if (!strcmp (token, "rnum")) {	// armor number
			int color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_ARMOR];
			if (value < 1)
				continue;

			color = 0;			// green

		/*	if (cl.frame.playerstate.stats[STAT_FLASHES] & 2)
				Draw_PicScaled (x, y, hud_sx, hud_sy, "field_3"); */

			SCR_DrawField (x, y, hud_sx, hud_sy, color, width, value);
			continue;
		}


		if (!strcmp (token, "stat_string")) {

			token = COM_Parse (&s);
			index = atoi (token);
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			index = cl.frame.playerstate.stats[index];
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			Set_FontShader (qtrue);
			Draw_StringScaled (x, y, hud_sx, hud_sy, cl.configstrings[index]);
			Set_FontShader (qfalse);
			continue;
		}

		if (!strcmp (token, "cstring")) {
			token = COM_Parse (&s);
			DrawHUDString (x, y, hud_sx, hud_sy, 320, 0, token);
			continue;
		}

		if (!strcmp (token, "string")) {
			token = COM_Parse (&s);
			Set_FontShader (qtrue);
			Draw_StringScaled (x, y, hud_sx, hud_sy, token);
			Set_FontShader (qfalse);
			continue;
		}

		if (!strcmp (token, "cstring2")) {	// F1 messages upper block
			token = COM_Parse (&s);
			DrawHUDString (x, y, hud_sx, hud_sy, 320, 0x80, token);
			continue;
		}

		if (!strcmp (token, "string2")) {	// F1 messages lower block
			token = COM_Parse (&s);
			DrawAltStringScaled (x, y, hud_sx, hud_sy, token);
			continue;
		}

		if (!strcmp (token, "if")) {	// draw a number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi (token)];
			if (!value) {		// skip to endif
				while (s && strcmp (token, "endif")) {
					token = COM_Parse (&s);
				}
			}

			continue;
		}


	}
}






void SCR_ExecuteLayoutString3d (char *s) {
	int x, y;
	int value;
	char *token;
	float scale, hud_sx, hud_sy;
	float screenAspect, scaledHeight;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!s[0])
		return;

	if (!cl_drawhud->integer)
		return;

	screenAspect = (float)viddef.width / (float)viddef.height;
	scaledHeight = 320.0 / screenAspect;

	scale = cl_hudScale->value;

	hud_sx = (float)viddef.width / 320.0 * scale;
	hud_sy = (float)viddef.height / scaledHeight * scale;

	x = 0;
	y = 0;

	while (s) {
		token = COM_Parse (&s);
		if (!strcmp (token, "xl")) {
			token = COM_Parse (&s);
			x = atoi (token)*hud_sx;
			continue;
		}
		if (!strcmp (token, "xr")) {
			token = COM_Parse (&s);
			x = viddef.width + atoi (token)*hud_sx;
			continue;
		}
		if (!strcmp (token, "xv")) {
			token = COM_Parse (&s);
			x = viddef.width / 2 - (160 - atoi (token))*hud_sx;
			continue;
		}

		if (!strcmp (token, "yt")) {
			token = COM_Parse (&s);
			y = atoi (token)*hud_sy;
			continue;
		}
		if (!strcmp (token, "yb")) {
			token = COM_Parse (&s);
			y = viddef.height + atoi (token)*hud_sy;
			continue;
		}
		if (!strcmp (token, "yv")) {
			token = COM_Parse (&s);
			y = viddef.height / 2 - (120 - atoi (token))*hud_sy;
			continue;
		}


		if (!strcmp (token, "pic")) {	// draw a pic from a stat number
			token = COM_Parse (&s);
			value = cl.frame.playerstate.stats[atoi (token)];


			if (value >= MAX_IMAGES)
				//	Com_Error(ERR_DROP, "Pic >= MAX_IMAGES");
				continue;

			if (cl.configstrings[CS_IMAGES + value]) {

				SCR_AddDirtyPoint (x, y);
				SCR_AddDirtyPoint (x + 24 * hud_sx, y + 24 * hud_sy);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_mask"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_predator);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_help")) {
					stopRotation = qtrue;
					SCR_DrawHudModel(x, y - cl_hudModelScale->value * hud_sy, hudmodel.cl_hud_comp);
				} else
					stopRotation = qfalse;

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_health3"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_health_large);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_health2"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_health_stim);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_health"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_health_medium);
				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_health4"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_health_mega);

				// weapon =====
				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_blaster"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_blaster);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_shotgun"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_shotgun);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_sshotgun"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_sshotgun);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_machinegun"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_machinegun);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_chaingun"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_chaingun);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_glauncher"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_glauncher);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_rlauncher"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_rocketl);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_hyperblaster"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_hyperb);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_railgun"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_rail);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_bfg"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_bfg);

				// =================

				// armor====
				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_jacketarmor"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_armor1);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_combatarmor"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_armor2);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_bodyarmor"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_armor3);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_shard"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_armor4);

				// =====================
				// Ammo=======
				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_bullets"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_bullet);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_shells"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_shells);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_grenades"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_grenades);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_rockets"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_rockets);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_cells"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_cells);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_slugs"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_slugs);

				// ====================
				// Power-ups===========
				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_adrenaline"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_adrenaline);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_bandolier"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_bandolier);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_envirosuit"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_envirosuit);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_invulnerability"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_invulnerability);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_megahealth"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_megahealth);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_quad"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_quad);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_rebreather"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_rebreather);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"p_silencer"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_silencer);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_pack"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_pack);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"i_airstrike"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_airstrike);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_fixme"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_carmak);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"i_powerscreen"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_powerscreen);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value],
					"i_powershield"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_powershield);

				// ====================
				// Keys================

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_bluekey"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_bluekey);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_comhead"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_comhead);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_datacd"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_datacd);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_dataspin"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_dataspin);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_powercube"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_powercube);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_pyramid"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_pyramid);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_redkey"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_redkey);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_security"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_security);

				// ==The Reckoning==//

				if (!strcmp
				(cl.configstrings[CS_IMAGES + value], "a_mslugs")) {
					xatrix = qtrue;
					SCR_DrawHudModel(x, y, hudmodel.cl_hud_mslugs);
				}
				else
					xatrix = qfalse;

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_trap"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_trap);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "k_green"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_greenkey);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_quadfire"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_quadfire);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_phallanx"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_phallanx);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_ripper"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_ripper);

				// ======Rogue========
				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_heatbeam"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_heatbeam);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_etf_rifle"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_flech);

				if (!strcmp
				(cl.configstrings[CS_IMAGES + value], "a_flechettes"))
					SCR_DrawHudModel(x, y, hudmodel.cl_hud_flech_a);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_proxlaunch"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_proxlch);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_prox"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_proxlch_a);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "w_chainfist"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_chainfist);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "a_tesla"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_tesla);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_nuke"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_nuke);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_contain"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_nuke);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "i_nuke"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_nuke);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_ir"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_ir);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_double"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_double);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_compass"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_compass);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_vengeance"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_vengnce);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_hunter"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_hunter);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_defender"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_def);

				if (!strcmp
					(cl.configstrings[CS_IMAGES + value], "p_doppleganger"))
					SCR_DrawHudModel (x, y, hudmodel.cl_hud_doppleganger);

			}
			continue;
		}

	}
}


/*
================
SCR_DrawStats

The status bar is a small layout program that
is based on the stats array
================
*/

void SCR_DrawStats () {
	if (cl_3dhud->integer)
		SCR_ExecuteLayoutString3d (cl.configstrings[CS_STATUSBAR]);

	SCR_ExecuteLayoutString (cl.configstrings[CS_STATUSBAR]);
}


/*
================
SCR_DrawLayout

================
*/
#define	STAT_LAYOUTS		13

void SCR_DrawLayout (void) {
	if (!cl.frame.playerstate.stats[STAT_LAYOUTS])
		return;
	SCR_ExecuteLayoutString (cl.layout);

}


/*
================
CL_ParseInventory
================
*/
void CL_ParseInventory (void) {
	int i;

	for (i = 0; i < MAX_ITEMS; i++)
		cl.inventory[i] = MSG_ReadShort (&net_message);
}


/*
================
Inv_DrawString
================
*/
void Inv_DrawString (int x, int y, char *string) {
	while (*string) {
		Draw_CharScaled (x, y, cl_fontScale->value, cl_fontScale->value, *string);
		x += 8 * cl_fontScale->value;
		string++;
	}
}

void SetStringHighBit (char *s) {
	while (*s)
		*s++ |= 128;
}

/*
================
CL_DrawInventory
================
*/
#define	DISPLAY_ITEMS	17

void CL_DrawInventory (void) {
	int i, j;
	int num, selected_num, item;
	int index[MAX_ITEMS];
	char string[1024];
	int x, y;
	char binding[1024];
	char *bind;
	int selected;
	int top;

	selected = cl.frame.playerstate.stats[STAT_SELECTED_ITEM];

	num = 0;
	selected_num = 0;
	for (i = 0; i < MAX_ITEMS; i++) {
		if (i == selected)
			selected_num = num;
		if (cl.inventory[i]) {
			index[num] = i;
			num++;
		}
	}

	// determine scroll point
	top = selected_num - DISPLAY_ITEMS * 0.5;
	if (num - top < DISPLAY_ITEMS)
		top = num - DISPLAY_ITEMS;
	if (top < 0)
		top = 0;

	x = (viddef.width - 256 * cl_fontScale->value) * 0.5;
	y = (viddef.height - 240 * cl_fontScale->value) * 0.5;

	// repaint everything next frame
	SCR_DirtyScreen ();

	Draw_ScaledPic (x, y + 8, (float)cl_fontScale->value, (float)cl_fontScale->value, i_inventory);
	Draw_PicBumpScaled(x, y + 8, (float)cl_fontScale->value, (float)cl_fontScale->value, "inventory", "inventory_bump");
	
	y += 24 * cl_fontScale->value;
	x += 24 * cl_fontScale->value;

	Set_FontShader (qtrue);

	Inv_DrawString (x, y, "hotkey ### item");

	Inv_DrawString (x, y + 8 * cl_fontScale->value, "------ --- ----");

	y += 16 * cl_fontScale->value;

	for (i = top; i < num && i < top + DISPLAY_ITEMS; i++) {
		item = index[i];
		// search for a binding
		Com_sprintf (binding, sizeof(binding), "use %s",
			cl.configstrings[CS_ITEMS + item]);
		bind = "";
		for (j = 0; j < 256; j++)
		if (keybindings[j] && !Q_stricmp (keybindings[j], binding)) {
			bind = Key_KeynumToString (j);
			break;
		}

		Com_sprintf (string, sizeof(string), "%6s %3i %s", bind,
			cl.inventory[item], cl.configstrings[CS_ITEMS + item]);
		if (item != selected)
			SetStringHighBit (string);
		else					// draw a blinky cursor by the selected
			// item
		{
			if ((int)(cls.realtime * 10) & 1)
				Draw_CharScaled (x - 8, y, cl_fontScale->value, cl_fontScale->value, 15);

		}
		Inv_DrawString (x, y, string);
		y += 8 * cl_fontScale->value;
	}

	Set_FontShader (qfalse);
}
