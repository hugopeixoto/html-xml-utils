/*
 * Routines to open a URL instead of a local file
 *
 * int openurl(const char *path)
 * FILE *fopenurl(const char *path)
 *
 * TODO: set CURLOPT_FAILONERROR, check return codes and then return
 * NULL from fopenurl2() with a proper error code, instead of just
 * passing the body of the HTTP error message.

 * TODO: Add arguments for PUT, POST; parse and return headers.
 *
 * TODO: authentication, use relevant fields in url.
 *
 * TODO: In the code for !HAVE_LIBCURL, apply URL_to_ascii() before
 * using a URL. (Libcurl handles IRIs automatically.)
 *
 * Uses http_proxy and ftp_proxy environment variables.
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 7 March 1999
 *
 * Copyright © 1999-2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 *
 * The write_cb() and wait_for_data() functions are inspired by
 * http://curl.haxx.se/libcurl/c/fopen.html which has the following
 * copyright:
 *
 *   Copyright (c) 2003 Simtec Electronics
 *
 *   Re-implemented by Vincent Sanders <vince@kyllikki.org> with
 *   extensive reference to original curl example code
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *   PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 *   OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *   DAMAGE.
 */
#include "config.h"
#define _GNU_SOURCE		/* Try to get fopencookie() from stdio.h */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "export.h"
#if HAVE_LIBCURL && !HAVE_FOPENCOOKIE
# include "fopencookie.h"	/* Use our own fopencookie() */
#endif
#include "dict.e"
#include "heap.e"
#include "types.e"
#include "errexit.e"

#define MAXREDIRECTS 10		/* Maximum # of 30x redirects to follow */


#ifdef DEBUG
/* debug -- print debugging info */
static void debug(char *format,...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}
#else
#define debug(...)
#endif


/* http_strerror -- return a string describing the status code */
EXPORT conststring http_strerror(int code)
{
  switch(code) {
  case 100: return "Continue";
  case 101: return "Switching Protocols";
  case 200: return "OK";
  case 201: return "Created";
  case 202: return "Accepted";
  case 203: return "Non-Authoritative Information";
  case 204: return "No Content";
  case 205: return "Reset Content";
  case 206: return "Partial Content";
  case 300: return "Multiple Choices";
  case 301: return "Moved Permanently";
  case 302: return "Found";
  case 303: return "See Other";
  case 304: return "Not Modified";
  case 305: return "Use Proxy";
  case 306: return "(Unused)";
  case 307: return "Temporary Redirect";
  case 400: return "Bad Request";
  case 401: return "Unauthorized";
  case 402: return "Payment Required";
  case 403: return "Forbidden";
  case 404: return "Not Found";
  case 405: return "Method Not Allowed";
  case 406: return "Not Acceptable";
  case 407: return "Proxy Authentication Required";
  case 408: return "Request Timeout";
  case 409: return "Conflict";
  case 410: return "Gone";
  case 411: return "Length Required";
  case 412: return "Precondition Failed";
  case 413: return "Request Entity Too Large";
  case 414: return "Request-URI Too Long";
  case 415: return "Unsupported Media Type";
  case 416: return "Requested Range Not Satisfiable";
  case 417: return "Expectation Failed";
  case 500: return "Internal Server Error";
  case 501: return "Not Implemented";
  case 502: return "Bad Gateway";
  case 503: return "Service Unavailable";
  case 504: return "Gateway Timeout";
  case 505: return "HTTP Version Not Supported";
  default: return "(Unknown status code)";
  }
}


#if HAVE_LIBCURL

#include <curl/curl.h>

typedef struct fcurl_data {
  CURL *curl;
  char *buffer;			/* buffer to store cached data */
  size_t buffer_len;		/* currently allocated buffers length */
  size_t buffer_pos;		/* end of data in buffer */
  int still_running;		/* is background url fetch still in progress */
  Dictionary headers;		/* response headers */
  struct curl_slist *req;	/* extra or overridden request headers */
  int *statusptr;		/* ptr to var with received HTTP status code */
} URL_FILE;

CURLM *multi_handle = NULL;	/* We use a global one for convenience */

int libcurl_is_initialized = 0;


