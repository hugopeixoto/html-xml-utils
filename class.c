/*
 * Routines to check for the occurrence of a class.
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 20 Aug 2000
 *
 **/

#define _GNU_SOURCE		/* We need strcasestr() */
#include "config.h"
#include <assert.h>
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
#include <ctype.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"


/* contains -- check if s contains word (case-insenstive), return pointer */
EXPORT conststring contains(const conststring s, const conststring word)
{
  conststring t = s;
  unsigned char c;

  while ((t = strcasestr(t, word))) {
    if ((c = *(t + strlen(word))) && !isspace(c)) t++; /* Not end of word */
    else if (t != s && !isspace(*(t - 1))) t++;	/* Not beginning of word */
    else return t;				/* Found it */
  }
  return NULL;					/* Not found */
}

/* has_class -- check for class=word in list of attributes */
EXPORT bool has_class(pairlist attribs, const string word)
{
  pairlist p;

  for (p = attribs; p; p = p->next) {
    if (strcasecmp(p->name, "class") == 0 && contains(p->value, word))
      return true;
  }
  return false;
}

/* in_list -- check if word occurs in array list */
static bool in_list(const string word, const string *list)
{
  int i;

  for (i = 0; list[i]; i++) if (contains(word, list[i])) return true;
  return false;
}

/* has_class_in_list -- check for class=word for all words in array c */
EXPORT bool has_class_in_list(const pairlist attribs, const string *c)
{
  pairlist p;

  assert(c);
  for (p = attribs; p; p = p->next) {
    if (strcasecmp(p->name, "class") == 0 && in_list(p->value, c))
      return true;
  }
  return false;
}


