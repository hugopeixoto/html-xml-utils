/*
 * Routines to read headers of the style found in HTTP and mail
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Copyright © 2008 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 6 August 2008
 */
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "export.h"
#include "heap.e"
#include "types.e"
#include "dict.e"

#define BUFLEN 4096				/* Max len of header lines */


/* read_mail_headers -- read mail-style headers into headers, false on error */
EXPORT bool read_mail_headers(FILE *f, Dictionary headers)
{
  int i;
  string p, line = NULL;
  char buf[BUFLEN];

  /* Read first line */
  if (!fgets(buf, sizeof(buf), f)) return (ferror(f) == 0);
  if (!(line = strdup(buf))) return false;

  /* While not read an empty line, read and process more lines */
  while (line[0] != '\r' && line[0] != '\n' && fgets(buf, sizeof(buf), f)) {
    i = strlen(line);
    assert(i != 0);
    if (line[i-1] != '\n') {
      strapp(&line, buf, NULL); /* Previous fgets() didn't reach eol */
    } else if (buf[0] == ' ' || buf[0] == '\t') { /* Continuation line */
      if (line[i-1] == '\n') line[i-1] = '\0';
      if (i > 1 && line[i-2] == '\r') line[i-2] = '\0';
      strapp(&line, buf + 1, NULL);
    } else {
      if (!(p = strchr(line, ':'))) {free(line); return 0;} /* Syntax error */
      *p = '\0';
      for (p++; isspace(*p); p++) ;
      for (i = strlen(p) - 1; i >= 0 && isspace(p[i]); i--) p[i] = '\0';
      down(line);		/* Header name to lowercase */
      if (!dict_add(headers, line, p)) {free(line); return 0;}
      free(line);
      line = newstring(buf);
    }
  }
  free(line);

  return ferror(f) == 0;
}
