/*
 * Insert an active ToC between "<!--begin-toc-->" and "<!--end-toc-->",
 * or replacing the comment "<!--toc-->"
 *
 * Headers with class "no-toc" will not be listed in the ToC.
 *
 * The ToC links to elements with ID attributes as well as with
 * empty <A NAME> elements.
 *
 * Tags for a <SPAN> with class "index" are assumed to be used by
 * a cross-reference generator and will not be copied to the ToC.
 *
 * Similarly, DFN tags are not copied to the ToC (but the element's
 * content is).
 *
 * Any <A> tags with a class of "bctarget" are not copied, but
 * regenerated. They are assumed to be backwards-compatible versions
 * of ID attributes on their parent elements. With the option -t or -x
 * they are removed.
 *
 * Copyright © 1994-2013 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created Sep 1997
 * Version: $Id: hxtoc.c,v 1.13 2019/08/28 19:14:07 bbos Exp $
 *
 **/
#include "config.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
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
#include "genid.e"
#include "class.e"

#define BEGIN_TOC "begin-toc"			/* <!--begin-toc--> */
#define END_TOC "end-toc"			/* <!--end-toc--> */
#define TOC "toc"				/* <!--toc--> */
#define NO_TOC "no-toc"				/* CLASS="... no-toc..." */
#define INDEX "index"				/* CLASS="... index..." */
#define TARGET "bctarget"			/* CLASS="...bctarget..." */

#define EXPAND true
#define NO_EXPAND false
#define KEEP_ANCHORS true
#define REMOVE_ANCHORS false
#define DONT_FLATTEN false

#define INDENT " "				/* Amount to indent ToC per level */

static Tree tree;
static int toc_low = 1, toc_high = INT_MAX;	/* Which headers to include */
static bool xml = false;			/* Use <empty /> convention */
static bool bctarget = true;			/* Generate <a name=> after IDs */
static string toc_class = "toc";		/* <ul class="..."> */
static bool use_div = false;			/* Option -d */
static bool do_flatten = false;			/* Option -f */


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
  conststring id;

  tree = html_push(tree, name, attribs);

  /* If it has an ID, store it (so we don't accidentally generate it) */
  if ((id = pairlist_get(attribs, "id"))) storeID(id);
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

/* indent -- print level times a number of spaces */
static void indent(int level)
{
  for (; level > 0; level--) printf(INDENT);
}

/* is_div -- t is a division (DIV, SECTION, ARTICLE, ASIDE or NAV) */
static bool is_div(Tree t)
{
  assert(t->tp == Element);
  return eq(t->name, "div") ||
    eq(t->name, "section") ||   /* HTML5 */
    eq(t->name, "article") ||   /* HTML5 */
    eq(t->name, "aside") ||	/* HTML5 */
    eq(t->name, "nav");		/* HTML5 */
}

/* heading_level -- return 1..6 if t is H1..H6, else 0 */
static int heading_level(Tree t)
{
  assert(t->tp == Element);
  if (has_class(t->attribs, NO_TOC)) return 0;
  if (eq(t->name, "h1")) return 1;
  if (eq(t->name, "h2")) return 2;
  if (eq(t->name, "h3")) return 3;
  if (eq(t->name, "h4")) return 4;
  if (eq(t->name, "h5")) return 5;
  if (eq(t->name, "h6")) return 6;
  return 0;
}

/* div_parent -- if t is the first heading in a section elt, return that elt */
static Tree div_parent(Tree t)
{
  Tree h, result = NULL;

  assert(t->tp == Element);
  assert(t->parent);
  assert(eq(t->name, "hgroup") || heading_level(t) > 0);
  if (t->parent->tp != Element) return NULL;
  if (has_class(t->parent->attribs, NO_TOC)) return NULL;
  if (is_div(t->parent)) result = t->parent;
  else if (!eq(t->parent->name, "hgroup")) return NULL;
  else if (!(result = div_parent(t->parent))) return NULL;

  /* Check if t is the first heading in its parent. */
  for (h = t->parent->children; h != t; h = h->sister)
    if (h->tp == Element && (eq(h->name, "hgroup") || heading_level(h) > 0))
      return NULL;		/* No, it's not */
  return result;		/* Yes, it is */
}

/* has_heading -- true if the element has at least one Hn or HGROUP as child */
static bool has_heading(Tree t)
{
  Tree h;

  assert(t->tp == Element);
  for (h = t->children; h; h = h->sister) {
#if 0
    switch (h->tp) {
    case Element:
      return eq(h->name, "hgroup") || heading_level(h) > 0;
    case Text:
      if (!only_space(h->text))
	return false;
      break;
    default:
      break;
    }
#else
    if (h->tp == Element &&
	(eq(h->name, "hgroup") || heading_level(h) > 0)) return true;
#endif
  }
  return false;
}

