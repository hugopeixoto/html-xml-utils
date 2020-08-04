/*
 * Match a selector against a element in a document tree.
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Copyright © 2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 11 Aug 2017
 */
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#ifdef HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif
#include "types.e"
#include "tree.e"
#include "selector.e"
#include "export.h"
#include "heap.e"
#include "errexit.e"


static conststring language = "";		/* Initial language */
static bool case_insensitive = false;		/* How to match elems/attrs */


bool matches_sel(const Tree t, const Selector s);


/* init_language -- set the initial language for the document */
EXPORT void init_language(const conststring lang)
{
  language = newstring(lang);
}


/* set_case_insensitive -- make all string comparisons case-insensitive */
EXPORT void set_case_insensitive(void)
{
  case_insensitive = true;
}


/* get_language -- return the (inherited) human language of an element */
static conststring get_language(const Node *n)
{
  conststring s;

  assert(n);
  if (n->tp == Root) return language; /* Language from -l option */
  assert(n->tp == Element);
  if ((s = get_attrib(n, "xml:lang"))) return s;
  if ((s = get_attrib(n, "lang"))) return s;
  return get_language(n->parent);
}


/* same -- compare two names, case-(in)sensitively, depending */
EXPORT bool same(const string a, const string b)
{
  return case_insensitive ? strcasecmp(a, b) == 0 : eq(a, b);
}


/* count_siblings - compute own index and total number of siblings of t */
static void count_siblings(const Node *n,
			   int *index,     /* Index of n among its siblings */
			   int *typeindex, /* Index among those of same type */
			   int *total,     /* Total # of siblings, including n */
			   int *typetotal) /* Total # of siblings of same type */
{
  Node *h;

  for (*total = *typetotal = 0, h = n->parent->children; h; h = h->sister)
    if (h->tp == Element) {
      (*total)++;
      if (same(h->name, n->name)) (*typetotal)++;
      if (h == n) {*index = *total; *typeindex = *typetotal;}
    }
}


/* get_attr -- return the value of the named attribute, or NULL */
static string get_attr(const Node *n, const string name)
{
  pairlist p;

  for (p = n->attribs; p && !same(p->name, name); p = p->next) ;
  return p ? p->value : NULL;
}


/* includes -- check for word in the space-separated words of line */
static bool includes(const string line, const string word)
{
  int i = 0, n = strlen(word);

  /* What should happen if word is the empty string? */
  /* To do: compare with contains() in class.c, keep the best */
  while (line[i]) {
    if (case_insensitive) {
      if (!strncasecmp(line+i, word, n) && (!line[i+n] || isspace(line[i+n])))
	return true;
    } else {
      if (!strncmp(line+i, word, n) && (!line[i+n] || isspace(line[i+n])))
	return true;
    }
    do i++; while (line[i] && !isspace(line[i]));
    while (isspace(line[i])) i++;
  }
  return false;
}


/* starts_with -- check if line starts with prefix */
static bool starts_with(const string line, const string prefix)
{
  return case_insensitive
    ? strncasecmp(line, prefix, strlen(prefix)) == 0
    : strncmp(line, prefix, strlen(prefix)) == 0;
}


/* ends_with -- check if line ends with suffix */
static bool ends_with(const string line, const string suffix)
{
  int n1 = strlen(line), n2 = strlen(suffix);
  return n1 >= n2 && eq(line + n1 - n2, suffix);
}


/* contains -- check if line contains s */
static bool contains(const string line, const string s)
{
  return strstr(line, s) != NULL;
}


/* lang_match -- check if language specific is subset of general */
static bool lang_match(const conststring specific, const conststring general)
{
  assert(specific);
  assert(general);
  size_t n = strlen(general);
  return !strncasecmp(specific, general, n)
    && (specific[n] == '-' || !specific[n]);
}


