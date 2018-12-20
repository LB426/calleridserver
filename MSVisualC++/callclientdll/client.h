int _stdcall runclient(void * f1);
void _stdcall SendMyMsgToMySrv(char * cmd);
SOCKET openipconnection(char *host, int port);
int closeipconnection(SOCKET sock);
void sendmsgtoserver(SOCKET sock, char prio, char * cmd, char * data);
int sendipdata(SOCKET sock, char * buf, int buflen);
int recvipdata(SOCKET sock, char * buf, int buflen);
void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param);
int ErrorShow(char * text);