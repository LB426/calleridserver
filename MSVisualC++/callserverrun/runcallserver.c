#include <windows.h>
#include <stdio.h>

extern int _stdcall runserver(PVOID pvParam);
//int _stdcall runisdnterminalfinder();

typedef int (APIENTRY RUNISDNTERMINALFINDER) ();
RUNISDNTERMINALFINDER* runisdnterminalfinder;
HINSTANCE h;
int err = 0;
HINSTANCE instance = NULL;

int loaddll(){
    h=LoadLibrary("ISDNFINDER.DLL");
    if(!h){
        err = GetLastError();
        printf("error LoadLibrary : %d\n",err);
    }
    return err;
}

int unloaddll(){
    if((err = FreeLibrary(h)) == 0){
        err = GetLastError();
        printf("error FreeLibrary : %d\n",err);
    }
    return err;
}

int iniTrunisdnterminalfinder(){
    runisdnterminalfinder = (RUNISDNTERMINALFINDER*)GetProcAddress((HMODULE) h, "runisdnterminalfinder");
    if(!runisdnterminalfinder){
        err = GetLastError();
        printf("error GetProcAddress : %d\n",err);
    }
    return err;
}

int main(int argc, char * argv[]){
    HWND hwnd;
    char c = '0';
    MSG msg = {0};
    hwnd = GetConsoleWindow();
//  ShowWindow(hwnd,SW_HIDE);
    
    runserver(NULL);
    loaddll();
    iniTrunisdnterminalfinder();
    runisdnterminalfinder();
    while((c=getchar())!='q'){
    }
    unloaddll();
    return 0;
}