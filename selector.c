/*
 * Type definitions and a parser for CSS selectors.
 *
 * Only parses selectors that allow incremental rendering
 * of a document.
 *
 * The Selector type is a linked list of simple selectors, with the
 * subject at the head, and its context linked from the "context"
 * field. The "combinator" field is the relation between this simple
 * selector and its context.
 *
 * To do: Allow multiple, comma-separated selectors
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 8 July 2001
 * Version: $Id: selector.c,v 1.14 2017/11/24 17:33:50 bbos Exp $
 **/

#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif
#include "export.h"
#include "heap.e"
#include "types.e"
#include "errexit.e"

EXPORT typedef enum {				/* Pseudo-classes & -elements */
  AttrNode,					/* ::attr() */
  RootSel, NthChild, NthOfType, FirstChild, FirstOfType, Lang,
  NthLastChild, NthLastOfType, LastChild, LastOfType, OnlyChild,
  OnlyOfType, Empty,
  Not,
} PseudoType;

EXPORT typedef struct _PseudoCond {
  PseudoType type;
  int a, b;					/* :nth-child(an+b) */
  string s;					/* :lang(s) */
  struct _SimpleSelector *sel;			/* :not(sel) */
  struct _PseudoCond *next;
} PseudoCond;

EXPORT typedef enum {				/* =, ~=, ^=, $= *= |= */
  Exists, Equals, Includes, StartsWith, EndsWith, Contains, LangMatch,
  HasClass, HasID				/* ".foo", "#foo" */
} Operator;

EXPORT typedef struct _AttribCond {
  Operator op;
  string name;					/* If not HasClass/ID */
  string value;					/* If op!=Exists */
  struct _AttribCond *next;
} AttribCond;

EXPORT typedef enum {
  Descendant, Child, Adjacent, Sibling		/* <space>, >, +, ~ */
} Combinator;

EXPORT typedef struct _SimpleSelector {
  string name;					/* NULL is "*" */
  AttribCond *attribs;
  PseudoCond *pseudos;
  PseudoCond *pseudoelts;			/* E.g., ::attr(foo) */
  Combinator combinator;			/* If context not NULL */
  struct _SimpleSelector *context;
  struct _SimpleSelector *next;			/* Comma-separated selectors */
} SimpleSelector, *Selector;

typedef enum {
  INIT, START_SIMPLE, AFTER_SIMPLE, AFTER_TYPE, CLASS, ID, ATTR, PSEUDO,
  AFTER_ATTR, EQ, START_VALUE, DSTRING, SSTRING, AFTER_VALUE,
  PSEUDO_ELT, IN_PSEUDO, LANG, IN_PSEUDO_, PSEUDO__O, PSEUDO__E, AFTER_N,
  AFTER_NUM, END_PSEUDO, AFTER_PLUS, AFTER_MINUS, PSEUDO__OD, PSEUDO__ODD,
  PSEUDO__EV, PSEUDO__EVE, PSEUDO__EVEN, LANG_, PSEUDO_ELT_ATTR,
  PSEUDO_ELT_ATTR_, IN_PSEUDO_PLUS, IN_PSEUDO_MINUS, NOT,
} State;


/* strappc -- append a character to a malloc'ed string */
static void strappc(string *s, char c)
{
  assert(s);
  int len = *s ? strlen(*s) : 0;
  renewarray(*s, len + 2);
  (*s)[len] = c;
  (*s)[len+1] = '\0';
}


/* push_sel -- allocate memory for a new selector; initialize */
static void push_sel(Selector *selector, Combinator combinator)
{
  Selector h;

  new(h);
  h->name = NULL;
  h->attribs = NULL;
  h->pseudos = NULL;
  h->pseudoelts = NULL;
  h->context = *selector;
  h->combinator = combinator;
  h->next = NULL;
  *selector = h;
}

