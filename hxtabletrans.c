/*
 * hxtabletrans -- transpose an HTML or XHTML table
 *
 * The input is an HTML or XHTML table, the output is the same table
 * transposed, i.e., rows become columns and columns become rows, as
 * if the table was flipped along the main diagonal.
 *
 * Any TBODY, THEAD, COLGROUP are lost. Comments and processing
 * instructions occurring outside of table cells are also lost.
 *
 * TODO: reinsert comments (at some reasonable place).
 *
 * TODO: handle rows that have too few cells. (Requires adding up
 * colspans and rowspans, possibly discounting overlap because of
 * incorrect rowspans.)
 *
 * TODO: Turn thead, tfoot and tbody into colgroup?
 *
 * Copyright © 2012 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 *
 * Author: Bert Box <bert@w3.org>
 * Created: 23 Oct 2012
 */
#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#if HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif
#include <ctype.h>
#include "heap.e"
#include "types.e"
#include "tree.e"
#include "scan.e"
#include "html.e"
#include "dict.e"
#include "openurl.e"

static Tree tree;
static bool has_error = false;
static bool do_xml = false;
static bool do_content = false;	/* Whether to try and transpose characters */


/*======================== Parser callbacks =======================*/


/* handle_error -- called when a parse error occurred */
void handle_error(void *clientdata, const string s, int lineno)
{
  warnx("%s:%d: %s\n", get_yyin_name(), lineno, s);
  has_error = true;
}


/* start -- called before the first event is reported */
/* void* handle_start(void) */


/* end -- called after the last event is reported */
/* void handle_end(void *clientdata) */


/* handle_comment -- called after a comment is parsed */
void handle_comment(void *clientdata, const string commenttext)
{
  tree = append_comment(tree, commenttext);
}


/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, const string text)
{
  tree = append_text(tree, text);
}


/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, const string gi, const string fpi,
		 const string url)
{
  tree = append_declaration(tree, gi, fpi, url);
}


/* handle_pi -- called after a Processing Instruction is parsed */
void handle_pi(void *clientdata, const string pi_text)
{
  tree = append_procins(tree, pi_text);
}


/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, const string name, pairlist attribs)
{
  tree = html_push(tree, name, attribs);
  free(name);
}


/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, const string name, pairlist attribs)
{
  tree = html_push(tree, name, attribs);
  free(name);
}


/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, const string name)
{
  tree = html_pop(tree, name);
  free(name);
}


/* handle_endincl -- called at the end of an included file */
/* void handle_endincl(void *clientdata) */


/*==================== End of parser callbacks ====================*/


/* print_start -- print a start tag */
static void print_start(Tree t)
{
  pairlist p;

  printf("<%s", t->name);
  for (p = t->attribs; p; p = p->next)
    if (p->value) printf(" %s=\"%s\"", p->name, p->value);
    else printf(" %s", p->name);
  if (do_xml && is_empty(t->name)) printf("/>"); else printf(">");
}


/* print_tree -- print the subtree t */
static void print_tree(Tree t)
{
  if (t) {
    switch (t->tp) {
    case Element:
      print_start(t);
      if (!is_empty(t->name)) {
	print_tree(t->children);
	printf("</%s>", t->name);
      }
      print_tree(t->sister);
      break;
    case Text:
      printf("%s", t->text);
      print_tree(t->sister);
      break;
    case Procins:
      printf("<?%s>", t->text);
      print_tree(t->sister);
      break;
    case Declaration:
      printf("<!DOCTYPE %s", t->name);
      if (t->text && t->url) printf(" PUBLIC \"%s\" \"%s\"", t->text, t->url);
      else if (t->text) printf(" PUBLIC \"%s\"", t->text);
      else if (t->url)  printf(" SYSTEM \"%s\"", t->url);
      printf(">");
      print_tree(t->sister);
      break;
    case Comment:
      printf("<!--%s-->", t->text);
      print_tree(t->sister);
      break;
    case Root:
    default:
      assert(!"Cannot happen!");
      break;
    }
  }
}


