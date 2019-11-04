
// myLCD
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2008  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#include <windows.h>
#include <conio.h>
#include <fcntl.h>

#include <mylcd.h>

#include "../winamp.h"
#include "../net.h"
#include "../config.h"
#include "../plugin_about.h"
#include "../addler.c"


#if (BUILD_MEDIAMONKEY)
int OpenSDB(TMP *mp);
# include "../mm/mmcom.c"
# include "../mm/mmhost.c"
#endif

#if (BUILD_SVP)
# include "../aimp/amphost.c"
#endif

#include "../server.h"



void config ();
int init ();
void quit ();
winampGeneralPurposePlugin plugin = {GPPHDR_VER, "WVS: mylcd.sourceforge.net", init, config, quit, 0, 0};
LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcPE (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void sendCurentTrackDetails (TMP *mp);
static void sendIPC (TMP *mp, LPARAM ipc, WPARAM wParam);

WNDPROC waWndProc = NULL;
WNDPROC waWndProcPE = NULL;
static TMP mpp;
HWND hPlaylist = NULL;
static HINSTANCE hInstance;
static char szPath[MAX_PATH];
static int TimerID1S = 0;

static HWND mlhwin = NULL;
static int mlCloseRetry = 0;


__declspec (dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin ()
{
	return &plugin;
}


__declspec (dllexport) void *getMPStructPtr ()
{
	return (void *)&mpp;
}


char *getWinampPath ()
{	
	memset(szPath, 0, sizeof(szPath));
	GetModuleFileName(hInstance, szPath, sizeof(szPath)-sizeof(char));
	//printf("#%s#\n",szPath);
	return szPath;
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
#else
void initConsole (){}
#endif

void config ()
{
	plugin_about(&mpp, "WVS host: gen_wamp.dll");
}

static HANDLE hMapFile = NULL;


void map()
{
#if (BUILD_MEDIAMONKEY)
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x40000, (LPCTSTR)"MM_VISUALIZATION_DATA");
	if (hMapFile == NULL){
		MessageBox(0, "WVS: Unable to create file mapping object", "WVS: CreateFileMapping()", MB_OK);

	}else{
		mpp.mm.visMap = (ubyte *)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0,0,0);
		if (mpp.mm.visMap == NULL){
			MessageBox(0, "WVS: Unable to map file", "WVS: MapViewOfFile()", MB_OK);
		}else{
			mpp.mm.visMap += 35; // vis data offset
		}
	}
#endif
}


static void startWVS (TMP *mp, char *wvsmodule)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	
	_splitpath(getWinampPath(), drive, dir, NULL, NULL);
	sprintf(szPath, "%s%s", drive, dir);
	//printf("-%s-\n", szPath);
	SetCurrentDirectory(szPath);

	char path[2][4096];
	sprintf(path[0], "%s\\wvs\\", szPath);
	sprintf(path[1], "%s%s", path[0], wvsmodule);
	ShellExecute(mp->hwndParent, "open", path[1], "", path[0], SW_SHOWNORMAL);
	//printf("#%s#\n#%s#\n",path[0],path[1]);
}

#if (BUILD_MEDIAMONKEY)

int OpenSDB(TMP *mp)
{
	mp->mm.pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (mp->mm.pRoot){
		mp->mm.pPlayer = SetTable(mp->mm.pRoot, L"Player");	
		if (mp->mm.pPlayer){
			return 1;
		}else{
			CloseConnection();
			return 0;
		}
	}

	return 0;
}
#endif


void (CALLBACK IntervalTimer)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (mpp.Shuttingdown)
		return;

	if (mpp.net.socket.server > 0){
		sendCurentTrackDetails(&mpp);

	if (mlCloseRetry < 6){
		mlCloseRetry++;
		//if (mlhwin == NULL){
			mlhwin = FindWindow(NULL, "MiniLyrics (Evaluation Version)");
			if (mlhwin != NULL)
				PostMessage(mlhwin, WM_CLOSE, 0, 0);
			Sleep(10);
		//}
	}

#if (!BUILD_WINAMP)
		static int cacheListLength = 0;
		static uint32_t firstTrackAddler = 0;

		// has playlist been modified? if so send a notification
		if (cacheListLength != mpp.totalTracks){
			cacheListLength = mpp.totalTracks;
			sendIPC(&mpp, IPC_PLAYLIST_MODIFIED, 0);
		}else{
			// should verify at least two tracks are the same
			uint8_t *path = (uint8_t*)SendMessage(mpp.hwndParent, WM_WA_IPC, 0, IPC_GETPLAYLISTFILE);
			if (path){
				uint32_t addler = getAddler(path, strlen(path));
				if (firstTrackAddler != addler){
					if (firstTrackAddler)// don't clear the playlist at startup
						sendIPC(&mpp, IPC_PLAYLIST_MODIFIED, 0);
					firstTrackAddler = addler;
				}
			}
		}
#endif
	}
}

