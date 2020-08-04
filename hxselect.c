/*
 * select -- extract elements matching a selector
 *
 * Assumes that class selectors (".foo") refer to an attribute called
 * "class".
 *
 * Assumes that ID selectors ("#foo") refer to an attribute called
 * "id".
 *
 * Options:
 *
 * -l language
 *
 *     Sets the default language, in case the root element doesn't
 *     have an xml: lang attribute. Example: -l en. Default: none.
 *
 * -s separator
 *
 *     A string to print after each match. Accepts C-like escapes.
 *     Example: -s '\n\n' to print an empty line after each match.
 *     Default: empty string.
 *
 * -i
 *
 *     Match case-insensitively. Useful for HTML and some other
 *     SGML-based languages.
 *
 * -c
 *
 *     Print content only. Without -c, the start and end tag of the
 *     matched element are printed as well; with -c only the contents
 *     of the matched element are printed.
 *
 * TODO: Escape double quotes when printing an attribute that matches
 * ::attr().
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Copyright © 2001-2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 5 Jul 2001
 * Version: $Id: hxselect.c,v 1.10 2017/11/24 09:50:25 bbos Exp $
 *
 **/
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
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
#include "types.e"
#include "tree.e"
#include "selector.e"
#include "heap.e"
#include "errexit.e"
#include "html.e"
#include "scan.e"
#include "selmatch.e"


static Tree tree = NULL;			/* Current elt in tree */
static Selector selector;			/* The selector to match */
static bool content_only = false;		/* Omit start/end tag */
static string separator = "";			/* Printed between matches */


/* print_starttag -- print a start tag */
static void print_starttag(Node *n)
{
  pairlist p;

  printf("<%s", n->name);
  for (p = n->attribs; p; p = p->next) printf(" %s=\"%s\"", p->name, p->value);
  putchar('>');
}


/* printsep -- print the separator string, interpret escapes */
static void printsep(const string separator)
{
  string s = separator;
  int c;

  while (*s) {
    if (*s != '\\') putchar(*(s++));
    else if ('0' <= *(++s) && *s <= '7') {
      c = *s - '0';
      if ('0' <= *(++s) && *s <= '7') {
	c = 8 * c + *s - '0';
	if ('0' <= *(++s) && *s <= '7') c = 8 * c + *s - '0';
      }
      putchar(c); s++;
    } else
      switch (*s) {
      case '\0': putchar('\\'); break;
      case 'n': putchar('\n'); s++; break;
      case 't': putchar('\t'); s++; break;
      case 'r': putchar('\r'); s++; break;
      case 'f': putchar('\f'); s++; break;
      default: putchar(*(s++)); break;
      }
  }
}


/* print_recursively -- print the element t, its children and sisters */
static void print_recursively(Tree t)
{
  if (!t) return;

  switch (t->tp) {
  case Element:
    print_starttag(t);
    print_recursively(t->children);
    printf("</%s>", t->name);
    break;
  case Text: printf("%s", t->text); break;
  case Comment: printf("<!--%s-->", t->text); break;
  case Declaration: assert(!"Cannot happen"); break;
  case Procins: printf("<?%s>", t->text); break;
  case Root: print_recursively(t->children); break;
  default: assert(!"Cannot happen");
  }

  print_recursively(t->sister);
}


/* print_tree -- print the element t, with or without its tags */
static void print_tree(Tree t)
{
  assert(t->tp == Element);
  if (!content_only) print_starttag(t);
  print_recursively(t->children);
  if (!content_only) printf("</%s>", t->name);
  printsep(separator);
}


/* match_pseudoelts -- print pseudo-elements of node t that match the selector */
static void match_pseudoelts(Tree t)
{
  pairlist p;

  assert(selector->pseudoelts);

  /* ::attr() is the only pseudo-element we can handle so far */
  if (selector->pseudoelts->type == AttrNode) {
    assert(selector->combinator == Child && selector->context);
    p = t->attribs;
    while (p && !same(p->name, selector->pseudoelts->s)) p = p->next;
    if (p && matches_sel(t, selector->context)) {
      if (!content_only) printf("%s=\"", p->name);
      printf("%s", p->value);
      if (!content_only) printf("\"");
      printsep(separator);
    }
  }
}


/* walk_tree -- find all nodes in the tree that match the selector */
static void walk_tree(Tree t)
{
  if (!t) return;

  switch (t->tp) {
  case Element:
    if (selector->pseudoelts) match_pseudoelts(t);
    else if (matches_sel(t, selector)) print_tree(t);
    walk_tree(t->children);
    break;
  case Text: break;
  case Comment: break;
  case Declaration: break;
  case Procins: break;
  case Root: walk_tree(t->children); break;
  default: assert(!"Cannot happen");
  }

  walk_tree(t->sister);
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
  tree = create();
  return NULL;
}


/* handle_end -- called after the last event is reported */
static void handle_end(void *clientdata)
{
  walk_tree(tree);	       /* Find all matches for the selector */
  tree_delete(tree);	       /* Clean up */
}


/* handle_comment -- called after a comment is parsed */
static void handle_comment(void *clientdata, const string commenttext)
{
  free(commenttext);
}


/* handle_text -- called after a text chunk is parsed */
static void handle_text(void *clientdata, const string text)
{
  tree = tree_append_text(tree, text);
}


/* handle_decl -- called after a declaration is parsed */
static void handle_decl(void *clientdata, const string gi, const string fpi,
			const string url)
{
  free(gi);
  free(fpi);
  free(url);
}


/* handle_pi -- called after a Processing Instruction is parsed */
static void handle_pi(void *clientdata, const string pi_text)
{
  free(pi_text);
}


/* handle_starttag -- called after a start tag is parsed */
static void handle_starttag(void *clientdata, const string name,
			    pairlist attribs)
{
  tree = tree_push(tree, name, attribs);	/* Add to tree */
  free(name);
}


/* handle_emptytag -- called after an empty tag is parsed */
static void handle_emptytag(void *clientdata, const string name,
			    pairlist attribs)
{
  tree = tree_push(tree, name, attribs);	/* Add to tree */
  tree = tree_pop(tree, name);			/* Remove from tree again */
  free(name);
}


/* handle_endtag -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *clientdata, const string name)
{
  tree = tree_pop(tree, name);
  free(name);
}


/* handle_endincl -- called at the end of an included file */
/* void handle_endincl(void *clientdata) */

/********************* End of parser callbacks *********************/


/* usage -- print usage message and exit */
static void usage(const conststring progname)
{
  errexit("Usage: %s [-v] [-i] [-c] [-l language] [-s separator] selector\n",
	  progname);
}


int main(int argc, char *argv[])
{
  string s;
  int c;

  /* Command line options */
  while ((c = getopt(argc, argv, "icl:s:v")) != -1) {
    switch (c) {
    case 'c': content_only = true; break;
    case 'l': init_language(optarg); break;
    case 's': separator = optarg; break;
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

  /* Walk the tree */
  yyin = stdin;
  if (yyparse() != 0) exit(3);
  return 0;
}
