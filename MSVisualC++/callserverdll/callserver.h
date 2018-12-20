#define SERVER_RUN                  9
#define LISTEN_PORT                 1
#define SHOW_MESSAGES_IN_LOG        2
#define RECORD_MESSAGES_IN_LOG_FILE 3
#define GET_PERIOD                  4
#define DEFAULT_BUFFER              4096
#define MAX_CLIENT_CONNECTIONS      8

struct sockaddr_in local;
typedef struct MESSAGE{
    SOCKET sock;
    char msg[32];
    char dat[64];
    int Priority;
} MESSAGE;

BOOL viewlog = TRUE;
BOOL recordlog = TRUE;
unsigned short iPort;
WSADATA wsd;
SOCKET sListen;
HANDLE mtxQMsg = NULL;
HANDLE mtxQDat = NULL;
HANDLE evntDat = NULL;
HANDLE evntMsg = NULL;
HANDLE mtxMass[4] = {0};
HEAP *Qmsg;
HEAP *Qdat;
HEAP *Qusr;
int datacount = 0;
//extern int verbose;
int verbose;
int CountThreads = 0;

extern void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param);

int _stdcall runserver(PVOID pvParam);
int _stdcall stopserver();
void exithread(char * stopstringmessage);
DWORD WINAPI ClientManagerThread(PVOID pvParam);
int DispatchMyMessage(SOCKET sock, 
                      char * stringmessage, 
                      int messagesize);
void sendMsgForAll(char * str);
void ErrorShow(char * text);
int ComparePriority(const void *Left,
                    int LTag,
                    const void *Right,
                    int RTag);
                    
int EqualClients(SOCKET s);                     
void ShowParam();
