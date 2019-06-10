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
// client.h -- primary header for client

//define    PARANOID            // speed sapping error checking

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ref.h"
#include "vid.h"

#include "screen.h"
#include "sound.h"
#include "input.h"
#include "keys.h"
#include "cl_console.h"
#include "snd_loc.h"

#ifndef VIDDEF_LOCK
#define VIDDEF_LOCK
typedef struct {
	unsigned width, height;		// coordinates from main game
} viddef_t;
#endif
//#define viddef vid
//extern viddef_t vid;
extern viddef_t viddef;

//=============================================================================
// Sound effect ID's:
#define weapons_blastf1a		0
#define weapons_hyprbf1a		1
#define weapons_machgf1b		2
#define weapons_machgf2b		3
#define weapons_machgf3b		4
#define weapons_machgf4b		5
#define weapons_machgf5b		6
#define weapons_shotgf1b		7
#define weapons_shotgr1b		8
#define weapons_sshotf1b		9
#define weapons_railgf1a		10
#define weapons_rocklf1a		11
#define weapons_rocklr1b		12
#define weapons_grenlf1a		13
#define weapons_grenlr1b		14
#define weapons_bfg__f1y		15
#define misc_talk				16
#define menu_in_sound			17
#define menu_move_sound			18
#define menu_out_sound			19
#define id_cl_sfx_ric1			20	// +0
#define id_cl_sfx_ric2			21	// +1
#define id_cl_sfx_ric3			22	// +2
#define id_cl_sfx_lashit		23
#define id_cl_sfx_spark5		24	// +0
#define id_cl_sfx_spark6		25	// +1
#define id_cl_sfx_spark7		26	// +2
#define id_cl_sfx_railg			27
#define id_cl_sfx_rockexp		28
#define id_cl_sfx_grenexp		29
#define id_cl_sfx_watrexp		30
//q2xp
#define id_cl_sfx_lava			31
#define id_cl_sfx_shell			32
#define id_cl_sfx_debris		33
#define id_cl_sfx_footsteps_0	34
#define id_cl_sfx_footsteps_1	35
#define id_cl_sfx_footsteps_2	36
#define id_cl_sfx_footsteps_3	37
//PGM
#define id_cl_sfx_lightning		38
#define id_cl_sfx_disrexp		39

typedef enum {
	ex_free, ex_explosion, ex_misc, ex_flash, ex_mflash, ex_poly, ex_poly2
} exptype_t;

typedef struct {
	exptype_t type;
	entity_t ent;

	int frames;
	float light;
	vec3_t lightcolor;
	float start;
	int baseframe;
} explosion_t;


//=============================================================================
#include "../game/game.h"
trace_t SV_Trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end,
	edict_t * passedict, int contentmask);
trace_t CL_Trace (vec3_t start, vec3_t end, float size, int contentmask);
void CL_ParticleSmoke2 (vec3_t org, vec3_t dir, float r, float g, float b,
	int count, qboolean add);
void CL_LaserParticle (vec3_t org, vec3_t dir, int count);
#define	MAX_LASERS	32
typedef struct {
	entity_t ent;
	int endtime;
} laser_t;
laser_t cl_lasers[MAX_LASERS];
extern vec3_t cl_indexPalette[256];

qboolean loadingMessage;
char loadingMessages[5][96];
float loadingPercent;

extern vec3_t viewweapon;
#define NUM_CROSS_FRAMES 15

extern vec3_t cl_indexPalette[256];

typedef struct {
	qboolean valid;				// cleared if delta parsing was invalid
	int serverframe;
	int servertime;				// server time the message is valid for
	// (in msec)
	int deltaframe;
	byte areabits[MAX_MAP_AREAS / 8];	// portalarea visibility bits
	player_state_t playerstate;
	int num_entities;
	int parse_entities;			// non-masked index into cl_parse_entities
	int rsaFrame;
	// array
} frame_t;

typedef struct {
	entity_state_t baseline;	// delta from this if not from a previous
	// frame
	entity_state_t current;
	entity_state_t prev;		// will always be valid, but might just be
	// a copy of current

	int serverframe;			// if not current, this ent isn't in the
	// frame

	int trailcount;				// for diminishing grenade trails
	vec3_t lerp_origin;			// for trails (variable hz)

	int fly_stoptime;
} centity_t;

