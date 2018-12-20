typedef struct _sioterminalparam {
  SOCKET s;
  char sioportname[8];
  char modeltelephname[64];
  char comment[32];
  unsigned long   watchperiod;
} SIOTERMINALPARAM;

unsigned long _stdcall GENERIC_SIO_Terminal_loop(void * param);
unsigned long _stdcall SERI_T630_SIO_Terminal_loop(void * param);
unsigned long _stdcall NOKIA_SIO_Terminal_loop(void * param);
int _stdcall runserialterminalfinder();
int _stdcall stopserialterminalfinder();
SOCKET openipconnection(char *host, int port);
int closeipconnection(SOCKET sock);
void sendmsgtoserver(SOCKET sock, char prio, char * cmd, char * data);
int sendipdata(SOCKET sock, char * buf, int buflen);
int recvipdata(SOCKET sock, char * buf, int buflen);
void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param);
int ErrorShow(char * text);
void printbuf(char * buf, int len);
char * anbrAnalise(char * anomber);
char * anbrGet(char * string);
char * anbrGetAcorp(char * string);
void WriteToLog(char * line);
void logger(SIOTERMINALPARAM * pChParam, const char *fmt, ...);