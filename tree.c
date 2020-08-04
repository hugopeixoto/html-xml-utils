/*
 * Copyright © 1997-2016 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 1997
 **/
#include "config.h"
#include <assert.h>
#include <stdlib.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
# ifndef HAVE_STRDUP
#  include "strdup.e"
# endif
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include "export.h"
#include "heap.e"
#include "types.e"
#include "dtd.e"
#include "errexit.e"
#include "scan.e"

EXPORT typedef enum {
  Element, Text, Comment, Declaration, Procins, Root
} Nodetype;

EXPORT typedef struct _node {
  Nodetype tp;
  string name;
  pairlist attribs;
  string text;
  string url;
  struct _node *parent;
  struct _node *sister;
  struct _node *children;
} Node, *Tree;

  

/* create -- create an empty tree */
EXPORT Tree create(void)
{
  Tree t = malloc(sizeof(*t));
  assert(t != NULL);
  t->tp = Root;
  t->name = "";
  t->sister = t->children = NULL;
  return t;
}

/* tree_delete -- recursively free the memory occupied by a tree */
EXPORT void tree_delete(Tree t)
{
  if (t != NULL) {
    switch (t->tp) {
      case Element:
	dispose(t->name);
	pairlist_delete(t->attribs);
	tree_delete(t->sister);
	tree_delete(t->children);
	break;
      case Text:
	dispose(t->text);
	assert(t->children == NULL);
	tree_delete(t->sister);
	break;
      case Comment:
	dispose(t->text);
	assert(t->children == NULL);
	tree_delete(t->sister);
	break;
      case Declaration:
	dispose(t->name);
	dispose(t->text);
	dispose(t->url);
	assert(t->children == NULL);
	tree_delete(t->sister);
	break;
      case Procins:
	dispose(t->text);
	assert(t->children == NULL);
	tree_delete(t->sister);
	break;
      case Root:
	assert(t->sister == NULL);
	tree_delete(t->children);
	break;
      default:
	assert(!"Cannot happen");
    }
    dispose(t);
  }
}

/* get_root -- return root of tree */
EXPORT Tree get_root(Tree t)
{
  while (t->tp != Root) t = t->parent;
  return t;
}


/* get_attrib -- return a ptr to the value of a named attibute, or false */
EXPORT conststring get_attrib(const Node *e, const conststring attname)
{
  assert(e->tp == Element);
  return pairlist_get(e->attribs, attname);
}


/* set_attrib -- set an attribute to a value */
EXPORT void set_attrib(Node *e, string name, conststring value)
{
  assert(e->tp == Element);
  pairlist_set(&e->attribs, name, value);
}

/* delete_attrib -- remove attribute from element, false if not found */
EXPORT bool delete_attrib(Node *e, const conststring name)
{
  assert(e->tp == Element);
  return pairlist_unset(&e->attribs, name);
}

/* get_by_id -- recursively get the element node with the id in subtree n */
static Tree get_by_id(Node *n, const conststring id)
{
  conststring val;
  Tree h, p;

  assert(n->tp == Element);
  if ((val = get_attrib(n, "id")) && eq(val, id)) return n;
  for (h = n->children; h; h = h->sister)
    if (h->tp == Element && (p = get_by_id(h, id))) return p;
  return NULL;
}

/* get_elt_by_id -- get the element node with the ID attribute id */
EXPORT Tree get_elt_by_id(Node *n, const conststring id)
{
  /* To do: should this use a hash table? */
  Tree h, p;

  h = get_root(n);
  for (h = h->children; h; h = h->sister)
    if (h->tp == Element && (p = get_by_id(h, id))) return p;
  return NULL;
}

/* wrap_contents -- wrap contents of a node in an element, return new elt */
EXPORT Tree wrap_contents(Node *n, const string elem, pairlist attr)
{
  Node *h, *k;

  new(h);
  h->tp = Element;
  h->name = newstring(elem);
  h->attribs = attr;
  h->sister = NULL;
  h->parent = n;
  h->children = n->children;
  n->children = h;
  for (k = h->children; k; k = k->sister) k->parent = h;
  return h;
}