#define MAX_CLIENTWEAPONMODELS		20	// PGM -- upped from 16 to fit the
// chainfist vwep

typedef struct {
	char name[MAX_QPATH];
	char cinfo[MAX_QPATH];
	struct image_s *skin;
	struct image_s *icon;
	struct image_s *bump;
	char iconname[MAX_QPATH];
	char sex[MAX_QPATH];
	struct model_s *model;
	struct model_s *weaponmodel[MAX_CLIENTWEAPONMODELS];
} clientinfo_t;

extern char cl_weaponmodels[MAX_CLIENTWEAPONMODELS][MAX_QPATH];
extern int num_cl_weaponmodels;
extern char *rsaDat;

#define	CMD_BACKUP		64		// allow a lot of command backups for very
// fast systems

//
// the client_state_t structure is wiped completely at every
// server map change
//
typedef struct {
	int timeoutcount;

	int timedemo_frames;
	int timedemo_start;

	qboolean refresh_prepped;	// qfalse if on new level or new ref dll
	qboolean sound_prepped;		// ambient sounds can start
	qboolean force_refdef;		// vid has changed, so we can't use a
	// paused refdef

	int parse_entities;			// index (not anded off) into
	// cl_parse_entities[]

	usercmd_t cmd;
	usercmd_t cmds[CMD_BACKUP];	// each mesage will send several old cmds
	int cmd_time[CMD_BACKUP];	// time sent, for calculating pings
	short predicted_origins[CMD_BACKUP][3];	// for debug comparing against
	// server

	float predicted_step;		// for stair up smoothing
	unsigned predicted_step_time;

	vec3_t predicted_origin;	// generated by CL_PredictMovement
	vec3_t predicted_angles;
	vec3_t prediction_error;



	frame_t frame;				// received from server
	int surpressCount;			// number of messages rate supressed
	frame_t frames[UPDATE_BACKUP];

	// the client maintains its own idea of view angles, which are
	// sent to the server each frame.  It is cleared to 0 upon entering
	// each level.
	// the server sends a delta each frame which is added to the locally
	// tracked view angles to account for standing on rotating objects,
	// and teleport direction changes
	vec3_t viewangles;
	float		viewangles_YAW;
	float		viewangles_PITCH;

	int time;					// this is the time value that the client
	// is rendering at.  always <= cls.realtime
	float lerpfrac;				// between oldframe and frame
	
	int minFps, maxFps;

	refdef_t refdef;

	vec3_t v_forward, v_right, v_up;	// set when refdef.angles is set

	//
	// transient data from server
	//
	char layout[1024];			// general 2D overlay
	int inventory[MAX_ITEMS];

	//
	// non-gameserver infornamtion
	// FIXME: move this cinematic stuff into the cin_t structure
	qFILE cinematic_file;
	int cinematictime;			// cls.realtime for first cinematic frame
	int cinematicframe;
	char cinematicpalette[768];
	qboolean cinematicpalette_active;

	//
	// server state information
	//
	qboolean attractloop;		// running the attract loop, any key will
	// menu
	int servercount;			// server identification for prespawns
	char gamedir[MAX_QPATH];
	int playernum;

	char configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];

	//
	// locally derived information from server state
	//
	struct model_s *model_draw[MAX_MODELS];
	struct cmodel_s *model_clip[MAX_MODELS];

	ALuint sound_precache[MAX_SOUNDS];
	const char *sound_sexedname[MAX_SOUNDS];

	unsigned long sound_precache_hacks[MAX_SOUNDS];
	ALfloat sound_precache_rolloff_factor[MAX_SOUNDS];
	ALfloat sound_precache_gain[MAX_SOUNDS];
#define			sound_precache_hacks_FLAT2D					1
#define			sound_precache_hacks_AUTOFIX_gain			2
#define			sound_precache_hacks_AUTOFIX_rolloff_factor	4

	struct image_s *image_precache[MAX_IMAGES];

	clientinfo_t clientinfo[MAX_CLIENTS];
	clientinfo_t baseclientinfo;

	vec3_t v_Forward, v_Right, v_Up;	// set when refdef.angles is set
} client_state_t;

extern client_state_t cl;

/*
==================================================================

the client_static_t structure is persistant through an arbitrary number
of server connections

==================================================================
*/

