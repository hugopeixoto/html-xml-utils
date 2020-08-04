/*
 * Copyright © 1994-2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 31 Mar 2000
 * Version: $Id: strstr.c,v 1.4 2017/11/24 09:50:25 bbos Exp $
 **/
#include "config.h"
#include "export.h"

#ifndef HAVE_STRSTR
EXPORT char *strstr(const char *haystack, const char *needle)
{
  char *s, *t, *u;

  if (! needle) return haystack;		/* No needle */
  for (s = haystack; *s; s++) {
    for (t = needle, u = s; *t == *u && *t; t++, u++);
    if (! *t) return s;				/* Found it */
  }
  return NULL;					/* Not found */
}
#endif /* HAVE_STRSTR */