/* wrap_elt -- wrap an element in a new element, return the new element */
EXPORT Tree wrap_elt(Node *n, const conststring elem, pairlist attr)
{
  Node *h, *k;

  new(h);
  h->tp = Element;
  h->name = newstring(elem);
  h->attribs = attr;
  h->sister = n->sister;
  h->parent = n->parent;
  h->children = n;
  n->sister = NULL;
  n->parent = h;
  if (h->parent->children == n) {
    h->parent->children = h;
  } else {
    k = h->parent->children;
    while (k->sister != n) {assert(k->sister->sister); k = k->sister;}
    k->sister = h;
  }
  return h;
}

/* rename_elt -- change the name of an element to elem */
EXPORT void rename_elt(Node *n, const string elem)
{
  assert(n->tp == Element);
  n->name = newstring(elem);
}

/* push -- add a child node to the tree */
static Tree push(Tree t, Node *n)
{
  if (t->children == NULL) {
    t->children = n;
  } else {
    Tree h = t->children;
    while (h->sister != NULL) h = h->sister;
    h->sister = n;
  }
  n->parent = t;
  return n;
}

/* pop -- go up one level */
static Tree pop(Tree t)
{
  assert(t != NULL);
  assert(t->tp != Root);
  return t->parent;
}

/* append -- add at end of children */
static Tree append(Tree t, Node *n)
{
  assert(t != NULL);
  if (t->children == NULL) {
    t->children = n;
  } else {
    Tree h = t->children;
    while (h->sister != NULL) h = h->sister;
    h->sister = n;
  }
  n->parent = t;
  return t;
}

/* lookup -- lookup info about an element case-insensitively */
static const ElementType *lookup(const string e)
{
  char h[MAXNAMELEN+2];
  strncpy(h, e, sizeof(h) - 1);
  h[sizeof(h)-1] = '\0';
  down(h);
  return lookup_element(h, strlen(h));
}

/* is_known -- true if the element is an HTML 4 element */
EXPORT bool is_known(const string e)
{
  return lookup(e) != NULL;
}

/* is_pre -- true if the element has preformatted content */
EXPORT bool is_pre(const string e)
{
  const ElementType *info = lookup(e);
  return info && info->pre;
}

/* need_stag -- true if the element's start tag is required */
EXPORT bool need_stag(const string e)
{
  const ElementType *info = lookup(e);
  return !info || info->stag;
}

/* need_etag -- true if the element's end tag is required */
EXPORT bool need_etag(const string e)
{
  const ElementType *info = lookup(e);
  return !info || info->etag;
}

/* is_empty -- true if element is empty */
EXPORT bool is_empty(const string e)
{
  const ElementType *info = lookup(e);
  return info && info->empty;
}

/* has_parent -- true if c accepts p as a parent */
EXPORT bool has_parent(const string c, const string p)
{
  const ElementType *info = lookup_element(c, strlen(c));
  int i;
  if (!info) return false;
  for (i = 0; info->parents[i]; i++)
    if (eq(info->parents[i], p)) return true;
  return false;
}

/* preferred_parent -- return first possible parent of e */
static string preferred_parent(const string e)
{
  const ElementType *info = lookup_element(e, strlen(e));
  assert(info != NULL);				/* element is known */
  assert(info->parents[0] != NULL);		/* element is not root */
  return info->parents[0];
}
  
/* is_root -- true if e has no possible parents */
static bool is_root(const string e)
{
  const ElementType *info = lookup_element(e, strlen(e));
  assert(info != NULL);				/* element is known */
  return info->parents[0] == NULL;
}
  
/* is_mixed -- true if e accepts text content */
EXPORT bool is_mixed(const string e)
{
  const ElementType *info = lookup(e);
  return !info || info->mixed;
}
  
/* break_before -- true if element looks better with a newline before it */
EXPORT bool break_before(const string e)
{
  const ElementType *info = lookup(e);
  return info && info->break_before;
}
  
