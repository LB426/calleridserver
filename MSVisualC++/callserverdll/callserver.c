#include <windowsx.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "heap.h"
#include "callserver.h"

int serverun = 0;

BOOL WINAPI DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
  return 1;
  UNREFERENCED_PARAMETER(hInst);
  UNREFERENCED_PARAMETER(ul_reason_being_called);
  UNREFERENCED_PARAMETER(lpReserved);
}

void runthread(LPTHREAD_START_ROUTINE StartAddress, void * param){
  HANDLE Thread;
  DWORD dwThreadID;   
  Thread = CreateThread(NULL, 0, StartAddress, (PVOID) param, 0, &dwThreadID);
  if(Thread!=NULL){
      CountThreads++;
  }else{
      ErrorShow("поток НЕ запущен\n");
  }
  //printf("потоков : %d\n",CountThreads);
  return;
}

void exithread(char * stopstringmessage){
  CountThreads--;
  printf("%s\n", stopstringmessage);
}

void ErrorShow(char * text)
{
    LPVOID lpMessageBuffer;
    int error = GetLastError();
    char * str;
    char buf[8];
    int len;
    FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                    (LPTSTR) &lpMessageBuffer,
                    0,
                    NULL);
    len = strlen(text) + strlen((char*)lpMessageBuffer) + 10;
    str = (char*)malloc(len * sizeof(char));
    if (str == NULL)
    {
        MessageBox(0,"Not enougth memory",0,MB_ICONERROR);
        return;
    }
    strcpy(str,text);
    strcat(str,": (");
    strcat(str,itoa(error,buf,10));
    strcat(str,") ");
    strcat(str,(char*)lpMessageBuffer);
    MessageBox(0,str,0,MB_ICONERROR);
    printf("%s\n",str);
    //toLog(str);
    //LogLineAdd(str);
    LocalFree( lpMessageBuffer );
}