typedef enum {
	ca_uninitialized,
	ca_disconnected,			// not talking to a server
	ca_connecting,				// sending request packets to the server
	ca_connected,				// netchan_t established, waiting for
	// svc_serverdata
	ca_active					// game views should be displayed
} connstate_t;

typedef enum {
	dl_none,
	dl_model,
	dl_sound,
	dl_skin,
	dl_single
} dltype_t;						// download type

typedef enum {
	key_game, key_console, key_message, key_menu
} keydest_t;

typedef struct {
	connstate_t state;
	keydest_t key_dest;
	qboolean consoleActive;
	qboolean menuActive;

	int framecount;
	int realtime;				// always increasing, no clamping, etc
	float frametime;			// seconds since last frame

	// screen rendering information
	float disable_screen;		// showing loading plaque between levels
	// or changing rendering dlls
	// if time gets > 30 seconds ahead, break it
	int disable_servercount;	// when we receive a frame and
	// cl.servercount
	// > cls.disable_servercount, clear disable_screen

	// connection information
	char servername[MAX_OSPATH];	// name of server from original
	// connect
	float connect_time;			// for connection retransmits

	int quakePort;				// a 16 bit value that allows quake
	// servers
	// to work around address translating routers
	netchan_t netchan;
	int serverProtocol;			// in case we are doing some kind of
	// version hack

	int challenge;				// from the server to use for connecting

	FILE *download;				// file transfer from server
	char downloadtempname[MAX_OSPATH];
	char downloadname[MAX_OSPATH];
	int downloadnumber;
	dltype_t downloadtype;
	int downloadpercent;

	// demo recording info must be here, so it isn't cleared on level change
	qboolean demorecording;
	qboolean demowaiting;		// don't record until a non-delta message
	// is received
	FILE *demofile;

} client_static_t;

extern client_static_t cls;

//=============================================================================

//
// cvars
//
extern cvar_t *cl_stereo_separation;
extern cvar_t *cl_stereo;

extern cvar_t *cl_gun;
extern cvar_t *cl_add_blend;
extern cvar_t *cl_add_lights;
extern cvar_t *cl_add_particles;
extern cvar_t *cl_add_entities;
extern cvar_t *cl_predict;
extern cvar_t *cl_footsteps;
extern cvar_t *cl_noskins;
extern cvar_t *cl_autoskins;

extern cvar_t *zoomfov;

#define MAX_FOV 120

extern cvar_t *cl_upspeed;
extern cvar_t *cl_forwardspeed;
extern cvar_t *cl_sidespeed;

extern cvar_t *cl_yawspeed;
extern cvar_t *cl_pitchspeed;

extern cvar_t *cl_run;

extern cvar_t *cl_anglespeedkey;

extern cvar_t *cl_shownet;
extern cvar_t *cl_showmiss;
extern cvar_t *cl_showclamp;

extern cvar_t *dmflags;

extern cvar_t *lookspring;
extern cvar_t *lookstrafe;
extern cvar_t *sensitivity;

extern cvar_t *m_pitch;
extern cvar_t *m_yaw;
extern cvar_t *m_forward;
extern cvar_t *m_side;

extern cvar_t *freelook;

extern cvar_t *cl_lightlevel;	// FIXME HACK

extern cvar_t *cl_paused;
extern cvar_t *cl_timedemo;

extern cvar_t *cl_vwep;
extern cvar_t *cl_drawFPS;
extern cvar_t *cl_drawTime;


extern cvar_t *cl_thirdPerson;;
extern cvar_t *cl_thirdPersonAngle;
extern cvar_t *cl_thirdPersonRange;
extern cvar_t *cl_blood;

extern cvar_t *music_source;
extern cvar_t *music_volume;

extern cvar_t *cl_brass;
extern cvar_t *cl_brassTimeScale;

extern cvar_t *cl_3dhud;

extern cvar_t *cl_railcore_red;
extern cvar_t *cl_railcore_green;
extern cvar_t *cl_railcore_blue;
extern cvar_t *cl_railspiral_red;
extern cvar_t *cl_railspiral_green;
extern cvar_t *cl_railspiral_blue;

extern cvar_t *cl_decals;
extern cvar_t *net_compatibility;
extern cvar_t *cl_drawhud;
extern cvar_t *deathmatch;
extern cvar_t *s_initSound;
extern cvar_t *cl_fontScale;
extern cvar_t *cl_itemsBobbing;
extern cvar_t *scr_showTexName;