/* header_cb -- libcurl calls this once for every protocol header line */
static size_t header_cb(char *buf, size_t size, size_t nmemb, URL_FILE *file)
{
  char *p, *header, *value, *url;
  size_t i, j;

  /* This routine currently only handles HTTP headers */
  if (curl_easy_getinfo(file->curl, CURLINFO_EFFECTIVE_URL, &url) != CURLE_OK ||
      (strncasecmp("http:", url, 5) != 0 &&
       strncasecmp("https:", url, 6) != 0))
    return size * nmemb;

  if (size * nmemb == 2) {	/* CRLF signals end of headers */

    assert(buf[0] == '\r' && buf[1] == '\n');

  } else if (!(p = memchr(buf, ':', size * nmemb))) { /* Must be status code */

    debug("+ < %s", buf);
    *file->statusptr = atoi(buf + strcspn(buf, " "));
    if (file->headers) dict_destroy_all(file->headers); /* Clear old headers */

  } else if (file->headers) {	/* Normal header */

    header = down(newnstring(buf, p - buf));
    i = strspn(p + 1, " \t") + 1; /* Skip white space */
    j = strcspn(p + i, "\r\n");	  /* End before CR-LF */
    assert(j < size * nmemb);
    value = newnstring(p + i, j);
    debug("+ < %s: %s\n", header, value);
    if (!dict_add(file->headers, header, value)) return 0; /* Memory error */
    dispose(header);
    dispose(value);
  }

  return size * nmemb;
}


/* write_cb -- curl calls this routine when it has read some data */
static size_t write_cb(char *buf, size_t size, size_t nitems, void *userdata)
{
  URL_FILE *file = (URL_FILE*)userdata;

  size *= nitems;
  if (file->buffer_len < file->buffer_pos + size) { /* Need bigger buffer */
    file->buffer_len = file->buffer_pos + size;
    file->buffer = realloc(file->buffer, file->buffer_len);
    if (!file->buffer) return 0; /* Out of memory */
  }
  memcpy(file->buffer + file->buffer_pos, buf, size);
  file->buffer_pos += size;
  return size;
}


/* wait_for_data -- fill the read buffer up to requested # of bytes */
static CURLcode wait_for_data(URL_FILE *file, size_t want)
{
  fd_set fdread, fdwrite, fdexcep;
  struct timeval timeout;
  CURLMcode rc;
  int maxfd, n;
  CURLMsg *msg;
  long curl_timeout;

  while (1) {

    do rc = curl_multi_perform(multi_handle, &file->still_running);
    while (rc == CURLM_CALL_MULTI_PERFORM);

    if (rc == CURLM_OUT_OF_MEMORY) return CURLE_OUT_OF_MEMORY;
    if (rc != CURLM_OK) errexit("Error waiting for data (%d)\n", rc);

    /* Stop if the connection is closed, after checking why */
    if (!file->still_running) {
      if (!(msg = curl_multi_info_read(multi_handle, &n))) return CURLE_OK;
      else return msg->msg == CURLMSG_DONE ? msg->data.result : CURLE_OK;
    }

    /* Stop when we have enough data */
    if (file->buffer_pos >= want) return CURLE_OK;

    /* Determine how long to wait in select(), max is 1 second */
    curl_multi_timeout(multi_handle, &curl_timeout);
    if (curl_timeout < 0 || curl_timeout >= 1000) {
      timeout.tv_sec = 1;	/* Set 1 second timeout */
      timeout.tv_usec = 0;
    } else {			/* Use precise timeout */
      timeout.tv_sec = 0;
      timeout.tv_usec = curl_timeout * 1000;
    }

    /* Get file descriptors from the transfers */
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);
    maxfd = -1;
    rc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
    if (rc == CURLM_OUT_OF_MEMORY) return CURLE_OUT_OF_MEMORY;
    if (rc != CURLM_OK) errexit("Error waiting for data (%d)\n", rc);

    /* Call select() to wait for either some data or a timeout */
    if (select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout) < 0)
      errexit("select error: %s\n", strerror(errno));
  }

  return CURLE_OK;
}


/* free_file -- decommission an easy_handle and free memory */
static void free_file(URL_FILE *file)
{
  debug("+ free_file()\n");
  (void) curl_multi_remove_handle(multi_handle, file->curl);
  curl_easy_cleanup(file->curl);
  if (file->buffer) free(file->buffer);
  if (file->req) curl_slist_free_all(file->req);
  free(file);
}


/* close_cb -- callback called when close(2) is called on our connection */
static int close_cb(void *cookie)
{
  debug("+ close_cb()\n");
  (void)free_file((URL_FILE*)cookie);
  return 0;
}


/* read_cb -- callback called when read(2) is called on our connection */
static ssize_t read_cb(void *cookie, char *buf, size_t n)
{
  URL_FILE *file = (URL_FILE*)cookie;

  /* Todo: set errno to something corresponding to the error */

  if (wait_for_data(file, n) != CURLE_OK) {errno = EIO; return -1;}

  if (file->buffer_pos < n) n = file->buffer_pos;
  if (!buf) {errno = EFAULT; return -1;}
  memcpy(buf, file->buffer, n);
  file->buffer_pos -= n;
  memmove(file->buffer, file->buffer + n, file->buffer_pos);
  return n;
}


