#include <windows.h>
#include <stdio.h>
#include "resources.h"

#define WM_MYICONNOTIFY     WM_USER+5
#define CLASS_NAME "CallServerWindow"
#define CLASS_NAME2 "CallServerWindowContext1"

typedef int (APIENTRY RUNISDNTERMINALFINDER) ();

RUNISDNTERMINALFINDER* runisdnterminalfinder;

HINSTANCE h;
int err = 0;
HINSTANCE instance = NULL;
HWND    hWnd;
HWND        conshwnd;
HDC     hDC;
BOOL    fullscreen;
unsigned char bpp;
BOOL        selectMode = FALSE;
HMODULE modulehndl;
HWND hWndContext;

int isdnfinderstate = 0;

extern int _stdcall runserver(PVOID pvParam);
extern int _stdcall runserialterminalfinder();
extern int _stdcall stopserialterminalfinder();
extern int _stdcall runclient(void * f1);

BOOL initWnd(int width, int height,unsigned char bpp, int top, int left);
int mainLoop();
LONG WINAPI mainWndProc (HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM  lParam);
void terminateWnd();
void RollToTray(HWND hwnd);
void OnIcon(HWND hwnd, WPARAM wp, LPARAM lp);
int loaddll();
int unloaddll();
int iniTrunisdnterminalfinder();
void ErrorShow(char * text);
void contextmenu();
void logger(const char *fmt, ...);

int main(int argc, char * argv[])
{
    HICON hIcon1;
    if(NULL != FindWindow(CLASS_NAME, "CallServerWindow.")){
        MessageBox(0,"Ýòà ïðîãðàììà óæå çàïóùåíà!!!",0,MB_ICONERROR);
        return 1;
    }
    logger("CALLSRVWND STARTED\n");
    conshwnd = GetConsoleWindow();
    modulehndl = GetModuleHandle(NULL);
    initWnd(200,50,32,300,400);
    runserver(NULL);
    hIcon1 = LoadIcon(modulehndl, MAKEINTRESOURCE(IDI_ICON1));
    DrawIcon(hDC, 10, 20, hIcon1); 
    loaddll();
    iniTrunisdnterminalfinder();
    runisdnterminalfinder();
    runserialterminalfinder();
    runclient(NULL);
    mainLoop();
    stopserialterminalfinder();
    unloaddll();
    printf("ª®­¥æ ¯à®£à ¬¬ë.\n");
    getchar();
    return 0;
}

BOOL initWnd(int width, int height,unsigned char bpp, int top, int left)
{
    WNDCLASS                wc;
    RECT                    WindowRect;
    DWORD                   dwStyle = WS_OVERLAPPEDWINDOW;

    fullscreen = fullscreen;

    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_VREDRAW|CS_HREDRAW|CS_OWNDC;
    wc.lpfnWndProc = mainWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);//IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(COLOR_WINDOW+1);
    wc.lpszClassName = CLASS_NAME;
    wc.lpszMenuName = CLASS_NAME;
        RegisterClass(&wc);
        
    WindowRect.left=0;
    WindowRect.right=width;
    WindowRect.top=0;
    WindowRect.bottom=height;

    AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindow(CLASS_NAME, "CallServerWindow.", 
        WS_CAPTION|WS_CLIPSIBLINGS|WS_SYSMENU|WS_OVERLAPPED|WS_MINIMIZEBOX,
        //WS_POPUPWINDOW,
        //WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE,
        //dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE , 
        CW_USEDEFAULT, CW_USEDEFAULT, 
        WindowRect.right-WindowRect.left, 
        WindowRect.bottom-WindowRect.top, 
        NULL, NULL, wc.hInstance, NULL);

    bpp=bpp;
    
    //SetWindowPos(hWnd, HWND_TOP, left, top, width, height, SWP_NOSIZE);
    //ShowWindow(hWnd, SW_SHOW);
    //SetForegroundWindow(hWnd);

    return TRUE;
}

int mainLoop()
{
    MSG msg;
    BOOL bRet;

    while((bRet = GetMessage(&msg,NULL,0,0))!=0){
       if (bRet == -1){
          // handle the error and possibly exit
          printf("GetMessage ¢ £« ¢­®¬ æ¨ª«¥ ¢¥à­ã« -1\n");
       }else{
          TranslateMessage( &msg );
          DispatchMessage( &msg );
       }
    }
          
    terminateWnd();
    return 0;
}