static void expand(Tree t, bool *write, bool exp, bool keep_anchors,
		   int div_depth, bool flatten);

/* toc -- create a table of contents */
static void toc(Tree t, int *curlevel, bool *item_is_open, int div_depth)
{
  conststring val, id;
  int level;
  Tree h, div = NULL;
  bool write = true;

  switch (t->tp) {
    case Text: break;
    case Comment: break;
    case Declaration: break;
    case Procins: break;
    case Element:
      if (use_div && is_div(t) && has_heading(t)) {
	/* It's a section element with a heading */
	div_depth++;
	level = 0;
      } else {
	/* Check if the element is a heading and what its level is */
	level = heading_level(t);
	if (level && use_div && (div = div_parent(t))) level = div_depth;
      }
      /* If it's a header for the ToC, create a list item for it */
      if (level >= toc_low && level <= toc_high) {
	/* Ensure there is an ID to point to */
	h = use_div && div ? div : t;
	if (! (id = get_attrib(h, "id"))) {
	  id = gen_id(h);
	  set_attrib(h, "id", id);
	}
	assert(*curlevel <= level || *item_is_open);
	while (*curlevel > level) {
	  printf(xml ? "</li>\n" : "\n");
	  indent(*curlevel - toc_low);
	  printf("</ul>");
	  (*curlevel)--;
	}
	if (*curlevel == level && *item_is_open) {
	  printf(xml ? "</li>\n" : "\n");
	} else if (*item_is_open) {
	  printf("\n");
	  (*curlevel)++;
	  indent(*curlevel - toc_low);
	  printf("<ul class=\"%s\">\n", toc_class);
	}
	while (*curlevel < level) {
	  indent(*curlevel - toc_low);
 	  printf("<li>\n");
	  (*curlevel)++;
	  indent(*curlevel - toc_low);
	  printf("<ul class=\"%s\">\n", toc_class);
	}
	indent(*curlevel - toc_low);
	if ((val = get_attrib(t, "class"))) {
	  printf("<li class=\"%s\"><a href=\"#%s\">", val, id);
	} else {
	  printf("<li><a href=\"#%s\">", id);
	}
	expand(t, &write, NO_EXPAND, REMOVE_ANCHORS, div_depth, do_flatten);
	printf("</a>");
	*item_is_open = true;
      } else {
	for (h = t->children; h != NULL; h = h->sister)
	  toc(h, curlevel, item_is_open, div_depth);
      }
      break;
    case Root:
      for (h = t->children; h != NULL; h = h->sister)
	toc(h, curlevel, item_is_open, div_depth);
      break;
    default: assert(! "Cannot happen");
  }
}

