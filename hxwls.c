/*
 * List all links from the given document.
 *
 * Copyright © 1994-2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Bert Bos <bert@w3.org>
 * Created 31 July 1999
 * $Id: hxwls.c,v 1.13 2019/08/28 19:14:34 bbos Exp $
 */
#include "config.h"
#include <assert.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
# ifndef HAVE_STRDUP
#  include "strdup.e"
# endif
#endif
#include "export.h"
#include "heap.e"
#include "types.e"
#include "html.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "url.e"
#include "errexit.e"
#include "unent.e"

static bool has_error = false;
static string base = NULL;
static string self;
static enum {Short, Long, HTML, Tuple} format = Short;	/* Option -l -h -t */
static bool relative = false;		/* Option -r */
static bool ascii = false;		/* Option -a */


/* append_utf8 -- add UTF-8 bytes for code n at s, return end of string */
static string append_utf8(string s, const unsigned long n)
{
  /* We assume s is long enough */
  if (n <= 0x7F) {
    *(s++) = n;
  } else if (n <= 0x7FF) {
    *(s++) = 0xC0 | (n >> 6);
    *(s++) = 0x80 | (n & 0x3F);
  } else if (n <= 0xFFFF) {
    *(s++) = 0xE0 | (n >> 12);
    *(s++) = 0x80 | ((n >> 6) & 0x3F);
    *(s++) = 0x80 | (n & 0x3F);
  } else if (n <= 0x1FFFFF) {
    *(s++) = 0xF0 | (n >> 18);
    *(s++) = 0x80 | ((n >> 12) & 0x3F);
    *(s++) = 0x80 | ((n >> 6) & 0x3F);
    *(s++) = 0x80 | (n & 0x3F);
  } else if (n <= 0x3FFFFFF) {
    *(s++) = 0xF0 | (n >> 24);
    *(s++) = 0x80 | ((n >> 18) & 0x3F);
    *(s++) = 0x80 | ((n >> 12) & 0x3F);
    *(s++) = 0x80 | ((n >> 6) & 0x3F);
    *(s++) = 0x80 | (n & 0x3F);
  } else {
    *(s++) = 0xF0 | (n >> 30);
    *(s++) = 0x80 | ((n >> 24) & 0x3F);
    *(s++) = 0x80 | ((n >> 18) & 0x3F);
    *(s++) = 0x80 | ((n >> 12) & 0x3F);
    *(s++) = 0x80 | ((n >> 6) & 0x3F);
    *(s++) = 0x80 | (n & 0x3F);
  }
  return s;
}


/* output -- print the link (lowercases rel argument) */
static void output(const conststring type, const conststring rel,
		   conststring url)
{
  string h = NULL, q, r, rel1;
  conststring p, s;
  const struct _Entity *e;
  unsigned long c;

  if (url) {					/* If we found a URL */

    /* Replace entities. */
    h = newnstring(url, 2 * strlen(url));	/* Reserve sufficient space */
    for (p = url, q = h; *p; p++) {
      if (*p != '&') {
	*(q++) = *p;
      } else if (*(p+1) == '#') {		/* Numeric entity */
	if (*(p+2) == 'x') c = strtoul(p + 3, &r, 16);
	else c = strtoul(p + 2, &r, 10);
	if (c > 0 && c <= 2147483647) q = append_utf8(q, c);
	p = *r == ';' ? r : r - 1;
      } else {					/* Entity */
	for (s = p + 1; isalnum(*s); s++);
	if (!(e = lookup_entity(p+1, s - (p+1)))) *(q++) = '&'; /* Unknown */
	else {q = append_utf8(q, e->code); p = *s == ';' ? s : s -1;}
      }
    }
    *q = '\0';
    url = h;

    /* Make URL absolute */
    if (! relative && base) {
      h = URL_s_absolutize(base, url);
      dispose(url);
      url = h;
    }
    /* Convert IRI to URL, if requested */
    if (ascii) {
      h = URL_s_to_ascii(url);
      dispose(url);
      url = h;
    }
    rel1 = newstring(rel ? rel : "");
    down(rel1);
    switch (format) {
      case HTML:
	printf("<li><a class=\"%s\" rel=\"%s\" href=\"%s\">%s</a></li>\n",
		 type, rel1, url, url);
	break;
      case Long:
	printf("%s\t%s\t%s\n", type, rel1, url);
	break;
      case Short:
	printf("%s\n", url);
	break;
      case Tuple:
	printf("%s\t%s\t%s\t%s\n", self, type, rel1, url);
	break;
      default:
	assert(!"Cannot happen!");
    }
    free(rel1);
    free(h);
  }
}


/* --------------- implements parser interface api------------------------- */

/* handle_error -- called when a parse error occurred */
void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
  has_error = true;
}

/* start -- called before the first event is reported */
void* start(void)
{
  if (format == HTML) {
    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\"\n");
    printf("  \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n");
    printf("<html>\n");
    printf("<head><title>Output of listlinks</title></head>\n");
    printf("<body>\n");
    printf("<ol>\n");
  }
  return NULL;
}

/* end -- called after the last event is reported */
void end(void *clientdata)
{
  if (format == HTML) {
    printf("</ol>\n");
    printf("</body>\n");
    printf("</html>\n");
  }
}

