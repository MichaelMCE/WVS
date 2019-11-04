
#include <windows.h>
#include <commctrl.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>

#include "xmpdsp.h"
#include "../../wa.c"

#include <signal.h>


volatile int initWVSOnce = 0;
volatile TWINAMP *wvs = NULL;
volatile HANDLE hThread = NULL;
volatile __int64 startTick = 0;

static HINSTANCE hInstance;
static wchar_t szPath[MAX_PATH];


void *WINAPI DSP_New ();
void WINAPI DSP_Free (void *inst);
const char *WINAPI DSP_GetDescription (void *inst);
void WINAPI DSP_Config (void *inst, HWND win);
DWORD WINAPI DSP_GetConfig (void *inst, void *config);
BOOL WINAPI DSP_SetConfig (void *inst, void *config, DWORD size);
void WINAPI DSP_NewTrack (void *inst, const char *file);
void WINAPI DSP_SetFormat (void *inst, const XMPFORMAT *form);
void WINAPI DSP_Reset (void *inst);
DWORD WINAPI DSP_Process (void *inst, float *srce, DWORD count);
void WINAPI DSP_About (HWND win);



void clientKill();
void clientStart();


XMPDSP dsp = {
	0, // doesn't support multiple instances
	"WVS client. mylcd.sourceforge.net",
	DSP_About,
	DSP_New,
	DSP_Free,
	DSP_GetDescription,
	DSP_Config,
	DSP_GetConfig,
	DSP_SetConfig,
	DSP_NewTrack,
	DSP_SetFormat,
	DSP_Reset,
	DSP_Process
};


#if 1
void initConsole ()
{
	if (AllocConsole()){
		int hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
		if (hCrt){
			FILE *hf = fdopen(hCrt, "wr");
			if (hf){
				*stdout = *hf;
				setvbuf(stdout, NULL, _IONBF, 0);
			}else{
				FreeConsole();
			}
		}else{
			FreeConsole();
		}
	}
}
#endif

void WINAPI DSP_About (HWND win)
{
	//plugin_about(&mpp, "WVS: xmp-wvsclient.dll");
}

volatile static unsigned int threadID = 0;

// new DSP instance
void *WINAPI DSP_New ()
{
	printf("DSP_New()\n");
	Sleep(50);
		
	if (GetTickCount()-startTick > 1000)
		clientStart();
	return (void*)0; // no multi-instance, so just return anything not 0
}

void WINAPI DSP_Free (void *inst)
{
	printf("DSP_Free()\n");
	Sleep(50);	
	//if (GetTickCount()-startTick > 1000)
		//clientKill();
		
}

// get description for plugin list
const char *WINAPI DSP_GetDescription (void *inst)
{
	printf("DSP_GetDescription()\n");
	Sleep(50);
	return dsp.name;
}

// show config options
void WINAPI DSP_Config (void *inst, HWND win)
{
	printf("DSP_Config()\n");
//	plugin_about(&mpp, "WVS: xmp-wvs.dll");
}

// get DSP config
DWORD WINAPI DSP_GetConfig (void *inst, void *config)
{
	printf("DSP_GetConfig()\n");
	return 0; // return size of config info
}

// set DSP config
BOOL WINAPI DSP_SetConfig (void *inst, void *config, DWORD size)
{
	printf("DSP_SetConfig()\n");
	return 0;
}

// reset DSP when seeking
void WINAPI DSP_Reset (void *inst)
{
	printf("DSP_Reset()\n");
	return;
}

// new track has been opened (or closed if file is NULL)
void WINAPI DSP_NewTrack (void *inst, const char *file)
{

	return;
}

// set the sample format at start (or end if rate/chans = 0) of playback
void WINAPI DSP_SetFormat (void *inst, const XMPFORMAT *form)
{
	printf("DSP_SetFormat()\n");
	return;
}

// process the sample data
DWORD WINAPI DSP_Process (void *inst, float *buffer, DWORD count)
{
	return count;
}

static wchar_t *getPathW ()
{	
	memset(szPath, 0, sizeof(szPath));
	GetModuleFileNameW(hInstance, szPath, sizeof(szPath)-sizeof(wchar_t));
	wprintf(L"#%s#\n",szPath);
	return szPath;
}

void _exit_ ()
{
	printf("_exit_() in\n");
	clientKill();
	printf("_exit_() out\n");
}

void clientStart ()
{
	if (wvs) return;

	wchar_t drive[_MAX_DRIVE*sizeof(wchar_t)];
	wchar_t dir[_MAX_DIR*sizeof(wchar_t)];
		
	printf("clientStart() %i %p\n", initWVSOnce, wvs);
	
	if (!initWVSOnce)
		initWVSOnce = 1;
	else
		return;

	atexit(_exit_);

	if (wvs == NULL)
		wvs = (TWINAMP *)calloc(1, sizeof(TWINAMP));
		
		
	_wsplitpath(getPathW(), drive, dir, NULL, NULL);
	swprintf(szPath, L"%s%s", drive, dir);
	wprintf(L"-%s-\n", (wchar_t*)szPath);
	SetCurrentDirectoryW(szPath);
		
	wvs->state = 0;
	wvs->wvsPath = szPath;
	unsigned int threadID = 0;
	hThread = (HANDLE)_beginthreadex(NULL, 0, (void*)DoWVS, (void*)wvs, 0, &threadID);
}

void clientKill ()
{
	if (!initWVSOnce)
		return;

	printf("clientKill()\n");
	
	if (wvs){
		wvs->state = 1;
		if (hThread){
			printf("waiting for WVS thread\n");
			WaitForSingleObject(hThread, 5000);
			printf("waiting for WVS thread done\n");
			CloseHandle(hThread);
		}else{
			Sleep(2000);
		}
		free((void*)wvs);
	}
	wvs = NULL;
	hThread = NULL;
	initWVSOnce = 0;
}

__declspec(dllexport) XMPDSP *WINAPI XMPDSP_GetInterface2 (DWORD face, InterfaceProc faceproc)
{
	//printf("XMPDSP_GetInterface2() %i\n", face);

	if (face != XMPDSP_FACE)
		return NULL;

	startTick = GetTickCount();
	//clientStart();
	return &dsp;
}

__declspec(dllexport) BOOL APIENTRY DllMain (HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			hInstance = hDLL;
			initConsole();
			break;
		case DLL_PROCESS_DETACH:
		//	clientKill();
			break;
	}
	return 1;
}
