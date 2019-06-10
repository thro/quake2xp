/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

/*
* Copyright(C) 1997 - 2001 Id Software, Inc.
* Copyright(C) 2011 Knightmare
* Copyright(C) 2011 Yamagi Burmeister
* Copyright(C) 2018 Quake2xp Team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or (at
	* your option) any later version.
	*
	* This program is distributed in the hope that it will be useful, but
	* WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	*
	* See the GNU General Public License for more details.
	*
	* You should have received a copy of the GNU General Public License
	* along with this program; if not, write to the Free Software
	* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	* 02111 - 1307, USA.
	*
	* ====================================================================== =
	*
	* The savegame system.
	*
	* ====================================================================== =
	*/

	/*
	* This is the Quake 2 savegame system, fixed by Yamagi
	* based on an idea by Knightmare of kmquake2. This major
	* rewrite of the original g_save.c is much more robust
	* and portable since it doesn't use any function pointers.
	*
	* Inner workings:
	* When the game is saved all function pointers are
	* translated into human readable function definition strings.
	* The same way all mmove_t pointers are translated. This
	* human readable strings are then written into the file.
	* At game load the human readable strings are retranslated
	* into the actual function pointers and struct pointers. The
	* pointers are generated at each compilation / start of the
	* client, thus the pointers are always correct.
	*
	* Limitations:
	* While savegames survive recompilations of the game source
	* and bigger changes in the source, there are some limitation
	* which a nearly impossible to fix without a object orientated
	* rewrite of the game.
	*  - If functions or mmove_t structs that a referencenced
	*    inside savegames are added or removed (e.g. the files
	*    in tables/ are altered) the load functions cannot
	*    reconnect all pointers and thus not restore the game.
	*  - If the operating system is changed internal structures
	*    may change in an unrepairable way.
	*  - If the architecture is changed pointer length and
	*    other internal datastructures change in an
	*    incompatible way.
	*  - If the edict_t struct is changed, savegames
	*    will break.
	* This is not so bad as it looks since functions and
	* struct won't be added and edict_t won't be changed
	* if no big, sweeping changes are done. The operating
	* system and architecture are in the hands of the user.
	*/

#include "g_local.h"
#include "g_save.h"

