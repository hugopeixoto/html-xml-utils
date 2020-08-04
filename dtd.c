/* C code produced by gperf version 3.0.4 */
/* Command-line: gperf -a -c -C -o -t -p -T -k '1,2,$' -N lookup_element dtd.hash  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "dtd.hash"
						/* -*-indented-text-*- */

/*
 * Copyright © 1998-2017 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Part of HTML-XML-utils, see:
 * http://www.w3.org/Tools/HTML-XML-utils/
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 5 Nov 1998
 *
 * Input file for gperf, to generate a perfect hash function
 * for all HTML tags, and to store each element's type.
 *
 * mixed = element accepts text content
 * empty = element is empty
 * cdata = element has character data content (i.e., unparsed content)
 * stag = start tag is required
 * etag = end tag is required
 * pre = element is preformatted
 * break_before, break_after = pretty-print with a newline before/after the elt
 * parents = array of possible parents, first one is preferred parent
 *
 * The DTD is a mixture of strict HTML 4.0 and some HTML5
 *
 */
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"

#define MAXNAMELEN 10				/* Max. len. of elt. name */
EXPORTDEF(MAXNAMELEN)

EXPORT typedef struct _ElementType {
  string name;
  bool mixed, empty, cdata, stag, etag, pre, break_before, break_after;
  string parents[76];
} ElementType;

/* lookup_element -- look up the string in the hash table */
EXPORT const ElementType * lookup_element(/* register const char *str,
					  register unsigned int len */);

/* Different kinds of parent elements: */
#define PHRASE "abbr", "acronym", "b", "bdi", "bdo", "big", "cite", "code", "dfn", "em", "i", "kbd", "q", "s", "samp", "small", "span", "strong", "sub", "sup", "time", "tt", "u", "var"
#define BRIDGE "p", "address", "caption", "dt", "h1", "h2", "h3", "h4", "h5", "h6", "legend", "pre"
#define FLOW "article", "aside", "dd", "del", "details", "div", "fieldset", "figcaption", "figure", "footer", "header", "ins", "li", "nav", "section", "main", "object", "td", "th"
#define INLINE_PARENT BRIDGE, PHRASE, FLOW
#define BLOCK_PARENT "body", "blockquote", "map", "main"