extern ALuint cl_sfx_lava;
extern ALuint cl_sfx_shell;
extern ALuint cl_sfx_debris;
extern struct model_s *cl_mod_mshell;
extern struct model_s *cl_mod_sshell;
extern struct model_s *cl_mod_debris1;
extern struct model_s *cl_mod_debris2;
extern struct model_s *cl_mod_debris3;
extern struct model_s *cl_mod_gib0;
extern struct model_s *cl_mod_gib1;
extern struct model_s *cl_mod_gib2;
extern struct model_s *cl_mod_gib3;
extern struct model_s *cl_mod_gib4;
extern struct model_s *cl_mod_gib5;
extern struct model_s *cl_mod_debris0;
extern vec3_t r_origin;
extern vec3_t  smoke_puff, shell_puff;

typedef struct {
	int key;					// so entities can reuse same entry
	vec3_t color;
	vec3_t origin;
	float radius;
	float die;					// stop lighting after this time
	float decay;				// drop this each second
	float minlight;				// don't add when contributing less
} cdlight_t;

extern centity_t cl_entities[MAX_EDICTS];
extern cdlight_t cl_dlights[MAX_DLIGHTS];

// the cl_parse_entities must be large enough to hold UPDATE_BACKUP frames of
// entities, so that when a delta compressed message arives from the server
// it can be un-deltad from the original
#define	MAX_PARSE_ENTITIES	1024
extern entity_state_t cl_parse_entities[MAX_PARSE_ENTITIES];

//=============================================================================

extern netadr_t net_from;
extern sizebuf_t net_message;

qboolean CL_CheckOrDownloadFile (char *filename);

//ROGUE
typedef struct cl_sustain {
	int id;
	int type;
	int endtime;
	int nextthink;
	int thinkinterval;
	vec3_t org;
	vec3_t dir;
	int color;
	int count;
	int magnitude;
	void (*think) (struct cl_sustain * self);
} cl_sustain_t;

#define MAX_SUSTAINS		32
void CL_ParticleSteamEffect2 (cl_sustain_t * self);

void CL_TeleporterParticles (entity_state_t * ent);
void CL_ParticleEffect (vec3_t org, vec3_t dir, float r, float g, float b,
	int count);

int CL_PMpointcontents (vec3_t point);
void CL_GetEntityOrigin (int ent, vec3_t origin);
void LoadHudEnts (void);
void CL_ParticleBlood (vec3_t org, vec3_t dir, int count);
void CL_ParticleRick (vec3_t org, vec3_t dir);
void CL_ParticleSplash (vec3_t org, vec3_t dir, float r, float g, float b);
void CL_ParticleSmoke (vec3_t org, vec3_t dir, int count);
void CL_InitImages ();
void CL_TeleportParticles (vec3_t org);
void CL_ClearParticles (void);
void CL_ClearDecals (void);
void CL_ClearClEntities ();
void CL_RocketFire (vec3_t start, vec3_t end);
void CL_AddClEntities ();
void VectorReflect (const vec3_t v, const vec3_t normal, vec3_t out);
void V_ClearScene (void);
void CL_ParticleSpark (vec3_t org, vec3_t dir, int count);
void CL_LaserParticle2 (vec3_t org, vec3_t dir, int color, int count);
void CL_Debris (vec3_t org, vec3_t dir);
void CL_Explosion (vec3_t org);
void CL_ParticleBlood2 (vec3_t org, vec3_t dir, int count);
void CL_ParticleHeadBlood (vec3_t org);
void CL_AddLasers (void);
void CL_GibExplosion (vec3_t org, vec3_t dir);
void CL_ParticleTracer (vec3_t start, vec3_t end);
qboolean AL_Init (int hardreset);
void AL_Shutdown (void);
void S_fastsound (vec3_t origin, int entnum, int entchannel,
	ALuint bufferNum, ALfloat gain, ALfloat rolloff_factor);
void CL_ParticleArmorSpark (vec3_t org, vec3_t dir, int count,
	qboolean power);
void CL_ParticleGibBlood (vec3_t org);
void CL_ParticleGunSmoke (vec3_t org, vec3_t dir, int count);
//=================================================
extern vec3_t dclAng;