/* find_all_rows -- create an array pointing to the 1st child of each row */
static int find_all_rows(Tree t, Tree **rows)
{
  int nrows = 0;
  Tree g, h;

  assert(t->tp == Element && eq(t->name, "table"));
  *rows = NULL;
  for (h = t->children; h; h = h->sister) {
    if (h->tp != Element) continue;
    if (eq(h->name, "tr")) {
      renewarray(*rows, ++nrows);
      (*rows)[nrows-1] = h->children;
    } else if (eq(h->name, "thead") ||
	       eq(h->name, "tbody") ||
	       eq(h->name, "tfoot")) {
      for (g = h->children; g; g = g->sister) {
	if (h->tp != Element) continue;
	assert(eq(g->name, "tr"));
	renewarray(*rows, ++nrows);
	(*rows)[nrows-1] = g->children;
      }
    } else {
      assert(eq(h->name, "colgroup") || eq(h->name, "col"));
    }
  }
  return nrows;
}


/* transpose_char -- if c has a transposed form, return it; otherwise NULL */
static conststring transpose_char(conststring *c)
{
  static conststring pairs[] = {
    "⋮", "⋯",
    "…", "︙",
    "←", "↑",
    "→", "↓",
    "⇐", "⇑",
    "⇒", "⇓",
    "⇠", "⇡",
    "⇢", "⇣",
    "⇦", "⇧",
    "⇨", "⇩",
    "⇄", "⇅",
    "⇆", "⇅",
    "⇇", "⇈",
    "⇉", "⇊",
    NULL,
  };
  int i;

  for (i = 0; pairs[i]; i++)
    if (hasprefix(*c, pairs[i])) {
      *c += strlen(pairs[i]);
      return pairs[i % 2 ? i - 1 : i + 1];
    }
  return NULL;
}


/* appendc -- reallocate and append one character */
static void appendc(string *s, int c)
{
  int i;

  if (!s) {new(s); *s = NULL;}
  i = *s ? strlen(*s) : 0;
  renewarray(*s, i + 2);
  (*s)[i] = c;
  (*s)[i+1] = '\0';
}


/* transpose_content -- try to transpose character content, true if success */
static bool transpose_content(const Tree t, Tree *result)
{
  Tree q = NULL, r;
  conststring h, c;
  string s = NULL;

  *result = NULL;
  if (!t) return true;
  if (t->tp == Element && !transpose_content(t->children, &q)) return false;
  if (!transpose_content(t->sister, &r)) {tree_delete(q); return false;}
  if (t->tp == Text) {
    for (h = t->text; *h;)
      if (isspace(*h)) appendc(&s, *(h++));
	else if ((c = transpose_char(&h))) strapp(&s, c, NULL);
	else {dispose(s); tree_delete(q); tree_delete(r); return false;}
  }
  assert(t->tp != Root);
  new(*result);
  (*result)->tp = t->tp;
  (*result)->parent = t->parent;
  (*result)->children = q;
  (*result)->sister = r;
  if (t->tp == Comment || t->tp == Procins || t->tp == Declaration)
    (*result)->text = newstring(t->text);
  else
    (*result)->text = s;
  if (t->tp == Declaration) (*result)->url = newstring(t->url);
  if (t->tp == Element || t->tp == Declaration)
    (*result)->name = newstring(t->name);
  if (t->tp == Element) (*result)->attribs = pairlist_copy(t->attribs);
  return true;
}


