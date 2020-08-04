/*
 * Insert an index between "<!--begin-index-->" and "<!--end-index-->",
 * or replacing the comment "<!--index-->"
 *
 * The index links to elements with ID attributes as well as with
 * empty <A NAME> elements.
 *
 * Any <A> tags with a class of "bctarget" are not copied, but
 * regenerated. They are assumed to be backwards-compatible versions
 * of ID attributes on their parent elements. But if the option -t or
 * -x are given, those <A> elements are removed.
 *
 * There's a limit of 100000 index terms (10^(MAXIDLEN-1)).
 *
 * Index terms are elements with a class of "index", "index-inst" or
 * "index-def", as well as all <dfn> elements. The contents of the
 * element is the index term, unless the element has a title
 * attribute. The title attribute can contain "|" and "!!":
 *
 * "term"
 * "term1|term2|term3|..."
 * "term!!subterm!!subsubterm!!..."
 * "term1!!subterm1|term2!!subterm2|..."
 * etc.
 *
 * For backward compatibility with an earlier Perl program, "::" is
 * accepted as an alternative for "!!", but it is better not to use
 * both separators in the same project, since the sorting maybe
 * adversely affected.
 *
 * Class "index-def" results in a bold entry in the index, "index" in
 * a normal one. "index-inst" is an alias for "index", provided for
 * backward compatibility.
 *
 * To do: an option to split the index at each new first letter.
 *
 * Copyright © 1994-2005 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 11 Apr 2000
 * Version: $Id: hxindex.c,v 1.24 2018/02/23 19:05:04 bbos Exp $
 *
 **/
#include "config.h"
#include <assert.h>
#include <locale.h>
#include <wchar.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include <unistd.h>
#include <err.h>
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
#else
extern int errno;
char *strerror(int errnum);
int strerror_r(int errnum, char *buf, size_t n);
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
#include "genid.e"
#include "class.e"

#undef USE_DATA_ATTRIBUTE	/* Data attributes are a proposal in HTML5 */

#define BEGIN_INDEX "begin-index" /* <!--begin-index--> */
#define END_INDEX "end-index"	/* <!--end-index--> */
#define INDEX "index"		/* <!--index--> */
#define INDEX_INST "index-inst"	/* class="index-inst" */
#define INDEX_DEF "index-def"	/* class="index-def" */
#define TARGET "bctarget"	/* CLASS="...bctarget..." */

#define MAXSUBS 20		/* Max. depth of subterms */
#define SECNO "secno"		/* Class of elements that define section # */
#define NO_NUM "no-num"		/* Class of elements without a section # */

typedef struct _indexterm {
  string url;
  int importance;		/* 1 (low) or 2 (high) */
  string secno;			/* For option -n */
  string sectitle;		/* For option -N */
  string doctitle;
  string *terms;		/* Array of subterms */
  wchar_t **sortkeys;		/* Array of normalized subterms */
  int nrkeys;			/* Length of term and sortkeys arrays */
} *Indexterm;

static Tree tree;
static bool xml = false;	/* Use <empty /> convention */
static string base = NULL;	/* (Rel.) URL of output file */
static string indexdb = NULL;	/* Persistent store of terms */
static string* userclassnames = NULL;	/* Persistent store of class names */
static FILE *globalfile;	/* Must be global for twalk */
static Indexterm globalprevious; /* Must be global for twalk */
static string globalurlprevious;/* Must be global for twalk */
static bool bctarget = true;	/* Add <A name=> after IDs */
static bool use_secno = false;	/* Anchor text is "#" instead of section # */
static bool use_sectitle = false; /* Anchor text is section title, not # */
static bool final = false;	/* Leave used attributes in document */
static bool trim_punct = true;	/* Remove trailing punctuation from terms */
static string section_name = NULL; /* Term meaning "section %s" */
static string unknown_name = NULL; /* Term meaning "without number" */
static string* exclude_elts = NULL; /* Don't index these elements */
static string* only_elts = NULL; /* Only index these elements */


