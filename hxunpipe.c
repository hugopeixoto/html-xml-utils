/*
 * unpipe - takes output of pipe and convert to HTML/XML form
 *
 * Copyright © 1994-2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 23 May 1999
 * Version: $Id: hxunpipe.c,v 1.11 2017/11/24 09:50:25 bbos Exp $
 */
#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <assert.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif
#include "export.h"
#include "types.e"
#include "heap.e"
#include "errexit.e"
#include "dict.e"
#include "openurl.e"

static int nrattrs = 0;
static char **attrs = NULL;
static bool escape = false;

/* put_text -- replace newlines and print text */
static void put_text(FILE *in)
{
  int c, c1, c2;

  while ((c = getc(in)) != EOF && c != '\n')
    if (c != '\\') {
      if (!escape) putchar(c);
      else if (c == '<') printf("&lt;");
      else if (c == '>') printf("&gt;");
      else if (c == '"') printf("&quot;");
      else if (c == '&') printf("&amp;");
      else if (c == '\'') printf("&apos;");
      else putchar(c);
    }
    else if ((c = getc(in)) == EOF) return; /* Error */
    else if (c == '\n') return;		    /* Error */
    else if (c == 'n') putchar('\n');
    else if (c == 'r') putchar('\r');
    else if (c == 't') putchar('\t');
    else if (c == '#') printf("&#");
    else if (c < '0' || '7' < c) putchar(c);
    else if ((c1 = getc(in)) < '0' || '7' < c1) printf("%c%c", c, c1);
    else if ((c2 = getc(in)) < '0' || '7' < c2) printf("%c%c%c", c, c1, c2);
    else printf("&#%d;", 64 * (c - '0') + 8 * (c1 - '0') + (c2 - '0'));
}

/* store_attr -- store attributes temporarily */
static void store_attr(FILE *in)
{
# define INC 1014
  int c, n = 0;

  renewarray(attrs, nrattrs + 1);
  attrs[nrattrs] = NULL;
  renewarray(attrs[nrattrs], INC);
  while ((c = getc(in)) != EOF && c != '\n') {
    attrs[nrattrs][n++] = c;
    if (n % INC == 0) {renewarray(attrs[nrattrs], INC * (n/INC + 1));}
  }
  attrs[nrattrs][n] = '\0';
  nrattrs++;
}

/* put_attr -- write out attributes */
static void put_attr(void)
{
  int i, j;

  for (j = 0; j < nrattrs; j++) {
    for (i = 0; attrs[j][i] && attrs[j][i] != ' '; i++);
    if (attrs[j][i] != ' ') errexit("Incorrect A (attribute) line\n");
    if (! eq(attrs[j] + i + 1, "IMPLIED")) {
      putchar(' ');
      for (i = 0; attrs[j][i] && attrs[j][i] != ' '; i++) putchar(attrs[j][i]);
      if (attrs[j][i] != ' ') errexit("Incorrect A (attribute) line\n");
      putchar('=');
      for (i++; attrs[j][i] && attrs[j][i] != ' '; i++) ; /* skip type */
      if (attrs[j][i] != ' ') errexit("Incorrect A (attribute) line\n");
      putchar('"');
      for (i++; attrs[j][i]; i++) {
	if (attrs[j][i] != '\\') putchar(attrs[j][i]);
	else if (attrs[j][i+1]) {
	  i++;
	  if (attrs[j][i] == 'n') putchar('\n');
	  else if (attrs[j][i] == 'r') putchar('\r');
	  else if (attrs[j][i] == 't') putchar('\t');
	  else if (attrs[j][i] == '#') printf("&#");
	  else if ('0' <= attrs[j][i] && attrs[j][i] <= '7' &&
		   '0' <= attrs[j][i+1] && attrs[j][i+1] <= '7' &&
		   '0' <= attrs[j][i+2] && attrs[j][i+2] <= '7') {
	    printf("&#%d;", 64 * (attrs[j][i] - '0') +
		   8 * (attrs[j][i+1] - '0') + (attrs[j][i+2] - '0'));
	    i += 2;
	  } else putchar(attrs[j][i]);
	}
      }
      putchar('"');
    }
    dispose(attrs[j]);
  }
  nrattrs = 0;
}

/* put_decl -- write a DOCTYPE declaration */
static void put_decl(FILE *in)
{
  int c;
  bool hasfpi = false;

  printf("<!DOCTYPE ");

  /* Write name of root element */
  while ((c = getc(in)) != EOF && c != '\n' && c != ' ') putchar(c);

  /* Write FPI if present */
  while (c == ' ') c = getc(in);
  if (c == '"') {
    if ((c = getc(in)) == EOF || c == '\n')
      errexit("Incorrect DOCTYPE declaration\n");
    if (c != '"') {
      hasfpi = true;
      printf(" PUBLIC \"%c", c);
      while ((c = getc(in)) != EOF && c != '\n' && c != '"') putchar(c);
      if (c != '"') errexit("Incorrect DOCTYPE declaration\n");
      putchar('"');
    }
    c = getc(in);
  }

  /* Write URL if present */
  while (c == ' ') c = getc(in);
  if (c != EOF && c != '\n') {
    if (hasfpi) printf(" \"%c", c); else printf(" SYSTEM \"%c", c);
    while ((c = getc(in)) != EOF && c != '\n') putchar(c);
    putchar('"');
  }

  putchar('>');
}

/* usage -- print usage message and exit */
static void usage(string prog)
{
  fprintf(stderr, "Version %s\nUsage: %s [file_or_url]\n", VERSION, prog);
  exit(1);
}

int main(int argc, char *argv[])
{
  int c, status = 200;
  FILE *in = NULL;
  bool empty = false;

  while ((c = getopt(argc, argv, "b")) != -1)
    switch (c) {
      case 'b': escape = true; break;
      default: usage(argv[0]);
    }
  if (optind == argc) in = stdin;
  else if (optind == argc - 1) in = fopenurl(argv[optind], "r", &status);
  else usage(argv[0]);

  if (in == NULL) { perror(argv[optind]); exit(2); }
  if (status != 200) errexit("%s : %s\n", argv[optind], http_strerror(status));

  while ((c = getc(in)) != EOF) {
    switch (c) {
    case '-': put_text(in); break;
    case '?': printf("<?"); put_text(in); printf(">"); break;
    case '_': case '*': printf("<!--"); put_text(in); printf("-->"); break;
    case 'L': break;
    case 'A': store_attr(in); break;
    case '(': putchar('<'); put_text(in); put_attr(); putchar('>'); break;
    case ')':
      if (!empty) {printf("</"); put_text(in); putchar('>');}
      else empty = false;
      break;
    case '|': putchar('<'); put_text(in); put_attr(); printf(" />"); break;
    case '!': put_decl(in); break;
    case 'e': empty = true; break; /* Generated by onsgmls */
    case 'i': case 'o': break;	   /* Generated by onsgmls */
    case 'C': break;
    }
  }
  if (! feof(in)) { perror(argv[0]); exit(1); }
  fclose(in);
  return 0;
}
