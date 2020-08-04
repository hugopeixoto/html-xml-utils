%option 8bit nodefault noyywrap nounput
/* %option yylineno */

%{
/*
 * Copyright © 1997-2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 1997
 **/
#include "config.h"
#include <assert.h>

#if HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif
#if !HAVE_STRDUP
#  include "strdup.e"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"
#include "heap.e"
#include "html.h"
#include "html.e"
#include "errexit.e"


EXPORT extern FILE *yyin;
string yyin_name = NULL;

string cur_cdata_element = NULL;

typedef struct _Stack {
  YY_BUFFER_STATE buf;
  FILE *f;
  string name;
  struct _Stack *next;
} *Stack;

static Stack stack = NULL;


/* set_yyin -- routine to set yyin and store its file name */
EXPORT void set_yyin(FILE *f, const conststring name)
{
  yyin = f;
  free(yyin_name);
  yyin_name = newstring(name);
}

/* get_yyin_name -- return the name of the current input, if known */
EXPORT conststring get_yyin_name(void)
{
  return yyin_name;
}

/* include_file -- stack current file and switch to another one */
EXPORT void include_file(FILE *f, const conststring name)
{
  Stack h;

  new(h);
  h->buf = YY_CURRENT_BUFFER;
  h->f = f;
  h->name = yyin_name;
  h->next = stack;
  stack = h;
  yyin_name = newstring(name);
  yy_switch_to_buffer(yy_create_buffer(f, YY_BUF_SIZE));
}

/* pop_file -- back to previous input file */
static bool pop_file(void)
{
  Stack h;

  if (!stack) {
    return false;
  } else {
    h = stack;
    yy_delete_buffer(YY_CURRENT_BUFFER);
    fclose(h->f);
    free(yyin_name);
    yyin_name = h->name;
    yy_switch_to_buffer(h->buf);
    stack = h->next;
    dispose(h);
    return true;
  }
}

/* esc -- remove outer quotes, escape ", remove \n, return malloc'ed string */
static string esc(string s)
{
  int i, j;
  string u;

  /* Find new length */
  for (i = 0, j = 1; s[j] != s[0]; i++, j++) {
    if (s[j] == '"' || s[j] == '<' || s[j] == '>') i+= 4;
  }
  /* Copy and expand */
  u = malloc(i + 1);
  if (!u) errexit("Out of memory\n");
  for (i = 0, j = 1; s[j] != s[0]; i++, j++) {
    if (s[j] == '"')  {strcpy(u + i, "&#34;"); i += 4;}
    else if (s[j] == '<')  {strcpy(u + i, "&#60;"); i += 4;}
    else if (s[j] == '>')  {strcpy(u + i, "&#62;"); i += 4;}
    else if (s[j] == '\n') u[i] = ' ';		/* \n */
    else if (s[j] == '\r' && s[j+1] == '\n') {u[i] = ' '; j++;}	/* \r\n */
    else if (s[j] == '\r') {u[i] = ' ';}	/* \r */
    else u[i] = s[j];
  }
  u[i] = '\0';
  return u;
}
 
#ifndef HAVE_STRNDUP

/* strndup -- allocate a string, copy n characters into it and add \0 */
static string strndup(const string s, size_t n)
{
  string t = malloc(n + 1);
  if (!t) errexit("Out of memory\n");
  strncpy(t, s, n);
  t[n] = '\0';
  return t;
}

#else
# ifndef strndup

/* We know strndup() exists (HAVE_STRNDUP) and it is not defined as a
macro (!strndup), but older versions of string.h do not provide the
declaration, so let's declare it here to be sure. */

extern char *strndup(const char *s, size_t n);

# endif
#endif

/* lns -- count newlines */
static void lns(const string t)
{
  string s = t;

  while (*s) {
    if (*s == '\n') lineno++;
    else if (*s != '\r') ;
    else if (*(s+1) == '\n') {lineno++; s++;}
    else lineno++;
    s++;
  }
}

%}

/* thing is rather too permissive, but it will accept <img src=/path>... */