/* break_after -- true if element looks better with a newline after it */
EXPORT bool break_after(const string e)
{
  const ElementType *info = lookup(e);
  return info && info->break_after;
}

/* is_cdata_elt -- true if element has character data content */
EXPORT bool is_cdata_elt(const string e)
{
  const ElementType *info = lookup(e);
  return info && info->cdata;
}

/* build_path -- try to add omittable start tags to make elem acceptable */
static bool build_path(Tree *t, string elem)
{
  const ElementType *info;
  Node *n;
  int i;

  assert(is_known(elem));
  assert(is_known((*t)->name));

  /* Check if we are done */
  if (has_parent(elem, (*t)->name)) return true;
  
  /* Try recursively if any possible parent can be a child of t */
  info = lookup(elem);
  for (i = 0; info->parents[i]; i++) {
    if (!need_stag(info->parents[i]) && build_path(t, info->parents[i])) {
      /* Success, so add this parent and return true */
      n = malloc(sizeof(*n));
      assert(n != NULL);
      n->tp = Element;
      n->name = newstring(info->parents[i]);
      assert(islower(n->name[0]));
      n->attribs = NULL;
      n->sister = n->children = NULL;
      *t = push(*t, n);
      return true;
    }
  }
  return false;
}

/* tree_push -- add an element to the tree, without checking the DTD */
EXPORT Tree tree_push(Tree t, string elem, pairlist attr)
{
  Node *n;
  new(n);
  n->tp = Element;
  n->name = newstring(elem);
  n->attribs = attr;
  n->sister = n->children = NULL;
  return push(t, n);
}

/* html_push -- add an element to the tree, open or close missing elements */
EXPORT Tree html_push(Tree t, string elem, pairlist attr)
{
  pairlist a;
  Node *h, *n = malloc(sizeof(*n));
  assert(n != NULL);
  n->tp = Element;
  n->name = down(newstring(elem));
  for (a = attr; a; a = a->next) down(a->name);
  n->attribs = attr;
  n->sister = n->children = NULL;

  /* Unknown elements are just pushed where they are */
  if (!is_known(n->name)) return push(t, n);

  if (is_root(n->name)) {
    while (t->tp != Root) t = pop(t);		/* Make sure root is at root */
  } else if (is_known(t->name) && build_path(&t, n->name)) {
    ;						/* Added missing start tags */
  } else {
    /* Check if there is a possible parent further up the tree */
    for (h=t; h->tp!=Root && is_known(h->name) && !has_parent(n->name,h->name);
	 h = h->parent) ;
    /* Close omitted end tags */
    if (h->tp != Root) while (t != h) t = pop(t);
    /* If no valid parent, fabricate one */
    if (t->tp == Root || (is_known(t->name) && !has_parent(n->name, t->name)))
      t = html_push(t, preferred_parent(n->name), NULL);
  }
  t = push(t, n);

  if (is_empty(n->name)) t = pop(t);
  if (is_cdata_elt(n->name)) set_cdata_element(n->name); /* Change scanner */
  return t;
}

/* tree_pop -- close an open element, without checking the DTD */
EXPORT Tree tree_pop(Tree t, string elem)
{
  assert(t != NULL);
  if (t->tp == Root) errexit("End tag </%s> without matching start tag\n", elem);
  assert(t->tp == Element);
  if (*elem == '\0') return pop(t); /* Empty end tag </> */
  if (eq(t->name, elem)) return pop(t);
  errexit("End tag </%s> doesn't match start tag <%s>\n", elem, t->name);
  return NULL;			/* Keep lint happy */
}

/* html_pop -- close an open element */
EXPORT Tree html_pop(Tree t, string elem)
{
  Tree h = t;
  assert(t != NULL);
  down(elem);
  if (*elem == '\0') {				/* </> */
    if (t->tp != Root) t = pop(t);
  } else {					/* </name> */
    for (h = t; h->tp != Root && !eq(h->name, elem); h = h->parent) ;
    if (h->tp != Root) {			/* Found open element */
      while (t != h) t = pop(t);
      t = pop(t);
    }
  }
  return t;
}

