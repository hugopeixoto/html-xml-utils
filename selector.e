typedef enum {
  AttrNode,
  RootSel, NthChild, NthOfType, FirstChild, FirstOfType, Lang,
  NthLastChild, NthLastOfType, LastChild, LastOfType, OnlyChild,
  OnlyOfType, Empty,
  Not,
} PseudoType;
typedef struct _PseudoCond {
  PseudoType type;
  int a, b;
  string s;
  struct _SimpleSelector *sel;
  struct _PseudoCond *next;
} PseudoCond;
typedef enum {
  Exists, Equals, Includes, StartsWith, EndsWith, Contains, LangMatch,
  HasClass, HasID
} Operator;
typedef struct _AttribCond {
  Operator op;
  string name;
  string value;
  struct _AttribCond *next;
} AttribCond;
typedef enum {
  Descendant, Child, Adjacent, Sibling
} Combinator;
typedef struct _SimpleSelector {
  string name;
  AttribCond *attribs;
  PseudoCond *pseudos;
  PseudoCond *pseudoelts;
  Combinator combinator;
  struct _SimpleSelector *context;
  struct _SimpleSelector *next;
} SimpleSelector, *Selector;
extern Selector parse_selector(const string selector, string *rest);
extern void dump_simple_selector(FILE *f, const SimpleSelector *s);
extern void dump_selector(FILE *f, const Selector s);
