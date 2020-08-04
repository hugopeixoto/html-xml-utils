/*
 * Copyright © 1994-2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 1997
 **/
#include "config.h"
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif
#include <ctype.h>
#include <stdbool.h>
#include "export.h"
#include "heap.e"

EXPORT typedef char *string;
EXPORT typedef const char *conststring;

EXPORT typedef struct _pairlist {
  string name;
  string value;
  struct _pairlist *next;
} *pairlist;

EXPORT typedef unsigned int MediaSet;
EXPORT enum _Media {
  MediaNone = 0,
  MediaPrint = (1 << 0),
  MediaScreen = (1 << 1),
  MediaTTY = (1 << 2),
  MediaBraille = (1 << 3),
  MediaTV = (1 << 4),
  MediaProjection = (1 << 5),
  MediaEmbossed = (1 << 6),
  MediaAll = 0xFF
};

#define eq(s, t) (*(s) == *(t) && strcmp(s, t) == 0)
EXPORTDEF(eq(s, t))

#define hexval(c) ((c) <= '9' ? (c)-'0' : (c) <= 'F' ? 10+(c)-'A' : 10+(c)-'a')
EXPORTDEF(hexval(c))

/* pairlist_delete -- free all memory occupied by a pairlist */
EXPORT void pairlist_delete(pairlist p)
{
  if (p) {
    pairlist_delete(p->next);
    dispose(p->name);
    dispose(p->value);
    dispose(p);
  }
}

/* pairlist_copy -- make a deep copy of a pairlist */
EXPORT pairlist pairlist_copy(const pairlist p)
{
  pairlist h = NULL;

  if (p) {
    new(h);
    h->name = newstring(p->name);
    h->value = newstring(p->value);
    h->next = pairlist_copy(p->next);
  }
  return h;
}

/* pairlist_get -- get value corresponding to name, or NULL */
EXPORT conststring pairlist_get(pairlist p, const conststring name)
{
  for (; p && strcasecmp(p->name, name) != 0; p = p->next);
  return p ? p->value : NULL;
}

/* pairlist_set -- add or change a name/value pair */
EXPORT void pairlist_set(pairlist *p, const conststring name,
			 const conststring val)
{
  pairlist h;

  for (h = *p; h && strcasecmp(h->name, name) != 0; h = h->next);
  if (h) {
    free(h->value);
    h->value = newstring(val);
  } else {
    new(h);
    h->name = newstring(name);
    h->value = newstring(val);
    h->next = *p;
    *p = h;
  }
}

/* pairlist_unset -- remove a name/value pair from list, false if not found */
EXPORT bool pairlist_unset(pairlist *p, const conststring name)
{

  pairlist h, h1;

  if (! *p) return false;
  if (strcasecmp((*p)->name, name) == 0) {
    /* Remove first pair in list */
    h = *p;
    free(h->name);
    free(h->value);
    *p = h->next;
    free(h);
    return true;
  }
  for (h = *p; h->next && strcasecmp(h->next->name, name) != 0; h = h->next);
  if (! h->next) return false;	/* Not found */
  free(h->next->name);
  free(h->next->value);
  h1 = h->next;
  h->next = h->next->next;
  free(h1);
  return true;
}

/* strapp -- append to a string, re-allocating memory; last arg must be 0 */
EXPORT string strapp(string *s,...)
{
  va_list ap;
  int i, j;
  conststring h;

  va_start(ap, s);
  if (!s) {new(s); *s = NULL;}
  i = *s ? strlen(*s) : 0;
  while ((h = va_arg(ap, conststring))) {
    j = strlen(h);
    renewarray(*s, i + j + 1);
    strcpy(*s + i, h);
    i += j;
  }
  va_end(ap);
  return *s;
}

/* chomp -- remove trailing \n or \r\n (if any) from string */
EXPORT void chomp(string s)
{
  int i;

  if (s && (i = strlen(s)) != 0 && s[i-1] == '\n') {
    s[i-1] = '\0';
    if (i > 1 && s[i-2] == '\r') s[i-2] = '\0';
  }
}

EXPORT int min(int a, int b) { return a < b ? a : b; }
EXPORT int max(int a, int b) { return a > b ? a : b; }

/* down -- convert a string to lowercase, return pointer to arg */
EXPORT string down(const string s)
{
  string t;

  for (t = s; *t; t++) *t = tolower(*t);
  return s;
}

/* hasprefix -- true if s starts with prefix */
EXPORT bool hasprefix(conststring s, conststring prefix)
{
  if (!prefix) return true;	/* NULL is prefix of everything */
  if (!s) return !prefix;	/* Only NULL is prefix of NULL */
  while (*prefix && *prefix == *s) prefix++, s++;
  return *prefix == '\0';
}

/* hasaffix -- true if s ends with affix */
EXPORT bool hasaffix(conststring s, conststring affix)
{
  size_t i, j;

  if (!affix) return true;
  if (!s) return !affix;
  i = strlen(s);
  j = strlen(affix);
  if (i < j) return false;
  s = s + i - j;
  while (*affix && *affix == *s) affix++, s++;
  return *affix == '\0';
}

/* only_space -- check if s contains only whitespace */
EXPORT bool only_space(conststring s)
{
  while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t' || *s == '\f')
    s++;
  return *s == '\0';
}