// ========
// PGM
typedef struct particle_s {
	struct particle_s *next;
	float time;
	float endTime;
	vec3_t org;
	vec3_t oldOrg;
	vec3_t vel;
	vec3_t accel;
	vec3_t mins;
	vec3_t maxs;
	vec3_t length;
	vec3_t dir;
	int type;
	vec3_t color;
	vec3_t colorVel;
	float alpha;
	float alphavel;
	float size;
	float sizeVel;
	float lightradius;
	vec3_t lcolor;
	float orient;
	int flags;
	int sFactor;
	int dFactor;
	float len;
	float endLen;

} cparticle_t;
/*
typedef struct {
qboolean	isactive;

vec3_t		lightcol;
float		light;
float		lightvel;
} cplight_t;
*/
// Berserker client entities code
#define     MAX_CLENTITIES     256

typedef struct clentity_s {
	struct clentity_s *next;
	float time;
	vec3_t org;
	vec3_t lastOrg;
	vec3_t vel;
	vec3_t accel;
	struct model_s *model;
	struct entity_s *ent;
	float ang[3];
	float avel[3];
	float alpha;
	float alphavel;
	int flags;

} clentity_t;



#define	PARTICLE_GRAVITY	40.f

// PMM
#define INSTANT_PARTICLE	-10000.f
// PGM
// ========

void CL_ClearEffects (void);
void CL_ClearTEnts (void);
void CL_BlasterTrail (vec3_t start, vec3_t end);
void CL_QuadTrail (vec3_t start, vec3_t end);
void CL_RailTrail (vec3_t start, vec3_t end);
void CL_BubbleTrail (vec3_t start, vec3_t end);
void CL_FlagTrail (vec3_t start, vec3_t end, float r, float g, float b);

// RAFAEL
void CL_IonripperTrail (vec3_t start, vec3_t end);

// ========
// PGM

void CL_Flashlight (int ent, vec3_t pos);
void CL_ForceWall (vec3_t start, vec3_t end, int color);
void CL_Heatbeam (vec3_t start, vec3_t end);
void CL_ParticleSteamEffect (vec3_t org, vec3_t dir, int color, int count,
	int magnitude);
void CL_TrackerTrail (vec3_t start, vec3_t end, int particleColor);

void CL_TagTrail (vec3_t start, vec3_t end, float color);
void CL_ColorFlash (vec3_t pos, int ent, int intensity, float r, float g,
	float b);
void CL_Tracker_Shell (vec3_t origin);
void CL_MonsterPlasma_Shell (vec3_t origin);
void CL_Widowbeamout (cl_sustain_t * self);
void CL_Nukeblast (cl_sustain_t * self);
void CL_WidowSplash (vec3_t org);
// PGM
// ========

int CL_ParseEntityBits (unsigned *bits);
void CL_ParseDelta (entity_state_t * from, entity_state_t * to, int number,
	int bits);
void CL_ParseFrame (void);

void CL_ParseTEnt (void);
void CL_ParseConfigString (void);
void CL_ParseMuzzleFlash (void);
void CL_ParseMuzzleFlash2 (void);

void CL_SetLightstyle (int i);

void CL_RunDLights (void);
void CL_RunLightStyles (void);

void CL_AddEntities (void);
void CL_AddDLights (void);
void CL_AddTEnts (void);
void CL_AddLightStyles (void);

//=================================================

void CL_PrepRefresh (void);
void CL_RegisterSounds (void);

void CL_Quit_f (void);



//
// cl_main
//

void CL_Init (void);

void CL_FixUpGender (void);
void CL_Disconnect (void);
void CL_Disconnect_f (void);
void CL_PingServers_f (void);
void CL_Snd_Restart_f (void);
void CL_RequestNextDownload (void);

void CL_AddDecalToScene (vec3_t origin, vec3_t dir,
	float red, float green, float blue, float alpha,
	float endRed, float endGreen, float endBlue,
	float endAlpha, float size,
	float endTime, int type, int flags, float angle,
	int sFactor, int dFactor);


trace_t CL_PMTraceWorld (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end,
	int mask, qboolean checkAliases);

void CL_NewDlight (int key, vec3_t org, float r, float g, float b,
	float radius, float time);

void CL_ParticleRailRick (vec3_t org, vec3_t dir);

