#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "main.h"
#ifdef __cplusplus
extern "C"
{
#include "heap.h"
}
#endif
#include "callserver.h"

HANDLE mtxQMsg = NULL;
HANDLE mtxQDat = NULL;
HANDLE evntDat = NULL;
HANDLE evntMsg = NULL;
HANDLE mtxMass[4] = {0};
HEAP *Qmsg;
HEAP *Qdat;
HEAP *Qusr;
int datacount = 0;
extern int verbose;

int ComparePriority(const void *Left,
                 		int LTag,
                 		const void *Right,
                 		int RTag);
                 		
int EqualClients(SOCKET s);                 		

////////////////////////////////////////////////////////////////////////////
//�⠥� ���䨣��樮��� 䠩� � �롨ࠥ� �� ���� �㦭� ��ࠬ����
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
            if (strstr(buf,"SERVER") != NULL){
                while (strstr(buf,"END") == NULL){
                    fgets(buf,256,fini);
                    if (strstr(buf,"END") != NULL) break;
                    if (strstr(buf,"PORT") != NULL){
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
    case SHOW_MESSAGES_IN_LOG:
        while (!feof(fini)){
            fgets(buf,256,fini);
            if (strstr(buf,"SHOW MESSAGES IN LOG") != NULL){
                while (strstr(buf,"END") == NULL){
                    fgets(buf,256,fini);
                    if (strstr(buf,"END") != NULL) break;
                    if (strstr(buf,"YES") != NULL){
                        viewlog = TRUE;
                        fclose(fini);
                        return;
                    }else{
                        viewlog = FALSE;
                        fclose(fini);
                        return;
                    }
                }
            }           
        }
        break;
    case RECORD_MESSAGES_IN_LOG_FILE:
        while (!feof(fini)){
            fgets(buf,256,fini);
            if (strstr(buf,"RECORD MESSAGES IN LOG FILE") != NULL){
                while (strstr(buf,"END") == NULL){
                    fgets(buf,256,fini);
                    if (strstr(buf,"END") != NULL) break;
                    if (strstr(buf,"YES") != NULL){
                        recordlog = TRUE;
                        fclose(fini);
                        return;
                    }else{
                        recordlog = FALSE;
                        fclose(fini);
                        return;
                    }
                }
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
    }
    fclose(fini);
}
/////////////////////////////////////////////////////////////////////////////////////////
int delsocketfromheap(HEAP * h, SOCKET s){
	size_t j;
	HEAP_ELEMENT e;
	int err = 0;
	
	printf("㤠��� ᮪��: %d\n", s);
	DWORD wres = WaitForSingleObject(mtxQMsg,INFINITE);
	if(wres == WAIT_OBJECT_0){ 
		for(j=1; j<= HeapGetSize(h); j++){
				e = h->Heap[j-1];
				MESSAGE *m = (MESSAGE*)e.Object;
				SOCKET sock = (SOCKET)m->sock;
				if(sock == s){
					m->Priority = 0;
					err = HeapVerify(h, ComparePriority, stderr);
					MESSAGE msg;
					HeapDelete(h, NULL, NULL, &msg, ComparePriority);
				}
		}
	}
	ReleaseMutex(mtxQMsg);
	
	return err;
}
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ServerThread(PVOID pvParam)
{
    toLog("Server thread started");

    SOCKET sock = (SOCKET)pvParam;
    char szBuffer[DEFAULT_BUFFER];
    int ret, err;
		err = 0;
    while (1){
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
        
//�������� DispatchMyMessage �㦭� ���ந�� � 楫����!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if(verbose == 1) printf("->%s, %d, sock=%d\n",szBuffer, ret, sock);
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
    
		if(err == 0) printf("���������� � �����⮬ ����ﭮ, recv() ���� 0.\n");
		if(err == 1000) printf("���������� ࠧ�ࢠ�� �����⮬, ��᫠� END.\n");
    
    toLog("Server thread ended");
    return (0);
}
/////////////////////////////////////////////////////////////////////////////////////////
//�ਭ����� ᮥ������� �����⮢
DWORD WINAPI AcceptThread(PVOID pvParam){
    int iAddrSize;
    SOCKET sClient;
    struct sockaddr_in clientAddr;

    toLog("Accept thread started");

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

    //exithread();
    toLog("Accept thread ended");
    return (0);
}
/////////////////////////////////////////////////////////////////////////////////////////
//����᪠��� �㭪��
//����᪠�� ��⮪ �ਭ����騩 ���� ᮥ������� �
//�ࠢ���騩 ����㯮� � ��।� ������ - �������� �����⮢
//����� � ��।� ������  - � ����ᨬ���� �� �ਮ��� ������
DWORD WINAPI runserver(PVOID pvParam){
		printf("�믮���� ���樠������ . . .\n");
		
    mtxQMsg = CreateMutex(NULL,FALSE,NULL);
    if(NULL == mtxQMsg){
        ErrorShow("���⥪� ��।� ᮮ�饭�� �� ᮧ���\n");
    }
    mtxQDat = CreateMutex(NULL,FALSE,NULL);
    if(NULL == mtxQDat){
        ErrorShow("���⥪� ��।� ������ �� ᮧ���\n");
    }
    evntMsg = CreateEvent(NULL, TRUE, FALSE, NULL);
    evntDat = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    mtxMass[0] = mtxQMsg;
    mtxMass[1] = mtxQDat;
    mtxMass[2] = evntMsg;
    mtxMass[3] = evntDat;
    
    size_t NumClients = 0; 
    GetFromINI(MAX_CLIENT_CONNECTIONS, &NumClients);
		if(0 == NumClients){
			ErrorShow("MAX_CLIENT_CONNECTIONS not defined!");
		}else{
			Qmsg = Heap_Create(NumClients);
			Qdat = Heap_Create(NumClients);
			Qusr = Heap_Create(NumClients);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0){
        ErrorShow("WSAStartup() failed");
        return -2;
    }
    GetFromINI(LISTEN_PORT, &iPort);

    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_family = AF_INET;
    local.sin_port = htons(iPort);
    sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sListen == SOCKET_ERROR){
        ErrorShow("socket() failed");
        return -3;
    }
    if (bind(sListen, (struct sockaddr*)&local,sizeof(local)) == SOCKET_ERROR){
        ErrorShow("bind() failed");
        return -4;
    }
    if (listen(sListen, MAX_CLIENT_CONNECTIONS) == SOCKET_ERROR){
        ErrorShow("listen() failed");
        return -5;
    }
    runthread(AcceptThread,NULL);
    //����᪠�� �������� �����⮢ - ��।���� ����� �����⮢ � ��।� ������ � ��ࠡ��뢠�� �㦥��� ᮮ�饭��
    runthread(ClientManagerThread,NULL);
    
    printf("���樠������ �����襭�.\n");
    
    return (0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//�ࠢ������ �ਮ���� �ਭ������� �����⮢
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
////////////////////////////////////////////////////////////////////////////////////////////////////////
//��ࠡ��뢠�� ᮮ�饭�� �� �����⮢
//�᫨ ������ ��᫠� READY ����� �� ��⮢ �ਭ����� ����� � �� �⠢����� � 
//��।� ᮮ�饭�� - �.�. � ��।� �����⮢ ��⮢�� �ਭ����� �����
int DispatchMyMessage(SOCKET sock, char * stringmessage, int messagesize){
		MESSAGE msg = {0};
    if(strlen(stringmessage+sizeof(char)) > 31){
        ErrorShow("Buffer may be overflow\n");
        exit(1);
    }
    if(strcmp(stringmessage+sizeof(char),"REFRESH") == 0){
        sendMsgForAll("CMDREFRESH\n");
        return 0;
    }
    if(strcmp(stringmessage+sizeof(char),"BEGIN") == 0){
        msg.sock = sock;
        memcpy((void*)(msg.msg),(void*)(stringmessage+sizeof(char)),messagesize-sizeof(char));
        msg.Priority = *stringmessage;
    		HeapInsert(Qusr, 0, sizeof(MESSAGE), (void*)(&msg), ComparePriority);
        sendMsgForAll("CMDAREYOUREADY");
        return 0;
    }
    if(strcmp(stringmessage+sizeof(char),"END") == 0){
        return 1000;
    }
    //�᫨ ������ ��⮢ ��ࠡ���� ����� � �⠢�� ��� � ��।� �����⮢
    //� ᨣ����� �� ���� ᢮����� ������� - ��� �������, �᫨ ��� ������!!!, ����� ᨣ������ ����易⥫쭮!!!
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
				SetEvent(evntDat);
				ReleaseMutex(mtxQMsg);
				
        return wres;
    }   
    //�᫨ ��諨 ����� �� ��।���⥫�� ����� � �⠢�� �� � ��।� ������
    //� ᨣ����� �� ���� ����� ��� ��ࠡ�⪨
    if(strstr(stringmessage+sizeof(char),"DATA") != NULL){
        msg.sock = sock;
        memcpy((void*)msg.msg,(void*)(stringmessage+sizeof(char)),4);
        memcpy((void*)msg.dat,(void*)(stringmessage+5*sizeof(char)),12);
        msg.Priority = *stringmessage;
        
        DWORD wres = WaitForSingleObject(mtxQDat,INFINITE);
    	  if(wres == WAIT_OBJECT_0){ 
    	  	if(NULL != Qdat){
    				HeapInsert(Qdat, 0, sizeof(MESSAGE), (void*)(&msg), ComparePriority);	  		
    				++datacount;
    				printf("����� �%d\t\t%s\n",datacount,msg.dat);
    	  	}
        }
        SetEvent(evntDat);
        ReleaseMutex(mtxQDat);

        return 0;
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
void sendMsgForAll(char * str){
    int ret;
    char szCmdBuffer[32] = {0};
    if(strlen(str)>31) exit(1);
    strcpy(szCmdBuffer,str);
    size_t j;
    for(j=0; j < HeapGetSize(Qusr); j++){
    		HEAP_ELEMENT e;
    		e = Qusr->Heap[j];
    		MESSAGE *m = (MESSAGE*)e.Object;
        SOCKET sock = m->sock;
        ret = send(sock, szCmdBuffer, strlen(szCmdBuffer)+1, 0);
        if (ret == SOCKET_ERROR){
            ErrorShow("send() for client failed,");
        }else{
            if(ret == 0){
                ErrorShow("ClientManagerThread ret = 0\n"); 
            }
        }
    }
    return;
}
///////////////////////////////////////////////////////////////////////////////
//�஢���� ���� �� ᮮ�饭�� �⮣� ������ � ��।� ᮮ�饭�� � ��⮢���� �ਭ��� �����
//����室��� ��� ��������� ����७�� � ��।� ᮮ�饭��.
//�⮡� � ��।� ᮮ�饭�� ���� � �� �� ������ �� ��������, �.�. �� �ᥣ� ���� ࠧ
int EqualClients(SOCKET s){
	int res = 0;
	size_t j;
	for(j=0; j< Qmsg->Count; j++){
			HEAP_ELEMENT e;
			e = Qmsg->Heap[j];
			MESSAGE *m = (MESSAGE*)e.Object;
			SOCKET s1 = m->sock;
			if(s1 == s) res = 1;		
	}
	return res;
}
///////////////////////////////////////////////////////////////////////////////
//���ᯥ稢��� �뤠�� ������ ��⮢� �� �ਭ��� �����⠬
DWORD WINAPI ClientManagerThread(PVOID pvParam){
    int ret;
    MESSAGE msg = {0};
    MESSAGE dat = {0};
    DWORD wres;
    int successM = 0;
    int successD = 0;

    while (1){
        successM = 0;
        successD = 0;
        WaitForSingleObject(evntDat, INFINITE);
        ResetEvent(evntDat);
        
        wres = WaitForSingleObject(mtxQMsg,200);
        if(wres == WAIT_OBJECT_0){
        	if(Qmsg->Count != 0){
        		successM = 1;
        	}else{
        		successM = -1;
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
					wres = WaitForSingleObject(mtxQMsg,INFINITE);
					if(wres == WAIT_OBJECT_0)	HeapDelete(Qmsg, NULL, NULL, &msg, ComparePriority);
					ReleaseMutex(mtxQMsg);
					
					wres = WaitForSingleObject(mtxQDat,INFINITE);
					if(wres == WAIT_OBJECT_0) HeapDelete(Qdat, NULL, NULL, &dat, ComparePriority);
					ReleaseMutex(mtxQDat);
					
					char szMessage[64] = {'D','A','T',0};
					strcat(szMessage,dat.dat);
					//printf("dat.dat=%s,szMessage = %s\n",dat.dat,szMessage);
					ret = send(msg.sock, szMessage, strlen(szMessage)+1, 0);
					if (ret == SOCKET_ERROR){
							ErrorShow("send() for client failed,");
					}else{
							if(ret == 0) printf("ClientManagerThread ret = 0\n"); 
					}
					szMessage[0] = 0;
				}
    }
    return (0);
}
///////////////////////////////////////////////////////////////////////////////
int PrintMessage(const void *Object,
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
	printf("ᮮ�饭��:%d,������:%d,�����⮢:%d\n",
					HeapGetSize(Qmsg),
					HeapGetSize(Qdat),
					HeapGetSize(Qusr));
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