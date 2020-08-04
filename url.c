/*
 * Routines and data structures to parse URLs
 *
 * Assumes the strings are encoded in UTF-8
 *
 * Bug: URL_s_absolutize("foo/bar", "../") yields "" but should return "./"
 *
 * Copyright © 1994-2011 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 7 March 1999
 */
#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
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
#include <regex.h>
#include <err.h>
#include <sysexits.h>
#if HAVE_LIBIDN2
# include <idn2.h>
#elif HAVE_LIBIDN
# include <idna.h>
#endif
#include "export.h"
#include "heap.e"
#include "types.e"

EXPORT typedef struct {
  string full;					/* Full URL as a string */
  string proto;					/* Protocol */
  string user;					/* User name */
  string password;				/* User's password */
  string machine;				/* Domain name or IP number */
  string port;					/* Port number or service */
  string path;					/* Path part of URL */
  string query;					/* Query part of URL */
  string fragment;				/* Fragment ID part of URL */
} *URL;


/* idn_to_ascii -- convert UTF-8 to Punycode, allocate on heap */
static string idn_to_ascii(const conststring s)
{
#if HAVE_LIBIDN2
  string p;
  int rc;

  rc = idn2_lookup_u8((uint8_t*)s, (uint8_t**)&p, IDN2_NFC_INPUT);
  if (rc == IDN2_OK) return p;
  warnx("%s", idn2_strerror(rc));
  return newstring(s);
#elif HAVE_LIBIDN
  string p;
  Idna_rc rc;

  rc = idna_to_ascii_8z(s, &p, 0);
  if (rc == IDNA_SUCCESS) return p;
  warnx("%s", idna_strerror(rc));
  return newstring(s);
#else
  conststring p;

  for (p = s; *p; p++)
    if ((unsigned char)(*p) < 33 || (unsigned char)(*p) > 127)
      warnx("Internationalized domain names in URLs require libidn.");
  return newstring(s);
#endif
}

/* utf8tohex -- convert UTF-8 to %HH hex encoding, allocate on heap */
static string utf8tohex(const conststring s)
{
  static string hex = "0123456789ABCDEF";
  string h;
  int i, j;

  if (!s) return NULL;
  newarray(h, 3 * strlen(s) + 1);		/* Usually too much */
  for (i = 0, j = 0; s[i]; i++) {
    if (s[i] & 0x80) {				/* Not ASCII */
      h[j++] = '%';
      h[j++] = hex[((unsigned char)s[i])/16];
      h[j++] = hex[((unsigned char)s[i])%16];
    } else if (s[i] == ' ') {			/* Also escape spaces */
      h[j++] = '%'; h[j++] = '2'; h[j++] = '0';
    } else
      h[j++] = s[i];
  }
  h[j] = '\0';
  return h;
}

/* URL_dispose -- free the memory used by a URL struct */
EXPORT void URL_dispose(URL url)
{
  if (url) {
    dispose(url->full);
    dispose(url->proto);
    dispose(url->user);
    dispose(url->password);
    dispose(url->machine);
    dispose(url->port);
    dispose(url->path);
    dispose(url->query);
    dispose(url->fragment);
    dispose(url);
  }
}

