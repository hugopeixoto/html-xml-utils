/*
 * Output all elements with a certain name and/or class.
 * Input must be well-formed, since no HTML heuristics are applied.
 *
 * Copyright © 2000-2012 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 20 Aug 2000
 * Version: $Id: hxextract.c,v 1.7 2017/11/24 09:50:25 bbos Exp $
 */
#include "config.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
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
#include "export.h"
#include "types.e"
#include "html.e"
#include "heap.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "class.e"

#define INDEX "index"				/* CLASS="... index..." */

#define MAXLINELEN 1024				/* In configfile */

static bool xml = false;			/* Use <empty /> convention */
static int copying = 0;				/* Start by not copying */
static string base = NULL;			/* URL of each file */
static string endtext = "";			/* Text to insert at end */
static string targetelement = NULL;		/* Element to extract */
static string targetclass = NULL;		/* Class to extract */


/* add_href -- add an "href" attribute to a list of attributes */
static void add_href(pairlist *attribs, const string base, const conststring id)
{
  string h = NULL;

  pairlist_set(attribs, "href", strapp(&h, base, "#", id, NULL));
  free(h);
}

/* handle_error -- called when a parse error occurred */
static void handle_error(void *unused, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
}

/* start -- called before the first event is reported */
static void* start(void) {return NULL;}
  
/* end -- called after the last event is reported */
static void end(void *unused) {}

/* handle_comment -- called after a comment is parsed */
static void handle_comment(void *unused, const string commenttext) {}

/* handle_text -- called after a text chunk is parsed */
static void handle_text(void *unused, const string text)
{
  if (copying > 0) fputs(text, stdout);
}

/* handle_declaration -- called after a declaration is parsed */
static void handle_decl(void *unused, const string gi,
			const string fpi, const string url) {}

/* handle_proc_instr -- called after a PI is parsed */
static void handle_pi(void *unused, const string pi_text) {}

/* print_tag -- print a start- or empty tag */
static void print_tag(const string name, pairlist attribs, bool empty)
{
  pairlist a;
  conststring t, h;

  printf("<%s", name);
  for (a = attribs; a != NULL; a = a->next) {
    printf(" %s", a->name);
    if (strcasecmp(a->name, "class") == 0 && (t = contains(a->value, INDEX))) {
      /* Print value excluding INDEX */
      printf("=\"");
      for (h = a->value; h != t; h++) putchar(*h);
      printf("%s\"", t + sizeof(INDEX) - 1);
    } else {
      if (a->value) printf("=\"%s\"", a->value);
    }
  }
  printf((empty && xml) ? " />" : ">");
}

/* is_match check whether the element matches the target element and class */
static bool is_match(const string name, pairlist attribs)
{
  return ((!targetelement || strcasecmp(name, targetelement) == 0)
	  && (!targetclass || has_class(attribs, targetclass)));
}

/* handle_starttag -- called after a start tag is parsed */
static void handle_starttag(void *unused, const string name, pairlist attribs)
{
  conststring id;

  if (copying || is_match(name, attribs)) {
    if (!copying && (id = pairlist_get(attribs, "id")))
      add_href(&attribs, base, id);
    if (!eq(name, "a") && !eq(name, "A")) print_tag(name, attribs, false);
    copying++;
  }
}

/* handle_emptytag -- called after an empty tag is parsed */
static void handle_emptytag(void *unused, const string name, pairlist attribs)
{
  conststring id;

  if (copying || is_match(name, attribs)) {
    if (!copying && (id = pairlist_get(attribs, "id")))
      add_href(&attribs, base, id);
    if (!eq(name, "a") && !eq(name, "A")) print_tag(name, attribs, true);
  }
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *unused, const string name)
{
  if (copying) {
    if (!eq(name, "a") && !eq(name, "A")) printf("</%s>", name);
    copying--;
  }
}

/* process_configfile -- read @chapter lines from config file */
static void process_configfile(const string configfile)
{
  char line[MAXLINELEN], chapter[MAXLINELEN];
  FILE *f;

  if (! (f = fopenurl(configfile, "r", NULL))) {perror(configfile); exit(2);}

  /* ToDo: accept quoted file names with spaces in their name */
  while (fgets(line, sizeof(line), f)) {
    if (sscanf(line, " @chapter %s", chapter) == 1) {
      if (!base) base = chapter;
      yyin = fopenurl(chapter, "r", NULL);
      if (yyin == NULL) {perror(chapter); exit(2);}
      if (yyparse() != 0) exit(3);
      fclose(yyin);
      base = NULL;
    }
  }

  fclose(f);
}

/* usage -- print usage message and exit */
static void usage(const string name)
{
  fprintf(stderr, "Usage: %s [-v] [-x] [-s text] [-e text] [-b base] element-or-class [-c configfile | file-or-URL]...\n",
	  name);
  exit(1);
}

int main(int argc, char *argv[])
{
  char *p;
  int i;

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

  /* Loop over arguments; options may be in between file names */
  for (i = 1; i < argc; i++) {
    if (eq(argv[i], "-h") || eq(argv[i], "-?")) { /* Usage */
      usage(argv[0]);
    } else if (eq(argv[i], "-x")) {		/* XML format */
      xml = true;
    } else if (eq(argv[i], "-s")) {		/* Insert text at start */
      printf("%s", argv[++i]);
    } else if (eq(argv[i], "-e")) {		/* Insert text at end */
      endtext = argv[++i];
    } else if (eq(argv[i], "-b")) {		/* URL base */
      base = argv[++i];
    } else if (eq(argv[i], "-c")) {		/* Config file */
      process_configfile(argv[++i]);
    } else if (eq(argv[i], "-v")) {
      printf("Version: %s %s\n", PACKAGE, VERSION);
      return 0;
    } else if (eq(argv[i], "-")) {		/* "-" = stdin */
      if (!base) base = "";
      yyin = stdin;
      if (yyparse() != 0) exit(3);
      base = NULL;				/* Reset base */
    } else if (targetelement || targetclass) {	/* It's a file name or URL */
      if (!base) base = argv[i];
      yyin = fopenurl(argv[i], "r", NULL);
      if (yyin == NULL) {perror(argv[i]); exit(2);}
      if (yyparse() != 0) exit(3);
      fclose(yyin);
      base = NULL;
    } else if (argv[i][0] == '.') {		/* Class name */
      targetclass = argv[i] + 1;
    } else {					/* Element name */
      targetelement = argv[i];
      if ((p = strchr(targetelement, '.'))) {
	*p = '\0';
	targetclass = p + 1;
      }
    }
  }
  if (!targetelement && !targetclass) usage(argv[0]);

  printf("%s", endtext);			/* Insert text at end */
  return 0;
}