/* handle_error -- called when a parse error occurred */
static void handle_error(void *clientdata, const string s, int lineno)
{
  (void) fprintf(stderr, "%d: %s\n", lineno, s);
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
  conststring id;

  tree = html_push(tree, name, attribs);

  /* If it has an ID, store it (so we don't accidentally generate it) */
  if ((id = pairlist_get(attribs, "id"))) storeID(id);
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
static void handle_endtag(void *clientdata, string name)
{
  tree = html_pop(tree, name);
}

/* trim -- remove leading and trailing white space, collapse white space */
static void trim(string s)
{
  string t;
  int i, j;

  if (!s) return;
  t = newstring(s);
  for (i = 0; isspace(t[i]); i++); /* Skip leading white space */
  for (j = 0; t[i]; i++)
    if (!isspace(t[i])) s[j++] = t[i];
    else if (!isspace(t[i-1])) s[j++] = ' ';
  if (j == 0) s[j] = '\0';
  else if (isspace(s[j-1])) s[j-1] = '\0';
  else s[j] = '\0';
  dispose(t);
}

/* parse_subterms -- parse s to create terms & sortkeys array in an Indexterm */
static void parse_subterms(const Indexterm term, const conststring s)
{
  enum {TEXT, TAG, DQUOTE, SQUOTE} state;
  string h, k, p, q;
  iconv_t cd;
  size_t len, len2;
  int i;

  /* Create the terms array and count the number of subterms */
  h = newstring(s);
  trim(h);
  term->nrkeys = 1;
  newarray(term->terms, 1);
  term->terms[0] = h;
  while ((k = strstr(h, "!!")) || (k = strstr(h, "::"))) {
    h = k + 2;
    *k = '\0';
    renewarray(term->terms, term->nrkeys + 1);
    trim(h);				/* Remove leading & trailing space */
    term->terms[term->nrkeys] = h;	/* All terms point into h */
    term->nrkeys++;
  }

  /* Create the sortkeys array by normalizing each term */
  newarray(term->sortkeys, term->nrkeys);
  for (i = 0; i < term->nrkeys; i++) {

    /* First remove mark-up and expand the standard XML entities */
    h = newstring(term->terms[i]);
    for (state = TEXT, p = q = h; *p; p++) {
      switch (state) {
      case TEXT:
	if (*p == '<') state = TAG;
	else if (hasprefix(p, "&lt;"))   {*(q++) = '<'; p += 3;}
	else if (hasprefix(p, "&#60;"))  {*(q++) = '<'; p += 4;}
	else if (hasprefix(p, "&#x3c;")) {*(q++) = '<'; p += 5;}
	else if (hasprefix(p, "&#x3C;")) {*(q++) = '<'; p += 5;}
	else if (hasprefix(p, "&gt;"))   {*(q++) = '>'; p += 3;}
	else if (hasprefix(p, "&#62;"))  {*(q++) = '>'; p += 4;}
	else if (hasprefix(p, "&#x3e;")) {*(q++) = '>'; p += 5;}
	else if (hasprefix(p, "&#x3E;")) {*(q++) = '>'; p += 5;}
	else if (hasprefix(p, "&quot;")) {*(q++) = '"'; p += 5;}
	else if (hasprefix(p, "&#34;"))  {*(q++) = '"'; p += 4;}
	else if (hasprefix(p, "&#x22;")) {*(q++) = '"'; p += 5;}
	else if (hasprefix(p, "&amp;"))  {*(q++) = '&'; p += 4;}
	else if (hasprefix(p, "&#38;"))  {*(q++) = '&'; p += 4;}
	else if (hasprefix(p, "&#x26;")) {*(q++) = '&'; p += 5;}
	else *(q++) = tolower(*p);
	break;
      case TAG:
	if (*p == '>') state = TEXT;
	else if (*p == '"') state = DQUOTE;
	else if (*p == '\'') state = SQUOTE;
	break;
      case DQUOTE:
	if (*p == '"') state = TAG;
	break;
      case SQUOTE:
	if (*p == '\'') state = TAG;
	break;
      default:
	assert(!"Cannot happen!");
      }
    }
    *q = '\0';

    if (trim_punct) {
      /* Remove some trailing white space and punctuation */
      for (q--; q != h && strspn(q, " \r\n\t\f\v,:;!?"); q--) *q = '\0';

      /* Remove final '.' only if it is the only '.' in the term */
      if ((q = strrchr(h, '.')) && !*(q+1) && q == strchr(h, '.')) *q = '\0';
    }

    /* Then convert from UTF-8 to wchar_t */
    cd = iconv_open("wchar_t", "UTF-8");
    if (cd == (iconv_t)(-1)) {perror("hxindex"); exit(1);}
    len = strlen(h) + 1;
    newarray(term->sortkeys[i], len); /* Large enough */
    p = (string) term->sortkeys[i];
    len2 = len * sizeof(term->sortkeys[i][0]);
    if (iconv(cd, &h, &len, &p, &len2) == (size_t)(-1)) {
      perror("hxindex"); exit(1);
    }
    /* *p = L'\0'; */
    if (iconv_close(cd) == -1) {perror("hxindex"); exit(1);}
  }
}

/* folding_cmp -- compare two arrays of sort keys */
static int folding_cmp(wchar_t **a, const int alen, wchar_t **b, const int blen)
{
  int i, j;

  assert(a && alen >= 0);
  assert(b && blen >= 0);
  for (i = 0;; i++) {
    if (i == alen) return i == blen ? 0 : -1;
    if (i == blen) return 1;
    assert(a[i] && b[i]);
    if ((j = wcscoll(a[i], b[i])) != 0) return j;
  }
  assert(! "Cannot happen!");
}

/* indent -- print newline and n times 2 spaces */
static void indent(int n)
{
  putchar('\n');
  for (; n > 0; n--) printf("  ");
}

/* print_escaped -- print s escaped for use in an attribute */
static void print_escaped(const conststring s)
{
  conststring h;

  for (h = s; *h; h++) if (*h == '"') printf("&quot;"); else putchar(*h);
}

/* print_title -- print a TITLE attribute */
static void print_title(const Indexterm term)
{
  enum {TEXT, TAG, DQUOTE, SQUOTE} state;
  string h;

  assert(use_secno);
  fputs(" title=\"", stdout);
  if (base[0]) {		/* Only add document titles if needed */
    for (state = TEXT, h = term->doctitle; *h; h++) {
      switch (state) {
      case TEXT:
	if (*h == '<') state = TAG;
	else if (*h == '"') fputs("&quot;", stdout);
	else putchar(*h);
	break;
      case TAG:
	if (*h == '>') state = TEXT;
	else if (*h == '"') state = DQUOTE;
	else if (*h == '\'') state = SQUOTE;
	break;
      case DQUOTE:
	if (*h == '"') state = TAG;
	break;
      case SQUOTE:
	if (*h == '\'') state = TAG;
	break;
      default:
	assert(!"Cannot happen!");
      }
    }
    fputs(", ", stdout);
  }
  for (h = section_name; *h; h++)
    switch (*h) {
    case '"': fputs("&quot;", stdout); break;
    case '>': fputs("&gt;", stdout); break;
    case '<': fputs("&lt;", stdout); break;
    case '%':
      if (*(h+1) == '%') {putchar('%'); h++;}
      else if (*(h+1) != 's') putchar('%');
      else if (term->secno) {print_escaped(term->secno); h++;}
      else {print_escaped(unknown_name); h++;}
      break;
    default: putchar(*h);
    }
  putchar('"');
}

/* write_index_item -- write one item in the list of index terms */
static void write_index_item(const void *term1, const VISIT which,
			     const int depth)
{
  Indexterm term = *(Indexterm*)term1;
  int i, j;

  if (which != postorder && which != leaf) return;

  /* Count how many subterms are equal to the previous entry */
  i = 0;
  while (i < min(term->nrkeys, globalprevious->nrkeys) &&
	 !folding_cmp(term->sortkeys + i, 1, globalprevious->sortkeys + i, 1))
    i++;

  /* Close lists as needed */
  for (j = globalprevious->nrkeys - 1; j > i; j--) {
    indent(j);
    printf("</ul>");
  }

  /* Open a list if needed */
  if (term->nrkeys > globalprevious->nrkeys && globalprevious->nrkeys == i) {
    indent(i);
    printf("<ul>");
  }

  /* Print new subterms, if any */
  for (j = i; j < term->nrkeys; j++) {
    indent(j);
    printf("<li>%s", term->terms[j]);
    if (j != term->nrkeys - 1) {
      indent(j + 1);
      printf("<ul>");
    }
  }

  /* Print a link */
  printf(", ");
  printf("<a href=\"");
  print_escaped(term->url);
  printf("\"");
  if (use_secno) print_title(term);
  if (term->importance == 2) printf("><strong>"); else printf(">");
  if (use_sectitle)
    printf("%s", term->sectitle ? term->sectitle : term->doctitle);
  else if (!use_secno) putchar('#');
  else if (term->secno) print_escaped(term->secno);
  else print_escaped(unknown_name);
  if (term->importance == 2) printf("</strong></a>"); else printf("</a>");

  /* Remember this term */
  globalprevious = term;
  globalurlprevious = term->url;

}

/* mkindex -- write out an index */
static void mkindex(Indexterm terms)
{
  int i;

  printf("<ul class=\"indexlist\">");

  /* Initialize globalprevious to a term with an unlikely sortkey */
  new(globalprevious);
  globalprevious->nrkeys = 1;
  newarray(globalprevious->sortkeys, globalprevious->nrkeys);
  newarray(globalprevious->sortkeys[0], 15);
  wcscpy(globalprevious->sortkeys[0], L"zzzzzzzzzzzzzz");

  twalk(terms, write_index_item);

  /* Close all open lists */
  for (i = 0; i < globalprevious->nrkeys; i++) printf("\n</ul>");
}

/* expand -- write the tree, add <A NAME> if needed and replace <!--index--> */
static void expand(Tree t, bool *write, Indexterm terms)
{
  conststring val;
  Tree h;
  pairlist a;
  string s;
  bool do_tag;

  for (h = t->children; h != NULL; h = h->sister) {
    switch (h->tp) {
      case Text:
	if (*write) printf("%s", h->text);
	break;
      case Comment:
	s = newstring(h->text);
	trim(s);
	if (eq(s, INDEX) || eq(s, BEGIN_INDEX)) {
	  if (!final) printf("<!--%s-->\n", BEGIN_INDEX);
	  mkindex(terms);
	  if (!final) printf("<!--%s-->", END_INDEX);
	  if (eq(s, BEGIN_INDEX)) *write = false;	/* Skip old index */
	} else if (eq(s, END_INDEX)) {
	  *write = true;
	} else {
	  printf("<!--%s-->", h->text);
	}
	dispose(s);
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
	if (*write) {
	  /* If an <a> was inserted by index itself, remove it */
	  do_tag = !eq(h->name, "a") || !has_class(h->attribs, TARGET);
	  if (do_tag) {
	    printf("<%s", h->name);
	    for (a = h->attribs; a != NULL; a = a->next) {
	      printf(" %s", a->name);
	      if (a->value != NULL) printf("=\"%s\"", a->value);
	    }
	    assert(! is_empty(h->name) || h->children == NULL);
	    printf(xml && is_empty(h->name) ? " />" : ">");
	    /* Insert an <A NAME> if element has an ID and is not <A> */
	    if (bctarget && is_mixed(h->name) && (val = get_attrib(h, "id"))
		&& !eq(h->name, "a") && ! xml)
	      printf("<a class=\"%s\" name=\"%s\"></a>", TARGET, val);
	  }
	  expand(h, write, terms);
	  if (do_tag && ! is_empty(h->name)) printf("</%s>", h->name);
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

/* termcmp -- comparison routine for Indexterms */
static int termcmp(const void *a1, const void *b1)
{
  Indexterm a = (Indexterm)a1, b = (Indexterm)b1;
  int r;

  assert(a);
  assert(b);
  assert(a->sortkeys);
  assert(b->sortkeys);
  assert(a->nrkeys > 0);
  assert(b->nrkeys > 0);

  r = folding_cmp(a->sortkeys, a->nrkeys, b->sortkeys, b->nrkeys);
  if (r != 0) return r;
  return strcmp(a->url, b->url); /* Terms are equal, compare URL instead */
}

/* copy_contents -- recursively expand contents of element t into a string */
static void copy_contents(Tree t, string *s)
{
  Tree h;
  int i;
  pairlist a;
  string p;

  for (h = t->children; h != NULL; h = h->sister) {
    switch (h->tp) {
      case Text:
	i = *s ? strlen(*s) : 0;
	renewarray(*s, i + strlen(h->text) + 1);
	/* Copy, but transform all whitespace to spaces */
	for (p = h->text; *p; p++, i++) (*s)[i] = isspace(*p) ? ' ' : *p;
	(*s)[i] = '\0';
	break;
      case Comment: break;
      case Declaration: break;
      case Procins: break;
      case Element:
	/* Only certain tags are retained */
	if (eq(h->name, "span") || eq(h->name, "code") || eq(h->name, "tt")
	    || eq(h->name, "acronym") || eq(h->name, "abbr")
	    || eq(h->name, "bdo") || eq(h->name, "kbd") || eq(h->name, "samp")
	    || eq(h->name, "sub") || eq(h->name, "sup")
	    || eq(h->name, "var")) {
	  strapp(s, "<", h->name, NULL);
	  for (a = h->attribs; a != NULL; a = a->next) {
	    if (! a->value) strapp(s, " ", a->name, NULL);
	    else strapp(s, " ", a->name, "=\"", a->value, "\"", NULL);
	  }
	  assert(! is_empty(h->name) || h->children == NULL);
	  if (is_empty(h->name)) {
	    strapp(s, xml ? " />" : ">", NULL);
	  } else {
	    strapp(s, ">", NULL);
	    copy_contents(h, s);
	    strapp(s, "</", h->name, ">", NULL);
	  }
	} else {				/* Ignore tag, copy contents */
	  copy_contents(h, s);
	}
	break;
      case Root: assert(! "Cannot happen"); break;
      default: assert(! "Cannot happen");
    }
  }
}

/* copy_to_index -- copy the contents of element h to the index db */
static void copy_to_index(Tree t, Indexterm *terms, int importance,
			  conststring secno, conststring sectitle,
			  conststring doctitle)
{
  conststring id, title;
  string h;
  Indexterm term;
  int i, n;

  id = get_attrib(t, "id");
#ifdef USE_DATA_ATTRIBUTE
  if (! (title = get_attrib(t, "data-index")))
#endif
    title = get_attrib(t, "title");

  /* Get term either from title attribute or contents */
  if (title) {

    i = 0;
    while (title[i]) {
      n = strcspn(title + i, "|");		/* Find | or \0 */
      new(term);
      term->importance = importance;
      term->secno = secno ? newstring(secno) : NULL;
      term->sectitle = sectitle ? newstring(sectitle) : NULL;
      term->doctitle = newstring(doctitle);
      term->url = NULL;
      strapp(&term->url, base, "#", id, NULL);
      h = newnstring(title + i, n);
      parse_subterms(term, h);
      if (! tsearch(term, (void**)terms, termcmp))
	errx(1, "Out of memory while parsing term %s\n", h);
      i += n;
      if (title[i]) i++;			/* Skip '|' */
    }
    if (final)					/* Remove used attribute */
#ifdef USE_DATA_ATTRIBUTE
      if (!delete_attrib(t, "data-index"))
#endif
	delete_attrib(t, "title");

  } else {					/* Recursively copy contents */

    h = NULL;
    copy_contents(t, &h);
    if (h) {					/* Non-empty contents */
      new(term);
      term->importance = importance;
      term->secno = secno ? newstring(secno) : NULL;
      term->sectitle = sectitle ? newstring(sectitle) : NULL;
      term->doctitle = newstring(doctitle);
      term->url = NULL;
      strapp(&term->url, base, "#", id, NULL);
      parse_subterms(term, h);
      if (! tsearch(term, (void**)terms, termcmp))
	errx(1, "Out of memory while parsing term %s", h);
    }

  }
}

/* in_list -- check if word occurs in array list */
static bool in_list(const string word, const string *list)
{
  int i;

  for (i = 0; list[i]; i++) if (eq(word, list[i])) return true;
  return false;
}

/* collect -- collect index terms, add IDs where needed */
static void collect(Tree t, Indexterm *terms, string *secno,
		    string *sectitle, string *doctitle)
{
  int importance;
  Tree h;

  for (h = t->children; h != NULL; h = h->sister) {
    switch (h->tp) {
      case Text: case Comment: case Declaration: case Procins: break;
      case Element:
	if (eq(h->name, "title")) {
	  dispose(*doctitle);
	  copy_contents(h, doctitle);
	}
	if (has_class(h->attribs, SECNO)) {
	  dispose(*secno);
	  copy_contents(h, secno);
	  trim(*secno);
	} else if (has_class(h->attribs, NO_NUM)) {
	  dispose(*secno);
	  *secno = newstring(unknown_name);
	}
	if (eq(h->name, "h1") || eq(h->name, "h2") || eq(h->name, "h3") ||
	    eq(h->name, "h4") || eq(h->name, "h5") || eq(h->name, "h5")) {
	  dispose(*sectitle);
	  copy_contents(h, sectitle);
	  trim(*sectitle);
	}
	if (eq(h->name, "dfn")) importance = 2;
	else if (exclude_elts && in_list(h->name, exclude_elts)) importance = 0;
	else if (only_elts && !in_list(h->name, only_elts)) importance = 0;
	else if (has_class(h->attribs,INDEX)||has_class(h->attribs,INDEX_INST))
	  importance = 1;
	else if (userclassnames && has_class_in_list(h->attribs, userclassnames))
	  importance = 1;
	else if (has_class(h->attribs, INDEX_DEF)) importance = 2;
	else importance = 0;
	if (importance != 0) {
	  /* Give it an ID, if it doesn't have one */
	  if (! get_attrib(h, "id")) set_attrib(h, "id", gen_id(h));
	  copy_to_index(h, terms, importance, *secno, *sectitle, *doctitle);
	} else {
	  collect(h, terms, secno, sectitle, doctitle);
	}
	break;
      case Root: assert(! "Cannot happen"); break;
      default: assert(! "Cannot happen");
    }
  }
}

/* load_index -- read persistent term db from file */
static void load_index(const string indexdb, Indexterm *terms)
{
  FILE *f;
  int n1, n2, n3, n4, n5, n6;
  char *line = NULL;
  size_t linesize = 0;
  Indexterm term;
  string h;

  if (! (f = fopen(indexdb, "r"))) return;	/* Assume file not found... */

  while (getline(&line, &linesize, f) != -1) {
    n1 = strcspn(line, "\t");
    if (line[n1] != '\t') errx(1, "Illegal syntax in %s", indexdb);
    n2 = n1 + 1 + strcspn(line + n1 + 1, "\t");
    if (line[n2] != '\t') errx(1, "Illegal syntax in %s", indexdb);
    n3 = n2 + 1 + strcspn(line + n2 + 1, "\t");
    if (line[n3] != '\t') errx(1, "Illegal syntax in %s", indexdb);
    n4 = n3 + 1 + strcspn(line + n3 + 1, "\t");
    if (line[n4] != '\t') errx(1, "Illegal syntax in %s", indexdb);
    n5 = n4 + 1 + strcspn(line + n4 + 1, "\t");
    if (line[n5] != '\t') errx(1, "Illegal syntax in %s", indexdb);
    n6 = n5 + 1 + strcspn(line + n5 + 1, "\t\n");
    if (line[n6] != '\n') errx(1, "Illegal syntax in %s", indexdb);
    new(term);
    h = newnstring(line, n1);
    switch (line[n1 + 1]) {
      case '1': term->importance = 1; break;
      case '2': term->importance = 2; break;
    default: errx(1, "Error in %s (column 2 must be '1' or '2')", indexdb);
    }
    term->url = newnstring(line + n2 + 1, n3 - n2 - 1);
    term->secno = newnstring(line + n3 + 1, n4 - n3 - 1);
    term->sectitle = newnstring(line + n4 + 1, n5 - n4 - 1);
    term->doctitle = newnstring(line + n5 + 1, n6 - n5 - 1);
    parse_subterms(term, h);
    if (! tsearch(term, (void**)terms, termcmp))
      errx(1, "Out of memory while loading %s", indexdb);
  }

  fclose(f);
  free(line);
}

/* save_a_term -- write one term to globalfile */
static void save_a_term(const void *term1, const VISIT which, const int dp)
{
  Indexterm term = *(Indexterm*)term1;
  int i;

  if (which == endorder || which == leaf) {
    for (i = 0; i < term->nrkeys; i++) {
      if (i > 0) fprintf(globalfile, "!!");
      fprintf(globalfile, "%s", term->terms[i]);
    }
    fprintf(globalfile, "\t%d\t%s\t%s\t%s\t%s\n", term->importance, term->url,
	    term->secno ? term->secno : (use_secno ? unknown_name : "#"),
	    term->sectitle ? term->sectitle : term->doctitle,
	    term->doctitle);
  }
}

/* save_index -- write terms to file */
static void save_index(const string indexdb, Indexterm terms)
{
  if (! (globalfile = fopen(indexdb, "w")))
    errx(1, "%s: %s", indexdb, strerror(errno));
  twalk(terms, save_a_term);
  fclose(globalfile);
}

/* usage -- print usage message and exit */
static void usage(string name)
{
  errx(1, "Version %s\nUsage: %s [-i indexdb] [-b base] [-x] [-t] [-n] [-c userclass] [-s template] [-u phrase] [html-file]",
	  VERSION, name);
}

/* tokenize -- split string s into tokens at each comma, return an array */
static string * tokenize(string s)
{
  string * t;
  int i, n;

  assert(s && s[0]);
  for (t = NULL, n = 0; *s; s += i + 1, n++) {
    i = strcspn(s, ",");
    renewarray(t, n + 1);
    t[n] = newnstring(s, i);
  }
  renewarray(t, n + 1);		/* Make final item NULL */
  t[n] = NULL;
  return t;
}

/* main */
int main(int argc, char *argv[])
{
  bool write = true;
  Indexterm termtree = NULL;	/* Sorted tree of terms */
  string secno, doctitle, sectitle;
  int c, status = 200;

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

  yyin = NULL;

  while ((c = getopt(argc, argv, "txb:i:cnNfrs:u:O:X:")) != -1)
  switch (c) {
  case 't': bctarget = false; break; /* Don't write <a name> after each ID */
  case 'x': xml = true; break;	/* Output as XML */
  case 'b': base = newstring(optarg); break; /* Set base of URL */
  case 'i': indexdb = newstring(optarg); break;	/* Set name of index db */
  case 'c': userclassnames = tokenize(optarg); break; /* Set class names */
  case 'n': use_secno = true; use_sectitle = false; break;
  case 'N': use_sectitle = true; use_secno = false; break;
  case 'f': final = true; break; /* "Final": remove used attributes */
  case 'r': trim_punct = false; break; /* Do not remove trailing punctuation */
  case 's': section_name = newstring(optarg); break;
  case 'u': unknown_name = newstring(optarg); break;
  case 'O': only_elts = tokenize(optarg); break; /* Index only these elements */
  case 'X': exclude_elts = tokenize(optarg); break; /* Don't index these */
  default: usage(argv[0]);
  }
  if (optind == argc) yyin = stdin;
  else if (argc > optind + 1) usage(argv[0]);
  else if (eq(argv[optind], "-")) yyin = stdin;
  else yyin = fopenurl(argv[optind], "r", &status);

  if (yyin == NULL) {perror(argv[optind]); exit(1);}
  if (status != 200) errx(1, "%s : %s", argv[optind], http_strerror(status));

  if (!base) base = newstring("");
  if (!section_name) section_name = newstring("section %s");
  if (!unknown_name) unknown_name = newstring("??");

  /* Apply user's locale */
  setlocale(LC_ALL, "");

  /* Read the index DB into memory */
  if (indexdb) load_index(indexdb, &termtree);

  /* Parse, build tree, collect existing IDs */
  if (yyparse() != 0) exit(3);

  /* Scan for index terms, add them to the tree, add IDs where needed */
  secno = NULL;
  sectitle = NULL;
  doctitle = newstring("");
  collect(get_root(tree), &termtree, &secno, &sectitle, &doctitle);

  /* Write out the document, adding <A NAME> and replacing <!--index--> */
  expand(get_root(tree), &write, termtree);

  /* Store terms to file */
  if (indexdb) save_index(indexdb, termtree);

  fclose(yyin);
  return 0;
}