/*
* List with function pointer
* to each of the functions
* prototyped above.
*/
functionList_t functionList[] = { 
{ "Com_DefaultExtension", (byte *)Com_DefaultExtension },
{ "Com_DefaultPath", (byte *)Com_DefaultPath },
{ "Q_snprintfz", (byte *)Q_snprintfz },
{ "Q_strncatz", (byte *)Q_strncatz },
{ "Q_memcpy", (byte *)Q_memcpy },
{ "Q_free", (byte *)Q_free },
{ "Q_malloc", (byte *)Q_malloc },
{ "Q_strncpyz", (byte *)Q_strncpyz },
{ "Com_SkipWhiteSpace", (byte *)Com_SkipWhiteSpace },
{ "Com_ParseExt", (byte *)Com_ParseExt },
{ "Q_strcat", (byte *)Q_strcat },
{ "Q_strnicmp", (byte *)Q_strnicmp },
{ "Com_SkipRestOfLine", (byte *)Com_SkipRestOfLine },
{ "Info_SetValueForKey", (byte *)Info_SetValueForKey },
{ "Info_Validate", (byte *)Info_Validate },
{ "Info_RemoveKey", (byte *)Info_RemoveKey },
{ "Info_ValueForKey", (byte *)Info_ValueForKey },
{ "Com_sprintf", (byte *)Com_sprintf },
{ "Com_PageInMemory", (byte *)Com_PageInMemory },
{ "Com_Parse", (byte *)Com_Parse },
{ "va", (byte *)va },
{ "Swap_Init", (byte *)Swap_Init },
{ "FloatNoSwap", (byte *)FloatNoSwap },
{ "FloatSwap", (byte *)FloatSwap },
{ "LongNoSwap", (byte *)LongNoSwap },
{ "LongSwap", (byte *)LongSwap },
{ "ShortNoSwap", (byte *)ShortNoSwap },
{ "ShortSwap", (byte *)ShortSwap },
{ "LittleFloat", (byte *)LittleFloat },
{ "BigFloat", (byte *)BigFloat },
{ "LittleLong", (byte *)LittleLong },
{ "BigLong", (byte *)BigLong },
{ "LittleShort", (byte *)LittleShort },
{ "BigShort", (byte *)BigShort },
{ "COM_DefaultExtension", (byte *)COM_DefaultExtension },
{ "COM_FilePath", (byte *)COM_FilePath },
{ "COM_FileBase", (byte *)COM_FileBase },
{ "COM_FileExtension", (byte *)COM_FileExtension },
{ "COM_StripExtension", (byte *)COM_StripExtension },
{ "COM_SkipPath", (byte *)COM_SkipPath },
{ "Q_log2", (byte *)Q_log2 },
{ "VectorScale", (byte *)VectorScale },
{ "VectorInverse", (byte *)VectorInverse },
{ "VectorLength", (byte *)VectorLength },
{ "VectorNormalizeFast", (byte *)VectorNormalizeFast },
{ "Q_rsqrt", (byte *)Q_rsqrt },
{ "CrossProduct", (byte *)CrossProduct },
{ "_VectorCopy", (byte *)_VectorCopy },
{ "_VectorAdd", (byte *)_VectorAdd },
{ "_VectorSubtract", (byte *)_VectorSubtract },
{ "_DotProduct", (byte *)_DotProduct },
{ "VectorMA", (byte *)VectorMA },
{ "VectorNormalize2", (byte *)VectorNormalize2 },
{ "VectorNormalize", (byte *)VectorNormalize },
{ "VectorCompare", (byte *)VectorCompare },
{ "BoundsAdd", (byte *)BoundsAdd },
{ "AddVec4ToBounds", (byte *)AddVec4ToBounds },
{ "AddPointToBounds", (byte *)AddPointToBounds },
{ "ClearBounds", (byte *)ClearBounds },
{ "BoxOnPlaneSide", (byte *)BoxOnPlaneSide },
{ "BoxOnPlaneSide2", (byte *)BoxOnPlaneSide2 },
{ "anglemod", (byte *)anglemod },
{ "LerpAngle", (byte *)LerpAngle },
{ "Q_fabs", (byte *)Q_fabs },
{ "R_ConcatTransforms", (byte *)R_ConcatTransforms },
{ "R_ConcatRotations", (byte *)R_ConcatRotations },
{ "PerpendicularVector", (byte *)PerpendicularVector },
{ "ProjectPointOnPlane", (byte *)ProjectPointOnPlane },
{ "AngleVectors", (byte *)AngleVectors },
{ "RotatePointAroundVector", (byte *)RotatePointAroundVector },
{ "Weapon_BFG", (byte *)Weapon_BFG },
{ "weapon_bfg_fire", (byte *)weapon_bfg_fire },
{ "Weapon_Railgun", (byte *)Weapon_Railgun },
{ "weapon_railgun_fire", (byte *)weapon_railgun_fire },
{ "Weapon_SuperShotgun", (byte *)Weapon_SuperShotgun },
{ "weapon_supershotgun_fire", (byte *)weapon_supershotgun_fire },
{ "Weapon_Shotgun", (byte *)Weapon_Shotgun },
{ "weapon_shotgun_fire", (byte *)weapon_shotgun_fire },
{ "Weapon_Chaingun", (byte *)Weapon_Chaingun },
{ "Chaingun_Fire", (byte *)Chaingun_Fire },
{ "Weapon_Machinegun", (byte *)Weapon_Machinegun },
{ "Machinegun_Fire", (byte *)Machinegun_Fire },
{ "Extr_Shell", (byte *)Extr_Shell },
{ "Extr_Shell_Touch", (byte *)Extr_Shell_Touch },
{ "Weapon_HyperBlaster", (byte *)Weapon_HyperBlaster },
{ "Weapon_HyperBlaster_Fire", (byte *)Weapon_HyperBlaster_Fire },
{ "Weapon_Blaster", (byte *)Weapon_Blaster },
{ "Weapon_Blaster_Fire", (byte *)Weapon_Blaster_Fire },
{ "Blaster_Fire", (byte *)Blaster_Fire },
{ "Weapon_RocketLauncher", (byte *)Weapon_RocketLauncher },
{ "Weapon_RocketLauncher_Fire", (byte *)Weapon_RocketLauncher_Fire },
{ "Weapon_GrenadeLauncher", (byte *)Weapon_GrenadeLauncher },
{ "weapon_grenadelauncher_fire", (byte *)weapon_grenadelauncher_fire },
{ "Weapon_Grenade", (byte *)Weapon_Grenade },
{ "weapon_grenade_fire", (byte *)weapon_grenade_fire },
{ "Weapon_Generic", (byte *)Weapon_Generic },
{ "Drop_Weapon", (byte *)Drop_Weapon },
{ "Use_Weapon", (byte *)Use_Weapon },
{ "Think_Weapon", (byte *)Think_Weapon },
{ "NoAmmoWeaponChange", (byte *)NoAmmoWeaponChange },
{ "ChangeWeapon", (byte *)ChangeWeapon },
{ "Pickup_Weapon", (byte *)Pickup_Weapon },
{ "PlayerNoise", (byte *)PlayerNoise },
{ "P_ProjectSource", (byte *)P_ProjectSource },
{ "ClientEndServerFrame", (byte *)ClientEndServerFrame },
{ "G_SetClientFrame", (byte *)G_SetClientFrame },
{ "G_SetClientSound", (byte *)G_SetClientSound },
{ "G_SetClientEvent", (byte *)G_SetClientEvent },
{ "G_SetClientEffects", (byte *)G_SetClientEffects },
{ "P_WorldEffects", (byte *)P_WorldEffects },
{ "P_FallingDamage", (byte *)P_FallingDamage },
{ "SV_CalcBlend", (byte *)SV_CalcBlend },
{ "SV_AddBlend", (byte *)SV_AddBlend },
{ "SV_CalcGunOffset", (byte *)SV_CalcGunOffset },
{ "SV_CalcViewOffset", (byte *)SV_CalcViewOffset },
{ "P_DamageFeedback", (byte *)P_DamageFeedback },
{ "ClientAutoGenHealth", (byte *)ClientAutoGenHealth },
{ "SV_CalcRoll", (byte *)SV_CalcRoll },
{ "PlayerTrail_LastSpot", (byte *)PlayerTrail_LastSpot },
{ "PlayerTrail_PickNext", (byte *)PlayerTrail_PickNext },
{ "PlayerTrail_PickFirst", (byte *)PlayerTrail_PickFirst },
{ "PlayerTrail_New", (byte *)PlayerTrail_New },
{ "PlayerTrail_Add", (byte *)PlayerTrail_Add },
{ "PlayerTrail_Init", (byte *)PlayerTrail_Init },
{ "G_SetSpectatorStats", (byte *)G_SetSpectatorStats },
{ "G_CheckChaseStats", (byte *)G_CheckChaseStats },
{ "G_SetStats", (byte *)G_SetStats },
{ "Cmd_Help_f", (byte *)Cmd_Help_f },
{ "HelpComputer", (byte *)HelpComputer },
{ "Cmd_Score_f", (byte *)Cmd_Score_f },
{ "DeathmatchScoreboard", (byte *)DeathmatchScoreboard },
{ "DeathmatchScoreboardMessage", (byte *)DeathmatchScoreboardMessage },
{ "BeginIntermission", (byte *)BeginIntermission },
{ "MoveClientToIntermission", (byte *)MoveClientToIntermission },
{ "FL_make", (byte *)FL_make },
{ "FL_think", (byte *)FL_think },
{ "ClientBeginServerFrame", (byte *)ClientBeginServerFrame },
{ "ClientThink", (byte *)ClientThink },
{ "PrintPmove", (byte *)PrintPmove },
{ "CheckBlock", (byte *)CheckBlock },
{ "PM_trace", (byte *)PM_trace },
{ "ClientDisconnect", (byte *)ClientDisconnect },
{ "ClientConnect", (byte *)ClientConnect },
{ "ClientUserinfoChanged", (byte *)ClientUserinfoChanged },
{ "ClientBegin", (byte *)ClientBegin },
{ "ClientBeginDeathmatch", (byte *)ClientBeginDeathmatch },
{ "PutClientInServer", (byte *)PutClientInServer },
{ "spectator_respawn", (byte *)spectator_respawn },
{ "respawn", (byte *)respawn },
{ "CopyToBodyQue", (byte *)CopyToBodyQue },
{ "body_die", (byte *)body_die },
{ "InitBodyQue", (byte *)InitBodyQue },
{ "SelectSpawnPoint", (byte *)SelectSpawnPoint },
{ "SelectCoopSpawnPoint", (byte *)SelectCoopSpawnPoint },
{ "SelectDeathmatchSpawnPoint", (byte *)SelectDeathmatchSpawnPoint },
{ "SelectFarthestDeathmatchSpawnPoint", (byte *)SelectFarthestDeathmatchSpawnPoint },
{ "SelectRandomDeathmatchSpawnPoint", (byte *)SelectRandomDeathmatchSpawnPoint },
{ "PlayersRangeFromSpot", (byte *)PlayersRangeFromSpot },
{ "FetchClientEntData", (byte *)FetchClientEntData },
{ "SaveClientData", (byte *)SaveClientData },
{ "InitClientResp", (byte *)InitClientResp },
{ "InitClientPersistant", (byte *)InitClientPersistant },
{ "player_die", (byte *)player_die },
{ "LookAtKiller", (byte *)LookAtKiller },
{ "TossClientWeapon", (byte *)TossClientWeapon },
{ "ClientObituary", (byte *)ClientObituary },
{ "IsNeutral", (byte *)IsNeutral },
{ "IsFemale", (byte *)IsFemale },
{ "player_pain", (byte *)player_pain },
{ "SP_info_player_intermission", (byte *)SP_info_player_intermission },
{ "SP_info_player_coop", (byte *)SP_info_player_coop },
{ "SP_info_player_deathmatch", (byte *)SP_info_player_deathmatch },
{ "SP_info_player_start", (byte *)SP_info_player_start },
{ "SP_monster_tank", (byte *)SP_monster_tank },
{ "tank_die", (byte *)tank_die },
{ "tank_dead", (byte *)tank_dead },
{ "tank_attack", (byte *)tank_attack },
{ "tank_doattack_rocket", (byte *)tank_doattack_rocket },
{ "tank_refire_rocket", (byte *)tank_refire_rocket },
{ "tank_poststrike", (byte *)tank_poststrike },
{ "tank_reattack_blaster", (byte *)tank_reattack_blaster },
{ "TankMachineGun", (byte *)TankMachineGun },
{ "TankRocket", (byte *)TankRocket },
{ "TankStrike", (byte *)TankStrike },
{ "TankBlaster", (byte *)TankBlaster },
{ "tank_pain", (byte *)tank_pain },
{ "tank_run", (byte *)tank_run },
{ "tank_walk", (byte *)tank_walk },
{ "tank_stand", (byte *)tank_stand },
{ "tank_idle", (byte *)tank_idle },
{ "tank_windup", (byte *)tank_windup },
{ "tank_thud", (byte *)tank_thud },
{ "tank_footstep", (byte *)tank_footstep },
{ "tank_sight", (byte *)tank_sight },
{ "SP_monster_supertank", (byte *)SP_monster_supertank },
{ "supertank_die", (byte *)supertank_die },
{ "BossExplode", (byte *)BossExplode },
{ "supertank_dead", (byte *)supertank_dead },
{ "supertank_attack", (byte *)supertank_attack },
{ "supertankMachineGun", (byte *)supertankMachineGun },
{ "supertankRocket", (byte *)supertankRocket },
{ "supertank_pain", (byte *)supertank_pain },
{ "supertank_reattack1", (byte *)supertank_reattack1 },
{ "supertank_run", (byte *)supertank_run },
{ "supertank_walk", (byte *)supertank_walk },
{ "supertank_forward", (byte *)supertank_forward },
{ "supertank_stand", (byte *)supertank_stand },
{ "supertank_search", (byte *)supertank_search },
{ "TreadSound", (byte *)TreadSound },
{ "SP_monster_soldier_ss", (byte *)SP_monster_soldier_ss },
{ "SP_monster_soldier", (byte *)SP_monster_soldier },
{ "SP_monster_soldier_light", (byte *)SP_monster_soldier_light },
{ "SP_monster_soldier_x", (byte *)SP_monster_soldier_x },
{ "soldier_die", (byte *)soldier_die },
{ "soldier_dead", (byte *)soldier_dead },
{ "soldier_fire7", (byte *)soldier_fire7 },
{ "soldier_fire6", (byte *)soldier_fire6 },
{ "soldier_dodge", (byte *)soldier_dodge },
{ "soldier_duck_hold", (byte *)soldier_duck_hold },
{ "soldier_sight", (byte *)soldier_sight },
{ "soldier_attack", (byte *)soldier_attack },
{ "soldier_attack6_refire", (byte *)soldier_attack6_refire },
{ "soldier_fire8", (byte *)soldier_fire8 },
{ "soldier_fire4", (byte *)soldier_fire4 },
{ "soldier_attack3_refire", (byte *)soldier_attack3_refire },
{ "soldier_fire3", (byte *)soldier_fire3 },
{ "soldier_duck_up", (byte *)soldier_duck_up },
{ "soldier_duck_down", (byte *)soldier_duck_down },
{ "soldier_attack2_refire2", (byte *)soldier_attack2_refire2 },
{ "soldier_attack2_refire1", (byte *)soldier_attack2_refire1 },
{ "soldier_fire2", (byte *)soldier_fire2 },
{ "soldier_attack1_refire2", (byte *)soldier_attack1_refire2 },
{ "soldier_attack1_refire1", (byte *)soldier_attack1_refire1 },
{ "soldier_fire1", (byte *)soldier_fire1 },
{ "soldier_fire", (byte *)soldier_fire },
{ "soldier_pain", (byte *)soldier_pain },
{ "soldier_run", (byte *)soldier_run },
{ "soldier_walk", (byte *)soldier_walk },
{ "soldier_walk1_random", (byte *)soldier_walk1_random },
{ "soldier_stand", (byte *)soldier_stand },
{ "soldier_cock", (byte *)soldier_cock },
{ "soldier_idle", (byte *)soldier_idle },
{ "soldier_step", (byte *)soldier_step },
{ "SP_monster_parasite", (byte *)SP_monster_parasite },
{ "parasite_die", (byte *)parasite_die },
{ "parasite_dead", (byte *)parasite_dead },
{ "parasite_attack", (byte *)parasite_attack },
{ "parasite_drain_attack", (byte *)parasite_drain_attack },
{ "parasite_pain", (byte *)parasite_pain },
{ "parasite_walk", (byte *)parasite_walk },
{ "parasite_start_walk", (byte *)parasite_start_walk },
{ "parasite_run", (byte *)parasite_run },
{ "parasite_start_run", (byte *)parasite_start_run },
{ "parasite_stand", (byte *)parasite_stand },
{ "parasite_idle", (byte *)parasite_idle },
{ "parasite_refidget", (byte *)parasite_refidget },
{ "parasite_do_fidget", (byte *)parasite_do_fidget },
{ "parasite_end_fidget", (byte *)parasite_end_fidget },
{ "parasite_search", (byte *)parasite_search },
{ "parasite_scratch", (byte *)parasite_scratch },
{ "parasite_tap", (byte *)parasite_tap },
{ "parasite_sight", (byte *)parasite_sight },
{ "parasite_reel_in", (byte *)parasite_reel_in },
{ "parasite_launch", (byte *)parasite_launch },
{ "SP_monster_mutant", (byte *)SP_monster_mutant },
{ "mutant_die", (byte *)mutant_die },
{ "mutant_dead", (byte *)mutant_dead },
{ "mutant_pain", (byte *)mutant_pain },
{ "mutant_checkattack", (byte *)mutant_checkattack },
{ "mutant_check_jump", (byte *)mutant_check_jump },
{ "mutant_check_melee", (byte *)mutant_check_melee },
{ "mutant_jump", (byte *)mutant_jump },
{ "mutant_check_landing", (byte *)mutant_check_landing },
{ "mutant_jump_takeoff", (byte *)mutant_jump_takeoff },
{ "mutant_jump_touch", (byte *)mutant_jump_touch },
{ "mutant_melee", (byte *)mutant_melee },
{ "mutant_check_refire", (byte *)mutant_check_refire },
{ "mutant_hit_right", (byte *)mutant_hit_right },
{ "mutant_hit_left", (byte *)mutant_hit_left },
{ "mutant_run", (byte *)mutant_run },
{ "mutant_walk", (byte *)mutant_walk },
{ "mutant_walk_loop", (byte *)mutant_walk_loop },
{ "mutant_idle", (byte *)mutant_idle },
{ "mutant_idle_loop", (byte *)mutant_idle_loop },
{ "mutant_stand", (byte *)mutant_stand },
{ "mutant_swing", (byte *)mutant_swing },
{ "mutant_search", (byte *)mutant_search },
{ "mutant_sight", (byte *)mutant_sight },
{ "mutant_step", (byte *)mutant_step },
{ "M_walkmove", (byte *)M_walkmove },
{ "M_MoveToGoal", (byte *)M_MoveToGoal },
{ "SV_CloseEnough", (byte *)SV_CloseEnough },
{ "SV_NewChaseDir", (byte *)SV_NewChaseDir },
{ "SV_FixCheckBottom", (byte *)SV_FixCheckBottom },
{ "SV_StepDirection", (byte *)SV_StepDirection },
{ "M_ChangeYaw", (byte *)M_ChangeYaw },
{ "SV_movestep", (byte *)SV_movestep },
{ "M_CheckBottom", (byte *)M_CheckBottom },
{ "SP_monster_medic", (byte *)SP_monster_medic },
{ "medic_checkattack", (byte *)medic_checkattack },
{ "medic_attack", (byte *)medic_attack },
{ "medic_hook_retract", (byte *)medic_hook_retract },
{ "medic_cable_attack", (byte *)medic_cable_attack },
{ "medic_hook_launch", (byte *)medic_hook_launch },
{ "medic_continue", (byte *)medic_continue },
{ "medic_dodge", (byte *)medic_dodge },
{ "medic_duck_up", (byte *)medic_duck_up },
{ "medic_duck_hold", (byte *)medic_duck_hold },
{ "medic_duck_down", (byte *)medic_duck_down },
{ "medic_die", (byte *)medic_die },
{ "medic_dead", (byte *)medic_dead },
{ "medic_fire_blaster", (byte *)medic_fire_blaster },
{ "medic_pain", (byte *)medic_pain },
{ "medic_run", (byte *)medic_run },
{ "medic_walk", (byte *)medic_walk },
{ "medic_stand", (byte *)medic_stand },
{ "medic_sight", (byte *)medic_sight },
{ "medic_search", (byte *)medic_search },
{ "medic_idle", (byte *)medic_idle },
{ "medic_FindDeadMonster", (byte *)medic_FindDeadMonster },
{ "medic_step", (byte *)medic_step },
{ "SP_misc_insane", (byte *)SP_misc_insane },
{ "insane_die", (byte *)insane_die },
{ "insane_dead", (byte *)insane_dead },
{ "insane_stand", (byte *)insane_stand },
{ "insane_checkup", (byte *)insane_checkup },
{ "insane_checkdown", (byte *)insane_checkdown },
{ "insane_onground", (byte *)insane_onground },
{ "insane_pain", (byte *)insane_pain },
{ "insane_run", (byte *)insane_run },
{ "insane_walk", (byte *)insane_walk },
{ "insane_cross", (byte *)insane_cross },
{ "insane_scream", (byte *)insane_scream },
{ "insane_moan", (byte *)insane_moan },
{ "insane_shake", (byte *)insane_shake },
{ "insane_fist", (byte *)insane_fist },
{ "insane_step", (byte *)insane_step },
{ "SP_monster_infantry", (byte *)SP_monster_infantry },
{ "infantry_attack", (byte *)infantry_attack },
{ "infantry_smack", (byte *)infantry_smack },
{ "infantry_swing", (byte *)infantry_swing },
{ "infantry_fire", (byte *)infantry_fire },
{ "infantry_cock_gun", (byte *)infantry_cock_gun },
{ "infantry_dodge", (byte *)infantry_dodge },
{ "infantry_duck_up", (byte *)infantry_duck_up },
{ "infantry_duck_hold", (byte *)infantry_duck_hold },
{ "infantry_duck_down", (byte *)infantry_duck_down },
{ "infantry_die", (byte *)infantry_die },
{ "infantry_unhead", (byte *)infantry_unhead },
{ "infantry_dead", (byte *)infantry_dead },
{ "infantry_sight", (byte *)infantry_sight },
{ "InfantryMachineGun", (byte *)InfantryMachineGun },
{ "infantry_pain", (byte *)infantry_pain },
{ "infantry_run", (byte *)infantry_run },
{ "infantry_walk", (byte *)infantry_walk },
{ "infantry_fidget", (byte *)infantry_fidget },
{ "infantry_stand", (byte *)infantry_stand },
{ "infantry_step", (byte *)infantry_step },
{ "SP_monster_hover", (byte *)SP_monster_hover },
{ "hover_die", (byte *)hover_die },
{ "hover_dead", (byte *)hover_dead },
{ "hover_deadthink", (byte *)hover_deadthink },
{ "hover_pain", (byte *)hover_pain },
{ "hover_attack", (byte *)hover_attack },
{ "hover_start_attack", (byte *)hover_start_attack },
{ "hover_walk", (byte *)hover_walk },
{ "hover_run", (byte *)hover_run },
{ "hover_stand", (byte *)hover_stand },
{ "hover_fire_blaster", (byte *)hover_fire_blaster },
{ "hover_reattack", (byte *)hover_reattack },
{ "hover_search", (byte *)hover_search },
{ "hover_sight", (byte *)hover_sight },
{ "SP_monster_gunner", (byte *)SP_monster_gunner },
{ "gunner_refire_chain", (byte *)gunner_refire_chain },
{ "gunner_fire_chain", (byte *)gunner_fire_chain },
{ "gunner_attack", (byte *)gunner_attack },
{ "GunnerGrenade", (byte *)GunnerGrenade },
{ "GunnerFire", (byte *)GunnerFire },
{ "gunner_opengun", (byte *)gunner_opengun },
{ "gunner_dodge", (byte *)gunner_dodge },
{ "gunner_duck_up", (byte *)gunner_duck_up },
{ "gunner_duck_hold", (byte *)gunner_duck_hold },
{ "gunner_duck_down", (byte *)gunner_duck_down },
{ "gunner_die", (byte *)gunner_die },
{ "gunner_dead", (byte *)gunner_dead },
{ "gunner_pain", (byte *)gunner_pain },
{ "gunner_runandshoot", (byte *)gunner_runandshoot },
{ "gunner_run", (byte *)gunner_run },
{ "gunner_walk", (byte *)gunner_walk },
{ "gunner_stand", (byte *)gunner_stand },
{ "gunner_fidget", (byte *)gunner_fidget },
{ "gunner_search", (byte *)gunner_search },
{ "gunner_sight", (byte *)gunner_sight },
{ "gunner_idlesound", (byte *)gunner_idlesound },
{ "gunner_step", (byte *)gunner_step },
{ "SP_monster_gladiator", (byte *)SP_monster_gladiator },
{ "gladiator_die", (byte *)gladiator_die },
{ "gladiator_dead", (byte *)gladiator_dead },
{ "gladiator_pain", (byte *)gladiator_pain },
{ "gladiator_attack", (byte *)gladiator_attack },
{ "GladiatorGun", (byte *)GladiatorGun },
{ "gladiator_melee", (byte *)gladiator_melee },
{ "GaldiatorMelee", (byte *)GaldiatorMelee },
{ "gladiator_run", (byte *)gladiator_run },
{ "gladiator_walk", (byte *)gladiator_walk },
{ "gladiator_stand", (byte *)gladiator_stand },
{ "gladiator_cleaver_swing", (byte *)gladiator_cleaver_swing },
{ "gladiator_search", (byte *)gladiator_search },
{ "gladiator_sight", (byte *)gladiator_sight },
{ "gladiator_idle", (byte *)gladiator_idle },
{ "gladiator_step", (byte *)gladiator_step },
{ "SP_monster_flyer", (byte *)SP_monster_flyer },
{ "flyer_die", (byte *)flyer_die },
{ "flyer_pain", (byte *)flyer_pain },
{ "flyer_check_melee", (byte *)flyer_check_melee },
{ "flyer_melee", (byte *)flyer_melee },
{ "flyer_nextmove", (byte *)flyer_nextmove },
{ "flyer_setstart", (byte *)flyer_setstart },
{ "flyer_attack", (byte *)flyer_attack },
{ "flyer_loop_melee", (byte *)flyer_loop_melee },
{ "flyer_slash_right", (byte *)flyer_slash_right },
{ "flyer_slash_left", (byte *)flyer_slash_left },
{ "flyer_fireright", (byte *)flyer_fireright },
{ "flyer_fireleft", (byte *)flyer_fireleft },
{ "flyer_fire", (byte *)flyer_fire },
{ "flyer_start", (byte *)flyer_start },
{ "flyer_stop", (byte *)flyer_stop },
{ "flyer_stand", (byte *)flyer_stand },
{ "flyer_walk", (byte *)flyer_walk },
{ "flyer_run", (byte *)flyer_run },
{ "flyer_pop_blades", (byte *)flyer_pop_blades },
{ "flyer_idle", (byte *)flyer_idle },
{ "flyer_sight", (byte *)flyer_sight },
{ "SP_monster_floater", (byte *)SP_monster_floater },
{ "floater_die", (byte *)floater_die },
{ "floater_dead", (byte *)floater_dead },
{ "floater_pain", (byte *)floater_pain },
{ "floater_melee", (byte *)floater_melee },
{ "floater_attack", (byte *)floater_attack },
{ "floater_zap", (byte *)floater_zap },
{ "floater_wham", (byte *)floater_wham },
{ "floater_walk", (byte *)floater_walk },
{ "floater_run", (byte *)floater_run },
{ "floater_stand", (byte *)floater_stand },
{ "floater_fire_blaster", (byte *)floater_fire_blaster },
{ "floater_idle", (byte *)floater_idle },
{ "floater_sight", (byte *)floater_sight },
{ "SP_monster_flipper", (byte *)SP_monster_flipper },
{ "flipper_die", (byte *)flipper_die },
{ "flipper_sight", (byte *)flipper_sight },
{ "flipper_dead", (byte *)flipper_dead },
{ "flipper_pain", (byte *)flipper_pain },
{ "flipper_melee", (byte *)flipper_melee },
{ "flipper_preattack", (byte *)flipper_preattack },
{ "flipper_bite", (byte *)flipper_bite },
{ "flipper_start_run", (byte *)flipper_start_run },
{ "flipper_walk", (byte *)flipper_walk },
{ "flipper_run", (byte *)flipper_run },
{ "flipper_run_loop", (byte *)flipper_run_loop },
{ "flipper_stand", (byte *)flipper_stand },
{ "SP_monster_chick", (byte *)SP_monster_chick },
{ "chick_sight", (byte *)chick_sight },
{ "chick_attack", (byte *)chick_attack },
{ "chick_melee", (byte *)chick_melee },
{ "chick_slash", (byte *)chick_slash },
{ "chick_reslash", (byte *)chick_reslash },
{ "chick_attack1", (byte *)chick_attack1 },
{ "chick_rerocket", (byte *)chick_rerocket },
{ "ChickReload", (byte *)ChickReload },
{ "Chick_PreAttack1", (byte *)Chick_PreAttack1 },
{ "ChickRocket", (byte *)ChickRocket },
{ "ChickSlash", (byte *)ChickSlash },
{ "chick_dodge", (byte *)chick_dodge },
{ "chick_duck_up", (byte *)chick_duck_up },
{ "chick_duck_hold", (byte *)chick_duck_hold },
{ "chick_duck_down", (byte *)chick_duck_down },
{ "chick_die", (byte *)chick_die },
{ "chick_dead", (byte *)chick_dead },
{ "chick_pain", (byte *)chick_pain },
{ "chick_run", (byte *)chick_run },
{ "chick_walk", (byte *)chick_walk },
{ "chick_stand", (byte *)chick_stand },
{ "chick_fidget", (byte *)chick_fidget },
{ "ChickMoan", (byte *)ChickMoan },
{ "chick_step", (byte *)chick_step },
{ "SP_monster_brain", (byte *)SP_monster_brain },
{ "brain_die", (byte *)brain_die },
{ "brain_dead", (byte *)brain_dead },
{ "brain_pain", (byte *)brain_pain },
{ "brain_run", (byte *)brain_run },
{ "brain_melee", (byte *)brain_melee },
{ "brain_chest_closed", (byte *)brain_chest_closed },
{ "brain_tentacle_attack", (byte *)brain_tentacle_attack },
{ "brain_chest_open", (byte *)brain_chest_open },
{ "brain_hit_left", (byte *)brain_hit_left },
{ "brain_swing_left", (byte *)brain_swing_left },
{ "brain_hit_right", (byte *)brain_hit_right },
{ "brain_swing_right", (byte *)brain_swing_right },
{ "brain_dodge", (byte *)brain_dodge },
{ "brain_duck_up", (byte *)brain_duck_up },
{ "brain_duck_hold", (byte *)brain_duck_hold },
{ "brain_duck_down", (byte *)brain_duck_down },
{ "brain_walk", (byte *)brain_walk },
{ "brain_idle", (byte *)brain_idle },
{ "brain_stand", (byte *)brain_stand },
{ "brain_search", (byte *)brain_search },
{ "brain_sight", (byte *)brain_sight },
{ "brain_step", (byte *)brain_step },
{ "MakronToss", (byte *)MakronToss },
{ "MakronSpawn", (byte *)MakronSpawn },
{ "SP_monster_makron", (byte *)SP_monster_makron },
{ "MakronPrecache", (byte *)MakronPrecache },
{ "Makron_CheckAttack", (byte *)Makron_CheckAttack },
{ "makron_die", (byte *)makron_die },
{ "makron_dead", (byte *)makron_dead },
{ "makron_torso", (byte *)makron_torso },
{ "makron_torso_think", (byte *)makron_torso_think },
{ "makron_attack", (byte *)makron_attack },
{ "makron_sight", (byte *)makron_sight },
{ "makron_pain", (byte *)makron_pain },
{ "MakronHyperblaster", (byte *)MakronHyperblaster },
{ "MakronRailgun", (byte *)MakronRailgun },
{ "MakronSaveloc", (byte *)MakronSaveloc },
{ "makronBFG", (byte *)makronBFG },
{ "makron_run", (byte *)makron_run },
{ "makron_walk", (byte *)makron_walk },
{ "makron_prerailgun", (byte *)makron_prerailgun },
{ "makron_brainsplorch", (byte *)makron_brainsplorch },
{ "makron_step_right", (byte *)makron_step_right },
{ "makron_step_left", (byte *)makron_step_left },
{ "makron_popup", (byte *)makron_popup },
{ "makron_hit", (byte *)makron_hit },
{ "makron_stand", (byte *)makron_stand },
{ "makron_taunt", (byte *)makron_taunt },
{ "SP_monster_jorg", (byte *)SP_monster_jorg },
{ "Jorg_CheckAttack", (byte *)Jorg_CheckAttack },
{ "jorg_die", (byte *)jorg_die },
{ "jorg_dead", (byte *)jorg_dead },
{ "jorg_attack", (byte *)jorg_attack },
{ "jorg_firebullet", (byte *)jorg_firebullet },
{ "jorg_firebullet_left", (byte *)jorg_firebullet_left },
{ "jorg_firebullet_right", (byte *)jorg_firebullet_right },
{ "jorgBFG", (byte *)jorgBFG },
{ "jorg_pain", (byte *)jorg_pain },
{ "jorg_attack1", (byte *)jorg_attack1 },
{ "jorg_reattack1", (byte *)jorg_reattack1 },
{ "jorg_run", (byte *)jorg_run },
{ "jorg_walk", (byte *)jorg_walk },
{ "jorg_stand", (byte *)jorg_stand },
{ "jorg_step_right", (byte *)jorg_step_right },
{ "jorg_step_left", (byte *)jorg_step_left },
{ "jorg_death_hit", (byte *)jorg_death_hit },
{ "jorg_idle", (byte *)jorg_idle },
{ "jorg_search", (byte *)jorg_search },
{ "SP_monster_boss3_stand", (byte *)SP_monster_boss3_stand },
{ "Think_Boss3Stand", (byte *)Think_Boss3Stand },
{ "Use_Boss3", (byte *)Use_Boss3 },
{ "SP_monster_boss2", (byte *)SP_monster_boss2 },
{ "Boss2_CheckAttack", (byte *)Boss2_CheckAttack },
{ "boss2_die", (byte *)boss2_die },
{ "boss2_dead", (byte *)boss2_dead },
{ "boss2_pain", (byte *)boss2_pain },
{ "boss2_reattack_mg", (byte *)boss2_reattack_mg },
{ "boss2_attack_mg", (byte *)boss2_attack_mg },
{ "boss2_attack", (byte *)boss2_attack },
{ "boss2_walk", (byte *)boss2_walk },
{ "boss2_run", (byte *)boss2_run },
{ "boss2_stand", (byte *)boss2_stand },
{ "Boss2MachineGun", (byte *)Boss2MachineGun },
{ "boss2_firebullet_left", (byte *)boss2_firebullet_left },
{ "boss2_firebullet_right", (byte *)boss2_firebullet_right },
{ "Boss2Rocket", (byte *)Boss2Rocket },
{ "boss2_search", (byte *)boss2_search },
{ "SP_monster_berserk", (byte *)SP_monster_berserk },
{ "berserk_die", (byte *)berserk_die },
{ "berserk_dead", (byte *)berserk_dead },
{ "berserk_pain", (byte *)berserk_pain },
{ "berserk_melee", (byte *)berserk_melee },
{ "berserk_strike", (byte *)berserk_strike },
{ "berserk_attack_club", (byte *)berserk_attack_club },
{ "berserk_swing", (byte *)berserk_swing },
{ "berserk_attack_spike", (byte *)berserk_attack_spike },
{ "berserk_run", (byte *)berserk_run },
{ "berserk_walk", (byte *)berserk_walk },
{ "berserk_fidget", (byte *)berserk_fidget },
{ "berserk_stand", (byte *)berserk_stand },
{ "berserk_search", (byte *)berserk_search },
{ "berserk_sight", (byte *)berserk_sight },
{ "berserk_step", (byte *)berserk_step },
{ "SP_target_actor", (byte *)SP_target_actor },
{ "target_actor_touch", (byte *)target_actor_touch },
{ "SP_misc_actor", (byte *)SP_misc_actor },
{ "actor_use", (byte *)actor_use },
{ "actor_attack", (byte *)actor_attack },
{ "actor_fire", (byte *)actor_fire },
{ "actor_die", (byte *)actor_die },
{ "actor_dead", (byte *)actor_dead },
{ "actorMachineGun", (byte *)actorMachineGun },
{ "actor_pain", (byte *)actor_pain },
{ "actor_run", (byte *)actor_run },
{ "actor_walk", (byte *)actor_walk },
{ "actor_stand", (byte *)actor_stand },
{ "fire_bfg", (byte *)fire_bfg },
{ "bfg_think", (byte *)bfg_think },
{ "bfg_touch", (byte *)bfg_touch },
{ "bfg_explode", (byte *)bfg_explode },
{ "fire_rail", (byte *)fire_rail },
{ "fire_rocket", (byte *)fire_rocket },
{ "rocket_touch", (byte *)rocket_touch },
{ "fire_grenade2", (byte *)fire_grenade2 },
{ "fire_grenade", (byte *)fire_grenade },
{ "Grenade_Touch", (byte *)Grenade_Touch },
{ "Grenade_Explode", (byte *)Grenade_Explode },
{ "fire_blaster", (byte *)fire_blaster },
{ "blaster_touch", (byte *)blaster_touch },
{ "fire_shotgun", (byte *)fire_shotgun },
{ "fire_bullet", (byte *)fire_bullet },
{ "fire_hit", (byte *)fire_hit },
{ "KillBox", (byte *)KillBox },
{ "G_TouchSolids", (byte *)G_TouchSolids },
{ "G_TouchTriggers", (byte *)G_TouchTriggers },
{ "G_FreeEdict", (byte *)G_FreeEdict },
{ "G_Spawn", (byte *)G_Spawn },
{ "G_InitEdict", (byte *)G_InitEdict },
{ "G_CopyString", (byte *)G_CopyString },
{ "vectoangles", (byte *)vectoangles },
{ "vectoyaw", (byte *)vectoyaw },
{ "G_SetMovedir", (byte *)G_SetMovedir },
{ "vtos", (byte *)vtos },
{ "tv", (byte *)tv },
{ "G_UseTargets", (byte *)G_UseTargets },
{ "Think_Delay", (byte *)Think_Delay },
{ "G_PickTarget", (byte *)G_PickTarget },
{ "findradius", (byte *)findradius },
{ "G_Find", (byte *)G_Find },
{ "G_ProjectSource", (byte *)G_ProjectSource },
{ "SP_turret_driver", (byte *)SP_turret_driver },
{ "turret_driver_link", (byte *)turret_driver_link },
{ "turret_driver_think", (byte *)turret_driver_think },
{ "turret_driver_die", (byte *)turret_driver_die },
{ "SP_turret_base", (byte *)SP_turret_base },
{ "SP_turret_breach", (byte *)SP_turret_breach },
{ "turret_breach_finish_init", (byte *)turret_breach_finish_init },
{ "turret_breach_think", (byte *)turret_breach_think },
{ "turret_breach_fire", (byte *)turret_breach_fire },
{ "turret_blocked", (byte *)turret_blocked },
{ "SnapToEights", (byte *)SnapToEights },
{ "AnglesNormalize", (byte *)AnglesNormalize },
{ "SP_trigger_monsterjump", (byte *)SP_trigger_monsterjump },
{ "trigger_monsterjump_touch", (byte *)trigger_monsterjump_touch },
{ "SP_trigger_gravity", (byte *)SP_trigger_gravity },
{ "trigger_gravity_touch", (byte *)trigger_gravity_touch },
{ "SP_trigger_hurt", (byte *)SP_trigger_hurt },
{ "hurt_touch", (byte *)hurt_touch },
{ "hurt_use", (byte *)hurt_use },
{ "SP_trigger_push", (byte *)SP_trigger_push },
{ "trigger_push_touch", (byte *)trigger_push_touch },
{ "SP_trigger_always", (byte *)SP_trigger_always },
{ "SP_trigger_counter", (byte *)SP_trigger_counter },
{ "trigger_counter_use", (byte *)trigger_counter_use },
{ "SP_trigger_key", (byte *)SP_trigger_key },
{ "trigger_key_use", (byte *)trigger_key_use },
{ "SP_trigger_relay", (byte *)SP_trigger_relay },
{ "trigger_relay_use", (byte *)trigger_relay_use },
{ "SP_trigger_once", (byte *)SP_trigger_once },
{ "SP_trigger_multiple", (byte *)SP_trigger_multiple },
{ "trigger_enable", (byte *)trigger_enable },
{ "Touch_Multi", (byte *)Touch_Multi },
{ "Use_Multi", (byte *)Use_Multi },
{ "multi_trigger", (byte *)multi_trigger },
{ "multi_wait", (byte *)multi_wait },
{ "InitTrigger", (byte *)InitTrigger },
{ "SP_target_earthquake", (byte *)SP_target_earthquake },
{ "target_earthquake_use", (byte *)target_earthquake_use },
{ "target_earthquake_think", (byte *)target_earthquake_think },
{ "SP_target_lightramp", (byte *)SP_target_lightramp },
{ "target_lightramp_use", (byte *)target_lightramp_use },
{ "target_lightramp_think", (byte *)target_lightramp_think },
{ "SP_target_laser", (byte *)SP_target_laser },
{ "target_laser_start", (byte *)target_laser_start },
{ "target_laser_use", (byte *)target_laser_use },
{ "target_laser_off", (byte *)target_laser_off },
{ "target_laser_on", (byte *)target_laser_on },
{ "target_laser_think", (byte *)target_laser_think },
{ "SP_target_crosslevel_target", (byte *)SP_target_crosslevel_target },
{ "target_crosslevel_target_think", (byte *)target_crosslevel_target_think },
{ "SP_target_crosslevel_trigger", (byte *)SP_target_crosslevel_trigger },
{ "trigger_crosslevel_trigger_use", (byte *)trigger_crosslevel_trigger_use },
{ "SP_target_blaster", (byte *)SP_target_blaster },
{ "use_target_blaster", (byte *)use_target_blaster },
{ "SP_target_spawner", (byte *)SP_target_spawner },
{ "use_target_spawner", (byte *)use_target_spawner },
{ "SP_target_splash", (byte *)SP_target_splash },
{ "use_target_splash", (byte *)use_target_splash },
{ "SP_target_changelevel", (byte *)SP_target_changelevel },
{ "use_target_changelevel", (byte *)use_target_changelevel },
{ "SP_target_explosion", (byte *)SP_target_explosion },
{ "use_target_explosion", (byte *)use_target_explosion },
{ "target_explosion_explode", (byte *)target_explosion_explode },
{ "SP_target_goal", (byte *)SP_target_goal },
{ "use_target_goal", (byte *)use_target_goal },
{ "SP_target_secret", (byte *)SP_target_secret },
{ "use_target_secret", (byte *)use_target_secret },
{ "SP_target_help", (byte *)SP_target_help },
{ "Use_Target_Help", (byte *)Use_Target_Help },
{ "SP_target_speaker", (byte *)SP_target_speaker },
{ "Use_Target_Speaker", (byte *)Use_Target_Speaker },
{ "SP_target_temp_entity", (byte *)SP_target_temp_entity },
{ "Use_Target_Tent", (byte *)Use_Target_Tent },
{ "ServerCommand", (byte *)ServerCommand },
{ "SVCmd_WriteIP_f", (byte *)SVCmd_WriteIP_f },
{ "SVCmd_ListIP_f", (byte *)SVCmd_ListIP_f },
{ "SVCmd_RemoveIP_f", (byte *)SVCmd_RemoveIP_f },
{ "SVCmd_AddIP_f", (byte *)SVCmd_AddIP_f },
{ "SV_FilterPacket", (byte *)SV_FilterPacket },
{ "Svcmd_Test_f", (byte *)Svcmd_Test_f },
{ "SP_worldspawn", (byte *)SP_worldspawn },
{ "LoadStatusbarProgram", (byte *)LoadStatusbarProgram },
{ "Com_sprintf2", (byte *)Com_sprintf2 },
{ "SpawnEntities", (byte *)SpawnEntities },
{ "G_FindTeams", (byte *)G_FindTeams },
{ "ED_ParseEdict", (byte *)ED_ParseEdict },
{ "ED_ParseField", (byte *)ED_ParseField },
{ "ED_NewString", (byte *)ED_NewString },
{ "ED_CallSpawn", (byte *)ED_CallSpawn },
{ "ReadLevel", (byte *)ReadLevel },
{ "WriteLevel", (byte *)WriteLevel },
{ "ReadLevelLocals", (byte *)ReadLevelLocals },
{ "ReadEdict", (byte *)ReadEdict },
{ "WriteLevelLocals", (byte *)WriteLevelLocals },
{ "WriteEdict", (byte *)WriteEdict },
{ "ReadGame", (byte *)ReadGame },
{ "WriteGame", (byte *)WriteGame },
{ "ReadClient", (byte *)ReadClient },
{ "WriteClient", (byte *)WriteClient },
{ "ReadField", (byte *)ReadField },
{ "WriteField2", (byte *)WriteField2 },
{ "WriteField1", (byte *)WriteField1 },
{ "InitGame", (byte *)InitGame },
{ "G_RunEntity", (byte *)G_RunEntity },
{ "SV_Physics_Step", (byte *)SV_Physics_Step },
{ "SV_AddRotationalFriction", (byte *)SV_AddRotationalFriction },
{ "SV_Physics_Toss", (byte *)SV_Physics_Toss },
{ "SV_Physics_Noclip", (byte *)SV_Physics_Noclip },
{ "SV_Physics_None", (byte *)SV_Physics_None },
{ "SV_Physics_Pusher", (byte *)SV_Physics_Pusher },
{ "SV_Push", (byte *)SV_Push },
{ "SV_PushEntity", (byte *)SV_PushEntity },
{ "SV_AddGravity", (byte *)SV_AddGravity },
{ "SV_FlyMove", (byte *)SV_FlyMove },
{ "ClipVelocity", (byte *)ClipVelocity },
{ "SV_Impact", (byte *)SV_Impact },
{ "SV_RunThink", (byte *)SV_RunThink },
{ "SV_CheckVelocity", (byte *)SV_CheckVelocity },
{ "SV_TestEntityPosition", (byte *)SV_TestEntityPosition },
{ "swimmonster_start", (byte *)swimmonster_start },
{ "swimmonster_start_go", (byte *)swimmonster_start_go },
{ "flymonster_start", (byte *)flymonster_start },
{ "flymonster_start_go", (byte *)flymonster_start_go },
{ "walkmonster_start", (byte *)walkmonster_start },
{ "walkmonster_start_go", (byte *)walkmonster_start_go },
{ "monster_start_go", (byte *)monster_start_go },
{ "monster_start", (byte *)monster_start },
{ "monster_death_use", (byte *)monster_death_use },
{ "monster_triggered_start", (byte *)monster_triggered_start },
{ "monster_triggered_spawn_use", (byte *)monster_triggered_spawn_use },
{ "monster_triggered_spawn", (byte *)monster_triggered_spawn },
{ "monster_use", (byte *)monster_use },
{ "monster_think", (byte *)monster_think },
{ "M_MoveFrame", (byte *)M_MoveFrame },
{ "M_SetEffects", (byte *)M_SetEffects },
{ "M_droptofloor", (byte *)M_droptofloor },
{ "M_WorldEffects", (byte *)M_WorldEffects },
{ "M_CatagorizePosition", (byte *)M_CatagorizePosition },
{ "M_CheckGround", (byte *)M_CheckGround },
{ "AttackFinished", (byte *)AttackFinished },
{ "Touch_Corpse", (byte *)Touch_Corpse },
{ "monster_reborn", (byte *)monster_reborn },
{ "monster_respawn", (byte *)monster_respawn },
{ "M_FlyCheck", (byte *)M_FlyCheck },
{ "M_FliesOn", (byte *)M_FliesOn },
{ "M_FliesOff", (byte *)M_FliesOff },
{ "monster_fire_bfg", (byte *)monster_fire_bfg },
{ "monster_fire_railgun", (byte *)monster_fire_railgun },
{ "monster_fire_rocket", (byte *)monster_fire_rocket },
{ "monster_fire_grenade", (byte *)monster_fire_grenade },
{ "monster_fire_blaster", (byte *)monster_fire_blaster },
{ "monster_fire_shotgun", (byte *)monster_fire_shotgun },
{ "monster_fire_bullet", (byte *)monster_fire_bullet },
{ "SP_misc_teleporter_dest", (byte *)SP_misc_teleporter_dest },
{ "SP_misc_teleporter", (byte *)SP_misc_teleporter },
{ "teleporter_touch", (byte *)teleporter_touch },
{ "SP_func_clock", (byte *)SP_func_clock },
{ "func_clock_use", (byte *)func_clock_use },
{ "func_clock_think", (byte *)func_clock_think },
{ "func_clock_format_countdown", (byte *)func_clock_format_countdown },
{ "SP_target_string", (byte *)SP_target_string },
{ "target_string_use", (byte *)target_string_use },
{ "SP_target_character", (byte *)SP_target_character },
{ "SP_misc_gib_head", (byte *)SP_misc_gib_head },
{ "SP_misc_gib_leg", (byte *)SP_misc_gib_leg },
{ "SP_misc_gib_arm", (byte *)SP_misc_gib_arm },
{ "SP_light_mine2", (byte *)SP_light_mine2 },
{ "SP_light_mine1", (byte *)SP_light_mine1 },
{ "SP_misc_satellite_dish", (byte *)SP_misc_satellite_dish },
{ "misc_satellite_dish_use", (byte *)misc_satellite_dish_use },
{ "misc_satellite_dish_think", (byte *)misc_satellite_dish_think },
{ "SP_misc_strogg_ship", (byte *)SP_misc_strogg_ship },
{ "misc_strogg_ship_use", (byte *)misc_strogg_ship_use },
{ "SP_misc_viper_bomb", (byte *)SP_misc_viper_bomb },
{ "misc_viper_bomb_use", (byte *)misc_viper_bomb_use },
{ "misc_viper_bomb_prethink", (byte *)misc_viper_bomb_prethink },
{ "misc_viper_bomb_touch", (byte *)misc_viper_bomb_touch },
{ "SP_misc_bigviper", (byte *)SP_misc_bigviper },
{ "SP_misc_viper", (byte *)SP_misc_viper },
{ "misc_viper_use", (byte *)misc_viper_use },
{ "SP_misc_deadsoldier", (byte *)SP_misc_deadsoldier },
{ "misc_deadsoldier_die", (byte *)misc_deadsoldier_die },
{ "SP_misc_banner", (byte *)SP_misc_banner },
{ "misc_banner_think", (byte *)misc_banner_think },
{ "SP_monster_commander_body", (byte *)SP_monster_commander_body },
{ "commander_body_drop", (byte *)commander_body_drop },
{ "commander_body_use", (byte *)commander_body_use },
{ "commander_body_think", (byte *)commander_body_think },
{ "SP_misc_easterchick2", (byte *)SP_misc_easterchick2 },
{ "misc_easterchick2_think", (byte *)misc_easterchick2_think },
{ "SP_misc_easterchick", (byte *)SP_misc_easterchick },
{ "misc_easterchick_think", (byte *)misc_easterchick_think },
{ "SP_misc_eastertank", (byte *)SP_misc_eastertank },
{ "misc_eastertank_think", (byte *)misc_eastertank_think },
{ "SP_misc_blackhole", (byte *)SP_misc_blackhole },
{ "misc_blackhole_think", (byte *)misc_blackhole_think },
{ "misc_blackhole_use", (byte *)misc_blackhole_use },
{ "SP_misc_explobox", (byte *)SP_misc_explobox },
{ "barrel_delay", (byte *)barrel_delay },
{ "barrel_explode", (byte *)barrel_explode },
{ "barrel_touch", (byte *)barrel_touch },
{ "SP_func_explosive", (byte *)SP_func_explosive },
{ "func_explosive_spawn", (byte *)func_explosive_spawn },
{ "func_explosive_use", (byte *)func_explosive_use },
{ "func_explosive_explode", (byte *)func_explosive_explode },
{ "SP_func_object", (byte *)SP_func_object },
{ "func_object_use", (byte *)func_object_use },
{ "func_object_release", (byte *)func_object_release },
{ "func_object_touch", (byte *)func_object_touch },
{ "SP_func_wall", (byte *)SP_func_wall },
{ "func_wall_use", (byte *)func_wall_use },
{ "SP_light", (byte *)SP_light },
{ "light_use", (byte *)light_use },
{ "SP_info_notnull", (byte *)SP_info_notnull },
{ "SP_info_null", (byte *)SP_info_null },
{ "SP_viewthing", (byte *)SP_viewthing },
{ "TH_viewthing", (byte *)TH_viewthing },
{ "SP_point_combat", (byte *)SP_point_combat },
{ "point_combat_touch", (byte *)point_combat_touch },
{ "SP_path_corner", (byte *)SP_path_corner },
{ "path_corner_touch", (byte *)path_corner_touch },
{ "BecomeExplosion2", (byte *)BecomeExplosion2 },
{ "BecomeExplosion1", (byte *)BecomeExplosion1 },
{ "ThrowDebris", (byte *)ThrowDebris },
{ "debris_die", (byte *)debris_die },
{ "NetThrowGibs", (byte *)NetThrowGibs },
{ "WriteScaledDir", (byte *)WriteScaledDir },
{ "ThrowClientHead", (byte *)ThrowClientHead },
{ "ThrowHead", (byte *)ThrowHead },
{ "ThrowGib", (byte *)ThrowGib },
{ "Spawn_Gib_Blood", (byte *)Spawn_Gib_Blood },
{ "gib_die", (byte *)gib_die },
{ "gib_touch", (byte *)gib_touch },
{ "gib_think", (byte *)gib_think },
{ "ClipGibVelocity", (byte *)ClipGibVelocity },
{ "VelocityForDamage", (byte *)VelocityForDamage },
{ "SP_func_areaportal", (byte *)SP_func_areaportal },
{ "Use_Areaportal", (byte *)Use_Areaportal },
{ "G_RunFrame", (byte *)G_RunFrame },
{ "ExitLevel", (byte *)ExitLevel },
{ "CheckDMRules", (byte *)CheckDMRules },
{ "CheckNeedPass", (byte *)CheckNeedPass },
{ "EndDMLevel", (byte *)EndDMLevel },
{ "CreateTargetChangeLevel", (byte *)CreateTargetChangeLevel },
{ "ClientEndServerFrames", (byte *)ClientEndServerFrames },
{ "Com_Printf", (byte *)Com_Printf },
{ "Sys_Error", (byte *)Sys_Error },
{ "GetGameAPI", (byte *)GetGameAPI },
{ "ShutdownGame", (byte *)ShutdownGame },
{ "SetItemNames", (byte *)SetItemNames },
{ "InitItems", (byte *)InitItems },
{ "SP_item_health_mega", (byte *)SP_item_health_mega },
{ "SP_item_health_large", (byte *)SP_item_health_large },
{ "SP_item_health_small", (byte *)SP_item_health_small },
{ "SP_item_health", (byte *)SP_item_health },
{ "Use_IR", (byte *)Use_IR },
{ "SpawnItem", (byte *)SpawnItem },
{ "PrecacheItem", (byte *)PrecacheItem },
{ "droptofloor", (byte *)droptofloor },
{ "Use_Item", (byte *)Use_Item },
{ "Drop_Item", (byte *)Drop_Item },
{ "drop_make_touchable", (byte *)drop_make_touchable },
{ "drop_temp_touch", (byte *)drop_temp_touch },
{ "Touch_Item", (byte *)Touch_Item },
{ "Drop_PowerArmor", (byte *)Drop_PowerArmor },
{ "Pickup_PowerArmor", (byte *)Pickup_PowerArmor },
{ "Use_PowerArmor", (byte *)Use_PowerArmor },
{ "PowerArmorType", (byte *)PowerArmorType },
{ "Pickup_Armor", (byte *)Pickup_Armor },
{ "ArmorIndex", (byte *)ArmorIndex },
{ "Pickup_Health", (byte *)Pickup_Health },
{ "MegaHealth_think", (byte *)MegaHealth_think },
{ "Drop_Ammo", (byte *)Drop_Ammo },
{ "Pickup_Ammo", (byte *)Pickup_Ammo },
{ "Add_Ammo", (byte *)Add_Ammo },
{ "Pickup_Key", (byte *)Pickup_Key },
{ "Use_Silencer", (byte *)Use_Silencer },
{ "Use_Invulnerability", (byte *)Use_Invulnerability },
{ "Use_Envirosuit", (byte *)Use_Envirosuit },
{ "Use_Breather", (byte *)Use_Breather },
{ "Use_Quad", (byte *)Use_Quad },
{ "Pickup_Pack", (byte *)Pickup_Pack },
{ "Pickup_Bandolier", (byte *)Pickup_Bandolier },
{ "Pickup_AncientHead", (byte *)Pickup_AncientHead },
{ "Pickup_Adrenaline", (byte *)Pickup_Adrenaline },
{ "Drop_General", (byte *)Drop_General },
{ "Pickup_Powerup", (byte *)Pickup_Powerup },
{ "SetRespawn", (byte *)SetRespawn },
{ "DoRespawn", (byte *)DoRespawn },
{ "FindItem", (byte *)FindItem },
{ "FindItemByClassname", (byte *)FindItemByClassname },
{ "GetItemByIndex", (byte *)GetItemByIndex },
{ "Coop_Respawn_Items", (byte *)Coop_Respawn_Items },
{ "SP_func_killbox", (byte *)SP_func_killbox },
{ "use_killbox", (byte *)use_killbox },
{ "SP_func_door_secret", (byte *)SP_func_door_secret },
{ "door_secret_die", (byte *)door_secret_die },
{ "door_secret_blocked", (byte *)door_secret_blocked },
{ "door_secret_done", (byte *)door_secret_done },
{ "door_secret_move6", (byte *)door_secret_move6 },
{ "door_secret_move5", (byte *)door_secret_move5 },
{ "door_secret_move4", (byte *)door_secret_move4 },
{ "door_secret_move3", (byte *)door_secret_move3 },
{ "door_secret_move2", (byte *)door_secret_move2 },
{ "door_secret_move1", (byte *)door_secret_move1 },
{ "door_secret_use", (byte *)door_secret_use },
{ "SP_func_conveyor", (byte *)SP_func_conveyor },
{ "func_conveyor_use", (byte *)func_conveyor_use },
{ "SP_func_timer", (byte *)SP_func_timer },
{ "func_timer_use", (byte *)func_timer_use },
{ "func_timer_think", (byte *)func_timer_think },
{ "SP_trigger_elevator", (byte *)SP_trigger_elevator },
{ "trigger_elevator_init", (byte *)trigger_elevator_init },
{ "trigger_elevator_use", (byte *)trigger_elevator_use },
{ "SP_func_train", (byte *)SP_func_train },
{ "train_use", (byte *)train_use },
{ "func_train_find", (byte *)func_train_find },
{ "train_resume", (byte *)train_resume },
{ "train_next", (byte *)train_next },
{ "train_wait", (byte *)train_wait },
{ "train_blocked", (byte *)train_blocked },
{ "SP_func_water", (byte *)SP_func_water },
{ "SP_func_door_rotating", (byte *)SP_func_door_rotating },
{ "SP_func_door", (byte *)SP_func_door },
{ "door_touch", (byte *)door_touch },
{ "door_killed", (byte *)door_killed },
{ "door_blocked", (byte *)door_blocked },
{ "Think_SpawnDoorTrigger", (byte *)Think_SpawnDoorTrigger },
{ "Think_CalcMoveSpeed", (byte *)Think_CalcMoveSpeed },
{ "Touch_DoorTrigger", (byte *)Touch_DoorTrigger },
{ "door_use", (byte *)door_use },
{ "door_go_up", (byte *)door_go_up },
{ "door_go_down", (byte *)door_go_down },
{ "door_hit_bottom", (byte *)door_hit_bottom },
{ "door_hit_top", (byte *)door_hit_top },
{ "door_use_areaportals", (byte *)door_use_areaportals },
{ "SP_func_button", (byte *)SP_func_button },
{ "button_killed", (byte *)button_killed },
{ "button_touch", (byte *)button_touch },
{ "button_use", (byte *)button_use },
{ "button_fire", (byte *)button_fire },
{ "button_wait", (byte *)button_wait },
{ "button_return", (byte *)button_return },
{ "button_done", (byte *)button_done },
{ "SP_func_rotating", (byte *)SP_func_rotating },
{ "rotating_use", (byte *)rotating_use },
{ "rotating_touch", (byte *)rotating_touch },
{ "rotating_blocked", (byte *)rotating_blocked },
{ "SP_func_plat", (byte *)SP_func_plat },
{ "plat_spawn_inside_trigger", (byte *)plat_spawn_inside_trigger },
{ "Touch_Plat_Center", (byte *)Touch_Plat_Center },
{ "Use_Plat", (byte *)Use_Plat },
{ "plat_blocked", (byte *)plat_blocked },
{ "plat_go_up", (byte *)plat_go_up },
{ "plat_go_down", (byte *)plat_go_down },
{ "plat_hit_bottom", (byte *)plat_hit_bottom },
{ "plat_hit_top", (byte *)plat_hit_top },
{ "Think_AccelMove", (byte *)Think_AccelMove },
{ "plat_Accelerate", (byte *)plat_Accelerate },
{ "plat_CalcAcceleratedMove", (byte *)plat_CalcAcceleratedMove },
{ "AngleMove_Calc", (byte *)AngleMove_Calc },
{ "AngleMove_Begin", (byte *)AngleMove_Begin },
{ "AngleMove_Final", (byte *)AngleMove_Final },
{ "AngleMove_Done", (byte *)AngleMove_Done },
{ "Move_Calc", (byte *)Move_Calc },
{ "Move_Begin", (byte *)Move_Begin },
{ "Move_Final", (byte *)Move_Final },
{ "Move_Done", (byte *)Move_Done },
{ "T_RadiusDamage", (byte *)T_RadiusDamage },
{ "T_Damage", (byte *)T_Damage },
{ "CheckTeamDamage", (byte *)CheckTeamDamage },
{ "M_ReactToDamage", (byte *)M_ReactToDamage },
{ "SpawnDamage", (byte *)SpawnDamage },
{ "Killed", (byte *)Killed },
{ "CanDamage", (byte *)CanDamage },
{ "ClientCommand", (byte *)ClientCommand },
{ "Cmd_PlayerList_f", (byte *)Cmd_PlayerList_f },
{ "Cmd_Say_f", (byte *)Cmd_Say_f },
{ "Cmd_Wave_f", (byte *)Cmd_Wave_f },
{ "Cmd_Players_f", (byte *)Cmd_Players_f },
{ "PlayerSort", (byte *)PlayerSort },
{ "Cmd_PutAway_f", (byte *)Cmd_PutAway_f },
{ "Cmd_Kill_f", (byte *)Cmd_Kill_f },
{ "Cmd_InvDrop_f", (byte *)Cmd_InvDrop_f },
{ "Cmd_WeapLast_f", (byte *)Cmd_WeapLast_f },
{ "Cmd_WeapNext_f", (byte *)Cmd_WeapNext_f },
{ "Cmd_WeapPrev_f", (byte *)Cmd_WeapPrev_f },
{ "Cmd_InvUse_f", (byte *)Cmd_InvUse_f },
{ "Cmd_Inven_f", (byte *)Cmd_Inven_f },
{ "Cmd_Drop_f", (byte *)Cmd_Drop_f },
{ "Cmd_Use_f", (byte *)Cmd_Use_f },
{ "Cmd_Noclip_f", (byte *)Cmd_Noclip_f },
{ "Cmd_Notarget_f", (byte *)Cmd_Notarget_f },
{ "Cmd_God_f", (byte *)Cmd_God_f },
{ "Cmd_Give_f", (byte *)Cmd_Give_f },
{ "ValidateSelectedItem", (byte *)ValidateSelectedItem },
{ "SelectPrevItem", (byte *)SelectPrevItem },
{ "SelectNextItem", (byte *)SelectNextItem },
{ "OnSameTeam", (byte *)OnSameTeam },
{ "ClientTeam", (byte *)ClientTeam },
{ "GetChaseTarget", (byte *)GetChaseTarget },
{ "ChasePrev", (byte *)ChasePrev },
{ "ChaseNext", (byte *)ChaseNext },
{ "UpdateChaseCam", (byte *)UpdateChaseCam },
{ "ai_run", (byte *)ai_run },
{ "ai_checkattack", (byte *)ai_checkattack },
{ "ai_run_slide", (byte *)ai_run_slide },
{ "ai_run_missile", (byte *)ai_run_missile },
{ "ai_run_melee", (byte *)ai_run_melee },
{ "M_CheckAttack", (byte *)M_CheckAttack },
{ "FacingIdeal", (byte *)FacingIdeal },
{ "FindTarget", (byte *)FindTarget },
{ "FoundTarget", (byte *)FoundTarget },
{ "HuntTarget", (byte *)HuntTarget },
{ "infront", (byte *)infront },
{ "visible", (byte *)visible },
{ "range", (byte *)range },
{ "ai_turn", (byte *)ai_turn },
{ "ai_charge", (byte *)ai_charge },
{ "ai_walk", (byte *)ai_walk },
{ "ai_stand", (byte *)ai_stand },
{ "ai_move", (byte *)ai_move },
{ "AI_SetSightClient", (byte *)AI_SetSightClient },
{ 0, 0 }

};