/* isnmstart -- check if a character can start an identifier */
static bool isnmstart(unsigned int c)
{
  return ('a' <= c && c <= 'z')
    || ('A' <= c && c <= 'Z')
    || (c == '_')
    || (c == '\\')
    || (c >= '\200');
}

/* isnmchar -- check if a character can be inside an identifier */
static bool isnmchar(unsigned int c)
{
  return ('a' <= c && c <= 'z')
    || ('A' <= c && c <= 'Z')
    || ('0' <= c && c <= '9')
    || (c == '_')
    || (c == '-')
    || (c == '\\')
    || (c >= '\200');
}

/* parse_comment -- skip over a comment */
static void parse_comment(string *s)
{
  assert(s && *s && **s == '/');
  if (*(++*s) != '*') errexit("Unexpected \"/\"\n");
  for ((*s)++; **s; (*s)++)
    if (**s == '*' && *(*s+1) == '/') {(*s) += 2; return;}
  errexit("Comment is missing the closing \"*/\"\n");
}

/* parse_escape -- parse a backslash-escaped character, append UTF-8 to value */
static void parse_escape(string *s, string *value)
{
  int n;

  assert(value && s && *s && **s == '\\');
  (*s)++;
  if (!isxdigit(**s)) {strappc(value, **s); (*s)++; return;}
  n = hexval(**s); (*s)++;
  if (isxdigit(**s)) {n = 16 * n + hexval(**s); (*s)++;}
  if (isxdigit(**s)) {n = 16 * n + hexval(**s); (*s)++;}
  if (isxdigit(**s)) {n = 16 * n + hexval(**s); (*s)++;}
  if (isxdigit(**s)) {n = 16 * n + hexval(**s); (*s)++;}
  if (isxdigit(**s)) {n = 16 * n + hexval(**s); (*s)++;}
  if (isspace(**s)) (*s)++;

  /* Convert to a UTF-8 string */
  if (n <= 0x7F) {
    strappc(value, n);
  } else if (n <= 0x7FF) {
    strappc(value, 0xC0 | (n >> 6));
    strappc(value, 0x80 | (n & 0x3F));
  } else if (n <= 0xFFFF) {
    strappc(value, 0xE0 | (n >> 12));
    strappc(value, 0x80 | ((n >> 6) & 0x3F));
    strappc(value, 0x80 | (n & 0x3F));
  } else if (n <= 0x1FFFFF) {
    strappc(value, 0xF0 | (n >> 18));
    strappc(value, 0x80 | ((n >> 12) & 0x3F));
    strappc(value, 0x80 | ((n >> 6) & 0x3F));
    strappc(value, 0x80 | (n & 0x3F));
  } else if (n <= 0x3FFFFFF) {
    strappc(value, 0xF0 | (n >> 24));
    strappc(value, 0x80 | ((n >> 18) & 0x3F));
    strappc(value, 0x80 | ((n >> 12) & 0x3F));
    strappc(value, 0x80 | ((n >> 6) & 0x3F));
    strappc(value, 0x80 | (n & 0x3F));
  } else {
    strappc(value, 0xF0 | (n >> 30));
    strappc(value, 0x80 | ((n >> 24) & 0x3F));
    strappc(value, 0x80 | ((n >> 18) & 0x3F));
    strappc(value, 0x80 | ((n >> 12) & 0x3F));
    strappc(value, 0x80 | ((n >> 6) & 0x3F));
    strappc(value, 0x80 | (n & 0x3F));
  }
}

/* parse_ident -- parse and return an identifier */
static string parse_ident(string *s)
{
  string ident = NULL;
  
  assert(*s && isnmchar(**s));	/* Not isnmstart(), it may be a HASH (#...) */
  if (**s == '\\') parse_escape(s, &ident);
  else strappc(&ident, *(*s)++);
  while (isnmchar(**s))
    if (**s == '\\') parse_escape(s, &ident);
    else strappc(&ident, *(*s)++);
  return ident;
}