/* trans -- print the transposition of table t */
static void trans(const Tree t)
{
  Tree *rows;
  int nrows, i;
  pairlist p;

  static Node _SPANNED;
  static Tree SPANNED = &_SPANNED;
  Tree **table, h;
  int j, k, l, colspan, rowspan, maxcols, *ncols;
  conststring s;
  Tree transposed;

  assert(t->tp == Element && eq(t->name, "table"));
  nrows = find_all_rows(t, &rows);

  /* Find the maximum number of columns */
  /* TODO: deal with overlapping cells */
  newarray(ncols, nrows);
  for (i = 0; i < nrows; i++) ncols[i] = 0;

  for (i = 0; i < nrows; i++) {
    for (h = rows[i]; h; h = h->sister) {
      assert(h->tp == Element || h->tp == Comment || h->tp == Procins);
      if (h->tp == Element) {
	assert(eq(h->name, "td") || eq(h->name, "th"));
	if (! (s = get_attrib(h, "colspan"))) colspan = 1;
	else if ((colspan = atoi(s)) < 1) colspan = 1;
	if (! (s = get_attrib(h, "rowspan"))) rowspan = 1;
	else if ((rowspan = atoi(s)) < 1) rowspan = 1;
	for (j = i; j < i + rowspan && j < nrows; j++) ncols[j] += colspan;
      }
    }
  }
  for (maxcols = 0, i = 0; i < nrows; i++)
    if (maxcols < ncols[i]) maxcols = ncols[i];

  /* Make an empty table */
  newarray(table, nrows);
  for (i = 0; i < nrows; i++) {
    newarray(table[i], maxcols);
    for (j = 0; j < maxcols; j++) table[i][j] = NULL;
  }

  /* Put the cells at their correct coordinates in the table */
  for (i = 0; i < nrows; i++) {
    for (j = 0, h = rows[i]; h; h = h->sister) {
      if (h->tp == Element) {
	while (table[i][j]) j++; /* Find next free cell in this row */
	assert(j < maxcols);
	if (! (s = get_attrib(h, "colspan"))) colspan = 1;
	else if ((colspan = atoi(s)) < 1) colspan = 1;
	if (! (s = get_attrib(h, "rowspan"))) rowspan = 1;
	else if ((rowspan = atoi(s)) < 1) rowspan = 1;
	for (k = i; k < i + rowspan; k++)
	  for (l = j; l < j + colspan; l++)
	    table[k][l] = SPANNED; /* Mark cells as occupied */
	table[i][j] = h;
	j += colspan;
      }
    }
  }

  /* Print the table transposed */
  print_start(t);
  printf("\n");

  for (j = 0; j < maxcols; j++) {
    printf("<tr>\n");
    for (i = 0; i < nrows; i++) {
      if (table[i][j] == NULL) {
	printf("<td></td>\n");
      } else if (table[i][j] != SPANNED) {
	/* Swap colspan and rowspan attributes */
	for (p = table[i][j]->attribs; p; p = p->next)
	  if (eq(p->name, "colspan")) strcpy(p->name, "rowspan");
	  else if (eq(p->name, "rowspan")) strcpy(p->name, "colspan");
	print_start(table[i][j]);
	if (do_content && transpose_content(table[i][j]->children, &transposed))
	  print_tree(transposed);
	else
	  print_tree(table[i][j]->children);
	printf("</%s>\n", table[i][j]->name);
      }
    }
    printf("</tr>\n");
  }
  printf("</table>\n");
}


/* transpose -- look for the first table in the tree and output it transposed */
static bool transpose(const Tree t)
{
  if (!t) return false;
  else if (t->tp == Element && eq(t->name, "table")) {trans(t); return true;}
  else if (transpose(t->sister)) return true;
  else return transpose(t->children);
}


/* usage -- print usage message and exit */
static void usage(const string prog)
{
  fprintf(stderr, "Usage: %s [-c] [-x] [-v] [file-or-url]\n", prog);
  exit(1);
}


/* main -- main body */
int main(int argc, char *argv[])
{
  int c, status = 200;

  /* Parse command line */
  while ((c = getopt(argc, argv, "cxv")) != -1)
    switch (c) {
    case 'c': do_content = true; break;
    case 'x': do_xml = true; break;
    case 'v': printf("Version: %s %s\n", PACKAGE, VERSION); return 0;
    default: usage(argv[0]);
    }

  if (optind == argc)
    set_yyin(stdin, "stdin");
  else if (optind == argc - 1)
    set_yyin(fopenurl(argv[optind], "r", &status), argv[optind]);
  else
    usage(argv[0]);

  if (!yyin) err(1, "%s", argv[argc-1]);
  if (status != 200) errx(1, "%s: %s", argv[argc-1], http_strerror(status));

  /* Bind the parser callback routines to our handlers */
  set_error_handler(handle_error);
  set_comment_handler(handle_comment);
  set_text_handler(handle_text);
  set_decl_handler(handle_decl);
  set_pi_handler(handle_pi);
  set_starttag_handler(handle_starttag);
  set_emptytag_handler(handle_emptytag);
  set_endtag_handler(handle_endtag);

  /* Parse input and transpose it as a table, if possible */
  tree = create();
  if (yyparse() != 0) exit(1);
  if (!transpose(get_root(tree))) errx(1, "Found no table to transpose");

  return has_error ? 1 : 0;
}
