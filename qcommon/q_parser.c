/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "../game/q_shared.h"

void Com_Error (int code, const char *fmt, ...);


static const char *com_defaultPunctuationTable[] = {
	">>=",
	"<<=",
	"...",
	"##",
	"&&",
	"||",
	">=",
	"<=",
	"==",
	"!=",
	"*=",
	"/=",
	"%=",
	"+=",
	"-=",
	"++",
	"--",
	"&=",
	"|=",
	"^=",
	">>",
	"<<",
	"->",
	"::",
	".*",
	"*",
	"/",
	"%",
	"+",
	"-",
	"=",
	"&",
	"|",
	"^",
	"~",
	"!",
	">",
	"<",
	".",
	":",
	",",
	";",
	"?",
	"{",
	"}",
	"[",
	"]",
	"(",
	")",
	"#",
	"$",
	"\\",
	NULL
};

static void Parser_UpdateNumber (token_t *token) {
	token->integerValue = (int)atoi (token->text);
	token->unsignedValue = (unsigned)atoi (token->text);
	token->floatValue = (float)atof (token->text);
	token->doubleValue = (double)atof (token->text);
}

/*
===================
Parser_Reset

===================
*/
void Parser_Reset (parser_t *parser, const char *name, const char *text) {
	memset (parser, 0, sizeof(parser_t));
	Q_strncpyz (parser->name, name ? name : "noname", sizeof(parser->name));

	parser->line = 1;
	parser->text = text;
}

/*
===================
Parser_SetFlags

===================
*/
void Parser_SetFlags (parser_t *parser, parserFlags_t flags) {
	parser->flags = flags;
}

/*
===================
Parser_GetCurrentLine

===================
*/
int Parser_GetCurrentLine (parser_t *parser) {
	return parser->line;
}

/*
===================
Parser_Error

Prints the script name and line number in the message.
===================
*/
void Parser_Error (parser_t *parser, const char *msg, ...) {
	va_list		argptr;
	char		string[32768];

	if (parser->flags & PF_NOERRORS)
		return;

	va_start (argptr, msg);
	vsnprintf (string, sizeof(string), msg, argptr);
	va_end (argptr);

	Com_Error (ERR_DROP, "%s, line %i: %s", parser->name, parser->line, string);
}

/*
===================
Parser_Warning

===================
*/
void Parser_Warning (parser_t *parser, const char *msg, ...) {
	va_list		argptr;
	char		string[32768];

	if (parser->flags & PF_NOWARNINGS)
		return;

	va_start (argptr, msg);
	vsnprintf (string, sizeof(string), msg, argptr);
	va_end (argptr);

	Com_Printf ("%s, line %i: %s", parser->name, parser->line, string);
}

/*
===================
Parser_SkipWhiteSpace

===================
*/
static qboolean Parser_SkipWhiteSpace (parser_t *parser, qboolean allowLineBreaks) {
	qboolean	hasNewLines = qfalse;

	while (1) {
		while (*parser->text <= ' ') {
			if (!*parser->text)
				return qfalse;

			if (*parser->text == '\n') {
				parser->line++;
				hasNewLines = qtrue;
			}

			parser->text++;
		}

		if (hasNewLines && !allowLineBreaks)
			return qfalse;

		// skip double slash comments
		if (*parser->text == '/' && parser->text[1] == '/') {
			while (*parser->text && *parser->text != '\n')
				parser->text++;

			continue;
		}

		// skip /* */ comments
		if (*parser->text == '/' && parser->text[1] == '*') {
#ifdef _WIN32
			// some VC6 compiler round-around fucking shit...
			// do not remove!
			_asm nop
#endif

			while (*parser->text && (*parser->text != '*' || parser->text[1] != '/')) {
				if (*parser->text == '\n')
					parser->line++;

				parser->text++;
			}

			if (*parser->text)
				parser->text += 2;

			continue;
		}

		// a real token to parse
		break;
	}

	return qtrue;
}