/*
* List with pointers to
* each of the mmove_t
* functions prototyped
* above.
*/
mmoveList_t mmoveList[] = {
	{ "tank_move_death", &tank_move_death },
{ "tank_move_attack_chain", &tank_move_attack_chain },
{ "tank_move_attack_post_rocket", &tank_move_attack_post_rocket },
{ "tank_move_attack_fire_rocket", &tank_move_attack_fire_rocket },
{ "tank_move_attack_pre_rocket", &tank_move_attack_pre_rocket },
{ "tank_move_attack_strike", &tank_move_attack_strike },
{ "tank_move_attack_post_blast", &tank_move_attack_post_blast },
{ "tank_move_reattack_blast", &tank_move_reattack_blast },
{ "tank_move_attack_blast", &tank_move_attack_blast },
{ "tank_move_pain3", &tank_move_pain3 },
{ "tank_move_pain2", &tank_move_pain2 },
{ "tank_move_pain1", &tank_move_pain1 },
{ "tank_move_stop_run", &tank_move_stop_run },
{ "tank_move_run", &tank_move_run },
{ "tank_move_start_run", &tank_move_start_run },
{ "tank_move_stop_walk", &tank_move_stop_walk },
{ "tank_move_walk", &tank_move_walk },
{ "tank_move_start_walk", &tank_move_start_walk },
{ "tank_move_stand", &tank_move_stand },
{ "supertank_move_end_attack1", &supertank_move_end_attack1 },
{ "supertank_move_attack1", &supertank_move_attack1 },
{ "supertank_move_attack2", &supertank_move_attack2 },
{ "supertank_move_attack3", &supertank_move_attack3 },
{ "supertank_move_attack4", &supertank_move_attack4 },
{ "supertank_move_backward", &supertank_move_backward },
{ "supertank_move_death", &supertank_move_death },
{ "supertank_move_pain1", &supertank_move_pain1 },
{ "supertank_move_pain2", &supertank_move_pain2 },
{ "supertank_move_pain3", &supertank_move_pain3 },
{ "supertank_move_turn_left", &supertank_move_turn_left },
{ "supertank_move_turn_right", &supertank_move_turn_right },
{ "supertank_move_forward", &supertank_move_forward },
{ "supertank_move_run", &supertank_move_run },
{ "supertank_move_stand", &supertank_move_stand },
{ "soldier_move_death6", &soldier_move_death6 },
{ "soldier_move_death5", &soldier_move_death5 },
{ "soldier_move_death4", &soldier_move_death4 },
{ "soldier_move_death3", &soldier_move_death3 },
{ "soldier_move_death2", &soldier_move_death2 },
{ "soldier_move_death1", &soldier_move_death1 },
{ "soldier_move_duck", &soldier_move_duck },
{ "soldier_move_attack6", &soldier_move_attack6 },
{ "soldier_move_attack4", &soldier_move_attack4 },
{ "soldier_move_attack3", &soldier_move_attack3 },
{ "soldier_move_attack2", &soldier_move_attack2 },
{ "soldier_move_attack1", &soldier_move_attack1 },
{ "soldier_move_pain4", &soldier_move_pain4 },
{ "soldier_move_pain3", &soldier_move_pain3 },
{ "soldier_move_pain2", &soldier_move_pain2 },
{ "soldier_move_pain1", &soldier_move_pain1 },
{ "soldier_move_run", &soldier_move_run },
{ "soldier_move_start_run", &soldier_move_start_run },
{ "soldier_move_walk2", &soldier_move_walk2 },
{ "soldier_move_walk1", &soldier_move_walk1 },
{ "soldier_move_stand3", &soldier_move_stand3 },
{ "soldier_move_stand1", &soldier_move_stand1 },
{ "parasite_move_death", &parasite_move_death },
{ "parasite_move_break", &parasite_move_break },
{ "parasite_move_drain", &parasite_move_drain },
{ "parasite_move_pain1", &parasite_move_pain1 },
{ "parasite_move_stop_walk", &parasite_move_stop_walk },
{ "parasite_move_start_walk", &parasite_move_start_walk },
{ "parasite_move_walk", &parasite_move_walk },
{ "parasite_move_stop_run", &parasite_move_stop_run },
{ "parasite_move_start_run", &parasite_move_start_run },
{ "parasite_move_run", &parasite_move_run },
{ "parasite_move_stand", &parasite_move_stand },
{ "parasite_move_end_fidget", &parasite_move_end_fidget },
{ "parasite_move_fidget", &parasite_move_fidget },
{ "parasite_move_start_fidget", &parasite_move_start_fidget },
{ "mutant_move_death2", &mutant_move_death2 },
{ "mutant_move_death1", &mutant_move_death1 },
{ "mutant_move_pain3", &mutant_move_pain3 },
{ "mutant_move_pain2", &mutant_move_pain2 },
{ "mutant_move_pain1", &mutant_move_pain1 },
{ "mutant_move_jump", &mutant_move_jump },
{ "mutant_move_attack", &mutant_move_attack },
{ "mutant_move_run", &mutant_move_run },
{ "mutant_move_start_walk", &mutant_move_start_walk },
{ "mutant_move_walk", &mutant_move_walk },
{ "mutant_move_idle", &mutant_move_idle },
{ "mutant_move_stand", &mutant_move_stand },
{ "medic_move_attackCable", &medic_move_attackCable },
{ "medic_move_attackBlaster", &medic_move_attackBlaster },
{ "medic_move_attackHyperBlaster", &medic_move_attackHyperBlaster },
{ "medic_move_duck", &medic_move_duck },
{ "medic_move_death", &medic_move_death },
{ "medic_move_pain2", &medic_move_pain2 },
{ "medic_move_pain1", &medic_move_pain1 },
{ "medic_move_run", &medic_move_run },
{ "medic_move_walk", &medic_move_walk },
{ "medic_move_stand", &medic_move_stand },
{ "insane_move_struggle_cross", &insane_move_struggle_cross },
{ "insane_move_cross", &insane_move_cross },
{ "insane_move_crawl_death", &insane_move_crawl_death },
{ "insane_move_crawl_pain", &insane_move_crawl_pain },
{ "insane_move_runcrawl", &insane_move_runcrawl },
{ "insane_move_crawl", &insane_move_crawl },
{ "insane_move_stand_death", &insane_move_stand_death },
{ "insane_move_stand_pain", &insane_move_stand_pain },
{ "insane_move_run_insane", &insane_move_run_insane },
{ "insane_move_walk_insane", &insane_move_walk_insane },
{ "insane_move_run_normal", &insane_move_run_normal },
{ "insane_move_walk_normal", &insane_move_walk_normal },
{ "insane_move_down", &insane_move_down },
{ "insane_move_jumpdown", &insane_move_jumpdown },
{ "insane_move_downtoup", &insane_move_downtoup },
{ "insane_move_uptodown", &insane_move_uptodown },
{ "insane_move_stand_insane", &insane_move_stand_insane },
{ "insane_move_stand_normal", &insane_move_stand_normal },
{ "infantry_move_attack2", &infantry_move_attack2 },
{ "infantry_move_attack1", &infantry_move_attack1 },
{ "infantry_move_duck", &infantry_move_duck },
{ "infantry_move_death3", &infantry_move_death3 },
{ "infantry_move_death2", &infantry_move_death2 },
{ "infantry_move_death1", &infantry_move_death1 },
{ "infantry_move_pain2", &infantry_move_pain2 },
{ "infantry_move_pain1", &infantry_move_pain1 },
{ "infantry_move_run", &infantry_move_run },
{ "infantry_move_walk", &infantry_move_walk },
{ "infantry_move_fidget", &infantry_move_fidget },
{ "infantry_move_stand", &infantry_move_stand },
{ "hover_move_end_attack", &hover_move_end_attack },
{ "hover_move_attack1", &hover_move_attack1 },
{ "hover_move_start_attack", &hover_move_start_attack },
{ "hover_move_backward", &hover_move_backward },
{ "hover_move_death1", &hover_move_death1 },
{ "hover_move_run", &hover_move_run },
{ "hover_move_walk", &hover_move_walk },
{ "hover_move_forward", &hover_move_forward },
{ "hover_move_land", &hover_move_land },
{ "hover_move_pain1", &hover_move_pain1 },
{ "hover_move_pain2", &hover_move_pain2 },
{ "hover_move_pain3", &hover_move_pain3 },
{ "hover_move_takeoff", &hover_move_takeoff },
{ "hover_move_stop2", &hover_move_stop2 },
{ "hover_move_stop1", &hover_move_stop1 },
{ "hover_move_stand", &hover_move_stand },
{ "gunner_move_attack_grenade", &gunner_move_attack_grenade },
{ "gunner_move_endfire_chain", &gunner_move_endfire_chain },
{ "gunner_move_fire_chain", &gunner_move_fire_chain },
{ "gunner_move_attack_chain", &gunner_move_attack_chain },
{ "gunner_move_duck", &gunner_move_duck },
{ "gunner_move_death", &gunner_move_death },
{ "gunner_move_pain1", &gunner_move_pain1 },
{ "gunner_move_pain2", &gunner_move_pain2 },
{ "gunner_move_pain3", &gunner_move_pain3 },
{ "gunner_move_runandshoot", &gunner_move_runandshoot },
{ "gunner_move_run", &gunner_move_run },
{ "gunner_move_walk", &gunner_move_walk },
{ "gunner_move_stand", &gunner_move_stand },
{ "gunner_move_fidget", &gunner_move_fidget },
{ "gladiator_move_death", &gladiator_move_death },
{ "gladiator_move_pain_air", &gladiator_move_pain_air },
{ "gladiator_move_pain", &gladiator_move_pain },
{ "gladiator_move_attack_gun", &gladiator_move_attack_gun },
{ "gladiator_move_attack_melee", &gladiator_move_attack_melee },
{ "gladiator_move_run", &gladiator_move_run },
{ "gladiator_move_walk", &gladiator_move_walk },
{ "gladiator_move_stand", &gladiator_move_stand },
{ "flyer_move_loop_melee", &flyer_move_loop_melee },
{ "flyer_move_end_melee", &flyer_move_end_melee },
{ "flyer_move_start_melee", &flyer_move_start_melee },
{ "flyer_move_attack2", &flyer_move_attack2 },
{ "flyer_move_bankleft", &flyer_move_bankleft },
{ "flyer_move_bankright", &flyer_move_bankright },
{ "flyer_move_defense", &flyer_move_defense },
{ "flyer_move_pain1", &flyer_move_pain1 },
{ "flyer_move_pain2", &flyer_move_pain2 },
{ "flyer_move_pain3", &flyer_move_pain3 },
{ "flyer_move_rollleft", &flyer_move_rollleft },
{ "flyer_move_rollright", &flyer_move_rollright },
{ "flyer_move_stop", &flyer_move_stop },
{ "flyer_move_start", &flyer_move_start },
{ "flyer_move_run", &flyer_move_run },
{ "flyer_move_walk", &flyer_move_walk },
{ "flyer_move_stand", &flyer_move_stand },
{ "floater_move_run", &floater_move_run },
{ "floater_move_walk", &floater_move_walk },
{ "floater_move_pain3", &floater_move_pain3 },
{ "floater_move_pain2", &floater_move_pain2 },
{ "floater_move_pain1", &floater_move_pain1 },
{ "floater_move_death", &floater_move_death },
{ "floater_move_attack3", &floater_move_attack3 },
{ "floater_move_attack2", &floater_move_attack2 },
{ "floater_move_attack1", &floater_move_attack1 },
{ "floater_move_activate", &floater_move_activate },
{ "floater_move_stand2", &floater_move_stand2 },
{ "floater_move_stand1", &floater_move_stand1 },
{ "flipper_move_death", &flipper_move_death },
{ "flipper_move_attack", &flipper_move_attack },
{ "flipper_move_pain1", &flipper_move_pain1 },
{ "flipper_move_pain2", &flipper_move_pain2 },
{ "flipper_move_start_run", &flipper_move_start_run },
{ "flipper_move_walk", &flipper_move_walk },
{ "flipper_move_run_start", &flipper_move_run_start },
{ "flipper_move_run_loop", &flipper_move_run_loop },
{ "flipper_move_stand", &flipper_move_stand },
{ "chick_move_start_slash", &chick_move_start_slash },
{ "chick_move_end_slash", &chick_move_end_slash },
{ "chick_move_slash", &chick_move_slash },
{ "chick_move_end_attack1", &chick_move_end_attack1 },
{ "chick_move_attack1", &chick_move_attack1 },
{ "chick_move_start_attack1", &chick_move_start_attack1 },
{ "chick_move_duck", &chick_move_duck },
{ "chick_move_death1", &chick_move_death1 },
{ "chick_move_death2", &chick_move_death2 },
{ "chick_move_pain3", &chick_move_pain3 },
{ "chick_move_pain2", &chick_move_pain2 },
{ "chick_move_pain1", &chick_move_pain1 },
{ "chick_move_walk", &chick_move_walk },
{ "chick_move_run", &chick_move_run },
{ "chick_move_start_run", &chick_move_start_run },
{ "chick_move_stand", &chick_move_stand },
{ "chick_move_fidget", &chick_move_fidget },
{ "brain_move_run", &brain_move_run },
{ "brain_move_attack2", &brain_move_attack2 },
{ "brain_move_attack1", &brain_move_attack1 },
{ "brain_move_death1", &brain_move_death1 },
{ "brain_move_death2", &brain_move_death2 },
{ "brain_move_duck", &brain_move_duck },
{ "brain_move_pain1", &brain_move_pain1 },
{ "brain_move_pain2", &brain_move_pain2 },
{ "brain_move_pain3", &brain_move_pain3 },
{ "brain_move_defense", &brain_move_defense },
{ "brain_move_walk1", &brain_move_walk1 },
{ "brain_move_idle", &brain_move_idle },
{ "brain_move_stand", &brain_move_stand },
{ "makron_move_attack5", &makron_move_attack5 },
{ "makron_move_attack4", &makron_move_attack4 },
{ "makron_move_attack3", &makron_move_attack3 },
{ "makron_move_sight", &makron_move_sight },
{ "makron_move_death3", &makron_move_death3 },
{ "makron_move_death2", &makron_move_death2 },
{ "makron_move_pain4", &makron_move_pain4 },
{ "makron_move_pain5", &makron_move_pain5 },
{ "makron_move_pain6", &makron_move_pain6 },
{ "makron_move_walk", &makron_move_walk },
{ "makron_move_run", &makron_move_run },
{ "makron_move_stand", &makron_move_stand },
{ "jorg_move_end_attack1", &jorg_move_end_attack1 },
{ "jorg_move_attack1", &jorg_move_attack1 },
{ "jorg_move_start_attack1", &jorg_move_start_attack1 },
{ "jorg_move_attack2", &jorg_move_attack2 },
{ "jorg_move_death", &jorg_move_death },
{ "jorg_move_pain1", &jorg_move_pain1 },
{ "jorg_move_pain2", &jorg_move_pain2 },
{ "jorg_move_pain3", &jorg_move_pain3 },
{ "jorg_move_end_walk", &jorg_move_end_walk },
{ "jorg_move_walk", &jorg_move_walk },
{ "jorg_move_start_walk", &jorg_move_start_walk },
{ "jorg_move_run", &jorg_move_run },
{ "jorg_move_stand", &jorg_move_stand },
{ "boss2_move_death", &boss2_move_death },
{ "boss2_move_pain_light", &boss2_move_pain_light },
{ "boss2_move_pain_heavy", &boss2_move_pain_heavy },
{ "boss2_move_attack_rocket", &boss2_move_attack_rocket },
{ "boss2_move_attack_post_mg", &boss2_move_attack_post_mg },
{ "boss2_move_attack_mg", &boss2_move_attack_mg },
{ "boss2_move_attack_pre_mg", &boss2_move_attack_pre_mg },
{ "boss2_move_run", &boss2_move_run },
{ "boss2_move_walk", &boss2_move_walk },
{ "boss2_move_fidget", &boss2_move_fidget },
{ "boss2_move_stand", &boss2_move_stand },
{ "berserk_move_death2", &berserk_move_death2 },
{ "berserk_move_death1", &berserk_move_death1 },
{ "berserk_move_pain2", &berserk_move_pain2 },
{ "berserk_move_pain1", &berserk_move_pain1 },
{ "berserk_move_attack_strike", &berserk_move_attack_strike },
{ "berserk_move_attack_club", &berserk_move_attack_club },
{ "berserk_move_attack_spike", &berserk_move_attack_spike },
{ "berserk_move_run1", &berserk_move_run1 },
{ "berserk_move_walk", &berserk_move_walk },
{ "berserk_move_stand_fidget", &berserk_move_stand_fidget },
{ "berserk_move_stand", &berserk_move_stand },
{ "actor_move_attack", &actor_move_attack },
{ "actor_move_death2", &actor_move_death2 },
{ "actor_move_death1", &actor_move_death1 },
{ "actor_move_taunt", &actor_move_taunt },
{ "actor_move_flipoff", &actor_move_flipoff },
{ "actor_move_pain3", &actor_move_pain3 },
{ "actor_move_pain2", &actor_move_pain2 },
{ "actor_move_pain1", &actor_move_pain1 },
{ "actor_move_run", &actor_move_run },
{ "actor_move_walk", &actor_move_walk },
{ "actor_move_stand", &actor_move_stand },
{ 0, 0 }

};

