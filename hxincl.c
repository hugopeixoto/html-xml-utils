/*
 * incl - expand included files
 *
 * Searches for <!--include "file"--> and expands the referenced file
 * in place. File may be a URL. Works recursively. Other accepted
 * syntaxes:
 *
 * <!--include "file"-->
 * <!--include 'file'-->
 * <!--include file-->
 * <!--begin-include "file"-->...<!--end-include-->
 * <!--begin-include 'file'-->...<!--end-include-->
 * <!--begin-include file-->...<!--end-include-->
 *
 * If there are no quotes, the file name may not include whitespace.
 *
 * Copyright © 1994-2012 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos
 * Created: 2 Dec 1998
 * Version: $Id: hxincl.c,v 1.14 2017/11/24 09:50:25 bbos Exp $
 *
 **/
#include "config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <ctype.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
# ifndef HAVE_STRSTR
#  include "strstr.e"
# endif
#endif
#include <stdlib.h>
#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"
#include "errexit.e"
#include "html.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "heap.e"
#include "url.e"

#define INCLUDE "include"
#define BEGIN "begin-include"
#define END "end-include"

typedef struct _stack {
  bool skipping;
  struct _stack *next;
} *stack;

typedef enum {KNone, KIncl, KBegin, KEnd} Key;

static bool do_xml = false;
static bool final = false;	/* Final means to remove the comments */
static bool has_error = false;
static stack skipping = NULL;
static Dictionary substitutions = NULL;
static string target = NULL;
static bool warn_missing = true;


/* push -- push a skipping state on the stack */
static void push(stack *skipping, bool s)
{
  stack h;

  new(h);
  h->next = *skipping;
  h->skipping = s;
  *skipping = h;
}

/* pop -- pop a skipping state off the stack */
static void pop(stack *skipping)
{
  stack h;

  assert(*skipping);
  h = *skipping;
  *skipping = (*skipping)->next;
  dispose(h);
}

/* top -- return value of top of skipping stack */
static bool top(stack skipping)
{
  assert(skipping);
  return skipping->skipping;
}

/* word_to_key -- check whether word s is one of the recognized keywords */
static Key word_to_key(const string s, int len)
{
  if (len == sizeof(END) - 1 && strncmp(s, END, len)== 0) return KEnd;
  if (len == sizeof(INCLUDE) - 1 && strncmp(s, INCLUDE, len)== 0) return KIncl;
  if (len == sizeof(BEGIN) - 1 && strncmp(s, BEGIN, len)== 0) return KBegin;
  return KNone;
}

/* add_substitution -- add a file name substitution to the dictionary */
static void add_substitution(const string assignment)
{
  string s;

  /* To do: handle file names containing '='. Add escapes? */
  if (!substitutions) substitutions = dict_create(10);
  if (!substitutions) errexit("Out of memory.\n");
  if (!(s = strchr(assignment, '='))) errexit("No '=' found in option -s\n");
  *s = '\0';
  if (!dict_add(substitutions, assignment, s + 1)) errexit("Out of memory?\n");
  *s = '=';
}

/* expand_vars -- look for and expand %variables% in s */
static string expand_vars(const conststring s)
{
  string h, k, sub2, var, result = NULL;
  conststring sub;

  k = strdup(s);
  if (!k) err(1, NULL);

  /* To do: avoid infinite loops */
  for (h = k; h;) {
    /* Append the text leading up to next '%' */
    result = strapp(&result, strsep(&h, "%"), NULL);
    /* If there is any text left, get the text until the next '%' into var */
    if ((var = strsep(&h, "%"))) {
      if (!h) {			/* No matching '%' found */
	result = strapp(&result, "%", var, NULL);
      } else if (*var == '\0') { /* Treat "%%" as a single "%" */
	result = strapp(&result, "%", NULL);
      } else if (substitutions && (sub = dict_find(substitutions, var))) {
	sub2 = expand_vars(sub);
	result = strapp(&result, sub2, NULL);
	free(sub2);
      } else {			/* Undefined %var% */
	result = strapp(&result, "%", var, "%", NULL);
      }
    }
  }
  free(k);
  return result;
}

/* --------------- implements interface api.h -------------------------- */

/* handle_error -- called when a parse error occurred */
void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
  has_error = true;
}

/* start -- called before the first event is reported */
void* start(void)
{
  push(&skipping, false);			/* Start by not skipping */
  return NULL;
}
  
/* end -- called after the last event is reported */
void end(void *clientdata)
{
  assert(clientdata == NULL);
  assert(top(skipping) == false);
  /* skip */
}