/*
===================
Parser_ReadString

===================
*/
static qboolean Parser_ReadString (parser_t *parser, token_t *token) {
	token->type = TT_STRING;

	token->line = parser->line;
	token->linesCrossed = parser->line - parser->lastLine;

	token->length = 0;

	parser->text++;

	while (1) {
		if (!*parser->text) {
			Parser_Warning (parser, "missing trailing quote\n");
			return qfalse;
		}

		if (*parser->text == '\\' && parser->text[1] == '\"') {
			// allow quoted strings to use \" to indicate the quote character
			parser->text += 2;
			continue;
		}

		if (*parser->text == '\"') {
			parser->text++;
			break;		// end of the string
		}

		if (*parser->text == '\n') {
			parser->line++;
			parser->text++;
			continue;
		}

		if (token->length == MAX_TOKEN_CHARS - 1) {
			Parser_Warning (parser, "string length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
			return qfalse;
		}

		token->text[token->length++] = *parser->text++;
	}

	token->text[token->length] = 0;

	Parser_UpdateNumber (token);

	return qtrue;
}

/*
===================
Parser_ReadNumber

===================
*/
static qboolean Parser_ReadNumber (parser_t *parser, token_t *token) {
	char	c;

	token->type = TT_NUMBER;

	token->length = 0;

	token->line = parser->line;
	token->linesCrossed = parser->line - parser->lastLine;

	while (1) {
		//		if ((*parser->text < '0' || *parser->text > '9') && *parser->text != '.')
		//			break;

		c = *parser->text;

		if ((c < '0' || c > '9') &&
			(c != '-' || parser->text[1] < '0' || parser->text[1] > '9') &&
			(c != '.' || parser->text[1] < '0' || parser->text[1] > '9'))
			break;

		if (token->length == MAX_TOKEN_CHARS - 1) {
			Parser_Warning (parser, "number length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
			return qfalse;
		}

		token->text[token->length++] = *parser->text++;
	}

	// parse the exponent
	if (*parser->text == 'e' || *parser->text == 'E') {
		if (token->length == MAX_TOKEN_CHARS - 1) {
			Parser_Warning (parser, "number length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
			return qfalse;
		}

		token->text[token->length++] = *parser->text++;

		if (*parser->text == '-' || *parser->text == '+') {
			if (token->length == MAX_TOKEN_CHARS - 1) {
				Parser_Warning (parser, "number length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
				return qfalse;
			}

			token->text[token->length++] = *parser->text++;
		}

		while (1) {
			if (*parser->text < '0' || *parser->text > '9')
				break;

			if (token->length == MAX_TOKEN_CHARS - 1) {
				Parser_Warning (parser, "number length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
				return qfalse;
			}

			token->text[token->length++] = *parser->text++;
		}
	}

	token->text[token->length] = 0;

	Parser_UpdateNumber (token);

	return qtrue;
}

/*
===================
Parser_ReadWord

===================
*/
static qboolean Parser_ReadWord (parser_t *parser, token_t *token) {
	char	c;

	token->type = TT_STRING;

	token->length = 0;

	token->line = parser->line;
	token->linesCrossed = parser->line - parser->lastLine;

	while (1) {
		c = *parser->text;

		if (parser->flags & PF_ALLOWPATHNAMES) {
			if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '_' && c != '/' && c != '\\' && c != ':' && c != '.')
				break;
		}
		else {
			if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '_')
				break;
		}

		if (token->length == MAX_TOKEN_CHARS - 1) {
			Parser_Warning (parser, "word length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
			return qfalse;
		}

		token->text[token->length++] = *parser->text++;
	}

	token->text[token->length] = 0;

	Parser_UpdateNumber (token);

	return qtrue;
}

/*
===================
Parser_ReadPunctuation

===================
*/
static qboolean Parser_ReadPunctuation (parser_t *parser, token_t *token) {
	const char	**punc;
	int			i;

	// check for multi-character punctuation token
	for (punc = com_defaultPunctuationTable; *punc; punc++) {
		for (i = 0; (*punc)[i] && parser->text[i]; i++) {
			if (parser->text[i] != (*punc)[i])
				break;
		}

		if ((*punc)[i])		// exceeded the rest of space or not equal 
			continue;

		parser->text += i;

		// a valid multi-character punctuation
		if (i > MAX_TOKEN_CHARS - 1) {
			Parser_Warning (parser, "punctuation length exceeds MAX_TOKEN_CHARS ( %i )\n", MAX_TOKEN_CHARS);
			return qfalse;
		}

		token->type = TT_PUNCTUATION;

		token->length = i;

		token->line = parser->line;
		token->linesCrossed = parser->line - parser->lastLine;

		Q_memcpy (token->text, *punc, token->length);
		token->text[token->length] = 0;

		Parser_UpdateNumber (token);

		return qtrue;
	}

	return qfalse;
}

/*
==============
Parser_GetTokenExt

Parse a token out of a string.
Will never return NULL, just empty strings.
An empty string will only be returned at end of file.
==============
*/
static qboolean Parser_GetTokenExt (parser_t *parser, token_t *token, qboolean allowLineBreaks) {
	char	c;

	if (parser->ungetToken) {
		parser->ungetToken = qfalse;
		Q_memcpy (token, &parser->token, sizeof(token_t));
		return qtrue;
	}

	// make sure incoming data is valid
	if (!parser->text)
		return qfalse;

	parser->lastLine = parser->line;
	parser->lastText = parser->text;

	memset (token, 0, sizeof(token_t));

	// skip any leading whitespace
	if (!Parser_SkipWhiteSpace (parser, allowLineBreaks))
		return qfalse;

	c = *parser->text;

	// handle quoted strings
	if (c == '\"') {
		if (Parser_ReadString (parser, token))
			return qtrue;
	}

	// check for a number
	// is this parsing of negative numbers going to cause expression problems
	else if ((c >= '0' && c <= '9') ||
		(c == '-' && parser->text[1] >= '0' && parser->text[1] <= '9') ||
		(c == '.' && parser->text[1] >= '0' && parser->text[1] <= '9')) {
		if (Parser_ReadNumber (parser, token))
			return qtrue;
	}

	// check for a regular word
	// we still allow forward and back slashes in name tokens for pathnames
	// and also colons for drive letters
	else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
		if (Parser_ReadWord (parser, token))
			return qtrue;
	}

	// check for punctuation
	else if (Parser_ReadPunctuation (parser, token))
		return qtrue;

	Parser_Warning (parser, "Parser_GetToken: couldn't read token\n");

	memset (token, 0, sizeof(token_t));

	return qfalse;
}

/*
===================
Parser_GetToken

===================
*/
qboolean Parser_GetToken (parser_t *parser, token_t *token) {
	if (!Parser_GetTokenExt (parser, token, qtrue))
		return qfalse;

	return qtrue;
}

/*
===================
Parser_GetTokenOnLine

===================
*/
qboolean Parser_GetTokenOnLine (parser_t *parser, token_t *token) {
	if (!Parser_GetTokenExt (parser, token, qfalse))
		return qfalse;

	return qtrue;
}

/*
====================
Parser_GetRestOfLine

====================
*/
qboolean Parser_GetRestOfLine (parser_t *parser, token_t *token) {
	token_t	tok;

	token->length = 0;

	token->line = parser->line;
	token->linesCrossed = parser->line - parser->lastLine;

	while (1) {
		if (!Parser_GetTokenOnLine (parser, &tok)) {
			if (!token->length)
				return qfalse;	// the rest of the line is empty

			break;
		}

		if (!token->length)
			Q_strncatz (token->text, sizeof(token->text), " ");

		Q_strncatz (token->text, sizeof(token->text), tok.text);

		token->length += tok.length;
	}

	return qtrue;
}

/*
===================
Parser_UngetToken

Calling this will make the next Parser_GetToken return
the current token instead of advancing the pointer.
===================
*/
void Parser_UngetToken (parser_t *parser, token_t *token) {
	parser->ungetToken = qtrue;
	Q_memcpy (&parser->token, token, sizeof(token_t));
}

/*
==================
Parser_CheckToken

==================
*/
qboolean Parser_CheckToken (parser_t *parser, const char *match, qboolean warning) {
	token_t		token;

	if (!Parser_GetToken (parser, &token))
		return qfalse;

	if (Q_stricmp (token.text, (char*)match)) {
		if (warning)
			Parser_Warning (parser, "expected '%s', found '%s'\n", match, token.text);
		else
			Parser_Error (parser, "expected '%s', found '%s'\n", match, token.text);

		return qfalse;
	}

	return qtrue;
}

/*
==================
Parser_CheckTokenType

==================
*/
qboolean Parser_CheckTokenType (parser_t *parser, tokenType_t type, qboolean warning) {
	token_t		token;

	if (!Parser_GetToken (parser, &token))
		return qfalse;

	if (token.type != type) {
		if (warning) {
			switch (type) {
				case TT_STRING:
					Parser_Warning (parser, "expected string, found '%s'", token.text);
					break;
				case TT_NUMBER:
					Parser_Warning (parser, "expected number, found '%s'", token.text);
					break;
				case TT_PUNCTUATION:
					Parser_Warning (parser, "expected punctuation, found '%s'", token.text);
					break;
				case TT_BAD:
					Parser_Warning(parser, "expected punctuation, found '%s'", token.text);
					break;
			}
		}
		else {
			switch (type) {
				case TT_STRING:
					Parser_Error (parser, "expected string, found '%s'", token.text);
					break;
				case TT_NUMBER:
					Parser_Error (parser, "expected number, found '%s'", token.text);
					break;
				case TT_PUNCTUATION:
					Parser_Error (parser, "expected punctuation, found '%s'", token.text);
					break;
				case TT_BAD:
					Parser_Error(parser, "expected punctuation, found '%s'", token.text);
					break;
			}
		}

		return qfalse;
	}

	return qtrue;
}

/*
=================
Parser_SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean Parser_SkipBracedSection (parser_t *parser, int depth) {
	token_t	token;

	do {
		if (!Parser_GetToken (parser, &token))
			return qfalse;

		if (token.type == TT_PUNCTUATION) {
			if (!Q_stricmp (token.text, "{"))
				depth++;
			else if (!Q_stricmp (token.text, "}"))
				depth--;
		}
	} while (depth);

	return qtrue;
}

/*
=================
Parser_SkipRestOfLine

=================
*/
void Parser_SkipRestOfLine (parser_t *parser) {
	const char	*p;
	int		c;

	p = parser->text;

	while ((c = *p++) != 0) {
		if (c == '\n') {
			parser->line++;
			break;
		}
	}

	parser->text = p;
}

//===============================================================
