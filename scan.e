extern FILE *yyin;
extern void set_yyin(FILE *f, const conststring name);
extern conststring get_yyin_name(void);
extern void include_file(FILE *f, const conststring name);
extern void set_cdata_element(const conststring e);
