/*
 * Output a table of content of all the headers in all the files
 * given on the command line.
 *
 * Headers with class "no-toc" will not be listed in the ToC.
 *
 * The ToC links to elements with ID attributes as well as with
 * empty <A NAME> elements.
 *
 * Tags for a <SPAN> with class "index" are assumed to be used by
 * a cross-reference generator and will not be copied to the ToC.
 *
 * howcome 2005-01-16: class attributes from header tags are now
 * copied to generated LI tags
 *
 * Copyright © 1997-2005 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Bert Bos <bert@w3.org>
 * Created Sep 1997
 * $Id: hxmultitoc.c,v 1.6 2017/11/24 09:50:25 bbos Exp $
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
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "class.e"

#define NO_TOC "no-toc"				/* CLASS="... no-toc..." */
#define INDEX "index"				/* CLASS="... index..." */

#define MAXLINELEN 1024				/* In configfile */

#define EXPAND true
#define NO_EXPAND false
#define KEEP_ANCHORS true
#define REMOVE_ANCHORS false

static int toc_low = 1, toc_high = 6;		/* Which headers to include */
static bool xml = false;			/* Use <empty /> convention */
static bool copying = false;			/* Start by not copying */
static int curlevel = 0;			/* Level of previous heading */
static string base = NULL;			/* URL of each file */
static string endtext = "";			/* Text to insert at end */


/* handle_error -- called when a parse error occurred */
static void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
}

/* start -- called before the first event is reported */
static void* start(void) {return NULL;}
  
/* end -- called after the last event is reported */
static void end(void *clientdata) {}

/* handle_comment -- called after a comment is parsed */
static void handle_comment(void *clientdata, const string commenttext) {}

/* handle_text -- called after a text chunk is parsed */
static void handle_text(void *clientdata, const string text)
{
  if (copying) fputs(text, stdout);
}

/* handle_declaration -- called after a declaration is parsed */
static void handle_decl(void *clientdata, const string gi,
			const string fpi, const string url) {}

/* handle_proc_instr -- called after a PI is parsed */
static void handle_pi(void *clientdata, const string pi_text) {}

/* handle_header -- handle a H? start tag */
static void handle_header(int level, pairlist attribs)
{
  conststring id, class;

  if (has_class(attribs, NO_TOC)) return;
  if (level < toc_low || level > toc_high) return;
  for (; curlevel > level; curlevel--) printf("</ul>\n");
  for (; curlevel < level - 1; curlevel++) printf("<li><ul class=\"toc\">\n");
  if (curlevel == level - 1) {printf("<ul class=\"toc\">\n"); curlevel++;}
  id = pairlist_get(attribs, "id");
  class = pairlist_get(attribs, "class");
  if (class) {
    printf("<li class=\"%s\"><a href=\"%s#%s\">", class, base, id ? id : (string) "");
  } else {
    printf("<li><a href=\"%s#%s\">", base, id ? id : (string) "");
  }
  copying = true;
}

/* handle_span -- print a <span> starttag but without class=index */
static void handle_span(pairlist attribs)
{
  pairlist a;
  conststring t, h;

  printf("<span");
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
  printf(">");
}

/* finalize -- close any open lists */
static void finalize(void)
{
  for (; curlevel >= toc_low; curlevel--) printf("</ul>\n");
}

/* handle_starttag -- called after a start tag is parsed */
static void handle_starttag(void *clientdata, const string name,
			    pairlist attribs)
{
  pairlist a;

  if (eq(name, "h1") || eq(name, "H1")) handle_header(1, attribs);
  else if (eq(name, "h2") || eq(name, "H2")) handle_header(2, attribs);
  else if (eq(name, "h3") || eq(name, "H3")) handle_header(3, attribs);
  else if (eq(name, "h4") || eq(name, "H4")) handle_header(4, attribs);
  else if (eq(name, "h5") || eq(name, "H5")) handle_header(5, attribs);
  else if (eq(name, "h6") || eq(name, "H6")) handle_header(6, attribs);
  else if (eq(name, "a") || eq(name, "A")) ;	/* Skip anchors */
  else if (copying && !strcasecmp(name, "span")) handle_span(attribs);
  else if (copying) {				/* Copy the tag */
    printf("<%s", name);
    for (a = attribs; a != NULL; a = a->next) {
      printf(" %s", a->name);
      if (a->value != NULL) printf("=\"%s\"", a->value);
    }
    printf(">");
  }
}

/* handle_emptytag -- called after an empty tag is parsed */
static void handle_emptytag(void *clientdata, const string name,
			    pairlist attribs)
{
  pairlist a;

  if (copying && !eq(name, "a") && !eq(name, "A")) { /* Copy the tag */
    printf("<%s", name);
    for (a = attribs; a != NULL; a = a->next) {
      printf(" %s", a->name);
      if (a->value != NULL) printf("=\"%s\"", a->value);
    }
    printf(xml ? " />" : ">");
  }
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *clientdata, const string name)
{
  if (copying) {
    if (eq(name, "h1") || eq(name, "H1") || eq(name, "h2")
	|| eq(name, "H2") || eq(name, "h3") || eq(name, "H3")
	|| eq(name, "h4") || eq(name, "H4") || eq(name, "h5")
	|| eq(name, "H5") || eq(name, "h6") || eq(name, "H6")) {
      printf("</a>\n");
      copying = false;
    } else if (eq(name, "a") || eq(name, "A")) {
      /* skip anchors */
    } else {
      printf("</%s>", name);
    }
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
  fprintf(stderr, "Version %s\n\
Usage: %s [-x] [-s text ] [-e text ] [-l low | -h high | -b base | html-file \
| -c configfile]+\n",
	  VERSION, name);
  exit(1);
}

int main(int argc, char *argv[])
{
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
    if (eq(argv[i], "-l")) {
      if (i >= argc - 1) usage(argv[0]);
      toc_low = atoi(argv[++i]);
      curlevel = toc_low - 1;
      if (toc_low < 1) toc_low = 1;
    } else if (eq(argv[i], "-h")) {
      if (i >= argc - 1) usage(argv[0]);
      toc_high = atoi(argv[++i]);
      if (toc_high > 6) toc_high = 6;
    } else if (eq(argv[i], "-x")) {		/* XML format */
      xml = true;
    } else if (eq(argv[i], "-s")) {		/* Insert text at start */
      printf("%s", argv[++i]);
    } else if (eq(argv[i], "-e")) {		/* Insert text at end */
      endtext = argv[++i];
    } else if (eq(argv[i], "-b")) {
      base = argv[++i];
    } else if (eq(argv[i], "-c")) {		/* Config file */
      process_configfile(argv[++i]);
    } else if (eq(argv[i], "-")) {
      if (!base) base = "";
      yyin = stdin;
      if (yyparse() != 0) exit(3);
      base = NULL;				/* Reset base */
    } else {
      if (!base) base = argv[i];
      yyin = fopenurl(argv[i], "r", NULL);
      if (yyin == NULL) {perror(argv[1]); exit(2);}
      if (yyparse() != 0) exit(3);
      fclose(yyin);
      base = NULL;
    }
  }
  finalize();
  printf("%s", endtext);			/* Insert text at end */
  return 0;
}
