/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "client.h"
#include "cdaudio.h"
#include "snd_loc.h"

// Implementation specific header, do not include outside music.c
#include "music.h"

typedef enum {
	MSTAT_STOPPED, MSTAT_PAUSED, MSTAT_PLAYING
} mstat_t;

static mstat_t mstat = MSTAT_STOPPED;
static music_type_t music_type = MUSIC_NONE;
static Gen_Interface_t *music_handle;

static char **fsList;
static int fsIndex;
static int fsNumFiles;

void Music_Init (void) {
	music_type = s_musicSrc->value;
	mstat = MSTAT_STOPPED;

	Com_Printf ("\n======== Init Music subsystem =======\n\n");

	switch (music_type) {
		case MUSIC_NONE:
			return;
		case MUSIC_CD:
			CDAudio_Init ();
			Com_Printf ("=====================================\n\n");
			break;
		case MUSIC_FILES:
			Com_Printf ("=====================================\n\n");
			break;
		case MUSIC_OTHER_FILES:
			fsList = FS_ListFilesAll ("music/*", &fsNumFiles, 0, SFF_SUBDIR);
			fsIndex = -1;

			if (fsList != NULL)
				Com_DPrintf (S_COLOR_YELLOW "found "S_COLOR_GREEN"%d "S_COLOR_YELLOW"music files\n\n", fsNumFiles);
			Com_Printf ("====================================\n\n");
			break;
		default:
			Cvar_SetValue ("s_musicSrc", MUSIC_NONE);
			music_type = MUSIC_NONE;
			return;
	}
}

void Music_Shutdown (void) {
	if (music_type == MUSIC_NONE)
		return;

	Com_Printf ("Music shutdown\n");
	Music_Stop ();

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Shutdown ();
			break;
		case MUSIC_FILES:
			break;
		case MUSIC_OTHER_FILES:
			FS_FreeList (fsList, fsNumFiles);
			break;
	}
	music_type = MUSIC_NONE;
}

// only to be called inside Music_Play
qboolean Music_PlayFile (const char *name, qboolean hasExt) {
	soundparams_t sp;

	if (hasExt)
		music_handle = Gen_Open (name, &sp);
	else
		music_handle = Gen_OpenAny (name, &sp);

	if (music_handle != NULL) {
		if (hasExt)
			Com_DPrintf (S_COLOR_GREEN "Music_Play: playing \"%s\"\n", name);
		else
			Com_DPrintf (S_COLOR_GREEN "Music_Play: playing \"%s.%s\"\n", name, music_handle->ext);

		S_Streaming_Start (sp.bits, sp.channels, sp.rate, s_musicVolume->value);
		mstat = cl_paused->integer ? MSTAT_PAUSED : MSTAT_PLAYING;
		return qtrue;
	}
	else {
		Com_Printf (S_COLOR_YELLOW "Music_Play: unable to load \"%s\"\n", name);
		return qfalse;
	}
}

void Music_Play (void) {
	int track, count;
	char name[MAX_QPATH];

	Music_Stop ();

	if (s_musicRandom->integer)
		// original soundtrack has tracks 2 to 11
		track = 2 + rand () % 10;
	else
		track = atoi (cl.configstrings[CS_CDTRACK]);

	if (music_type != MUSIC_OTHER_FILES &&
		track == 0 && !s_musicRandom->integer)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Play (track, qtrue);
			mstat = MSTAT_PLAYING;
			break;

		case MUSIC_FILES:
			if(modName("xatrix"))
				Q_snprintfz(name, sizeof(name), "music_sp1/track%02i", track);
			else
			if (modName("rogue"))
				Q_snprintfz(name, sizeof(name), "music_sp2/track%02i", track);
			else
				Q_snprintfz(name, sizeof(name), "music/track%02i", track);
			Music_PlayFile (name, qfalse);
			break;

		case MUSIC_OTHER_FILES:
			if (fsList == NULL)
				return;

			if (s_musicRandom->integer)
				fsIndex = rand () % fsNumFiles;
			else
				fsIndex = (fsIndex + 1) % fsNumFiles;
			count = fsNumFiles;
			while (count-- > 0) {
				if (Music_PlayFile (fsList[fsIndex], qtrue))
					return;
				fsIndex = (fsIndex + 1) % fsNumFiles;
			}
			break;
	}
}

void Music_Stop (void) {
	if (mstat == MSTAT_STOPPED)
		return;

	Com_DPrintf (S_COLOR_GREEN "Stopped playing music\n");

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Stop ();
			break;
		case MUSIC_FILES:
		case MUSIC_OTHER_FILES:
			music_handle->close (music_handle->f);
			S_Streaming_Stop ();
			break;
	}

	mstat = MSTAT_STOPPED;
}

void Music_Pause (void) {
	if (mstat != MSTAT_PLAYING)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Activate (qfalse);
			break;
		case MUSIC_FILES:
		case MUSIC_OTHER_FILES:
			alSourcePause (source_name[CH_STREAMING]);
			break;
	}

	mstat = MSTAT_PAUSED;
}

void Music_Resume (void) {
	if (mstat != MSTAT_PAUSED)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Activate (qtrue);
			break;
		case MUSIC_FILES:
		case MUSIC_OTHER_FILES:
			alSourcePlay (source_name[CH_STREAMING]);
			break;
	}

	mstat = MSTAT_PLAYING;
}