nondelim	[^ \t\r\n\f"'<>]
name		(\{[^} \t\r\n\f]*\})?[a-zA-Z0-9:._\200-\377-]+
thing		{nondelim}+
comment		"<!--"([^-]|-[^-]|--[^>])*"-->"
data		[^<\r\n]+
doctype		<![Dd][Oo][Cc][Tt][Yy][Pp][Ee][ \t\r\n\f]
nl		\n|\r\n|\r
cdata		<!\[[Cc][Dd][Aa][Tt][Aa]\[([^]]|\][^]]|\]\][^>])*\]\]>

%s MARKUP VALUE DECL INIT CDATA

%%


<INITIAL>\357\273\277		{BEGIN(INIT); /* Byte Order Mark is ignored */}

<INITIAL,INIT>"<"{name}		{BEGIN(MARKUP); yylval.s=strdup(yytext+1); return START;}
<INITIAL,INIT>"</"({name})?	{BEGIN(MARKUP); yylval.s=strdup(yytext+2); return END;}
<INITIAL,INIT>{data}		{yylval.s=strdup(yytext); return TEXT;}
<INITIAL,INIT>{cdata}		{yylval.s=strdup(yytext); lns(yytext); return TEXT;}
<INITIAL,INIT>{nl}		{yylval.s=strdup(yytext); lineno++; return TEXT;}
<INITIAL,INIT>{comment}	{yylval.s=strndup(yytext+4,yyleng-7); lns(yytext); return COMMENT;}
<INITIAL,INIT>{doctype}	{BEGIN(DECL); lns(yytext+9); return DOCTYPE;}
<INITIAL,INIT>"<?"[^>]*">"	{yylval.s=strndup(yytext+2,yyleng-3); lns(yytext); return PROCINS;}
<INITIAL,INIT>"<"		{yylval.s=strdup("&lt;"); return TEXT;}

<MARKUP>{name}		{yylval.s = strdup(yytext); return NAME;}
<MARKUP>"="		{BEGIN(VALUE); return '=';}
<MARKUP>[ \t\f]+	{; /* skip */}
<MARKUP>{nl}		{lineno++; /* skip */}
<MARKUP>">"		{BEGIN(INIT); return '>';}
<MARKUP>"/>"		{BEGIN(INIT); return EMPTYEND;}
<MARKUP>"<"		{BEGIN(INIT); yyless(0); return '>'; /* Implicit ">" */} 

<VALUE>[ \t\f]+		{; /* skip */}
<VALUE>{nl}		{lineno++; /* skip */}
<VALUE>{thing}		{BEGIN(MARKUP); yylval.s=strdup(yytext); return NAME;}
<VALUE>\"[^"]*\"	|
<VALUE>\'[^']*\'	{BEGIN(MARKUP); yylval.s=esc(yytext); lns(yytext); return STRING;}

<DECL>{name}		{yylval.s = strdup(yytext); return NAME;}
<DECL>[ \t\f]+		{; /* skip */}
<DECL>{nl}		{lineno++; /* skip */}
<DECL>\"[^"]*\"		|
<DECL>\'[^']*\'		{lns(yytext); yylval.s = esc(yytext); return STRING;}
<DECL>">"		{BEGIN(INIT); return '>';}

<CDATA>([^<]|\<[^/]|\<\/[^{a-z:._-])* {lns(yytext); yylval.s = strdup(yytext); return TEXT;}
<CDATA>"</"{name}	{lns(yytext);
			 if (strcasecmp(yytext+2, cur_cdata_element) == 0) {
			   BEGIN(MARKUP);
			   yylval.s = strdup(yytext+2);
			   return END;
			 } else {
			   yylval.s = strdup(yytext);
			   return TEXT;
			 }
			}

.			{return *yytext; /* illegal char, in fact */}

<<EOF>>			{if (pop_file()) return ENDINCL; else yyterminate();}

%%

/* set_cdata_element -- set parsing rule for an element with CDATA content */
EXPORT void set_cdata_element(const conststring e)
{
  dispose(cur_cdata_element);
  cur_cdata_element = newstring(e);
  BEGIN(CDATA);
}

/*
 * Local variables:
 * mode: indented-text
 * End:
 */
