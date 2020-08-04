/*
 * Copyright © 1994-2000 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 31 Mar 2000
 * Version: $Id: strdup.c,v 1.4 2017/11/24 09:50:25 bbos Exp $
 **/
#include "config.h"
#include <stdlib.h>
#include "export.h"

#ifndef HAVE_STRDUP
/* strdup -- allocate a copy of a string on the heap; NULL if no memory */
EXPORT char *strdup(const char *s)
{
  char *t;

  if ((t = malloc((strlen(s) + 1) * sizeof(*s)))) strcpy(t, s);
  return t;
}
#endif /* HAVE_STRDUP */
