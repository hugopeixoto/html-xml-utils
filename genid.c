/*
 * Generate unique IDs.
 *
 * TO DO: Also generate "readable" IDs if the text uses non-ASCII
 * characters.
 *
 * Copyright © 2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 4 August 2000
 **/

#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_ERRNO_H
#  include <errno.h>
#endif
#include <ctype.h>

#ifdef HAVE_SEARCH_H
#  include <search.h>
#else
#  include "search-freebsd.h"
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
#include <export.h>
#include "heap.e"
#include "types.e"
#include "tree.e"
#include "errexit.e"


#define MAXIDLEN 45				/* Max len of a generated ID */

typedef int(*compar_fn_t)(const void *, const void *);

static void *idtree = NULL;			/* Sorted tree of IDs */


/* storeID -- remember the existence of an ID (allocates a copy of the ID) */
EXPORT void storeID(conststring id)
{
  /* Case-insensitive: necessary for HTML, only a little wasteful for XML */
  (void) tsearch(newstring(id), &idtree, (compar_fn_t)strcasecmp);
}


/* gen_id_r -- find some text suitable for an ID recursively */
static void gen_id_r(Tree t, string s, int *len, int maxlen)
{
  int i;
  Tree h;

  assert(s);					/* s at least maxlen long */

  /* Loop over children looking for useful text */
  for (h = t->children; h && *len < maxlen - 1; h = h->sister) {
    switch (h->tp) {
      case Text:
	for (i = 0; *len < maxlen - 1 && h->text[i]; i++)
	  if (isalpha(h->text[i])) s[(*len)++] = tolower(h->text[i]);
	  else if (h->text[i] == '@') {s[(*len)++] = 'a'; s[(*len)++] = 't';}
	  else if (*len == 0) ;			/* Wait for a letter first */
	  else if (h->text[i]=='-') s[(*len)++] = h->text[i];
	  else if (h->text[i]=='.') s[(*len)++] = h->text[i];
	  else if (h->text[i]=='_') s[(*len)++] = h->text[i];
	  else if (isdigit(h->text[i])) s[(*len)++] = h->text[i];
	  else if (isspace(h->text[i]) && s[*len-1] != '-') s[(*len)++]='-';
	break;
      case Element:				/* Recursive */
	gen_id_r(h, s, len, maxlen);
	break;
      default:
	break;
    }
  }
#if 0
  /* Look for a nice break, i.e., just before a '-' */
  while (*len > 0 && s[(*len)-1] != '-') (*len)--;
  if (*len > 0) (*len)--;
#endif
  s[*len] = '\0';
}

/* gen_id -- try some heuristics to generate an ID for element t */
EXPORT string gen_id(Tree t)
{
  string s;
  int len = 0;

  if (! (s = malloc(MAXIDLEN + 1))) errexit("Out of memory\n");

  assert(MAXIDLEN > 4);
  gen_id_r(t, s, &len, MAXIDLEN - 4);
  if (len == 0) {
    s[len++] = 'x';		/* At least one character */
    s[len] = '\0';
  }
  if (tfind(s, &idtree, (compar_fn_t)strcasecmp)) {
    /* No suitable text found or text is already used elsewhere */
    int seqno = 0;
    do {					/* Try adding digits */
      sprintf(s + len, "%d", seqno);
      seqno++;
    } while (seqno != 10000 && tfind(s, &idtree, (compar_fn_t)strcasecmp));
    if (seqno == 10000) {			/* 10000 tried, giving up... */
      free(s);
      return NULL;
    }
  }
  (void) tsearch(s, &idtree, (compar_fn_t)strcasecmp); /* Store it */
  return s;
}