int init ()
{
	memset(&mpp, 0, sizeof(mpp));
	mpp.plugin = &plugin;
	mpp.hwndParent = plugin.hwndParent;
	mpp.net.socket.client = SOCKET_ERROR;
	mpp.net.socket.server = SOCKET_ERROR;
	if (mpp.wavedataLock == NULL){
		mpp.wavedataLock = CreateSemaphore(NULL, 1, 1, NULL);
		if (mpp.wavedataLock == NULL){
			//MessageBox(mpp.hwndParent, "Unable to create data lock.\nA restart is required.", "gen_wamp.dll", MB_OK);
			return 1;
		}
	}

	if (mpp.sendLock == NULL){
		mpp.sendLock = CreateSemaphore(NULL, 1, 1, NULL);
		if (mpp.sendLock == NULL){
			//MessageBox(mpp.hwndParent, "Unable to create sendLock.\nA restart is required.", "gen_wamp.dll", MB_OK);
			return 1;
		}
	}

	mpp.quitEvent = CreateEvent(NULL, 0, 0, "WVSQUITEVENT");
	//printf("gen_wamp: quitEvent %p\n",mpp.quitEvent);

	mpp.buffer.WChar = (wchar_t *)calloc(sizeof(wchar_t), SENDBUFFERSIZE/sizeof(wchar_t));
	if (mpp.buffer.WChar == NULL)
		return 0;
	
	map();
	serverStart(&mpp);
	startWVS(&mpp, WVSMODULE);

	// force vis_wamp.dll to load
	if ((int)SendMessage(mpp.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH)){
		if (!SendMessage(mpp.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING)){
			SendMessage(mpp.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2,0),0);	// play
			Sleep(20);
			int pos = (int)SendMessage(mpp.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
			char *track = (char*)SendMessage(mpp.hwndParent, WM_WA_IPC, pos, IPC_GETPLAYLISTFILE);
			if (track != NULL){
				if (strstr(track, "://")){
					PostMessage(mpp.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON3,0),0); // pause
					//Sleep(500);
				}else{
					SendMessage(mpp.hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON4,0),0); // stop
				}	
				Sleep(500);
			}	
		}
	
	  #if (BUILD_WINAMP)
		HMODULE hModule = GetModuleHandle("vis_wamp.dll");
		if (hModule == NULL)
			hModule = GetModuleHandle("plugins\\vis_wamp.dll");	
		if (hModule == NULL)
			hModule = GetModuleHandle("vis_wvs.svp");	
		if (hModule == NULL)
			hModule = GetModuleHandle("plugins\\vis_wvs.svp");	
			
		if (hModule == NULL){
			MessageBox(mpp.hwndParent, "vis_wamp.dll not loaded.\n\nVerify WVS (vis_wamp.dll) is the active visualization plugin.\n"\
			  "In Winamp's plug-in preferences ensure 'Auto execute visualization plug-in on playback' "\
			  "is selected.\n\ngen_wamp.dll can continue but visualization data may be unavailable."\
			  "\n\nIf you are not running Winamp then the incorrect plugin .dll's have been installed.",\
			  "WVS "MY_VERSIONSTR": gen_wamp.dll", MB_OK);
		}
	  #endif
	}
	
	TimerID1S = (int)timeSetEvent(1000, 100, IntervalTimer, 1, TIME_PERIODIC);
	waWndProc = (WNDPROC)SetWindowLong(mpp.hwndParent, GWL_WNDPROC, (LONG)WndProc);
	hPlaylist = (HWND)SendMessage(mpp.hwndParent, WM_WA_IPC, IPC_GETWND_PE, IPC_GETWND);
	waWndProcPE = (WNDPROC)SetWindowLong(hPlaylist, GWL_WNDPROC, (LONG)WndProcPE);

	// auto start/load Vis
#if (BUILD_MEDIAMONKEY)
	keybd_event(VK_CONTROL, 0, 0, 0);
	keybd_event(VK_MENU, 0, 0, 0);
	keybd_event('V', 0, 0, 0);
	Sleep(20);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
	keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
#endif
	return 0;
}

