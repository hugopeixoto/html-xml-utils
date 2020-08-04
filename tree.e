typedef enum {
  Element, Text, Comment, Declaration, Procins, Root
} Nodetype;
typedef struct _node {
  Nodetype tp;
  string name;
  pairlist attribs;
  string text;
  string url;
  struct _node *parent;
  struct _node *sister;
  struct _node *children;
} Node, *Tree;
extern Tree create(void);
extern void tree_delete(Tree t);
extern Tree get_root(Tree t);
extern conststring get_attrib(const Node *e, const conststring attname);
extern void set_attrib(Node *e, string name, conststring value);
extern _Bool 
           delete_attrib(Node *e, const conststring name);
extern Tree get_elt_by_id(Node *n, const conststring id);
extern Tree wrap_contents(Node *n, const string elem, pairlist attr);
extern Tree wrap_elt(Node *n, const conststring elem, pairlist attr);
extern void rename_elt(Node *n, const string elem);
extern _Bool 
           is_known(const string e);
extern _Bool 
           is_pre(const string e);
extern _Bool 
           need_stag(const string e);
extern _Bool 
           need_etag(const string e);
extern _Bool 
           is_empty(const string e);
extern _Bool 
           has_parent(const string c, const string p);
extern _Bool 
           is_mixed(const string e);
extern _Bool 
           break_before(const string e);
extern _Bool 
           break_after(const string e);
extern _Bool 
           is_cdata_elt(const string e);
extern Tree tree_push(Tree t, string elem, pairlist attr);
extern Tree html_push(Tree t, string elem, pairlist attr);
extern Tree tree_pop(Tree t, string elem);
extern Tree html_pop(Tree t, string elem);
extern Tree append_comment(Tree t, string comment);
extern Tree append_declaration(Tree t, string gi,
          string fpi, string url);
extern Tree append_procins(Tree t, string procins);
extern Tree tree_append_text(Tree t, string text);
extern Tree append_text(Tree t, string text);
extern void dumptree(Tree t, FILE *f);