/* expand -- write the tree, inserting ID's at H* and inserting a toc */
static void expand(Tree t, bool *write, bool exp, bool keep_anchors,
		   int div_depth, bool flatten)
{
  conststring val;
  Tree h;
  pairlist a;
  conststring s;
  int level;
  bool item_is_open = false;

  for (h = t->children; h != NULL; h = h->sister) {
    switch (h->tp) {
      case Text:
	if (*write) printf("%s", h->text);
	break;
      case Comment:
	for (s = h->text; isspace(*s); s++) ;
	if (exp && (!strncmp(s, TOC, sizeof(TOC) - 1)
		    || !strncmp(s, BEGIN_TOC, sizeof(BEGIN_TOC) - 1))) {
	  printf("<!--%s-->\n", BEGIN_TOC);
	  printf("<ul class=\"%s\">\n", toc_class);
	  level = toc_low;
	  toc(get_root(t), &level, &item_is_open, 1);
	  while (level > toc_low) {
	    printf(xml ? "</li>\n" : "\n");
	    indent(level - toc_low);
	    printf("</ul>");
	    level--;
	  }
	  if (item_is_open && xml) printf("</li>\n");
	  printf("</ul>\n");
	  printf("<!--%s-->", END_TOC);
	  if (!strncmp(s, BEGIN_TOC, sizeof(BEGIN_TOC) - 1))
	    *write = false;			/* Suppress old ToC */
	} else if (exp && !strncmp(s, END_TOC, sizeof(END_TOC) - 1)) {
	  *write = true;
	} else {
	  printf("<!--%s-->", h->text);
	}
	break;
      case Declaration:
	printf("<!DOCTYPE %s", h->name);
	if (h->text) printf(" PUBLIC \"%s\"", h->text);
	if (h->url) printf(" %s\"%s\"", h->text ? "" : "SYSTEM ", h->url);
	printf(">");
	break;
      case Procins:
	if (*write) printf("<?%s>", h->text);
	break;
      case Element:
	if (use_div && is_div(h) && has_heading(h)) {
	  /* It's a section element with a heading as first child */
	  div_depth++;
	  level = div_depth;
	} else {
	  /* Check if the element is a heading and what its level is */
	  level = heading_level(h);
	  if (level && use_div && div_parent(h)) level = 0;
	}
	/* Give DIVs and headers an ID, if they need one */
	if (level >= toc_low && level <= toc_high) {
	  if (!get_attrib(h, "id")) set_attrib(h, "id", gen_id(h));
	}
	if (*write) {
	  if (flatten && ((s = get_attrib(h, "alt")))) {
	    /* Flatten: use ALT attribute instead of element */
	    printf("%s", s);
	  } else if (flatten && !eq(h->name, "bdo")
		     && ((s = get_attrib(h, "dir")))) {
	    /* Flatten: keep DIR attributes */
	    printf("<span dir=\"%s\">", s);
	    expand(h, write, exp, false, div_depth, flatten);
	    printf("</span>");
	  } else if (flatten && !eq(h->name, "bdo")) {
	    /* Flatten: remove all elements except BDO */
	    expand(h, write, exp, false, div_depth, flatten);
	  } else if (! keep_anchors && eq(h->name, "a")) {
	    /* Don't write the <a> and </a> tags */
	    expand(h, write, exp, false, div_depth, flatten);
	  } else if (! keep_anchors && eq(h->name, "span")
		     && has_class(h->attribs, INDEX)) {
	    /* Don't write <span.index>...</span> tags */
	    expand(h, write, exp, false, div_depth, flatten);
	  } else if (! keep_anchors && eq(h->name, "dfn")) {
	    /* Don't copy dfn tags to the ToC */
	    expand(h, write, exp, false, div_depth, flatten);
	  } else if (eq(h->name, "a") && (has_class(h->attribs, TARGET)
		     || has_class(h->attribs, TOC))) {
	    /* This <a> was inserted by toc itself; remove it */
	    expand(h, write, exp, false, div_depth, flatten);
	  } else {
	    printf("<%s", h->name);
	    for (a = h->attribs; a != NULL; a = a->next) {
	      if (keep_anchors || !eq(a->name, "id")) {
		/* If we don't keep anchors, we don't keep IDs either */
		printf(" %s", a->name);
		if (a->value != NULL) printf("=\"%s\"", a->value);
	      }
	    }
	    if (is_empty(h->name)) {
	      assert(h->children == NULL);
	      printf(xml ? " />" : ">");
	    } else {
	      printf(">");
	      /* Insert an <A NAME> if element has an ID and is not <A> */
	      if (bctarget && is_mixed(h->name) && (val = get_attrib(h, "id"))
		  && !eq(h->name, "a") && ! xml)
		printf("<a class=\"%s\" name=\"%s\"></a>", TARGET, val);
	      expand(h, write, exp, keep_anchors, div_depth, flatten);
	      printf("</%s>", h->name);
	    }
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
  errexit("Version %s\nUsage: %s [-l low] [-h high] [-x] [-t] [-d] [-c class] [html-file]\n",
	  VERSION, name);
}


int main(int argc, char *argv[])
{
  int c, status;
  bool write = true;

  while ((c = getopt(argc, argv, "l:h:xtdc:f")) != -1) {
    switch (c) {
    case 'l': toc_low = atoi(optarg); break;
    case 'h': toc_high = atoi(optarg); break;
    case 'x': xml = true; break;
    case 't': bctarget = false; break;
    case 'd': use_div = true; break;
    case 'c': toc_class = newstring(optarg); break;
    case 'f': do_flatten = true; break;
    default: usage(argv[0]);
    }
  }
  if (toc_low < 1) toc_low = 1;

  if (argc > optind + 1) {
    usage(argv[0]);
  } else if (optind >= argc || eq(argv[optind], "-")) {
    yyin = stdin;
  } else if (!(yyin = fopenurl(argv[optind], "r", &status))) {
    perror(argv[optind]); exit(2);
  } else if (status != 200) {
    errexit("%s : %s\n", argv[optind], http_strerror(status));
  }

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

  if (yyparse() != 0) exit(3);

  tree = get_root(tree);
  expand(tree, &write, EXPAND, KEEP_ANCHORS, 1, DONT_FLATTEN);
#if 0
  tree_delete(tree);				/* Just to test memory mgmt */
#endif
  return 0;
}
