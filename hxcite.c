/*
 * cite - adds hyperlinks to bibliographic references in HTML
 *
 * The programs looks for strings of the form [[name]] (i.e., a
 * bibliographic label inside a double pair of square brackets), e.g.,
 * [[Knuth84]] or [[LieBos97]]. The label will be looked up in a
 * bibliography database and if it is found, the string will be
 * replaced by a pattern which is typically of the form <a
 * href="...">[name]</a>, but the pattern can be changed
 * with a command line option.
 *
 * If the string is of the form {{name}}, the name will be looked up,
 * but the string will be copied unchanged.
 *
 * If the label is not found, a warning is printed and the string is
 * left unchanged.
 *
 * All labels that are found are also stored, one label per line, in a
 * separate file with extension .aux. This file can be used by mkbib
 * to create the bibliography by extracting the corresponding
 * bibliographic entries from the database.
 *
 * The bibliography database must be a refer-style database. Though
 * for the purposes of this program all lines that don't start with
 * "%L" or %K are ignored. Lines with "%L" are assumed to contain a
 * label. Lines with %K are assumed to contain whitespace separated
 * keywords, which are effectively aliases for the label. Entries must
 * have one %L line and one or zero %K lines.
 *
 * Options:
 *
 * -b base
 *     Give the value for %b in the pattern.
 *
 * -p pattern
 *     The replacement for the string [[label]]. The default is
 *
 *     <a href=\"%b#%L\" rel=\"biblioentry\">[%L]<!--{{%m%L}}--></a>
 *
 *     %L will be replaced by the label, %b by the value of the -b
 *     option and %m by the marker (-m option).
 *
 * -a auxfile
 *     The name of the file in which the list of labels will be stored.
 *     Default is the name of the file given as argument, minus its
 *     extension, plus ".aux". If no file is give (input comes from
 *     stdin), the default name is "aux.aux".
 *
 * -m marker
 *     By default, the program looks for "[[name]]", but it can be
 *     made to look for "[[Xname]]" where X is some string, usually a
 *     symbol such as '!' or ='. This allows references to be
 *     classified, e.g., "[[!name]]" for normative references and
 *     "[[name]]" for non-normative references.
 *
 * -c
 *     Assume that every pair "<!--" and "-->" delimit a comment and
 *     do not process any [[label]] that occurs between them. Any
 *     "{{label}}" is processed as normal. This does not actually
 *     parse the input as HTML or XML and thus the program will
 *     mistake occurrences of these two strings inside CDATA sections
 *     or attribute values for comment delimiters.
 *
 * Copyright © 1994-2012 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 18 March 2000
 * Version: $Id: hxcite.c,v 1.11 2018/02/15 19:02:36 bbos Exp $
 **/

#include "config.h"
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
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

#ifdef HAVE_SEARCH_H
#  include <search.h>
#else
#  include "hash.e"
#endif

#include <ctype.h>
#include <stdbool.h>
#include "export.h"
#include "heap.e"
#include "types.e"
#include "errexit.e"


/* Warning: arbitrary limits! */
#define LINESIZE 32768
#define HASHSIZE 4096				/* Size of hash table */

#define WS " \t\r\n\f"				/* Separates %K keywords */

static string base = "";			/* URL of bibilography */
static string mark = "";			/* Flag after "'[[" */
static size_t marklen = 0;			/* Length of mark */
static string prog;				/* = argv[0] */
static string pattern =
  "<a href=\"%b#%L\" rel=\"biblioentry\">[%L]<!--{{%m%L}}--></a>";
static FILE *aux;
static bool skip_comments = false; /* Whether to skip [[ inside <!----> */


/* get_label -- get the label for the keyword, or NULL */
static string get_label(const string keyword)
{
  ENTRY *result, e = {keyword, NULL};

  result = hsearch(e, FIND);
  return result ? (string) result->data : NULL;
}


