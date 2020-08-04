/* dict -- hash table of strings indexed by strings
 *
 * dict_create(initial_size) -- create a new, empty hash table
 * dict_find(dict, key) -- return value for key, or NULL if not found
 * dict_add(dict, key, value) -- add value for key, overwriting if it exists
 * dict_destroy(dict, key) -- remove value for key
 * dict_delete(dict) -- remove table, free memory
 *
 * The dictionary will automatically expand beyond its initial size if
 * it gets full. But enlarging a large dictionary can be slow. The
 * algorithm will try to keep the dictionary less than 50% filled (as
 * long as there is memory available), so an initial size of more than
 * twice the total number of keys results in the best performance.
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Copyright © 2008 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 4 Aug 2008
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "export.h"

#define eq(s, t) (*(s) == *(t) && strcmp(s, t) == 0)


EXPORT typedef struct _Dictionary * Dictionary;

struct _Dictionary {
  unsigned long size, entries;
  unsigned long *seqno2index, *index2seqno;
  char **keys, **values;
};


/* hash -- compute a hash sum modulo n over a string */
static unsigned long hash(const unsigned long n, const char *s)
{
  unsigned long h = 5381;

  for (; *s; s++) h = ((h << 5) + h) ^ (unsigned)*s;
  h = h % n;
  if (h == 0) h = 1;		/* We don't use index 0 */
  return h;
}


/* find -- return index of key in dictionary, 0 if not found */
static unsigned long find(Dictionary d, const char *key)
{
  unsigned long index0, index, seqno;

  assert(d);
  index0 = hash(d->size, key);
  index = index0;
  while (1) {
    seqno = d->index2seqno[index];
    if (seqno >= d->entries || d->seqno2index[seqno] != index) return 0;
    if (eq(d->keys[index], key)) return index;
    index = (index + 1) % d->size;
    if (index == 0) index = 1;
    if (index == index0) return 0; /* We've tried all entries */
  }
}


/* dict_create -- create a new, empty dictionary; return NULL if no memory */
EXPORT Dictionary dict_create(int initial_size)
{
  Dictionary d;

  if (initial_size < 2) initial_size = 2;
  if (!(d = malloc(sizeof(*d)))) return NULL;
  d->keys = d->values = NULL;
  d->index2seqno = d->seqno2index = NULL;
  if (!(d->keys = malloc(initial_size * sizeof(*(d->keys)))) ||
      !(d->values = malloc(initial_size * sizeof(*(d->values)))) ||
      !(d->seqno2index = malloc(initial_size * sizeof(*(d->seqno2index)))) ||
      !(d->index2seqno = malloc(initial_size * sizeof(*(d->index2seqno))))) {
    free(d->index2seqno);
    free(d->seqno2index);
    free(d->values);
    free(d->keys);
    free(d);
    return NULL;
  }
  d->size = initial_size;
  d->entries = 0;
  return d;
}


/* dict_delete -- delete a dictionary, free all allocated memory */
EXPORT void dict_delete(Dictionary d)
{
  unsigned long i;

  assert(d);
  for (i = 0; i < d->entries; i++) {
    free(d->keys[d->seqno2index[i]]);
    free(d->values[d->seqno2index[i]]);
  }
  free(d->keys);
  free(d->values);
  free(d->seqno2index);
  free(d->index2seqno);
  free(d);
}


/* dict_destroy_all -- remove all keys and values from dictionary */
EXPORT void dict_destroy_all(Dictionary d)
{
  unsigned long i;

  assert(d);
  for (i = 0; i < d->entries; i++) {
    free(d->keys[d->seqno2index[i]]);
    free(d->values[d->seqno2index[i]]);
  }
  d->entries = 0;
}


/* dict_destroy -- delete a value from the dictionary */
EXPORT void dict_destroy(Dictionary d, const char *key)
{
  unsigned long index, seqno;

  assert(d);
  if ((index = find(d, key)) > 0) {
    free(d->keys[index]);
    free(d->values[index]);
    seqno = d->index2seqno[index];
    assert(seqno < d->entries);
    assert(d->entries > 0);
    d->entries--;
    d->seqno2index[seqno] = d->seqno2index[d->entries];
    d->index2seqno[d->seqno2index[seqno]] = seqno;
  }
}


/* Forward declaration */
static int expand(Dictionary d);


/* dict_add -- add a key-value pair to dictionary, return 0 if out of memory */
EXPORT int dict_add(Dictionary d, const char *key, const char *value)
{
  unsigned long index0, index, seqno, found;

  assert(d);
  index0 = hash(d->size, key);
  index = index0;
  while (1) {
    seqno = d->index2seqno[index];
    if (seqno >= d->entries) { found = 0; break; }
    if (d->seqno2index[seqno] != index) { found = 0; break; }
    if (eq(d->keys[index], key)) { found = 1; break; }
    index = (index + 1) % d->size;
    if (index == 0) index = 1;
    if (index == index0) {if (!expand(d)) return 0; } /* Dictionary full */
  }
  if (found) {
    free(d->values[index]);
    d->values[index] = strdup(value);
    if (!d->values[index]) return 0; /* Out of memory */
  } else {
    assert(d->entries < d->size);
    d->keys[index] = strdup(key);
    if (!d->keys[index]) return 0; /* Out of memory */
    d->values[index] = strdup(value);
    if (!d->values[index]) { free(d->keys[index]); return 0; } /* Out of mem. */
    d->seqno2index[d->entries] = index;
    d->index2seqno[index] = d->entries;
    d->entries++;
    if (d->entries > d->size/2) (void) expand(d); /* Try expand, fail is OK */
  }
  return 1;
}


/* expand -- make the tables in the dictionary larger, return 0 if out of mem */
static int expand(Dictionary d)
{
  unsigned long i;
  Dictionary h;

  assert(d);
  h = dict_create(2 * d->size);
  if (!h) return 0;

  /* Add all old entries to the new dictionary */
  for (i = 0; i < d->entries; i++) {
    if (!dict_add(h, d->keys[d->seqno2index[i]],d->values[d->seqno2index[i]])) {
      dict_delete(h);
      return 0;			/* Failed */
    }
  }

  /* Succeeded in making a copy, now delete the old entries */
  for (i = 0; i < d->entries; i++) {
    free(d->keys[d->seqno2index[i]]);
    free(d->values[d->seqno2index[i]]);
  }
  free(d->keys);
  free(d->values);
  free(d->seqno2index);
  free(d->index2seqno);

  /* Put the larger arrays from the new dictionary in the old dictionary */
  d->size = h->size;
  assert(d->entries == h->entries);
  d->keys = h->keys;
  d->values = h->values;
  d->index2seqno = h->index2seqno;
  d->seqno2index = h->seqno2index;
  free(h);

  return 1;
}


/* dict_find -- return value associated with key in dictionary, or NULL */
EXPORT const char* dict_find(Dictionary d, const char* key)
{
  unsigned long i;

  assert(d);
  if ((i = find(d, key)) > 0) return d->values[i]; else return NULL;
}


/* dict_next -- return next key, or 1st key if NULL, or NULL if no more */
EXPORT const char *dict_next(Dictionary d, const char *prev_key)
{
  unsigned long index, seqno;

  assert(d);
  if (d->entries == 0) return NULL; /* Empty dictionary */
  if (!prev_key) return d->keys[d->seqno2index[0]]; /* Return first key */
  index = find(d, prev_key);
  if (index == 0) return NULL;	/* Error, unknown key */
  seqno = d->index2seqno[index] + 1;
  if (seqno == d->entries) return NULL; /* No more keys */
  return d->keys[d->seqno2index[seqno]];
}
