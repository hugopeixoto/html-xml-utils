/*
 * unxmlns - convert <{namespace}foo> to <foo xmlns="namespace">
 *
 * This program is the reverse of xmlns.
 *
 * To do: optimize, i.e., reuse inherited namespace declaration,
 * instead of declaring them again on every element.
 *
 * Copyright © 2005 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos
 * Created: 8 November 2005
 * Version: $Id: hxunxmlns.c,v 1.7 2017/11/24 09:50:25 bbos Exp $
 *
 **/
#include "config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <ctype.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"
#include "html.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "errexit.e"


#define XMLNS "{http://www.w3.org/XML/1998/namespace}"
const static size_t XMLNSLEN = 39;		/* strlen(XMLNS) */

extern int yylineno;				/* From scan.l */

static bool has_error = false;


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
  return NULL;
}
  
/* end -- called after the last event is reported */
void end(void *clientdata)
{
  /* skip */
}

/* handle_comment -- called after a comment is parsed */
void handle_comment(void *clientdata, string commenttext)
{
  printf("<!--%s-->", commenttext);
}

/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, string text)
{
  fputs(text, stdout);
}

/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, string gi, string fpi,
		 string url)
{
  printf("<!DOCTYPE %s", gi);
  if (fpi) printf(" PUBLIC \"%s\"\n", fpi);
  if (url) printf(" %s\"%s\"", fpi ? "" : "SYSTEM ", url);
  printf(">");
}

/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  printf("<?%s>", pi_text);
}

/* print_attrs -- print attributes and declare their namespaces, if any */
static void print_attrs(const pairlist attribs)
{
  string h, s;
  pairlist p;
  int n = 0;

  for (p = attribs; p; p = p->next) {
    if (p->name[0] != '{') {
      printf(" %s=\"%s\"", p->name, p->value);
    } else if (p->name[1] == '}') {
      printf(" %s=\"%s\"", p->name + 2, p->value);
    } else if (strncmp(p->name, XMLNS, XMLNSLEN) == 0) {
      printf(" xml:%s=\"%s\"", p->name + XMLNSLEN, p->value);
    } else if (! (h = strchr(p->name, '}'))) {
      fprintf(stderr, "%d: Unmatched \"{\" in attribute name (\"%s\")\n",
	      yylineno, p->name);
      has_error = true;
    } else {
      printf(" xmlns:x%d=\"", n);
      for (s = p->name + 1; s != h; s++) putchar(*s);
      printf("\" x%d:%s=\"%s\"", n, h + 1, p->value);
    }
  }
}

/* print_tag_start -- print "<foo" and possibly an xmlns="..." */
static void print_tag_start(const string name)
{
  string s, h;

  if (name[0] != '{') {
    printf("<%s", name);
  } else if (name[1] == '}') {
    printf("<%s", name + 2);
  } else if (strncmp(name, XMLNS, XMLNSLEN) == 0) {
    printf("<xml:%s", name + XMLNSLEN);
  } else if (! (h = strchr(name, '}'))) {
    fprintf(stderr, "%d: Unmatched \"{\" in tag name (\"%s\")\n",
	    yylineno, name);
    has_error = true;
  } else {
    printf("<%s xmlns=\"", h + 1);
    for (s = name + 1; s != h; s++) putchar(*s);
    putchar('\"');
  }
}

/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  print_tag_start(name);
  print_attrs(attribs);
  putchar('>');
}

/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  print_tag_start(name);
  print_attrs(attribs);
  printf(" />");
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  string h;

  if (name[0] != '{') {
    printf("</%s>", name);
  } else if (! (h = strchr(name, '}'))) {
    fprintf(stderr, "%d: Unmatched \"{\" in tag name (\"%s\")\n",
	    yylineno, name);
    has_error = true;
  } else {
    printf("</%s>", h + 1);
  }
}

/* --------------------------------------------------------------------- */

/* usage -- print usage message and exit */
static void usage(string prog)
{
  fprintf(stderr, "Version %s\nUsage: %s [html-file-or-url]\n", VERSION, prog);
  exit(2);
}

int main(int argc, char *argv[])
{
  int i, status = 200;

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

  /* Parse command line arguments */
  for (i = 1; i < argc && argv[i][0] == '-' && !eq(argv[i], "--"); i++) {
    switch (argv[i][1]) {
      default: usage(argv[0]);
    }
  }
  if (i < argc && eq(argv[i], "--")) i++;

  if (i == argc) yyin = stdin;
  else if (i == argc - 1 && eq(argv[i], "-")) yyin = stdin;
  else if (i == argc - 1) yyin = fopenurl(argv[i], "r", &status);
  else usage(argv[0]);

  if (yyin == NULL) {perror(argv[i]); exit(1);}
  if (status != 200) errexit("%s : %s\n", argv[i], http_strerror(status));

  if (yyparse() != 0) exit(3);

  return has_error ? 1 : 0;
}
