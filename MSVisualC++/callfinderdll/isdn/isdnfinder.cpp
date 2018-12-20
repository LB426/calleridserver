#include <windowsx.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CapiFunc.h"
#include "isdnfinder.h"

extern int isdnfinderstate = 0;

CCapiFunc var;
char prio = '0';
int endcapi = 0;

BOOL WINAPI DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
	return 1;
	UNREFERENCED_PARAMETER(hInst);
	UNREFERENCED_PARAMETER(ul_reason_being_called);
	UNREFERENCED_PARAMETER(lpReserved);
}

int closeisdnobj()
{
 	var.Close();
 	endcapi = 1 ;
 	//var.FreeCapi32();
	return 0;
}

int _stdcall stopisdnterminalfinder()
{
	closeisdnobj();
	return 0;
}

int _stdcall runisdnterminalfinder()
{
	SOCKET skt = {0};
	char host[16] = {0};
	unsigned short port = 0;
	char prioritet = '1';
	int amount = 0;
	int i = 0;
	FILE * fini;
	char buf[256] = {0};
	
	prio = prioritet;
	
	isdnfinderstate = 100;
	
	if ((fini = fopen("settings.ini", "r")) == NULL){
			ErrorShow("Error open file log");
			return -1;
	}
//инициализация параметров запуска потока ISDN терминала
	while (!feof(fini)){
		fgets(buf,256,fini);
		if (strstr(buf,"isdn.bri.terminal.amount") != NULL){
			char * pszToken = strtok(buf,"=");
			pszToken = strtok(NULL,"\n");
			amount = atoi(pszToken);
			if(amount == 0){
				return 1;
			}else{
				printf("isdn amount=%d\n",amount);
			}
		}
		if (strstr(buf,"isdn.bri.terminal.server.host") != NULL){
			char * pszToken = strtok(buf,"=");
			pszToken = strtok(NULL,"\n");
			strcpy(host, pszToken);
			printf("isdn terminal host=%s\n",host);
		}
		if (strstr(buf,"isdn.bri.terminal.server.port") != NULL){
			char * pszToken = strtok(buf,"=");
			pszToken = strtok(NULL,"\n");
			port = atoi(pszToken);	
			printf("isdn terminal port=%d\n",port);
		}
	}
	fclose(fini);	
	
	if((amount == 0)||(host[0] == 0)||(port == 0)){
		return 0;
	}
	skt = openipconnection(host, port);
	if(skt != 0){
		runthread(ISDN_TerminalThread, (void*)skt);
	}else{
		ErrorShow("openipconnection() failed!");
		return -3;
	}
	
	return 0;
}

unsigned long _stdcall ISDN_TerminalThread(void * param)
{
	SOCKET sock = (SOCKET)param;
	int error;
	
	printf("ISDN_Terminal thread started\n");
	if ( (error = var.Init()) != 0 ) 
	{
		ErrorShow("Capi not Initialized\n");
		printf("ISDN_Terminal thread ended\n");
		return (error);
	}
	var.Reject = 1;
	var.m_pThisObject = &var;
	var.SetModeCapi(1);
	//do
	while(!endcapi)
	{ 
		ZeroMemory((PVOID)var.CallingPartyNomber, 64);
		ZeroMemory((PVOID)var.CalledPartyNomber, 64);
		var.Handle_CAPI_Msg();
		if (*var.CallingPartyNomber != 0)
		{
			/////////////
			char Nombers[256] = {0};
			/*if ( (Nombers = (char*)HeapAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, strlen(var.CallingPartyNomber) +
				strlen(var.CalledPartyNomber) + 2)) == NULL)
			{
				ErrorShow("not enougth space for buf pPointer\n");
				return(1);
			}
			*Nombers = 0;
			*/
			strcpy(Nombers, ANomberAnalise(var.CallingPartyNomber));
			strcat(Nombers, ";");
			strcat(Nombers, var.CalledPartyNomber);
			sendmsgtoserver(sock, prio, "DATA", Nombers);
			printf("%s\n",Nombers);
			
/*			if (!HeapFree(GetProcessHeap(), 0, (PVOID)var.CallingPartyNomber))
				printf("HeapFree() in Client NOT successed.\n");
			if (!HeapFree(GetProcessHeap(), 0, (PVOID)var.CalledPartyNomber))
				printf("HeapFree() in Client NOT successed.\n");

			/////////////
			var.CallingPartyNomber = NULL;
			var.CalledPartyNomber = NULL;
*/
			ZeroMemory((PVOID)Nombers, 256);	
		}
		ZeroMemory((PVOID)var.CallingPartyNomber, 64);
		ZeroMemory((PVOID)var.CalledPartyNomber, 64);
	}
	//while (1);
	
	var.Close();
	var.FreeCapi32();
	
	closeipconnection(sock);
	printf("ISDN_Terminal thread ended");

	return (0);
}


char * ANomberAnalise(char * anomber)
{
	switch (strlen(anomber))
	{
	case 10:
		{
			if ((*anomber == '8') &&
				(*(anomber+1) == '6') &&
				(*(anomber+2) == '1') &&
				(*(anomber+3) == '9') &&
				(*(anomber+4) == '6')) return anomber+5;
			//if (*anomber == '9') return anomber+3; 
			return anomber;
		}
	case 5:
		{
			return anomber;
		}
	case 7:
		{
			if ((*anomber == '9') && (*(anomber+1) == '6')) return anomber + 2;

			return anomber;
		}
	case 12:
		{
			if((*anomber=='+') && (*(anomber+1)=='7')) return anomber+2;
			return anomber;
		}
	default :
		{
			return anomber;
		}
	}
}

void sendmsgtoserver(SOCKET sock, char prio, char * cmd, char * data)
{
	char buf[128] = {0};
	buf[0] = prio;
	if(NULL == cmd){
		ErrorShow("cmd cannot be NULL");
		return;
	}
	strcat(buf,cmd);	
	if(NULL == data){
		ErrorShow("data cannot be NULL");
		return;
	}
	if(64 < strlen(data)){
		ErrorShow("data cannot be more then 64 byts");
		return;
	}
	strcat(buf,data);
	sendipdata(sock, buf, strlen(buf));
}

SOCKET openipconnection(char *host, int port)
{
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

int closeipconnection(SOCKET sock)
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
	WSACleanup();
	return 0;
}

int sendipdata(SOCKET sock, char * buf, int buflen)
{
	int ret = 0;
	ret = send(sock, buf, buflen, 0);
	if(ret == SOCKET_ERROR){
		ErrorShow("send() for client failed");
	}else{
		if(ret == 0) printf("clientsend ret = 0\n"); 
	}
	return ret;
}

int recvipdata(SOCKET sock, char * buf, int buflen)
{
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

void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param)
{
    HANDLE Thread;
    DWORD dwThreadID;   
    Thread = CreateThread(NULL, 0, StartAddress, (PVOID) param, 0, &dwThreadID);
    if(NULL == Thread) ErrorShow("поток НЕ запущен\n");   
    return;
}

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
