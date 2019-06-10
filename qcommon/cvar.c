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
// cvar.c -- dynamic variable tracking

#include "qcommon.h"
cvar_t *cvar_vars;

/*
============
Cvar_InfoValidate
============
*/
static qboolean Cvar_InfoValidate (char *s) {
	if (strstr (s, "\\"))
		return qfalse;
	if (strstr (s, "\""))
		return qfalse;
	if (strstr (s, ";"))
		return qfalse;
	return qtrue;
}

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (char *var_name) {
	cvar_t *var;

	for (var = cvar_vars; var; var = var->next)
	if (!strcmp (var_name, var->name))
		return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (char *var_name) {
	cvar_t *var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atof (var->string);
}

int Cvar_VariableInteger(char *var_name) {
	cvar_t *var;

	var = Cvar_FindVar(var_name);
	if (!var)
		return 0;
	return atoi(var->string);
}

/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (char *var_name) {
	cvar_t *var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return "";
	return var->string;
}


/*
============
Cvar_Get

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
cvar_t *Cvar_Get (char *var_name, char *var_value, int flags) {
	cvar_t *var;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO)) {
		if (!Cvar_InfoValidate (var_name)) {
			Com_Printf ("invalid info cvar name\n");
			return NULL;
		}
	}

	var = Cvar_FindVar (var_name);
	if (var) {
		var->flags |= flags;
		return var;
	}

	if (!var_value)
		return NULL;

	if (flags & (CVAR_USERINFO | CVAR_SERVERINFO)) {
		if (!Cvar_InfoValidate (var_value)) {
			Com_Printf ("invalid info cvar value\n");
			return NULL;
		}
	}

	var = (cvar_t*)Z_Malloc (sizeof(*var));

	var->name		= CopyString (var_name);
	var->string		= CopyString (var_value);
	var->modified	= qtrue;
	var->value		= atof (var->string);
	var->integer	= atoi(var->string);
	var->qbool		= (var->integer != 0);

	// link the variable in
	var->next = cvar_vars;
	cvar_vars = var;

	var->flags = flags;
	var->help = NULL;

	return var;
}

/*
============
Cvar_Set2
============
*/
cvar_t *Cvar_Set2 (char *var_name, char *value, qboolean force) {
	cvar_t *var;

	var = Cvar_FindVar (var_name);
	if (!var) {					// create it
		return Cvar_Get (var_name, value, 0);
	}

	if ((var->flags & CVAR_DEVELOPER) && developer->integer != ExtraDevMode) {
		Com_Printf("" S_COLOR_YELLOW "%s " S_COLOR_MAGENTA "is developer protected\n", var_name);
		return var;
	}

	if (var->flags & (CVAR_USERINFO | CVAR_SERVERINFO)) {
		if (!Cvar_InfoValidate (value)) {
			Com_Printf ("invalid info cvar value\n");
			return var;
		}
	}

	if (!force) {
		if (var->flags & CVAR_NOSET) {
			Com_Printf ("%s is write protected.\n", var_name);
			return var;
		}

		if (var->flags & CVAR_LATCH) {
			if (var->latched_string) {
				if (strcmp (value, var->latched_string) == 0)
					return var;
				Z_Free (var->latched_string);
			}
			else {
				if (strcmp (value, var->string) == 0)
					return var;
			}

			if (Com_ServerState ()) {
				Com_Printf ("%s will be changed for next game.\n",
					var_name);
				var->latched_string = CopyString (value);
			}
			else {
				var->string = CopyString (value);
				var->value = atof (var->string);
				if (!strcmp (var->name, "game")) {
					FS_SetGamedir (var->string);
					FS_ExecAutoexec ();
				}
			}
			return var;
		}
	}
	else {
		if (var->latched_string) {
			Z_Free (var->latched_string);
			var->latched_string = NULL;
		}
	}

	if (!strcmp (value, var->string))
		return var;				// not changed

	var->modified = qtrue;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = qtrue;	// transmit at next oportunity

	Z_Free (var->string);		// free the old value string

	var->string		= CopyString (value);
	var->value		= atof (var->string);
	var->integer	= atoi(var->string);
	var->qbool		= (var->integer != 0);
	return var;
}

/*
============
Cvar_ForceSet
============
*/
cvar_t *Cvar_ForceSet (char *var_name, char *value) {
	return Cvar_Set2 (var_name, value, qtrue);
}

/*
============
Cvar_Set
============
*/
cvar_t *Cvar_Set (char *var_name, char *value) {
	return Cvar_Set2 (var_name, value, qfalse);
}

/*
============
Cvar_FullSet
============
*/
cvar_t *Cvar_FullSet (char *var_name, char *value, int flags) {
	cvar_t *var;

	var = Cvar_FindVar (var_name);
	if (!var) {					// create it
		return Cvar_Get (var_name, value, flags);
	}

	var->modified = qtrue;

	if (var->flags & CVAR_USERINFO)
		userinfo_modified = qtrue;	// transmit at next oportunity

	Z_Free (var->string);		// free the old value string

	var->string		= CopyString (value);
	var->value		= atof (var->string);
	var->integer	= atoi(var->string);
	var->qbool		= (var->integer != 0);
	var->flags		= flags;

	return var;
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (char *var_name, float value) {
	char val[32];

	if (value == (int)value)
		Com_sprintf (val, sizeof(val), "%i", (int)value);
	else
		Com_sprintf (val, sizeof(val), "%f", value);
	Cvar_Set (var_name, val);
}

void Cvar_SetInteger(char *var_name, int value) {
	char val[32];

	Com_sprintf(val, sizeof(val), "%i", value);
	Cvar_Set(var_name, val);
}

/*
============
Cvar_ForceSetValue
============
*/
void Cvar_ForceSetValue (char *var_name, float value) {
	char val[32];

	if (value == (int)value)
		Com_sprintf (val, sizeof(val), "%i", (int)value);
	else
		Com_sprintf (val, sizeof(val), "%f", value);
	Cvar_ForceSet (var_name, val);
}



/*
============
Cvar_GetLatchedVars

Any variables with latched values will now be updated
============
*/
void Cvar_GetLatchedVars (void) {
	cvar_t *var;

	for (var = cvar_vars; var; var = var->next) {
		
		if (!var->latched_string)
			continue;

		Z_Free (var->string);			
		var->string			= var->latched_string;
		var->latched_string = NULL;
		var->value			= atof (var->string);
		var->integer		= atoi(var->string);
		var->qbool			= (var->integer != 0);

		if (!strcmp (var->name, "game")) {
			FS_SetGamedir (var->string);
			FS_ExecAutoexec ();
		}
	}
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void) {
	cvar_t *v;

	// check variables
	v = Cvar_FindVar (Cmd_Argv (0));
	if (!v)
		return qfalse;

	// perform a variable print or set
	if (Cmd_Argc () == 1) {
		Com_Printf ("\"%s\" is \"%s\"", v->name, v->string);

		if (v->flags & CVAR_ARCHIVE)
			Com_Printf (""S_COLOR_YELLOW" ARCHIVE");

		if (v->flags & CVAR_USERINFO)
			Com_Printf (" USERINFO");

		if (v->flags & CVAR_SERVERINFO)
			Com_Printf (""S_COLOR_BLUE" SERVERINFO");

		if (v->flags & CVAR_NOSET)
			Com_Printf (""S_COLOR_RED" NOSET");

		if (v->flags & CVAR_LATCH)
			Com_Printf (""S_COLOR_CYAN" LATCH");
		Com_Printf ("\n");

		Com_Printf ("["S_COLOR_YELLOW"%s"S_COLOR_WHITE"]\n", v->help);
		return qtrue;
	}

	Cvar_Set (v->name, Cmd_Argv (1));
	return qtrue;
}


/*
============
Cvar_Set_f

Allows setting and defining of arbitrary cvars from console
============
*/
void Cvar_Set_f (void) {
	int c;
	int flags;

	c = Cmd_Argc ();
	if (c != 3 && c != 4) {
		Com_Printf ("usage: set <variable> <value> [u / s]\n");
		return;
	}

	if (c == 4) {
		if (!strcmp (Cmd_Argv (3), "u"))
			flags = CVAR_USERINFO;
		else if (!strcmp (Cmd_Argv (3), "s"))
			flags = CVAR_SERVERINFO;
		else {
			Com_Printf ("flags can only be 'u' or 's'\n");
			return;
		}
		Cvar_FullSet (Cmd_Argv (1), Cmd_Argv (2), flags);
	}
	else
		Cvar_Set (Cmd_Argv (1), Cmd_Argv (2));
}


// cvar sorting from r1q2
int cvarsort(const void *_a, const void *_b) {
	const cvar_t	*a = (const cvar_t *)_a;
	const cvar_t	*b = (const cvar_t *)_b;
	return strcmp(a->name, b->name);
}

/*
============
Cvar_WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to qtrue.
============
*/
void Cvar_WriteVariables (char *path) {
	cvar_t	*var, *sortedList, *cvar;
	char	buffer[1024];
	FILE	*f;
	int		i, num;

	for (var = cvar_vars, num = 0; var; var = var->next, num++) {}
		sortedList = (cvar_t *)Z_Malloc(num * sizeof(cvar_t));

	if (sortedList) {
		for (var = cvar_vars, i = 0; var; var = var->next, i++)
			sortedList[i] = *var;

		qsort(sortedList, num, sizeof(sortedList[0]), cvarsort);
	}

	f = fopen (path, "a");
	for (cvar = cvar_vars, i = 0; cvar; cvar = cvar->next, i++) {
		if (sortedList)
			var = &sortedList[i];
		else
			var = cvar;

		if (var->flags & CVAR_ARCHIVE) {
			Com_sprintf (buffer, sizeof(buffer), "set %s \"%s\"\n",
				var->name, var->string);
			fprintf (f, "%s", buffer);
		}
	}
	fclose (f);

	if (sortedList)
		Z_Free(sortedList);
}

/*
============
Cvar_List_f

============
*/


void Cvar_List_f (void) {
	cvar_t		*var, *cvar, *sortedList;
	int			i, num;
	char		*hlp;
	qboolean	help = qfalse;

	for (var = cvar_vars, num = 0; var; var = var->next, num++) {}
		sortedList = (cvar_t *)Z_Malloc(num * sizeof(cvar_t));
		
	if (sortedList) {
		for (var = cvar_vars, i = 0; var; var = var->next, i++)
			sortedList[i] = *var;
		qsort (sortedList, num, sizeof(sortedList[0]), cvarsort);
	}

	if (Cmd_Argc () == 2) {
		hlp = Cmd_Argv (1);
		if (!Q_strcasecmp (hlp, "?") || !Q_strcasecmp (hlp, "h") || !Q_strcasecmp (hlp, "help"))
			help = qtrue;
	}

	for (cvar = cvar_vars, i = 0; cvar; cvar = cvar->next, i++) {
		if (sortedList)
			var = &sortedList[i];
		else
			var = cvar;

		if (var->flags & CVAR_ARCHIVE)
			Com_Printf (""S_COLOR_YELLOW"A");
		else
			Com_Printf (" ");
		if (var->flags & CVAR_USERINFO)
			Com_Printf ("U");
		else
			Com_Printf (" ");
		if (var->flags & CVAR_SERVERINFO)
			Com_Printf (""S_COLOR_BLUE"S");
		else
			Com_Printf (" ");

		if (var->flags & CVAR_DEVELOPER)
			Com_Printf(""S_COLOR_RED"D");
		else
			Com_Printf(" ");

		if (var->flags & CVAR_NOSET)
			Com_Printf (""S_COLOR_RED"-");
		else if (var->flags & CVAR_LATCH)
			Com_Printf (""S_COLOR_CYAN"L");
		else
			Com_Printf (" ");
		Com_Printf (" %s \"%s\"", var->name, var->string);

		if (help || var->help)
			Com_Printf (" ["S_COLOR_YELLOW"%s"S_COLOR_WHITE"]", var->help);

		Com_Printf ("\n");
	}

	Com_Printf ("%i cvars\n", num);

	if (sortedList)
		Z_Free (sortedList);
}

qboolean userinfo_modified;


char *Cvar_BitInfo (int bit) {
	static char info[MAX_INFO_STRING];
	cvar_t *var;

	info[0] = 0;

	for (var = cvar_vars; var; var = var->next) {
		if (var->flags & bit)
			Info_SetValueForKey (info, var->name, var->string);
	}
	return info;
}

// returns an info string containing all the CVAR_USERINFO cvars
char *Cvar_Userinfo (void) {
	return Cvar_BitInfo (CVAR_USERINFO);
}

// returns an info string containing all the CVAR_SERVERINFO cvars
char *Cvar_Serverinfo (void) {
	return Cvar_BitInfo (CVAR_SERVERINFO);
}

// r1ch cvar help system
void Cvar_Help_f (void) {
	cvar_t	*var;
	char	str[4096];

	if (Cmd_Argc () != 2) {
		Com_Printf ("Usage: cvarhelp <cvarName>\n");
		return;
	}

	var = Cvar_FindVar (Cmd_Argv (1));
	if (!var) {
		Com_Printf ("Cvar %s not found.\n", Cmd_Argv (1));
		return;
	}

	if (!var->help) {
		Com_Printf ("No help available for %s.\n", Cmd_Argv (1));
		return;
	}

	Com_sprintf (str, sizeof(str), "%s: %s\n", var->name, var->help);
	Com_Printf (str, var->string);
}

/*
============
Cvar_Init

Reads in all archived cvars
============
*/
void Cvar_Init (void) {
	Cmd_AddCommand ("set", Cvar_Set_f);
	Cmd_AddCommand ("cvarlist", Cvar_List_f);
	Cmd_AddCommand ("cvarhelp", Cvar_Help_f);	// r1ch

}
