/*
 * Program to (semi-)automatically link instances of terms and phrases
 * in an HTML file to their definitions.
 *
 * The program collects all <dfn> elements, and stores either their
 * title attribute, or if there is none, their content (without
 * mark-up). Then it looks for occurrences of the same text and makes
 * a link from the occurrence to the corresponding <dfn> element. The
 * occurrences that are checked are the contents of all inline
 * elements, such as <em> and <span>. HTML unfortunately forbids
 * nested links, so the program doesn't look for occurrences inside an
 * <a>.
 *
 * The program can store the <dfn> elements (the terms they define,
 * the file they occur in and their ID) in a file, so that
 * cross-references among several files are possible, by running the
 * program on each of the files. It may be necessary to run the
 * program twice on a series of files, to create all the references.
 *
 * Copyright © 2000-2012 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 4 August 2000
 * Version: $Id: hxref.c,v 1.14 2017/11/24 09:50:25 bbos Exp $
 **/

#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef HAVE_ERRNO_H
#  include <errno.h>
#endif
#ifdef HAVE_SEARCH_H
#  include <search.h>
#else
#  include "hash.e"
#endif

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
#include "heap.e"
#include "types.e"
#include "html.e"
#include "scan.e"
#include "tree.e"
#include "dict.e"
#include "openurl.e"
#include "genid.e"
#include "errexit.e"


/* Warning: arbitrary limit! */
#define MAXLINE 4096				/* Max. len. of url + term */
#define HASHSIZE 4096				/* Size of hash table */


static Tree tree;
static string base = NULL, progname;
static bool do_xml = false;
static bool use_language = false;
static char *extras = "-_@()";			/* Significant characters */


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
static void handle_decl(void *clientdata, string gi, string fpi, string url)
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


/* load_definitions -- read already defined terms from file */
static void load_definitions(FILE *f)
{
  char buf[MAXLINE];
  ENTRY entry;
  string h;

  while (fgets(buf, sizeof(buf), f)) {		/* Format is PHRASE\tURL\n */
    h = strchr(buf, '\t');
    if (! h) errexit("%s: index file not in correct format\n", progname);
    chomp(h);
    entry.key = newnstring(buf, h - buf);
    entry.data = newstring(h + 1);
    hsearch(entry, ENTER);
  }
}


/* get_contents -- collect all text content of an elt into a single string */
static string get_contents(Tree t)
{
  Node *h;
  string contents = NULL, k;

  assert(t->tp == Element);
  for (h = t->children; h; h = h->sister) {
    if (h->tp == Text) {
      strapp(&contents, h->text, NULL);
    } else if (h->tp == Element && !eq(h->name, "a") && !eq(h->name, "dfn")
	       && (k = get_contents(h))) {
      strapp(&contents, k, NULL);
      dispose(k);
    }
  }
  return contents;
}


/* normalize -- collapse whitespace, trim, lowercase (modifies s) */
static string normalize(string s)
{
  int i = 0, j;

  if (!s) return newstring("");

  for (j = 0; isspace(s[j]); j++) ;		/* Skip initial whitespace */

  for (; s[j]; j++)
    if (isupper(s[j])) s[i++] = tolower(s[j]);	/* Upper -> lowercase */
    else if (isalnum(s[j])) s[i++] = s[j];	/* Keep these */
    else if (strchr(extras, s[j])) s[i++] = s[j]; /* Keep these, too */
    else if (! isspace(s[j])) ;			/* Skip rest, except spaces */
    else if (s[i-1] != ' ') s[i++] = ' ';	/* Collapse whitespace */

  for (; i > 0 && s[i-1] == ' '; i--) ;		/* Remove trailing spaces */

  s[i] = '\0';
  return s;
}


/* search -- search a matching string in the hash table */
static ENTRY* search(string key, const conststring language)
{
  ENTRY entry, *e;
  int n;
  string t;

  /* Assumes key has already passed normalize() */

  /* First try the key as it is */
  entry.key = key;
  if ((e = hsearch(entry, FIND))) return e;

  /* Should we try language-specific modifications to the key? */
  if (!language || !use_language) return NULL;

  if (eq(language, "en") || hasprefix(language, "en-")) { /* English */

    /* Remove plural s */
    if ((n = strlen(key)) > 1 && key[n-1] == 's' && islower(key[n-2])) {
      t = newnstring(key, n - 1);
      entry.key = t;
      e = hsearch(entry, FIND);
      dispose(t);
      if (e) return e;
    }
    /* Remove plural es */
    if (n > 2 && key[n-1] == 's' && key[n-2] == 'e' && islower(key[n-3])) {
      t = newnstring(key, n - 2);
      entry.key = t;
      e = hsearch(entry, FIND);
      dispose(t);
      if (e) return e;
    }
    /* Replace plural ies by singular y */
    if (n > 3 && hasaffix(key, "ies") && islower(key[n-4])) {
      t = newnstring(key, n - 3);
      strapp(&t, "y", NULL);
      entry.key = t;
      e = hsearch(entry, FIND);
      dispose(t);
      if (e) return e;
    }
  }

  return NULL;
}