#ifndef __gl_h_
#ifndef __GL_H__
#define  GL_ZERO						0
#define  GL_ONE							1
#define  GL_SRC_COLOR					0x0300
#define  GL_ONE_MINUS_SRC_COLOR			0x0301
#define  GL_SRC_ALPHA					0x0302
#define  GL_ONE_MINUS_SRC_ALPHA			0x0303
#define  GL_DST_ALPHA					0x0304
#define  GL_ONE_MINUS_DST_ALPHA			0x0305
#define  GL_DST_COLOR					0x0306
#define  GL_ONE_MINUS_DST_COLOR			0x0307
#define  GL_SRC_ALPHA_SATURATE			0x0308
#define  GL_CONSTANT_COLOR				0x8001
#define  GL_ONE_MINUS_CONSTANT_COLOR	0x8002
#define  GL_CONSTANT_ALPHA				0x8003
#define  GL_ONE_MINUS_CONSTANT_ALPHA	0x8004
#endif
#endif

int	Developer_searchpath(int who);
qboolean modName(const char *gameDir);

//
// cl_input
//
typedef struct {
	int down[2];				// key nums holding it down
	unsigned downtime;			// msec timestamp
	unsigned msec;				// msec down this frame
	int state;
} kbutton_t;

extern kbutton_t in_mlook, in_klook;
extern kbutton_t in_strafe;
extern kbutton_t in_speed;
extern kbutton_t in_zoom, in_flashlight;

void CL_InitInput (void);
void CL_SendCmd (void);

void CL_ClearState (void);

void CL_ReadPackets (void);
void CL_BaseMove (usercmd_t * cmd);

void IN_CenterView (void);

float CL_KeyState (kbutton_t * key);
char *Key_KeynumToString (int keynum);

//
// cl_demo.c
//
void CL_WriteDemoMessage (void);
void CL_Stop_f (void);
void CL_Record_f (void);

//
// cl_parse.c
//
extern char *svc_strings[256];

void CL_ParseServerMessage (void);
void CL_LoadClientinfo (clientinfo_t * ci, char *s);
void SHOWNET (char *s);
void CL_ParseClientinfo (int player);
void CL_Download_f (void);




//
// cl_view.c
//
extern int gun_frame;
extern struct model_s *gun_model;

void V_Init (void);
void V_RenderView ();
void V_AddEntity (entity_t * ent);

void V_AddParticle (vec3_t org, vec3_t length, vec3_t color, float alpha,
	int type, float size, int sFactor, int dFactor,
	int flags, int time, float orient, float len,
	vec3_t oldOrg, vec3_t dir);

void V_AddLight (vec3_t org, float intensity, float r, float g, float b, vec3_t ang, float cone, int filter);

void V_AddLightStyle (int style, float r, float g, float b);

//
// cl_tent.c
//
void CL_RegisterTEntModels (void);


//
// cl_pred.c
//
void CL_CheckPredictionError (void);

//
// cl_fx.c
//
cdlight_t *CL_AllocDlight (int key);
void CL_BigTeleportParticles (vec3_t org);
void CL_RocketTrail (vec3_t start, vec3_t end, centity_t * old);
void CL_DiminishingTrail (vec3_t start, vec3_t end, centity_t * old,
	int flags);
void CL_FlyEffect (centity_t * ent, vec3_t origin);
void CL_BfgParticles (entity_t * ent);
void CL_AddParticles (void);
void CL_EntityEvent (entity_state_t * ent);
// RAFAEL
void CL_TrapParticles (entity_t * ent);


//
// menus
//
void M_Init (void);
void M_Keydown (int key);
void M_Draw (void);
void M_Menu_Main_f (void);
void M_ForceMenuOff (void);
void M_AddToServerList (netadr_t adr, char *info);

//
// cl_inv.c
//
void CL_ParseInventory (void);
void CL_DrawInventory (void);

//
// cl_pred.c
//
void CL_PredictMovement (void);

extern image_t *i_conback;
extern image_t *i_inventory;
extern image_t *i_net;
extern image_t *i_pause;
extern image_t *i_loading;
extern image_t *i_backtile;
extern image_t *i_turtle;


extern image_t *im_main_menu[5];
void Q_snprintfz (char *dst, int dstSize, const char *fmt, ...);