/* parse_int -- parse and return a decimal integer */
static int parse_int(string *s)
{
  int n = 0;
  bool neg = false;

  assert(s && *s && (**s == '-' || **s == '+' || isdigit(**s)));
  if (**s == '+') (*s)++; else if (**s == '-') {neg = true; (*s)++;}
  if (!isdigit(**s))errexit("Expected a number after +/- but found \"%c\"\n",**s);
  while (isdigit(**s)) {
    n = 10 * n + (**s - '0');
    if (n < 0) errexit("Cannot handle a number this big\n");
    (*s)++;
  }
  return neg ? -n : n;
}

/* parse_selector -- parse the selector in s */
EXPORT Selector parse_selector(const string selector, string *rest)
{
  State state = INIT;
  string name;
  string s = selector;
  AttribCond *attsel;
  PseudoCond *pseudosel;
  Selector sel = NULL, h;
  int n = 0;			/* Avoid warning about uninitialized */

  push_sel(&sel, Descendant);

  while (*s) {
    switch (state) {
    case INIT:					/* Expect a simple sel */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else state = START_SIMPLE;
      break;
    case AFTER_SIMPLE:				/* Expect a combinator */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (*s == '+') {s++; push_sel(&sel, Adjacent); state = INIT;}
      else if (*s == '>') {s++; push_sel(&sel, Child); state = INIT;}
      else if (*s == '~') {s++; push_sel(&sel, Sibling); state = INIT;}
      else if (*s == ',') {h = parse_selector(s+1, &s); h->next = sel; sel = h;}
      else if (*s == ')') {*rest = s; return sel;}
      else {push_sel(&sel, Descendant); state = INIT;}
      break;
    case START_SIMPLE:				/* Start simple sel */
      if (*s == '*') {s++; state = AFTER_TYPE;}
      else if (*s == '.') {s++; state = CLASS;}
      else if (*s == '#') {s++; state = ID;}
      else if (*s == '[') {s++; state = ATTR;}
      else if (*s == ':') {s++; state = PSEUDO;}
      else if (isnmstart(*s)) {sel->name = parse_ident(&s); state = AFTER_TYPE;}
      else errexit("Unexpected \"%c\"\n", *s);
      break;
    case AFTER_TYPE:				/* After a type sel */
      if (*s == '/') parse_comment(&s);
      else if (*s == '.') {s++; state = CLASS;}
      else if (*s == '#') {s++; state = ID;}
      else if (*s == '[') {s++; state = ATTR;}
      else if (*s == ':') {s++; state = PSEUDO;}
      else if (isnmstart(*s)) errexit("Unexpected \"%c\"\n", *s);
      else state = AFTER_SIMPLE;
      break;
    case CLASS:					/* Just seen a '.' */
      if (*s == '/') parse_comment(&s);
      else if (isnmstart(*s)) {new(attsel); attsel->op = HasClass;
	attsel->value = parse_ident(&s); attsel->next = sel->attribs;
	sel->attribs = attsel; state = AFTER_TYPE;}
      else errexit("Unexpected \"%c\" after \".\"\n", *s);
      break;
    case ID:					/* Just seen a '#' */
      if (isnmchar(*s)) {new(attsel); attsel->op = HasID;
	attsel->value = parse_ident(&s); attsel->next = sel->attribs;
	sel->attribs = attsel; state = AFTER_TYPE;}
      else errexit("Unexpected \"%c\" after \"#\"\n", *s);
      break;
    case ATTR:					/* Just seen '[' */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (isnmstart(*s)) {new(attsel); attsel->name = parse_ident(&s);
	attsel->next = sel->attribs; sel->attribs = attsel; state = AFTER_ATTR;}
      else errexit("Unexpected \"%c\" after \"[\"\n", *s);
      break;
    case AFTER_ATTR:				/* Just seen a '[' + ident */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (*s == ']') {s++; sel->attribs->op = Exists; state = AFTER_TYPE;}
      else if (*s == '~') {s++; sel->attribs->op = Includes; state = EQ;}
      else if (*s == '|') {s++; sel->attribs->op = LangMatch; state = EQ;}
      else if (*s == '^') {s++; sel->attribs->op = StartsWith; state = EQ;}
      else if (*s == '$') {s++; sel->attribs->op = EndsWith; state = EQ;}
      else if (*s == '*') {s++; sel->attribs->op = Contains; state = EQ;}
      else {sel->attribs->op = Equals; state = EQ;}
      break;
    case EQ:					/* Expect '=' */
      if (*s != '=') errexit("Expected '=' instead of \"%c\"\n", *s);
      else {s++; sel->attribs->value = NULL; state = START_VALUE;}
      break;
    case START_VALUE:				/* After '=' */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (*s == '"') {s++; state = DSTRING;}
      else if (*s == '\'') {s++; state = SSTRING;}
      else if (isnmstart(*s)) {sel->attribs->value = parse_ident(&s);
	state = AFTER_VALUE;}
      else errexit("Expected string or name after \"=\" but found \"%c\"\n",*s);
      break;
    case DSTRING:				/* Inside "..." */
      if (*s == '"') {s++; state = AFTER_VALUE;}
      else if (*s == '\\') parse_escape(&s, &sel->attribs->value);
      else {strappc(&sel->attribs->value, *s); s++;}
      break;
    case SSTRING:				/* Inside "..." */
      if (*s == '\'') {s++; state = AFTER_VALUE;}
      else if (*s == '\\') parse_escape(&s, &sel->attribs->value);
      else {strappc(&sel->attribs->value, *s); s++;}
      break;
    case AFTER_VALUE:				/* Expect ']' */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (*s == ']') {s++; state = AFTER_TYPE;}
      else errexit("Expected ']' instead of \"%c\"\n", *s);
      break;
    case PSEUDO:				/* After ':' */
      if (*s == '/') parse_comment(&s);
      else if (*s == ':') {s++; state = PSEUDO_ELT;}
      else if (!isnmstart(*s))
	errexit("Expected a pseudo-class after \":\" but found \"%c\"\n", *s);
      else {new(pseudosel); pseudosel->next = sel->pseudos;
	sel->pseudos = pseudosel; name = parse_ident(&s); 
	if (strcasecmp(name, "root") == 0) {
	  pseudosel->type = RootSel; state = AFTER_TYPE;}
	else if (strcasecmp(name, "nth-child") == 0) {
	  pseudosel->type = NthChild; state = IN_PSEUDO;}
	else if (strcasecmp(name, "nth-last-child") == 0) {
	  pseudosel->type = NthLastChild; state = IN_PSEUDO;}
	else if (strcasecmp(name, "nth-of-type") == 0) {
	  pseudosel->type = NthOfType; state = IN_PSEUDO;}
	else if (strcasecmp(name, "nth-last-of-type") == 0) {
	  pseudosel->type = NthLastOfType; state = IN_PSEUDO;}
	else if (strcasecmp(name, "first-child") == 0) {
	  pseudosel->type = FirstChild; state = AFTER_TYPE;}
	else if (strcasecmp(name, "last-child") == 0) {
	  pseudosel->type = LastChild; state = AFTER_TYPE;}
	else if (strcasecmp(name, "first-of-type") == 0) {
	  pseudosel->type = FirstOfType; state = AFTER_TYPE;}
	else if (strcasecmp(name, "last-of-type") == 0) {
	  pseudosel->type = LastOfType; state = AFTER_TYPE;}
	else if (strcasecmp(name, "only-child") == 0) {
	  pseudosel->type = OnlyChild; state = AFTER_TYPE;}
	else if (strcasecmp(name, "only-of-type") == 0) {
	  pseudosel->type = OnlyOfType; state = AFTER_TYPE;}
	else if (strcasecmp(name, "empty") == 0) {
	  pseudosel->type = Empty; state = AFTER_TYPE;}
	else if (strcasecmp(name, "lang") == 0) {
	  pseudosel->type = Lang; state = LANG;}
	else if (strcasecmp(name, "not") == 0) {
	  pseudosel->type = Not; state = NOT;}
	else errexit("Unknown pseudo-class \"%s\"\n", name);
      }
      break;
    case IN_PSEUDO:				/* After ':...', expect '(' */
      if (*s == '(') {s++; state = IN_PSEUDO_;}
      else errexit("Expecting a \"(\" but found \"%c\"\n", *s);
      break;
    case IN_PSEUDO_:				/* Expecting an+b */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (*s == 'o' || *s == 'O') {s++; state = PSEUDO__O;}
      else if (*s == 'e' || *s == 'E') {s++; state = PSEUDO__E;}
      else if (*s == 'n' || *s == 'N') {sel->pseudos->a = 1; s++;state=AFTER_N;}
      else if (*s == '+') {s++; state = IN_PSEUDO_PLUS;}
      else if (*s == '-') {s++; state = IN_PSEUDO_MINUS;}
      else if (isdigit(*s)) {n = parse_int(&s); state = AFTER_NUM;}
      else errexit("Expected a number, but found \"%c\"\n", *s);
      break;
    case IN_PSEUDO_PLUS:			/* After ':pseudo(+' */
      if (*s == 'n') {sel->pseudos->a = 1; s++; state = AFTER_N;}
      else if (isdigit(*s)) {n = parse_int(&s); state = AFTER_NUM;}
      else errexit("Expected a number after \"+\" but found \"%c\"\n", *s);
      break;
    case IN_PSEUDO_MINUS:			/* After ':pseudo(-' */
      if (*s == 'n') {sel->pseudos->a = -1; s++; state = AFTER_N;}
      else if (isdigit(*s)) {n = -parse_int(&s); state = AFTER_NUM;}
      else errexit("Expected a number after \"-\" but found \"%c\"\n", *s);
      break;
    case AFTER_NUM:				/* After ':pseudo(' + number */
      if (*s == 'n' || *s == 'N') {sel->pseudos->a = n;	s++; state = AFTER_N;}
      else {sel->pseudos->a = 0; sel->pseudos->b = n; state = END_PSEUDO;}
      break;
    case AFTER_N:				/* After ':pseudo(an' */
      if (isspace(*s)) s++;
      else if (*s == '+') {s++; state = AFTER_PLUS;}
      else if (*s == '-') {s++; state = AFTER_MINUS;}
      else {sel->pseudos->b = 0; state = END_PSEUDO;}
      break;
    case AFTER_PLUS:				/* After an+ */
      if (isspace(*s)) s++;
      else if (isdigit(*s)) {sel->pseudos->b = parse_int(&s); state=END_PSEUDO;}
      else errexit("Expected a number after the \"+\", but found \"%c\"\n", *s);
      break;
    case AFTER_MINUS:				/* After an- */
      if (isspace(*s)) s++;
      else if (isdigit(*s)) {sel->pseudos->b= -parse_int(&s); state=END_PSEUDO;}
      else errexit("Expected a number after the \"-\", but found \"%c\"\n", *s);
      break;
    case END_PSEUDO:				/* Expecting ')' */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (*s == ')') {s++; state = AFTER_TYPE;}
      else errexit("Expected \")\" but found \"%c\"\n", *s);
      break;
    case PSEUDO__O:				/* After :nth...(o */
      if (*s == 'd' || *s == 'D') {s++; state = PSEUDO__OD;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case PSEUDO__OD:				/* After :nth...(od */
      if (*s == 'd' || *s == 'D') {s++; state = PSEUDO__ODD;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case PSEUDO__ODD:				/* After :nth-...(odd */
      if (!isnmchar(*s)) {state = END_PSEUDO;
        sel->pseudos->a = 2; sel->pseudos->b = 1;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case PSEUDO__E:				/* After :nth-...(e */
      if (*s == 'v' || *s == 'V') {s++; state = PSEUDO__EV;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case PSEUDO__EV:				/* After :nth-...(ev */
      if (*s == 'e' || *s == 'E') {s++; state = PSEUDO__EVE;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case PSEUDO__EVE:				/* After :nth-...(eve */
      if (*s == 'n' || *s == 'N') {s++; state = PSEUDO__EVEN;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case PSEUDO__EVEN:				/* Afte :nth-...(even */
      if (!isnmchar(*s)) {state = END_PSEUDO;
        sel->pseudos->a = 2; sel->pseudos->b = 0;}
      else errexit("Illegal character \"%c\" in \":nth-...(\"\n", *s);
      break;
    case LANG:					/* After ':lang' */
      if (*s == '(') {s++; state = LANG_;}
      else errexit("Expecting \"(\" after \":lang\" but found \"%c\"\n", *s);
      break;
    case LANG_:					/* After ':lang(' */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (isnmstart(*s)) {sel->pseudos->s = parse_ident(&s);
	state = END_PSEUDO;}
      else errexit("Incorrect \":lang(\" at \"%c\"\n", *s);
      break;
    case NOT:					/* After ':not' */
      if (*s == '(') {sel->pseudos->sel = parse_selector(s + 1, &s);
	state = END_PSEUDO;}
      else errexit("Expecting \"(\" after \":not\" but found \"%c\"\n", *s);
      break;
    case PSEUDO_ELT:				/* After '::' */
      if (*s == '/') parse_comment(&s);
      else if (!isnmstart(*s))
	errexit("Expected a pseudo-element after \"::\" but found \"%c\"\n",*s);
      else {push_sel(&sel, Child); new(pseudosel);
	pseudosel->next = sel->pseudoelts; sel->pseudoelts = pseudosel;
	name = parse_ident(&s);
	if (strcasecmp(name, "attr") == 0) {
	  sel->pseudoelts->type = AttrNode; state = PSEUDO_ELT_ATTR;}
	else errexit("Unknown pseudo-element \"%s\"\n", name);
      }
      break;
    case PSEUDO_ELT_ATTR:			/* After '::attr' */
      if (*s == '(') {s++; state = PSEUDO_ELT_ATTR_;}
      else errexit("Expected \"(\" after \"::attr\" but found \"%c\"\n", *s);
      break;
    case PSEUDO_ELT_ATTR_:			/* After '::attr(' */
      if (isspace(*s)) s++;
      else if (*s == '/') parse_comment(&s);
      else if (!isnmstart(*s)) errexit("Expected a name after \"::attr(\"\n");
      else {sel->pseudoelts->s = parse_ident(&s); state = END_PSEUDO;}
      break;
    default:
      assert(!"Cannot happen");
    }
  }
  if (state != AFTER_TYPE && state != AFTER_SIMPLE)
    errexit("Incomplete selector (state %d)\n", state);

  *rest = s;
  return sel;
}

void dump_selector(FILE *f, Selector s);

/* utf8toint -- convert a UTF-8 sequence to a character code point */
static int utf8toint(conststring s, conststring *t)
{
  int n;

  assert(s);
  assert(*s);
  if ((*s & 0x80) == 0) {*t = s + 1; return *s;} /* 0xxxxxxx */
  if ((*s & 0xE0) == 0xC0) n = *s & 0x1F;	 /* 110xxxxx */
  else if ((*s & 0xF0) == 0xE0) n = *s & 0xF;	 /* 1110xxxx */
  else if ((*s & 0xF8) == 0xF0) n = *s & 0x7;	 /* 11110xxx */
  else {*t = s + 1; return *s;}	/* Error! */
  for (s++; *s && (*s & 0x80) == 0x80; s++) n = (n << 6) | (*s & 0x3F);
  *t = s;
  return n;
}

/* esc -- print a string, escaping special characters */
static void esc(FILE *f, conststring s)
{
  assert(s);
  assert(*s);

  if (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') ||
      *s == '_' || *s & 0x80)
    putc(*(s++), f);
  else
    fprintf(f, "\\%X ", utf8toint(s, &s));

  while (*s) {
    if (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') ||
	('0' <= *s && *s <= '9') || *s == '-' || *s == '_' || *s & 0x80)
      putc(*(s++), f);
    else
      fprintf(f, "\\%X ", utf8toint(s, &s));
  }
}

/* dump_simple_selector -- serialize a simple selector */
EXPORT void dump_simple_selector(FILE *f, const SimpleSelector *s)
{
  AttribCond *a;
  PseudoCond *p;

  if (s->name) esc(f, s->name); else putc('*', f);
  for (a = s->attribs; a; a = a->next) {
    if (a->op == HasClass) {putc('.', f); esc(f, a->value);}
    else if (a->op == HasID) {putc('#', f); esc(f, a->value);}
    else {
      putc('[', f); esc(f, a->name);
      switch (a->op) {
      case Exists: break;
      case Equals: putc('=', f); break;
      case Includes: fprintf(f, "~="); break;
      case StartsWith: fprintf(f, "^="); break;
      case EndsWith: fprintf(f, "$="); break;
      case Contains: fprintf(f, "*"); break;
      case LangMatch: fprintf(f, "|="); break;
      default: assert(!"Cannot happen");
      }
      if (a->op != Exists) esc(f, a->value);
      putc(']', f);
    }
  }
  for (p = s->pseudos; p; p = p->next) {
    switch (p->type) {
    case RootSel: fprintf(f, ":root"); break;
    case NthChild: fprintf(f, ":nth-child(%dn+%d)", p->a, p->b); break;
    case NthOfType: fprintf(f, ":nth-of-type(%dn+%d)", p->a, p->b); break;
    case FirstChild: fprintf(f, ":first-child"); break;
    case FirstOfType: fprintf(f, ":first-of-type"); break;
    case Lang: fprintf(f, ":lang("); esc(f, p->s); putc(')', f); break;
    case NthLastChild: fprintf(f, ":nth-last-child(%dn+%d)", p->a, p->b); break;
    case NthLastOfType: fprintf(f, ":nth-last-of-type(%dn+%d)",p->a,p->b);break;
    case LastChild: fprintf(f, ":last-child"); break;
    case LastOfType: fprintf(f, ":last-of-type"); break;
    case OnlyChild: fprintf(f, ":only-child"); break;
    case OnlyOfType: fprintf(f, ":only-of-type"); break;
    case Empty: fprintf(f, ":empty"); break;
    case Not: fprintf(f,":not("); dump_selector(f,p->sel); fprintf(f,")");break;
    default: assert(!"Cannot happen");
    }
  }
  for (p = s->pseudoelts; p; p = p->next) {
    switch (p->type) {
    case AttrNode: fprintf(f, "::attr("); esc(f, p->s); putc(')', f); break;
    default: assert(!"Cannot happen");
    }
  }
}

/* dump_selector -- serialize a selector */
EXPORT void dump_selector(FILE *f, const Selector s)
{
  assert(s);
  if (s->next) {
    dump_selector(f, s->next);
    fprintf(f, ", ");
  }
  if (s->context) {
    dump_selector(f, s->context);
    switch (s->combinator) {
    case Descendant: fprintf(f, " "); break;
    case Child: fprintf(f, " > "); break;
    case Adjacent: fprintf(f, " + "); break;
    case Sibling: fprintf(f, " ~ "); break;
    default: assert(!"Cannot happen");
    }
  }
  dump_simple_selector(f, s);
 }