void Music_Update (void) {
	int n;

	// if we are in the configuration menu, or paused we don't do anything
	if (mstat == MSTAT_PAUSED)
		return;

	// Check for configuration changes

	if (s_musicSrc->modified) {
		Music_Shutdown ();
		Music_Init ();
		Music_Play ();
		s_musicSrc->modified = qfalse;
		s_musicVolume->modified = qfalse;
		s_musicRandom->modified = qfalse;
		return;
	}

	if (music_type == MUSIC_NONE)
		return;

	if (s_musicRandom->modified) {
		s_musicRandom->modified = qfalse;
		Music_Play ();
		return;
	}

	if (s_musicVolume->modified) {
		switch (music_type) {
			case MUSIC_CD:
				Cvar_SetValue ("cd_volume", s_musicVolume->value);
				break;
			case MUSIC_FILES:
			case MUSIC_OTHER_FILES:
				alSourcef (source_name[CH_STREAMING], AL_GAIN, s_musicVolume->value);
				break;
		}
		s_musicVolume->modified = qfalse;
		return;
	}

	// Do the actual update

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Update ();
			break;

		case MUSIC_FILES:
		case MUSIC_OTHER_FILES:
			if (mstat != MSTAT_PLAYING || S_Streaming_NumFreeBufs () == 0)
				return;
			// Play a portion of the current file
			n = music_handle->read(music_handle->f, music_buffer, MAX_STRBUF_SIZE);
			if (n == 0)
			{	/// Berserker's FIX: когда уже нечего читать из файла, все-равно ждём окончания проигрывания всей очереди буферов:
				if (streaming.bNumAvail == NUM_STRBUF)	// когда число свободных буферов станет максимальным,

					Music_Play();			// запускаем музыку заново!
			}
			else
				// don't check return value as the buffer is guaranteed to fit
				S_Streaming_Add(music_buffer, n);

	}
}

void S_Music_f (void) {
	// CD: uses the level track, or another one if s_musicrandom
	// Other: advances to the next, or random if s_musircandom
	Music_Play ();
}

// Generic interface and type specific implementations

supported_exts_t supported_exts[] = {
	{ "wav", (openFunc_t)MC_OpenWAV },
	{ "ogg", (openFunc_t)MC_OpenVorbis }
};

// Gen_OpenAny: try all possible extensions of filename in sequence
Gen_Interface_t *Gen_OpenAny (const char *name, soundparams_t *sp) {
	// check all supported extensions, or better try in a loop somewhere else?
	int i;

	for (i = 0; i < sizeof(supported_exts) / sizeof(supported_exts[0]); i++) {
		char path[MAX_QPATH];
		Gen_Interface_t *res;

		Q_snprintfz (path, sizeof(path), "%s.%s", name, supported_exts[i].name);
		res = supported_exts[i].openFunc (path, sp);
		if (res != NULL)
			return res;
	}
	return NULL;
}

// Gen_Open: check given filename and call appropiate routine
Gen_Interface_t *Gen_Open (const char *name, soundparams_t *sp) {
	int i;
	const char *ext = strrchr (name, '.');

	if (ext == NULL)
		return NULL;

	Com_DPrintf ("trying to load %s\n", name);

	for (i = 0; i < sizeof(supported_exts) / sizeof(supported_exts[0]); i++)
	if (Q_strcasecmp (ext + 1, supported_exts[i].name) == 0)
		return supported_exts[i].openFunc (name, sp);

	return NULL;
}

static int MC_ReadVorbis(MC_Vorbis_t *f, char *buffer, int n)
{
	int total = 0;

	while (1)
	{
		int cur = ov_read(&f->ovFile, buffer + total, MUSIC_BUFFER_READ_SIZE, 0, 2, 1, &f->pos);
		if (cur < 0)
			return 0;
		if (cur == 0)
			return total;
		if (total + cur > n)
			return total + cur;
		total += cur;
	}
}

static void MC_RewindVorbis (MC_Vorbis_t *f) {
	f->pos = 0;
}

static void MC_CloseVorbis (MC_Vorbis_t *f) {
	ov_clear (&f->ovFile);
	FS_FreeFile (f->ovRawFile);
}

static Gen_Interface_t *MC_OpenVorbis (const char *name, soundparams_t *sp) {
	static MC_Vorbis_t f;
	static Gen_Interface_t res;

	f.size = FS_LoadFile (name, &f.ovRawFile);
	if (f.size < 0)
		return NULL;

	if (ov_open (NULL, &f.ovFile, f.ovRawFile, f.size) == 0) {
		f.info = ov_info (&f.ovFile, 0);
		f.pos = 0;
		sp->bits = 16;
		sp->channels = f.info->channels;
		sp->rate = f.info->rate;

		res.read = (readFunc_t)MC_ReadVorbis;
		res.rewind = (rewindFunc_t)MC_RewindVorbis;
		res.close = (closeFunc_t)MC_CloseVorbis;
		res.f = &f;
		res.ext = "ogg";

		return &res;
	}
	else {
		FS_FreeFile (f.ovRawFile);
		return NULL;
	}
}

static int MC_ReadWAV (MC_WAV_t *f, void *buffer, int n) {
	const int r = MIN (n, f->size - f->pos);

	if (r > 0) {
		memcpy (buffer, f->start + f->pos, r);
		f->pos += r;
	}

	return r;
}

static void MC_RewindWAV (MC_WAV_t *f) {
	f->pos = 0;
}

static void MC_CloseWAV (MC_WAV_t *f) {
	FS_FreeFile (f->data);
}

static Gen_Interface_t *MC_OpenWAV (const char *name, soundparams_t *sp) {
	static MC_WAV_t f;
	static Gen_Interface_t res;

	if (S_LoadWAV (name, &f.data, &f.start, &sp->bits, &sp->channels, &sp->rate, &f.size)) {
		f.pos = 0;

		res.read = (readFunc_t)MC_ReadWAV;
		res.rewind = (rewindFunc_t)MC_RewindWAV;
		res.close = (closeFunc_t)MC_CloseWAV;
		res.f = &f;
		res.ext = "wav";

		return &res;
	}
	return NULL;
}