/* collect_terms -- walk the document tree looking for <dfn> elements */
static void collect_terms(Tree tree, FILE *db)
{
  conststring id, title;
  string url = NULL, s;
  ENTRY entry, *e;
  int i, n;
  Node *h;

  switch (tree->tp) {
    case Text:
    case Comment:
    case Declaration:
    case Procins:
      break;
    case Root:
      for (h = tree->children; h; h = h->sister) collect_terms(h, db);
      break;
    case Element:
      if (! eq(tree->name, "dfn")) {
	for (h = tree->children; h; h = h->sister) collect_terms(h, db);
      } else {
	if (! (id = get_attrib(tree, "id"))) {	/* Make sure there's an ID */
	  id = gen_id(tree);
	  set_attrib(tree, "id", id);
	}
	if ((title = get_attrib(tree, "title")))  /* Use title if it exists */
	  s = newstring(title);			/* Don't normalize yet */
	else					/* otherwise grab contents */
	  s = normalize(get_contents(tree));	/* Normalize, also removes "|" */

	entry.data = strapp(&url, base ? base : (string)"", "#", id, NULL);
	for (i = 0; s[i];) {			/* Loop over |-separated terms */
	  n = strcspn(s + i, "|");
	  entry.key = normalize(newnstring(s + i, n));
	  /* Add to hash table and to db file, if not already there */
	  if (! (e = hsearch(entry, FIND))
	      || ! eq((string)e->data, (string)entry.data)) {
	    hsearch(entry, ENTER);
	    if (db) fprintf(db, "%s\t%s\n", entry.key, (char*)entry.data);
	  }
	  i += n;
	  if (s[i]) i++;			/* Skip "|" */
	}
      }
      break;
    default:
      assert(!"Cannot happen");
  }
}


/* find_instances -- walk tree, make instances of defined terms into links */
static void find_instances(Tree tree, const conststring language)
{
  ENTRY *e;
  conststring title, lang;
  string key;

  if (!tree) return;

  switch (tree->tp) {
    case Text: case Comment: case Declaration: case Procins:
      find_instances(tree->sister, language);
      break;
    case Root:
      find_instances(tree->children, language);	/* Recurse over children */
      find_instances(tree->sister, language);	/* Recurse over siblings */
      break;
    case Element:
      if (!(lang = get_attrib(tree, "lang")) &&
	  !(lang = get_attrib(tree, "xml:lang")))
	lang = language;
      if (eq(tree->name, "a") || eq(tree->name, "dfn"))
	;					/* Don't descend into these */
      else if (eq(tree->name, "abbr") || eq(tree->name, "acronym")
	       || eq(tree->name, "b") || eq(tree->name, "bdo")
	       || eq(tree->name, "big") /*|| eq(tree->name, "cite")*/
	       || eq(tree->name, "code") || eq(tree->name, "del")
	       /*|| eq(tree->name, "dt")*/ || eq(tree->name, "em")
	       || eq(tree->name, "i") || eq(tree->name, "ins")
	       || eq(tree->name, "kbd") || eq(tree->name, "label")
	       || eq(tree->name, "legend") || eq(tree->name, "q")
	       || eq(tree->name, "samp") || eq(tree->name, "small")
	       || eq(tree->name, "span") || eq(tree->name, "strong")
	       || eq(tree->name, "sub") || eq(tree->name, "sup")
	       || eq(tree->name, "tt") || eq(tree->name, "var")) {
	if ((title = get_attrib(tree, "title"))) /* Use title if it exists */
	  key = newstring(title);
	else					/* Get flattened contents */
	  key = get_contents(tree);
	if (!(e = search(normalize(key), lang))) { /* If not an instance */
	  find_instances(tree->children, lang); /* Recurse over children */
	} else if (eq(tree->name, "span")) {	/* Found an instance */
	  rename_elt(tree, "a");		/* Turn the span into an a */
	  set_attrib(tree, "href", e->data);
	} else {
	  tree = wrap_elt(tree, "a", NULL);	/* Wrap element in an <a> */
	  set_attrib(tree, "href", e->data);
	}
	dispose(key);
      } else {					/* Not an inline element */
	find_instances(tree->children, lang);	/* Recurse over children */
      }
      find_instances(tree->sister, language);	/* Recurse over siblings */
      break;
    default:
      assert(!"Cannot happen");
  }
}


