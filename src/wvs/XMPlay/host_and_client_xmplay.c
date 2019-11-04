
#include <windows.h>
#include <commctrl.h>
//#include <malloc.h>
//#include <math.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>
#include <ddeml.h>


#if defined (BUILDING_DLL)
#include "../../wa.c"
#else
typedef unsigned char ubyte;
#endif

#include "xmpdsp.h"
#include "../winamp.h"
#include "../net.h"
#include "../config.h"
#include "../plugin_about.h"
#include "dde.c"

#if defined (BUILDING_DLL)
int sendString (TMP *mp, TMPCMD *cmd, void *text, size_t len);
int sendNullString (TMP *mp, TMPCMD *cmd);
#include "xmplayhost.c"
#include "../server.c"

static int initWVSOnce = 0;
static TWINAMP *wvs = NULL;
static HANDLE hThread = NULL;


#else
#include "xmplayhost.c"
#include "../server.h"
#endif


static XMPFUNC_MISC *xmpfmisc = NULL;

WNDPROC xmpWndProc = NULL;
static TMP mpp;
static int TimerID1S = 0;


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



XMPDSP dsp = {
	0, // doesn't support multiple instances
	"WVS host. mylcd.sourceforge.net",
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


static void sendCurentTrackDetails (TMP *mp)
{
	if (mp->sendLock == NULL)
		return;
	
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_CURRENTTRKINFO;

	if (WaitForSingleObject(mp->sendLock, 100) == WAIT_OBJECT_0){
		if (mp->net.serverState && mp->net.clientState)
			xmp_sendGeneralTrackInfo(mp, &cmd);
		ReleaseSemaphore(mp->sendLock, 1, NULL);
	}
	return;
}

void (CALLBACK IntervalTimer)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (!mpp.hwndParent){
		mpp.hwndParent = xmpfmisc->GetWindow();
		
		//switch to visual display mode
		exeDDECmd("key259");
		exeDDECmd("key260");
	}

	if (mpp.net.socket.server > 0 && mpp.net.serverState)
		sendCurentTrackDetails(&mpp);
}


#if 0
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
	plugin_about(&mpp, "WVS: xmp-wvs.dll");
}

// new DSP instance
void *WINAPI DSP_New ()
{
	mpp.hwndParent = xmpfmisc->GetWindow();

	return (void*)1; // no multi-instance, so just return anything not 0
}

// free DSP instance
void WINAPI DSP_Free (void *inst)
{
	//printf("DSP_Free()\n");
	
	if (xmpWndProc)
		SetWindowLong(xmpfmisc->GetWindow(), GWL_WNDPROC, (LONG)xmpWndProc);
	xmpWndProc = NULL;
}

// get description for plugin list
const char *WINAPI DSP_GetDescription (void *inst)
{
	//printf("DSP_GetDescription()\n");
	return dsp.name;
}

// show config options
void WINAPI DSP_Config (void *inst, HWND win)
{
	//printf("DSP_Config()\n");
	plugin_about(&mpp, "WVS: xmp-wvs.dll");
}

// get DSP config
DWORD WINAPI DSP_GetConfig (void *inst, void *config)
{
	//printf("DSP_GetConfig()\n");
	return 0; // return size of config info
}

// set DSP config
BOOL WINAPI DSP_SetConfig (void *inst, void *config, DWORD size)
{
	//printf("DSP_SetConfig()\n");
	return 0;
}

// reset DSP when seeking
void WINAPI DSP_Reset (void *inst)
{
	//printf("DSP_Reset()\n");
	return;
}

// new track has been opened (or closed if file is NULL)
void WINAPI DSP_NewTrack (void *inst, const char *file)
{
	
	//if (file != NULL){
		//printf("DSP_NewTrack() \"%s\"\n",file);
	//}
	/*	
	char *tag = NULL;
	int index = 0;
	do{
		tag = xmpfmisc->GetTag(index);
		if (tag)
			printf("%i #%s#\n",index, tag);
		index++;
	}while(tag);
		*/
	return;
}

// set the sample format at start (or end if rate/chans = 0) of playback
void WINAPI DSP_SetFormat (void *inst, const XMPFORMAT *form)
{
	//printf("DSP_SetFormat()\n");
	return;
}

