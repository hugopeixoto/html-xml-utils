/*
 * Information about natural languages, in particular if the language
 * has spaces between words.
 *
 * Copyright © 1994-2012 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 *
 * Created 9 May 1998
 * Bert Bos <bert@w3.org>
 * $Id: langinfo.c,v 1.2 2019/10/05 23:25:46 bbos Exp $
 */
#include "config.h"
#ifdef HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif
#include <assert.h>
#include <stdbool.h>
#include "export.h"
#include "types.e"


/* with_spaces -- return true if the language has spaces between words */
EXPORT bool with_spaces(const conststring lang)
{
  if (!lang) return true;	/* Default is with spaces */
  if (eq(lang, "ja") || hasprefix(lang, "ja-")) return false; /* Japanese */
  if (eq(lang, "zh") || hasprefix(lang, "zh-")) return false; /* Chinese */
  if (eq(lang, "ko") || hasprefix(lang, "ko-")) return false; /* Korean */
  if (eq(lang, "km") || hasprefix(lang, "km-")) return false; /* Khmer */
  if (eq(lang, "th") || hasprefix(lang, "th-")) return false; /* Thai */
#if 0
  if (eq(lang, "lo") || hasprefix(lang, "lo-")) return false; /* Lao */
  if (eq(lang, "my") || hasprefix(lang, "my-")) return false; /* Myanmar */
#endif
  return true;			/* Other languages are with spaces */
}
