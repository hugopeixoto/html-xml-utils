/* hxcopy -- copy an HTML file and update relative URLs at the same time
 *
 * Copy an HTML file with all URLs that were relative to OLDURL
 * updated to be relative to NEWURL instead. (If the document has a
 * BASE element, only that is updated.) OLDURL and NEWURL may
 * themselves be relative (to the same base URL, which need not be
 * mentioned).
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * TO DO: Should it be an option whether URL references of the form
 * "", "#foo" and "?bar" are replaced by "oldurl", "oldurl#foo" and
 * "oldurl?bar"? (See adjust_url().)
 *
 * Created: 5 Dec 2008
 * Author: Bert Bos <bert@w3.org>
 *
 * Copyright © 2008-2012 W3C
 * See http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#if HAVE_STRING_H
#  include <string.h>
#endif
#if HAVE_STRINGS_H
#  include <strings.h>
#endif
#include "export.h"
#include "heap.e"
#include "types.e"
#include "html.e"
#include "scan.e"
#include "url.e"
#include "dict.e"
#include "openurl.e"
#include "errexit.e"

#define same(a, b) ((a) ? ((b) && eq((a), (b))) : !(b))

static bool has_errors = false;		/* Enconutered errors during parsing */
static FILE *out = NULL;		/* Where to write output */
static bool has_base = false;		/* Document has a <BASE> element */
static string newbase;			/* Path from OLDURL to NEWURL */
static bool replace_self = false;	/* Change link to self in link to old */


/* path_from_url_to_url -- compute URL that is path from one URL to another */
static string path_from_url_to_url(const conststring a, const conststring b)
{
  URL p, q;
  string s = NULL;
  char cwd[4096];
  int i, j;

  if (!getcwd(cwd, sizeof(cwd) - 1)) return NULL; /* To do: handle long path */
  strcat(cwd, "/");
  s = URL_s_absolutize(cwd, a); p = URL_new(s); dispose(s);
  s = URL_s_absolutize(cwd, b); q = URL_new(s); dispose(s);
  if (p->proto && !q->proto) {
    errno = EACCES;		/* Path from remote to local not possible */
  } else if (!same(p->proto, q->proto) ||
      !same(p->user, q->user) ||
      !same(p->password, q->password) ||
      !same(p->machine, q->machine) ||
      !same(p->port, q->port)) {
    s = newstring(b);		/* Just use the URL b */
  } else {
    /* Find the last '/' before which both paths are the same */
    for (j = i = 0; p->path[i] && q->path[i] && p->path[i] == q->path[i]; i++)
      if (p->path[i] == '/') j = i;

    /* Construct path from a to b by descending a and climbing b */
    for (i = j + 1; p->path[i]; i++)
      if (p->path[i] == '/') strapp(&s, "../", NULL);
    strapp(&s, q->path + j + 1, NULL);
  }
  URL_dispose(p);
  URL_dispose(q);
  return s;
}


/* adjust_url -- return a new URL relative to newurl instead of oldurl */
static conststring adjust_url(const conststring url)
{
  if (!replace_self && (!url || !url[0] || url[0] == '#' || url[0] == '?'))
    return url;			/* Don't replace references to self */
  else
    return URL_s_absolutize(newbase, url);
}


/* attribute_is_url -- check if the attribute is URL-valued */
static bool attribute_is_url(const conststring attrib)
{
  return strcasecmp(attrib, "href") == 0 ||
    strcasecmp(attrib, "src") == 0 ||
    strcasecmp(attrib, "action") == 0 ||
    strcasecmp(attrib, "background") == 0 ||
    strcasecmp(attrib, "cite") == 0 ||
    strcasecmp(attrib, "classid") == 0 ||
    strcasecmp(attrib, "codebase") == 0 ||
    strcasecmp(attrib, "data") == 0 ||
    strcasecmp(attrib, "longdesc") == 0 ||
    strcasecmp(attrib, "profile") == 0 ||
    strcasecmp(attrib, "usemap") == 0;
}


