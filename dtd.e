#define MAXNAMELEN  10
typedef struct _ElementType {
  string name;
  
 _Bool 
      mixed, empty, cdata, stag, etag, pre, break_before, break_after;
  string parents[76];
} ElementType;
extern const ElementType * lookup_element(
                                   );
