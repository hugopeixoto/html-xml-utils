#ifndef _FOPENCOOKIE_H_
#define _FOPENCOOKIE_H_
typedef signed long long int off64_t;
typedef ssize_t cookie_read_function_t (void *, char *, size_t);
typedef ssize_t cookie_write_function_t (void *, const char *, size_t);
typedef int cookie_seek_function_t (void *, off64_t, int);
typedef int cookie_close_function_t (void *);
typedef struct {
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t funcs);
#endif
