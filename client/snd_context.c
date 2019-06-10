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

#include "../client/client.h"
#include "../client/snd_loc.h"

alConfig_t alConfig;
qboolean	openalStop = qfalse;

/*
 =================
 AL_InitDriver
 =================
 */
static qboolean AL_InitDriver (void) {
	char *deviceName = s_device->string;

	Com_Printf ("\n...Initializing OpenAL Driver\n");

	if (!deviceName[0])
		deviceName = NULL;

	if (deviceName)
		Com_Printf ("...Opening Device ("S_COLOR_GREEN"%s"S_COLOR_WHITE"): ", deviceName);
	else
		Com_Printf ("...Opening Default Device: ");

	// Open the device
	if ((alConfig.hDevice = alcOpenDevice (deviceName)) == NULL) {
		Com_Printf (S_COLOR_RED"failed\n");
		return qfalse;
	}

	if (!deviceName)
		Com_Printf ("succeeded ("S_COLOR_GREEN"%s"S_COLOR_WHITE")\n",
		alcGetString (alConfig.hDevice, ALC_DEVICE_SPECIFIER));
	else
		Com_Printf (S_COLOR_GREEN"succeeded\n");

	// Create the AL context and make it current
	Com_Printf ("...Creating AL Context: ");
	
	qboolean hrtf = qfalse;
	if (alcIsExtensionPresent(alConfig.hDevice, "ALC_SOFT_HRTF") == AL_TRUE)
		hrtf = qtrue;

	int quality;
	#ifdef _WIN32
		quality = 48000;
	#else
		quality = 44100; //wtf? soft al under linux use only 44100hz
	#endif

		ALCint attrlist[6] = 
		{	ALC_FREQUENCY, quality, 
			ALC_HRTF_SOFT, s_useHRTF->integer && hrtf ? ALC_TRUE : AL_FALSE,
		0 };
		
		if ((alConfig.hALC =
			alcCreateContext (alConfig.hDevice, attrlist)) == NULL) {
			Com_Printf (S_COLOR_RED"failed\n");
			goto failed;
		}
	
	Com_Printf (S_COLOR_GREEN"succeeded\n");

	Com_Printf ("...Making Context Current: ");
	if (!alcMakeContextCurrent (alConfig.hALC)) {
		Com_Printf (S_COLOR_RED"failed\n");
		goto failed;
	}
	Com_Printf (S_COLOR_GREEN"succeeded\n");

	Com_Printf("\n=====================================\n\n");

	if (hrtf) {
		Com_Printf("...using ALC_SOFT_HRTF\n");
		ALCint	hrtfState;
		alcGetIntegerv(alConfig.hDevice, ALC_HRTF_SOFT, 1, &hrtfState);
		if (!hrtfState)
			Com_Printf("...HRTF Mode:" S_COLOR_YELLOW " off\n");
		else
		{
			const ALchar *name = alcGetString(alConfig.hDevice, ALC_HRTF_SPECIFIER_SOFT);
			Com_Printf("...using " S_COLOR_GREEN "%s\n", name);
			Com_Printf("...HRTF Mode:" S_COLOR_GREEN " on\n");
		}

	}else
		Com_Printf(S_COLOR_RED"...ALC_SOFT_HRTF not found\n");

	return qtrue;

failed:

	Com_Printf (S_COLOR_RED"...failed hard\n");

	openalStop = qtrue;

	if (alConfig.hALC) {
		alcDestroyContext (alConfig.hALC);
		alConfig.hALC = NULL;
	}

	if (alConfig.hDevice) {
		alcCloseDevice (alConfig.hDevice);
		alConfig.hDevice = NULL;
	}

	return qfalse;
}

/*
 =================
 AL_StartOpenAL
 =================
 */

qboolean AL_StartOpenAL (void) {
	extern const char *al_device[];

	// Get device list
	if (alcIsExtensionPresent (NULL, "ALC_ENUMERATE_ALL_EXT")) { // find all ))
		unsigned i = 0;
		const char *a = alcGetString (NULL, ALC_ALL_DEVICES_SPECIFIER);
		if (!a) {
			// We have no audio output devices. No hope.
			QAL_Shutdown ();
			return qfalse;
		}
		
		Com_Printf("====== Available Output Devices =====\n\n");

		while (*a) {
			al_device[++i] = a;
			Com_Printf (">:"S_COLOR_GREEN"%s\n", a);
			while (*a)
				a++;
			a++;
		}
		alConfig.device_count = i;
	}
	else {
		QAL_Shutdown ();
		return qfalse;
	}

	Com_Printf("\n=====================================\n");

	// Initialize the device, context, etc...
	if (AL_InitDriver ()) {
		return qtrue;
	}
	else {
		QAL_Shutdown ();
		return qfalse;
	}
}

/*
 =================
 AL_Shutdown
 =================
 */
void AL_Shutdown (void) {
	Com_Printf ("Shutting down OpenAL subsystem\n");

	if (alConfig.hALC) {
		if (alcMakeContextCurrent) {
			Com_Printf ("...alcMakeContextCurrent( NULL ): ");
			if (!alcMakeContextCurrent (NULL))
				Com_Printf ("failed\n");
			else
				Com_Printf ("succeeded\n");
		}

		if (alcDestroyContext) {
			Com_Printf ("...destroying AL context\n");
			alcDestroyContext (alConfig.hALC);
		}

		alConfig.hALC = NULL;
	}

	if (alConfig.hDevice) {
		if (alcCloseDevice) {
			Com_Printf ("...closing device\n");
			alcCloseDevice (alConfig.hDevice);
		}

		alConfig.hDevice = NULL;
	}

	QAL_Shutdown ();
}
