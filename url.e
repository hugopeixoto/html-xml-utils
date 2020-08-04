typedef struct {
  string full;
  string proto;
  string user;
  string password;
  string machine;
  string port;
  string path;
  string query;
  string fragment;
} *URL;
extern void URL_dispose(URL url);
extern URL URL_new(const conststring url);
extern URL URL_absolutize(const URL base, const URL url);
extern string URL_s_absolutize(const conststring base, const conststring url);
extern URL URL_to_ascii(const URL iri);
extern string URL_s_to_ascii(const conststring iri);