/*
* Fields to be saved
*/
field_t fields[] = {
{ "classname", FOFS(classname), F_LSTRING },
{ "model", FOFS(model), F_LSTRING },
{ "spawnflags", FOFS(spawnflags), F_INT },
{ "speed", FOFS(speed), F_FLOAT },
{ "accel", FOFS(accel), F_FLOAT },
{ "decel", FOFS(decel), F_FLOAT },
{ "target", FOFS(target), F_LSTRING },
{ "targetname", FOFS(targetname), F_LSTRING },
{ "pathtarget", FOFS(pathtarget), F_LSTRING },
{ "deathtarget", FOFS(deathtarget), F_LSTRING },
{ "killtarget", FOFS(killtarget), F_LSTRING },
{ "combattarget", FOFS(combattarget), F_LSTRING },
{ "message", FOFS(message), F_LSTRING },
{ "team", FOFS(team), F_LSTRING },
{ "wait", FOFS(wait), F_FLOAT },
{ "delay", FOFS(delay), F_FLOAT },
{ "random", FOFS(random), F_FLOAT },
{ "move_origin", FOFS(move_origin), F_VECTOR },
{ "move_angles", FOFS(move_angles), F_VECTOR },
{ "style", FOFS(style), F_INT },
{ "count", FOFS(count), F_INT },
{ "health", FOFS(health), F_INT },
{ "sounds", FOFS(sounds), F_INT },
{ "light", 0, F_IGNORE },
{ "dmg", FOFS(dmg), F_INT },
{ "mass", FOFS(mass), F_INT },
{ "volume", FOFS(volume), F_FLOAT },
{ "attenuation", FOFS(attenuation), F_FLOAT },
{ "map", FOFS(map), F_LSTRING },
{ "origin", FOFS(s.origin), F_VECTOR },
{ "angles", FOFS(s.angles), F_VECTOR },
{ "angle", FOFS(s.angles), F_ANGLEHACK },
{ "goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN },
{ "movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN },
{ "enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN },
{ "oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN },
{ "activator", FOFS(activator), F_EDICT, FFL_NOSPAWN },
{ "groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN },
{ "teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN },
{ "teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN },
{ "owner", FOFS(owner), F_EDICT, FFL_NOSPAWN },
{ "mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN },
{ "mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN },
{ "target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN },
{ "chain", FOFS(chain), F_EDICT, FFL_NOSPAWN },
{ "prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN },
{ "think", FOFS(think), F_FUNCTION, FFL_NOSPAWN },
{ "blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN },
{ "touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN },
{ "use", FOFS(use), F_FUNCTION, FFL_NOSPAWN },
{ "pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN },
{ "die", FOFS(die), F_FUNCTION, FFL_NOSPAWN },
{ "stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN },
{ "idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN },
{ "search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN },
{ "walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN },
{ "run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN },
{ "dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN },
{ "attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN },
{ "melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN },
{ "sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN },
{ "checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN },
{ "currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN },
{ "endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN },
{ "lip", STOFS(lip), F_INT, FFL_SPAWNTEMP },
{ "distance", STOFS(distance), F_INT, FFL_SPAWNTEMP },
{ "height", STOFS(height), F_INT, FFL_SPAWNTEMP },
{ "noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP },
{ "pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP },
{ "item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP },
{ "item", FOFS(item), F_ITEM },
{ "gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP },
{ "sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP },
{ "skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP },
{ "skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP },
{ "minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP },
{ "maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP },
{ "minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP },
{ "maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP },
{ "nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP },
{ 0, 0, 0, 0 }
};

/*
* Level fields to
* be saved
*/
field_t levelfields[] = {
{ "changemap", LLOFS(changemap), F_LSTRING },
{ "sight_client", LLOFS(sight_client), F_EDICT },
{ "sight_entity", LLOFS(sight_entity), F_EDICT },
{ "sound_entity", LLOFS(sound_entity), F_EDICT },
{ "sound2_entity", LLOFS(sound2_entity), F_EDICT },
{ NULL, 0, F_INT }
};

/*
* Client fields to
* be saved
*/
field_t clientfields[] = {
{ "pers.weapon", CLOFS(pers.weapon), F_ITEM },
{ "pers.lastweapon", CLOFS(pers.lastweapon), F_ITEM },
{ "newweapon", CLOFS(newweapon), F_ITEM },
{ NULL, 0, F_INT }
};

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/

void InitGame (void) {
	gi.dprintf ("==== InitGame ====\n");

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar ("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);


	g_monsterRespawn = gi.cvar ("g_monsterRespawn", "0.5", CVAR_ARCHIVE);
	sv_solidcorpse = gi.cvar ("sv_solidcorpse", "1", CVAR_ARCHIVE);
	net_compatibility = gi.cvar ("net_compatibility", "0", CVAR_SERVERINFO | CVAR_NOSET);
	r_radialBlur = gi.cvar ("r_radialBlur", "1", CVAR_ARCHIVE);
	sv_stopClock = gi.cvar ("sv_stopClock", "0", 0);

	g_autoHealth = gi.cvar("g_autoHealth", "25", CVAR_ARCHIVE);
	g_autoHealth->help = "Automatic recovery of health (25 max, 0 turn off it).\nNightmare skill only.\n";
	
	weaponHitAccuracy = gi.cvar("weaponHitAccuracy", "1", CVAR_USERINFO | CVAR_ARCHIVE);
	g_noStopMusic = gi.cvar("g_noStopMusic", "0", CVAR_ARCHIVE);
	g_infinityGibs = gi.cvar("g_infinityGibs", "0", CVAR_ARCHIVE);

	// items
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts = gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients + 1;
	gi.dprintf ("\n");
	gi.dprintf ("==== Use Quake2xp Net Protocol ====\n");
	gi.dprintf ("\n");

}

/* ========================================================= */

/*
* Helper function to get
* the human readable function
* definition by an address.
* Called by WriteField1 and
* WriteField2.
*/
functionList_t *
GetFunctionByAddress(byte *adr)
{
	int i;

	for (i = 0; functionList[i].funcStr; i++)
	{
		if (functionList[i].funcPtr == adr)
		{
			return &functionList[i];
		}
	}

	return NULL;
}

/*
* Helper function to get the
* pointer to a function by
* it's human readable name.
* Called by WriteField1 and
* WriteField2.
*/
byte *
FindFunctionByName(char *name)
{
	int i;

	for (i = 0; functionList[i].funcStr; i++)
	{
		if (!strcmp(name, functionList[i].funcStr))
		{
			return functionList[i].funcPtr;
		}
	}

	return NULL;
}

/*
* Helper function to get the
* human readable definition of
* a mmove_t struct by a pointer.
*/
mmoveList_t *
GetMmoveByAddress(mmove_t *adr)
{
	int i;

	for (i = 0; mmoveList[i].mmoveStr; i++)
	{
		if (mmoveList[i].mmovePtr == adr)
		{
			return &mmoveList[i];
		}
	}

	return NULL;
}

/*
* Helper function to get the
* pointer to a mmove_t struct
* by a human readable definition.
*/
mmove_t *
FindMmoveByName(char *name)
{
	int i;

	for (i = 0; mmoveList[i].mmoveStr; i++)
	{
		if (!strcmp(name, mmoveList[i].mmoveStr))
		{
			return mmoveList[i].mmovePtr;
		}
	}

	return NULL;
}

/* ========================================================= */

/*
* The following two functions are
* doing the dirty work to write the
* data generated by the functions
* below this block into files.
*/
void
WriteField1(FILE *f, field_t *field, byte *base)
{
	void *p;
	int len;
	int index;
	functionList_t *func;
	mmoveList_t *mmove;

	if (field->flags & FFL_SPAWNTEMP)
	{
		return;
	}

	p = (void *)(base + field->ofs);

	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:

		if (*(char **)p)
		{
			len = strlen(*(char **)p) + 1;
		}
		else
		{
			len = 0;
		}

		*(int *)p = len;
		break;
	case F_EDICT:

		if (*(edict_t **)p == NULL)
		{
			index = -1;
		}
		else
		{
			index = *(edict_t **)p - g_edicts;
		}

		*(int *)p = index;
		break;
	case F_CLIENT:

		if (*(gclient_t **)p == NULL)
		{
			index = -1;
		}
		else
		{
			index = *(gclient_t **)p - game.clients;
		}

		*(int *)p = index;
		break;
	case F_ITEM:

		if (*(edict_t **)p == NULL)
		{
			index = -1;
		}
		else
		{
			index = *(gitem_t **)p - itemlist;
		}

		*(int *)p = index;
		break;
	case F_FUNCTION:

		if (*(byte **)p == NULL)
		{
			len = 0;
		}
		else
		{
			func = GetFunctionByAddress(*(byte **)p);

			if (!func)
			{
				gi.error("WriteField1: function not in list, can't save game");
			}

			len = strlen(func->funcStr) + 1;
		}

		*(int *)p = len;
		break;
	case F_MMOVE:

		if (*(byte **)p == NULL)
		{
			len = 0;
		}
		else
		{
			mmove = GetMmoveByAddress(*(mmove_t **)p);

			if (!mmove)
			{
				gi.error("WriteField1: mmove not in list, can't save game");
			}

			len = strlen(mmove->mmoveStr) + 1;
		}

		*(int *)p = len;
		break;
	default:
		gi.error("WriteEdict: unknown field type");
	}
}

void
WriteField2(FILE *f, field_t *field, byte *base)
{
	int len;
	void *p;
	functionList_t *func;
	mmoveList_t *mmove;

	if (field->flags & FFL_SPAWNTEMP)
	{
		return;
	}

	p = (void *)(base + field->ofs);

	switch (field->type)
	{
	case F_LSTRING:

		if (*(char **)p)
		{
			len = strlen(*(char **)p) + 1;
			fwrite(*(char **)p, len, 1, f);
		}

		break;
	case F_FUNCTION:

		if (*(byte **)p)
		{
			func = GetFunctionByAddress(*(byte **)p);

			if (!func)
			{
				gi.error("WriteField2: function not in list, can't save game");
			}

			len = strlen(func->funcStr) + 1;
			fwrite(func->funcStr, len, 1, f);
		}

		break;
	case F_MMOVE:

		if (*(byte **)p)
		{
			mmove = GetMmoveByAddress(*(mmove_t **)p);
			if (!mmove)
			{
				gi.error("WriteField2: mmove not in list, can't save game");
			}

			len = strlen(mmove->mmoveStr) + 1;
			fwrite(mmove->mmoveStr, len, 1, f);
		}

		break;
	default:
		break;
	}
}

/* ========================================================= */

/*
* This function does the dirty
* work to read the data from a
* file. The processing of the
* data is done in the functions
* below
*/
void
ReadField(FILE *f, field_t *field, byte *base)
{
	void *p;
	int len;
	int index;
	char funcStr[2048];

	if (field->flags & FFL_SPAWNTEMP)
	{
		return;
	}

	p = (void *)(base + field->ofs);

	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int *)p;

		if (!len)
		{
			*(char **)p = NULL;
		}
		else
		{
			*(char **)p = gi.TagMalloc(32 + len, TAG_LEVEL);
			fread(*(char **)p, len, 1, f);
		}

		break;
	case F_EDICT:
		index = *(int *)p;

		if (index == -1)
		{
			*(edict_t **)p = NULL;
		}
		else
		{
			*(edict_t **)p = &g_edicts[index];
		}

		break;
	case F_CLIENT:
		index = *(int *)p;

		if (index == -1)
		{
			*(gclient_t **)p = NULL;
		}
		else
		{
			*(gclient_t **)p = &game.clients[index];
		}

		break;
	case F_ITEM:
		index = *(int *)p;

		if (index == -1)
		{
			*(gitem_t **)p = NULL;
		}
		else
		{
			*(gitem_t **)p = &itemlist[index];
		}

		break;
	case F_FUNCTION:
		len = *(int *)p;

		if (!len)
		{
			*(byte **)p = NULL;
		}
		else
		{
			if (len > sizeof(funcStr))
			{
				gi.error("ReadField: function name is longer than buffer (%i chars)",
					(int)sizeof(funcStr));
			}

			fread(funcStr, len, 1, f);

			if (!(*(byte **)p = FindFunctionByName(funcStr)))
			{
				gi.error("ReadField: function %s not found in table, can't load game", funcStr);
			}

		}
		break;
	case F_MMOVE:
		len = *(int *)p;

		if (!len)
		{
			*(byte **)p = NULL;
		}
		else
		{
			if (len > sizeof(funcStr))
			{
				gi.error("ReadField: mmove name is longer than buffer (%i chars)",
					(int)sizeof(funcStr));
			}

			fread(funcStr, len, 1, f);

			if (!(*(mmove_t **)p = FindMmoveByName(funcStr)))
			{
				gi.error("ReadField: mmove %s not found in table, can't load game", funcStr);
			}
		}
		break;

	default:
		gi.error("ReadEdict: unknown field type");
	}
}

/* ========================================================= */

/*
* Write the client struct into a file.
*/
void
WriteClient(FILE *f, gclient_t *client)
{
	field_t *field;
	gclient_t temp;

	/* all of the ints, floats, and vectors stay as they are */
	temp = *client;

	/* change the pointers to indexes */
	for (field = clientfields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	/* write the block */
	fwrite(&temp, sizeof(temp), 1, f);

	/* now write any allocated data following the edict */
	for (field = clientfields; field->name; field++)
	{
		WriteField2(f, field, (byte *)client);
	}
}

/*
* Read the client struct from a file
*/
void
ReadClient(FILE *f, gclient_t *client)
{
	field_t *field;

	fread(client, sizeof(*client), 1, f);

	for (field = clientfields; field->name; field++)
	{
		ReadField(f, field, (byte *)client);
	}
}

int
Q_strlcpy(char *dst, const char *src, int size)
{
	const char *s = src;

	while (*s)
	{
		if (size > 1)
		{
			*dst++ = *s;
			size--;
		}
		s++;
	}
	if (size > 0)
	{
		*dst = '\0';
	}

	return s - src;
}
/* ========================================================= */

/*
* Writes the game struct into
* a file. This is called when
* ever the games goes to e new
* level or the user saves the
* game. Saved informations are:
* - cross level data
* - client states
* - help computer info
*/
void
WriteGame(char *filename, qboolean autosave)
{
	FILE *f;
	int i;
	char str_ver[32];
	char str_game[32];
	char str_os[32];
	char str_arch[32];

	if (!autosave)
	{
		SaveClientData();
	}

	f = fopen(filename, "wb");

	if (!f)
	{
		gi.error("Couldn't open %s", filename);
	}

	/* Savegame identification */
	memset(str_ver, 0, sizeof(str_ver));
	memset(str_game, 0, sizeof(str_game));
	memset(str_os, 0, sizeof(str_os));
	memset(str_arch, 0, sizeof(str_arch));

	Q_strlcpy(str_ver, SAVEGAMEVER, sizeof(str_ver) - 1);
	Q_strlcpy(str_game, GAMEVERSION, sizeof(str_game) - 1);
	Q_strlcpy(str_os, OSTYPE_1, sizeof(str_os) - 1);
	Q_strlcpy(str_arch, ARCH_1, sizeof(str_arch) - 1);

	fwrite(str_ver, sizeof(str_ver), 1, f);
	fwrite(str_game, sizeof(str_game), 1, f);
	fwrite(str_os, sizeof(str_os), 1, f);
	fwrite(str_arch, sizeof(str_arch), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = qfalse;

	for (i = 0; i < game.maxclients; i++)
	{
		WriteClient(f, &game.clients[i]);
	}

	fclose(f);
}

/*
* Read the game structs from
* a file. Called when ever a
* savegames is loaded.
*/
void
ReadGame(char *filename)
{
	FILE *f;
	int i;
	char str_ver[32];
	char str_game[32];
	char str_os[32];
	char str_arch[32];

	gi.FreeTags(TAG_GAME);

	f = fopen(filename, "rb");

	if (!f)
	{
		gi.error("Couldn't open %s", filename);
	}

	/* Sanity checks */
	fread(str_ver, sizeof(str_ver), 1, f);
	fread(str_game, sizeof(str_game), 1, f);
	fread(str_os, sizeof(str_os), 1, f);
	fread(str_arch, sizeof(str_arch), 1, f);

	if (!strcmp(str_ver, SAVEGAMEVER))
	{
		if (strcmp(str_game, GAMEVERSION))
		{
			fclose(f);
			gi.error("Savegame from another game.so.\n");
		}
		else if (strcmp(str_os, OSTYPE_1))
		{
			fclose(f);
			gi.error("Savegame from another os.\n");
		}
		else if (strcmp(str_arch, ARCH_1))
		{
			fclose(f);
			gi.error("Savegame from another architecture.\n");
		}
	}
	else
	{
		fclose(f);
		gi.error("Savegame from an incompatible version.\n");
	}

	g_edicts = gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread(&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc(game.maxclients * sizeof(game.clients[0]),
		TAG_GAME);

	for (i = 0; i < game.maxclients; i++)
	{
		ReadClient(f, &game.clients[i]);
	}

	fclose(f);
}

/* ========================================================== */

/*
* Helper function to write the
* edict into a file. Called by
* WriteLevel.
*/
void
WriteEdict(FILE *f, edict_t *ent)
{
	field_t *field;
	edict_t temp;

	/* all of the ints, floats, and vectors stay as they are */
	temp = *ent;

	/* change the pointers to lengths or indexes */
	for (field = fields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	/* write the block */
	fwrite(&temp, sizeof(temp), 1, f);

	/* now write any allocated data following the edict */
	for (field = fields; field->name; field++)
	{
		WriteField2(f, field, (byte *)ent);
	}
}

/*
* Helper function to write the
* level local data into a file.
* Called by WriteLevel.
*/
void
WriteLevelLocals(FILE *f)
{
	field_t *field;
	level_locals_t temp;

	/* all of the ints, floats, and vectors stay as they are */
	temp = level;

	/* change the pointers to lengths or indexes */
	for (field = levelfields; field->name; field++)
	{
		WriteField1(f, field, (byte *)&temp);
	}

	/* write the block */
	fwrite(&temp, sizeof(temp), 1, f);

	/* now write any allocated data following the edict */
	for (field = levelfields; field->name; field++)
	{
		WriteField2(f, field, (byte *)&level);
	}
}

/*
* Writes the current level
* into a file.
*/
void
WriteLevel(char *filename)
{
	int i;
	edict_t *ent;
	FILE *f;

	f = fopen(filename, "wb");

	if (!f)
	{
		gi.error("Couldn't open %s", filename);
	}

	/* write out edict size for checking */
	i = sizeof(edict_t);
	fwrite(&i, sizeof(i), 1, f);

	/* write out level_locals_t */
	WriteLevelLocals(f);

	/* write out all the entities */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
		{
			continue;
		}

		fwrite(&i, sizeof(i), 1, f);
		WriteEdict(f, ent);
	}

	i = -1;
	fwrite(&i, sizeof(i), 1, f);

	fclose(f);
}

/* ========================================================== */

/*
* A helper function to
* read the edict back
* into the memory. Called
* by ReadLevel.
*/
void
ReadEdict(FILE *f, edict_t *ent)
{
	field_t *field;

	fread(ent, sizeof(*ent), 1, f);

	for (field = fields; field->name; field++)
	{
		ReadField(f, field, (byte *)ent);
	}
}

/*
* A helper function to
* read the level local
* data from a file.
* Called by ReadLevel.
*/
void
ReadLevelLocals(FILE *f)
{
	field_t *field;

	fread(&level, sizeof(level), 1, f);

	for (field = levelfields; field->name; field++)
	{
		ReadField(f, field, (byte *)&level);
	}
}

/*
* Reads a level back into the memory.
* SpawnEntities were already called
* in the same way when the level was
* saved. All world links were cleared
* before this function was called. When
* this function is called, no clients
* are connected to the server.
*/
void
ReadLevel(char *filename)
{
	int entnum;
	FILE *f;
	int i;
	edict_t *ent;

	f = fopen(filename, "rb");

	if (!f)
	{
		gi.error("Couldn't open %s", filename);
	}

	/* free any dynamic memory allocated by
	loading the level  base state */
	gi.FreeTags(TAG_LEVEL);

	/* wipe all the entities */
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value + 1;

	/* check edict size */
	fread(&i, sizeof(i), 1, f);

	if (i != sizeof(edict_t))
	{
		fclose(f);
		gi.error("ReadLevel: mismatched edict size");
	}

	/* load the level locals */
	ReadLevelLocals(f);

	/* load all the entities */
	while (1)
	{
		if (fread(&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose(f);
			gi.error("ReadLevel: failed to read entnum");
		}

		if (entnum == -1)
		{
			break;
		}

		if (entnum >= globals.num_edicts)
		{
			globals.num_edicts = entnum + 1;
		}

		ent = &g_edicts[entnum];
		ReadEdict(f, ent);

		/* let the server rebuild world links for this ent */
		memset(&ent->area, 0, sizeof(ent->area));
		gi.linkentity(ent);
	}

	fclose(f);

	/* mark all clients as unconnected */
	for (i = 0; i < maxclients->value; i++)
	{
		ent = &g_edicts[i + 1];
		ent->client = game.clients + i;
		ent->client->pers.connected = qfalse;
	}

	/* do any load time things at this point */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
		{
			continue;
		}

		/* fire any cross-level triggers */
		if (ent->classname)
		{
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
			{
				ent->nextthink = level.time + ent->delay;
			}
		}
	}
}
