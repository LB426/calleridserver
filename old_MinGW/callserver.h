#define LISTEN_PORT                 1
#define SHOW_MESSAGES_IN_LOG        2
#define RECORD_MESSAGES_IN_LOG_FILE 3
#define GET_PERIOD                  4
#define DEFAULT_BUFFER              4096
#define MAX_CLIENT_CONNECTIONS      8

BOOL viewlog = TRUE;
BOOL recordlog = TRUE;
unsigned short iPort;
WSADATA wsd;
SOCKET sListen;
struct sockaddr_in local;

typedef struct MESSAGE{
    SOCKET sock;
    char msg[32];
    char dat[32];
    int Priority;
} MESSAGE;

extern void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param);

DWORD WINAPI runserver(PVOID pvParam);
DWORD WINAPI ClientManagerThread(PVOID pvParam);
int DispatchMyMessage(SOCKET sock, char * stringmessage, int messagesize);
void sendMsgForAll(char * str);

