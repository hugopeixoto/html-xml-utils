struct _Entity {char *name; unsigned int code;};
extern const struct _Entity *lookup_entity (register const char *str,
                             register size_t len);
