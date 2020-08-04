typedef struct _Dictionary * Dictionary;
extern Dictionary dict_create(int initial_size);
extern void dict_delete(Dictionary d);
extern void dict_destroy_all(Dictionary d);
extern void dict_destroy(Dictionary d, const char *key);
extern int dict_add(Dictionary d, const char *key, const char *value);
extern const char* dict_find(Dictionary d, const char* key);
extern const char *dict_next(Dictionary d, const char *prev_key);