/* valid_label -- check if the label is well-formed */
static bool valid_label(const string label)
{
  int i;

  for (i = 0; label[i]; i++)
    if (! isalnum(label[i])
	&& label[i] != '-'
	&& label[i] != '_'
	&& label[i] != '.') return false;
  return true;
}


/* expand_ref -- print the reformatted reference */
static void expand_ref(const string label)
{
  int i;

  /* ToDo: somehow allow sequence numbers for references [1], [2], etc. */
  for (i = 0; pattern[i]; i++) {
    if (pattern[i] != '%') {
      putchar(pattern[i]);
    } else {
      switch (pattern[++i]) {
	case '%': putchar('%'); break;		/* Literal '%' */
	case 'b': printf("%s", base); break;	/* Base URL */
	case 'L': printf("%s", label); break;	/* Label */
	case 'm': printf("%s", mark); break;	/* Mark (-m option) */
	default: break;				/* Error in pattern */
      }
    }
  }
}


/* process_line -- look for citations in a line */
EXPORT void process_line(const string text, const string fname, int lineno,
			 bool *in_comment)
{
  string h = text, p, q, label = NULL, key;
  char c;

  /* Loop over occurrences of "[[" + mark + label + "]]"
   and "{{" + mark + label + "}}" */

  while (*in_comment ? (p = strpbrk(h, "-{")) : (p = strpbrk(h, "[{<"))) {

    while (h != p) putchar(*(h++));		/* Print text up to here */

    if (strncmp(p, "-->", 3) == 0) {		/* End of comment */
      putchar(*(h++));
      *in_comment = false;
      continue;
    }
    if (strncmp(p, "<!--", 4) == 0) {		/* Begin of comment */
      putchar(*(h++));
      *in_comment = skip_comments;
      continue;
    }
    if (strncmp(p, "{{", 2) && strncmp(p, "[[", 2)) { /* Not {{ or [[ */
      putchar(*(h++));
      continue;
    }

    /* Is there a corresponding closing bracket? */
    if (! (q = strstr(p + 2, *p == '[' ? "]]" : "}}"))) break;

    c = *p;					/* Remember [ or { */

    if (marklen == 0 || strncmp(p + 2, mark, marklen) == 0) {

      p += 2 + marklen;				/* Skip "[["/"{{" + mark */
      key = newnstring(p, q - p);		/* Extract the key */

      if (! valid_label(key)) {			/* Cannot be a key */
	while (h != q) putchar(*(h++));		/* Copy unchanged */
	putchar(*q); putchar(*(q+1));
      } else if (!(label = get_label(key))) {	/* No citation found: warn */
	while (h != q) putchar(*(h++));		/* Copy unchanged */
	putchar(*q); putchar(*(q+1));
	fprintf(stderr, "%s:%d: warning: no bib entry found for %s\n",
		fname ? fname : (string)"<stdin>", lineno, key);
      } else if (c == '[') {			/* Key found: expand */
	expand_ref(label);			/* Insert full reference */
	fprintf(aux, "%s\n", label);		/* Store label */
      } else {					/* "{{" so don't expand */
	while (h != q) putchar(*(h++));		/* Copy unchanged */
	putchar(*q); putchar(*(q+1));
	fprintf(aux, "%s\n", label);		/* Store label */
      }
      dispose(key);

    } else {					/* No valid mark */

      while (h != q) putchar(*(h++));		/* Copy unchanged */
      putchar(*q); putchar(*(q+1));
    }
    h = q + 2;
  }

  printf("%s", h);				/* Print rest of text */
}


/* store_labels_and_keywords -- store label in hash table */
static void store_labels_and_keywords(const string label, const string keys)
{
  string label1, h, b;
  ENTRY entry;

  assert(label);
  label1 = strtok_r(label, WS, &b);		/* Remove white space */
  if (!label1) return;				/* Empty label */
  entry.key = newstring(label1);
  entry.data = newstring(label1);
  if (!hsearch(entry, ENTER)) errexit("%s: %s\n", prog, strerror(errno));
  if (keys) {
    for (h = strtok_r(keys, WS, &b); h; h = strtok_r(NULL, WS, &b)) {
      entry.key = newstring(h);
      entry.data = newstring(label1);
      if (!hsearch(entry, ENTER)) errexit("%s: %s\n", prog, strerror(errno));
    }
  }
}