static int processWindowMessages ()
{
	MSG messages;
	int ret = 0;
	
	//if ((ret = GetMessage(&messages, NULL, 0, 0)) > 0){
	if ((ret = PeekMessage(&messages, NULL, 0, 0, PM_REMOVE)) > 0){
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
	return ret;
}

void quit ()
{
	//printf("gen_wamp() quit\n");
	
	if (TimerID1S)
		timeKillEvent(TimerID1S);
	TimerID1S = 0;
		
	//NET_SEND_LOCK();
	//NET_READ_LOCK();

	HWND hwin = FindWindow(NULL, "you_shouldn't_see_me");
	if (hwin != NULL)
		PostMessage(hwin, 8000, 0, 0);

	if (waWndProc)
		SetWindowLong(mpp.hwndParent, GWL_WNDPROC, (LONG)waWndProc);

	if (waWndProcPE)
		SetWindowLong(hPlaylist, GWL_WNDPROC, (LONG)waWndProcPE);


#if (BUILD_MEDIAMONKEY)
	mpp.net.serverState = 0;
	mpp.net.clientState = 0;
	mpp.Shuttingdown = 1;
	
	/*
	while(processWindowMessages() > 0){
		Sleep(10);
	}
	*/
#endif

	if (WaitForSingleObject(mpp.wavedataLock, 60000) == WAIT_OBJECT_0){
		serverShutdown(&mpp);
		
	//HWND hwin = FindWindow(NULL, "you_shouldn't_see_me");
	//if (hwin != NULL)
	//	PostMessage(hwin, 8000, 0, 0);
		
#if (BUILD_MEDIAMONKEY)
		if (hMapFile)
			CloseHandle(hMapFile);
		hMapFile = NULL;
#endif

		Sleep(100);
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

static void sendIPC (TMP *mp, LPARAM ipc, WPARAM wParam)
{
	if (mp->sendLock == NULL)
		return;

	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_HOSTIPC;
	cmd.data1 = (unsigned int)ipc;
	cmd.data2 = (unsigned int)wParam;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (WaitForSingleObject(mpp.sendLock, 200) == WAIT_OBJECT_0){
		if (mp->net.serverState)
			sendSocket(&mp->net, &cmd, &cmdLen);
		ReleaseSemaphore(mpp.sendLock, 1, NULL);
	}
	return;
}

void sendCurentTrackDetails (TMP *mp)
{
	if (mp->sendLock == NULL)
		return;
	
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_CURRENTTRKINFO;

	if (WaitForSingleObject(mp->sendLock, 100) == WAIT_OBJECT_0){
		if (mp->net.serverState && mp->net.clientState && !mp->Shuttingdown)
			sendGeneralTrackInfo(mp, &cmd);
		ReleaseSemaphore(mp->sendLock, 1, NULL);
	}
	return;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (waWndProc)
		ret = CallWindowProc(waWndProc, hWnd, uMsg, wParam, lParam);

	if (!mpp.net.serverState)
		return ret;
		
		
	//if (uMsg != 132 && uMsg != 32 && uMsg != 160 /*&& uMsg != 1024*/)
		//printf("wndProc: %i %i %i\n",uMsg, lParam, wParam);
	
	if (uMsg == WM_CLOSE){
		if (TimerID1S)
			timeKillEvent(TimerID1S);
		TimerID1S = 0;
		mpp.Shuttingdown = 1;
	}else if (uMsg == WM_WA_IPC){
		//if (wParam != IPC_CB_MISC_INFO && lParam != IPC_CB_MISC && lParam != IPC_GETPLAYLISTFILEW && lParam != 124 && lParam != 125 && lParam != 126 && lParam != 105)
			//printf("ipc: %i %i\n",lParam, wParam);
		
   		if (lParam == IPC_CB_MISC){
   			if (wParam == IPC_CB_MISC_TITLE || wParam == IPC_CB_MISC_STATUS)
   				sendIPC(&mpp, lParam, wParam);
   		}else if (lParam == IPC_FILE_TAG_MAY_HAVE_UPDATEDW){
   			sendIPC(&mpp, lParam, wParam);
   		}else if (lParam == 891){
			sendIPC(&mpp, IPC_PLAYLIST_MODIFIED, 0);
		}
	}else{
		//if (uMsg != WM_TIMER && uMsg != 132 && uMsg != 32 && uMsg != 160)
			//printf("other: %i %i %i\n",uMsg, wParam, lParam);
		
		/*
		if (uMsg == WM_TIMER){
			if (wParam == 64){	// 1 sec interval timer
				sendCurentTrackDetails(&mpp);
			//	printf("other: %i %i %i\n",uMsg, wParam, lParam);
			}
		}
		*/
	}
	return ret;
}

LRESULT CALLBACK WndProcPE (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if (waWndProcPE)
		ret = CallWindowProc(waWndProcPE, hWnd, uMsg, wParam, lParam);
	if (!mpp.net.serverState)
		return ret;
		
	if (uMsg == WM_WA_IPC){
		//if (wParam != 109)
			//printf("pe: %i %i\n",wParam, lParam);
		
		if (wParam == 666)	// playback started
			sendIPC(&mpp, IPC_PLAYING_FILEW, lParam&0xFFFFFFF);

	}else if (uMsg == WM_NCDESTROY || uMsg == WM_QUIT){		// Winamp is shutting down
		mpp.net.clientState = mpp.net.serverState = 0;
	}

	return ret;
}


__declspec(dllexport) BOOL APIENTRY DllMain (HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			initConsole();
			hInstance = hDLL;
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return 1;
}


