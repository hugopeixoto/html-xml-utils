/*
 * Number headers. Counters are inserted at the start
 * of H1 to H6. CLASS="no-num" suppresses numbering for
 * that heading.
 *
 * Copyright © 1994-2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Bert Bos
 * Created Sep 1997
 * $Id: hxnum.c,v 1.12 2018/08/02 01:20:14 bbos Exp $
 */
#include "config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <ctype.h>
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
#include <stdlib.h>
#include <assert.h>
#include "export.h"
#include "types.e"
#include "class.e"
#include "html.e"
#include "scan.e"
#include "dict.e"
#include "openurl.e"
#include "errexit.e"

#define SECNO "secno"				/* class attribute */
#define NO_NUM "no-num"				/* class-attribute */
#define OPTS ":l:h:n:1:2:3:4:5:6:?"		/* Command line options */

static int h[] = {-1, 0, 0, 0, 0, 0, 0};	/* Counters for each level */
static int low = 1;				/* First counter to use */
static int high = 6;				/* Last counter to use */
static string format[7] = {			/* Format for each counter */
  NULL, "%d. ", "%d.%d. ", "%d.%d.%d. ", "%d.%d.%d.%d. ",
  "%d.%d.%d.%d.%d. ", "%d.%d.%d.%d.%d.%d. "};
static int skipping = 0;			/* >0 to suppress output */


/* romannumeral -- generate roman numeral for 1 <= n <= 4000 */
static char* romannumeral(int n)
{
  static char buf[30];
  int len = 0;

  while (n >= 1000) {buf[len++] = 'M'; n -= 1000;}
  if (n >= 500) {buf[len++] = 'D'; n -= 500;}
  while (n >= 100) {buf[len++] = 'C'; n -= 100;}
  if (n >= 50) {buf[len++] = 'L'; n -= 50;}
  while (n >= 10) {buf[len++] = 'X'; n -= 10;}
  if (n >= 9) {buf[len++] = 'I'; buf[len++] = 'X'; n -= 9;}
  if (n >= 5) {buf[len++] = 'V'; n -= 5;}
  if (n >= 4) {buf[len++] = 'I'; buf[len++] = 'V'; n -= 4;}
  while (n >= 1) {buf[len++] = 'I'; n -= 1;}
  buf[len] = '\0';
  return buf;
}

/* --------------- implements interface api.h -------------------------- */

/* handle_error -- called when a parse error occurred */
void handle_error(void *clientdata, const string s, int lineno)
{
  fprintf(stderr, "%d: %s\n", lineno, s);
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
  printf("<!--%s-->", commenttext);
}

/* handle_text -- called after a text chunk is parsed */
void handle_text(void *clientdata, string text)
{
  if (skipping == 0) fputs(text, stdout);
}

/* handle_decl -- called after a declaration is parsed */
void handle_decl(void *clientdata, const string gi,
		 const string fpi, const string url)
{
  if (skipping == 0) {
    printf("<!DOCTYPE %s", gi);
    if (fpi) printf(" PUBLIC \"%s\"\n", fpi);
    if (url) printf(" %s\"%s\"", fpi ? "" : "SYSTEM ", url);
    printf(">");
  }
}

/* handle_pi -- called after a PI is parsed */
void handle_pi(void *clientdata, string pi_text)
{
  if (skipping == 0) printf("<?%s>", pi_text);
}

/* handle_starttag -- called after a start tag is parsed */
void handle_starttag(void *clientdata, string name, pairlist attribs)
{
  pairlist p;
  int lev, i;
  string s;

  /* Skip everything inside <span class=secno> */
  if (skipping > 0) {
    skipping++;
    return;
  }

  /* Check for old counters, skip them */
  if (strcasecmp(name, "span") == 0 && has_class(attribs, SECNO)) {
    skipping = 1;
    return;
  }

  /* Print tag and attributes */
  printf("<%s", name);
  for (p = attribs; p != NULL; p = p->next) {
    printf(" %s", p->name);
    if (p->value != NULL) printf("=\"%s\"", p->value);
  }
  printf(">");

  /* If header, insert counters */
  if (eq("h1", name) || eq("H1", name)) lev = 1;
  else if (eq("h2", name) || eq("H2", name)) lev = 2;
  else if (eq("h3", name) || eq("H3", name)) lev = 3;
  else if (eq("h4", name) || eq("H4", name)) lev = 4;
  else if (eq("h5", name) || eq("H5", name)) lev = 5;
  else if (eq("h6", name) || eq("H6", name)) lev = 6;
  else lev = 0;

  /* Don't number headers with class "no-num" */
  if (lev > 0 && has_class(attribs, NO_NUM)) lev = 0;

  if (low <= lev && lev <= high) {
    h[lev]++;
    for (i = lev + 1; i <= high; i++) h[i] = 0;
    printf("<span class=\"%s\">", SECNO);
    for (i = low, s = format[lev]; *s; s++) {
      if (*s == '%') {
	s++;
	switch (*s) {
	  case 'n': i++; break;			/* No number */
	  case 'd': printf("%d", h[i++]); break; /* Decimal */
	  case 'a': printf("%c", 'a' + (h[i++] - 1)); break; /* Lowercase */
	  case 'A': printf("%c", 'A' + (h[i++] - 1)); break; /* Uppercase */
	  case 'i': printf("%s", down(romannumeral(h[i++]))); break;
	  case 'I': printf("%s", romannumeral(h[i++])); break; /* Roman */
	  default: putchar(*s);			/* Escaped char */
	}
      } else {
	putchar(*s);
      }
    }
    printf("</span>");
  }
}

