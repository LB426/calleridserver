extern DWORD WINAPI runserver(PVOID pvParam);
extern void exithread();
extern void ShowParam();
extern void ShowQmsg();
extern void ShowQusr();
extern void ShowQdat();
extern void hVerify();

void ErrorShow(char * text);
void GetDataTime(char *datatime);
void toLog(char * szString);
void toLog(int n);
char * ANomberAnalise(char * anomber);
