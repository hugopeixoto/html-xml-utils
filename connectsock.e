extern u_short portbase ;
extern int connectsock(const char *host, const char *service, char *protocol);
extern int connectTCP(const char *host, const char *service);
extern int connectUDP(char *host, char *service);
extern int passivesock(char *service, char *protocol, int qlen);
extern int passiveTCP(char *service, int qlen);
extern int passiveUDP(char *service);
