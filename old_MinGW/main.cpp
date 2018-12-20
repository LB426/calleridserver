#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "main.h"

int verbose = 0;

///////////////////////////Вывести сообщение об ошибке/////////////////////////
void ErrorShow(char * text)
{
    LPVOID lpMessageBuffer;
    int error = GetLastError();
    FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                    (LPTSTR) &lpMessageBuffer,
                    0,
                    NULL);
    char * str;
    char buf[8];
    int len = strlen(text) + strlen((char*)lpMessageBuffer) + 10;
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
///////////////////////////////////////////////////////////////////////////////
//получить время
void GetDataTime(char *datatime)
{
    time_t now;
    struct tm * ptr;
    
    time(&now);
    ptr = localtime(&now);
    strcpy(datatime,asctime(ptr));
    int x=strlen(datatime);
    datatime[x-1]=0;
}
///////////////////////////////////////////////////////////////////////////////
//Выводит строку в окно сообщений
void toLog(char * szString){
    printf("%s\n",szString);
}

void toLog(int n){
    printf("%d\n",n);
}
///////////////////////////////////////////////////////////////////////////////
char * ANomberAnalise(char * anomber){
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
///////////////////////////////////////////////////////////////////////////////
int CountThreads = 0;

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

void exithread(){
    CountThreads--;
    printf("потоков : %d\n",CountThreads);
    return;
}

void run(){
    runthread(runserver,NULL);
    return;
}
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[]){

    if(argc>1){
        for(int i=1;i<argc;i++){
            if(strcmp(argv[i],"-RUN")==0){
                run();
            }
        }
    }
    if(argc==1){
        printf("программа запущена без параметров!!!\n");
        printf("\t-RUN\t: запускает сервер.\n");        
        printf("\tV - пошаговый вывод\n");
        printf("\tКоманды во время выполнения:\n");
        printf("\td - показать текущее состояние\n");
        printf("\tm - показать очередь сообщений\n");
        printf("\tu - показать очередь клиентов\n");
        printf("\ts - показать очередь данных\n");
        return (1);
    }
    
    while(1){
    		printf("введите команду : ");
        char c;
        c = getchar();
        if((c=='e')||(c=='E')||(c=='у')||(c=='У')){
            printf("exit\nПРОГРАММА ОСТАНАВЛИВАЕТСЯ...\n");
            break;
        }
        if((c=='d')||(c=='D')||(c=='в')||(c=='В')){
        	ShowParam();
        }
        if(c=='m')	ShowQmsg();
        if(c=='u')	ShowQusr();
        if(c=='s')	ShowQdat();
        if(c=='v') 	{
        	int x,y,z,q; x=1;y=2;z=3;q=5; 
        	printf("1/2=%d , 2/2=%d , 3/2=%d , 5/2=%d\n",x/2,y/2,z/2,q/2);
        	hVerify();
        }
        if(c=='V') verbose = 1;
    }
		
    return (0);
}