// process the sample data
DWORD WINAPI DSP_Process (void *inst, float *buffer, DWORD count)
{
	return count;

}

int initMPP (TMP *mp)
{
	memset(mp, 0, sizeof(TMP));
	mp->plugin = NULL;
	mp->hwndParent = xmpfmisc->GetWindow();
	mp->net.socket.client = SOCKET_ERROR;
	mp->net.socket.server = SOCKET_ERROR;
	if (mp->wavedataLock == NULL){
		mp->wavedataLock = CreateSemaphore(NULL, 1, 1, NULL);
		if (mp->wavedataLock == NULL){
			//MessageBox(mp->hwndParent, "Unable to create data lock.\nA restart is required.", "xmp-wvs.dll", MB_OK);
			return 1;
		}
	}

	if (mp->sendLock == NULL){
		mp->sendLock = CreateSemaphore(NULL, 1, 1, NULL);
		if (mp->sendLock == NULL){
			//MessageBox(mp->hwndParent, "Unable to create sendLock.\nA restart is required.", "xmp-wvs.dll", MB_OK);
			return 1;
		}
	}

	mp->buffer.WChar = (wchar_t *)calloc(sizeof(wchar_t), SENDBUFFERSIZE/sizeof(wchar_t));
	if (mp->buffer.WChar == NULL)
		return 0;
		
	TimerID1S = (int)timeSetEvent(1000, 100, IntervalTimer, 1, TIME_PERIODIC);

	return 1;
}

void clientStart ()
{
#if defined (BUILDING_DLL)
	
	if (initWVSOnce)
		return;
	else
		initWVSOnce = 1;

	if (wvs == NULL)
		wvs = (TWINAMP *)calloc(1, sizeof(TWINAMP));
	hThread =(HANDLE)_beginthread((void*)DoWVS, 0, (void*)wvs);

#endif
}

void clientKill ()
{
#if defined (BUILDING_DLL)

	if (!initWVSOnce)
		return;
		
	if (wvs && hThread){
		wvs->state = 1;
		WaitForSingleObject(hThread, 5000);
		free(wvs);
	}
	wvs = NULL;
	hThread = NULL;
	initWVSOnce = 0;
#endif
}

void quit ()
{
	if (TimerID1S)
		timeKillEvent(TimerID1S);
	TimerID1S = 0;	


	clientKill();
	/*
	HWND hwin = FindWindow(NULL, "you_shouldn't_see_me");
	if (hwin != NULL)
		PostMessage(hwin, 8000, 0, 0);
	*/

	if (WaitForSingleObject(mpp.wavedataLock, 2000) == WAIT_OBJECT_0){
		serverShutdown(&mpp);
		Sleep(50);
		if (mpp.wavedataLock)
			CloseHandle(mpp.wavedataLock);
		mpp.wavedataLock = NULL;
	}

	if (mpp.sendLock)
		CloseHandle(mpp.sendLock);
	mpp.sendLock = NULL;

	if (mpp.buffer.WChar != NULL)
		free(mpp.buffer.WChar);
	mpp.buffer.WChar = NULL;	
}


__declspec(dllexport) XMPDSP *WINAPI XMPDSP_GetInterface2 (DWORD face, InterfaceProc faceproc)
{
	//printf("XMPDSP_GetInterface2() %i\n", face);

	if (face != XMPDSP_FACE)
		return NULL;
	
	xmp_status = (XMPFUNC_STATUS*)faceproc(XMPFUNC_STATUS_FACE); // import "status" functions
	xmpfmisc = (XMPFUNC_MISC*)faceproc(XMPFUNC_MISC_FACE); // import "misc" functions
	
	initMPP(&mpp);
	serverStart(&mpp);
	clientStart();
	return &dsp;
}

__declspec(dllexport) BOOL APIENTRY DllMain (HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			//initConsole();
			//clientStart();
			break;
		case DLL_PROCESS_DETACH:
			quit();
			break;
	}
	return 1;
}

__declspec (dllexport) void *getMPStructPtr ()
{
	return (void *)&mpp;
}
