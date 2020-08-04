/*
 * Copy input (well-formed XML) without any elements that match a given selector
 *
 * To do: Put common routines with hxselect in separate module.
 *
 * To do: Only look for a LANG attribute if the input is XHTML.
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Copyright © 2012-2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 21 Oct 2012
 */
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#ifdef HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif
#include "types.e"
#include "tree.e"
#include "selector.e"
#include "heap.e"
#include "errexit.e"
#include "html.e"
#include "scan.e"
#include "selmatch.e"


static Selector selector;			/* The selector to match */


/* print_tree -- print tree below t, omitting elements that match the selector */
static void print_tree(Tree t)
{
  pairlist p;

  if (!t) return;

  switch (t->tp) {
  case Element:
    /* Print the element, unless it matches the selector */
    if (!matches_sel(t, selector)) {
      printf("<%s", t->name);
      /* Print each attribute, unless it matches the selector */
      for (p = t->attribs; p; p = p->next)
	if (!selector->pseudoelts || selector->pseudoelts->type != AttrNode
	    || !same(p->name, selector->pseudoelts->s)
	    || !matches_sel(t, selector->context))
	  printf(" %s=\"%s\"", p->name, p->value);
      if (!t->children) {
	printf("/>");
      } else {
	printf(">");
	print_tree(t->children);
	printf("</%s>", t->name);
      }
    }
    break;
  case Text:
    printf("%s", t->text);
    break;
  case Comment:
    printf("<!--%s-->", t->text);
    break;
  case Declaration:
    printf("<!DOCTYPE %s", t->name);
    if (t->text && t->url) printf(" PUBLIC \"%s\" \"%s\"", t->text, t->url);
    else if (t->text) printf(" PUBLIC \"%s\"", t->text);
    else if (t->url) printf(" SYSTEM \"%s\"", t->url);
    printf(">");
    break;
  case Procins:
    printf("<?%s>", t->text);
    break;
  case Root:
    print_tree(t->children);
    break;
  default: assert(!"Cannot happen!");
  }

  print_tree(t->sister);
}


/*********************** Parser callback API ***********************/

/* handle_error -- called when a parse error occurred */
static void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
}


/* handle_start -- called before the first event is reported */
static void* handle_start(void)
{
  Tree *tp;
  new(tp);
  *tp = create();		/* Create an empty tree */
  return tp;
}


/* handle_end -- called after the last event is reported */
static void handle_end(void *clientdata)
{
  Tree *t = (Tree*)clientdata;
  print_tree(get_root(*t)); /* Print tree, filtering out unwanted elements */
  tree_delete(*t);
}


/* handle_comment -- called after a comment is parsed */
static void handle_comment(void *clientdata, const string commenttext)
{
  Tree *t = (Tree*)clientdata;
  *t = append_comment(*t, commenttext);
}


/* handle_text -- called after a text chunk is parsed */
static void handle_text(void *clientdata, const string text)
{
  Tree *t = (Tree*)clientdata;
  *t = tree_append_text(*t, text);
}


/* handle_decl -- called after a declaration is parsed */
static void handle_decl(void *clientdata, const string gi, const string fpi,
			const string url)
{
  Tree *t = (Tree*)clientdata;
  *t = append_declaration(*t, gi, fpi, url);
}


/* handle_pi -- called after a Processing Instruction is parsed */
static void handle_pi(void *clientdata, const string pi_text)
{
  Tree *t = (Tree*)clientdata;
  *t = append_procins(*t, pi_text);
}


/* handle_starttag -- called after a start tag is parsed */
static void handle_starttag(void *clientdata, const string name,
			    pairlist attribs)
{
  Tree *t = (Tree*)clientdata;
  *t = tree_push(*t, name, attribs);
}


/* handle_emptytag -- called after an empty tag is parsed */
static void handle_emptytag(void *clientdata, const string name,
			    pairlist attribs)
{
  Tree *t = (Tree*)clientdata;
  *t = tree_push(*t, name, attribs);
  *t = tree_pop(*t, name);
}


/* handle_endtag -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *clientdata, const string name)
{
  Tree *t = (Tree*)clientdata;
  *t = tree_pop(*t, name);
}


/* handle_endincl -- called at the end of an included file */
/* void handle_endincl(void *clientdata) */

/********************* End of parser callbacks *********************/


/* usage -- print usage message and exit */
static void usage(const conststring progname)
{
  errexit("Usage: %s [-v] [-l language] [-i] selector\n", progname);
}


int main(int argc, char *argv[])
{
  string s;
  int c;

  /* Command line options */
  while ((c = getopt(argc, argv, "il:v")) != -1) {
    switch (c) {
    case 'l': init_language(optarg); break;
    case 'i': set_case_insensitive();  break;
    case 'v': printf("Version: %s %s\n", PACKAGE, VERSION); return 0;
    case '?': usage(argv[0]); break;
    default: assert(!"Cannot happen");
    }
  }

  /* Bind the parser callback routines to our handlers */
  set_error_handler(handle_error);
  set_start_handler(handle_start);
  set_end_handler(handle_end);
  set_comment_handler(handle_comment);
  set_text_handler(handle_text);
  set_decl_handler(handle_decl);
  set_pi_handler(handle_pi);
  set_starttag_handler(handle_starttag);
  set_emptytag_handler(handle_emptytag);
  set_endtag_handler(handle_endtag);

  /* Parse the selector */
  if (optind >= argc) usage(argv[0]);		/* Need at least 1 arg */
  for (s = newstring(argv[optind++]); optind < argc; optind++)
    strapp(&s, " ", argv[optind], NULL);
  selector = parse_selector(s, &s);
  if (*s) errexit("Syntax error at \"%c\"\n", *s);

  /* Parse the input, build a tree, filter the tree */
  yyin = stdin;
  if (yyparse() != 0) exit(3);
  return 0;
}
