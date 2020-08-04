/*
 *
 * Program to convert files from UTF-8 to ASCII, using the
 * &#-escapes from XML to escape non-ASCII characters.
 *
 * Usage:
 *
 *   xml2asc
 *
 * Reads from stdin and write to stdout. Converts from UTF8 (with or
 * without &#-escapes) to ASCII, inserting &#-escapes for all
 * non-ASCII characters.
 *
 * Version: $Revision: 1.9 $ ($Date: 2017/11/24 10:14:49 $)
 * Author: Bert Bos <bert@w3.org>
 *
 * Copyright © 1994-2011 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 **/
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif
#include <ctype.h>

#define NOT_A_CHAR 2097152	/* One more than the largest code point */

static int nerrors = 0;

/* getUTF8 -- read UTF8 encoded char from stdin, return NOT_A_CHAR on error */
static long getUTF8()
{
  int b;
  long c;

  /* 0 = 0000  1 = 0001  2 = 0010  3 = 0011
     4 = 0100  5 = 0101  6 = 0110  7 = 0111
     8 = 1000  9 = 1001  A = 1010  B = 1011
     C = 1100  D = 1101  E = 1110  F = 1111 */

  if ((b = getchar()) == EOF) return EOF;	/* EOF */
  if ((b & 0x80) == 0) return b;		/* 0xxxxxxx = ASCII */
  if ((b & 0xE0) == 0xC0) {			/* 110xxxxx + 10xxxxxx */
    c = b & 0x1F;
    if ((b = getchar()) == EOF) {ungetc(EOF, stdin); return NOT_A_CHAR;}
    c = (c << 6) | (b & 0x3F);
    return c <= 0x7F ? NOT_A_CHAR : c;
  }
  if ((b & 0xF0) == 0xE0) {			/* 1110xxxx + (2) */
    c = b & 0x0F;
    if ((b = getchar()) == EOF) {ungetc(EOF, stdin); return NOT_A_CHAR;}
    c = (c << 6) | (b & 0x3F);
    if ((b = getchar()) == EOF) {ungetc(EOF, stdin); return NOT_A_CHAR;}
    c = (c << 6) | (b & 0x3F);
    if (0xD800 <= c && c <= 0xDFFF) return NOT_A_CHAR; /* Surrogate pair */
    return c <= 0x7FF ? NOT_A_CHAR : c;
  }
  if ((b & 0xF8) == 0xF0) {			/* 11110xxx + (3) */
    c = b & 0x07;
    if ((b = getchar()) == EOF) {ungetc(EOF, stdin); return NOT_A_CHAR;}
    c = (c << 6) | (b & 0x3F);
    if ((b = getchar()) == EOF) {ungetc(EOF, stdin); return NOT_A_CHAR;}
    c = (c << 6) | (b & 0x3F);
    if ((b = getchar()) == EOF) {ungetc(EOF, stdin); return NOT_A_CHAR;}
    c = (c << 6) | (b & 0x3F);
    return c <= 0xFFFF ? NOT_A_CHAR : c;
  }
  return NOT_A_CHAR;
}

/* xml2asc -- copy stdin to stdout, converting UTF8 XML to ASCII XML */
static void xml2asc(void)
{
  long c;

  while ((c = getUTF8()) != EOF) {
    if (c == NOT_A_CHAR) nerrors++;
    else if (c <= 127) putchar(c);
    else printf("&#%ld;", c);
  }
}

/* Print usage message, then exit */
static void usage(char *progname)
{
  fprintf(stderr, "Version %s\nUsage: %s <infile >outfile\n", VERSION, progname);
  exit(1);
}

/* main -- main body */
int main(int argc, char *argv[])
{
  if (argc != 1) usage(argv[0]);
  xml2asc();
  return nerrors;
}
