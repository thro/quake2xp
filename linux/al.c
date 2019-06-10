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

/*
 * Linux version:
 * - We link to OpenAL at compile time.
 * - EFX is available for effects.
 */
#include "../client/client.h"
#include "../client/snd_loc.h"

/*
 =================
 QAL_Shutdown

 Unloads the specified DLL then nulls out all the proc pointers
 =================
*/
void QAL_Shutdown (void)
{
	Com_Printf("...shutting down QAL\n");
}

void S_SoundInfo_f(void);

/*
 =================
 AL_Init
 =================
*/
qboolean AL_StartOpenAL (void);
extern cvar_t *s_useHRTF;

static int ClampCvarInteger(int min, int max, int value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

qboolean AL_Init (int hardreset)
{
    Com_Printf("\n");
    Com_Printf("==="S_COLOR_YELLOW"Starting OpenAL audio subsystem"S_COLOR_WHITE"===\n");
    Com_Printf("\n");

	// Initialize OpenAL subsystem
	if (!AL_StartOpenAL())
	{
		// Let the user continue without sound
		Com_Printf (S_COLOR_RED"WARNING: OpenAL initialization failed\n");
		openalStop = qtrue;
		return qfalse;
	}
  
  if (!alIsExtensionPresent("AL_SOFT_source_resampler"))
	{
		Com_Printf(S_COLOR_MAGENTA"...AL_SOFT_source_resampler not found!\n");
	}else
		Com_Printf("...using AL_SOFT_source_resampler\n");

	alConfig.numResamplers = alGetInteger(AL_NUM_RESAMPLERS_SOFT);
	alConfig.defResampler = alGetInteger(AL_DEFAULT_RESAMPLER_SOFT);

	s_resamplerQuality->integer = ClampCvarInteger(0, alConfig.numResamplers, s_resamplerQuality->integer);
	ALint i;
	Com_Printf("...Available Resamplers:\n");
	for (i = 0; i < alConfig.numResamplers; ++i){

		const ALchar *name = alGetStringiSOFT(AL_RESAMPLER_NAME_SOFT, i);
		Com_Printf(">" S_COLOR_GREEN "%i" S_COLOR_WHITE " %s\n", i, name);
	}
	const ALchar *currName = alGetStringiSOFT(AL_RESAMPLER_NAME_SOFT, s_resamplerQuality->integer);
	Com_Printf("\n...select " S_COLOR_GREEN "%s" S_COLOR_WHITE " resampler\n", currName);
 
  
	// Initialize extensions
	alConfig.efx = qfalse;

	// Check for ALC Extensions
  	
    if (s_useEfx->integer) {
        if (alcIsExtensionPresent(alConfig.hDevice, "ALC_EXT_EFX") == AL_TRUE)
        {
            ALuint		uiEffectSlots[128] = { 0 };
            ALuint		uiEffects[1] = { 0 };
            ALuint		uiFilters[1] = { 0 };
            ALint		iEffectSlotsGenerated;
            ALint		iSends;

            alConfig.efx = qtrue;

            // To determine how many Auxiliary Effects Slots are available, create as many as possible (up to 128)
            // until the call fails.
            for (iEffectSlotsGenerated = 0; iEffectSlotsGenerated < 128; iEffectSlotsGenerated++)
            {
                alGenAuxiliaryEffectSlots(1, &uiEffectSlots[iEffectSlotsGenerated]);
                if (alGetError() != AL_NO_ERROR)
                    break;
            }

            Com_Printf("\n%d Auxiliary Effect Slot%s\n", iEffectSlotsGenerated, (iEffectSlotsGenerated == 1) ? "" : "s");

            // Retrieve the number of Auxiliary Effect Slots Sends available on each Source
            alcGetIntegerv(alConfig.hDevice, ALC_MAX_AUXILIARY_SENDS, 1, &iSends);
            Com_Printf("%d Auxiliary Send%s per Source\n", iSends, (iSends == 1) ? "" : "s");

            // To determine which Effects are supported, generate an Effect Object, and try to set its type to
            // the various Effect enum values
            Com_Printf("\nEffects Supported: -\n");
            alGenEffects(1, &uiEffects[0]);
            if (alGetError() == AL_NO_ERROR)
            {
                // Try setting Effect Type to known Effects
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_REVERB);
                Com_Printf("'Reverb' Support            %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
                Com_Printf("'EAX Reverb' Support        %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
                Com_Printf("'Chorus' Support            %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);
                Com_Printf("'Distortion' Support        %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_ECHO);
                Com_Printf("'Echo' Support              %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
                Com_Printf("'Flanger' Support           %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);
                Com_Printf("'Frequency Shifter' Support %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);
                Com_Printf("'Vocal Morpher' Support     %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);
                Com_Printf("'Pitch Shifter' Support     %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);
                Com_Printf("'Ring Modulator' Support    %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_AUTOWAH);
                Com_Printf("'Autowah' Support           %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);
                Com_Printf("'Compressor' Support        %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alEffecti(uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
                Com_Printf("'Equalizer' Support         %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
            }
            // To determine which Filters are supported, generate a Filter Object, and try to set its type to
            // the various Filter enum values
            Com_Printf("\nFilters Supported: -\n");
            // Generate a Filter to use to determine what Filter Types are supported
            alGenFilters(1, &uiFilters[0]);
            if (alGetError() == AL_NO_ERROR)
            {
                // Try setting the Filter type to known Filters
                alFilteri(uiFilters[0], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
                Com_Printf("'Low Pass'  Support         %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alFilteri(uiFilters[0], AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
                Com_Printf("'High Pass' Support         %s\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
                alFilteri(uiFilters[0], AL_FILTER_TYPE, AL_FILTER_BANDPASS);
                Com_Printf("'Band Pass' Support         %s\n\n", (alGetError() == AL_NO_ERROR) ? "YES" : "NO");
            }
            // Clean-up ...
            // Delete Filter
            alDeleteFilters(1, &uiFilters[0]);
            // Delete Effect
            alDeleteEffects(1, &uiEffects[0]);
            // Delete Auxiliary Effect Slots
            alDeleteAuxiliaryEffectSlots(iEffectSlotsGenerated, uiEffectSlots);
        }
    }
	return qtrue;
}
