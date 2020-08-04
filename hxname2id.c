/*
 * Move target anchors to the element they belong to, i.e., look for
 * <FOO><A NAME=bar> and <FOO><A ID=bar> and replace it with <FOO
 * ID=bar><A>
 *
 * There is no attempt to check if the name is a valid SGML/XML token
 * or whether it is unique. The replacement is syntactical only.
 *
 * Copyright © 2004 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: Dec 2004
 * Version: $Id: hxname2id.c,v 1.7 2017/11/24 09:50:25 bbos Exp $
 *
 **/
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

#ifdef HAVE_ERRNO_H
#  include <errno.h>
#endif
#ifdef HAVE_SEARCH_H
#  include <search.h>
#else
#  include "search-freebsd.h"
#endif

#include "export.h"
#include "types.e"
#include "heap.e"
#include "tree.e"
#include "html.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "errexit.e"


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
  handle_starttag(clientdata, name, attribs);
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *clientdata, string name)
{
  tree = html_pop(tree, name);
}

/* has_anchor_child -- check if the first thing in the element is an <A NAME> */
static bool has_anchor_child(Tree t, conststring *nameval)
{
  Tree h;

  /* Loop until either text or an element is found */
  for (h = t->children; h != NULL; h = h->sister) {
    switch (h->tp) {
    case Comment:		/* Skip these */
    case Procins:
      break;
    case Text:			/* Skip if whitespace, otherwise return false */
      if (! only_space(h->text)) return false;
      break;
    case Element:		/* true if <A NAME> or <A ID>, else false */
      return eq(h->name, "a") &&
	((*nameval = get_attrib(h, "id")) ||
	 (*nameval = get_attrib(h, "name")));
    default:
      assert(! "Cannot happen");
    }
  }
  return false;
}

/* process -- write the tree, add IDs at elements with an <A NAME> child */
static void process(Tree t, bool remove_anchor)
{
  Tree h;
  conststring nameval;
  bool remove_next_anchor = false;
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
	if (!get_attrib(h, "id") && has_anchor_child(h, &nameval)) {
	  /* Put the anchor on this element and remove it from the child */
	  set_attrib(h, "id", nameval);
	  remove_next_anchor = true;
	}
	printf("<%s", h->name);
	for (a = h->attribs; a != NULL; a = a->next) {
	  /* Print attribs, except id/name that the parent wants us to remove */
	  if (!remove_anchor || (!eq(a->name, "id") && !eq(a->name, "name"))) {
	    printf(" %s", a->name);
	    if (a->value != NULL) printf("=\"%s\"", a->value);
	  }
	}
	if (is_empty(h->name)) {
	  assert(h->children == NULL);
	  printf(xml ? " />" : ">");
	} else {
	  printf(">");
	  process(h, remove_next_anchor);
	  printf("</%s>", h->name);
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
  errexit("Version %s\nUsage: %s [-x] [html-file]\n", VERSION, name);
}


int main(int argc, char *argv[])
{
  int i, status;

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
    if (eq(argv[i], "-x")) {
      xml = true;
    } else if (eq(argv[i], "-?")) {
      usage(argv[0]);
    } else if (eq(argv[i], "-")) {
      /* yyin = stdin; */
    } else {
      yyin = fopenurl(argv[i], "r", &status);
      if (yyin == NULL) {perror(argv[1]); exit(2);}
      if (status != 200) errexit("%s : %s\n", argv[i], http_strerror(status));
    }
  }
  if (yyparse() != 0) exit(3);

  tree = get_root(tree);
  process(tree, false);
  tree_delete(tree);				/* Just to test memory mgmt */
  return 0;
}