/* append_comment -- add a comment to the tree */
EXPORT Tree append_comment(Tree t, string comment)
{
  Node *n = malloc(sizeof(*n));
  assert(n != NULL);
  n->tp = Comment;
  n->text = comment;
  n->sister = n->children = NULL;
  return append(t, n);
}

/* append_declaration -- add a declaration to the tree */
EXPORT Tree append_declaration(Tree t, string gi,
			       string fpi, string url)
{
  Node *n = malloc(sizeof(*n));
  assert(n != NULL);
  n->tp = Declaration;
  n->name = down(gi);
  n->text = fpi;
  n->url = url;
  n->sister = n->children = NULL;
  return append(t, n);
}

/* append_procins -- append a processing instruction */
EXPORT Tree append_procins(Tree t, string procins)
{
  Node *n = malloc(sizeof(*n));
  assert(n != NULL);
  n->tp = Procins;
  n->text = procins;
  n->sister = n->children = NULL;
  return append(t, n);
}

/* tree_append_text -- append a text chunk, without checking the DTD */
EXPORT Tree tree_append_text(Tree t, string text)
{
  Node *n;

  assert(text);
  new(n);
  n->tp = Text;
  n->text = text;
  n->sister = n->children = NULL;
  return append(t, n);
}

/* append_text -- append a text chunk to the document tree */
EXPORT Tree append_text(Tree t, string text)
{
  Node *n;
  string new_parent;

  if (only_space(text) && (t->tp == Root || !is_mixed(t->name))) {
    /* Drop text, since it is non-significant whitespace */
    return t;
  }
  if (t->tp == Root || !is_mixed(t->name)) {
    /* Need heuristics to make a valid tree */
    new_parent = preferred_parent("%data");
    /* Close omitted end tags until text or preferred parent fits */
    while (t->tp != Root && !is_mixed(t->name) && !need_etag(t->name)
	   && !has_parent(new_parent, t->name))
      t = pop(t);
    /* Fabricate a parent if needed */
    if (t->tp == Root || !is_mixed(t->name))
      t = html_push(t, new_parent, NULL);
  }
  n = malloc(sizeof(*n));
  assert(n != NULL);
  n->tp = Text;
  n->text = text;
  assert(n->text != NULL);
  n->sister = n->children = NULL;
  return append(t, n);
}

static void dump2(Tree n, FILE *f)
{
  pairlist h;
  Tree l;

  switch (n->tp) {
    case Text: fprintf(f, "%s", n->text); break;
    case Comment: fprintf(f, "<!--%s-->", n->text); break;
    case Declaration:
      fprintf(f, "<!DOCTYPE %s", n->name);
      if (n->text) fprintf(f, " PUBLIC \"%s\">", n->text);
      if (n->url) fprintf(f, " %s\"%s\">", n->text ? "" : "SYSTEM ", n->url);
      fprintf(f, ">");
      break;
    case Procins: fprintf(f, "<?%s>", n->text); break;
    case Element:
      fprintf(f, "<%s", n->name);
      for (h = n->attribs; h != NULL; h = h->next) {
	fprintf(f, " %s", h->name);
	if (h->value != NULL) fprintf(f, "=\"%s\"", h->value);
      }
      if (is_empty(n->name)) {
	assert(n->children == NULL);
	fprintf(f, " />");
      } else {
	fprintf(f, ">");
	for (l = n->children; l != NULL; l = l->sister) dump2(l, f);
	fprintf(f, "</%s>", n->name);
      }
      break;
    default:
      assert(!"Cannot happen");
  }
}

/* dumptree -- write out the tree below t (t's children, not t itself)*/
EXPORT void dumptree(Tree t, FILE *f)
{
  Tree h;

  for (h = t->children; h != NULL; h = h->sister) dump2(h, f);
}