/* handle_error -- called when a parse error occurred */
void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
  has_errors = true;
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
  fprintf(out, "<!--%s-->", commenttext);
}


/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, string text)
{
  fprintf(out, "%s", text);
}


/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, string gi,
		 string fpi, string url)
{
  fprintf(out, "<!DOCTYPE %s", gi);
  if (fpi) fprintf(out, " PUBLIC \"%s\"", fpi);
  if (url) fprintf(out, " %s\"%s\"", fpi ? "" : "SYSTEM ", url);
  fprintf(out, ">");
}


/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  fprintf(out, "<?%s>", pi_text);
}


/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  conststring v;
  pairlist p;

  fprintf(out, "<%s", name);
  for (p = attribs; p; p = p->next) {
    fprintf(out, " %s", p->name);
    if (!p->value) v = NULL;
    else if (has_base) v = newstring(p->value);	/* No need to adjust */
    else if (attribute_is_url(p->name)) v = adjust_url(p->value);
    else v = newstring(p->value);		/* No need to adjust */
    if (v) fprintf(out, "=\"%s\"", v);
    dispose(v);
  }
  fprintf(out, ">");

  /* If this is a <BASE> tag, no further adjustments are needed */
  if (strcasecmp(name, "base") == 0) has_base = true;
}


/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  conststring v;
  pairlist p;

  fprintf(out, "<%s", name);
  for (p = attribs; p; p = p->next) {
    fprintf(out, " %s", p->name);
    if (!p->value) v = NULL;
    else if (has_base) v = newstring(p->value);	/* No need to adjust */
    else if (attribute_is_url(p->name)) v = adjust_url(p->value);
    else v = newstring(p->value);		/* No need to adjust */
    if (v) fprintf(out, "=\"%s\"", v);
    dispose(v);
  }
  fprintf(out, " />");

  /* If this is a <BASE> tag, no further adjustments are needed */
  if (strcasecmp(name, "base") == 0) has_base = true;
}


/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  fprintf(out, "</%s>", name);
}


/* usage -- print usage message and exit */
static void usage(const conststring progname)
{
  fprintf(stderr, "Usage: %s [-v] [-s] [-i old-URL] [-o new-URL] [URL [URL]]\n", progname);
  exit(1);
}


int main(int argc, char *argv[])
{
  int c, status = 200;
  string oldurl = NULL, newurl = NULL;

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
  while ((c = getopt(argc, argv, "i:o:sv")) != -1)
    switch (c) {
    case 'o': newurl = optarg; break;
    case 'i': oldurl = optarg; break;
    case 's': replace_self = true; break;
    case 'v': printf("Version: %s %s\n", PACKAGE, VERSION); return 0;
    default: usage(argv[0]);
    }
  if (argc > optind + 2) usage(argv[0]);
  if (argc > optind + 1) out =  fopenurl(argv[optind+1], "w", NULL);
  else if (newurl) out = stdout;
  else errexit("%s: option -o is required if output is to stdout\n", argv[0]);
  if (!out) {perror(argv[optind+1]); exit(3);}
  if (argc > optind) yyin = fopenurl(argv[optind], "r", &status);
  else if (oldurl) yyin = stdin;
  else errexit("%s: option -i is required if input is from stdin\n", argv[0]);
  if (!yyin) {perror(argv[optind]); exit(2);}
  if (status != 200) errexit("%s : %s\n", argv[1], http_strerror(status));
  if (!oldurl) oldurl = argv[optind];
  if (!newurl) newurl = argv[optind+1];
  newbase = path_from_url_to_url(newurl, oldurl);
  if (!newbase) errexit("%s: could not parse argument as a URL\n", argv[0]);
  if (yyparse() != 0) exit(4);
  return has_errors ? 1 : 0;
}
