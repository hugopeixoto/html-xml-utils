/*
 * Remove subtrees which have a certain class attribute.
 *
 * Copyright © 1994-2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Bert Bos <bert@w3.org>
 * Created Feb 2000
 * $Id: hxprune.c,v 1.8 2017/11/24 09:50:25 bbos Exp $
 *
 **/
/* #include <mcheck.h> */
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
#include "tree.e"
#include "html.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "class.e"
#include "errexit.e"

#define EXCLUDE_CLASS "exclude"			/* Default value for class */

static Tree tree;
static bool xml = false;			/* Use <empty /> convention */


/* handle_error -- called when a parse error occurred */
static void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
}

/* start -- called before the first event is reported */
static void* start(void)
{
  tree = create();
  return NULL;
}
  
/* end -- called after the last event is reported */
static void end(void *clientdata)
{
  /* skip */
}

/* handle_comment -- called after a comment is parsed */
static void handle_comment(void *clientdata, string commenttext)
{
  tree = append_comment(tree, commenttext);
}

/* handle_text -- called after a tex chunk is parsed */
static void handle_text(void *clientdata, string text)
{
  tree = append_text(tree, text);
}

/* handle_declaration -- called after a declaration is parsed */
static void handle_decl(void *clientdata, string gi,
			string fpi, string url)
{
  tree = append_declaration(tree, gi, fpi, url);
}

/* handle_proc_instr -- called after a PI is parsed */
static void handle_pi(void *clientdata, string pi_text)
{
  tree = append_procins(tree, pi_text);
}

/* handle_starttag -- called after a start tag is parsed */
static void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  tree = html_push(tree, name, attribs);
}

/* handle_emptytag -- called after an empty tag is parsed */
static void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  tree = html_push(tree, name, attribs);
}

/* handle_pop -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *clientdata, string name)
{
  tree = html_pop(tree, name);
}

/* prune -- write the tree, suppressing elements with a certain class */
static void prune(Tree t, const string class)
{
  Tree h;
  pairlist a;

  for (h = t->children; h != NULL; h = h->sister) {
    switch (h->tp) {
      case Text:
	printf("%s", h->text);
	break;
      case Comment:
	printf("<!--%s-->", h->text);
	break;
      case Declaration:
	printf("<!DOCTYPE %s", h->name);
	if (h->text) printf(" PUBLIC \"%s\"", h->text);
	if (h->url) printf(" %s\"%s\"", h->text ? "" : "SYSTEM ", h->url);
	printf(">");
	break;
      case Procins:
	printf("<?%s>", h->text);
	break;
      case Element:
	if (! has_class(h->attribs, class)) {
	  printf("<%s", h->name);
	  for (a = h->attribs; a != NULL; a = a->next) {
	    printf(" %s", a->name);
	    if (a->value != NULL) printf("=\"%s\"", a->value);
	    else if (xml) printf("=\"%s\"", a->name);
	  }
	  if (is_empty(h->name)) {
	    assert(h->children == NULL);
	    printf(xml ? " />" : ">");
	  } else {
	    printf(">");
	    prune(h, class);
	    printf("</%s>", h->name);
	  }
	}
	break;
      case Root:
	assert(! "Cannot happen");
	break;
      default:
	assert(! "Cannot happen");
    }
  }
}

/* usage -- print usage message and exit */
static void usage(string name)
{
  fprintf(stderr, "Usage: %s [-c class] [-x] [html-file]\n", name);
  exit(1);
}


int main(int argc, char *argv[])
{
  int i, status;
  string class = EXCLUDE_CLASS;

  /*  mtrace(); */

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

  yyin = stdin;
  for (i = 1; i < argc; i++) {
    if (eq(argv[i], "-c")) {
      if (i >= argc - 1) usage(argv[0]);
      class = argv[++i];
    } else if (eq(argv[i], "-x")) {
      xml = true;
    } else {
      yyin = fopenurl(argv[i], "r", &status);
      if (yyin == NULL) {perror(argv[1]); exit(2);}
      if (status != 200) errexit("%s : %s\n", argv[i], http_strerror(status));
    }
  }

  if (yyparse() != 0) {
    exit(3);
  }
  tree = get_root(tree);
  prune(tree, class);
  tree_delete(tree);				/* Just to test memory mgmt */
  return 0;
}
