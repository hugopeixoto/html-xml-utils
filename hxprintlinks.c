/*
 * Add a numbered list of links at the end of an HTML file
 *
 * Copyright © 2001-2015 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 *
 * Created 23 Jan 2015 (based on a Perl version from 1 Feb 2001)
 * Bert Bos <bert@w3.org>
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "types.e"
#include "dict.e"
#include "openurl.e"
#include "errexit.e"
#include "heap.e"
#include "html.e"
#include "scan.e"
#include "url.e"


static conststring attname[] = {	/* Attributes that contain URLs: */
  "src", "href", "data", "longdesc", "cite", "action", "profile",
  "background", "usemap", "classid", "codebase"};
static pairlist list = NULL;		/* Stored list of URLs */
static bool has_error = false;		/* Parsing errors occurred */
static conststring base = NULL;		/* Make URLs relative to this base */


/* pairlist_push -- insert a name/value pair at the start of a list */
static void pairlist_push(pairlist *p, const conststring name, const conststring val)
{
  pairlist h;

  new(h);
  h->name = newstring(name);
  h->value = newstring(val);
  h->next = *p;
  *p = h;
}


/* print_list_recursive -- print LI items for all entries in list */
static void print_list_recursive(const pairlist list)
{
  conststring url;

  /* ToDo: Escape double quotes */
  if (list) {
    print_list_recursive(list->next);
    url = base ? URL_s_absolutize(base, list->name) : list->name;
    printf("<li><a class=\"%s\" href=\"%s\">%s</a></li>\n",
	   list->value, url, url);
  }
}


/* print_list -- print an OL with the entries of list */
static void print_list(const pairlist list)
{
  if (list) {
    printf("<ol class=\"urllist\">\n");
    print_list_recursive(list);
    printf("</ol>\n");
  }
}


/* handle_error -- called when a parse error occurred */
void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
  has_error = true;
}


/* start -- called before the first event is reported */
void* start(void)
{
  return NULL;
}

  
/* end -- called after the last event is reported */
void end(void *clientdata)
{
  /* If we still have a list, print it here */
  if (list) {
    print_list(list);
    pairlist_delete(list);
    list = NULL;
  }
}


/* handle_comment -- called after a comment is parsed */
void handle_comment(void *clientdata, string commenttext)
{
  printf("<!--%s-->", commenttext);
}


/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, string text)
{
  printf("%s", text);
}


/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, string gi, string fpi,
		 string url)
{
  if (fpi && url)
    printf("<!DOCTYPE %s PUBLIC \"%s\" \"%s\">\n", gi, fpi, url);
  else if (fpi)
    printf("<!DOCTYPE %s PUBLIC \"%s\">\n", gi, fpi);
  else if (url)
    printf("<!DOCTYPE %s SYSTEM \"%s\">\n", gi, url);
  else
    printf("<!DOCTYPE %s>\n", gi);
}


/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  printf("<?%s>", pi_text);
}


/* print_attrs -- print attributes */
static void print_attrs(const pairlist attribs)
{
  pairlist p;

  /* ToDo: Distinguish SGML (a NULL value means that the name is the
     value and the actual attribute name is implicit) and XML? */
  for (p = attribs; p; p = p->next)
    printf(" %s=\"%s\"", p->name, p->value ? p->value : p->name);
}


/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  int i;
  conststring url;

  /* Store any URLs from attributes */
  for (i = 0; i < sizeof(attname)/sizeof(*attname); i++)
    if ((url = pairlist_get(attribs, attname[i])))
      pairlist_push(&list, url, attname[i]);

  printf("<%s", name);
  print_attrs(attribs);
  printf(">");
}


/* handle_emptytag -- called after an empty element is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  int i;
  conststring url;

  /* Store any URLs from attributes */
  for (i = 0; i < sizeof(attname)/sizeof(*attname); i++)
    if ((url = pairlist_get(attribs, attname[i])))
      pairlist_push(&list, url, attname[i]);

  printf("<%s", name);
  print_attrs(attribs);
  printf(" />");
}


/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  /* Just before </body>, print the list. or if we see </html> and
     haven't printed the list yet, print it there */
  if (list && (strcasecmp(name,"body") == 0 || strcasecmp(name,"html") == 0)) {
    print_list(list);
    pairlist_delete(list);
    list = NULL;
  }
  printf("</%s>", name);
}


/* usage -- print usage message and exit */
static void usage(string prog)
{
  fprintf(stderr, "Usage: %s [html-file-or-url]\n", prog);
  exit(2);
}


/* main -- main body */
int main(int argc, char *argv[])
{
  int c, status = 200;

  while ((c = getopt(argc, argv, "b:")) != -1)
    switch (c) {
    case 'b': base = optarg; break;
    default: usage(argv[0]);
    }

  if (optind < argc - 1) usage(argv[0]);
  else if (optind > argc - 1 || eq(argv[optind], "-")) yyin = stdin;
  else yyin = fopenurl(argv[optind], "r", &status);

  if (!yyin) {perror(argv[optind]); exit(2);}
  if (status != 200) errexit("%s : %s\n", argv[optind], http_strerror(status));

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

  if (yyparse() != 0) {exit(3);}
  return has_error ? 1 : 0;
}