/* cleanup -- callback for exit(3) to clean up libcurl connections */
static void cleanup(void)
{
  if (multi_handle) {
    curl_multi_cleanup(multi_handle);
    multi_handle = NULL;
  }
  if (libcurl_is_initialized) {
    curl_global_cleanup();
    libcurl_is_initialized = 0;
  }
}


/* fopenurl3 -- like fopenurl2, but uses method instead of GET */
EXPORT FILE *fopenurl3(const conststring method, const conststring url,
		       const conststring mode, const Dictionary request,
		       Dictionary response, int maxredirs, int *status)
{
  const char *h, *v, *headerline;
  cookie_io_functions_t iofuncs;
  URL_FILE *file;
  CURLMcode rc;
  CURLcode result;
  int dummy;
  size_t n;

  debug("+ fopenurl3(%s, %s, %s,..., %d,...)\n", method, url, mode, maxredirs);

  if (!status) status = &dummy;
  *status = 200;

  /* In case the url isn't a full URL, assume it is a local file */
  assert(url != NULL);
  n = strspn(url,
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-.");
  if (url[n] != ':') return fopen(url, mode);

  debug("+ Not a local file, set up libcurl...\n");

  if (!libcurl_is_initialized) {
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {errno = EIO; return NULL;}
    (void) atexit(cleanup);
    libcurl_is_initialized = 1;
  }

  new(file);
  file->still_running = 1;
  file->buffer = NULL;
  file->buffer_len = 0;
  file->buffer_pos = 0;
  file->headers = response;
  file->req = NULL;
  file->statusptr = status;

  /* Construct the extra request headers, if any */
  if (request)
    for (h = dict_next(request, NULL); h; h = dict_next(request, h)) {
      v = dict_find(request, h);
      headerline = strapp(NULL, h, ": ", v, NULL);
      debug("+ > %s\n", headerline);
      file->req = curl_slist_append(file->req, headerline);
      dispose(headerline);
    }

  file->curl = curl_easy_init();
#ifdef DEBUG
  curl_easy_setopt(file->curl, CURLOPT_VERBOSE, 1L);
#endif
  curl_easy_setopt(file->curl, CURLOPT_CUSTOMREQUEST, method);
  curl_easy_setopt(file->curl, CURLOPT_URL, url);
  curl_easy_setopt(file->curl, CURLOPT_WRITEDATA, file);
  curl_easy_setopt(file->curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(file->curl, CURLOPT_HEADERFUNCTION, header_cb);
  curl_easy_setopt(file->curl, CURLOPT_HEADERDATA, file);
  curl_easy_setopt(file->curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(file->curl, CURLOPT_MAXREDIRS, (long)maxredirs);
  curl_easy_setopt(file->curl, CURLOPT_HTTPHEADER, file->req);
#if LIBCURL_VERSION_NUM >= 0x071506
  /* needs libcurl >= 7.21.6 */
  curl_easy_setopt(file->curl, CURLOPT_TRANSFER_ENCODING, 1L);
#endif
  curl_easy_setopt(file->curl, CURLOPT_ENCODING, ""); /* Let curl decode it */

  if (!multi_handle && !(multi_handle = curl_multi_init())) {
    free_file(file);
    errno = EIO;
    return NULL;
  }

  rc = curl_multi_add_handle(multi_handle, file->curl);
  debug("+ added to multi_handle -> %d\n", rc);
  if (rc != CURLM_OK) {free_file(file); errno = EIO; return NULL;}
  /* Todo: make errno more specific, if possible */

  debug("+ Set up the connection...\n");

  /* Set up the connection */
  (void)curl_multi_perform(multi_handle, &file->still_running);

#if 0
  if (file->buffer_pos == 0 && !file->still_running) {
    int n;
    CURLMsg *m = curl_multi_info_read(multi_handle, &n);
    if (m->msg == CURLMSG_DONE && m->data.result == CURLE_UNSUPPORTED_PROTOCOL)
      errno = EPROTONOSUPPORT;
    else
      errno = EIO;		/* Todo: be more specific, if possible */
    debug("+ closed before the first data\n");
    free_file(file);
    return NULL;
  }
#endif

  /* Get the first data, i.e., after any headers */
  result = wait_for_data(file, 1);

  if (result != CURLE_OK) {
    free_file(file);
    if (result == CURLE_TOO_MANY_REDIRECTS) errno = EMLINK;
    else if (result == CURLE_UNSUPPORTED_PROTOCOL) errno = EPROTONOSUPPORT;
    else errno = EIO;
    return NULL;}
  /* Todo: make errno more specific, if possible */

  iofuncs.read = read_cb;
  iofuncs.write = NULL;
  iofuncs.seek = NULL;
  iofuncs.close = close_cb;
  return fopencookie(file, mode, iofuncs);
}


/* fopenurl2 -- like fopenurl, but sends and returns HTTP headers */
EXPORT FILE *fopenurl2(const conststring url, const conststring mode,
		       const Dictionary request, Dictionary response,
		       int maxredirs, int *status)
{
  assert(url != NULL);
  return fopenurl3("GET", url, mode, request, response, maxredirs, status);
}


/* fopenurl -- like fopen, but takes a URL; HTTP headers are parsed */
EXPORT FILE *fopenurl(const conststring path, const conststring mode,
		      int *status)
{
  assert(path != NULL);
  debug("+ fopenurl(\"%s\", \"%s\")\n", path, mode);
  return fopenurl2(path, mode, NULL, NULL, MAXREDIRECTS, status);
}


#else /* HAVE_LIBCURL */


#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#include <errno.h>
#if HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#if HAVE_STRINGS_H
#  include <strings.h>
#endif
#include <ctype.h>
#include "url.e"
#include "connectsock.e"
#include "headers.e"

#define BUFLEN 4096		/* Max len of header lines */

static URL http_proxy = NULL, ftp_proxy = NULL;
static int http_proxy_init = 0, ftp_proxy_init = 0;



/* Forward declaration */
FILE *fopenurl2(const conststring path, const conststring mode,
		const Dictionary request, Dictionary response,
		int maxredirs, int *status);


/* open_http2 -- open resource via HTTP; return file pointer or NULL */
static FILE *open_http2(const conststring machine, const conststring port,
			const conststring path, Dictionary request,
			Dictionary response, int maxredirs, int *status)
{
  int delete_response = !response;
  conststring h, v;
  char buf[BUFLEN];
  int fd, n, i;
  string s, t;
  FILE *f;

  assert(machine);
  assert(port);
  assert(path);

  debug("+ open_http2(\"%s\", \"%s\", \"%s\",...)\n", machine, port, path);

  /* Too many redirects? */
  if (maxredirs < 0) {errno = EMLINK; return NULL;}

  /* Connect */
  if ((fd = connectTCP(machine, port)) < 0) return NULL;

  /* Construct the request */
  t = strapp(NULL, "GET ", path, " HTTP/1.1\r\nHost: ", machine, NULL);
  if (!eq(port, "80")) strapp(&t, ":", port, NULL);
  strapp(&t, "\r\n", NULL);

  /* Add other headers */
  if (request)
    for (h = dict_next(request, NULL); h; h = dict_next(request, h))
      strapp(&t, h, ": ", dict_find(request, h), "\r\n", NULL);
  strapp(&t, "\r\n", NULL);

  /* Send the request, end with n = 0 (success) or n = -1 (failure) */
  n = strlen(t);
  while (n > 0) {
    i = write(fd, t, n);
    if (i < 0) n = -1; else n -= i;
  }
  dispose(t);
  if (n < 0) return NULL;

  /* No more output to server */
  (void)shutdown(fd, 1);

  /* Create FILE* */
  if (!(f = fdopen(fd, "r"))) return NULL;

  /* Check protocol version, read status code */
  if (!fgets(buf, sizeof(buf), f) ||
      !(hasprefix(buf, "HTTP/1.1 ") || hasprefix(buf, "HTTP/1.0 "))) {
    (void)fclose(f);
    return NULL;
  }
  *status = atoi(buf + strcspn(buf, " "));
  debug("+ Status = %d\n", *status);

  /* Read response headers */
  if (!response) response = dict_create(50);
  if (!read_mail_headers(f, response)) {
    (void)fclose(f);
    f = NULL;
  } else if (hasprefix(buf+9, "301") || hasprefix(buf+9, "302") ||
	     hasprefix(buf+9, "303") || hasprefix(buf+9, "307")) {
    (void)fclose(f);
    if (!(v = dict_find(response, "location"))) {
      errno = 121;		/* EREMOTEIO */
      f = NULL;			/* Redirect without a location!? */
    } else {
      s = newstring(v);		/* Because we'll delete the response dict. */
      dict_destroy_all(response);
      f = fopenurl2(s, "r", request, response, maxredirs - 1, status);
      dispose(s);
    }
  }
  /* To do: handle 305 Use Proxy */

  /* Return the body of the stream */
  if (response && delete_response) dict_delete(response);
  return f;
}


/* open_http -- open resource via HTTP; return file pointer or NULL */
static FILE *open_http(const URL url, Dictionary request, Dictionary response,
		       int maxredirs, int *status)
{
  string s, machine, port;
  FILE *f;

  /* Initialize proxy from environment variable, if not already done */
  if (! http_proxy_init) {
    if ((s = getenv("http_proxy"))) http_proxy = URL_new(s);
    http_proxy_init = 1;
  }

  /* What server do we connect to: a proxy or the end server? */
  machine = (http_proxy ? http_proxy : url)->machine;
  port = (http_proxy ? http_proxy : url)->port;
  if (!port) port = "80";

  if (http_proxy)
    f = open_http2(machine, port, url->full, request,response,maxredirs,status);
  else {
    s = NULL;
    if (url->path) strapp(&s, url->path, NULL);
    if (url->query) strapp(&s, "?", url->query, NULL);
    if (url->fragment) strapp(&s, "#", url->fragment, NULL);
    if (!s || !*s) strapp(&s, "/", NULL);
    debug("+ path = \"%s\"\n", s);
    f = open_http2(machine, port, s, request, response, maxredirs, status);
    dispose(s);
  }
  return f;
}


/* open_ftp -- open resource via FTP; return file pointer or NULL */
static FILE *open_ftp(const URL url, Dictionary request, Dictionary response,
		      int maxredirs)
{
  string proxy;
  int dummy;

  if (! ftp_proxy_init) {
    if ((proxy = getenv("ftp_proxy"))) ftp_proxy = URL_new(proxy);
    ftp_proxy_init = 1;
  }

  /* Can only work via proxy for now... */
  if (!ftp_proxy) {errno = ENOSYS; return NULL;}

  return open_http2(ftp_proxy->machine,
		    ftp_proxy->port ? ftp_proxy->port : "80",
		    url->full, request, response, maxredirs, &dummy);
}


/* open_file -- open resource as local file or FTP; return file ptr or NULL */
static FILE *open_file(const URL url, const conststring mode,
		       Dictionary request, Dictionary response, int maxredirs)
{
  FILE *f = NULL;

  if (! url->machine || eq(url->machine, "localhost")) {
    f = fopen(url->path, mode);
  }
  if (! f) {
    if (! eq(mode, "r")) errno = EACCES;	/* Not yet supported */
    else f = open_ftp(url, request, response, maxredirs);
  }
  return f;
}


/* fopenurl3 -- like fopenurl2, but with a method other than GET */
EXPORT FILE *fopenurl3(const conststring method, const conststring path,
		       const conststring mode, const Dictionary request,
		       Dictionary response, int maxredirs, int *status)
{
  FILE *f = NULL;
  int dummy;
  URL url;

  if (!status) status = &dummy;
  *status = 200;

  if (strcmp(method, "GET") != 0) {errno = EPROTONOSUPPORT; return NULL;}

  url = URL_new(path);
  if (! url) {
    errno = EACCES;				/* Invalid URL */
  } else if (! url->proto) {
    f = fopen(path, mode);			/* Assume it's a local file */
  } else if (eq(url->proto, "http")) {
    if (! eq(mode, "r")) errno = ENOSYS;	/* Not yet supported */
    else f = open_http(url, request, response, maxredirs, status);
  } else if (eq(url->proto, "ftp")) {
    if (! eq(mode, "r")) errno = ENOSYS;	/* Not yet supported */
    else f = open_ftp(url, request, response, maxredirs);
  } else if (eq(url->proto, "file")) {
    f = open_file(url, mode, request, response, maxredirs);
  } else {
    errno = EPROTONOSUPPORT;			/* Unimplemented protocol */
  }
  URL_dispose(url);
  return f;
}


/* fopenurl2 -- like fopenurl, but sends and returns HTTP headers */
EXPORT FILE *fopenurl2(const conststring url, const conststring mode,
		       const Dictionary request, Dictionary response,
		       int maxredirs, int *status)
{
  return fopenurl3("GET", url, mode, request, response, maxredirs, status);
}


/* fopenurl -- like fopen, but takes a URL; HTTP headers are parsed */
EXPORT FILE *fopenurl(const conststring url, const conststring mode,
		      int *status)
{
  return fopenurl3("GET", url, mode, NULL, NULL, MAXREDIRECTS, status);
}


#endif /* HAVE_LIBCURL */