/* handle_emptytag -- called after an empty tag is parsed */
void handle_emptytag(void *clientdata, string name, pairlist attribs)
{
  pairlist p;

  if (skipping == 0) {
    printf("<%s", name);
    for (p = attribs; p != NULL; p = p->next) {
      printf(" %s", p->name);
      if (p->value != NULL) printf("=\"%s\"", p->value);
    }
    printf(" />");
  }
}

/* handle_endtag -- called after an endtag is parsed (name may be "") */
void handle_endtag(void *clientdata, string name)
{
  if (skipping == 0) printf("</%s>", name);
  else skipping--;
}

/* --------------------------------------------------------------------- */

/* usage -- print usage message and exit */
static void usage(string prog)
{
  fprintf(stderr, "Version %s\n\
Usage: %s [-l low] [-h high] [-1 format] [-2 format] [-3 format]\n\
  [-4 format] [-5 format] [-6 format] [html-file]\n", VERSION, prog);
  exit(2);
}

/* help -- print help */
static void help(void)
{
  printf("Version %s\n", VERSION);
  printf("Options:\n");
  printf("  -l low     lowest header level to number (1-6) [default 1]\n");
  printf("  -h high    highest header level to number (1-6) [default 6]\n");
  printf("  -n start   number of first heading [default: 1]\n");
  printf("  -1 format  format for level 1 [default \"%%d. \"]\n");
  printf("  -2 format  format for level 2 [default \"%%d.%%d. \"]\n");
  printf("  -3 format  format for level 3 [default \"%%d.%%d.%%d. \"]\n");
  printf("  -4 format  format for level 4 [default \"%%d.%%d.%%d.%%d. \"]\n");
  printf("  -5 format  format for level 5 [default \"%%d.%%d.%%d.%%d.%%d. \"]\n");
  printf("  -6 format  format for level 6 [default \"%%d.%%d.%%d.%%d.%%d.%%d. \"]\n");
  printf("  -?         this help\n");
  printf("The format strings may contain:\n");
  printf("  %%d  replaced by decimal number\n");
  printf("  %%a  replaced by letter a, b, c,..., z\n");
  printf("  %%A  replaced by letter A, B, C,..., Z\n");
  printf("  %%i  replaced by lowercase roman numeral i, ii, iii,...\n");
  printf("  %%I  replaced by roman numeral I, II, III,...\n");
  printf("  %%n  replaced by nothing, but skips a level\n");
  printf("  %%%%  replaced by a %%\n");
  printf("The first %% in the format is replaced by the counter for level\n");
  printf("low, the second by the counter for low+1, etc.\n");
  exit(0);
}

int main(int argc, char *argv[])
{
  int i, status = 200, c;

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

  /* First find -l and -h */
  while ((c = getopt(argc, argv, OPTS)) != -1) {
    switch (c) {
    case 'l': low = atoi(optarg); break;
    case 'h': high = atoi(optarg); break;
    default: /* skip */;
    }
  }
  if (low < 1 || low > 6 || high < 1 || high > 6) usage(argv[0]);

#ifdef HAVE_GETOPT_OPTRESET
  optreset = 1;
#endif
  optind = 1;

  /* If -l and/or -h have been set, the default formats are different */
  if (low != 1 || high != 6) {
    for (i = high; i >= low; i--) format[i] = format[i-low+1];
    for (i = high + 1; i <= 6; i++) format[i] = "";
  }

  /* Then treat other options */
  while ((c = getopt(argc, argv, OPTS)) != -1) {
    switch (c) {
    case 'l': break;		/* Already handled */
    case 'h': break;		/* Already handled */
    case 'n': h[low] = atoi(optarg) - 1; break;
    case '1': format[1] = optarg; break;
    case '2': format[2] = optarg; break;
    case '3': format[3] = optarg; break;
    case '4': format[4] = optarg; break;
    case '5': format[5] = optarg; break;
    case '6': format[6] = optarg; break;
    case '?': help(); break;
    default: usage(argv[0]);
    }
  }

  if (optind == argc) yyin = stdin;
  else if (optind == argc - 1 && eq(argv[optind], "-")) yyin = stdin;
  else if (optind == argc - 1) yyin = fopenurl(argv[optind], "r", &status);
  else usage(argv[0]);

  if (yyin == NULL) {perror(argv[optind]); exit(1);}
  if (status != 200) errexit("%s : %s\n", argv[optind], http_strerror(status));

  if (yyparse() != 0) {
    exit(3);
  }
  return 0;
}