/* write_doc -- write the tree to a file */
static void write_doc(Tree n, bool do_xml, FILE *f)
{
  pairlist h;
  Tree l;

  switch (n->tp) {
    case Root:
      for (l = n->children; l; l = l->sister) write_doc(l, do_xml, f);
      break;
    case Text:
      fprintf(f, "%s", n->text);
      break;
    case Comment:
      fprintf(f, "<!--%s-->", n->text);
      break;
    case Declaration:
      fprintf(f, "<!DOCTYPE %s", n->name);
      if (n->text) fprintf(f, " PUBLIC \"%s\"", n->text);
      if (n->url) fprintf(f, " %s\"%s\"", n->text ? "" : "SYSTEM ", n->url);
      fprintf(f, ">");
      break;
    case Procins:
      fprintf(f, "<?%s>", n->text);
      break;
    case Element:
      fprintf(f, "<%s", n->name);
      for (h = n->attribs; h != NULL; h = h->next) {
	fprintf(f, " %s", h->name);
	if (h->value != NULL) fprintf(f, "=\"%s\"", h->value);
	else if (do_xml) fprintf(f, "=\"%s\"", h->name);
      }
      if (is_empty(n->name)) {
	assert(n->children == NULL);
	fprintf(f, do_xml ? " />" : ">");
      } else {
	fprintf(f, ">");
	for (l = n->children; l; l = l->sister) write_doc(l, do_xml, f);
	fprintf(f, "</%s>", n->name);
      }
      break;
    default:
      assert(!"Cannot happen");
  }
}


/* usage -- print usage message and exit */
static void usage(void)
{
  fprintf(stderr,
	  "Usage: %s [-v] [-b base] [-i index] [-x] [-l] [--] [input [output]]\n",
	  progname);
  exit(1);
}


/* main -- main body of xref */
int main(int argc, char *argv[])
{
  int i, status = 200;
  FILE *outfile = NULL, *db = NULL;

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

  /* Parse command line */
  progname = argv[0];
  yyin = NULL;
  for (i = 1; i < argc && argv[i][0] == '-' && !eq(argv[i], "--"); i++) {
    switch (argv[i][1]) {
      case 'b':
	if (!argv[i][2] && i + 1 == argc) usage(); /* Missing argument */
	if (base) usage();			/* Option was already set */
	base = argv[i][2] ? argv[i] + 2 : argv[++i];
	break;
      case 'x':
	if (do_xml) usage();			/* Option was already set */
	do_xml = true;
	break;
      case 'i':
	if (!argv[i][2] && i + 1 == argc) usage(); /* Missing argument */
	if (db) usage();			/* Index was already set */
	db = fopen(argv[i][2] ? argv[i] + 2 : argv[++i], "a+");
	if (! db) errexit("%s: %s\n", argv[i], strerror(errno));
	break;
      case 'l':
	if (use_language) usage(); 		/* Option was already set */
	use_language = true;
	break;
      case 'v':
	printf("Version: %s %s\n", PACKAGE, VERSION);
	return 0;
      case '\0':
	if (!yyin) yyin = stdin;
	else if (!outfile) outfile = stdout;
	else usage();				/* Was already set */
	break;
      default:
      usage();					/* Unknown option */
    }
  }
  if (i < argc && eq(argv[i], "--")) i++;

  if (i < argc) {
    if (yyin) usage();				/* Input was already set */
    if (eq(argv[i], "-")) yyin = stdin;
    else yyin = fopenurl(argv[i], "r", &status);
    if (! yyin) errexit("%s: %s\n", argv[i], strerror(errno));
    if (status != 200) errexit("%s : %s\n", argv[i], http_strerror(status));
  }
  if (++i < argc) {
    if (outfile) usage();			/* Output was already set */
    if (eq(argv[i], "-")) outfile = stdout;
    else outfile = fopen(argv[i], "w");
    if (! outfile) perror(argv[i]);
  }
  if (++i < argc) usage();			/* Too many args */

  if (! yyin) yyin = stdin;
  if (! outfile) outfile = stdout;

  if (! hcreate(HASHSIZE))
    errexit("%s: cannot create hash table (out of memory?)\n", argv[0]);

  if (db) {
    if (fseek(db, 0L, SEEK_SET) == -1)
      errexit("%s: %s\n", progname, strerror(errno));
    load_definitions(db);
  }

  if (yyparse() != 0) exit(3);
  
  tree = get_root(tree);
  collect_terms(tree, db);
  find_instances(tree, NULL);

  if (db) fclose(db);

  write_doc(tree, do_xml, outfile);

  return 0;
}