/* parse_db -- extract all labels from the refer-style database */
static void parse_db(const string db)
{
  char line[LINESIZE];
  FILE *f;
  int e;
  string label = NULL, keywords = NULL;

  if (!(f = fopen(db,"r"))) errexit("%s: %s: %s\n", prog, db, strerror(errno));

  /* Initialize the hash table */
  if (! hcreate(HASHSIZE)) errexit("%s: %s\n", prog, strerror(errno));

  /* Search for %L lines */
  clearerr(f);
  while (fgets(line, sizeof(line), f)) {
    if (line[0] != '%') {	/* We're outside an entry */
      if (label) store_labels_and_keywords(label, keywords);
      dispose(label);
      dispose(keywords);
    } else if (strncmp(line, "%L ", 3) == 0) {
      label = newstring(line + 3);
    } else if (strncmp(line, "%K ", 3) == 0) {
      keywords = newstring(line + 3);
    }
  }
  if (label) store_labels_and_keywords(label, keywords);

  if ((e = ferror(f))) errexit("%s: %s: %s\n", prog, db, strerror(e));

  if (fclose(f) != 0) errexit("%s: %s: %s\n", prog, db, strerror(errno));
}


/* usage -- print usage message and exit */
static void usage(void)
{
  errexit("Usage: %s [-b base] [-p pattern] [-a auxfile] [-c] [-v] bib-file [HTML-file]\n",
	  prog);
}


int main(int argc, char *argv[])
{
  char line[LINESIZE];
  string h, auxfile = NULL, dbfile = NULL, infile = NULL;
  bool in_comment = false;
  int e, lineno, c;
  FILE *f;

  /* Parse command line arguments */
  prog = argv[0];
  while ((c = getopt(argc, argv, "b:p:a:m:cv")) != -1) {
    switch (c) {
    case 'b': base = optarg; break;		/* Set base of URL */
    case 'p': pattern = optarg; break;		/* Form of expanded ref */
    case 'a': auxfile = optarg; break;		/* Name of auxfile */
    case 'm': mark = optarg; marklen = strlen(mark); break; /* After "[[" */
    case 'c': skip_comments = true; break;	/* Skip [[ in comments */
    case 'v': printf("Version: %s %s\n", PACKAGE, VERSION); return 0;
    default: usage();
    }
  }
  if (optind == argc || argc > optind + 2) usage();

  dbfile = argv[optind++];
  if (optind != argc) infile = argv[optind++];

  /* Read the labels from the bibliography database */
  parse_db(dbfile);

  /* Construct auxfile */
  if (! auxfile) {
    if (infile) {
      newarray(auxfile, strlen(infile) + 5);
      strcpy(auxfile, infile);
      if ((h = strrchr(auxfile, '.'))) *h = '\0';
      strcat(auxfile, ".aux");
    } else {
      auxfile = "aux.aux";
    }
  }
  if (! (aux = fopen(auxfile, "w")))
    errexit("%s: %s: %s\n", prog, auxfile, strerror(errno));

  /* Open input file or use stdin */
  f = infile ? fopen(infile, "r") : stdin;
  if (!f) errexit("%s: %s: %s\n", prog, infile, strerror(errno));

  /* Read input line by line */
  clearerr(f);
  lineno = 1;
  while (fgets(line, sizeof(line), f))
    process_line(line, infile, lineno++, &in_comment);
  if ((e = ferror(f))) errexit("%s: %s\n", prog, strerror(e));

  fclose(aux);
  fclose(f);
  return 0;
}
