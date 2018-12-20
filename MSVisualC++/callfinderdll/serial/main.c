#include <windowsx.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

#define NOTANMBR "A NOMBER NOT DEFINED"

char  AT[7] = {13,10,'A','T',13,10,0};
char  ATZ[6] = {'A','T','Z',13,10,0};
char  ATE0[6] = {'A','T','E','0',13,0};
char  ATI[7] = {'A','T','I','0',13,10,0};
char  ATCLIP1[12] = {'A','T','+','C','L','I','P','=','1',13,10,0};
char  ATCLIPV[11] = {'A','T','+','C','L','I','P','?',13,10,0};
char  ATVCID1[12] = {'A','T','+','V','C','I','D','=','1',13,10,0};
char  CRLF[3] = {13,10,0};
BOOL  end = FALSE;
char    ownnum[11] = {'0','0','0','0','0','0','0','0','0','0',0};
char  prio = '0';
SIOTERMINALPARAM sioterminalparam [8] = {0};

BOOL WINAPI DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
  return 1;
  UNREFERENCED_PARAMETER(hInst);
  UNREFERENCED_PARAMETER(ul_reason_being_called);
  UNREFERENCED_PARAMETER(lpReserved);
}

unsigned long _stdcall GENERIC_SIO_Terminal_loop(void * param)
{
  SIOTERMINALPARAM * pSiopar = (SIOTERMINALPARAM*)param;
  SOCKET sock;
  char comportname[64] = {0};
  char modelteleph[64] = {0};
  char comment[64] = {0};
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS tm;
  DWORD w1,w2;
  char buf[64] = {0};
  DWORD read=0;
  DWORD err=0;
  int count=0;
  int ringcount = 0;
  LARGE_INTEGER PerformanceCountLast;
  LARGE_INTEGER PerformanceCount;
  __int64 period = 0;
  unsigned long   watchPeriod = 20000000;
  FILE *f;
  char prevNomber[64]= {0};
  
  sock = pSiopar->s;
  strcpy(comportname, pSiopar->sioportname);
  strcpy(modelteleph, pSiopar->modeltelephname);
  strcpy(comment, pSiopar->comment);
  watchPeriod = pSiopar->watchperiod;

  printf("COM порт поток стартанул на порту: %s, телефон %s, комментарий %s\n",comportname,modelteleph,comment);
  hPort = CreateFile( comportname,
                      GENERIC_READ|GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
  if (hPort == INVALID_HANDLE_VALUE){
      printf("Порт %s не открыт\n\n",comportname);
  }else{
      printf("Порт %s открыт успешно\n\n",comportname);
      memset(&dcb,0,sizeof(dcb));

      GetCommState(hPort,&dcb);
      f = fopen("comparam.dcb", "rb");
      if(f == NULL){
        f = fopen("comparam.dcb", "wb");
        fwrite(&dcb,sizeof(dcb),1,f);
      }else{
        memset(&dcb,0,sizeof(dcb)); 
        fread(&dcb,sizeof(dcb),1,f);
        if(!SetCommState(hPort,&dcb)){
            printf("SetCommState failed\n");
            CloseHandle(hPort);
            return (1);
        }
      }
      fclose(f);

      dcb.DCBlength = sizeof(dcb);
      dcb.BaudRate = 2400;
      dcb.fBinary = 1;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;
      dcb.Parity = NOPARITY;
      dcb.StopBits = ONESTOPBIT;
      dcb.ByteSize = 8;
      dcb.XonLim = 80;
      dcb.XoffLim = 200;
      
      if(!SetCommState(hPort,&dcb)){
          printf("SetCommState failed\n");
          CloseHandle(hPort);
          return (1);
      } 
        
      if(GetCommTimeouts(hPort, &tm)){
          printf("Timeouts OK\n");
          printf("ReadIntervalTimeout=%d\n",tm.ReadIntervalTimeout);
          printf("ReadTotalTimeoutMultiplier=%d\n",tm.ReadTotalTimeoutMultiplier);
          printf("ReadTotalTimeoutConstant=d%\n",tm.ReadTotalTimeoutConstant);
      tm.ReadIntervalTimeout = 10;
      tm.ReadTotalTimeoutMultiplier = 0;
      tm.ReadTotalTimeoutConstant = 0;
      tm.ReadTotalTimeoutConstant = 0;
      tm.ReadTotalTimeoutConstant = 5000;

//          tm.ReadTotalTimeoutMultiplier = 100;
//          tm.ReadTotalTimeoutConstant = 100;
          if(SetCommTimeouts(hPort,&tm)){
              printf("SetCommTimeouts OK.\n");
          }else{
              printf("SetCommTimeouts ERROR.\n");
          }
          //return (0);
      }else{
          return (1);
      }
    
      PerformanceCountLast.QuadPart = 0;

      w1=strlen(ATE0);
      printf("ATE0\n");
      WriteFile(hPort,ATE0,w1,&w2,0);
      ReadFile(hPort,buf,63,&read,0); //если пришло только ATE0 то ловим OK
      if(read == 0){
        //ErrorShow("ATE0, яюЁЄ эх юЄтхўрхЄ!!!");
        printf("порт не отвечает!!!\n");
    err = 3000;
      } 
      buf[read]=0;
      printf("%d ",count++);
      printbuf(buf,read);

      if((strstr(buf,"ATE0") != NULL)&&(strstr(buf,"OK") != NULL)){ //если пришло и ATE0 и OK то ничего не ловим
      }
      if(strcmp(buf,"ATE0\r\n") == 0){        //если пришло только ATE0 то ловим OK
          ReadFile(hPort,buf,63,&read,0);
          buf[read]=0;
          printf("%d ",count++);
          printbuf(buf,read);
          if(strstr(buf,"OK") != NULL){       //если пришло только OK
          }
      }
            
      w1=strlen(ATCLIP1);
      printf("AT+CLIP=1\n");
      WriteFile(hPort,ATCLIP1,w1,&w2,0);
      ReadFile(hPort,buf,63,&read,0);
      if(read == 0){
        //ErrorShow("AT+CLIP, яюЁЄ эх юЄтхўрхЄ!!!");
        printf("порт не отвечает!!!\n");
        err = 3000;
      }
      buf[read]=0;
      printf("%d, %s ",count++, modelteleph);
      printbuf(buf,read);

      printf("запускаю цикл ожидания номера несмотря ни на что!!!\n");
    
      while(!end){
          ZeroMemory(buf,64);
          if(err == 3000){
            char b[128] = {0};
            strcpy(b,"чртхЁ°р■ яюЄюъ юяЁхфхыхэш  эюьхЁр ё яюёыхфютрЄхы№эюую ЄхЁьшэрыр, ");
            strcat(b,modelteleph);
            strcat(b, ", ");
            strcat(b, comment);
            //ErrorShow(b);
            //break;
            printf("несмотря ни на какие ошибки всёравно не прерываю цикл ожидания номера!!!\n");
          } 
          //printf("ЖДУ ЗВОНКА...\n");
          ReadFile(hPort,buf,63,&read,0);
          printf("прочитано из порта: %d\n",read);
          if (read < sizeof(buf)){
              buf[read] = 0;
              //printf("прилетел буфер: %s\n",buf);
          }else{
              printf("Buffer overflow\n");
              break;
          }
          //printf("прилетел буфер buf-> %s\n",buf);
                                              
          period = 0;
          if(strstr(buf,"+CLIP") != NULL){
              ringcount++;
              if (QueryPerformanceCounter(&PerformanceCount) == 0) 
                  printf("QueryPerformanceCounter");
              period = PerformanceCount.QuadPart - PerformanceCountLast.QuadPart;
              PerformanceCountLast = PerformanceCount;
              if (period > watchPeriod){
         		char Nomber[64] = {0};
                strcpy(Nomber, anbrAnalise(anbrGet(buf)));
         		if(strstr(prevNomber,Nomber) == NULL){
            		strcat(Nomber, ";");
                    strcat(Nomber, comment);
                    sendmsgtoserver(sock, prio, "DATA", Nomber);
            		strcpy(prevNomber,Nomber);
                    printf("отправили на сервер Nomber %s, buf = \"%s\"\n",Nomber,buf);
         		}
              }           
          }
          w1=strlen(AT);
          //printf("посылаю в порт: ->%s",AT);
          WriteFile(hPort,AT,w1,&w2,0);
      buf[0] = 0; 
      }
      CloseHandle(hPort);                 
  }
  closeipconnection(sock);  
  printf("Завершился поток определения номера с последовательного терминала.\n");
  
  return 0;
}

unsigned long _stdcall SERI_T630_SIO_Terminal_loop(void * param) //Sony Ericsson T630
{
  SIOTERMINALPARAM * pSiopar = (SIOTERMINALPARAM*)param;
  SOCKET sock;
  char comportname[64] = {0};
  char modelteleph[64] = {0};
  char comment[64] = {0};
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS tm;
  DWORD w1,w2;
  char buf[64] = {0};
  DWORD read=0;
  DWORD err=0;
  int count=0;
  int ringcount = 0;
  LARGE_INTEGER PerformanceCountLast;
  LARGE_INTEGER PerformanceCount;
  __int64 period = 0;
  unsigned long   watchPeriod = 20000000;
  char prevNomber[64]= {0};
  
  sock = pSiopar->s;
  strcpy(comportname, pSiopar->sioportname);
  strcpy(modelteleph, pSiopar->modeltelephname);
  strcpy(comment, pSiopar->comment);
  watchPeriod = pSiopar->watchperiod;

  printf("COM порт поток стартанул на порту: %s, телефон %s, комментарий %s\n",comportname,modelteleph,comment);
  hPort = CreateFile( comportname,
                      GENERIC_READ|GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
  if (hPort == INVALID_HANDLE_VALUE){
      printf("Порт %s не открыт\n\n",comportname);
  }else{
      printf("Порт %s открыт успешно\n\n",comportname);
      memset(&dcb,0,sizeof(dcb));
      dcb.DCBlength = sizeof(dcb);
      dcb.BaudRate = 2400;
      dcb.fBinary = 1;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;
      dcb.Parity = NOPARITY;
      dcb.StopBits = ONESTOPBIT;
      dcb.ByteSize = 8;
      dcb.XonLim = 80;
      dcb.XoffLim = 200;

      if(!SetCommState(hPort,&dcb)){
          printf("SetCommState failed\n");
          CloseHandle(hPort);
          return (1);
      }
        
     if(GetCommTimeouts(hPort, &tm)){
          printf("Timeouts OK\n");
          printf("ReadIntervalTimeout=%d\n",tm.ReadIntervalTimeout);
          printf("ReadTotalTimeoutMultiplier=%d\n",tm.ReadTotalTimeoutMultiplier);
          printf("ReadTotalTimeoutConstant=%d\n",tm.ReadTotalTimeoutConstant);

      tm.ReadIntervalTimeout = 10;
      tm.ReadTotalTimeoutMultiplier = 0;
      tm.ReadTotalTimeoutConstant = 0;
      tm.ReadTotalTimeoutConstant = 0;
      tm.ReadTotalTimeoutConstant = 5000;
          if(SetCommTimeouts(hPort,&tm)){
              printf("SetCommTimeouts OK.\n");
          }else{
              printf("SetCommTimeouts ERROR.\n");
          }
      }else{
          return (1);
      }
    
      PerformanceCountLast.QuadPart = 0;

      w1=strlen(ATE0);
      printf("->%s",ATE0);
      WriteFile(hPort,ATE0,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим OK
        if(strstr(buf,"OK") != NULL){
            printf("<-OK\n");
            break;
        }   
      }
      w1=strlen(ATCLIP1);
      printf("->%s",ATCLIP1);
      WriteFile(hPort,ATCLIP1,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим OK
        if(strstr(buf,"OK") != NULL){
            printf("<-OK\n");
            break;
        }   
      }
      w1=strlen(ATCLIPV);
      printf("->%s",ATCLIPV);
      WriteFile(hPort,ATCLIPV,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим OK
        if(strstr(buf,"+CLIP: 1,1") != NULL){
            printf("<-+CLIP: 1,1\n");
            break;
        }   
      }      
      
      while(!end){
          ZeroMemory(buf,64);
          if(err == 3000){
            char b[128] = {0};
            strcpy(b,"чртхЁ°р■ яюЄюъ юяЁхфхыхэш  эюьхЁр ё яюёыхфютрЄхы№эюую ЄхЁьшэрыр, ");
            strcat(b,modelteleph);
            strcat(b, ", ");
            strcat(b, comment);
            ErrorShow(b);
            break;
          } 
          //printf("ЖДУ ЗВОНКА...\n");
          ReadFile(hPort,buf,63,&read,0);
          if (read < sizeof(buf)){
              buf[read] = 0;
              //printf("%s\n",buf);
          }else{
              printf("Buffer overflow\n");
              break;
          }
          printf("прилетел buf-> \"%s\"\n",buf);
            
          period = 0;
          if(strstr(buf,"+CLIP") != NULL){
              ringcount++;
              if (QueryPerformanceCounter(&PerformanceCount) == 0) 
                  printf("QueryPerformanceCounter");
              period = PerformanceCount.QuadPart - PerformanceCountLast.QuadPart;
          printf("period: %d\n",period);
              PerformanceCountLast = PerformanceCount;
              if (period > watchPeriod){
                  char Nomber[64] = {0};
                  strcpy(Nomber, anbrAnalise(anbrGet(buf)));
          if(strstr(prevNomber,Nomber) == NULL){
            strcat(Nomber, ";");
                    strcat(Nomber, comment);
                    sendmsgtoserver(sock, prio, "DATA", Nomber);
            strcpy(prevNomber,Nomber);
                    printf("отправили на сервер Nomber %s, buf = \"%s\"\n",Nomber,buf);
          }
              }           
          }
          w1=strlen(AT);
          //printf("->%s",AT);
          WriteFile(hPort,AT,w1,&w2,0);
      }
      CloseHandle(hPort);                 
  }
  closeipconnection(sock);  
  printf("Завершился поток определения номера с последовательного терминала.\n");
  
  return 0;
}

unsigned long _stdcall NOKIA_SIO_Terminal_loop(void * param)
{
  SIOTERMINALPARAM * pSiopar = (SIOTERMINALPARAM*)param;
  SOCKET sock;
  char comportname[64] = {0};
  char modelteleph[64] = {0};
  char comment[64] = {0};
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS tm;
  DWORD w1,w2;
  char buf[64] = {0};
  DWORD read=0;
  DWORD err=0;
  int count=0;
  int ringcount = 0;
  LARGE_INTEGER PerformanceCountLast;
  LARGE_INTEGER PerformanceCount;
  __int64 period = 0;
  unsigned long   watchPeriod = 20000000;
  FILE *f;
  char prevNomber[64]= {0};
  
  sock = pSiopar->s;
  strcpy(comportname, pSiopar->sioportname);
  strcpy(modelteleph, pSiopar->modeltelephname);
  strcpy(comment, pSiopar->comment);
  watchPeriod = pSiopar->watchperiod;

  printf("COM порт поток стартанул на порту: %s, телефон %s, комментарий %s\n",comportname,modelteleph,comment);
  hPort = CreateFile( comportname,
                      GENERIC_READ|GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
  if (hPort == INVALID_HANDLE_VALUE){
      printf("Порт %s не открыт\n\n",comportname);
  }else{
      printf("Порт %s открыт успешно\n\n",comportname);
      memset(&dcb,0,sizeof(dcb));
      
      GetCommState(hPort,&dcb);
      f = fopen("comparam-nokia.dcb", "rb");
      if(f == NULL){
        f = fopen("comparam-nokia.dcb", "wb");
        fwrite(&dcb,sizeof(dcb),1,f);
      }else{
        memset(&dcb,0,sizeof(dcb)); 
        fread(&dcb,sizeof(dcb),1,f);
        if(!SetCommState(hPort,&dcb)){
            printf("SetCommState failed\n");
            CloseHandle(hPort);
            return (1);
        }
      }
      fclose(f);
      
      dcb.DCBlength = sizeof(dcb);
      dcb.BaudRate = 2400;
      dcb.fBinary = 1;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;
      dcb.Parity = NOPARITY;
      dcb.StopBits = ONESTOPBIT;
      dcb.ByteSize = 8;
      dcb.XonLim = 80;
      dcb.XoffLim = 200;

      if(!SetCommState(hPort,&dcb)){
          printf("SetCommState failed\n");
          CloseHandle(hPort);
          return (1);
      }

    if(GetCommTimeouts(hPort, &tm)){
          printf("Timeouts OK\n");
          printf("ReadIntervalTimeout=%d\n",tm.ReadIntervalTimeout);
          printf("ReadTotalTimeoutMultiplier=%d\n",tm.ReadTotalTimeoutMultiplier);
          printf("ReadTotalTimeoutConstant=%d\n",tm.ReadTotalTimeoutConstant);
          
      tm.ReadIntervalTimeout = 10;
      tm.ReadTotalTimeoutMultiplier = 0;
      tm.ReadTotalTimeoutConstant = 0;
      tm.ReadTotalTimeoutConstant = 0;
      tm.ReadTotalTimeoutConstant = 5000;

          if(SetCommTimeouts(hPort,&tm)){
              printf("SetCommTimeouts OK.\n");
          }else{
              printf("SetCommTimeouts ERROR.\n");
          }
      }else{
          return (1);
      }
    
      PerformanceCountLast.QuadPart = 0;

      w1=strlen(ATZ);
      printf("->%s",ATZ);
      WriteFile(hPort,ATZ,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим OK
        if(strstr(buf,"OK") != NULL){
            printf("<-OK\n");
            break;
        }   
      }
      w1=strlen(ATE0);
      printf("->%s",ATE0);
      WriteFile(hPort,ATE0,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим OK
        if(strstr(buf,"OK") != NULL){
            printf("<-OK\n");
            break;
        }   
      }
      w1=strlen(ATCLIP1);
      printf("->%s",ATCLIP1);
      WriteFile(hPort,ATCLIP1,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим OK
        if(strstr(buf,"OK") != NULL){
            printf("<-OK\n");
            break;
        }   
      }
      w1=strlen(ATCLIPV);
      printf("->%s",ATCLIPV);
      WriteFile(hPort,ATCLIPV,w1,&w2,0);
      while(!end){
        ZeroMemory(buf,64);
        ReadFile(hPort,buf,63,&read,0); //ловим +CLIP: 1,1
        printf("%d->\"%s\"", read,buf);
        if(strstr(buf,"+CLIP: 1,1") != NULL){
            printf("<-+CLIP: 1,1\n");
            break;
        }   
      }      
      
      while(!end){
          ZeroMemory(buf,64);
          if(err == 3000){
            char b[128] = {0};
            strcpy(b,"чртхЁ°р■ яюЄюъ юяЁхфхыхэш  эюьхЁр ё яюёыхфютрЄхы№эюую ЄхЁьшэрыр, ");
            strcat(b,modelteleph);
            strcat(b, ", ");
            strcat(b, comment);
            ErrorShow(b);
            break;
          } 
          printf("ЖДУ ЗВОНКА...\n");
          ReadFile(hPort,buf,63,&read,0);
          if (read < sizeof(buf)){
              buf[read] = 0;
          }else{
              printf("Buffer overflow\n");
              break;
          }
          //printf("прилетел buf-> \"%s\"\n",buf);
            
          period = 0;
          if(strstr(buf,"+CLIP") != NULL){
              ringcount++;
              if (QueryPerformanceCounter(&PerformanceCount) == 0) 
                  printf("QueryPerformanceCounter");
              period = PerformanceCount.QuadPart - PerformanceCountLast.QuadPart;
              PerformanceCountLast = PerformanceCount;
              if (period > watchPeriod){
          char Nomber[64] = {0};
                  strcpy(Nomber, anbrAnalise(anbrGet(buf)));
          if(strstr(prevNomber,Nomber) == NULL){
            strcat(Nomber, ";");
                    strcat(Nomber, comment);
                    sendmsgtoserver(sock, prio, "DATA", Nomber);
            strcpy(prevNomber,Nomber);
                    printf("отправили на сервер Nomber %s, buf = \"%s\"\n",Nomber,buf);
          }
              }           
          }
          w1=strlen(AT);
          //printf("->%s",AT);
          WriteFile(hPort,AT,w1,&w2,0);
      }
      CloseHandle(hPort);                 
  }
  closeipconnection(sock);  
  printf("Завершился поток определения номера с последовательного терминала.\n");
  
  return 0;
}

unsigned long _stdcall OFFICEGATE_SIO_Terminal_loop(void * param)
{
  SIOTERMINALPARAM * pSiopar = (SIOTERMINALPARAM*)param;
  SOCKET sock;
  char comportname[64] = {0};
  char modelteleph[64] = {0};
  char comment[64] = {0};
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS tm;
  char buf[64] = {0};
  int bufsize = 0;
  DWORD read=0;
  DWORD err=0;
  int count=0;
  int ringcount = 0;
  char prevNomber[64]= {0};
  char logtext[2048] = {0};
  char coll[128] = {0};
  
  bufsize = sizeof(buf);
  sock = pSiopar->s;
  strcpy(comportname, pSiopar->sioportname);
  strcpy(modelteleph, pSiopar->modeltelephname);
  strcpy(comment, pSiopar->comment);

  logger(pSiopar, "port: %s, teleph %s, comment %s\n",comportname,modelteleph,comment);
  hPort = CreateFile( comportname, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hPort == INVALID_HANDLE_VALUE){
      logger(pSiopar, "port open error %s\n",comportname);
  }else{
      logger(pSiopar, "port open success %s\n",comportname);
      memset(&dcb,0,sizeof(dcb));
      dcb.DCBlength = sizeof(dcb);
      dcb.BaudRate = 115200;
      dcb.fBinary = 1;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;
      dcb.Parity = NOPARITY;
      dcb.StopBits = ONESTOPBIT;
      dcb.ByteSize = 8;
      dcb.XonLim = 80;
      dcb.XoffLim = 200;
      if(!SetCommState(hPort,&dcb)){
          logger(pSiopar, "SetCommState failed\n");
          CloseHandle(hPort);
          return (1);
      } 
      if(GetCommTimeouts(hPort, &tm)){
          logger(pSiopar, "Timeouts OK\n");
          logger(pSiopar, "ReadIntervalTimeout=%d\n",tm.ReadIntervalTimeout);
          logger(pSiopar, "ReadTotalTimeoutMultiplier=%d\n",tm.ReadTotalTimeoutMultiplier);
          logger(pSiopar, "ReadTotalTimeoutConstant=d%\n",tm.ReadTotalTimeoutConstant);
          tm.ReadIntervalTimeout = 10;
          tm.ReadTotalTimeoutMultiplier = 0;
          tm.ReadTotalTimeoutConstant = 0;
          tm.ReadTotalTimeoutConstant = 0;
          tm.ReadTotalTimeoutConstant = 5000;
          if(SetCommTimeouts(hPort,&tm)){
              logger(pSiopar, "SetCommTimeouts OK.\n");
          }else{
              logger(pSiopar, "SetCommTimeouts ERROR.\n");
          }
      }else{
          return (1);
      }
      logger(pSiopar, "start loop wait incoming call\n");
      ZeroMemory(coll,sizeof(coll));
      while(!end){
          ZeroMemory(buf,bufsize);
          if(err == 3000){
            logger(pSiopar, "err == 3000\n");
            break;
          } 
          printf(".");
          ReadFile(hPort,buf,bufsize,&read,0);
          logger(pSiopar, "Priletel BUFFFER, SIZE = %d, BUF = \"%s\"\n",read,buf);
          if (read > 0){
            if (read < bufsize){
              buf[read] = 0;
              strcat(coll,buf);
              if( (strstr(coll,"+CRING: VOICE") != NULL) && 
                  (strstr(coll,"+CLIP:") != NULL) &&
                  (strstr(coll,"\",145,\"") != NULL))
              {
                char Nomber[64] = {0};
                strcpy(Nomber, anbrAnalise(anbrGet(coll)));
                if(strstr(prevNomber,Nomber) == NULL){
                  strcat(Nomber, ";");
                  strcat(Nomber, comment);
                  sendmsgtoserver(sock, prio, "DATA", Nomber);
                  strcpy(prevNomber,Nomber);
                  logger(pSiopar, "send to server Nomber %s\n",Nomber);
                }
                ZeroMemory(coll,sizeof(coll));
              }
            }else{
              logger(pSiopar, "Priletel BUFFER bolshe chem BUF variable. Thread wil be stopped.\n");
              break;
            }
          }
          
          // хёыш ЄЁєсъє эх ёэ ыш - юўш∙рхь prevNomber фы  Єюую ўЄюс√ юэ юяЁхфхышыё  хёыш яючтюэ Є х∙╕ Ёрч ё Єюую цх эюьхЁр
          if(strstr(buf,"NO CARRIER") != NULL){
            ZeroMemory(prevNomber,64);
          }
      }
      CloseHandle(hPort);                 
  }
  closeipconnection(sock);  
  logger(pSiopar, "Thread stopped\n");
  
  return 0;
}

/*
Для Акорпа нужна версия микропрограммы V1.88(V90)
видимо это файл ACF31088.S37
*/

unsigned long _stdcall ACORPSprinter56KExt_SIO_Terminal_loop(void * param)
{
  SIOTERMINALPARAM * pSiopar = (SIOTERMINALPARAM*)param;
  SOCKET sock;
  char comportname[64] = {0};
  char modelteleph[64] = {0};
  char comment[64] = {0};
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS tm;
  char buf[256] = {0};
  DWORD read=0;
  DWORD err=0;
  DWORD w1,w2;
  int count=0;
  int ringcount = 0;
  LARGE_INTEGER PerformanceCountLast;
  LARGE_INTEGER PerformanceCount;
  __int64 period = 0;
  unsigned long   watchPeriod = 20000000;
  char prevNomber[256] = {0};
  char logtext[2048] = {0};
  
  sock = pSiopar->s;
  strcpy(comportname, pSiopar->sioportname);
  strcpy(modelteleph, pSiopar->modeltelephname);
  strcpy(comment, pSiopar->comment);
  watchPeriod = pSiopar->watchperiod;

  printf("COM порт поток стартанул на порту: %s, телефон %s, комментарий %s\n",comportname,modelteleph,comment);
  hPort = CreateFile( comportname,
                      GENERIC_READ|GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
  if (hPort == INVALID_HANDLE_VALUE){
      printf("Порт %s не открыт\n\n",comportname);
  }else{
      printf("Порт %s открыт успешно\n\n",comportname);
      memset(&dcb,0,sizeof(dcb));

      dcb.DCBlength = sizeof(dcb);
      dcb.BaudRate = 9600;
      dcb.fBinary = 1;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
      dcb.fOutxCtsFlow = 1;
      dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;
      dcb.Parity = NOPARITY;
      dcb.StopBits = ONESTOPBIT;
      dcb.ByteSize = 8;
      dcb.XonLim = 80;
      dcb.XoffLim = 200;
      
      if(!SetCommState(hPort,&dcb)){
          printf("SetCommState failed\n");
          CloseHandle(hPort);
          return (1);
      } 
        
      if(GetCommTimeouts(hPort, &tm)){
          printf("Timeouts OK\n");
          printf("ReadIntervalTimeout=%d\n",tm.ReadIntervalTimeout);
          printf("ReadTotalTimeoutMultiplier=%d\n",tm.ReadTotalTimeoutMultiplier);
          printf("ReadTotalTimeoutConstant=d%\n",tm.ReadTotalTimeoutConstant);
          tm.ReadIntervalTimeout = 10;
          tm.ReadTotalTimeoutMultiplier = 0;
          tm.ReadTotalTimeoutConstant = 0;
          tm.ReadTotalTimeoutConstant = 0;
          tm.ReadTotalTimeoutConstant = 5000;

          if(SetCommTimeouts(hPort,&tm)){
              printf("SetCommTimeouts OK.\n");
          }else{
              printf("SetCommTimeouts ERROR.\n");
          }
      }else{
          return (1);
      }
    
      PerformanceCountLast.QuadPart = 0;

      w1=strlen(ATVCID1);
      printf("-> AT+VCID=1");
      WriteFile(hPort,ATVCID1,w1,&w2,0);
      printf("запускаю цикл ожидания номера!!!\n");
    
      while(!end){
          ZeroMemory(buf,256);
          if(err == 3000){
            char b[512] = {0};
            strcpy(b,"завершаю поток определения номера с последовательного терминала, ");
            strcat(b,modelteleph);
            strcat(b, ", ");
            strcat(b, comment);
            printf("несмотря ни на какие ошибки всёравно не прерываю цикл ожидания номера!!!\n");
            break;
          } 
          printf("\nACORP ЖДЁТ ЗВОНКА...\n");
          ReadFile(hPort,buf,256,&read,0);
          printf("прилетел буфер buf-> \"%s\"\nдлиной %d байт\n",buf,read);
          if (read < sizeof(buf)){
              buf[read] = 0;
          }else{
              printf("Buffer overflow\n");
              break;
          }
                                             
          if(strstr(buf,"NMBR =") != NULL){
            char Nomber[64] = {0};
            strcpy(Nomber, anbrAnalise(anbrGetAcorp(buf)));
            strcat(Nomber, ";");
            strcat(Nomber, comment);
            sendmsgtoserver(sock, prio, "DATA", Nomber);
            printf("отправили на сервер Nomber %s, buf = \"%s\"\n",Nomber,buf);
          }
          ZeroMemory(prevNomber,64);
          buf[0] = 0;
      }
      CloseHandle(hPort);                 
  }
  closeipconnection(sock);  
  printf("Завершился поток определения номера с последовательного терминала.\n");
  
  return 0;
}

int _stdcall runserialterminalfinder()
{
  SOCKET skt = {0};
  char host[16] = {0};
  unsigned short port = 0;
  char prioritet = '1';
  FILE * fini;
  char buf[256];
  int amount = 0;
  int i = 0;

  prio = prioritet;
  if ((fini = fopen("settings.ini", "r")) == NULL){
      ErrorShow("Error open file log");
      return -1;
  }
//инициализация параметров запуска потоков ком портов
  while (!feof(fini)){
    fgets(buf,256,fini);
    if (strstr(buf,"serial.terminal.amount") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      amount = atoi(pszToken);
      if(amount == 0){
        return 1;
      }else{
        printf("amount=%d\n",amount);
      }
    }
    if (strstr(buf,"serial.terminal.server.host") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      strcpy(host, pszToken);
      printf("terminal host=%s\n",host);
    }
    if (strstr(buf,"serial.terminal.server.port") != NULL){
      char * pszToken = strtok(buf,"=");
      pszToken = strtok(NULL,"\n");
      port = atoi(pszToken);  
      printf("terminal port=%d\n",port);
    }
  }
  fclose(fini);
//инициализация массива содержащего структуры описывающие телефон - ком порт, сокет и название телефона 
  for(i=1;i<=amount;i++){
    char stnp[128] = {0};
    char stntm[128] = {0};
    char stnperi[128] = {0};
    char stncomm[128] = {0};
    char n[32] = {0};
    
    
    sprintf(n,"%d",i);  
    strcpy(stnp,"serial.terminal.number.");
    strcat(stnp,n);
    strcat(stnp,".portname");
    strcpy(stntm,"serial.terminal.number.");
    strcat(stntm,n);
    strcat(stntm,".telef.model");
    strcpy(stnperi,"serial.terminal.number.");
    strcat(stnperi,n);
    strcat(stnperi,".period");
    strcpy(stncomm,"serial.terminal.number.");
    strcat(stncomm,n);
    strcat(stncomm,".comment");
    if ((fini = fopen("settings.ini", "r")) == NULL){
        ErrorShow("Error open file log");
        return -1;
    }
    while (!feof(fini)){
      fgets(buf,256,fini);
      if (strstr(buf,stnp) != NULL){
        char * pszToken = strtok(buf,"=");
        pszToken = strtok(NULL,"\n");
        strcpy(sioterminalparam[i].sioportname,pszToken);
      }
      if (strstr(buf,stntm) != NULL){
        char * pszToken = strtok(buf,"=");
        pszToken = strtok(NULL,"\n");
        strcpy(sioterminalparam[i].modeltelephname,pszToken);
      }
      if (strstr(buf,stnperi) != NULL){
        char * pszToken = strtok(buf,"=");
        pszToken = strtok(NULL,"\n");
        sioterminalparam[i].watchperiod = atoi(pszToken);
      }
      if (strstr(buf,stncomm) != NULL){
        char * pszToken = strtok(buf,"=");
        pszToken = strtok(NULL,"\n");
        strcpy(sioterminalparam[i].comment,pszToken);
      }
    }
    fclose(fini);
  }
  for(i=1;i<=amount;i++){
    skt = openipconnection(host, port);
    if(skt == 0) break;
    sioterminalparam[i].s = skt;
    if(strstr(sioterminalparam[i].modeltelephname, "SONYERICSSONT630") != NULL){
            runthread(SERI_T630_SIO_Terminal_loop, (void*)&sioterminalparam[i]);
    }
    if(strstr(sioterminalparam[i].modeltelephname, "NOKIA") != NULL){
            runthread(NOKIA_SIO_Terminal_loop, (void*)&sioterminalparam[i]);
    }
    if(strstr(sioterminalparam[i].modeltelephname, "OFFICEGATE") != NULL){
            runthread(OFFICEGATE_SIO_Terminal_loop, (void*)&sioterminalparam[i]);
    }
    if(strstr(sioterminalparam[i].modeltelephname, "ACORPSprinter56KExt") != NULL){
            runthread(ACORPSprinter56KExt_SIO_Terminal_loop, (void*)&sioterminalparam[i]);
    }
    if(strstr(sioterminalparam[i].modeltelephname, "GENERIC") != NULL){
            runthread(GENERIC_SIO_Terminal_loop, (void*)&sioterminalparam[i]);
    }
  }
  return 0;
}

int _stdcall stopserialterminalfinder()
{
  end = TRUE;
  //exit(1);
  return 0;
}

char * anbrGet(char * string)
{
    char * pszToken = strtok(string,"\"");
    pszToken = strtok(NULL,"\"");
    if(pszToken==NULL) 
      return NOTANMBR;
    else
      return pszToken;
}

char * anbrGetAcorp(char * string)
{
    char *p = NULL;
    int i = 0;
    int j = 0;
    char text[16] = {0};
    char digit[32] = {0};
    int flag = 0;
    p = string;
    for(i=0; *p!='\0'; p++){
      //printf("=============== char: '%c', digit: %d ============\n", *p, *p);
      if(isupper(*p)){
        text[j] = *p ;
        j++;
      }
      if(isspace(*p)){
        if(strstr(text,"NMBR") != NULL){
          flag = 1;
        }
        //printf("====%s=====\n",text);
        if(flag == 1 ){
          if((int)*p == 13) break ;
        }
        j = 0 ;
      }
      if(flag == 1){
        if(isdigit(*p)){
          digit[j] = *p ;
          j++;
        }
      }
    }
    digit[j] = '\0';
    //printf("--------------- %s ------------------\n",digit);
    strcpy(string,digit);
    return string;
}

char * anbrAnalise(char * anomber)
{
    switch (strlen(anomber)){
    case 10:
        {
            if ((*anomber == '8') &&
                (*(anomber+1) == '6') &&
                (*(anomber+2) == '1') &&
                (*(anomber+3) == '9') &&
                (*(anomber+4) == '6')) return anomber+5;
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
    case 11:
        {
            if(*anomber=='7') return anomber+1;
            if(*anomber=='8') return anomber+1;
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
      ErrorShow("sioterminal.dll, connect() for client failed");
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
    ErrorShow("sendipdata serial, send() for client failed");
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

void WriteToLog(char * line)
{
    return;
}

void logger(SIOTERMINALPARAM * pChParam, const char *fmt, ...)
{
    va_list ap;
    FILE *fp;
    SYSTEMTIME str_t;
    char logname[64] = {0};
    sprintf(logname, "%s-%s-%s.LOG", pChParam->sioportname, pChParam->modeltelephname, pChParam->comment);
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fp = fopen(logname,"a");
    GetSystemTime(&str_t);
    fprintf(fp, "%d-%d-%d %d:%d:%d\t>>\t", str_t.wYear,str_t.wMonth,str_t.wDay,str_t.wHour,str_t.wMinute,str_t.wSecond);
    vfprintf(fp, fmt, ap);
    fclose(fp);
    va_end(ap);
    return;
}

void printbuf(char * buf, int len){
  int i=0;  
  printf("Прочитан буфер: длиной %d байт. ",len);
  for(i=0;i<len;i++){
      if(isdigit(buf[i])||isprint(buf[i])) printf("%c",buf[i]);
      if(iscntrl(buf[i])) printf("^%d",buf[i]);
  }
  printf("\n\n");
  return;
}