#define TOTAL_KEYWORDS 97
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 227
/* maximum key range = 227, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228,   0, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228,  35,
       30,  20,  15,   5,   0, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228,   5,  60,  80,
        5,   0,  85,  37,  65,  20, 228,  15,  10,  10,
       25,  35,  25,   0,  80,  30,   0,  65, 105,   0,
      228,  40, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228, 228, 228, 228, 228,
      228, 228, 228, 228, 228, 228
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const ElementType *
lookup_element (str, len)
     register const char *str;
     register unsigned int len;
{
  static const ElementType wordlist[] =
    {
      {""},
#line 130 "dtd.hash"
      {"q",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 153 "dtd.hash"
      {"tt",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""}, {""},
#line 90 "dtd.hash"
      {"dt",		1, 0, 0, 1, 0, 0, 1, 1, {"dl", NULL}},
      {""}, {""},
#line 143 "dtd.hash"
      {"table",		0, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 62 "dtd.hash"
      {"a",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "label", NULL}},
#line 145 "dtd.hash"
      {"td",		1, 0, 0, 1, 0, 0, 1, 1, {"tr", NULL}},
#line 146 "dtd.hash"
      {"textarea",	1, 0, 0, 1, 1, 1, 0, 0, {INLINE_PARENT, "a", "label", NULL}},
      {""},
#line 61 "dtd.hash"
      {"%data",	1, 0, 0, 1, 0, 0, 0, 0, {"p", NULL}},
      {""},
#line 84 "dtd.hash"
      {"dd",		1, 0, 0, 1, 0, 0, 1, 1, {"dl", NULL}},
#line 85 "dtd.hash"
      {"del",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, BLOCK_PARENT, "a", "button", "noscript", "form", "label", "option", "textarea", NULL}},
#line 120 "dtd.hash"
      {"meta",		0, 1, 0, 1, 0, 0, 1, 0, {INLINE_PARENT, "head", "a", "button", "noscript", "label", NULL}},
#line 92 "dtd.hash"
      {"embed",		0, 1, 0, 1, 0, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 115 "dtd.hash"
      {"legend",		1, 0, 0, 1, 1, 0, 1, 1, {"fieldset", NULL}},
#line 91 "dtd.hash"
      {"em",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""},
#line 150 "dtd.hash"
      {"title",		1, 0, 0, 1, 1, 0, 1, 1, {"head", NULL}},
      {""},
#line 89 "dtd.hash"
      {"dl",		0, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""}, {""},
#line 114 "dtd.hash"
      {"label",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", NULL}},
      {""}, {""}, {""}, {""},
#line 139 "dtd.hash"
      {"style",		1, 0, 1, 1, 1, 1, 1, 0, {"head", NULL}},
#line 134 "dtd.hash"
      {"select",		0, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "label", NULL}},
      {""}, {""}, {""},
#line 68 "dtd.hash"
      {"aside",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 108 "dtd.hash"
      {"i",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 86 "dtd.hash"
      {"details",	1, 0, 0, 1, 1, 0, 1, 1, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 119 "dtd.hash"
      {"map",		0, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 118 "dtd.hash"
      {"main",		1, 0, 0, 1, 1, 0, 1, 1, {"body", "div", "noscript", NULL}},
#line 128 "dtd.hash"
      {"param",		0, 1, 0, 1, 0, 0, 1, 1, {"object", NULL}},
#line 113 "dtd.hash"
      {"keygen",		0, 1, 0, 1, 0, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 65 "dtd.hash"
      {"address",	1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""},
#line 117 "dtd.hash"
      {"link",		0, 1, 0, 1, 0, 0, 1, 0, {"head", NULL}},
#line 110 "dtd.hash"
      {"input",		0, 1, 0, 1, 0, 0, 0, 0, {INLINE_PARENT, "a", "label", NULL}},
#line 127 "dtd.hash"
      {"p",		1, 0, 0, 1, 0, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 116 "dtd.hash"
      {"li",		1, 0, 0, 1, 0, 0, 1, 1, {"ul", "ol", NULL}},
      {""}, {""},
#line 135 "dtd.hash"
      {"small",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""},
#line 124 "dtd.hash"
      {"ol",		0, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""}, {""}, {""}, {""},
#line 133 "dtd.hash"
      {"section",	1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""},
#line 131 "dtd.hash"
      {"samp",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""},
#line 103 "dtd.hash"
      {"h6",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 122 "dtd.hash"
      {"noscript",	1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, BLOCK_PARENT, "a", "button", "noscript", "form", "label", "option", "textarea",  NULL}},
#line 71 "dtd.hash"
      {"base",		0, 1, 0, 1, 0, 0, 1, 1, {"head", NULL}},
#line 109 "dtd.hash"
      {"img",		0, 1, 0, 1, 0, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 136 "dtd.hash"
      {"source",		0, 1, 0, 1, 0, 0, 1, 1, {"audio", "video", "template", NULL}},
      {""},
#line 138 "dtd.hash"
      {"strong",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 104 "dtd.hash"
      {"head",		0, 0, 0, 0, 0, 0, 1, 1, {"html", NULL}},
#line 149 "dtd.hash"
      {"thead",		0, 0, 0, 1, 0, 0, 1, 1, {"table", NULL}},
      {""},
#line 102 "dtd.hash"
      {"h5",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 111 "dtd.hash"
      {"ins",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, BLOCK_PARENT, "a", "button", "noscript", "form", "label", "option", "textarea",  NULL}},
#line 107 "dtd.hash"
      {"html",		0, 0, 0, 0, 0, 0, 1, 1, {NULL, NULL}},
#line 75 "dtd.hash"
      {"blockquote",	0, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""}, {""},
#line 112 "dtd.hash"
      {"kbd",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 137 "dtd.hash"
      {"span",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""},
#line 154 "dtd.hash"
      {"ul",		0, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 72 "dtd.hash"
      {"bdi",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""},
#line 147 "dtd.hash"
      {"tfoot",		0, 0, 0, 1, 0, 0, 1, 1, {"table", NULL}},
#line 126 "dtd.hash"
      {"option",		1, 0, 0, 1, 0, 0, 1, 1, {"select", "optgroup", NULL}},
#line 67 "dtd.hash"
      {"article",	1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 125 "dtd.hash"
      {"optgroup",	0, 0, 0, 1, 1, 0, 1, 1, {"select", NULL}},
#line 66 "dtd.hash"
      {"area",		0, 1, 0, 1, 0, 0, 0, 0, {"map", NULL}},
      {""}, {""},
#line 101 "dtd.hash"
      {"h4",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""}, {""},
#line 152 "dtd.hash"
      {"track",		0, 1, 0, 1, 0, 0, 1, 1, {"audio", "video", "template", NULL}},
#line 123 "dtd.hash"
      {"object",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "del", "header", "aside", "section", "main", "article", "nav", "fieldset", "head", "ins", "label", "object", NULL}},
#line 64 "dtd.hash"
      {"acronym",	1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 73 "dtd.hash"
      {"bdo",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 80 "dtd.hash"
      {"cite",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 144 "dtd.hash"
      {"tbody",		0, 0, 0, 0, 0, 0, 1, 1, {"table", NULL}},
      {""},
#line 100 "dtd.hash"
      {"h3",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 129 "dtd.hash"
      {"pre",		1, 0, 0, 1, 1, 1, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""},
#line 69 "dtd.hash"
      {"audio",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 95 "dtd.hash"
      {"figure",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""},
#line 93 "dtd.hash"
      {"fieldset",	1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "form", NULL}},
      {""}, {""},
#line 132 "dtd.hash"
      {"script",		1, 0, 1, 1, 1, 1, 1, 0, {"body", INLINE_PARENT, "blockquote", "head", "map", "a", "button", "noscript", "form", "label", NULL}},
#line 79 "dtd.hash"
      {"caption",	1, 0, 0, 1, 1, 0, 1, 1, {"table", NULL}},
#line 87 "dtd.hash"
      {"dfn",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 81 "dtd.hash"
      {"code",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 74 "dtd.hash"
      {"big",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
#line 70 "dtd.hash"
      {"b",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""},
#line 142 "dtd.hash"
      {"sup",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""},
#line 99 "dtd.hash"
      {"h2",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 82 "dtd.hash"
      {"col",		0, 1, 0, 1, 0, 0, 0, 0, {"colgroup", "table", NULL}},
      {""}, {""}, {""},
#line 148 "dtd.hash"
      {"th",		1, 0, 0, 1, 0, 0, 1, 1, {"tr", NULL}},
#line 88 "dtd.hash"
      {"div",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 97 "dtd.hash"
      {"form",		0, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, NULL}},
      {""}, {""},
#line 98 "dtd.hash"
      {"h1",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 121 "dtd.hash"
      {"nav",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
#line 76 "dtd.hash"
      {"body",		0, 0, 0, 0, 0, 0, 1, 1, {"html", NULL}},
#line 94 "dtd.hash"
      {"figcaption",	1, 0, 0, 1, 1, 0, 1, 1, {"figure", NULL}},
      {""},
#line 141 "dtd.hash"
      {"summary",	1, 0, 0, 1, 1, 0, 0, 0, {"details", NULL}},
#line 157 "dtd.hash"
      {"wbr",		0, 1, 0, 1, 0, 0, 0, 1, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""}, {""},
#line 83 "dtd.hash"
      {"colgroup",	0, 0, 0, 1, 1, 0, 1, 1, {"table", NULL}},
#line 63 "dtd.hash"
      {"abbr",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""},
#line 105 "dtd.hash"
      {"header",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""}, {""}, {""}, {""},
#line 78 "dtd.hash"
      {"button",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "label", NULL}},
      {""},
#line 140 "dtd.hash"
      {"sub",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""},
#line 151 "dtd.hash"
      {"tr",		0, 0, 0, 1, 0, 0, 1, 1, {"tbody", "tfoot", "thead", NULL}},
      {""}, {""},
#line 156 "dtd.hash"
      {"video",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 155 "dtd.hash"
      {"var",		1, 0, 0, 1, 1, 0, 0, 0, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 96 "dtd.hash"
      {"footer",		1, 0, 0, 1, 1, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 77 "dtd.hash"
      {"br",		0, 1, 0, 1, 0, 0, 0, 1, {INLINE_PARENT, "a", "button", "noscript", "label", NULL}},
      {""}, {""}, {""}, {""},
#line 106 "dtd.hash"
      {"hr",		0, 1, 0, 1, 0, 0, 1, 1, {BLOCK_PARENT, FLOW, "button", "noscript", "form", NULL}}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
#line 158 "dtd.hash"