////////////////////////////////////////////////////////////////////////////
//читает конфигурационный файл и выбирает из него нужные параметры
void GetFromINI(int code, void * parameter)
{
    FILE * fini;
    char buf[256];

    if ((fini = fopen("settings.ini", "r")) == NULL){
        ErrorShow("Error open file log");
        return;
    }

    switch (code){
    case LISTEN_PORT:
        while (!feof(fini)){
            fgets(buf,256,fini);
            if(strstr(buf,"server.port") != NULL){
         			char * pszToken = strtok(buf,"=");
          		pszToken = strtok(NULL,"\n");
          		*((int*)parameter) = atoi(pszToken);
          		break;
        		}
        }
        break;
    case MAX_CLIENT_CONNECTIONS:
      while(!feof(fini)){
        fgets(buf,256,fini);
        if(strstr(buf,"MAXIMAL CLIENTS CONNECTIONS PER TIME") != NULL){
          while (strstr(buf,"END") == NULL){
            fgets(buf,256,fini);
            if (strstr(buf,"END") != NULL) break;
            if (strstr(buf,"MAX_CLIENT_CONNECTIONS") != NULL){
              char * pszToken = strtok(buf,"\"");
              pszToken = strtok(NULL,"\"");
              *((int*)parameter) = atoi(pszToken);
              fclose(fini);
              return;
            }
          }
        }
      }
      break;
    case SERVER_RUN:
      while(!feof(fini)){
        fgets(buf,256,fini);
        if(strstr(buf,"server.run") != NULL){
          char * pszToken = strtok(buf,"=");
          pszToken = strtok(NULL,"\n");
          serverun = atoi(pszToken);
          break;
        }
      }
      break;
    }
    fclose(fini);
}
/////////////////////////////////////////////////////////////////////////////////////////
int delsocketfromheap(HEAP * h, SOCKET s){
  size_t j;
  HEAP_ELEMENT e;
  int err = 0;
  DWORD wres;
  
  //printf("удаляю сокет: %d\n", s);
  wres = WaitForSingleObject(mtxQMsg,INFINITE);
  if(wres == WAIT_OBJECT_0){ 
    for(j=1; j<= HeapGetSize(h); j++){
        MESSAGE * m;
        SOCKET sock;
        e = h->Heap[j-1];
        m = (MESSAGE*)e.Object;
        sock = (SOCKET)m->sock;
        if(sock == s){
          MESSAGE msg;
          m->Priority = 0;
          err = HeapVerify(h, ComparePriority, stderr);
          HeapDelete(h, NULL, NULL, &msg, ComparePriority);
        }
    }
  }
  ReleaseMutex(mtxQMsg);
  
  return err;
}
///////////////////////////////////////////////////////////////////////////////
//Серверный поток - принимает всё что шлет ему клиент или определиель и 
//передает на обработку в функцию DispatchMyMessage
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ServerThread(PVOID pvParam)
{
    SOCKET sock;
    //char szBuffer[DEFAULT_BUFFER];
    int ret, err;
    err = 0;
    
    printf("Server thread started\n");
    sock = (SOCKET)pvParam;
    while (1){
        char szBuffer[DEFAULT_BUFFER] = {0};
        ret = recv(sock, szBuffer, DEFAULT_BUFFER, 0);
        if (ret == SOCKET_ERROR){
            ErrorShow("recv() failed.");
            break;
        }else{
            if (ret == 0){
                /* ErrorShow("recv() failed, ret=0"); */
                err = ret;
                break;
            }
        }
        szBuffer[ret] = 0;
        
//возможно DispatchMyMessage нужно встроить сюда целиком!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //printf("->%s, %d, sock=%d\n",szBuffer, ret, sock);
        if(DispatchMyMessage(sock, szBuffer, ret) == 1000){
            ZeroMemory(szBuffer,DEFAULT_BUFFER);
            err = 1000;
            break;
        }

        ZeroMemory(szBuffer,DEFAULT_BUFFER);
    }

    delsocketfromheap(Qusr, sock);
    delsocketfromheap(Qmsg, sock);
    shutdown(sock,SD_BOTH);
    closesocket(sock);
    
    if(err == 0) printf("Соединение с клиентом потеряно, recv() вернул 0.\n");
    if(err == 1000) printf("Соединение разорвано клиентом, прислан END.\n");
    exithread("callserver.dll, Server thread ended");
    return (0);
}
/////////////////////////////////////////////////////////////////////////////////////////
//принимает соединения клиентов
//как только клиент открыл соединение запускает для него поток ServerThread
/////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI AcceptThread(PVOID pvParam){
    int iAddrSize;
    SOCKET sClient;
    struct sockaddr_in clientAddr;

    printf("Accept thread started\n");

    if (sListen != 0){
        iAddrSize = sizeof(clientAddr);
        if ((sClient = accept(sListen, (struct sockaddr*)&clientAddr, &iAddrSize)) == INVALID_SOCKET){
            ErrorShow("accept() failed");
        }else{
            runthread(ServerThread, (void *)sClient);
            runthread(AcceptThread, NULL);
        }
    }else{
        printf("sListen = NULL\n");
    }

    exithread("callserver.dll, Accept thread ended");
    return (0);
}
/////////////////////////////////////////////////////////////////////////////////////////
//запускающая функция
//запускает поток принимающий новые соединения и
//управляющий доступом к очереди данных - Менеджер клиентов
//доступ к очереди данных  - в зависимости от приоритета клиента
int _stdcall runserver(PVOID pvParam){
    size_t NumClients = 0; 
    
    GetFromINI(SERVER_RUN, &serverun);
    if(serverun == 0){
      printf("режим сервера отключен.\n");
      return 1;
    }else{
      printf("режим сервера включен.\n");
    }
    printf("Выполняю инициализацию . . .\n");
    
    mtxQMsg = CreateMutex(NULL,FALSE,NULL);
    if(NULL == mtxQMsg){
        ErrorShow("Мьютекс очереди сообщений НЕ создан\n");
    }
    mtxQDat = CreateMutex(NULL,FALSE,NULL);
    if(NULL == mtxQDat){
        ErrorShow("Мьютекс очереди данных НЕ создан\n");
    }
    evntMsg = CreateEvent(NULL, TRUE, FALSE, NULL);
    evntDat = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    mtxMass[0] = mtxQMsg;
    mtxMass[1] = mtxQDat;
    mtxMass[2] = evntMsg;
    mtxMass[3] = evntDat;
    
    GetFromINI(MAX_CLIENT_CONNECTIONS, &NumClients);
    if(0 == NumClients){
      ErrorShow("MAX_CLIENT_CONNECTIONS not defined!\n");
    }else{
      Qmsg = Heap_Create(NumClients);
      Qdat = Heap_Create(NumClients);
      Qusr = Heap_Create(NumClients);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0){
        ErrorShow("WSAStartup() failed\n");
        return -2;
    }
    GetFromINI(LISTEN_PORT, &iPort);

    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_family = AF_INET;
    local.sin_port = htons(iPort);
    sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sListen == SOCKET_ERROR){
        ErrorShow("socket() failed\n");
        return -3;
    }
    if (bind(sListen, (struct sockaddr*)&local,sizeof(local)) == SOCKET_ERROR){
        ErrorShow("bind() failed\n");
        return -4;
    }
    if (listen(sListen, MAX_CLIENT_CONNECTIONS) == SOCKET_ERROR){
        ErrorShow("listen() failed\n");
        return -5;
    }
    runthread(AcceptThread,NULL);
    //запускаем менеджера клиентов - определяет доступ клиентов к очереди данных и обрабатывает служебные сообщения
    runthread(ClientManagerThread,NULL);
    
    printf("Инициализация завершена.\n");
    
    return (0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//Сравнивает приоритеты принимаемых клиентов
int ComparePriority(const void *Left,
                    int LTag,
                    const void *Right,
                    int RTag)
{
  const MESSAGE *L = (MESSAGE *)Left;
  const MESSAGE *R = (MESSAGE *)Right;

  int diff = 0;

  if(L->Priority > R->Priority)
  {
    diff = 1;
  }
  else if(L->Priority < R->Priority)
  {
    diff = -1;
  }

  return diff;
}
//////////////////////////////////////////////////////////////////////////////////////////
//Все сообщения имеют формат - приоритет+ключевое слово+что тоеще если надо но не более 31симв+0 - конец соощения(строки)
//Обрабатывает сообщения от клиентов и определителей, но активно взаимодействует только с
//клиентами, т.е. BEGIN, END, READY, REFRESH, CMDREFRESH, CMDAREYOUREADY - касаются только клиентов.
//Формат сообщения приритет+сообщение, приритет - 1 байт.
//Самое первое сообщение - BEGIN, т.е. приритет+BEGIN, без данных, все клиенты приславшие BEGIN 
//ставятся в очередь Qusr и в ответ на него !!!!!!всем шлется команда CMDAREYOUREADY.
//Если клиент прислал READY значит он готов принимать данные и он ставиться в 
//очередь сообщений Qmsg - т.е. в очередь клиентов готовых принимать данные.
//Если сообщение начинается с приоритета и DATA значит это определитель прислал номер, 
//такое сообщение ставиться в очередь данных.
//////////////////////////////////////////////////////////////////////////////////////////
int DispatchMyMessage(SOCKET sock, char * stringmessage, int messagesize){
    MESSAGE msg = {0};
    if(strlen(stringmessage+sizeof(char)) > 31){
        printf("Buffer may be overflow\n");
        return -1;
    }
    if(strcmp(stringmessage+sizeof(char),"REFRESH") == 0){
        sendMsgForAll("CMDREFRESH\n");
        return 0;
    }
    if(strcmp(stringmessage+sizeof(char),"CHDISP") == 0){
        sendMsgForAll("CMDCHDISP\n");
        return 0;
    }
    if(strcmp(stringmessage+sizeof(char),"BEGIN") == 0){
        msg.sock = sock;
        memcpy((void*)(msg.msg),(void*)(stringmessage+sizeof(char)),messagesize-sizeof(char));
        msg.Priority = *stringmessage;
        HeapInsert(Qusr, 0, sizeof(MESSAGE), (void*)(&msg), ComparePriority);
        //sendMsgForAll("CMDAREYOUREADY");
        return 0;
    }
    if(strcmp(stringmessage+sizeof(char),"END") == 0){
        int ret;
        ret = send(sock, "END", 3, 0);
        /*if (ret == SOCKET_ERROR){
            ErrorShow("send() for client failed,");
        }else{
            if(ret == 0){
                ErrorShow("ClientManagerThread ret = 0\n"); 
            }
        }*/
        return 1000;
    }
    //если клиент готов обработать данные то ставим его в очередь сообщений
    //и сигналим что есть свободные клиенты - ЭТО НЕВАЖНО, если нет ДАННЫХ!!!, значит сигналить необязательно!!!
    if(strcmp(stringmessage+sizeof(char),"READY") == 0){
        DWORD wres = WaitForSingleObject(mtxQMsg,INFINITE);
        if(wres == WAIT_OBJECT_0){ 
          if(NULL != Qmsg){
            wres = EqualClients(sock);
            if(0 == wres){
              msg.sock = sock;
              memcpy((void*)(msg.msg),(void*)(stringmessage+sizeof(char)),messagesize-sizeof(char));
              msg.Priority = *stringmessage;
              HeapInsert(Qmsg, 0, sizeof(MESSAGE), (void*)(&msg), ComparePriority);       
              //printf("msg=%s, equal res=%d, prio:%d\n",msg->msg,wres,msg->Priority);
            }
          }
        }
        //SetEvent(evntDat);
        ReleaseMutex(mtxQMsg);
        
        return wres;
    }
    if(strcmp(stringmessage+sizeof(char),"NOTREADY") == 0){
        DWORD wres = WaitForSingleObject(mtxQMsg,INFINITE);
        //printf("->NOTREADY\n");
        if(wres == WAIT_OBJECT_0){ 
          if(NULL != Qmsg){
            delsocketfromheap(Qmsg, sock);
          }
        }
        ReleaseMutex(mtxQMsg);
        return wres;
    }
       
    //если пришли данные от определителей номера то ставим их в очередь данных
    //и сигналим что есть данные для обработки
    if(strstr(stringmessage+sizeof(char),"DATA") != NULL){
        DWORD wres;
        int datalen = 0;

        datalen = strlen(stringmessage+sizeof(char)+4);
        msg.sock = sock;
        memcpy((void*)msg.msg,(void*)(stringmessage+sizeof(char)),4);
        memcpy((void*)msg.dat,(void*)(stringmessage+5*sizeof(char)),datalen);
        msg.Priority = *stringmessage;
        
        wres = WaitForSingleObject(mtxQDat,INFINITE);
        if(wres == WAIT_OBJECT_0){ 
          if(NULL != Qdat){
            HeapInsert(Qdat, 0, sizeof(MESSAGE), (void*)(&msg), ComparePriority);       
            ++datacount;
            printf("данные №%d\t\t\"%s\"\n",datacount,msg.dat);
          }
        }
        SetEvent(evntDat);
        ReleaseMutex(mtxQDat);

        return 0;
    }
    if(strstr(stringmessage+sizeof(char),"STATE") != NULL){
      ShowParam();
      return 0;
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
void sendMsgForAll(char * str){
    int ret;
    size_t j;
    char szCmdBuffer[256] = {0};
    
    if(strlen(str)>255) exit(1);
    strcpy(szCmdBuffer,str);
    for(j=0; j < HeapGetSize(Qusr); j++){
        HEAP_ELEMENT e;
        MESSAGE * m;
        SOCKET sock;
        e = Qusr->Heap[j];
        m = (MESSAGE*)e.Object;
        sock = m->sock;
        ret = send(sock, szCmdBuffer, strlen(szCmdBuffer)+1, 0);
        if (ret == SOCKET_ERROR){
            ErrorShow("msgforall, send() for client failed,");
        }else{
            if(ret == 0){
                ErrorShow("ClientManagerThread ret = 0\n"); 
            }
        }
    }
    return;
}
///////////////////////////////////////////////////////////////////////////////
//Проверяет есть ли сообщение этого клиента в очереди сообщений о готовности принять данные
//необходима для избежания повторений в очереди сообщений.
//Чтобы в очереди сообщений один и тот же клиент не повторялся, т.е. был всего один раз
int EqualClients(SOCKET s){
  int res = 0;
  size_t j;
  for(j=0; j< Qmsg->Count; j++){
      HEAP_ELEMENT e;
      MESSAGE * m;
      SOCKET s1;
      e = Qmsg->Heap[j];
      m = (MESSAGE*)e.Object;
      s1 = m->sock;
      if(s1 == s) res = 1;    
  }
  return res;
}
///////////////////////////////////////////////////////////////////////////////
//Обеспечивает выдачу данных готовым их принять клиентам
DWORD WINAPI ClientManagerThread(PVOID pvParam){
    int ret;
    MESSAGE msg = {0};
    MESSAGE dat = {0};
    DWORD wres;
    int successM = 0;
    int successD = 0;

    while (serverun != 2000){
        successM = 0;
        successD = 0;
        WaitForSingleObject(evntDat, 500); //INFINITE);
        ResetEvent(evntDat);
        
        //проверяем очередь сообщений и очередь данных на наличие чего-то каждые 200мс
        wres = WaitForSingleObject(mtxQMsg,200);
        if(wres == WAIT_OBJECT_0){
          if(Qmsg->Count != 0){
            successM = 1;
          }else{
            successM = -1;
            //printf("->CMDAREYOUREADY\n");
            sendMsgForAll("CMDAREYOUREADY");
          }
        }
        ReleaseMutex(mtxQMsg);
        
        wres = WaitForSingleObject(mtxQDat,200);
        if(wres == WAIT_OBJECT_0){
          if(Qdat->Count != 0){
            successD = 1;
          }else{
            successD = -1;
          }
        }
        ReleaseMutex(mtxQDat);
        
        if((1 == successD) && (1 == successM)){
          char szMessage[64] = {'D','A','T',0};
          
          wres = WaitForSingleObject(mtxQMsg,INFINITE);
          if(wres == WAIT_OBJECT_0) HeapDelete(Qmsg, NULL, NULL, &msg, ComparePriority);
          ReleaseMutex(mtxQMsg);
          
          wres = WaitForSingleObject(mtxQDat,INFINITE);
          if(wres == WAIT_OBJECT_0) HeapDelete(Qdat, NULL, NULL, &dat, ComparePriority);
          ReleaseMutex(mtxQDat);
          
          strcat(szMessage,dat.dat);
          printf("dat.dat=%s,szMessage = %s\n",dat.dat,szMessage);
          ret = send(msg.sock, szMessage, strlen(szMessage)+1, 0);
          if (ret == SOCKET_ERROR){
              ErrorShow("client manager, send() for client failed,");
          }else{
              if(ret == 0) printf("ClientManagerThread ret = 0\n"); 
          }

          szMessage[0] = 0;
        }
    }
    exithread("callserver.dll, остановлен менеджер клиентов\n");
    return (0);
}
///////////////////////////////////////////////////////////////////////////////
int PrintMessage( const void *Object,
                  int Tag,
                  size_t Size,
                  FILE *fp)
{
  const MESSAGE *msg= (MESSAGE*)Object;
  fprintf(fp, "\n===============================\n");
  fprintf(fp, "SOCK %d\t", msg->sock);
  fprintf(fp, "Priority: %d\t", msg->Priority);
  fprintf(fp, "Message: %s\t", msg->msg);
  fprintf(fp, "Data: %s", msg->dat);
  fprintf(fp, "\n===============================\n");
  
  return 0;
}

void ShowParam(){
  char params[128] = {0};
  sprintf(params,"STATE;ёююс∙хэшщ:%d,фрээ√ї т юўхЁхфш: %d,ъышхэЄют:%d | яюЄюъют:%d",
          HeapGetSize(Qmsg),
          HeapGetSize(Qdat),
          HeapGetSize(Qusr),
          CountThreads);
  //printf("%s\n",params);        
  sendMsgForAll(params);        
}

void ShowQmsg(){
  HeapPrint(Qmsg, PrintMessage);
}

void ShowQusr(){
  HeapPrint(Qusr, PrintMessage);
}

void ShowQdat(){
  HeapPrint(Qdat, PrintMessage);
}

void hVerify(){
  HeapVerify(Qmsg, ComparePriority, stderr);
}

int _stdcall stopserver(){
  serverun = 2000;
  SetEvent(evntDat);
  return CountThreads;
}