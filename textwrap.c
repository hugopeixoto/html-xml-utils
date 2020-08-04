/*
 * Routines to wrap lines and indent them.
 *
 * Copyright © 1998-2016 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * To do: count characters, not bytes
 *
 * Bert Bos
 * Created 10 May 1998
 * $Id: textwrap.c,v 1.34 2019/10/05 17:49:44 bbos Exp $
 */
#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif
#include <stdlib.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"
#include "errexit.e"
#include "heap.e"

/* To do: XML 1.1 allows &#1;, so the following isn't safe anymore */
#define NBSP 1					/* Marks non-break-space */
#define BREAKOP 2		/* '\002' marks a line break opportunity */

static unsigned char *buf = NULL;
static int buflen = 0;				/* Size of buf */
static int len = 0;				/* Length of buf */
static int bufchars = 0;			/* Ditto in chars, not bytes */
static int linelen = 0;				/* Length of printed line */
static int level = 0;				/* Indentation level */
static int indent = 2;				/* # of spaces per indent */
static int maxlinelen = 72;			/* Desired line length */
static unsigned char prev = NBSP;      		/* Previously added char */

/* set_indent -- set the amount of indent per level */
EXPORT void set_indent(int n) {indent = n;}

/* set_linelen -- set the maximum length of a line */
EXPORT void set_linelen(int n) {maxlinelen = n;}

/* flush -- print word in buf */
EXPORT void flush()
{
  int i, j, k;

  assert(len <= buflen);
  while (len != 0 && linelen + bufchars >= maxlinelen) { /* Need break */
    /* Find last breakpoint i before maxlinelen, or first after it */
    for (i = -1, j = 0, k = 0; j < len && (k < maxlinelen || i == -1); j++)
      if (buf[j] == ' ') {i = j; k++;}
      else if (buf[j] == BREAKOP) i = j; /* Breakpoint but no character */
      else if ((buf[j] & 0xC0) != 0x80) k++; /* Start of a UTF-8 sequence */
    if (i < 0) break;				/* No breakpoint */
    assert(i >= 0);				/* Found a breakpoint at i */
    assert(buf[i] == ' ' || buf[i] == BREAKOP);
    /* Print up to breakpoint (removing non-break-space markers) */
    for (j = 0; j < i; j++)
      if (buf[j] != BREAKOP) {
	putchar(buf[j] == NBSP ? ' ' : buf[j]);
	if ((buf[j] & 0xC0) != 0x80) bufchars--;
      }
    putchar('\n');				/* Break line */
    linelen = 0;
    assert(level >= 0);
    assert(len >= 0);
    assert(i <= len);
    i++;					/* Skip the breakpoint */
    len -= i;
    if (len != 0) {		/* If anything left, insert the indent */
      memmove(buf + level * indent, buf + i, len);
      for (j = 0; j < level * indent; j++) buf[j] = NBSP; /* Indent */
      len += level * indent;
      bufchars += level * indent;
    }
  }
  /* Print rest, if any (removing non-break-space markers) */
  /* First remove spaces at end of line */
  while (len > 0 && buf[len-1] == ' ') {len--; bufchars--;}
  for (j = 0; j < len; j++)
    if (buf[j] == BREAKOP) /* skip */;
    else if (buf[j] == '\n' || buf[j] == '\r') {putchar(buf[j]); linelen = 0;}
    else if (buf[j] == NBSP) {putchar(' '); linelen++;}
    else if ((buf[j] & 0xC0) != 0x80) {putchar(buf[j]); linelen++;}
    else putchar(buf[j]);
  bufchars = 0;
  len = 0;
}

/* outc -- add one character to output buffer */
EXPORT void outc(char c, bool preformatted, bool with_space)
{
  if (c == '\n' || c == '\r' || c == '\f') {
    if (preformatted) ;		  /* Keep unchanged */
    else if (with_space) c = ' '; /* Treated as space */
    else c = BREAKOP;		  /* Treated as a break opportunity */
  } else if (c == '\t') {
    if (preformatted) ;		  /* Keep unchanged */
    else c = ' ';		  /* Tab is just a space */
  }
  if (c == ' ') {
    if (preformatted) c = NBSP;	  /* Non-break-space marker */
    else if (prev == ' ') return; /* Don't add another space */
    else if (prev == BREAKOP) return; /* Don't add a space after \n or similar */
  }
  if ((c == ' ' || c == BREAKOP) && linelen + bufchars >= maxlinelen) flush();
  if (c == '\n' || c == '\r' || c == '\f') flush(); /* Empty the buf */
  if (c == ' ' && linelen + len == 0) return;	/* No insert at BOL */
  while (level * indent >= buflen) {buflen += 1024; renewarray(buf, buflen);}
  if (linelen + len == 0 && !preformatted)
    while (len < level * indent) {buf[len++] = NBSP; bufchars++;}
  if (c == ' ' && len && buf[len-1] == ' ') return; /* Skip multiple spaces */
  while (len >= buflen) {buflen += 1024; renewarray(buf, buflen);}
    if ((c & 0xC0) != 0x80) bufchars++; /* Character */
  buf[len++] = c;				/* Finally, insert c */
  prev = c;					/* Remember for next round */
}

/* out -- add text to current output line, print line if getting too long */
EXPORT void out(string s, bool preformatted, bool with_space)
{
  if (s) for (; *s; s++) outc(*s, preformatted, with_space);
}

/* outn -- add n chars to current output, print line if getting too long */
EXPORT void outn(string s, size_t n, bool preformatted, bool with_space)
{
  size_t i;
  for (i = 0; i < n; i++) outc(s[i], preformatted, with_space);
}

/* outln -- add string to output buffer, followed by '\n' */
EXPORT void outln(char *s, bool preformatted, bool with_space)
{
  out(s, preformatted, with_space);
  flush();
  assert(len == 0);
  assert(bufchars == 0);
  putchar('\n');
  linelen = 0;
}

/* outbreak -- conditional new line; make sure next text starts on new line */
EXPORT void outbreak(void)
{
  flush();
  assert(len == 0);
  assert(bufchars == 0);
  if (linelen != 0) {
    putchar('\n');
    linelen = 0;
  }
}

/* outbreakpoint -- mark a possible line break point */
EXPORT void outbreakpoint(void)
{
  outc(BREAKOP, false, true);
}


/* inc_indent -- increase indentation level by 1 */
EXPORT void inc_indent(void)
{
  flush();
  level++;
}

/* dec_indent -- decrease indentation level by 1 */
EXPORT void dec_indent(void)
{
  flush();
  level--;
}
