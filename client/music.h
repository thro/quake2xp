#ifndef _MUSIC_H_
#define _MUSIC_H_

// Implementation header, do not include in other places than music.c

#define OV_EXCLUDE_STATIC_CALLBACKS
#ifdef _WIN32
#include "vorbis/vorbisfile.h"
#else
#include <vorbis/vorbisfile.h>
#endif

// Generic interfaces

typedef struct {
	int bits;
	int channels;
	int rate;
} soundparams_t;

typedef int (*readFunc_t)(void *, void *, int);
typedef void (*rewindFunc_t)(void *);
typedef void (*closeFunc_t)(void *);

typedef struct {
	readFunc_t read;
	rewindFunc_t rewind;
	closeFunc_t close;
	void *f;
	const char *ext;
} Gen_Interface_t;

typedef Gen_Interface_t *(*openFunc_t)(const char *, soundparams_t *);

Gen_Interface_t *Gen_Open (const char *name, soundparams_t *sp);
Gen_Interface_t *Gen_OpenAny (const char *name, soundparams_t *sp);

typedef struct {
	char *name;
	openFunc_t openFunc;
} supported_exts_t;

// Extension specific interfaces

typedef struct {
	void *ovRawFile;
	int pos, size;

	OggVorbis_File ovFile;
	vorbis_info *info;
} MC_Vorbis_t;

typedef struct {
	byte *data, *start;
	int pos, size;
} MC_WAV_t;

static Gen_Interface_t *MC_OpenVorbis (const char *name, soundparams_t *sp);
static Gen_Interface_t *MC_OpenWAV (const char *name, soundparams_t *sp);

#endif