/* URL_new -- create a new URL struct; return NULL if not a valid URL */
EXPORT URL URL_new(const conststring url)
{
#define PROTO    "([^:/?#]+)"
  /*              2--------2						*/
#define USER     "([^/?#@:[]*)"
  /*              5----------5						*/
#define PASSWORD "([^/?#@[]*)"
  /*              7---------7						*/
#define HOST     "(([^/?#:[]+)|\\[([0-9a-fA-F:]*)])?"
  /*              89---------9----A-------------A-8			*/
#define PORT     "([^/?#]*)"
  /*              C-------C						*/
#define AUTH     "(" USER "(:" PASSWORD ")?@)?" HOST "(:" PORT ")?"
  /*              4--5--5--6---7------7--6--4   8--8  B---C--C--B	*/
#define PATH     "([^?#[]*)"
  /*              D------D						*/
#define QUERY    "([^#]*)"
  /*              F---F							*/
#define FRAGM    "(.*)"
  /*              H--H							*/

#define PAT  "(" PROTO ":)?(//" AUTH ")?" PATH "(\\?" QUERY ")?(#" FRAGM ")?"
  /*          1  2---2   1 3          3   D--D  E     F---F  E G   H---H  G */

   /*
   * 2 = proto, 5 = user, 7 = password, 9/A = machine, C = port, D = path,
   * F = query, H = fragment
   */
# define MAXSUB 18
  static regex_t re;
  static int initialized = 0;
  regmatch_t pm[MAXSUB];
  URL result;

  assert(url != NULL);
  
  /* Compile the regexp, only once */
  if (! initialized) {
    assert(regcomp(&re, PAT, REG_EXTENDED) == 0); /* Could be memory... */
    initialized = 1;
  }

  /* Match the URL against the pattern; return NULL if no match */
  if (regexec(&re, url, MAXSUB, pm, 0) != 0) return NULL;

  /* Store the various parts */
  new(result);
  result->full = newstring(url);
  result->proto = pm[2].rm_so == -1
    ? NULL : down(newnstring(url, pm[2].rm_eo));
  result->user = pm[5].rm_so == -1
    ? NULL : newnstring(url + pm[5].rm_so, pm[5].rm_eo - pm[5].rm_so);
  result->password = pm[7].rm_so == -1
    ? NULL : newnstring(url + pm[7].rm_so, pm[7].rm_eo - pm[7].rm_so);
  result->machine = pm[9].rm_so != -1
    ? newnstring(url + pm[9].rm_so, pm[9].rm_eo - pm[9].rm_so)
    : pm[10].rm_so != -1
    ? newnstring(url + pm[10].rm_so, pm[10].rm_eo - pm[10].rm_so)
    : NULL;
  result->port = pm[12].rm_so == -1
    ? NULL : newnstring(url + pm[12].rm_so, pm[12].rm_eo - pm[12].rm_so);
  result->path = pm[13].rm_so == -1
    ? NULL : newnstring(url + pm[13].rm_so, pm[13].rm_eo - pm[13].rm_so);
  result->query = pm[15].rm_so == -1
    ? NULL : newnstring(url + pm[15].rm_so, pm[15].rm_eo - pm[15].rm_so);
  result->fragment = pm[17].rm_so == -1
    ? NULL : newnstring(url + pm[17].rm_so, pm[17].rm_eo - pm[17].rm_so);
  return result;
}

/* merge -- merge a base path and a relative path */
static string  merge(const URL base, const string path)
{
  string s;
  int j;

  if (base->machine && (!base->path || !base->path[0])) {
    newarray(s, strlen(path) + 2);
    s[0] = '/';
    strcpy(s + 1, path);
  } else if (!base->path) {
    s = newstring(path);
  } else {
    for (j = strlen(base->path); j > 0 && base->path[j-1] != '/'; j--);
    newarray(s, j + strlen(path) + 1);
    memmove(s, base->path, j);
    strcpy(s + j, path);
  }
  return s;
}

/* remove_dot_segments -- remove /./ and /foo/../ */
static void remove_dot_segments(string path)
{
  int i = 0, len = strlen(path), j;

  while (len) {
    if (hasprefix(path + i, "/../")) {
      len -= 3;
      if (i == 0) {
	memmove(path + 1, path + 4, len);
      } else {
	for (j = i - 1; j > 0 && path[j-1] != '/'; j--) ;
	if (!hasprefix(path + j, "../")) {
	  memmove(path + j, path + i + 4, len);
	  i = j != 0 ? j - 1 : 0;
	} else {
	  i += 3;
	}
      }
    } else if (eq(path + i, "/..")) {
      len = 0;
      if (i == 0) {
	path[1] = '\0';
	i = 1;
      } else {
	for (j = i - 1; j > 0 && path[j-1] != '/'; j--) ;
	if (!hasprefix(path + j, "../")) {
	  path[j] = '\0';
	  i = j;
	} else {
	  i += 3;
	}
      }
    } else if (hasprefix(path + i, "/./")) {
      memmove(path + i, path + i + 2, len - 1);
      len -= 2;
    } else if (eq(path + i, "/.")) {
      path[i+1] = '\0';
      len--;
    } else {
      i++;
      len--;
    }
  }
}

