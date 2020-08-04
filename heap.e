#define fatal(msg)  fatal3(msg, __FILE__, __LINE__)
#define new(p)  if (((p)=malloc(sizeof(*(p))))); else fatal3("out of memory", __FILE__, __LINE__)
#define dispose(p)  if (!(p)) ; else (free((void*)p), (p) = (void*)0)
#define heapmax(p)  9999999
#define newstring(s)  heap_newstring(s, __FILE__, __LINE__)
#define newnstring(s,n)  heap_newnstring(s, n, __FILE__, __LINE__)
#define newarray(p,n)  if (((p)=malloc((n)*sizeof(*(p))))); else fatal3("out of memory", __FILE__, __LINE__)
#define renewarray(p,n)  if (((p)=realloc(p,(n)*sizeof(*(p))))); else fatal3("out of memory", __FILE__, __LINE__)
extern void fatal3(const char *s, const char *file, const unsigned int line);
extern char * heap_newstring(const char *s, const char *file, const int line);
extern char * heap_newnstring(const char *s, const size_t n,
         const char *file, const int line);
