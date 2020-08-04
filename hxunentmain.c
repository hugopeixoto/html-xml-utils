/* unent -- expand HTML entities
 *
 * Author: Bert Bos
 * Created: 10 Aug 2008
 */

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "export.h"
#include "unent.e"

static int leave_builtin = 0;	/* Leave standard entities untouched */
static int fix_ampersands = 0;	/* Replace lone and unrecognized & by &amp; */

/* append_utf8 -- append the UTF-8 sequence for code n */
static void append_utf8(const int n)
{
  if (n <= 0x7F) {
    putchar(n);
  } else if (n <= 0x7FF) {
    putchar(0xC0 | (n >> 6));
    putchar(0x80 | (n & 0x3F));
  } else if (n <= 0xFFFF) {
    putchar(0xE0 | (n >> 12));
    putchar(0x80 | ((n >> 6) & 0x3F));
    putchar(0x80 | (n & 0x3F));
  } else if (n <= 0x1FFFFF) {
    putchar(0xF0 | (n >> 18));
    putchar(0x80 | ((n >> 12) & 0x3F));
    putchar(0x80 | ((n >> 6) & 0x3F));
    putchar(0x80 | (n & 0x3F));
  } else if (n <= 0x3FFFFFF) {
    putchar(0xF0 | (n >> 24));
    putchar(0x80 | ((n >> 18) & 0x3F));
    putchar(0x80 | ((n >> 12) & 0x3F));
    putchar(0x80 | ((n >> 6) & 0x3F));
    putchar(0x80 | (n & 0x3F));
  } else {
    putchar(0xF0 | (n >> 30));
    putchar(0x80 | ((n >> 24) & 0x3F));
    putchar(0x80 | ((n >> 18) & 0x3F));
    putchar(0x80 | ((n >> 12) & 0x3F));
    putchar(0x80 | ((n >> 6) & 0x3F));
    putchar(0x80 | (n & 0x3F));
  }
}


#define is_builtin(c) ((c)=='&'||(c)=='\''||(c)=='"'||(c)=='<'||(c)=='>')
#define hexval(c) ((c) <= '9' ? (c)-'0' : (c) <= 'F' ? 10+(c)-'A' : 10+(c)-'a')


/* expand -- print string, expanding entities to UTF-8 sequences */
static void expand(FILE *infile)
{
  const struct _Entity *e;
  int n, c;
  char s[12];		    /* Longest entity name has 8 characters */

  while ((c = fgetc(infile)) != EOF) {
    if (c != '&') {		/* Literal character */
      putchar(c);
    } else if ((c = fgetc(infile)), isalnum(c)) { /* Named entity, e.g., &lt; */
      s[0] = c;
      n = 1;
      while ((c = fgetc(infile)), isalnum(c) && n < sizeof(s) - 1) s[n++] = c;
      s[n] = '\0';
      if (! (e = lookup_entity(s, n))) {	/* Unknown entity */
	if (fix_ampersands) fputs("&amp;", stdout); else putchar('&');
	fputs(s, stdout);
	ungetc(c, infile);
      } else {
	if (leave_builtin && is_builtin(e->code)) printf("&%s;", s);
	else append_utf8(e->code);
	if (c != ';') ungetc(c, infile);
      }
    } else if (c == '#') {		     /* Numeric entity */
      if ((c = fgetc(infile)), isdigit(c)) { /* Decimal entity, e.g., &#10; */
	n = c - '0';
	while ((c = fgetc(infile)), isdigit(c)) n = 10 * n + c - '0';
	if (leave_builtin && is_builtin(n)) printf("&#%d;", n);
	else append_utf8(n);
	if (c != ';') ungetc(c, infile);
      } else if (c == 'x') {	/* Hexadecimal entity, e.g., &#x0A; */
	if ((c = fgetc(infile)), isxdigit(c)) {
	  n = hexval(c);
	  while ((c = fgetc(infile)), isxdigit(c)) n = 16 * n + hexval(c);
	  if (leave_builtin && is_builtin(n)) printf("&#x%x;", n);
	  else append_utf8(n);
	  if (c != ';') ungetc(c, infile);
	} else {	  	/* Invalid hexadecimal entity syntax */
	  if (fix_ampersands) fputs("&amp;", stdout); else putchar('&');
	  printf("#x");
	  ungetc(c, infile);
	}
      } else {			/* Invalid numerical entity */
	if (fix_ampersands) fputs("&amp;", stdout); else putchar('&');
	putchar('#');
	ungetc(c, infile);
      }
    } else {			/* Neither a letter nor a '#' */
      if (fix_ampersands) fputs("&amp;", stdout); else putchar('&');
      ungetc(c, infile);
    }
  }
  /* SGML says also that a record-end (i.e., an end-of-line) may be
   * used instead of a semicolon to end an entity reference. But the
   * record-end is not suppressed in HTML and such an entity reference
   * is invalid in XML, so we don't implement that rule here. Instead,
   * the end-of-line is treated as any other character (other than
   * semicolon) and left in the document.
   */
}

static void usage(const char *prog)
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 5
  __attribute__((__noreturn__))
#endif
;

/* usage -- print usage message and exit */
static void usage(const char *prog)
{
  fprintf(stderr, "Version %s\nUsage: %s [-b] [-f] [file]\n", VERSION, prog);
  exit(2);
}

/* main -- read input, expand entities, write out again */
int main(int argc, char *argv[])
{
  FILE *infile;
  int c;

  while ((c = getopt(argc, argv, "bf")) != -1)
    switch (c) {
    case 'b': leave_builtin = 1; break;
    case 'f': fix_ampersands = 1; break;
    default: usage(argv[0]);
    }
  if (optind == argc) infile = stdin;
  else if (optind == argc - 1) infile = fopen(argv[optind], "r");
  else usage(argv[0]);
  if (infile == NULL) {perror(argv[optind]); exit(1);}

  expand(infile);

  fclose(infile);
  return 0;
}
