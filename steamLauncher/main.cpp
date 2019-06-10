/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*==============================================
STEAM(TM) LAUNCHER FOR QUAKE2XP
THIS CODE IS PART OF QUAKE2XP ENGINE
THIS IS OPEN SOURCE SOFTWARE, IF YOU USE IT, 
JUST LEAVE IN THE AUTHOR'S CREDITS ORIGINAL CODE

WE CAN NOT JUST RENAME QUAKE2XP TO QUAKE2. VIDEO DRIVER 
OF NVIDIA OR AMD WILL SEE THE NAME FROM THE LIST AND
CUT OFF EXTENSION STRING TO CIRCUMVENT THE BUFFER 
OVERFLOW ERROR IN THE CONSOLE.
==============================================*/

#include <stdio.h>
#include <windows.h>

#define	MAX_TOKEN_CHARS		2048

char *Com_Parse(char **data_p) {
	static char	token[MAX_TOKEN_CHARS];
	static int	parseLine;
	int		c, len;
	char	*data;

	data = *data_p;
	len = 0;
	token[0] = 0;

	if (!data) {
		*data_p = NULL;
		return "";
	}

skipWhite:
	// skip whitespace
	while ((c = *data) <= ' ') {
		if (c == 0) {
			*data_p = NULL;
			return "";
		}

		data++;
	}

	// skip // comments
	if (c == '/' && data[1] == '/') {
		while (*data && *data != '\n')
			data++;

		goto skipWhite;
	}

	// handle quoted strings specially
	if (c == '\"') {
		data++;

		while (1) {
			c = *data++;

			if (c == '\"' || !c) {
				token[len] = 0;
				*data_p = data;
				return token;
			}

			if (len < MAX_TOKEN_CHARS) {
				token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do {
		if (len < MAX_TOKEN_CHARS) {
			token[len] = c;
			len++;
		}

		data++;

		c = *data;
	} while (c > 32);

	if (len == MAX_TOKEN_CHARS) {
		//	Com_Printf("Com_Parse: token length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
		len = 0;
	}

	token[len] = 0;
	*data_p = data;

	return token;
}
#define COM_Parse(p)	Com_Parse(p)

int Q_strncasecmp(const char *s1, const char *s2, int n) {
	int		c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point

		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);

	return 0;		// strings are equal
}

int Q_strcasecmp(const char *s1, const char *s2) {
	return Q_strncasecmp(s1, s2, 99999);
}

#define BUFF_SIZE	24
#define CMD_SIZE	512

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;

	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.lpReserved = NULL;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_SHOWNORMAL;
	StartupInfo.cbReserved2 = 0;
	StartupInfo.lpReserved2 = NULL;

	FILE *f;
	char *text, *token, cmd[CMD_SIZE];
	char *defaultString = "steamRun quake2xp\n";
	text = (char*)calloc(BUFF_SIZE, 1);

	fopen_s(&f, "steamLauncher.ini", "r");
	if (f) {

		fseek(f, 0, SEEK_END);
		int fSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		fgets(text, fSize, f);
		fclose(f);
	}else {		
		//create ini file
		fopen_s(&f, "steamLauncher.ini", "w");
		if (f){
			// add data
			fprintf(f, "steamRun quake2xp\n");
			fclose(f);

			memcpy(text, defaultString, strlen(defaultString));
		}
		else {
			MessageBox(NULL, "Can't create steamLauncher.ini!", "Steam Launcher Error", MB_OK);
			return 0;
		}
	}

	while (1) {
		token = COM_Parse(&text);

		if (!text) {
			break;
		}

		if (!Q_strcasecmp(token, "steamRun")) {
			token = COM_Parse(&text);
			strcpy_s(cmd, token);
			continue;
		}
	}

	strcat_s(cmd, " ");
	strcat_s(cmd, lpCmdLine);

	CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);


	return 0;
}