#include <windowsx.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

//typedef void (_stdcall DISPATCHSERVERMESSAGECALLBACK) (char * str);
//DISPATCHSERVERMESSAGECALLBACK * pDispatchServerMessageCallback;
extern int isdnfinderstate = 0;

char prio = '0';
SOCKET connid = 0;
int end = 0;
int clientrun = -1;

BOOL WINAPI DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
  return 1;
  UNREFERENCED_PARAMETER(hInst);
  UNREFERENCED_PARAMETER(ul_reason_being_called);
  UNREFERENCED_PARAMETER(lpReserved);
}

unsigned long _stdcall clientloop(void * param)
{
  SOCKET sock = (SOCKET)param;
  sendmsgtoserver(sock, prio, "BEGIN", NULL);
  while (end!=1){
    char buf[2048] = {0};
    int result = 0;
    //if(SOCKET_ERROR == recvipdata(sock, buf, 2047)) break;
    //printf("!!!!!!!!!!!!!!!!!!!!!!! %s\n",buf);
    //if(0 == strcmp(buf, "END")) break;
    //pDispatchServerMessageCallback(buf);
    //buf[0] = 0;
    result = recvipdata(sock, buf, 2047);
    buf[0] = 0;
    printf("result= %d, data = \"\", %d\n", result, buf, isdnfinderstate);
    if(result == SOCKET_ERROR){ 
    	printf("SOCKET_ERROR\n");
    	break;
    }
  }
  closeipconnection(sock);
  printf("клиентский поток, принимающий сообщения от сервера завершился.\n");
  return 0;
}

void _stdcall SendMyMsgToMySrv(char * cmd){
  if(clientrun == 0) return;
  if(end == 0) sendmsgtoserver(connid, prio, cmd, NULL);
  if(0 == strcmp(cmd, "END")) end = 1;
  return;
}

int _stdcall runclient(void * f1)
{
  SOCKET skt = {0};
  char host[16] = {0};
  unsigned short port = 0;
  FILE * fini;
  char buf[256] = {0};
  
  if ((fini = fopen("settings.ini", "r")) == NULL){
      ErrorShow("Error open file log");
      return -1;
  }
//инициализация параметров запуска
  while (!feof(fini)){
    fgets(buf,256,fini);
    if (strstr(buf,"client.run") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      clientrun = atoi(pszToken);
      if(clientrun == 0){
        printf("режим клиента отключен.\n");
        return 1;
      }else{
        printf("режиме клиента включен.\n");
      }
    }
    if (strstr(buf,"client.connect.host") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      strcpy(host, pszToken);
      printf("client connect host=%s\n",host);
    }
    if (strstr(buf,"client.connect.port") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      port = atoi(pszToken);  
      printf("client connect port=%d\n",port);
    }
    if (strstr(buf,"client.prioritet") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      prio = *pszToken; 
      printf("client prioritet=%d\n",port);
    }
  }
  fclose(fini);
  
/*  if(f1 != NULL){ 
    //инициализируем указатель на функцию котораяя обрабатывает передачу сервера
    pDispatchServerMessageCallback = f1;
  }else{
    ErrorShow("CallBack function not defined!!!");
    return -2;
  }*/ 
  skt = openipconnection(host, port);
  if(skt != 0){
    connid = skt;
    runthread(clientloop, (void*)skt);
  }else{
    ErrorShow("openipconnection() failed!");
    return -3;
  }
  return 0;
}

void sendmsgtoserver(SOCKET sock, char prio, char * cmd, char * data)
{
  char buf[128] = {0};
  buf[0] = prio;
  if(NULL == cmd) ErrorShow("cmd cannot be NULL");
  else strcat(buf,cmd);
  if(data != NULL) strcat(buf,data);
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
    ErrorShow("sendipdata client, send() for client failed");
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
    //MessageBox(0,str,0,MB_ICONERROR);
    fprintf(stderr,"%s\n",str);
    free( lpMessageBuffer );
    SetLastError(0);
    return error;
}