/* handle_comment -- called after a comment is parsed */
void handle_comment(void *clientdata, string commenttext)
{
  free(commenttext);
}

/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, string text)
{
  /* There may be several consecutive calls to this routine. */
  /* escape(text); */
  free(text);
}

/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, string gi, string fpi, string url)
{
  /* skip */
  if (gi) free(gi);
  if (fpi) free(fpi);
  if (url) free(url);
}

/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  if (pi_text) free(pi_text);
}

/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  /* ToDo: print text of anchor, if available */
  conststring h;

  if (strcasecmp(name, "base") == 0) {
    h = pairlist_get(attribs, "href");
    if (h) base = strdup(h);			/* Use as base from now on */
    output("base", NULL, h);
  } else if (strcasecmp(name, "link") == 0) {
    output("link", pairlist_get(attribs, "rel"), pairlist_get(attribs, "href"));
  } else if (strcasecmp(name, "a") == 0) {
    output("a", pairlist_get(attribs, "rel"), pairlist_get(attribs, "href"));
  } else if (strcasecmp(name, "img") == 0) {
    output("img", NULL, pairlist_get(attribs, "src"));
    output("img", "longdesc", pairlist_get(attribs, "longdesc"));
    output("img", "srcset", pairlist_get(attribs, "srcset"));
  } else if (strcasecmp(name, "input") == 0) {
    output("input", "src", pairlist_get(attribs, "src"));
  } else if (strcasecmp(name, "object") == 0) {
    output("object", NULL,  pairlist_get(attribs, "data"));
    output("object", "classid",  pairlist_get(attribs, "classid"));
    output("object", "codebase",  pairlist_get(attribs, "codebase"));
  } else if (strcasecmp(name, "area") == 0) {
    output("area", pairlist_get(attribs, "rel"), pairlist_get(attribs, "href"));
  } else if (strcasecmp(name, "ins") == 0) {
    output("ins", NULL, pairlist_get(attribs, "cite"));
  } else if (strcasecmp(name, "del") == 0) {
    output("del", NULL, pairlist_get(attribs, "cite"));
  } else if (strcasecmp(name, "q") == 0) {
    output("q", NULL, pairlist_get(attribs, "cite"));
  } else if (strcasecmp(name, "blockquote") == 0) {
    output("bq", NULL, pairlist_get(attribs, "cite"));
  } else if (strcasecmp(name, "form") == 0) {
    output("form", pairlist_get(attribs, "method"), pairlist_get(attribs, "action"));
  } else if (strcasecmp(name, "frame") == 0) {
    output("frame", NULL, pairlist_get(attribs, "src"));
  } else if (strcasecmp(name, "iframe") == 0) {
    output("iframe", NULL, pairlist_get(attribs, "src"));
  } else if (strcasecmp(name, "head") == 0) {
    output("head", NULL, pairlist_get(attribs, "profile"));
  } else if (strcasecmp(name, "script") == 0) {
    output("script", NULL, pairlist_get(attribs, "src"));
  } else if (strcasecmp(name, "body") == 0) {
    output("body", NULL, pairlist_get(attribs, "background"));
  } else if (strcasecmp(name, "video") == 0) {
    output("video", NULL, pairlist_get(attribs, "src"));
  } else if (strcasecmp(name, "audio") == 0) {
    output("audio", NULL, pairlist_get(attribs, "src"));
  } else if (strcasecmp(name, "source") == 0) {
    output("source", "srcset", pairlist_get(attribs, "srcset"));
    output("source", "src", pairlist_get(attribs, "src"));
  }

  /* Free memory */
  pairlist_delete(attribs);
  free(name);
}

/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  handle_starttag(clientdata, name, attribs);
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  free(name);
}

/* --------------------------------------------------------------------- */


/* usage -- print usage message and exit */
static void usage(string progname)
{
  fprintf(stderr,
	  "Version %s\nUsage: %s [-l] [-r] [-h] [-b base] [-t] [-a] [HTML-file]\n",
	  VERSION, progname);
  exit(1);
}


int main(int argc, char *argv[])
{
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

  /* Parse command line arguments */
  while ((c = getopt(argc, argv, "lb:rhta")) != -1) {
    switch (c) {
      case 'l': format = Long; break;		/* Long listing */
      case 'b': base = strdup(optarg); break;	/* Set base of URL */
      case 'r': relative = true; break;		/* Do not make URLs absolute */
      case 'h': format = HTML; break;		/* Output in HTML format */
      case 't': format = Tuple; break;		/* Output as 4-tuples */
      case 'a': ascii = true; break;		/* Convert IRIs to URLs */
      default: usage(argv[0]);
    }
  }

  if (optind == argc) {
    yyin = stdin;
    self = "-";
  } else if (optind == argc - 1) {
    if (!base) base = strdup(argv[optind]);
    if (eq(argv[optind], "-")) yyin = stdin;
    else yyin = fopenurl(argv[optind], "r", &status);
    self = argv[optind];
  } else {
    usage(argv[0]);
  }

  if (yyin == NULL) {perror(argv[optind]); exit(1);}
  if (status != 200) errexit("%s : %s\n", argv[optind], http_strerror(status));

  if (yyparse() != 0) exit(3);

  if (base) free(base);

  return has_error ? 1 : 0;
}