/* handle_comment -- called after a comment is parsed */
void handle_comment(void *clientdata, string commenttext)
{
  /* A push() occurs at <!--begin-include...--> and at include_file() */
  /* A pop() occurs at <!--end-include...--> and at ENDINCL */
  int i, j, status;
  conststring s, url;
  FILE *f;
  Key key;

  i = strspn(commenttext, " \t\n\r\f");		/* Skip whitespace */
  j = strcspn(commenttext + i, " \t\n\r\f");	/* First word */
  key = word_to_key(commenttext + i, j);
  
  if (key == KEnd) {				/* <!--end-include...--> */

    /* Don't print anything, just pop a level */
    pop(&skipping);

  } else if (top(skipping)) {			/* Are we already skipping? */

    /* Don't print anything; push a level if this is a begin-include */
    if (key == KBegin) push(&skipping, true);

  } else if (key == KNone) {			/* Unrecognized comment? */

    /* Print the comment verbatim */
    if (!target) printf("<!--%s-->", commenttext);

  } else {					/* include or begin-include */

    /* Push a level if this is a begin-include */
    if (key == KBegin) push(&skipping, true);

    /* Find start of file name */
    i += j;
    i += strspn(commenttext + i, " \t\n\r\f");	/* Skip whitespace */

    /* Accept either "...", '...', or any string without spaces */
    if (commenttext[i] == '"') {
      j = strcspn(commenttext + i + 1, "\"");
      url = newnstring(commenttext + i + 1, j);
    } else if (commenttext[i] == '\'') {
      j = strcspn(commenttext + (++i), "'");
      url = newnstring(commenttext + i + 1, j);
    } else {
      j = strcspn(commenttext + i, " \t\n\r\f");
      url = newnstring(commenttext + i, j);
    }

    /* If we have a substitution for it, use that instead */
    if (substitutions && (s = dict_find(substitutions, url))) url = s;

    /* Expand any %variables% in the url */
    url = expand_vars(url);

    /* Get the file and recursively parse it */
    assert(get_yyin_name());
    s = URL_s_absolutize(get_yyin_name(), url);
    if (target) printf(" \\\n %s", s); /* To do: escape spaces in s */
    if (!(f = fopenurl(s, "r", &status))) {
      if (!target || warn_missing) perror(url);
    } else if (status != 200) {
      if (!target || warn_missing)
	fprintf(stderr, "%s : %s\n", url, http_strerror(status));
    } else {
      if (!final && !target) printf("<!--%s %s-->", BEGIN, commenttext + i);
      push(&skipping, false);
      include_file(f, s);
    }
    dispose(url);
    dispose(s);
  }

  free(commenttext);
}

/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, string text)
{
  if (top(skipping) == false && !target) printf("%s", text);
  free(text);
}

/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, string gi, string fpi, string url)
{
  if (!target) {
    printf("<!DOCTYPE %s", gi);
    if (fpi) printf(" PUBLIC \"%s\"", fpi);
    if (url) printf(" %s\"%s\"", fpi ? "" : "SYSTEM ", url);
    printf(">");
  }
  free(gi);
  free(fpi);
  free(url);
}

/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  if (top(skipping) == false && !target) printf("<?%s>", pi_text);
  free(pi_text);
}

/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  pairlist p;

  if (top(skipping) == false && !target) {
    printf("<%s", name);
    for (p = attribs; p; p = p->next) {
      if (p->value != NULL) printf(" %s=\"%s\"", p->name, p->value);
      else if (do_xml) printf(" %s=\"%s\"", p->name, p->name);
      else printf(" %s", p->name);
    }
    printf(">");
  }
  free(name);
  pairlist_delete(attribs);
}

/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  pairlist p;

  if (top(skipping) == false && !target) {
    printf("<%s", name);
    for (p = attribs; p; p = p->next) {
      if (p->value != NULL) printf(" %s=\"%s\"", p->name, p->value);
      else if (do_xml) printf(" %s=\"%s\"", p->name, p->name);
      else printf(" %s", p->name);
    }
    printf(do_xml ? " />" : ">");
  }
  free(name);
  pairlist_delete(attribs);
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  if (top(skipping) == false && !target) printf("</%s>", name);
  free(name);
}

/* handle_endincl -- called after the end of an included file is reached */
void handle_endincl(void *clientdata)
{
  pop(&skipping);

  /* Mark the end of the inclusion */
  if (!final && !target) printf("<!--%s-->", END);
}

/* --------------------------------------------------------------------- */

/* usage -- print usage message and exit */
static void usage(string prog)
{
  fprintf(stderr, "Usage: %s [-v] [-x] [-b base] [-s name=subst ...] [-M target [-G]] [file-or-url]\n",
	  prog);
  exit(2);
}

int main(int argc, char *argv[])
{
  int c, status = 200;
  string base = NULL;

  /* Bind the parser callback routines to our handlers */
  set_error_handler(handle_error);
  set_start_handler(start);
  set_end_handler(end);
  set_comment_handler(handle_comment);
  set_text_handler(handle_text);
  set_decl_handler(handle_decl);
  set_pi_handler(handle_pi);
  set_starttag_handler(handle_starttag);
  set_emptytag_handler(handle_emptytag);
  set_endtag_handler(handle_endtag);
  set_endincl_handler(handle_endincl);

  /* Parse command line arguments */
  while ((c = getopt(argc, argv, ":xb:fs:M:Gv")) != -1)
    switch (c) {
    case 'x': do_xml = true; break;
    case 'b': base = optarg; break;
    case 'f': final = true; break;
    case 's': add_substitution(optarg); break;
    case 'M': target = optarg; break;
    case 'G': warn_missing = false; break;
    case 'v': printf("Version: %s %s\n", PACKAGE, VERSION); return 0;
    default: usage(argv[0]);
    }

  if (optind == argc) {
    set_yyin(stdin, base ? base : "stdin");
  } else if (optind == argc - 1) {
    set_yyin(fopenurl(argv[optind], "r", &status), base ? base : argv[optind]);
  } else {
    usage(argv[0]);
  }

  if (yyin == NULL) {perror(argv[optind]); exit(1);}
  if (status != 200) errexit("%s : %s\n", argv[optind], http_strerror(status));

  if (target) printf("%s:", target);
  if (yyparse() != 0) exit(3);
  if (target) printf("\n");

  return has_error ? 1 : 0;
}