LONG WINAPI mainWndProc (HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM  lParam)
{
        int wmId, wmEvent;
        POINT pnt;
        HMENU hMenu;
              
    switch(uMsg){
    case WM_CREATE:
        hDC = GetDC(hWnd);
        RollToTray(hWnd);
        break;
        
        case WM_SHOWWINDOW:
                break;
            
    case WM_KEYDOWN:
        switch(wParam){
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        default:
            break;
        }
        switch(HIWORD(lParam)){
        case 0x001F:
            break;
        }
        break;

    case WM_LBUTTONUP:
        //printf("WM_LBUTTONUP\n");
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    
    case WM_SIZE:
            //printf("WM_SIZE, wParam= %d, lParam= %d\n",wParam,lParam);
            switch(wParam){
                case SIZE_MINIMIZED:
                         RollToTray(hWnd);
                         break;
                }
        break;
    
    case WM_SYSCOMMAND:
                /*switch(wParam){
                case SC_MINIMIZE:
                         return 0;
                }*/
            break;
        case WM_COMMAND:
                        wmId    = LOWORD(wParam);
                        wmEvent = HIWORD(wParam);

                        switch (wmId) {
                        case IDM_ABOUT:
                                printf("IDM_ABOUT\n");
                                break;
                        }
                break;      

        case WM_RBUTTONDOWN: 
                pnt.x = LOWORD(lParam);
                pnt.y = HIWORD(lParam);
                ClientToScreen(hWnd, (LPPOINT) &pnt);
                hMenu = GetSubMenu (GetMenu (hWnd), 1);
                if (hMenu) {
                        TrackPopupMenu (hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
                } else {
                // Couldn't find the menu...
                        MessageBeep(0);
                }
                printf("WM_RBUTTONDOWN\n");
                break;

    case WM_MYICONNOTIFY:
            OnIcon(hWnd, wParam, lParam);
            break;          
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void terminateWnd()
{
    ReleaseDC(hWnd, hDC);
    if(fullscreen){
        ChangeDisplaySettings(NULL, 0);
    }
    DestroyWindow(hWnd);
}

///////////////////////////á¢¥à­ãâì ®ª­® ¢ âàí©////////////////////////////////
void RollToTray(HWND hwnd)
{
    NOTIFYICONDATA nf;
    HICON hIcon;
    nf.hWnd = hwnd;
    nf.uID = 0;
    nf.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nf.uCallbackMessage = WM_MYICONNOTIFY;
    strcpy(nf.szTip,"Çâîíêîâûé ñåðâåð");
    hIcon=LoadIcon(modulehndl, MAKEINTRESOURCE(IDI_ICON1));
    nf.hIcon = hIcon;
    Shell_NotifyIcon(NIM_ADD,&nf);
    ShowWindow(conshwnd, SW_HIDE);
    ShowWindow(hwnd, SW_HIDE);
}
//////////////////////////‚¥à­ãâì ¨§ âà¥ï//////////////////////////////////////
void OnIcon(HWND hwnd, WPARAM wp, LPARAM lp)
{
  if(lp == WM_LBUTTONUP)
  {
    NOTIFYICONDATA nf;
    nf.hWnd = hwnd;
    nf.uID = 0;
    nf.uFlags = NIF_ICON;
    nf.uCallbackMessage = 0;
    nf.hIcon = NULL;
    Shell_NotifyIcon(NIM_DELETE,&nf);
    ShowWindow(conshwnd, SW_SHOW);
    BringWindowToTop(conshwnd);
    SetActiveWindow(conshwnd);
    SetFocus(conshwnd);
    SetWindowPos(hwnd, HWND_TOP, 100, 100, 100, 50, SWP_NOSIZE);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    SetForegroundWindow(hwnd);
  }
  if(lp == WM_RBUTTONDOWN){
    printf("WM_RBUTTONDOWN\n");
    contextmenu();
  }
}
///////////////////////////////////////////////////////////////////////////////
LONG WINAPI contextWndProc (HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM  lParam)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
    return 0;
}

void contextmenu()
{
/*  WNDCLASS                wc;
    RECT                    WindowRect;
    DWORD                   dwStyle = WS_OVERLAPPEDWINDOW;

    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_VREDRAW|CS_HREDRAW|CS_OWNDC;
    wc.lpfnWndProc = contextWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(COLOR_WINDOW+1);
    wc.lpszClassName = CLASS_NAME2;

    if(0 != RegisterClass(&wc)){
        
        WindowRect.left=0;
        WindowRect.right=100;
        WindowRect.top=0;
        WindowRect.bottom=100;

        AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

        hWndContext = CreateWindow(CLASS_NAME2, "CallServerWindowContext.", 
                //WS_CAPTION|WS_CLIPSIBLINGS|WS_SYSMENU|WS_OVERLAPPED|WS_MINIMIZEBOX,
                //WS_POPUPWINDOW,
                WS_DLGFRAME,
                //WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE,
                //dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE , 
                CW_USEDEFAULT, CW_USEDEFAULT, 
                WindowRect.right-WindowRect.left, 
                WindowRect.bottom-WindowRect.top, 
                NULL, NULL, wc.hInstance, NULL);

        SetWindowPos(hWndContext, HWND_TOP, 100, 100, 200, 200, SWP_NOSIZE);
        ShowWindow(hWndContext, SW_SHOW);
        SetForegroundWindow(hWndContext);
    }else{
        ErrorShow("contextmenu create error\n");
    }*/
        
    return;
}
///////////////////////////////////////////////////////////////////////////////
int loaddll()
{
    h=LoadLibrary("ISDNFINDER.DLL");
    if(!h){
        ErrorShow("error LoadLibrary isdnfinder.dll");
        exit(1);
    }
    return -1;
}

int unloaddll()
{
    if((err = FreeLibrary(h)) == 0){
        ErrorShow("error FreeLibrary isdnfinder.dll");
        exit(1);
    }
    return err;
}

int iniTrunisdnterminalfinder()
{
    runisdnterminalfinder = (RUNISDNTERMINALFINDER*)GetProcAddress((HMODULE) h, "runisdnterminalfinder");
    if(!runisdnterminalfinder){
        ErrorShow("error GetProcAddress");
        exit(1);
    }
    return err;
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

void logger(const char *fmt, ...)
{
    va_list ap;
    FILE *fp;
    SYSTEMTIME str_t;
    char logname[64] = {0};
    sprintf(logname, "CALLSRVWND.LOG");
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