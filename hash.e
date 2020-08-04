typedef struct entry {char *key; void *data;} ENTRY;
typedef enum {FIND, ENTER} ACTION;
extern int hcreate(size_t nel);
extern void hdestroy(void);
extern ENTRY *hsearch(ENTRY item, ACTION action);
