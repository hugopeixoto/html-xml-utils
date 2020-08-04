typedef char *string;
typedef const char *conststring;
typedef struct _pairlist {
  string name;
  string value;
  struct _pairlist *next;
} *pairlist;
typedef unsigned int MediaSet;
enum _Media {
  MediaNone = 0,
  MediaPrint = (1 << 0),
  MediaScreen = (1 << 1),
  MediaTTY = (1 << 2),
  MediaBraille = (1 << 3),
  MediaTV = (1 << 4),
  MediaProjection = (1 << 5),
  MediaEmbossed = (1 << 6),
  MediaAll = 0xFF
};
#define eq(s, t)  (*(s) == *(t) && strcmp(s, t) == 0)
#define hexval(c)  ((c) <= '9' ? (c)-'0' : (c) <= 'F' ? 10+(c)-'A' : 10+(c)-'a')
extern void pairlist_delete(pairlist p);
extern pairlist pairlist_copy(const pairlist p);
extern conststring pairlist_get(pairlist p, const conststring name);
extern void pairlist_set(pairlist *p, const conststring name,
    const conststring val);
extern _Bool 
           pairlist_unset(pairlist *p, const conststring name);
extern string strapp(string *s,...);
extern void chomp(string s);
extern int min(int a, int b);
extern int max(int a, int b);
extern string down(const string s);
extern _Bool 
           hasprefix(conststring s, conststring prefix);
extern _Bool 
           hasaffix(conststring s, conststring affix);
extern _Bool 
           only_space(conststring s);