/* URL_absolutize -- make a relative URL absolute */
EXPORT URL URL_absolutize(const URL base, const URL url)
{
  URL abs;

  new(abs);

  /* RFC 3986, section 5.2.2 */
  if (url->proto) {
    abs->proto = newstring(url->proto);
    abs->user = newstring(url->user);
    abs->password = newstring(url->password);
    abs->machine = newstring(url->machine);
    abs->port = newstring(url->port);
    abs->path = newstring(url->path);
    remove_dot_segments(abs->path);
    abs->query = newstring(url->query);
  } else {
    if (url->machine) {
      abs->user = newstring(url->user);
      abs->password = newstring(url->password);
      abs->machine = newstring(url->machine);
      abs->port = newstring(url->port);
      abs->path = newstring(url->path);
      remove_dot_segments(abs->path);
      abs->query = newstring(url->query);
    } else {
      if (!url->path || !url->path[0]) {
	abs->path = newstring(base->path);
	if (url->query) {
	  abs->query = newstring(url->query);
	} else {
	  abs->query = newstring(base->query);
	}
      } else {
	if (url->path[0] == '/') {
	  abs->path = newstring(url->path);
	  remove_dot_segments(abs->path);
	} else {
	  abs->path = merge(base, url->path);
	  remove_dot_segments(abs->path);
	}
	abs->query = newstring(url->query);
      }
      abs->user = newstring(base->user);
      abs->password = newstring(base->password);
      abs->machine = newstring(base->machine);
      abs->port = newstring(base->port);
    }
    abs->proto = newstring(base->proto);
  }
  abs->fragment = newstring(url->fragment);

  newarray(abs->full, (abs->proto ? strlen(abs->proto) + 1 : 0)
	   + (abs->user ? strlen(abs->user) + 1 : 0)
	   + (abs->password ? strlen(abs->password) + 1 : 0)
	   + (abs->machine ? strlen(abs->machine) + 4 : 0)
	   + (abs->port ? strlen(abs->port) + 1 : 0)
	   + (abs->path ? strlen(abs->path) : 0)
	   + (abs->query ? strlen(abs->query) + 1 : 0)
	   + (abs->fragment ? strlen(abs->fragment) + 1 : 0)
	   + 1);
  sprintf(abs->full, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	  abs->proto ? abs->proto : (string) "",
	  abs->proto ? (string) ":" : (string) "",
	  abs->machine ? (string) "//" : (string) "",
	  abs->user ? abs->user : (string) "",
	  abs->password ? (string) ":" : (string) "",
	  abs->password ? abs->password : (string) "",
	  abs->user ? (string) "@" : (string) "",
	  abs->machine && strchr(abs->machine, ':') ? "[" : "",
	  abs->machine ? abs->machine : (string) "",
	  abs->machine && strchr(abs->machine, ':') ? "]" : "",
	  abs->port ? (string) ":" : (string) "",
	  abs->port ? abs->port : (string) "",
	  abs->path ? abs->path : (string) "",
	  abs->query ? "?" : (string) "",
	  abs->query ? abs->query : (string) "",
	  abs->fragment ? (string) "#" : (string) "",
	  abs->fragment ? abs->fragment : (string) "");
  /* Instead of strchr() above, we could have an IPv6 flag. Necessary? */

  return abs;
}

/* URL_s_absolutize -- make a relative URL absolute */
EXPORT string URL_s_absolutize(const conststring base, const conststring url)
{
  URL url1 = URL_new(url), base1 = URL_new(base);
  URL abs = URL_absolutize(base1, url1);
  string result = newstring(abs->full);
  URL_dispose(abs);
  URL_dispose(url1);
  URL_dispose(base1);
  return result;
}

/* URL_to_ascii -- use punycode and %-escaping to turn an IRI into a URI */
EXPORT URL URL_to_ascii(const URL iri)
{
  URL url;

  new(url);
  url->proto = newstring(iri->proto);
  url->user = newstring(iri->user);
  url->password = newstring(iri->password);
  url->machine = idn_to_ascii(iri->machine);
  url->port = newstring(iri->port);
  url->path = utf8tohex(iri->path);
  url->query = utf8tohex(iri->query);
  url->fragment = utf8tohex(iri->fragment);

  newarray(url->full, (url->proto ? strlen(url->proto) + 1 : 0)
	   + (url->user ? strlen(url->user) + 1 : 0)
	   + (url->password ? strlen(url->password) + 1 : 0)
	   + (url->machine ? strlen(url->machine) + 4 : 0)
	   + (url->port ? strlen(url->port) + 1 : 0)
	   + (url->path ? strlen(url->path) : 0)
	   + (url->query ? strlen(url->query) + 1 : 0)
	   + (url->fragment ? strlen(url->fragment) + 1 : 0)
	   + 1);
  sprintf(url->full, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	  url->proto ? url->proto : (string) "",
	  url->proto ? (string) ":" : (string) "",
	  url->machine ? (string) "//" : (string) "",
	  url->user ? url->user : (string) "",
	  url->password ? (string) ":" : (string) "",
	  url->password ? url->password : (string) "",
	  url->user ? (string) "@" : (string) "",
	  url->machine && strchr(url->machine, ':') ? "[" : "",
	  url->machine ? url->machine : (string) "",
	  url->machine && strchr(url->machine, ':') ? "]" : "",
	  url->port ? (string) ":" : (string) "",
	  url->port ? url->port : (string) "",
	  url->path ? url->path : (string) "",
	  url->query ? "?" : (string) "",
	  url->query ? url->query : (string) "",
	  url->fragment ? (string) "#" : (string) "",
	  url->fragment ? url->fragment : (string) "");
  /* Instead of strchr() above, we could have an IPv6 flag. Necessary? */

  return url;
}

/* URL_s_to_ascii -- use punycode and %-escaping to turn an IRI into a URI */
EXPORT string URL_s_to_ascii(const conststring iri)
{
  URL x = URL_new(iri);
  URL y = URL_to_ascii(x);
  string result = newstring(y->full);
  URL_dispose(y);
  URL_dispose(x);
  return result;
}
