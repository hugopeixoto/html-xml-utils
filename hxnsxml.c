/*
 * hxnsxml - convert output of hxxmlns back to normal XML
 *
 * To do: handle quotes in Namespace URLs.
 * To do: handle XML's own Namespace.
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Copyright © 1994-2010 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos
 * Created: 12 July 2010
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

#define XML "{http://www.w3.org/XML/1998/namespace}"

static bool has_error = false;
static bool has_ns = false;	/* true if Namespaces occur anywhere in document */


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
  printf("%s", text);
}

/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, string gi, string fpi,
		 string url)
{
  printf("<!DOCTYPE %s", gi);
  if (fpi) printf(" PUBLIC \"%s\"", fpi);
  if (url) printf(" %s\"%s\"", fpi ? "" : "SYSTEM ", url);
  printf(">\n");
}

/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  printf("<?%s>", pi_text);
}

/* print_attrs -- print attributes */
void print_attrs(const pairlist attribs)
{
  pairlist p;
  int i, j;
  char c = 'a';

  for (p = attribs; p; p = p->next) {

    if (p->name[0] != '{') {
      i = 0;
    } else {
      for (i = 1; p->name[i] && p->name[i] != '}'; i++);
      if (p->name[i]) i++;
    }
    if (i > 2) {
      if (c > 'z') {
	fprintf(stderr, "Bug: hxnsxml cannot handle > 26 namespaces per element.\n");
	exit(2);
      }
      printf(" xmlns:%c=\"", c);
      for (j = 1; j < i - 1; j++) putchar(p->name[j]);
      putchar('\"');
      printf(" %c:", c);
      c++;
    } else {
      printf(" ");
    }
    printf("%s=\"%s\"", p->name + i, p->value);
  }
}

/* print_tag -- print "<" and the element name, optionally with a namespace */
static void print_tag(const conststring name)
{
  int i, j;

  if (name[0] != '{') {
    i = 0;
  } else {
    for (i = 1; name[i] && name[i] != '}'; i++);
    if (name[i]) i++;
  }
  printf("<%s", name + i);
  if (i > 2) {			/* Element has a Namespace */
    printf(" xmlns=\"");
    for (j = 1; j < i - 1; j++) putchar(name[j]);
    putchar('"');
    has_ns = true;
  } else if (has_ns) {		/* Document has Namespaces, this element not */
    printf(" xmlns=\"\"");
  }
}

/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  print_tag(name);
  print_attrs(attribs);
  putchar('>');
}

/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  print_tag(name);
  print_attrs(attribs);
  printf(" />");
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  int i;

  if (name[0] != '{') {
    i = 0;
  } else {
    for (i = 1; name[i] && name[i] != '}'; i++);
    if (name[i]) i++;
  }
  printf("</%s>", name + i);
}

/* --------------------------------------------------------------------- */

/* usage -- print usage message and exit */
static void usage(string prog)
{
  fprintf(stderr, "Version %s\nUsage: %s [file-or-url]\n", VERSION, prog);
  exit(2);
}

int main(int argc, char *argv[])
{
  int status = 200;

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

  if (argc > 2) usage(argv[0]);
  else if (argc == 2) yyin = fopenurl(argv[1], "r", &status);
  else yyin = stdin;

  if (!yyin) {perror(argv[1]); exit(1);}
  if (status != 200) errexit("%s : %s\n", argv[1], http_strerror(status));

  if (yyparse() != 0) exit(3);

  return has_error ? 1 : 0;
}
