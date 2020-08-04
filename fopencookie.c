/* fopencookie.c -- implement GNU-style fopencookie() with BSD-style funopen()
 *
 * Todo: handle "a+" mode (probably by keeping read and write file pointers and
 * calling seek() before calling read or write)
 *
 * Todo: Is this solution, calling readfn() which in turn calls
 * sc->read() with some typecasts, better than ignoring the compiler
 * warnings and calling sc->read directly?
 *
 * Created: 22 August 2011
 * Author: Bert Bos <bert@w3.org>
 *
 * Copyright © 2011 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 */

#include "config.h"
#if HAVE_LIBCURL		/* We only need this if we're using libcurl */
#if !HAVE_FOPENCOOKIE		/* We don't need this on GNU Linux */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "errexit.e"
#include "fopencookie.h"

typedef struct {
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
  void *cookie;
} cookiewrapper;


/* readfn -- callback that in turn calls sc->read with proper typecasts */
static int readfn(void *sc, char *buf, int n)
{
  cookiewrapper *c = (cookiewrapper*)sc;
  return (int)(c->read(c->cookie, buf, (size_t)n));
}

/* writefn -- callback that in turn calls sc->write with proper typecasts */
static int writefn(void *sc, const char *buf, int n)
{
  cookiewrapper *c = (cookiewrapper*)sc;
  return (int)(c->write(c->cookie, buf, (size_t)n));
}

/* seekfn -- callback that in turn calls sc->seek with proper typecasts */
static fpos_t seekfn(void *sc, fpos_t offset, int whence)
{
  cookiewrapper *c = (cookiewrapper*)sc;
  return (fpos_t)(c->seek(c->cookie, (off64_t)offset, whence));
}

/* closefn -- callback that in turn calls sc->close and then frees memory */
static int closefn(void *sc)
{
  cookiewrapper *c = (cookiewrapper*)sc;
  int r = c->close ? c->close(c->cookie) : 0;
  free(sc);
  return r;
}


/* fopencookie -- open a stream defined by four callback functions */
FILE *fopencookie(void *cookie, const char *mode,
			 cookie_io_functions_t funcs)
{
  cookiewrapper *s;
  int mask;			/* 1 = read, 2 = write, 4 = append */
  FILE *f;

  /* Check that the parameters make sense */
  if (!mode) {errno = EINVAL; return NULL;}

  if (mode[0] == 'r') mask = 1;
  else if (mode[0] == 'w') mask = 2;
  else if (mode[0] == 'a') mask = 4;
  else {errno = EINVAL; return NULL;}
  if (mode[1] == '+' || (mode[1] =='b' && mode[2] == '+')) mask |= 3;

  if ((mask & 1) && !funcs.read) {errno = EINVAL; return NULL;}
  if ((mask & 2) && !funcs.write) {errno = EINVAL; return NULL;}
  if ((mask & 4) && !funcs.seek) {errno = EINVAL; return NULL;}

  if (mask == 7) errexit("Bug: fopencookie() can't yet handle mode \"a+\"\n");

  /* Open the "file" */
  if (!(s = malloc(sizeof(*s)))) {errno = ENOMEM; return NULL;}
  s->read = funcs.read;
  s->write = funcs.write;
  s->seek = funcs.seek;
  s->close = funcs.close;
  s->cookie = cookie;

  f = funopen(s,
	      s->read ? readfn : NULL,
	      s->write ? writefn : NULL,
	      s->seek ? seekfn : NULL,
	      closefn);
  if (!f) {free(s); return NULL;}

  /* If the mode is "a" (append), position the file pointer at the end */
  if ((mask & 4) && fseek(f, 0L, SEEK_END) < 0) {fclose(f); return NULL;}

  return f;
}

#endif /* !HAVE_FOPENCOOKIE */
#endif /* HAVE_LIBCURL */
