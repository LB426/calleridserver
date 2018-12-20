#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>

char prio = 'W'; 
char d[32] = {0};

int ErrorShow(char * text)
{
    LPVOID lpMessageBuffer;
    int error;
    int len;
		char * str;
    char buf[32];
    error = GetLastError();
    FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* The user default language */
                    (LPTSTR) &lpMessageBuffer,
                    0,
                    NULL);
    len = strlen(text) + strlen((char*)lpMessageBuffer) + 10;
    str = (char*)malloc(len * sizeof(char));
    if (str == NULL)
    {
        MessageBox(0,"Not enougth memory in func ErrorShow",0,MB_ICONERROR);
        fprintf(stderr,"Not enougth memory in func ErrorShow\n");
        return GetLastError();
    }
    strcpy(str,text);
    strcat(str,": (");
    sprintf(buf,"%d",error);
    strcat(str,buf);
    strcat(str,") ");
    strcat(str,(char*)lpMessageBuffer);
    MessageBox(0,str,0,MB_ICONERROR);
    fprintf(stderr,"%s\n",str);
    free( lpMessageBuffer );
    SetLastError(0);
    return error;
}

int myrand(int min, int max){
	int res;
	res = rand()%(max-min+1)+min ;
	return res;
}

void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param){
    HANDLE Thread;
    DWORD dwThreadID;   
    Thread = CreateThread(NULL, 0, StartAddress, (PVOID) param, 0, &dwThreadID);
    if(NULL == Thread) ErrorShow("поток НЕ запущен\n");
    
    return;
}

SOCKET openipconnection(char *host, int port){
	WSADATA wsd = {0}; 
  SOCKET sock = 0;
	struct sockaddr_in server;
	char ipadr[16] = {0};
	
	if(strstr(host,".")!=NULL){
		strcpy(ipadr,host);
	}else{
		ErrorShow("Вы ввели IP адрес неправильно");
		return 0;
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0){
		ErrorShow("WSAStartup() failed");
		return 0;
	}
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET){
			/* printf("socket() for client failed, : %d\n", WSAGetLastError()); */
			ErrorShow("socket() for client failed");
			return 0;
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ipadr);
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
			/* printf("connect() for client failed, : %d\n", WSAGetLastError()); */
			ErrorShow("connect() for client failed");
			return 0;
	}	
	return sock;
}

int closeipconnection(SOCKET sock){
	shutdown(sock, SD_BOTH);
	closesocket(sock);
	WSACleanup();
	return 0;
}

int sendipdata(SOCKET sock, char * buf, int buflen){
	int ret = 0;
	ret = send(sock, buf, buflen, 0);
	if(ret == SOCKET_ERROR){
		ErrorShow("send() for client failed");
	}else{
		if(ret == 0) printf("clientsend ret = 0\n"); 
	}
	return ret;
}

int recvipdata(SOCKET sock, char * buf, int buflen){
	int ret = 0;
	ret = recv(sock, buf, buflen-1, 0);
	if (ret == SOCKET_ERROR){
		ErrorShow("recv() failed.");
	}else{
		if (ret == 0){
				ErrorShow("recv() failed, ret=0");
		}
	}
	*(buf+ret) = 0 ;
	return ret;
}

void sendmsgtoserver(SOCKET sock, char prio, char * cmd, char * data){
	char buf[128] = {0};
	buf[0] = prio;
	if(NULL == cmd) ErrorShow("cmd cannot be NULL");
	else strcat(buf,cmd);
	if(data != NULL) strcat(buf,data);
	sendipdata(sock, buf, strlen(buf));
}

unsigned long __stdcall dispatchmessage(void * param){
	char buf[2048] = {0};
	SOCKET sock = (SOCKET)param;
	while (1){
		if(SOCKET_ERROR == recvipdata(sock, buf, 2047)) break;
		printf("%s\n",buf);
		if(strcmp(buf,"CMDAREYOUREADY") == 0){
			sendmsgtoserver(sock, prio, "READY", NULL);
		}
	/*	if(strncmp(buf,"DAT",3) == 0){
			sendmsgtoserver(sock, prio, "READY", NULL);
		} */
		buf[0] = 0;
  }  	
	return 0;
}

unsigned long __stdcall senddatatoserver(void * param){
	char buf[2048] = {0};
	SOCKET sock = (SOCKET)param;
	int rnd = 0;
	
	buf[0] = prio;
	strcat(buf,"DATA");
	while (1){
		strcat(buf,"+79610000000");
		rnd = myrand(1000,2000);
		sprintf(buf+13*sizeof(char),"%d",rnd);
		if(SOCKET_ERROR == sendipdata(sock, buf, 2047)) break;
		buf[5] = 0;
		Sleep(rnd);
  }  	
	return 0;
}

int main(int argc, char * argv[])
{
	char c, sw;
	char host[16] = {0};
	unsigned short port = 0;
	SOCKET skt = {0};

	if(argc==1){
			printf("программа запущена без параметров!!!\n");
			printf("Параметры идут в следующем порядке через пробел :\n");
			printf("IP адрес\nПорт\nПриоритет\n");        
			return (1);
	}
	if(argc>1){
		if(argv[1] != NULL) strcpy(host,argv[1]);		
		if(argv[2] != NULL) port = atoi(argv[2]);
		if(argv[3] != NULL) prio = *argv[3];
		if(argv[4] != NULL) sw = *argv[4];
		if(argv[5] != NULL) strcpy(d,argv[5]);
	}
/*	printf("%s %d %c %c\n",host,port,prio,sw); */
	skt = openipconnection(host, port);
	if(skt != 0){
		if(sw == 'c') sendmsgtoserver(skt, prio, "BEGIN", NULL);
		else runthread(senddatatoserver, (void*)skt);
		runthread(dispatchmessage, (void*)skt);
		while((c=getchar())!='e')
		{
			switch(c){
			case 'a':
				printf("A\n");
				break;
			case 'b':
				sendipdata(skt, "b", 1);
				break;
			case 'r':
				sendmsgtoserver(skt, prio, "READY", NULL);
				break;
			case 'e':
				break;
			}
		}
	}else{
		ErrorShow("openipconnection() failed!");
	}
	closeipconnection(skt);
	
	return 0;
}