/* simple_match -- check if a node matches a simple selector */
static bool simple_match(const Tree n, const SimpleSelector *s)
{
  int index = 0, tpindex = 0, total = 0, tptotal = 0;
  AttribCond *p;
  PseudoCond *q;
  string h;
  Node *c;

  /* Pseudo-elements can't match elements */
  if (s->pseudoelts) return false;

  /* Match the type selector */
  if (s->name && !same(s->name, n->name)) return false;

  /* Match the attribute selectors, including class and ID */
  for (p = s->attribs; p; p = p->next) {
    if (!(h = get_attr(n, (p->op == HasClass) ? (string)"class"
		       : (p->op == HasID) ? (string)"id" : p->name)))
      return false;
    switch (p->op) {
    case Exists: break;
    case Equals:
    case HasID: if (!eq(p->value, h)) return false; break;
    case Includes:
    case HasClass: if (!includes(h, p->value)) return false; break;
    case StartsWith: if (!starts_with(h, p->value)) return false; break;
    case EndsWith: if (!ends_with(h, p->value)) return false; break;
    case Contains: if (!contains(h, p->value)) return false; break;
    case LangMatch: if (!lang_match(h, p->value)) return false; break;
    default: assert(!"Cannot happen");
    }
  }

  /* Match the pseudo-classes */
  for (q = s->pseudos; q; q = q->next) {
    switch (q->type) {
    case RootSel:
      if (n->parent->tp != Root) return false;
      break;
    case NthChild:
      if (index == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      if (q->a == 0) return index == q->b;
      else return (index - q->b) % q->a == 0 && (index - q->b) / q->a >= 0;
      break;
    case NthLastChild:
      if (index == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      if (q->a == 0) return total - tpindex + 1 == q->b;
      else return (total - tpindex + 1 - q->b) % q->a == 0
	     && (total - tpindex + 1 - q->b) / q->a >= 0;
      break;
    case NthOfType:
      if (index == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      if (q->a == 0) return tpindex == q->b;
      else return (tpindex - q->b) % q->a == 0 && (tpindex - q->b) / q->a >= 0;
      break;
    case NthLastOfType:
      if (tpindex == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      if (q->a == 0) return tptotal - tpindex + 1 == q->b;
      else return (tptotal - tpindex + 1 - q->b) % q->a == 0
	     && (tptotal - tpindex + 1 - q->b) / q->a >= 0;
      break;
    case FirstChild:
      if (index == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      return index == 1;
      break;
    case LastChild:
      if (index == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      return index == total;
      break;
    case FirstOfType:
      if (tpindex == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      return tpindex == 1;
      break;
    case LastOfType:
      if (tpindex == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      return tpindex == tptotal;
      break;
    case OnlyChild:
      if (index == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      return total == 1;
      break;
    case OnlyOfType:
      if (tpindex == 0) count_siblings(n, &index, &tpindex, &total, &tptotal);
      return tptotal == 1;
      break;
    case Empty:
      for (c = n->children; c; c = c->sister)
	if (c->tp == Element || c->tp == Text) return false;
      return true;
      break;
    case Lang:
      if (!lang_match(get_language(n), q->s)) return false;
      break;
    case Not:
      if (matches_sel(n, q->sel)) return false;
      break;
    default:
      assert(!"Cannot happen");
    }
  }
  return true;
}


/* matches_sel -- check if an element t matches the selector s */
EXPORT bool matches_sel(const Tree t, const Selector s)
{
  Tree g, h;

  assert(s);
  if (!t || t->tp == Root) return false;
  assert(t->tp == Element);
  if (!simple_match(t, s)) return s->next && matches_sel(t, s->next);
  if (!s->context) return true;
  switch (s->combinator) {
  case Descendant:
    for (h = t->parent; h->tp != Root && !matches_sel(h, s->context);
	 h = h->parent);
    return h->tp != Root;
  case Child:
    return matches_sel(t->parent, s->context);
  case Adjacent:
    for (g = NULL, h = t->parent->children; h != t; h = h->sister)
      if (h->tp == Element) g = h;
    return g && matches_sel(g, s->context);
  case Sibling:
    for (h = t->parent->children; h != t; h = h->sister)
      if (matches_sel(h, s->context)) return true;
    return false;
  default:
    assert(!"Cannot happen");
    return false;
  }
}
