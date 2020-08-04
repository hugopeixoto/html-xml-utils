extern conststring http_strerror(int code);
extern FILE *fopenurl3(const conststring method, const conststring url,
         const conststring mode, const Dictionary request,
         Dictionary response, int maxredirs, int *status);
extern FILE *fopenurl2(const conststring url, const conststring mode,
         const Dictionary request, Dictionary response,
         int maxredirs, int *status);
extern FILE *fopenurl(const conststring path, const conststring mode,
        int *status);
