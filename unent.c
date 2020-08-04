/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -a -c -C -o -t -p -k '1,2,$' -D -N lookup_entity unent.hash  */

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
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "unent.hash"
						/* -*-indented-text-*- */

/*
 * Copyright © 1998-2010 World Wide Web Consortium
 * See http://www.w3.org/Consortium/Legal/copyright-software
 *
 * Author: Bert Bos <bert@w3.org>
 * Created: 2 Dec 1998
 *
 * Input file for gperf, to generate a perfect hash function
 * of all HTML named character entities. This list translates
 * names to Unicode numbers.
 */
#include <config.h>
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "export.h"

EXPORT struct _Entity {char *name; unsigned int code;};
EXPORT const struct _Entity *lookup_entity (register const char *str,
       	     	    	    		    register size_t len);


#line 37 "unent.hash"
struct _Entity;

#define TOTAL_KEYWORDS 253
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 10
#define MAX_HASH_VALUE 533
/* maximum key range = 524, duplicates = 2 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned short asso_values[] =
    {
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534,  40,
       70,  20,  50, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 193,   5, 215,   0, 219,
      534, 185,   5, 215, 190,  45,   5,   0,  30, 199,
       35, 534,  15,  15,  10, 110,   0, 534,  10,  40,
        0, 534, 534, 534, 534, 534, 534,  10, 210,   5,
      155,   0, 200,  10,  15, 100, 175, 155,  20, 150,
      145,  45,  65, 200,  35, 105,  55,  75, 140, 115,
        0, 250,  80, 534, 100, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534, 534, 534,
      534, 534, 534, 534, 534, 534, 534, 534
    };
  return len + asso_values[(unsigned char)str[1]+2] + asso_values[(unsigned char)str[0]] + asso_values[(unsigned char)str[len - 1]];
}

const struct _Entity *
lookup_entity (register const char *str, register size_t len)
{
  static const struct _Entity wordlist[] =
    {
#line 114 "unent.hash"
      {"ecirc", 234},
#line 113 "unent.hash"
      {"eacute", 233},
#line 60 "unent.hash"
      {"acute", 180},
#line 106 "unent.hash"
      {"acirc", 226},
#line 105 "unent.hash"
      {"aacute", 225},
#line 239 "unent.hash"
      {"ge", 8805},
#line 142 "unent.hash"
      {"Zeta", 918},
#line 140 "unent.hash"
      {"Delta", 916},
#line 147 "unent.hash"
      {"Lambda", 923},
#line 138 "unent.hash"
      {"Beta", 914},
#line 163 "unent.hash"
      {"gamma", 947},
#line 111 "unent.hash"
      {"ccedil", 231},
#line 238 "unent.hash"
      {"le", 8804},
#line 110 "unent.hash"
      {"aelig", 230},
#line 253 "unent.hash"
      {"lang", 9001},
#line 64 "unent.hash"
      {"cedil", 184},
#line 171 "unent.hash"
      {"lambda", 955},
#line 249 "unent.hash"
      {"lceil", 8968},
#line 288 "unent.hash"
      {"Dagger", 8225},
#line 223 "unent.hash"
      {"radic", 8730},
#line 101 "unent.hash"
      {"Yacute", 221},
#line 254 "unent.hash"
      {"rang", 9002},
#line 124 "unent.hash"
      {"ocirc", 244},
#line 123 "unent.hash"
      {"oacute", 243},
#line 54 "unent.hash"
      {"reg", 174},
#line 204 "unent.hash"
      {"harr", 8596},
#line 250 "unent.hash"
      {"rceil", 8969},
#line 200 "unent.hash"
      {"larr", 8592},
#line 146 "unent.hash"
      {"Kappa", 922},
#line 197 "unent.hash"
      {"real", 8476},
#line 266 "unent.hash"
      {"oelig", 339},
#line 42 "unent.hash"
      {"cent", 162},
#line 51 "unent.hash"
      {"laquo", 171},
#line 251 "unent.hash"
      {"lfloor", 8970},
#line 229 "unent.hash"
      {"cap", 8745},
#line 202 "unent.hash"
      {"rarr", 8594},
#line 109 "unent.hash"
      {"aring", 229},
#line 62 "unent.hash"
      {"para", 182},
#line 131 "unent.hash"
      {"ucirc", 251},
#line 130 "unent.hash"
      {"uacute", 250},
#line 226 "unent.hash"
      {"ang", 8736},
#line 67 "unent.hash"
      {"raquo", 187},
#line 252 "unent.hash"
      {"rfloor", 8971},
#line 155 "unent.hash"
      {"Tau", 932},
#line 192 "unent.hash"
      {"Prime", 8243},
#line 190 "unent.hash"
      {"hellip", 8230},
#line 205 "unent.hash"
      {"crarr", 8629},
#line 289 "unent.hash"
      {"permil", 8240},
#line 166 "unent.hash"
      {"zeta", 950},
#line 185 "unent.hash"
      {"omega", 969},
#line 112 "unent.hash"
      {"egrave", 232},
#line 118 "unent.hash"
      {"icirc", 238},
#line 117 "unent.hash"
      {"iacute", 237},
#line 273 "unent.hash"
      {"emsp", 8195},
#line 198 "unent.hash"
      {"trade", 8482},
#line 104 "unent.hash"
      {"agrave", 224},
#line 201 "unent.hash"
      {"uarr", 8593},
#line 99 "unent.hash"
      {"Ucirc", 219},
#line 98 "unent.hash"
      {"Uacute", 218},
#line 261 "unent.hash"
      {"amp", 38},
#line 191 "unent.hash"
      {"prime", 8242},
#line 212 "unent.hash"
      {"part", 8706},
#line 187 "unent.hash"
      {"upsih", 978},
#line 272 "unent.hash"
      {"ensp", 8194},
#line 41 "unent.hash"
      {"iexcl", 161},
#line 258 "unent.hash"
      {"hearts", 9829},
#line 228 "unent.hash"
      {"or", 8744},
#line 180 "unent.hash"
      {"tau", 964},
#line 115 "unent.hash"
      {"euml", 235},
#line 213 "unent.hash"
      {"exist", 8707},
#line 128 "unent.hash"
      {"oslash", 248},
#line 48 "unent.hash"
      {"uml", 168},
#line 247 "unent.hash"
      {"perp", 8869},
#line 281 "unent.hash"
      {"lsquo", 8216},
#line 290 "unent.hash"
      {"lsaquo", 8249},
#line 108 "unent.hash"
      {"auml", 228},
#line 196 "unent.hash"
      {"image", 8465},
#line 122 "unent.hash"
      {"ograve", 242},
#line 167 "unent.hash"
      {"eta", 951},
#line 262 "unent.hash"
      {"apos", 39},
#line 235 "unent.hash"
      {"asymp", 8776},
#line 107 "unent.hash"
      {"atilde", 227},
#line 236 "unent.hash"
      {"ne", 8800},
#line 120 "unent.hash"
      {"eth", 240},
#line 282 "unent.hash"
      {"rsquo", 8217},
#line 291 "unent.hash"
      {"rsaquo", 8250},
#line 292 "unent.hash"
      {"euro", 8364},
#line 215 "unent.hash"
      {"nabla", 8711},
#line 267 "unent.hash"
      {"Scaron", 352},
#line 270 "unent.hash"
      {"circ", 710},
#line 161 "unent.hash"
      {"alpha", 945},
#line 47 "unent.hash"
      {"sect", 167},
#line 170 "unent.hash"
      {"kappa", 954},
#line 89 "unent.hash"
      {"Ntilde", 209},
#line 56 "unent.hash"
      {"deg", 176},
#line 269 "unent.hash"
      {"Yuml", 376},
#line 164 "unent.hash"
      {"delta", 948},
#line 129 "unent.hash"
      {"ugrave", 249},
#line 126 "unent.hash"
      {"ouml", 246},
#line 154 "unent.hash"
      {"Sigma", 931},
#line 165 "unent.hash"
      {"epsilon", 949},
#line 230 "unent.hash"
      {"cup", 8746},
#line 224 "unent.hash"
      {"prop", 8733},
#line 245 "unent.hash"
      {"oplus", 8853},
#line 125 "unent.hash"
      {"otilde", 245},
#line 148 "unent.hash"
      {"Mu", 924},
#line 55 "unent.hash"
      {"macr", 175},
#line 193 "unent.hash"
      {"oline", 8254},
#line 195 "unent.hash"
      {"weierp", 8472},
#line 203 "unent.hash"
      {"darr", 8595},
#line 144 "unent.hash"
      {"Theta", 920},
#line 287 "unent.hash"
      {"dagger", 8224},
#line 74 "unent.hash"
      {"Acirc", 194},
#line 73 "unent.hash"
      {"Aacute", 193},
#line 139 "unent.hash"
      {"Gamma", 915},
#line 116 "unent.hash"
      {"igrave", 236},
#line 264 "unent.hash"
      {"gt", 62},
#line 92 "unent.hash"
      {"Ocirc", 212},
#line 91 "unent.hash"
      {"Oacute", 211},
#line 159 "unent.hash"
      {"Psi", 936},
#line 132 "unent.hash"
      {"uuml", 252},
#line 271 "unent.hash"
      {"tilde", 732},
#line 97 "unent.hash"
      {"Ugrave", 217},
#line 263 "unent.hash"
      {"lt", 60},
#line 234 "unent.hash"
      {"cong", 8773},
#line 103 "unent.hash"
      {"szlig", 223},
#line 149 "unent.hash"
      {"Nu", 925},
#line 231 "unent.hash"
      {"int", 8747},
#line 243 "unent.hash"
      {"sube", 8838},
#line 244 "unent.hash"
      {"supe", 8839},
#line 86 "unent.hash"
      {"Icirc", 206},
#line 85 "unent.hash"
      {"Iacute", 205},
#line 88 "unent.hash"
      {"ETH", 208},
#line 277 "unent.hash"
      {"lrm", 8206},
#line 82 "unent.hash"
      {"Ecirc", 202},
#line 81 "unent.hash"
      {"Eacute", 201},
#line 227 "unent.hash"
      {"and", 8743},
#line 162 "unent.hash"
      {"beta", 946},
#line 102 "unent.hash"
      {"THORN", 222},
#line 153 "unent.hash"
      {"Rho", 929},
#line 119 "unent.hash"
      {"iuml", 239},
#line 79 "unent.hash"
      {"Ccedil", 199},
#line 175 "unent.hash"
      {"omicron", 959},
#line 184 "unent.hash"
      {"psi", 968},
#line 59 "unent.hash"
      {"sup3", 179},
#line 168 "unent.hash"
      {"theta", 952},
#line 100 "unent.hash"
      {"Uuml", 220},
#line 237 "unent.hash"
      {"equiv", 8801},
#line 256 "unent.hash"
      {"spades", 9824},
#line 66 "unent.hash"
      {"ordm", 186},
#line 268 "unent.hash"
      {"scaron", 353},
#line 174 "unent.hash"
      {"xi", 958},
#line 177 "unent.hash"
      {"rho", 961},
#line 160 "unent.hash"
      {"Omega", 937},
#line 257 "unent.hash"
      {"clubs", 9827},
#line 133 "unent.hash"
      {"yacute", 253},
#line 181 "unent.hash"
      {"upsilon", 965},
#line 77 "unent.hash"
      {"Aring", 197},
#line 65 "unent.hash"
      {"sup1", 185},
#line 71 "unent.hash"
      {"iquest", 191},
#line 150 "unent.hash"
      {"Xi", 926},
#line 210 "unent.hash"
      {"hArr", 8660},
#line 284 "unent.hash"
      {"ldquo", 8220},
#line 44 "unent.hash"
      {"curren", 164},
#line 206 "unent.hash"
      {"lArr", 8656},
#line 179 "unent.hash"
      {"sigma", 963},
#line 219 "unent.hash"
      {"prod", 8719},
#line 194 "unent.hash"
      {"frasl", 8260},
#line 222 "unent.hash"
      {"lowast", 8727},
#line 183 "unent.hash"
      {"chi", 967},
#line 285 "unent.hash"
      {"rdquo", 8221},
#line 232 "unent.hash"
      {"there4", 8756},
#line 241 "unent.hash"
      {"sup", 8835},
#line 208 "unent.hash"
      {"rArr", 8658},
#line 121 "unent.hash"
      {"ntilde", 241},
#line 152 "unent.hash"
      {"Pi", 928},
#line 58 "unent.hash"
      {"sup2", 178},
#line 96 "unent.hash"
      {"Oslash", 216},
#line 246 "unent.hash"
      {"otimes", 8855},
#line 156 "unent.hash"
      {"Upsilon", 933},
#line 72 "unent.hash"
      {"Agrave", 192},
#line 214 "unent.hash"
      {"empty", 8709},
#line 274 "unent.hash"
      {"thinsp", 8201},
#line 255 "unent.hash"
      {"loz", 9674},
#line 50 "unent.hash"
      {"ordf", 170},
#line 90 "unent.hash"
      {"Ograve", 210},
#line 46 "unent.hash"
      {"brvbar", 166},
#line 283 "unent.hash"
      {"sbquo", 8218},
#line 68 "unent.hash"
      {"frac14", 188},
#line 70 "unent.hash"
      {"frac34", 190},
#line 199 "unent.hash"
      {"alefsym", 8501},
#line 157 "unent.hash"
      {"Phi", 934},
#line 169 "unent.hash"
      {"iota", 953},
#line 225 "unent.hash"
      {"infin", 8734},
#line 127 "unent.hash"
      {"divide", 247},
#line 95 "unent.hash"
      {"times", 215},
#line 84 "unent.hash"
      {"Igrave", 204},
#line 176 "unent.hash"
      {"pi", 960},
#line 216 "unent.hash"
      {"isin", 8712},
#line 80 "unent.hash"
      {"Egrave", 200},
#line 207 "unent.hash"
      {"uArr", 8657},
#line 69 "unent.hash"
      {"frac12", 189},
#line 76 "unent.hash"
      {"Auml", 196},
#line 278 "unent.hash"
      {"rlm", 8207},
#line 173 "unent.hash"
      {"nu", 957},
#line 94 "unent.hash"
      {"Ouml", 214},
#line 75 "unent.hash"
      {"Atilde", 195},
#line 172 "unent.hash"
      {"mu", 956},
#line 182 "unent.hash"
      {"phi", 966},
#line 93 "unent.hash"
      {"Otilde", 213},
#line 189 "unent.hash"
      {"bull", 8226},
#line 137 "unent.hash"
      {"Alpha", 913},
#line 87 "unent.hash"
      {"Iuml", 207},
#line 61 "unent.hash"
      {"micro", 181},
#line 83 "unent.hash"
      {"Euml", 203},
#line 57 "unent.hash"
      {"plusmn", 177},
#line 188 "unent.hash"
      {"piv", 982},
#line 248 "unent.hash"
      {"sdot", 8901},
#line 279 "unent.hash"
      {"ndash", 8211},
#line 63 "unent.hash"
      {"middot", 183},
#line 40 "unent.hash"
      {"nbsp", 160},
#line 280 "unent.hash"
      {"mdash", 8212},
#line 143 "unent.hash"
      {"Eta", 919},
#line 220 "unent.hash"
      {"sum", 8721},
#line 260 "unent.hash"
      {"quot", 34},
#line 134 "unent.hash"
      {"thorn", 254},
#line 186 "unent.hash"
      {"thetasym", 977},
#line 135 "unent.hash"
      {"yuml", 255},
#line 78 "unent.hash"
      {"AElig", 198},
#line 151 "unent.hash"
      {"Omicron", 927},
#line 265 "unent.hash"
      {"OElig", 338},
#line 218 "unent.hash"
      {"ni", 8715},
#line 52 "unent.hash"
      {"not", 172},
#line 141 "unent.hash"
      {"Epsilon", 917},
#line 45 "unent.hash"
      {"yen", 165},
#line 209 "unent.hash"
      {"dArr", 8659},
#line 233 "unent.hash"
      {"sim", 8764},
#line 221 "unent.hash"
      {"minus", 8722},
#line 259 "unent.hash"
      {"diams", 9830},
#line 43 "unent.hash"
      {"pound", 163},
#line 211 "unent.hash"
      {"forall", 8704},
#line 145 "unent.hash"
      {"Iota", 921},
#line 240 "unent.hash"
      {"sub", 8834},
#line 242 "unent.hash"
      {"nsub", 8836},
#line 49 "unent.hash"
      {"copy", 169},
#line 286 "unent.hash"
      {"bdquo", 8222},
#line 178 "unent.hash"
      {"sigmaf", 962},
#line 136 "unent.hash"
      {"fnof", 402},
#line 158 "unent.hash"
      {"Chi", 935},
#line 217 "unent.hash"
      {"notin", 8713},
#line 276 "unent.hash"
      {"zwj", 8205},
#line 275 "unent.hash"
      {"zwnj", 8204},
#line 53 "unent.hash"
      {"shy", 173}
    };

  static const short lookup[] =
    {
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,    0,    1,   -1,   -1,   -1,    2,
        -1,   -1,   -1,   -1,    3,    4,    5,   -1,
         6,    7,    8,   -1,   -1,    9,   10,   11,
        12,   -1,   -1,   13,   -1,   -1,   -1,   14,
        15,   16,   -1,   -1,   -1,   17,   18,   -1,
        -1,   -1,   19,   20,   -1,   -1,   21,   22,
        23,   -1,   24,   25,   26,   -1,   -1,   -1,
        27,   28,   -1,   -1,   -1,   29,   30,   -1,
        -1,   -1,   31,   32,   33,   -1,   34,   35,
        36,   -1,   -1,   -1,   37,   38,   39,   -1,
        40,   -1,   41,   42,   -1,   43,   -1,   44,
        45,   -1,   -1,   -1,   46,   47,   -1,   -1,
        48,   49,   50,   -1,   -1,   -1,   51,   52,
        -1,   -1,   53,   54,   55,   -1,   -1,   56,
        57,   58,   -1,   59,   -1,   60,   -1,   -1,
        -1,   61,   62,   -1,   -1,   -1,   63,   64,
        65,   66,   67,   68,   69,   70,   -1,   71,
        72,   73,   74,   -1,   -1,   75,   76,   77,
        -1,   78,   79,   80,   81,   82,   83,   -1,
        84,   85,   -1,   -1,   86,   87,   88,   -1,
        -1,   89,   90,   -1,   -1,   -1,   91,   92,
        93,   -1,   94,   95,   96,   97,   -1,   -1,
        98,   99,   -1,  100,  101,  102,  103,  104,
       105,   -1,  106,  107,  108,   -1,   -1,  109,
       110,  111,   -1,  112,  113,  114,  115,  116,
        -1,  117,  118,   -1,   -1,  119,  120,  121,
       122,  123,   -1,  124,  125,   -1,  126,  127,
      -485,  130,  131,  132,  133,  134,  135, -125,
        -2,  136,  137,  138,   -1,   -1,  139,  140,
        -1,  141,  142,  143,  144,  145,   -1,   -1,
        -1,  146,  147,  148,   -1,   -1,  149,   -1,
       150,  151,  152,  153,  154,  155,  156,  157,
       158,   -1,  159,  160,   -1,  161,  162,  163,
        -1,   -1,  164,  165,   -1,   -1,   -1,  166,
       167,  168,   -1,  169,   -1,  170,  171,   -1,
       172,  173,   -1,  174,  175,   -1,  176,  177,
       178,  179,   -1,  180,  181,  182,   -1,  183,
       184,  185,  186,   -1,   -1,   -1,  187, -571,
       190,  191,  192,  193,  194,  -65,   -2,   -1,
       195,  196,  197,   -1,  198,  199,   -1,   -1,
        -1,  200,   -1,  201,  202,  203,   -1,   -1,
        -1,  204,  205,  206,   -1,   -1,  207,  208,
        -1,  209,   -1,   -1,   -1,  210,   -1,   -1,
        -1,  211,  212,  213,   -1,   -1,  214,   -1,
        -1,  215,   -1,  216,  217,  218,  219,   -1,
        -1,  220,  221,   -1,  222,  223,  224,   -1,
        -1,   -1,   -1,   -1,  225,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,  226,  227,   -1,   -1,
        -1,  228,   -1,   -1,  229,   -1,   -1,  230,
        -1,   -1,  231,  232,   -1,   -1,  233,   -1,
       234,  235,   -1,   -1,   -1,  236,   -1,  237,
        -1,   -1,   -1,   -1,  238,   -1,   -1,   -1,
        -1,  239,  240,   -1,   -1,  241,   -1,   -1,
        -1,  242,  243,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,  244,  245,   -1,   -1,   -1,
        -1,   -1,  246,   -1,   -1,  247,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,  248,   -1,  249,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,  250,  251,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,  252
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist[index];
            }
          else if (index < -TOTAL_KEYWORDS)
            {
              register int offset = - 1 - TOTAL_KEYWORDS - index;
              register const struct _Entity *wordptr = &wordlist[TOTAL_KEYWORDS + lookup[offset]];
              register const struct _Entity *wordendptr = wordptr + -lookup[offset + 1];

              while (wordptr < wordendptr)
                {
                  register const char *s = wordptr->name;

                  if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                    return wordptr;
                  wordptr++;
                }
            }
        }
    }
  return 0;
}
#line 293 "unent.hash"

