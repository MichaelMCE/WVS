
// myLCD - http://mylcd.sourceforge.net/
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <math.h>
#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <objbase.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <wctype.h>
#include <mylcd.h>

#define u64 __int64


#if 0	


#define funcname		__func__
#define linenumber		(__LINE__)

MYLCD_EXPORT void * my_Malloc (size_t size, const char *func, const int line)
	__attribute__((malloc));
MYLCD_EXPORT void * my_Calloc (size_t nelem, size_t elsize, const char *func, const int line)
	__attribute__((malloc));
MYLCD_EXPORT void * my_Realloc (void *ptr, size_t size, const char *func, const int line)
	__attribute__((malloc));
MYLCD_EXPORT void my_Free (void *ptr, const char *func, const int line);
MYLCD_EXPORT char * my_Strdup (const char *str, const char *func, const int line)
	__attribute__((nonnull(1)));
MYLCD_EXPORT wchar_t * my_Wcsdup (const wchar_t *str, const char *func, const int line)
	__attribute__((nonnull(1)));
MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n)
	__attribute__((nonnull(1, 2)));
MYLCD_EXPORT void my_MemStatsDump (THWD *hw);


//#define my_memcpy(s1,s2,n)	my_Memcpy(s1, s2, n)

#define malloc(n)		my_Malloc(n, funcname, linenumber)
#define calloc(n,e)		my_Calloc(n, e, funcname, linenumber)
#define realloc(p,n)	my_Realloc(p, n, funcname, linenumber)
#define free(p)			my_Free(p, funcname, linenumber)
#define strdup(p)		my_Strdup(p, funcname, linenumber)
#define wcsdup(p)		my_Wcsdup(p, funcname, linenumber)

#endif


#include "../src/g15/g15display.h"
#include "demos.h"
#include "wvs/winamp/wa_ipc.h"
#include "wvs/mixer.h"
#include "wvs/net.h"
//#include "wvs/client.h"	// use this (netclient.dll) for shipping
#include "wvs/client.c"		// use this when updating/debugging the file
#include "wvs/1252to88591.h"
#include "wvs/hook/hook.h"
#include "wvs/packethandler.h"
#include "wvs/wvs.h"
#include "wvs/lang.c"
#include "wvs/addler.c"
#include "wvs/packethandler.c"
#include "wvs/cache.c"
#include "wvs/mixer.c"
#include "wvs/g15_hid.c"
#include "wvs/mystify.c"
#include "wvs/glfwparticles.c"
#include "wvs/bandwidth.c"
#include "wvs/proclist.c"
#include "wvs/rssfeed.c"
#include "wvs/sysinfo.c"
#include "wvs/atitray.c"
#include "wvs/calendar.c"
#include "wvs/eq.c"
#include "wvs/fraps.c"
#include "wvs/ping.c"
#include "wvs/about.c"
#include "wvs/port.c"
#include "wvs/amsn/msn.c"
#include "wvs/lyrics/lyrics.c"
#include "wvs/utorrent/ut.c"
#include "wvs/title.c"	// main page
#include "wvs/playlist.c"
#include "wvs/hitlist.c"
#include "wvs/metatag.c"
#include "wvs/config.c"




int startWVS (TWINAMP *wvs, int argc, char* argv[]);
static int initWVS (TWINAMP *wa);
void waCleanup (TWINAMP *wa);
static int configProcessFile (TWINAMP *wa, char *configpathfile);
static int configLoad (TWINAMP *wa, char *configpathfile);
static void doTimedEvents (TWINAMP *wa);
static void initPages (TWINAMP *wa);
static int getCurrentTrackDetails (TWINAMP *wa);
static void closeGUI (TWINAMP *wvs);
static HANDLE initGUI (TWINAMP *wa);
static int processWindowMessages (TWINAMP *wvs);

static int processKeyPress (TWINAMP *wa, unsigned int key);
int handleCommonKeys (TWINAMP *wa, int key, void *data);
DWORD g15keycb (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15);

static void renderOverlays (TWINAMP *wa, int frameid);
void renderFrames (TWINAMP *wa);
static void renderFrame (TWINAMP *wa, int frameid);
static int RefreshDisplay (TWINAMP *wa, TFRAME *frame);
static void updateDisplay (TWINAMP *wa, TFRAME *frame);
static int registerPage (TWINAMP *wa, char *title, TPAGEFUNCS *rpage, void *userPtr, int pageID, int dependsID);
static void registerPages (TWINAMP *wa);
//void forceFrameUpdates (TWINAMP *wa);
static int processSpectrumData (TWINAMP *wa);
static void processOscData (TWINAMP *wa);
static TDISPAGE *getPage (TWINAMP *wa, int frameid);
static void setActivePage (TWINAMP *wa, int disf_frame);
static int getPageLastRenderTime (TWINAMP *wa, int frameid);
char *getPageTitle (TWINAMP *wa, int frameid);
static int getPageFlags (TWINAMP *wa, int frameid);
static int getPageUpdatePeriod (TWINAMP *wa, int frameid);
static int gotoPrevPage (TWINAMP *wa);
static int gotoNextPage (TWINAMP *wa);
static void onPageChange (TWINAMP *wa);
static int defaultPageReset (TWINAMP *wa, int startPage);
static void resetPageSwapTime (TWINAMP *wa);
static int isPageOpen (TWINAMP *wa, int frameid);
static int isPageRegistered (TWINAMP *wa, int frameid);
static void setPageStatus (TWINAMP *wa, int frameid, int status);
static int getPageStatus (TWINAMP *wa, int frameid);
static clock_t getTime ();
static void calculatePageRotation (TWINAMP *wa);
static int requestWaveData (TWINAMP *wa);		// returns true if there was an update
static void volumeIncrease (TWINAMP *wa, int mixer);
static void volumeDecrease (TWINAMP *wa, int mixer);
static void track_togglePlayState (TWINAMP *wa);
static void track_Next (TWINAMP *wa);
static void track_Previous (TWINAMP *wa);
static void track_Stop (TWINAMP *wa);
static void toggle_DancingLights (TWINAMP *wa);
static void pagePanRight (TWINAMP *wa, TFRAME *src, TFRAME *des);
static void effectLeaveBottomUp (TWINAMP *wa, TFRAME *src, TFRAME *des);
static void effectEnterTopDown (TWINAMP *wa, TFRAME *src, TFRAME *des);
static int initG15 (TWINAMP *wa, DWORD ptrG15keyCB (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15));
static void getLedLevel (TWINAMP *wa);
static void setG15LedLevels (TWINAMP *wa);
static void resetG15Leds (TWINAMP *wa);
static void resetBarGraphScale (TWINAMP *wa);
static void closeG15 (TWINAMP *wvs);
static int volumeGetNewLevel (TWINAMP *wa, int mixer);
static void removeUpdateSignal (TWINAMP *wa);
static int getUpdateSignal (TWINAMP *wa);
static int doMediaPlayerFound (TWINAMP *wa);
static int doMediaPlayerQuit (TWINAMP *wa);
static int handleConnectionTimeout (TWINAMP *wa, unsigned int waittime);
static int connectToMediaPlayer (TWINAMP *wa);
static uint32_t getAddler (uint8_t *data, size_t len);
unsigned int __stdcall packet_listener (TWINAMP *wvs);
static int cmd_processGen (TWINAMP *wa, TMPGENTRACKINFO *gen);
static int cmd_processWave (TWINAMP *wa, TMPCMDWAVE *wave);


static CRITICAL_SECTION updatelock_cs;
static TWINAMP *g_wvs = NULL;







void REFRESH_LOCK ()
{
#if 0
	int retries = 0;
    while (!TryEnterCriticalSection((CRITICAL_SECTION*)&updatelock_cs)){
        if (retries++ < 100){
            Sleep(0);
            continue;
        }
        EnterCriticalSection((CRITICAL_SECTION*)&updatelock_cs);
        break;
    }
#else
	EnterCriticalSection((CRITICAL_SECTION*)&updatelock_cs);
#endif
}

void REFRESH_UNLOCK ()
{
	LeaveCriticalSection((CRITICAL_SECTION*)&updatelock_cs);
}

static void REFRESH_LOCK_CREATE ()
{
	InitializeCriticalSection((CRITICAL_SECTION*)&updatelock_cs);
}

static void REFRESH_LOCK_DELETE ()
{
	DeleteCriticalSection((CRITICAL_SECTION*)&updatelock_cs);
}

int startWVS (TWINAMP *wvs, int argc, char* argv[])
{
	
	if (wvs == NULL){
		dbprintf("startWVS: wvs is null, exiting\n");
		return 0;
	}else{
		g_wvs = wvs;
	}

	if (!myLCDInit(MYLCDCONFIG, wvs->wvsPath)){
		MessageBox(NULL, "WVS "MY_VERSIONSTR": mylcd.dll failed to initialise a display.\n",\
		  "WVS: Initialisation failure.", MB_OK|MB_SYSTEMMODAL);
		return 0;
	}

	REFRESH_LOCK_CREATE();
	if (!initWVS(wvs)){
		dbprintf("WVS failed to initialise\n");
		MessageBox(NULL, "WVS "MY_VERSIONSTR": WVS initialisation failed.\nWVS should be reinstalled.\n\nExiting..", "WVS: Initialisation failure.", MB_OK|MB_SYSTEMMODAL);
		waCleanup(wvs);
		REFRESH_LOCK_DELETE();
		demoCleanup();
		return 1;
	}else{
		/*
		printf("####\n");
		lPrintf(frame, (frame->width/2)-2, 0, LFTW_SNAP, LPRT_OR, "hello!!");
		printf("####\n");
		lRefresh(frame);*/
	
		registerPages(wvs);
		resetPageSwapTime(wvs);
		if (!configLoad(wvs, WACONFIG)){
			dbprintf("WVS config file '%s' not found\n", WACONFIG);
		}
		lng_create(wvs, wvs->config.langpath);
		netInit();
		initGUI(wvs); // hidden window for the various callbacks
		
		if (argc > 1)
			loadProfile(wvs, argv[1]);
		else
			loadProfile(wvs, WAPROFILE);
		setConfigItemValues(wvs);
		getConfigItemValues(wvs);
		defaultPageReset(wvs, wvs->config.startPage);
		if (!connectToMediaPlayer(wvs)){
			dbprintf("connectToMediaPlayer() failed\n");
			waCleanup(wvs);
			demoCleanup();
			REFRESH_LOCK_DELETE();
			return 1;
		}
	}

	REFRESH_LOCK();
	if (!wvs->config.mediaPlayDisabled){
		wvs->cacheLock = CreateSemaphore(NULL, 1, 1, NULL);
		cache_create(&wvs->net.cache);
		unsigned int threadID = 0;
		wvs->net.thdListener = (HANDLE)_beginthreadex(NULL, 0, (void*)packet_listener, (void*)wvs, 0, &threadID);
		//Sleep(10);
	}

	initG15(wvs, g15keycb);
	initMixerDevice(wvs, VOLUME_MASTER);
	initMixerDevice(wvs, VOLUME_WAVE);
	initMixerDevice(wvs, VOLUME_MEDIAPLAYER);
	initPages(wvs);
	initGraphTimer(wvs, wvs->graph.fps);
	WVSHookOn(wvs);
	REFRESH_UNLOCK();
	
	dbprintf("entering main program loop\n");
	
	while(!wvs->state && processWindowMessages(wvs) > 0){
#if 1
		if (kbhit()){
			int key = getch();
			//printf("%i\n",key);
			if (key == 27){	// esc
				wvs->state = 1;
			}else if (key == ','){	//	page left
				wvs->state = processKeyPress(wvs, G15_SOFTKEY_1);
				setUpdateSignal(wvs);
			}else if (key == '.'){	//	page right
				wvs->state = processKeyPress(wvs, G15_SOFTKEY_2);
				setUpdateSignal(wvs);
			}
		}
#endif
	}
	dbprintf("exited main program loop, shutting down\n");

	killGraphTimer(wvs);
	WVSHookOff(wvs);
	netCloseSocket(wvs->net.conn.socket.server);
	REFRESH_LOCK();
	if (WaitForSingleObject(wvs->net.thdListener, 200000) == WAIT_OBJECT_0)
		CloseHandle(wvs->net.thdListener);
	closeMixers(wvs);
	closeG15(wvs);
	closeGUI(wvs);
	netShutdown();
	cache_delete(&wvs->net.cache);
	if (wvs->cacheLock){
		if (WaitForSingleObject(wvs->cacheLock, 2000) == WAIT_OBJECT_0)
			CloseHandle(wvs->cacheLock);
	}
	waCleanup(wvs);
	REFRESH_LOCK_DELETE();
	dbprintf("WVS shutdown\n");
	//free(wvs);
	demoCleanup();

	return 1;
}

static int closeSignal = 0;

	
// engine ticker
void (CALLBACK EngineTickerCB)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	TWINAMP *wa = (TWINAMP *)dwUser;
	
#if 0
	PostMessage(wa->hook.hwnd, 4010, 0, (LPARAM)wa);
#else

	if (wa->state){
		//dbprintf("EngineTickerCB stopped\n");
		return;
	}
	
	if ((closeSignal == 2 && wa->config.exitWithPlayer) || closeSignal == 1){
		wa->state = 1;
		PostQuitMessage(0);
	}
	if (wa->net.notFound){
		if (wa->config.exitWithPlayer && wa->net.foundOnce){
			wa->state = 1;
		}
	}

	if (wa->graph.updateCount != wa->misc.timerUpdateCounter){
		wa->graph.updateCount = wa->misc.timerUpdateCounter;
		setUpdateSignal(wa);
		requestWaveData(wa);
		advanceTitleScroll(wa, 0);
	}
	if (!wa->config.mediaPlayDisabled && isPageOpen(wa, DISF_MAIN))
		cache_processPackets(wa);

	if (!wa->state){
		doTimedEvents(wa);
      	if (!getUpdateSignal(wa))
			renderFrames(wa);
		updateDisplay(wa, frame);
	}
#endif
}

// one second interval timer
void (CALLBACK OneSecIntervalTickerCB)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (dwUser){
		TWINAMP *wa = (TWINAMP *)dwUser;
		PostMessage(wa->hook.hwnd, 4020, 0, (LPARAM)wa);
	}
}

// title scroller, vis/osc and refresh ticker
void (CALLBACK TitleTickerCB)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (dwUser){
		TWINAMP *wa = (TWINAMP *)dwUser;
#if 0
		static unsigned int counter = 0;

		if (!counter){
			counter = GetTickCount();
			wa->misc.timerUpdateCounter = 0;
		}
		if (GetTickCount()-counter > 1000){
			//dbprintf("%i\n", wa->misc.timerUpdateCounter);
			counter = GetTickCount();
			wa->misc.timerUpdateCounter = 0;
		}
#endif

#if 0
		PostMessage(wa->hook.hwnd, 4000, 0, (LPARAM)wa);	
#else
		if (++wa->misc.timerUpdateCounter > 0xFFFF)
			wa->misc.timerUpdateCounter = 0;
#endif
	}
}

static int KillOtherInstances ()
{
	HWND hwin = FindWindow(NULL, szClassName);
	if (hwin != NULL){
		dbprintf("another instance of WVS found, terminating..\n");
		PostMessage(hwin, WM_CLOSE, 0, 0);
		Sleep(1000);

		hwin = FindWindow(NULL, szClassName);
		if (hwin != NULL){
			dbprintf("WVS persists, terminating..\n");

			if (IsWindow(hwin)){
				DWORD pid = 0;
				GetWindowThreadProcessId(hwin, &pid);
				if (pid){
					if (IsWindow(hwin)){
						killProcessOpenThread(pid);
						killProcessRemoteThread(pid);
					}
					Sleep(100);
				}
			}
		}
		return 1;	// an instance was found and termination attempted
	}
	return 0;	// none found
}

#if defined (BUILDING_DLL)

unsigned int __stdcall DoWVS (TWINAMP *wvs)
{
	if (!wvs->state){	// let media player start before this
		int i = 20;
		while (!wvs->state && i--)
			Sleep(50);
	}
	if (wvs->state)
		return 0;

	KillOtherInstances();
	startWVS(wvs, 0, NULL);
	dbprintf("WVS thread exited\n");
	_endthreadex(1);
	return 1;
}

#else

int main (int argc, char* argv[])
{
	KillOtherInstances();
	TWINAMP *wvs = (TWINAMP*)calloc(1, sizeof(TWINAMP));
	if (wvs){
		int ret = startWVS(wvs, argc, argv);
		//free(wvs);
		return ret;
	}else{
		return 0;
	}
}
#endif

static void WVSHookOn (TWINAMP *wvs)
{
#ifndef __NOHOOK__
	if (wvs){
		dbprintf("kb hook on\n");
		SetHook(wvs->hook.hwnd, wvs);
	}
#endif
}

static void WVSHookOff (TWINAMP *wvs)
{
#ifndef __NOHOOK__
	if (wvs){
		dbprintf("kb hook off\n");
		UnHook(wvs->hook.hwnd);
	}
#endif
}

static void closeG15 (TWINAMP *wvs)
{
	if (wvs->g15.hDev){
		resetG15Leds(wvs);
		hid_g15_close(wvs->g15.hDev);
	}
}

static void closeMixers (TWINAMP *wvs)
{
	if (!wvs->mixer[VOLUME_MASTER].status)
		mix_Close(wvs->mixer[VOLUME_MASTER].mixerHandle);
	if (!wvs->mixer[VOLUME_WAVE].status)
		mix_Close(wvs->mixer[VOLUME_WAVE].mixerHandle);
	wvs->mixer[VOLUME_MASTER].status = 0;
	wvs->mixer[VOLUME_WAVE].status = 0;
}

static void closeGUI (TWINAMP *wvs)
{
	if (IsWindow(wvs->hook.hwnd))
		DestroyWindow(wvs->hook.hwnd);
	//CloseHandle(wvs->hook.hwnd);
}
static void resetG15Leds (TWINAMP *wa)
{
	if (wa->overlay.g15Lights && wa->g15.hDev){
		hid_g15_setLCDBackLightlevel(wa->g15.hDev, wa->g15.LCDC);
		hid_g15_setMLEDs(wa->g15.hDev, wa->g15.MLeds);
		hid_g15_setKBBackLightlevel(wa->g15.hDev, wa->g15.KBLeds);
		//hid_g15_setState(wa->g15.hDev, wa->g15.states);
	}
}

static int processWindowMessages (TWINAMP *wvs)
{
	int ret = 0;
	if ((ret= GetMessage(&wvs->hook.messages, NULL, 0, 0)) > 0){
		TranslateMessage(&wvs->hook.messages);
		DispatchMessage(&wvs->hook.messages);
	}
	return ret;
}

// top down
static void effectEnterTopDown (TWINAMP *wa, TFRAME *src, TFRAME *des)
{
	int x,y;
	for (y=src->height-1;y>=0 ;y--){
		for (x=0; x < src->width; x++){
			if (lGetPixel(src,x,y))
				lDrawLine(des,x,0,x,y,LSP_SET);
		}
		//GetMessage(&wa->hook.messages, NULL, 0, 0);
		setUpdateSignal(wa);
		RefreshDisplay(wa, des);
		lSleep(10);
		for (x=0; x < src->width; x++){
			if (lGetPixel(src,x,y)){
				lDrawLine(des,x,0,x,y,LSP_CLEAR);
				lSetPixel(des,x,y,LSP_SET);
			}
		}
	}
}

// bottom up
static void effectLeaveBottomUp (TWINAMP *wa, TFRAME *src, TFRAME *des)
{
	lCopyFrame(src, des/*, LCASS_CPY*/);
	int x,y;
	for (y=src->height-1; y>=0;y--){
		for (x=0; x < src->width; x++){
			if (lGetPixel(src,x,y))
				lDrawLine(des,x,y,x,des->height,LSP_SET);
		}
		//GetMessage(&wa->hook.messages, NULL, 0, 0);
		setUpdateSignal(wa);
		RefreshDisplay(wa, des);
 		lSleep(10);
		for (x=0; x < src->width; x++){
			if (lGetPixel(src,x,y))
				lDrawLine(des,x,y,x,des->height,LSP_CLEAR);
		}
	}
}

static void do1sIntervalUpdates (TWINAMP *wa)
{
	//setUpdateSignal(wa);	// ensure at least one update per second for all frames

	if (!isPageOpen(wa, DISF_MAIN)) return;

	if (!wa->net.notFound){
		cacheTrackFilenames(wa, wa->track.number.value, wa->track.number.value);
		cacheTrackTitles(wa, wa->track.number.value, wa->track.number.value+6);
		cacheTrackTitles(wa, wa->playlist.highlightPos-2, wa->playlist.highlightPos+8);
		cacheMetaTags(wa, wa->track.number.value, wa->track.number.value);
	}else if (!wa->config.mediaPlayDisabled){
		netGetPluginVersion(&wa->net.conn, NULL, NULL, NULL);
	}
	if (!wa->config.mediaPlayDisabled && !wa->net.notFound){
		if (updateTitleScroll(wa, 0)){	// returns 1 if title has been updated. (a track has changed)
			if (!wa->misc.seeked){		// don't seek past signature at startup
				seekScroll(wa, getFrame(wa, DISF_MAIN)->width-12);	// display full title on track change
			}else{
				wa->misc.seeked = 0;
			}

			resetBarGraphScale(wa);		// scale is a per track setting
		}
	}
}

static void resetPageSwapTime (TWINAMP *wa)
{
	wa->dis.lastSwapTime = GetTickCount();
	wa->dis.curSwapTime = wa->dis.lastSwapTime;
}

static void doTimedEvents (TWINAMP *wa)
{
	wa->clock.tmillisecond = getTime();
	if (wa->config.autoPageSwap){
		unsigned int ticks = GetTickCount();
		if (ticks-wa->dis.lastSwapTime > SWAPSTARTTIME){
			if (ticks-wa->dis.curSwapTime > SWAPINTERVAL){
				if (kbHookGetStatus())
					kbHookOff();
				PostMessage(wa->hook.hwnd, 5053, 0, (LPARAM)wa);
				wa->dis.curSwapTime = GetTickCount();
			}
		}
	}
}

static clock_t getTime ()
{
	return clock();
}

static float pageStepLookup (int index)
{
	const float IndexToStep[20] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0};
	return IndexToStep[index];
}

static void calculatePageRotation (TWINAMP *wa)
{
	if (wa->dis.direction){
		if (wa->dis.direction == 1){	// right to left
			wa->dis.angle += (pageStepLookup(wa->config.rotateFlipStep)*2.0);

			if (wa->dis.angle > 90.0 && wa->dis.angle < 270.0){
				wa->dis.angle = 270.0;
				setActivePage(wa, wa->dis.index);
			}
			if (wa->dis.angle > 360.0){
				wa->dis.angle = 0.0;
				wa->dis.direction = 0;
			}

		}else if (wa->dis.direction == 2){	// left to right
			wa->dis.angle -= (pageStepLookup(wa->config.rotateFlipStep)*2.0);

			if ((wa->dis.angle > 90.0) && (wa->dis.angle < 270.0)){
				wa->dis.angle = 90.0;
				setActivePage(wa, wa->dis.index);
			}
			if (wa->dis.angle < 0.0){
				wa->dis.angle = 0.0;
				wa->dis.direction = 0;
			}
		}else{
			wa->dis.direction = 0;
		}
	}
}

static void pagePanRight (TWINAMP *wa, TFRAME *src, TFRAME *des)
{
	int x,y;
	for (x=0; x < src->width; x+=8){
		for (y=0; y < src->height; y+=8){
			lCopyArea(src, des, x, y, x, y, x+8, y+8);
			setUpdateSignal(wa);
			RefreshDisplay(wa, des);
			//GetMessage(&wa->hook.messages, NULL, 0, 0);
			lSleep(15);
		}
	}
}

static void updateDisplay (TWINAMP *wa, TFRAME *frame)
{
	if (!wa->config.pageSwapMode || wa->config.pageSwapMode == 3)
		calculatePageRotation(wa);

	if (!wa->dis.direction){
		if (RefreshDisplay(wa, getFrame(wa, DISF_ACTIVEFRAME))){
			// this just ensures final destination page is the expected page
			if (getFrame(wa, DISF_ACTIVEFRAME) != getFrame(wa, wa->dis.index)){
			 	if (!wa->config.pageSwapMode || wa->config.pageSwapMode == 3){
					if (wa->dis.directionLast == 1){
						wa->dis.direction = 1;
						wa->dis.angle = 0.0;
					}else{
						wa->dis.direction = 2;
						wa->dis.angle = 360.0;
					}
				}
			}
		}
	}else{
		if (!wa->config.pageSwapMode){	// rotate 1
			setUpdateSignal(wa);
			lClearFrame(frame);
			//lRotateFrameEx(getFrame(wa, DISF_ACTIVEFRAME), frame, 0.0, wa->dis.angle, 0.0, 180.0, -180.0, frame->width>>1, frame->height>>1);
			lRotateFrameEx(getFrame(wa, DISF_ACTIVEFRAME), frame, 0.0, wa->dis.angle, 0.0, 92.0, -90.0, (frame->width/2.0), (frame->height/2.0)+0.0);
			RefreshDisplay(wa, frame);
			lSleep(5);

		}else if (wa->config.pageSwapMode == 1){	// line fade
			wa->dis.direction = 0;
			wa->dis.angle = 0.0;
			effectLeaveBottomUp(wa, getFrame(wa, DISF_ACTIVEFRAME), frame);
			setActivePage(wa, wa->dis.index);
			effectEnterTopDown(wa, getFrame(wa, DISF_ACTIVEFRAME), frame);

		}else if (wa->config.pageSwapMode == 2){	// direct
			wa->dis.direction = 0;
			wa->dis.angle = 0.0;
			setActivePage(wa, wa->dis.index);
			RefreshDisplay(wa, getFrame(wa, DISF_ACTIVEFRAME));

		}else if (wa->config.pageSwapMode == 3){	// rotate 2
			setUpdateSignal(wa);
			lClearFrame(frame);
			lRotateFrameEx(getFrame(wa, DISF_ACTIVEFRAME), frame, wa->dis.angle, 0.0, 0.0, 180.0, -180.0, (frame->width/2.0), (frame->height/2.0)+1.0);
			RefreshDisplay(wa, frame);

		}else if (wa->config.pageSwapMode == 4){	// slide left/right
			wa->misc.lag = 1;
			if (wa->dis.direction && wa->misc.lag){
				if (++wa->misc.slideOffset > frame->width-1){
					wa->misc.slideOffset = 0;
					wa->dis.direction = 0;
					setActivePage(wa, wa->dis.index);
				}else{
					lClearFrame(frame);
					if (wa->dis.direction == 1){
						lCopyArea(getFrame(wa, DISF_ACTIVEFRAME), frame, 0, 0, wa->misc.slideOffset, 0, frame->width-1, frame->height-1);	// old frame
						lCopyArea(getFrame(wa, wa->dis.index), frame, frame->width-wa->misc.slideOffset, 0, 0, 0, wa->misc.slideOffset, frame->height-1); // new frame
					}else{
						lCopyArea(getFrame(wa, DISF_ACTIVEFRAME), frame, wa->misc.slideOffset, 0, 0, 0, frame->width-wa->misc.slideOffset, frame->height-1);
						lCopyArea(getFrame(wa, wa->dis.index), frame, 0, 0, frame->width-wa->misc.slideOffset, 0, frame->width-1, frame->height-1);
					}
				}
				setUpdateSignal(wa);
				RefreshDisplay(wa, frame);
				lSleep(10);
			}
		/*
		}else if (wa->config.pageSwapMode == 5){	//slide up/down
			wa->misc.lag ^= 1;
			if (wa->dis.direction && wa->misc.lag){
				if (++wa->misc.slideOffset > frame->height-1){
					wa->misc.slideOffset = 0;
					wa->dis.direction = 0;
					setActivePage(wa, wa->dis.index);
				}else{
					lClearFrame(frame);
					if (wa->dis.direction == 1){
						lCopyArea(getFrame(wa, DISF_ACTIVEFRAME), frame, 0, 0, 0, wa->misc.slideOffset, frame->width-1, frame->height-1);
						lCopyArea(getFrame(wa, wa->dis.index), frame, 0, frame->height-wa->misc.slideOffset, 0, 0, frame->width-1, wa->misc.slideOffset);
					}else{
						lCopyArea(getFrame(wa, DISF_ACTIVEFRAME), frame, 0, wa->misc.slideOffset, 0, 0, frame->width-1, frame->height-wa->misc.slideOffset);
						lCopyArea(getFrame(wa, wa->dis.index), frame, 0, 0, 0, frame->height-wa->misc.slideOffset,  frame->width-1, frame->height-1);
					}
				}
				setUpdateSignal(wa);
				RefreshDisplay(wa, frame);
			}
		*/
		}else if (wa->config.pageSwapMode == 5){	//slide up/down
			if (wa->dis.direction && ++wa->misc.lag > 5){
				wa->misc.lag = 0;
				if (++wa->misc.slideOffset > (frame->height-1)/2){
					wa->misc.slideOffset = 0;
					wa->dis.direction = 0;
					setActivePage(wa, wa->dis.index);
				}else{
					if (wa->dis.direction == 1){
						lCopyFrame(getFrame(wa, wa->dis.index), frame/*, LSP_SET*/);
						lCopyArea(getFrame(wa, DISF_ACTIVEFRAME), frame, 0, 0, 0, wa->misc.slideOffset, frame->width-1, (frame->height-1)/2);
						lCopyArea(getFrame(wa, DISF_ACTIVEFRAME), frame, 0, wa->misc.slideOffset+(frame->height/2), 0, (frame->height/2)+1, frame->width-1, frame->height-wa->misc.slideOffset);
					}else{
						lCopyFrame(getFrame(wa, DISF_ACTIVEFRAME), frame/*, LSP_SET*/);
						lCopyArea(getFrame(wa, wa->dis.index), frame, 0, 0, 0, (frame->height/2)-wa->misc.slideOffset, frame->width-1, frame->height/2);
						lCopyArea(getFrame(wa, wa->dis.index), frame, 0, frame->height-wa->misc.slideOffset, 0, (frame->height+1)/2, frame->width-1, frame->height-1);
					}
				}
				setUpdateSignal(wa);
				RefreshDisplay(wa, frame);
			}
		}else if (wa->config.pageSwapMode == 6){ //drag
			wa->dis.direction = 0;
			wa->dis.angle = 0.0;
			TFRAME *temp = lCloneFrame(getFrame(wa, DISF_ACTIVEFRAME));
			setActivePage(wa, wa->dis.index);
			if (wa->dis.index == DISF_PLAYLIST || wa->dis.index == DISF_HITLIST)
				renderFrame(wa, wa->dis.index);
			pagePanRight(wa, getFrame(wa, wa->dis.index), temp);
			lDeleteFrame(temp);
		}
	}
}

#if 0
static int utf16_wcstobs (ubyte *r, size_t r_n, wchar_t *wc, size_t wc_n)
{
	int i;
	for (i = 0; i < wc_n; i++){
		*r = (ubyte)(wc[i]&0xFF);
		r++;
	}
	*r = 0;
	return i;
}
#endif

static int hitlistIndexToFile (TWINAMP *wa, int hitlistIndex, int hitlist, char *buffer, size_t buffersize)
{
	if (hitlistOpen(wa, hitlist)){
		if (hitlistIndex < wa->hitlist.al->tlines){
			char *path = (char*)wa->hitlist.al->lines[hitlistIndex];
			strncpy(buffer, path, buffersize);
			hitlistClose(wa);
			return 1;
		}
		hitlistClose(wa);
	}
	return 0;
}

static int hitlistEnqueueFile (TWINAMP *wa, int hitlistIndex, int hitlist, int play)
{
	if (hitlistOpen(wa, hitlist)){
		if (hitlistIndex < wa->hitlist.al->tlines){
			char *path = (char*)wa->hitlist.al->lines[hitlistIndex];
			wchar_t buffer[4096];
			int ret = MultiByteToWideChar(CP_UTF8, 0, path, strlen(path), buffer, 4096);
			if (ret > 0 && ret < 4096){
				buffer[ret] = 0;
				netEnqueueFile(&wa->net.conn, CMD_ENQUEUEFILEW, (char*)buffer, play);
			}
		}
		hitlistClose(wa);
	}
	return 1;
}

// very hackish
static int asktoutf16 (ubyte *r, size_t r_n, wchar_t *wc, size_t wc_n)
{
	int i;
	ubyte *pp = (ubyte *)wc;

	for (i = 0; i < r_n && *r; i++){
		*pp++ = *r++;
		*pp++ = 0;
	}
	wc[i] = 0;
	return i;
}

static int detectWC (char *item)
{
	int i;
	int len = strlen(item);
	int count = 0;
	int lastPos = -100;

	for (i = 0; i < len; i++){
		if (item[i] == '?'){
			if (i == lastPos+1){
				count++;
				if (count > 0)	// "??" found in string
					return count;
			}
			lastPos = i;
		}
	}
	return count;
}

static int getMetaTag (TWINAMP *wa, char *buffer, size_t buffersize, int tagIndex, int trackIndex, int enc)
{
	char *item = NULL;
	*buffer = 0;
	int ret = 0;

	lSetCharacterEncoding(wa->hw, enc);
	if (enc == CMT_UTF8)
		item = cache_FindString(&wa->cache, CMD_GET_METADATAW, tagIndex, trackIndex);
	else
		item = cache_FindString(&wa->cache, CMD_GET_METADATAA, tagIndex, trackIndex);

	if (item){
		strncpy(buffer, item, buffersize);
		ret = enc;
	}

	if (enc != CMT_UTF8 && wa->config.autoUTF16){
		if (detectWC(buffer)){
			item = cache_FindString(&wa->cache, CMD_GET_METADATAW, tagIndex, trackIndex);
			if (item){
				strncpy(buffer, item, buffersize);
				enc = CMT_UTF8;
				ret = enc;
				lSetCharacterEncoding(wa->hw, CMT_UTF8);
			}
		}
	}
	lSetCharacterEncoding(wa->hw, enc);
	return ret;
}

static int isKeyPressed (int vk)
{
	return GetKeyState(vk)&0x80;
}

static int processKeyPress (TWINAMP *wa, unsigned int key)
{
	if (key){
		if (wa->dis.index <= DISF_TOTAL && wa->dis.index){
			int status = -1;
			if (isPageOpen(wa, wa->dis.index)){
				TDISPAGE *page = getPage(wa, wa->dis.index);
				status = page->proc.input(wa, key, page->userPtr);
			}
			if (status == -1)
				return handleCommonKeys(wa, key, NULL);
			else
				return status;
		}
	}
	return 0;
}

int handleCommonKeys (TWINAMP *wa, int key, void *DoNotUse)
{
	//dbprintf("common key: %i\n",key);

	if (key == G15_WHEEL_CLOCKWISE){
		if (!wa->config.reverseVolumeCtrl)
			volumeIncrease(wa, wa->config.volumeCtrl);
		else
			volumeDecrease(wa, wa->config.volumeCtrl);

	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		if (!wa->config.reverseVolumeCtrl)
			volumeDecrease(wa, wa->config.volumeCtrl);
		else
			volumeIncrease(wa, wa->config.volumeCtrl);

	}else if (isKeyPressed(VK_MENU) && key == G15_SOFTKEY_1){
		if (defaultPageReset(wa, wa->config.startPage))
			onPageChange(wa);

	}else if (isKeyPressed(VK_MENU) && key == G15_SOFTKEY_2){
		if (!wa->overlay.g15Lights)
			wa->overlay.g15Lights = 0x03;
		else if (wa->overlay.g15Lights == 0x03)
			wa->overlay.g15Lights = 0x02;
		else if (wa->overlay.g15Lights == 0x02)
			wa->overlay.g15Lights = 0x01;
		else /*if (wa->overlay.g15Lights == 0x01)*/
			wa->overlay.g15Lights = 0x03;

	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_2){
		toggle_DancingLights(wa);

	}else if (key>1024 && key&G15_SOFTKEY_1&0x0F && key&G15_SOFTKEY_4&0x0F){
		Sleep(900);
		wa->state = 1;
		return 1;		// exit wvs
		
	}else if (key == G15_SOFTKEY_1){	// previous screen
		SendMessage(wa->hook.hwnd, 5051, 0, (LPARAM)wa);
		resetPageSwapTime(wa);

	}else if (key == G15_SOFTKEY_2){	// next screen
		SendMessage(wa->hook.hwnd, 5052, 0, (LPARAM)wa);
		resetPageSwapTime(wa);

	}else if (key == VK_MEDIA_NEXT_TRACK){
		track_Next(wa);

	}else if (key == VK_MEDIA_PREV_TRACK){
		track_Previous(wa);

	}else if (key == VK_MEDIA_STOP){
		track_Stop(wa);

	}else if (key == VK_MEDIA_PLAY_PAUSE){
		track_togglePlayState(wa);

	//}else if (key == VK_VOLUME_MUTE){   // nothing to do

	//}else if (key == VK_BACK){
	}
	return 0;
}

static void volumeSetActive (TWINAMP *wa, int mixer)
{
	wa->config.volumeCtrl = mixer;
	wa->overlay.volumeLevel = 1;

	if (mixer == VOLUME_MEDIAPLAYER){
		if (!wa->net.notFound){
			wa->misc.winampVolumeTime = GetTickCount();
			if (!wa->net.notFound)
				netGetVolumeEx(&wa->net.conn, &wa->mixer[mixer].currentVolume, 0);
		}
	}else{
		volumeGetNewLevel(wa, mixer);
		wa->mixer[mixer].volumeTime = GetTickCount();
	}
}

static int hitlistOpen (TWINAMP *wa, int num)
{
	char name[1024];
	snprintf(name, sizeof(name), "%s%i.%s", HITLISTNAME, num, HITLISTEXT);

	if (wa->hitlist.al)
		freeASCIILINE(wa->hitlist.al);
	wa->hitlist.al = readFileA(name);
	return (wa->hitlist.al != NULL);
}

static int hitlistClose (TWINAMP *wa)
{
	if (wa->hitlist.al)
		freeASCIILINE(wa->hitlist.al);
	wa->hitlist.al = NULL;
	return 1;
}

static int hitlistWrite (TWINAMP *wa, int num, ubyte *mp3path, int action)
{

	char name[1024];
	snprintf(name, sizeof(name), "%s%i.%s", HITLISTNAME, num, HITLISTEXT);
	wa->hitlist.hFile = fopen(name, "w+b"); //wb
	if (wa->hitlist.hFile == NULL){
		//dbprintf("unable to open %s\n", name);
		return 0;
	}

	fseek(wa->hitlist.hFile, 0, SEEK_SET);
	fprintf(wa->hitlist.hFile,"ï»¿#EXTM3U\r\n");

	if (wa->hitlist.al != NULL){
		if (wa->hitlist.al->tlines){
			int i = 1;
			do{
				if (wa->hitlist.al->lines[i]){
					if (strcmp((char*)mp3path, (char*)wa->hitlist.al->lines[i]))
						fprintf(wa->hitlist.hFile,"%s\r\n", (char*)wa->hitlist.al->lines[i]);
				}
			}while(++i < wa->hitlist.al->tlines);
		}
	}

	if (action)
		fprintf(wa->hitlist.hFile,"%s\r\n", (char*)mp3path);

	if (wa->hitlist.hFile)
		fclose(wa->hitlist.hFile);
	wa->hitlist.hFile = NULL;

	return 1;
}

static void cacheTrackFilenames (TWINAMP *wa, int start, int end)
{
	if (wa->net.notFound)
		return;

	int i;
	for (i = start; i <= end && i < wa->playlist.totalTracks; i++){
		if (i >= 0){
			if (!cache_FindString(&wa->cache, CMD_GET_TRACKFILENAMEA, 0, i)){
				netGetTitle(&wa->net.conn, i, CMD_GET_TRACKFILENAMEA, NULL, 0);
				netGetTitle(&wa->net.conn, i, CMD_GET_TRACKFILENAMEW, NULL, 0);
				//dbprintf("caching track %i\n", i);
			}else{
				//dbprintf("track %i already cached\n", i);
			}
		}
	}
}

static void cacheMetaTagsNS (TWINAMP *wa, int trackStart, int trackEnd)
{
	int i,metaIndex;
	if (trackStart < 0)
		trackStart = 0;
	if (trackEnd > wa->playlist.totalTracks-1)
		trackEnd = wa->playlist.totalTracks-1;

	for (i = trackStart; i <= trackEnd; i++){
		metaIndex = METATAGTOTAL;
		while(metaIndex--)
			cacheMetaTagNS(wa, metaIndex, i);
	}
}

static void cacheMetaTagNS (TWINAMP *wa, int metaIndex, int trackIndex)
{
	wchar_t tag16[32];
	asktoutf16(metaTags[metaIndex], strlen((char*)metaTags[metaIndex]), tag16, sizeof(tag16));
	//dbprintf("getting tag %i %i\n", metaIndex, trackIndex);
	netGetMetaData(&wa->net.conn, metaIndex, CMD_GET_METADATAW, trackIndex, tag16, NULL, 0);
	netGetMetaData(&wa->net.conn, metaIndex, CMD_GET_METADATAA, trackIndex, metaTags[metaIndex], NULL, 0);
}

static void cacheMetaTags (TWINAMP *wa, int trackStart, int trackEnd)
{
	int i,metaIndex;
	if (trackStart < 0)
		trackStart = 0;
	if (trackEnd > wa->playlist.totalTracks-1)
		trackEnd = wa->playlist.totalTracks-1;

	for (i = trackStart; i <= trackEnd; i++){
		metaIndex = METATAGTOTAL;
		while(metaIndex--)
			cacheMetaTag(wa, metaIndex, i);
	}
}

static void cacheMetaTag (TWINAMP *wa, int metaIndex, int trackIndex)
{
	if (!cache_FindString(&wa->cache, CMD_GET_METADATAA, metaIndex, trackIndex)){
		wchar_t tag16[32];
		asktoutf16(metaTags[metaIndex], strlen((char*)metaTags[metaIndex]), tag16, sizeof(tag16));
		//dbprintf("getting tag %i %i\n", metaIndex, trackIndex);
		netGetMetaData(&wa->net.conn, metaIndex, CMD_GET_METADATAW, trackIndex, tag16, NULL, 0);
		netGetMetaData(&wa->net.conn, metaIndex, CMD_GET_METADATAA, trackIndex, metaTags[metaIndex], NULL, 0);
	}
}

static void cacheTrackTitles (TWINAMP *wa, int start, int end)
{
	if (wa->net.notFound)
		return;

	if (start < 0)
		start = 0;

	int i;
	for (i = start; i <= end && i < wa->playlist.totalTracks; i++){
		if (!cache_FindString(&wa->cache, CMD_GET_TRACKTITLEA, 0, i)){
			netGetTitle(&wa->net.conn, i, CMD_GET_TRACKTITLEA, NULL, 0);
			netGetTitle(&wa->net.conn, i, CMD_GET_TRACKTITLEW, NULL, 0);
			//dbprintf("requesting track %i\n", i);
		}else{
			//dbprintf("track %i already cached\n", i);
		}
	}
}

static void cacheTrackTitlesNS (TWINAMP *wa, int start, int end)
{
	if (wa->net.notFound)
		return;

	if (start < 0)
		start = 0;

	int i;
	for (i = start; i <= end && i < wa->playlist.totalTracks; i++){
		netGetTitle(&wa->net.conn, i, CMD_GET_TRACKTITLEA, NULL, 0);
		netGetTitle(&wa->net.conn, i, CMD_GET_TRACKTITLEW, NULL, 0);
	}
}

static void track_Next (TWINAMP *wa)
{
	netMCtrlNext(&wa->net.conn);
	resetBarGraphScale(wa);
}

static void track_Previous (TWINAMP *wa)
{
	netMCtrlPrevious(&wa->net.conn);
	resetBarGraphScale(wa);
}

static void track_Stop (TWINAMP *wa)
{
	netMCtrlStop(&wa->net.conn);
	netGetPlayState(&wa->net.conn, &wa->misc.lastKnownPlayState);
	resetBarGraphScale(wa);
	resetG15Leds(wa);
}

static void track_togglePlayState (TWINAMP *wa)
{
	netMCtrlPlay(&wa->net.conn);
	resetBarGraphScale(wa);
	netGetPlayState(&wa->net.conn, &wa->misc.lastKnownPlayState);
	if (wa->overlay.g15Lights && wa->misc.lastKnownPlayState != 1)
		resetG15Leds(wa);
}


static void toggle_DancingLights (TWINAMP *wa)
{
	if (wa->overlay.g15Lights){
		wa->overlay.g15Lights = 0;
	}else{
		if (!wa->overlay.g15LightsBackup)
			wa->overlay.g15Lights = 0x01|0x02;
		else
			wa->overlay.g15Lights = wa->overlay.g15LightsBackup;
	}

	if (!wa->overlay.g15Lights)
		resetG15Leds(wa);
}

static void toggle_overlayTrackNumber (TWINAMP *wa)
{
	wa->overlay.trackNumber ^= 1;
}

static int track_Rewind (TWINAMP *wa)
{
	if (!wa->net.notFound){
		netTrackRewind(&wa->net.conn, wa->config.trackSkipTime);
		getCurrentTrackDetails(wa);
	}
	return 1;
}

static int track_Forward (TWINAMP *wa)
{
	if (!wa->net.notFound){
		netTrackForward(&wa->net.conn, wa->config.trackSkipTime);
		getCurrentTrackDetails(wa);
	}
	return 1;
}

static void volumeDecrease (TWINAMP *wa, int mixer)
{
	if (mixer == VOLUME_MEDIAPLAYER){
		if (!wa->net.notFound){
			wa->overlay.volumeLevel = 1;
			wa->misc.winampVolumeTime = GetTickCount();
			if (!wa->net.notFound)
				netGetVolumeEx(&wa->net.conn, &wa->mixer[mixer].currentVolume, VOLUME_DECREASE);
		}
	}else{
		volumeGetNewLevel(wa, mixer);
		wa->mixer[mixer].currentVolume -= (wa->mixer[mixer].range.max/wa->mixer[mixer].range.step);
		if (wa->mixer[mixer].currentVolume <= wa->mixer[mixer].range.min)
			wa->mixer[mixer].currentVolume = wa->mixer[mixer].range.min;
		mix_SetControlValue(wa->mixer[mixer].mixerHandle, wa->mixer[mixer].dwControlID, wa->mixer[mixer].currentVolume);
		volumeGetNewLevel(wa, mixer);
	}
}

static void volumeIncrease (TWINAMP *wa, int mixer)
{
	if (mixer == VOLUME_MEDIAPLAYER){
		if (!wa->net.notFound){
			wa->misc.winampVolumeTime = GetTickCount();
			wa->overlay.volumeLevel = 1;
			if (!wa->net.notFound)
				netGetVolumeEx(&wa->net.conn, &wa->mixer[mixer].currentVolume, VOLUME_INCREASE);
		}
	}else{
		volumeGetNewLevel(wa, mixer);
		wa->mixer[mixer].currentVolume += (wa->mixer[mixer].range.max/wa->mixer[mixer].range.step);
		if (wa->mixer[mixer].currentVolume > wa->mixer[mixer].range.max)
			wa->mixer[mixer].currentVolume = wa->mixer[mixer].range.max;
		mix_SetControlValue(wa->mixer[mixer].mixerHandle, wa->mixer[mixer].dwControlID, wa->mixer[mixer].currentVolume);
		volumeGetNewLevel(wa, mixer);
	}
}

static int volumeGetNewLevel (TWINAMP *wa, int mixer)
{
	if (mixer < MIXER_TOTAL){
		if (!wa->mixer[mixer].status){
			mix_GetControlValue(wa->mixer[mixer].mixerHandle, wa->mixer[mixer].dwControlID, &wa->mixer[mixer].currentVolume);
			wa->overlay.volumeLevel = 1;
			wa->mixer[mixer].volumeTime = GetTickCount();
			return (int)wa->mixer[mixer].currentVolume;
		}
	}
	return 0;
}

static void volSensiIndexToLevel (int index, int *master, int *player)
{
	if (index == 1){		// medium
		if (player) *player = 4;
		if (master) *master = 96;
	}else if (index == 2){	// high
		if (player) *player = 1;
		if (master) *master = 192;
	}else{					// low, anything else
		if (player) *player = 8;
		if (master) *master = 48;
	}
}

static int initMixerDevice (TWINAMP *wa, int mixer)
{
	int master = 0;
	volSensiIndexToLevel(wa->config.volumeSensi, &master, NULL);

	if (mixer == VOLUME_MEDIAPLAYER && !wa->config.mediaPlayDisabled){
		if (!wa->net.notFound)
			netGetVolumeEx(&wa->net.conn, &wa->mixer[mixer].currentVolume, 0);
		else
			wa->mixer[mixer].currentVolume = 0;
		wa->mixer[mixer].range.step = master;	// isn't used.
		wa->mixer[mixer].status = 1;
		return 1;
	}

	if (mixer < MIXER_TOTAL){
		if (mix_Open(&wa->mixer[mixer].mixerHandle, wa->mixer[mixer].devicei)){
			if (mixer == VOLUME_MASTER){
				wa->mixer[mixer].dwControlID = mix_GetVolumeHandle(wa->mixer[mixer].mixerHandle, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, NO_SOURCE, MIXERCONTROL_CONTROLTYPE_VOLUME);
				strncpy((char*)wa->mixer[mixer].name, lng_getString(wa->lang, LNG_VOL_MASTER), 64);
			}else{
				wa->mixer[mixer].dwControlID = mix_GetVolumeHandle(wa->mixer[mixer].mixerHandle, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, MIXERCONTROL_CONTROLTYPE_VOLUME);
				strncpy((char*)wa->mixer[mixer].name, lng_getString(wa->lang, LNG_VOL_WAVE), 64);
			}
			if (mix_GetControlDetails(wa->mixer[mixer].mixerHandle, wa->mixer[mixer].dwLineID,\
				  &wa->mixer[mixer].range.min, &wa->mixer[mixer].range.max, &wa->mixer[mixer].range.step)){

				if (mix_GetControlValue(wa->mixer[mixer].mixerHandle, wa->mixer[mixer].dwControlID, &wa->mixer[mixer].currentVolume)){
					dbprintf("%i -%s-: %i min:%i max:%i step:%i\n",(int) wa->mixer[mixer].dwControlID, wa->mixer[mixer].name,(int)wa->mixer[VOLUME_MASTER].currentVolume, wa->mixer[mixer].range.min, wa->mixer[mixer].range.max, wa->mixer[mixer].range.step);

					wa->mixer[mixer].status = 0;
					if (wa->mixer[mixer].range.step > master)
						wa->mixer[mixer].range.step = master;
					else if (wa->mixer[mixer].range.step < 1)
						wa->mixer[mixer].range.step = 1;
				}
				return 1;
			}
		}
	}
	wa->mixer[mixer].status = 1;
	wa->mixer[mixer].range.step = 1;
	return 0;
}

static int getPlaylistFilename (TWINAMP *wa, char *buffer, size_t buffersize, int index, int enc)
{
	char *item = NULL;
	*buffer = 0;
	int ret = 0;

	lSetCharacterEncoding(wa->hw, enc);
	if (enc == CMT_UTF8)
		item = cache_FindString(&wa->cache, CMD_GET_TRACKFILENAMEW, 0, index);
	else
		item = cache_FindString(&wa->cache, CMD_GET_TRACKFILENAMEA, 0, index);

	if (item){
		strncpy(buffer, item, buffersize);
		ret = 1;
	}

	if (enc != CMT_UTF8 && wa->config.autoUTF16 && ret){
		if (detectWC(buffer)){
			item = cache_FindString(&wa->cache, CMD_GET_TRACKFILENAMEW, 0, index);
			if (item){
				strncpy(buffer, item, buffersize);
				lSetCharacterEncoding(wa->hw, CMT_UTF8);
				ret = 1;
			}
		}
	}
	return ret;

}

static int getPlaylistTrack (TWINAMP *wa, char *buffer, size_t buffersize, int start, int enc)
{
	char *item = NULL;
	*buffer = 0;
	int ret = 0;

	lSetCharacterEncoding(wa->hw, enc);
	if (enc == CMT_UTF8)
		item = cache_FindString(&wa->cache, CMD_GET_TRACKTITLEW, 0, start);
	else
		item = cache_FindString(&wa->cache, CMD_GET_TRACKTITLEA, 0, start);

	if (item){
		strncpy(buffer, item, buffersize);
		ret = 1;
	}

	if (enc != CMT_UTF8 && wa->config.autoUTF16){
		if (detectWC(buffer)){
			item = cache_FindString(&wa->cache, CMD_GET_TRACKTITLEW, 0, start);
			if (item){
				strncpy(buffer, item, buffersize);
				lSetCharacterEncoding(wa->hw, CMT_UTF8);
				ret = 1;
			}
		}
	}
	return ret;
}

static void resetBarGraphScale (TWINAMP *wa)
{
	wa->config.graphAudioScale = 1;
}

static void advanceTitleScroll (TWINAMP *wa, int mode)
{
	REFRESH_LOCK();
	if (!wa->config.mediaPlayDisabled && isPageOpen(wa, DISF_MAIN)){
		if (!mode){
			if (wa->clock.tmillisecond - wa->scroll.time > wa->config.trackTitleScrollPeriod){
				wa->scroll.time = wa->clock.tmillisecond;
				lUpdateScroll(wa->scroll.s);
			}
		}else{
			lUpdateScroll(wa->scroll.s);
		}
	}
	REFRESH_UNLOCK();
}

static void seekScroll (TWINAMP *wa, int pos)
{
	REFRESH_LOCK();
	if (isPageOpen(wa, DISF_MAIN)){
		wa->scroll.s->flags &= ~SCR_LOOP;
		int i;
		pos = MIN(pos, wa->scroll.s->srcFrm->width-2);
	
		for (i = 0; i < pos; i++)
			advanceTitleScroll(wa, 1);
		wa->scroll.s->flags |= SCR_LOOP;
	}
	REFRESH_UNLOCK();
}

static void seekEndTitleScroll (TWINAMP *wa)
{
	REFRESH_LOCK();
	wa->scroll.s->flags &= ~SCR_LOOP;
	int i;
	for (i = wa->scroll.s->pos; i<wa->scroll.s->srcFrm->width+wa->scroll.s->loopGapWidth; i++)
		advanceTitleScroll(wa, 1);

	wa->scroll.s->flags |= SCR_LOOP;
	REFRESH_UNLOCK();
}

static int setTitleScroll (TWINAMP *wa)
{
	REFRESH_LOCK();
	wa->scroll.s->srcFrm = wa->scroll.frame;
	wa->scroll.s->srcRect->x2 = wa->scroll.s->srcFrm->width-1;
	wa->scroll.s->srcRect->y2 = wa->scroll.s->srcFrm->height-1;
	wa->scroll.s->desRect->y2 = MAX(wa->scroll.s->desRect->y2, wa->scroll.s->desRect->y1 + wa->scroll.frame->height-1);
	wa->scroll.s->pos = 0;
	REFRESH_UNLOCK();
	return 1;
}

static int updateTitleScroll (TWINAMP *wa, int forceUpdate)
{
	REFRESH_LOCK();
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	//*buffer = 0;

	if (!getPlaylistTrack(wa, buffer, sizeof(buffer), wa->track.number.value, wa->config.encoding)){
		REFRESH_UNLOCK();
		return 0;
	}
		
	uint32_t crc = getAddler((ubyte*)buffer, strlen(buffer));
	if (wa->scroll.adler != crc || forceUpdate){
		wa->scroll.adler = crc;
		seekEndTitleScroll(wa);
		lDeleteFrame(wa->scroll.frame);
		wa->scroll.frame = lNewString(frame->hw, NEWSTRINGTYPE, PF_DONTFORMATBUFFER, wa->font.titleScroll, buffer);
		if (wa->scroll.frame){
			REFRESH_UNLOCK();
			return setTitleScroll(wa);
		}else{
			dbprintf("wa->scroll.frame == NULL\n");
		}
	}
	REFRESH_UNLOCK();
	return 0;
}

static int getCurrentTrackDetails (TWINAMP *wa)
{
	if (wa->config.mediaPlayDisabled)
		return 1;

	if (!netGetCurrentTrackInfo(&wa->net.conn, &wa->net.gen))
		return 0;

	return 1;
}

static int requestWaveData (TWINAMP *wa)
{
	if (isPageOpen(wa, DISF_MAIN)){
		if (!wa->net.notFound && !wa->config.mediaPlayDisabled){
			if (wa->graph.barType == 3 || wa->graph.barType == 4 || wa->graph.barType == 5){
				// 0x01 = spec only, 0x02 = wave only, 0x03 = both
				netGetWaveData(&wa->net.conn, 0x03, &wa->net.wave);
			}else{
				netGetWaveData(&wa->net.conn, 0x01, &wa->net.wave);
			}
		}
	}
	return 1;
}

static int registerOverlayFrame (TWINAMP *wa, TFRAMEOVER *ovr)
{
	int i;
	for (i = 0; i < wa->popup.count; i++){
		if (ovr == wa->popup.overlay[i] && ovr->reuse){
			wa->popup.overlay[i]->status = 1;
			if (!wa->popup.overlay[i]->ownerSetsT0)
				wa->popup.overlay[i]->t0 = GetTickCount();
			return 1;
		}
	}
	for (i = 0; i < 16; i++){
		if (wa->popup.overlay[i] == NULL){
			wa->popup.overlay[i] = ovr;
			wa->popup.overlay[i]->status = 1;
			if (!wa->popup.overlay[i]->ownerSetsT0)
				wa->popup.overlay[i]->t0 = GetTickCount();
			wa->popup.count++;
			return 1;
		}
	}
	return 0;
}

static void renderOverlays (TWINAMP *wa, int frameid)
{
	REFRESH_LOCK();
	int i;
	for (i = 0; i < wa->popup.count; i++){
		if (wa->popup.overlay[i]->status){
			if (wa->popup.overlay[i]->distination  == frameid || wa->popup.overlay[i]->distination == 255 || (!wa->popup.overlay[i]->distination && frameid == getActivePage(wa))){
				if (GetTickCount() >= wa->popup.overlay[i]->t0){
					if (GetTickCount()-wa->popup.overlay[i]->t0 < wa->popup.overlay[i]->requestedTime){
						TFRAME *des = getFrame(wa, frameid);
						TFRAME *src = wa->popup.overlay[i]->frame;
						int x = wa->popup.overlay[i]->x;
						int y = wa->popup.overlay[i]->y;

						if (x > des->width)
							x = des->width - src->width;
						if (y > des->height)
							y = des->height - src->height;
						if (x < 0) x = 0;
						if (y < 0) y = 0;

						/*
						wa->dis.angle += 1.5;
						if (wa->dis.angle > 359.0)
							wa->dis.angle = 0;
						*/
						//lRotateFrameEx(src, des, 0.0, wa->dis.angle, 0.0, 180.0, -175.0, x, y);
						//lRotateFrameEx(src, des, wa->dis.angle, 0.0, 0.0, 270.0, -260.0, x, y);
						lCopyAreaEx(src, des, x, y, 0, 0, src->width-1, src->height-1, 1, 1, wa->popup.overlay[i]->copyMode);
					}else{
						if (!wa->popup.overlay[i]->ownerCleans){
							lDeleteFrame(wa->popup.overlay[i]->frame);
							wa->popup.overlay[i]->frame = NULL;
						}
						wa->popup.overlay[i]->status = 0;
					}
				}
			}
		}
	}
	REFRESH_UNLOCK();
}

// block until an update has occurred. must only be called from the input message queue
static void renderFrameBlock (TWINAMP *wa, int frameid)
{
	renderFrame(wa, frameid);
	TDISPAGE *page = getPage(wa, frameid);
	while(!page->lastRenderTime && !wa->state){
		Sleep(0);
	}
}

static void _renderFrame (TWINAMP *wa, int frameid)
{
	TDISPAGE *page = getPage(wa, frameid);
	if (!isPageOpen(wa, frameid)){
		signalPageOpen(wa, frameid);
		if (!isPageOpen(wa, frameid)){
			dbprintf("render failed for '%s'\n", page->title);
			page->lastRenderTime = GetTickCount();
			return;
		}
	}

	//dbprintf("rendering: in '%s'\n", page->title);
	page->proc.render(wa, page->work, page->userPtr);
	//dbprintf("rendering: out '%s'\n",page->title);
	REFRESH_LOCK();
	lCopyFrame(page->work, page->frame/*, LCASS_CPY*/);
	REFRESH_UNLOCK();
	
	page->lastRenderTime = GetTickCount();
	if (frameid != DISF_RSS && !wa->overlay.trackFav && !wa->overlay.spectMarkers)
		renderOverlays(wa, frameid);
}

static void renderFrame (TWINAMP *wa, int frameid)
{
	getPage(wa, frameid)->lastRenderTime = 0;
}

void renderFrames (TWINAMP *wa)
{
	int i;
	for (i = 1; i <= DISF_TOTAL; i++){
		if (getPageStatus(wa, i)){	// is page enabled
			if ((isFrameActive(wa, i) && getPageFlags(wa, i)&RENDER_FOCUS) || getPageFlags(wa, i)&RENDER_ALWAYS){
				if (!getPageUpdatePeriod(wa, i)){
					_renderFrame(wa, i);
				}else{
					if (GetTickCount()-getPageLastRenderTime(wa, i) >= getPageUpdatePeriod(wa, i))
						_renderFrame(wa, i);
				}
			}
		}
	}
}

static int RefreshDisplay (TWINAMP *wa, TFRAME *frame)
{
#if 0
	static unsigned int ff = 0;
	static unsigned int counterf = 0;
	static unsigned int zerocounter = 0;

	if (!counterf){
		counterf = GetTickCount();
		ff = 0;
	}
	if (GetTickCount()-counterf > 1000){
		dbprintf("%i %i %i\n", ff, getUpdateSignal(wa), zerocounter);
		counterf = GetTickCount();
		zerocounter = 0;
		ff = 0;
	}
	ff++;

	if (!getUpdateSignal(wa)){
		removeUpdateSignal(wa);
		REFRESH_LOCK();
		lCopyFrame(frame, wa->clone/*, LCASS_CPY*/);
		REFRESH_UNLOCK();
		if (wa->overlay.invert)
			lInvertFrame(wa->clone);
		lRefreshAsync(wa->clone, 1);
		zerocounter++;
		return 1;
	}
#else
	if (!getUpdateSignal(wa)){
		removeUpdateSignal(wa);
		if (wa->overlay.lcdBeat && frame){
			// comment below four lines if using a display other than or along with G15
			display.left = wa->g15.beatXOffset;
			display.right = (frame->width-1)+wa->g15.beatXOffset;
			display.top = wa->g15.beatYOffset;
			display.btm = (frame->height-1)+wa->g15.beatYOffset;
		}

		REFRESH_LOCK();
		lCopyFrame(frame, wa->clone/*, LCASS_CPY*/);
		REFRESH_UNLOCK();
		if (wa->overlay.invert)
			lInvertFrame(wa->clone);
		return lRefreshAsync(wa->clone, 1);
	}
#endif
	return 0;
}

static void drawPlayerNotFound (TFRAME *frame, int font)
{
	TLPRINTR rect = {1,-1,frame->width-1,frame->height-1,0,0,0,0};
	lPrintEx(frame, &rect, font, PF_CLIPWRAP|PF_WORDWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, "Media Player not found");
}

static int getRandomFont ()
{
	int font = WVSFonts[rand()&0x07];
	if (!font)
		return getRandomFont();
	else
		return font;
}

static void g15_libusb_setLEDLevel (lDISPLAY id, int level)
{
	const static int leds[5] = {0, led1, led1|led2, led1|led2|led3, led1|led2|led3|led4};
	intptr_t val = leds[level];
	lSetDisplayOption(hw, id, lOPT_G15LU_MLEDS, &val);
}

static void setG15LedLevels (TWINAMP *wa)
{
	if (!wa->g15.status || !wa->overlay.g15Lights || !wa->g15.hDev)
		return;

	int i;
	// backlight
	if (wa->overlay.g15Lights & 0x01){
		for (i = 2; i >= 0; i--){
			if (wa->g15.ledlevel[0] > wa->config.ledkblevel[i]){
				if (wa->g15.lastkblevel != i){
					wa->g15.lastkblevel = i;
					if (wa->g15.hDev == (void*)0xFFFF){
						intptr_t val = i;
						lSetDisplayOption(hw, wa->g15.id, lOPT_G15LU_KBBRIGHTNESS, &val);
					}else{
						hid_g15_setKBBackLightlevel(wa->g15.hDev, i);
					}
				}
				break;
			}
		}
	}

	// Mx keys
	if (wa->overlay.g15Lights & 0x02){
		for (i = 4; i >= 0; i--){
			if (wa->g15.ledlevel[0] > wa->config.ledlevel[i]){
				if (wa->g15.lastledlevel != i){
					wa->g15.lastledlevel = i;
					if (wa->g15.hDev == (void*)0xFFFF)
						g15_libusb_setLEDLevel(wa->g15.id, i);
					else
						hid_g15_setLEDLevel(wa->g15.hDev, i);
				}
				break;
			}
		}
	}

	// LCD beat
	if (wa->overlay.lcdBeat){
		if (wa->g15.ledlevel[0] > wa->config.ledlevel[5] && wa->track.position.value > 4000){
			wa->g15.beatXOffset = (rand()%5)-2;
			wa->g15.beatYOffset = (rand()%5)-2;
		}else{
			wa->g15.beatXOffset = 0;
			wa->g15.beatYOffset = 0;
		}
	}
}

static void getLedLevel (TWINAMP *wa)  // compute led level
{
	if (wa->g15.status || wa->graph.barType == 6){
		const float barScaleFactor = ((float)frame->width-1.0)/wa->config.graphAudioScale;
		const float ledScale = ((float)frame->width-1.0)/wa->config.g15LEDScale;
		float total = 0;
		int x, channel;
		float last = wa->spectrum.channel1[wa->config.g15LEDLowerRange-1];

		for (channel = 0; channel < 2; channel++){
			total = 0;
			for (x = wa->config.g15LEDLowerRange; x < wa->config.g15LEDUpperRange; x++){
				if (!channel){
					total += fabs(last - wa->spectrum.channel1[x]);
					last = wa->spectrum.channel1[x];
				}else{
					total += fabs(last - wa->spectrum.channel2[x]);
					last = wa->spectrum.channel2[x];
				}
			}

			if (total > wa->config.graphAudioScale)
				wa->config.graphAudioScale = total;
			total *= barScaleFactor;
			wa->g15.ledlevel[channel] = ledScale*total;
			wa->g15.total[channel] = total;
		}
	}
}

static int renderSpecGraph (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	static int maxCount[2];
	wa->graph.x = x;
	wa->graph.y = y;

	int i = 0;
	if (wa->graph.barType == 1){
		for(i = wa->graph.x; i<wa->graph.width; i++)
			lDrawLine(frame, i, wa->graph.height + wa->graph.y, i, wa->graph.spectrum4[i], wa->graph.pixelMode);

	}else if (wa->graph.barType == 2){
		const int height = wa->graph.y + wa->graph.height-1;
		int minh;
		for (i = wa->graph.x; i<wa->graph.width-2; i+=4){
			minh = MIN(wa->graph.spectrum4[i], wa->graph.spectrum4[i+2]);
			if (minh <= frame->height-1)
				lDrawRectangleFilled(frame, i, minh, i+2, height, wa->graph.pixelMode);
		}
	}else if (wa->graph.barType == 3){
		lDrawPolyLineTo(frame, wa->graph.osc.chan1, wa->graph.width, LSP_SET);

		if (wa->overlay.channel2)
			lDrawLine(frame, frame->width>>1, 18+9, frame->width>>1, 18+9+15, LSP_SET);

	}else if (wa->graph.barType == 4){
		for (i = wa->graph.x; i<wa->graph.width-1; i++){
			lDrawLine(frame, i, wa->graph.osc.chan1[i].y, i+1, wa->graph.osc.chan1[i+1].y, LSP_SET);
			lDrawLine(frame, i, wa->graph.osc.chan1[i].y-1, i+1, wa->graph.osc.chan1[i+1].y-1, LSP_SET);
		}
		if (wa->overlay.channel2)
			lDrawLine(frame, frame->width>>1, 18+9, frame->width>>1, 18+9+15, LSP_SET);

	}else if (wa->graph.barType == 5){
		for (i = wa->graph.x; i<wa->graph.width-1; i++)
			lDrawRectangle(frame, i, wa->graph.osc.chan1[i].y, i+1, wa->graph.osc.chan1[i].y+1, LSP_SET);

		if (wa->overlay.channel2)
			lDrawLine(frame, frame->width>>1, 18+9, frame->width>>1, 18+9+15, LSP_SET);

	}else if (wa->graph.barType == 6){
		const int barheight = 6;
		int ch, tchs = 0;
		int yoffset[2];
		yoffset[0] = (frame->height-25)+10;
		yoffset[1] = yoffset[0]+barheight+2;

		if (!wa->track.Channels.value)
			return 1;
		else if (wa->track.Channels.value == 2)
			tchs = 2;
		else
			tchs = 1;

		for (ch = 0; ch < tchs; ch++){
			if (wa->overlay.vupeak){
				if (wa->graph.chMax[ch] < wa->g15.total[ch]){
					wa->graph.chMax[ch] = wa->g15.total[ch];
					if (wa->graph.chMax[ch] > frame->width-2)
						wa->graph.chMax[ch] = frame->width-2;
					maxCount[ch] = 15;
				}
			}
			lDrawRectangleFilled(frame, 0, yoffset[ch], wa->g15.total[ch], yoffset[ch]+barheight, LSP_SET);
			if (wa->overlay.vupeak){
				lDrawLine(frame, wa->graph.chMax[ch], yoffset[ch], wa->graph.chMax[ch], yoffset[ch]+barheight, LSP_XOR);
				lDrawLine(frame, wa->graph.chMax[ch]+1, yoffset[ch], wa->graph.chMax[ch]+1, yoffset[ch]+barheight, LSP_XOR);
				if (!maxCount[ch]){
					if (--wa->graph.chMax[ch] < 0)
						wa->graph.chMax[ch] = 0;
				}else{
					maxCount[ch]--;
				}
			}
		}
	}
	return 1;
}

static void processOscData (TWINAMP *wa)
{
	const int height = 10;
	float vscale = (float)height/128.0;
	const int offset = (frame->height - height) + 2;
	int x = 0, col = 0;
	int width;

	if (!wa->overlay.channel2)
		width = frame->width;
	else
		width = frame->width>>1;
	const float hscale = (float)wa->net.wave.bpc/(float)width;

	for (x = 0; x < width; x++){
		wa->graph.osc.chan1[x].x = x;
		wa->graph.osc.chan1[x].y = ((char)wa->wave.channel1[(int)((float)x*hscale)]*vscale) + offset;
	}

	if (wa->overlay.channel2){
		if (wa->track.Channels.value > 1){
			for (x = frame->width>>1; x < frame->width; x++){
				wa->graph.osc.chan1[x].x = x;
				wa->graph.osc.chan1[x].y = ((char)wa->wave.channel2[(int)((float)col++*hscale)]*vscale) + offset;
			}
		}else{
			for (x = frame->width>>1; x < frame->width; x++){
				wa->graph.osc.chan1[x].x = x;
				wa->graph.osc.chan1[x].y = offset;
			}
		}
	}
}

static int processSpectrumData (TWINAMP *wa)
{

	const int graphBottom = wa->graph.y + wa->graph.height;
	const int falloff = wa->graph.fallOff;
	const float graphHeightFactor = 256.0f/(float)wa->graph.height;
	const float barDataTimes05 = wa->net.wave.bpc/(wa->graph.width-1);
	const float barData = barDataTimes05*2.0;
	unsigned int col, k;
	float value;

	for (col = 0; col < wa->graph.width; col++){
		value = 0.0;
		for(k = col * barDataTimes05; k < (col+1) * barDataTimes05; k++)
			value += wa->spectrum.channel1[k]+wa->spectrum.channel2[k];

		value += (value*logf(col*col+1));
		wa->graph.spectrum[col] = MAX(wa->graph.spectrum[col]-falloff,(value/barData));
		wa->graph.spectrum4[col] = graphBottom-((float)wa->graph.spectrum[col]/graphHeightFactor);
	}

	return 1;
}

// set page to display
static void setActivePage (TWINAMP *wa, int frameid)
{
	getPage(wa, DISF_ACTIVEFRAME)->frame = getFrame(wa, frameid);
	wa->dis.index = frameid;
}

// return index of current page, which is the page on display
static int getActivePage (TWINAMP *wa)
{
	return wa->dis.index;
}

// return TFRAME data of current page
static TFRAME *getFrame (TWINAMP *wa, int frameid)
{
	return getPage(wa, frameid)->frame;
}

// get TDISPAGE structure of page
static TDISPAGE *getPage (TWINAMP *wa, int frameid)
{
	return &wa->dis.page[frameid];
}

static int getPageDID (TWINAMP *wa, int frameid)
{
	return wa->dis.page[frameid].dependsID;
}

static int getPageFlags (TWINAMP *wa, int frameid)
{
	return getPage(wa, frameid)->proc.flags;
}

static int getPageUpdatePeriod (TWINAMP *wa, int frameid)
{
	return getPage(wa, frameid)->proc.uperiod;
}

static int getPageLastRenderTime (TWINAMP *wa, int frameid)
{
	return getPage(wa, frameid)->lastRenderTime;
}

char *getPageTitle (TWINAMP *wa, int frameid)
{
	return getPage(wa, frameid)->title;
}

// has this page/frame got focus.
// a page will also have focus if its in the process of being rotated, input focus will be the distination page
static int isFrameActive (TWINAMP *wa, int frameNumber)
{
	return (getPageStatus(wa, frameNumber) && ((getActivePage(wa) == frameNumber) || ((wa->dis.indexLast == frameNumber) && wa->dis.direction)));
}

static int isPageRegistered (TWINAMP *wa, int frameid)
{
	return (getPage(wa, frameid)->isRegistered&0x01);
}

static int isPageOpen (TWINAMP *wa, int frameid)
{
	return (getPage(wa, frameid)->isOpen&0x01 && isPageRegistered(wa, frameid));
}

// enable/disable this page
static void setPageStatus (TWINAMP *wa, int frameid, int status)
{
	if (isPageRegistered(wa, frameid))
		getPage(wa, frameid)->isEnabled = status&0x01;
}

// get whether page is enabled or not
static int getPageStatus (TWINAMP *wa, int frameid)
{
	return (getPage(wa, frameid)->isEnabled&0x01 && isPageRegistered(wa, frameid));
}

// get index of next page 
static int getNextPage (TWINAMP *wa, int frameid)
{
	frameid++;
	if (frameid > wa->dis.total)
		frameid = 1;
	if (!getPageStatus(wa, frameid))
		frameid = getNextPage(wa, frameid);
	else if ((getPageDID(wa, frameid) == DISF_MAIN || frameid == DISF_MAIN) && wa->net.notFound)
		frameid = getNextPage(wa, frameid);
	
	return frameid;
}

// get index of previous page
static int getPrevPage (TWINAMP *wa, int frameid)
{
	frameid--;
	if (frameid < 1)
		frameid = wa->dis.total;
	if (!getPageStatus(wa, frameid))
		frameid = getPrevPage(wa, frameid);
	else if ((getPageDID(wa, frameid) == DISF_MAIN || frameid == DISF_MAIN) && wa->net.notFound)
		frameid = getPrevPage(wa, frameid);
		
	return frameid;
}

static void onPageChange (TWINAMP *wa)
{
	setUpdateSignal(wa);
	signalPageExit(wa, wa->dis.indexLast);
	signalPageEnter(wa, wa->dis.index);
}

static void signalPageExit (TWINAMP *wa, int pageIndex)
{
	if (pageIndex <= DISF_TOTAL){
		TDISPAGE *page = getPage(wa, pageIndex);
		if (page){
			if (page->proc.exit){
				dbprintf("page exiting '%s'\n", page->title);
				page->proc.exit(page->userPtr);
			}
		}
	}
}

int signalPageEnter (TWINAMP *wa, int pageIndex)
{
	if (pageIndex <= DISF_TOTAL){
		TDISPAGE *page = getPage(wa, pageIndex);
		if (page){
			if (!page->isOpen){
				if (!signalPageOpen(wa, pageIndex))
					return 0;
			}
			if (page->proc.enter){
				dbprintf("page entering '%s'\n", page->title);
				int ret = page->proc.enter(page->work, page->userPtr);
				REFRESH_LOCK();
				lCopyFrame(page->work, page->frame/*, LCASS_CPY*/);
				REFRESH_UNLOCK();
				return ret;
			}
		}
	}
	return 0;
}

static int signalPageOpen (TWINAMP *wa, int pageIndex)
{
	if (pageIndex <= DISF_TOTAL){
		TDISPAGE *page = getPage(wa, pageIndex);
		if (page){
			if (page->dependsID){
				if (!isPageOpen(wa, page->dependsID)){
					dbprintf("opening required page '%s' for page '%s'\n", getPageTitle(wa, page->dependsID), page->title);
					if (!signalPageOpen(wa, page->dependsID))
						return 0;
				}
			}
			if (page->proc.open && !page->isOpen){
				dbprintf("page open '%s'\n", page->title);
				int ret = page->proc.open(page->work, page->userPtr);
				REFRESH_LOCK();
				lCopyFrame(page->work, page->frame/*, LCASS_CPY*/);
				REFRESH_UNLOCK();
				if (ret){
					page->isOpen = 1;
					return 1;
				}else{
					dbprintf("page open failed for '%s'\n", page->title);
					page->isOpen = 0;
					//renderErrorPage(wa, getFrame(wa, pageIndex), P_OPNFAILED, page->title);
					return 0;
				}
			}
		}
	}
	return -1;
}

static int signalPageClose (TWINAMP *wa, int pageIndex)
{
	if (pageIndex <= DISF_TOTAL){
		TDISPAGE *page = getPage(wa, pageIndex);
		if (page){
			if (page->proc.close){
				dbprintf("page close '%s'\n", page->title);
				page->proc.close(page->userPtr);
				//dbprintf("page close '%s' out\n", page->title);
				page->isClosed = 1;
			}
		}
	}
	return 1;
}

static int gotoPrevPage (TWINAMP *wa)
{
	if (wa->dis.direction == 0)
		wa->dis.angle = 360.0;
	wa->dis.directionLast = wa->dis.direction;
	wa->dis.direction = 2;
	wa->dis.indexLast = wa->dis.index;
	wa->dis.index = getPrevPage(wa, wa->dis.index);
	return wa->dis.index;
}

static int gotoNextPage (TWINAMP *wa)
{
	wa->dis.directionLast = wa->dis.direction;
	wa->dis.direction = 1;
	wa->dis.indexLast = wa->dis.index;
	wa->dis.index = getNextPage(wa, wa->dis.index);
	return wa->dis.index;
}
/*
void forceFrameUpdates (TWINAMP *wa)
{
	int i;
	for (i = DISF_FIRST; i <= DISF_TOTAL; i++){
		setActivePage(wa, i);
		renderFrames(wa);
	}
	defaultPageReset(wa, wa->config.startPage);
}
*/
static int defaultPageReset (TWINAMP *wa, int desPage)
{
	if (desPage !=  wa->dis.index){
		wa->dis.angle = 0.0;
		wa->dis.direction = 0;
		wa->dis.total = DISF_TOTAL;
		wa->dis.indexLast = wa->dis.index;
		wa->dis.index = desPage;		// set active display on start up
		if (!getPageStatus(wa, desPage))
			desPage = gotoNextPage(wa);
		setActivePage(wa, desPage);
		return 1;
	}else{
		return 0;
	}
}

// wvs.cfg
static int configLoad (TWINAMP *wa, char *cfgpath)
{
	int ret = configProcessFile(wa, cfgpath);
	memcpy(&wa->font, &wa->config.font, sizeof(TFONTS));
	memcpy(&wa->overlay, &wa->config.overlay, sizeof(TDISSTATE));
	//defaultPageReset(wa, wa->config.startPage);
	volumeSetActive(wa, wa->config.volumeCtrl);

	if (wa->overlay.g15Lights && wa->g15.hDev /*== NULL*/){
		//wa->g15.hDev = hid_g15_open(wa->g15.deviceNumber);
		//if (wa->g15.hDev == NULL){
		//	wa->overlay.g15Lights = 0;
		//}else{
			hid_g15_getState(wa->g15.hDev, wa->g15.states);
		//}
	}
	return ret;
}

static int getFrapsFontType (TWINAMP *wa)
{
	return wa->fraps.fontType;
}

static void setFrapsFontType (TWINAMP *wa, int type)
{
	if (type == 0){			// small
		wa->font.frapsFPS = LFTW_5x7;
		wa->fraps.fontType = type;
	}else if (type == 1){	//medium
		wa->font.frapsFPS = LFTW_SNAP;
		wa->fraps.fontType = type;
	}else if (type == 2){	// cjkv
		wa->font.frapsFPS = LFTW_WENQUANYI9PT;
		wa->fraps.fontType = type;
	}else if (type == 3){	// large
		wa->font.frapsFPS = LFTW_ROUGHT18;
		wa->fraps.fontType = type;
	}
}


int renderErrorPage (TFRAME *frame, int err, char *msg)
{
	const int flags = PF_CLIPWRAP|PF_MIDDLEJUSTIFY|PF_WORDWRAP;
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);
	lSetCharacterEncoding(frame->hw, CMT_CP850);
	switch(err){
	  case P_OPNFAILED:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "An error occurred when opening page %s", msg);
		break;
	  case P_RNDFAILED:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "An error occurred when rendering page '%s'", msg);
		break;
	  case P_ENTFAILED:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "An error occurred when entering page '%s'", msg);
		break;
	  case P_EXTFAILED:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "An error occurred when exiting page '%s'", msg);
		break;
	  case P_CLSFAILED:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "An error occurred when closing page '%s'", msg);
		break;
	  case P_INPFAILED:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "An error occurred when processing an input request for page '%s'", msg);
		break;
	  default:
		lPrintEx(frame, &rect, LFTW_5x7, flags, LPRT_CPY,\
		  "who knows what just happened");
	}
	return 1;
}

static int registerPage (TWINAMP *wa, char *title, TPAGEFUNCS *rpage, void *userPtr, int pageID, int dependsID)
{
	if (pageID <= DISF_TOTAL){
		TDISPAGE *page = getPage(wa, pageID);
		if (page){
			page->work = lNewFrame(frame->hw, frame->width, frame->height, frame->bpp);
			page->frame = lNewFrame(frame->hw, frame->width, frame->height, frame->bpp);
			if (page->frame && page->work){
				page->proc.input = rpage->input;
				page->proc.render = rpage->render;
				page->proc.open = rpage->open;
				page->proc.close = rpage->close;
				page->proc.enter = rpage->enter;
				page->proc.exit = rpage->exit;
				page->proc.uperiod = rpage->uperiod;
				page->proc.flags = rpage->flags;
				
				page->title = title;
				page->pageID = pageID;
				page->dependsID = dependsID;
				page->userPtr = userPtr;
				page->lastRenderTime = 0;
				page->isEnabled = 1;
				page->isOpen = 0;
				page->isClosed = 0;
				page->isRegistered = 1;
				return 1;
			}
		}
	}
	return 0;
}
	
void registerPages (TWINAMP *wa)
{
	TPAGEFUNCS page;
	
	page.input = titlePageInput;
	page.render = titlePageRender;
	page.open = titlePageOpen;
	page.close = titlePageClose;
	page.enter = titlePageEnter;
	page.exit = titlePageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 0;
	registerPage(wa, "Main", &page, wa, DISF_MAIN, 0);
	
	page.input = lyricsPageInput;
	page.render = lyricsPageRender;
	page.open = lyricsPageOpen;
	page.close = lyricsPageClose;
	page.enter = lyricPageEnter;
	page.exit = lyricPageExit;
	page.flags = RENDER_ALWAYS;
	page.uperiod = 250;
	registerPage(wa, "Lyrics", &page, wa, DISF_LYRICS, DISF_MAIN);

	page.input = eqPageInput;
	page.render = eqPageRender;
	page.open = eqPageOpen;
	page.close = eqPageClose;
	page.enter = eqPageEnter;
	page.exit = eqPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "EQ", &page, &wa->eq, DISF_EQ, DISF_MAIN);

	page.input = bwPageInput;
	page.render = bwPageRender;
	page.open = bwPageOpen;
	page.close = bwPageClose;
	page.enter = bwPageEnter;
	page.exit = bwPageExit;
	page.flags = RENDER_ALWAYS;
	page.uperiod = 1000;
	registerPage(wa, "Bandwidth", &page, NULL, DISF_BANDWIDTH, 0);
	
	page.input = metatagPageInput;
	page.render = metatagPageRender;
	page.open = metatagPageOpen;
	page.close = metatagPageClose;
	page.enter = metatagPageEnter;
	page.exit = metatagPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Meta tags", &page, wa, DISF_META, DISF_MAIN);

	page.input = playlistPageInput;
	page.render = playlistPageRender;
	page.open = playlistPageOpen;
	page.close = playlistPageClose;
	page.enter = playlistPageEnter;
	page.exit = playlistPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 200;
	registerPage(wa, "Playlist", &page, &wa->playlist, DISF_PLAYLIST, DISF_MAIN);

	page.input = hitlistPageInput;
	page.render = hitlistPageRender;
	page.open = hitlistPageOpen;
	page.close = hitlistPageClose;
	page.enter = hitlistPageEnter;
	page.exit = hitlistPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 200;
	registerPage(wa, "Hitlist", &page, &wa->hitlist, DISF_HITLIST, DISF_MAIN);
	
	page.input = utPageInput;
	page.render = utPageRender;
	page.open = utPageOpen;
	page.close = utPageClose;
	page.enter = utPageEnter;
	page.exit = utPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "æTorrent", &page, wa, DISF_UTORRENT, 0);
			
	page.input = nsPageInput;
	page.render = nsPageRender;
	page.open = nsPageOpen;
	page.close = nsPageClose;
	page.enter = nsPageEnter;
	page.exit = nsPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Netstat", &page, &nsOption, DISF_NETSTAT, 0);
		
	page.input = proclistPageInput;
	page.render = proclistPageRender;
	page.open = proclistPageOpen;
	page.close = proclistPageClose;
	page.enter = proclistPageEnter;
	page.exit = proclistPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Process list", &page, &wa->proclist, DISF_PROCLIST, 0);
		
	page.input = sysinfoPageInput;
	page.render = sysinfoPageRender;
	page.open = sysinfoPageOpen;
	page.close = sysinfoPageClose;
	page.enter = sysinfoPageEnter;
	page.exit = sysinfoPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "System info", &page, &sysinfo, DISF_SYSINFO, 0);

	page.input = attPageInput;
	page.render = attPageRender;
	page.open = attPageOpen;
	page.close = attPageClose;
	page.enter = attPageEnter;
	page.exit = attPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Ati Tray Tools", &page, &wa->config.atitemp, DISF_ATT, 0);

	page.input = frapsPageInput;
	page.render = frapsPageRender;
	page.open = frapsPageOpen;
	page.close = frapsPageClose;
	page.enter = frapsPageEnter;
	page.exit = frapsPageExit;
	page.flags = RENDER_ALWAYS;
	page.uperiod = 1000;
	registerPage(wa, "Fraps", &page, wa, DISF_FRAPS, 0);

	page.input = rssPageInput;
	page.render = rssPageRender;
	page.open = rssPageOpen;
	page.close = rssPageClose;
	page.enter = rssPageEnter;
	page.exit = rssPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 70;
	registerPage(wa, "RSS 2.0", &page, wa, DISF_RSS, 0);
	
	page.input = amsnPageInput;
	page.render = amsnPageRender;
	page.open = amsnPageOpen;
	page.close = amsnPageClose;
	page.enter = amsnPageEnter;
	page.exit = amsnPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "aMSN", &page, wa, DISF_MSN, 0);

	page.input = mystPageInput;
	page.render = mystPageRender;
	page.open = mystPageOpen;
	page.close = mystPageClose;
	page.enter = mystPageEnter;
	page.exit = mystPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 45;
	registerPage(wa, "Mystify", &page, wa, DISF_MYSTIFY, 0);

	page.input = partPageInput;
	page.render = partPageRender;
	page.open = partPageOpen;
	page.close = partPageClose;
	page.enter = partPageEnter;
	page.exit = partPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 15;
	registerPage(wa, "Particles", &page, wa, DISF_PARTICLES, 0);

	page.input = pingPageInput;
	page.render = pingPageRender;
	page.open = pingPageOpen;
	page.close = pingPageClose;
	page.enter = pingPageEnter;
	page.exit = pingPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Ping", &page, wa, DISF_PING, 0);	

	page.input = calPageInput;
	page.render = calPageRender;
	page.open = calPageOpen;
	page.close = calPageClose;
	page.enter = calPageEnter;
	page.exit = calPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Calendar", &page, wa, DISF_CALENDAR, 0);

	page.input = abtPageInput;
	page.render = abtPageRender;
	page.open = abtPageOpen;
	page.close = abtPageClose;
	page.enter = abtPageEnter;
	page.exit = abtPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "About", &page, NULL, DISF_ABOUT, 0);
	
	page.input = cfgPageInput;
	page.render = cfgPageRender;
	page.open = cfgPageOpen;
	page.close = cfgPageClose;
	page.enter = cfgPageEnter;
	page.exit = cfgPageExit;
	page.flags = RENDER_FOCUS;
	page.uperiod = 1000;
	registerPage(wa, "Config", &page, &wa->cfPage, DISF_CONFIG, 0);
}

int initWVS (TWINAMP *wa)
{
	if (wa == NULL){
		dbprintf("initWVS(): invalid pointer\n");
		return 0;
	}

	wa->hw = hw;
	hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);

	wa->clone = lCloneFrame(frame);
	if (wa->clone == NULL)
		return 0;
	wa->state = 0;
	wa->ping.address = NULL;
	wa->net.notFound = 1; // 1 = not found,  0 = found
	wa->net.foundOnce = 0;
	wa->misc.timerUpdateCounter = 0xFFFF;

	// buffersize-n will prevent a buffer overrun if net_getStructType()
	// reads past end of buffer when determining type
	wa->net.bufferSize = NETREADBUFFERSIZE;
	wa->net.buffer = calloc(1, wa->net.bufferSize);
	if (wa->net.buffer == NULL)
		return 0;

	wa->net.port = TCPPORT;
	wa->net.address = (ubyte*)strdup("127.0.0.1");
	wa->net.conn.socket.server = SOCKET_ERROR;
	wa->net.wave.bpc = MAXCHANNELLENGTH;
	wa->net.wave.sRate = 44100;
	wa->net.wave.channels = 1;
	wa->net.version.v1 = 0;
	wa->net.version.v2 = 0;
	wa->net.version.v3 = 0;
	wa->net.version.vMP = 0;
	wa->net.protocol = 0;		// set default. 0=udp, 1=tcp
	wa->net.cfgProto = -1;
	//strcpy((char*)wa->net.mpName, "Media Player");

	wa->spectrum.channel1 = &wa->net.wave.data[0];
	wa->spectrum.channel2 = &wa->net.wave.data[MAXCHANNELLENGTH];
	wa->wave.channel1 = &wa->net.wave.data[MAXCHANNELLENGTH*2];
	wa->wave.channel2 = &wa->net.wave.data[MAXCHANNELLENGTH*3];

	wa->meta.tag = 0;
	wa->meta.track = 0;

	// set defaults
	wa->fraps.position = FRAPSPOS_BOTTOMRIGHT;
	wa->fraps.distination = 0;	// 0 = all
	wa->fraps.copyMode = LCASS_CPY;
	wa->fraps.fontType = 1;

	wa->playlist.fontType = 6;
	wa->playlist.scrollpercent = 5; // percent to scroll when shift+wheel is used
	wa->playlist.start = 0;
	wa->playlist.end = 0;
	wa->playlist.highlightPos = 0;
	wa->playlist.totalTracks = -1;
	wa->playlist.type = 0;	// list type. 0=mediaplayer playlist, 1=hitlist 1 ... 9=hitlist 9
	wa->playlist.al = NULL;
	wa->playlist.hFile = NULL;
	wa->playlist.wa = wa;

	wa->hitlist.fontType = 6;
	wa->hitlist.scrollpercent = 5; // pc to scroll by when shift+wheel is used
	wa->hitlist.start = 0;
	wa->hitlist.end = 0;
	wa->hitlist.highlightPos = 0;
	wa->hitlist.totalTracks = -1;
	wa->hitlist.type = 1;	// list type. 0=mediaplayer playlist, 1=hitlist No.1 ... 9=hitlist No.9
	wa->hitlist.al = NULL;
	wa->hitlist.hFile = NULL;
	wa->hitlist.wa = wa;

	//wa->trackText = 1; //0 = title, 1 = filename

	wa->graph.chMax[0] = 0;
	wa->graph.chMax[1] = 0;
	wa->graph.width = frame->width;
	wa->graph.height = 25;
	wa->graph.fps = 21;
	wa->graph.barType = 1;
	wa->graph.pixelMode = LSP_XOR;
	wa->graph.updateCount = 1;
	wa->graph.fallOff = 22;

	wa->misc.winampVolumeTime = 0;
	wa->misc.lag = 0;
	wa->misc.slideOffset = 0;
	wa->misc.hitlistr = 0;
	wa->misc.seeked = 1;
	wa->misc.updateSignal = 0;

	wa->cfPage.item = configopt;
	wa->cfPage.start = 0;
	wa->cfPage.end = 0;
	wa->cfPage.highlight = 0;
	wa->cfPage.hlmode = 0;
	wa->cfPage.wa = wa;

	wa->eq.mode = 0;
	wa->eq.bar = 0;
	wa->eq.direction = 0;

	wa->g15.status = 0;
	wa->g15.hDev = NULL;
	wa->g15.key = 0;
	wa->g15.total[0] = 0;
	wa->g15.total[1] = 0;
	wa->g15.ledlevel[0] = 0;
	wa->g15.ledlevel[1] = 0;
	wa->g15.lastledlevel = 1;
	wa->g15.lastkblevel = 1;
	wa->g15.deviceNumber = 1;
	wa->g15.upperBand = LightUpperBand;	//
	wa->g15.lowerBand = LightLowerBand;	// both are defined in wvs.h
	wa->g15.beatXOffset = 0;
	wa->g15.beatYOffset = 0;

	wa->mixer[VOLUME_MASTER].volumeTime = 0;
	wa->mixer[VOLUME_MASTER].status = 1;			// set default state to disabled
	wa->mixer[VOLUME_MASTER].devicei = 0;
	wa->mixer[VOLUME_WAVE].volumeTime = 0;
	wa->mixer[VOLUME_WAVE].status = 1;			// set default state to disabled
	wa->mixer[VOLUME_WAVE].devicei = 0;
	wa->mixer[VOLUME_MEDIAPLAYER].volumeTime = 0;
	wa->mixer[VOLUME_MEDIAPLAYER].status = 1;			// set default state to disabled
	wa->mixer[VOLUME_MEDIAPLAYER].devicei = 0;

	wa->proclist.modStart = 1;
	wa->proclist.modTotal = 1;
	wa->proclist.columnMode = 0;
	wa->proclist.highlight = 1;
	wa->proclist.subMode = 0;
	wa->proclist.killp = 0;

	wa->config.overlay.trackPos = 1;
	wa->config.overlay.trackInfo = 1;
	wa->config.overlay.scroll = 1;
	wa->config.overlay.channel2 = 0;
	wa->config.overlay.trackBps = 0;
	wa->config.overlay.trackKhz = 0;
	wa->config.overlay.playlistPos = 1;
	wa->config.overlay.trackNumber = 1;
	wa->config.overlay.invert = 0;
	wa->config.overlay.trackSeek = 0;
	wa->config.overlay.vupeak = 1;
	wa->config.overlay.trackFav = 0;
	wa->config.overlay.volumeLevel = 0;
	wa->config.overlay.g15Lights = 1;
	wa->config.overlay.bwClock = 1;
	wa->config.overlay.bwSplit = 1;
	wa->config.overlay.frapsFPS = 0;
	wa->config.overlay.lcdBeat = 0;
	wa->config.overlay.spectMarkers = 0;
	wa->config.overlay.bwSplit = 0;
	wa->config.overlay.clockFormat = 1;
	wa->config.overlay.g15LightsBackup = 0;
	//wa->config.overlay.lyricsOvrlay = 0;

	wa->config.font.titleScroll = LFTW_B16B;
	wa->config.font.trackNumber = LFT_COMICSANSMS7X8;
	wa->config.font.trackPosition = LFT_COMICSANSMS7X8;
	wa->config.font.trackLength = LFT_COMICSANSMS7X8;
	wa->config.font.samRate = LFT_SMALLFONTS7X7;
	wa->config.font.bitRate = LFT_SMALLFONTS7X7;
	wa->config.font.about = LFTW_SNAP;
	wa->config.font.frapsFPS = LFTW_SNAP;

	// set some default values in the event of configProcessFile() failing.
	wa->config.scrollX = 0;
	wa->config.scrollY = 0;
	wa->config.encoding = CMT_BIG5;
	wa->config.mystifyNodes = 10;
	wa->config.trackTitleScrollPeriod = 70;
	wa->config.rssScrollPeriod = 70;
	wa->config.rssUpdatePeriod = 60*1000;
	wa->config.rotateFlipStep = 8;		// is an index
	wa->config.tparticles = 3000;
	wa->config.trackSkipTime = 3000;
	wa->config.volumeTimeout = 3000;
	wa->config.pageSwapMode = 2;
	wa->config.atitemp = 0;
	wa->config.sysInfoUnit = 0;
	wa->config.sysInfoShowUnit = 0;
	wa->config.bwinterface = 0;
	wa->config.startPage = DISF_PLAYLIST;
	wa->config.volumeCtrl = VOLUME_MASTER;
	wa->config.exitWithPlayer = 1;
	wa->config.waitForPlayer = 0;
	wa->config.processPriority = WVSProcessPriority[2];	//normal
	wa->config.autoUTF16 = 1;
	wa->config.g15LEDLowerRange = 20;
	wa->config.g15LEDUpperRange = 288;
	wa->config.graphAudioScale = 1.0;
	wa->config.g15LEDScale = 110.0;
	wa->config.ledlevel[0] = 0.0;
	wa->config.ledlevel[1] = 25.0;
	wa->config.ledlevel[2] = 50.0;
	wa->config.ledlevel[3] = 75.0;
	wa->config.ledlevel[4] = 98.0;
	wa->config.ledlevel[5] = 140.0;
	wa->config.ledkblevel[0] = 0.0;
	wa->config.ledkblevel[1] = 55.0;
	wa->config.ledkblevel[2] = 95.0;
	wa->config.mediaPlayDisabled = 0;
	wa->config.handleMediaKeys = 1;
	wa->config.reverseVolumeCtrl = 0;
	wa->config.volumeSensi = 1;		// 0=low, 1=mod, 2=high
	wa->config.alwaysUseUTF8 = 1;		// only relates to meta page filepaths
	wa->config.autoPageSwap = 0;

	time_t t;
    srand((unsigned int)time(&t));	// rand() is used in getRandomFont() and setG15LedLevels()

	//lCacheCharacterRange(0, 255, LFTW_B16B);
	//lCacheCharacterRange(0, 255, LFTW_SNAP);
	//lCacheCharacterRange(0, 128, LFTW_WENQUANYI9PT);
	//lCacheCharacterRange(wa->hw, 0, 128, LFTW_5x7);
	
	//if (WVSFonts[wa->playlist.fontType] != LFTW_5x7)
		//lCacheCharacterRange(wa->hw, 0, 128, WVSFonts[wa->playlist.fontType]);
	return 1;
}

void initPages (TWINAMP *wa)
{
	resetPageSwapTime(wa);
	signalPageEnter(wa, wa->dis.index);
	
	if (getPageStatus(wa, DISF_MSN))
		signalPageOpen(wa, DISF_MSN);
	if (getPageStatus(wa, DISF_RSS))	
		signalPageOpen(wa, DISF_RSS);
	if (getPageStatus(wa, DISF_MAIN))
		signalPageOpen(wa, DISF_MAIN);
}

DWORD g15keycb (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15)
{
	TWINAMP *wa = (TWINAMP*)mylcdg15->ptrUser;
	if (wa != NULL){
		if (!wa->state){
			PostMessage(wa->hook.hwnd, 5000, dwButtons|1024, (LPARAM)wa);
			return 1;
		}
	}
	return 0;
}

static int initG15 (TWINAMP *wa, DWORD ptrG15keyCB (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15))
{
	wa->g15.id = lDriverNameToID(hw, "g15", LDRV_DISPLAY);
	if (wa->g15.id){
		intptr_t val = G15_PRIORITY_SYNC;
		lSetDisplayOption(hw, wa->g15.id, lOPT_G15_PRIORITY, &val);
		
	}else{
		wa->g15.id = lDriverNameToID(hw, "g15:libusb", LDRV_DISPLAY);
		if (wa->g15.id)
			wa->g15.hDev = (void*)0xFFFF;
	}
		
	if (wa->g15.id){
		TMYLCDG15 *mylcdg15 = NULL;
		lGetDisplayOption(hw, wa->g15.id, lOPT_G15_STRUCT, (void *)&mylcdg15);
		if (mylcdg15 != NULL){
			mylcdg15->ptrUser = (void*)wa;
			lSetDisplayOption(hw, wa->g15.id, lOPT_G15_SOFTKEYCB, (intptr_t*)ptrG15keyCB);
			wa->g15.status = 1;

			if (wa->overlay.g15Lights && wa->g15.hDev == NULL){
				wa->g15.hDev = hid_g15_open(wa->g15.deviceNumber);
				if (wa->g15.hDev == NULL){
					wa->overlay.g15Lights = 0;
				}
			}
			if (wa->g15.hDev){
				wa->g15.MLeds = hid_g15_getLEDLevel(wa->g15.hDev);
				wa->g15.KBLeds = hid_g15_getKBBackLightlevel(wa->g15.hDev);
				wa->g15.LCDC = hid_g15_getLCDBackLightlevel(wa->g15.hDev);
				hid_g15_setContrastLevel(wa->g15.hDev, 3); // improve clarity
			}
			return 1;
		}else{
			// mylcd g15 driver has previously failed to start
			wa->g15.status = 0;
			//MessageBox(NULL, "myLCD could not initiate the G15 driver.\nEnsure all is well.", "WVS", MB_OK|MB_SYSTEMMODAL);
			return 0;
		}
	}else{
		// g15 not in use, not connected or mylcd driver support not compiled in
		//MessageBox(NULL, "G15 not activated, not connected or myLCD driver support not compiled in.\nCheck G15 has been enabled in mylcd.cfg.", "WVS: G15 not found", MB_OK|MB_SYSTEMMODAL);
		wa->g15.status = 0;
		return -1;
	}
}

void waCleanup (TWINAMP *wa)
{
	
	TPLAYLIST *list = (TPLAYLIST *)getPage(wa, DISF_HITLIST)->userPtr;
	if (list){
		if (list->scroll)
			lDeleteScroll(list->scroll);
		if (list->frame)
			lDeleteFrame(list->frame);
	}
	list = (TPLAYLIST *)getPage(wa, DISF_PLAYLIST)->userPtr;
	if (list){
		if (list->scroll)
			lDeleteScroll(list->scroll);
		if (list->frame)
			lDeleteFrame(list->frame);
	}
	int i;
	for (i = 1; i <= wa->dis.total; i++){
		lDeleteFrame(wa->dis.page[i].frame);
		wa->dis.page[i].frame = NULL;
		lDeleteFrame(wa->dis.page[i].work);
		wa->dis.page[i].work = NULL;
	}
	
	if (wa->graph.osc.chan1)
		free(wa->graph.osc.chan1);
	if (wa->graph.spectrum)
		free(wa->graph.spectrum);
	if (wa->graph.spectrum4)
		free(wa->graph.spectrum4);
	if (wa->config.langpath)
		free(wa->config.langpath);
	if (wa->config.rss.url)
		free(wa->config.rss.url);
	if (wa->ping.address)
		free(wa->ping.address);
	if (wa->net.address)
		free(wa->net.address);
	if (wa->lyricsLocation)
		free(wa->lyricsLocation);
	if (wa->config.utorrent.username)
		free(wa->config.utorrent.username);
	if (wa->config.utorrent.password)
		free(wa->config.utorrent.password);
	if (wa->net.buffer)
		free(wa->net.buffer);

	if (wa->clone)
		lDeleteFrame(wa->clone);
	if (wa->track.number.text)
		lDeleteFrame(wa->track.number.text);
	if (wa->track.bitRate.text)
		lDeleteFrame(wa->track.bitRate.text);
	if (wa->track.favorite.text)
		lDeleteFrame(wa->track.favorite.text);
	if (wa->track.sampleRate.text)
		lDeleteFrame(wa->track.sampleRate.text);
	if (wa->track.length.text)
		lDeleteFrame(wa->track.length.text);
	if (wa->track.position.text)
		lDeleteFrame(wa->track.position.text);
	if (wa->track.seekPosition.text)
		lDeleteFrame(wa->track.seekPosition.text);
	if (wa->fraps.fps.text)
		lDeleteFrame(wa->fraps.fps.text);
	if (wa->scroll.frame)
		lDeleteFrame(wa->scroll.frame);
	if (wa->scroll.s)
		lDeleteScroll(wa->scroll.s);

	cache_FreeAll(&wa->cache);

	for (i = 1; i <= DISF_TOTAL; i++){
		if (isPageOpen(wa, i))
			signalPageClose(wa, i);
	}

	lng_freeTable(wa->lang);
	
	wa->config.langpath = NULL;
	wa->clone = NULL;
	wa->ping.address = NULL;
	wa->fraps.fps.text = NULL;
	wa->config.rss.url = NULL;
	wa->net.address = NULL;
	wa->lyricsLocation = NULL;
	wa->config.utorrent.password = NULL;
	wa->config.utorrent.username = NULL;
	wa->track.sampleRate.text = NULL;
	wa->track.position.text = NULL;
	wa->track.bitRate.text = NULL;
	wa->track.length.text = NULL;
	wa->track.number.text = NULL;
	wa->graph.osc.chan1 = NULL;
	wa->graph.spectrum4 = NULL;
	wa->graph.spectrum = NULL;
	wa->scroll.frame = NULL;
	wa->scroll.s = NULL;
}

static int isYes (char *text)
{
	if (!strnicmp(text,"on",2))
		return 1;
//	else if (!strnicmp(text,"an",2))		// german:on
//		return 1;
//	else if (!strnicmp(text,"ja",2))		// german:yes
//		return 1;
	else if (!strnicmp(text,"yes",3))
		return 1;
//	else if (!strnicmp(text,"aktiv",5))		// german:enabled
//		return 1;
	else if (!strnicmp(text,"enabled",7))
		return 1;
	else
		return 0;
}

static int configProcessFile (TWINAMP *wa, char *configpathfile)
{

	TASCIILINE *al = readFileA(configpathfile);
	if (al == NULL){
		//dbprintf("WVS: config file %s not found\n", configpathfile);
		return 0;
	}

	int i = 0;
	char *line = NULL;
	char *rep = NULL;
	do{
		line = (char*)al->lines[i];
		rep = (char*)strchr(line, ' ');
		if (rep) *rep = 0;
		rep = (char*)strchr(line, ';');
		if (rep) *rep = 0;
		rep = (char*)strchr(line, '\t');
		if (rep) *rep = 0;

		if (!strncmp(line, "rss_url=", 8)){
			if (wa->config.rss.url)
				free(wa->config.rss.url);
			wa->config.rss.url = (ubyte*)strdup(line+8);

		}else if (!strncmp(line, "ping_address=", 13)){
			if (wa->ping.address)
				free(wa->ping.address);
			wa->ping.address = (ubyte*)strdup(line+13);
			
		}else if (!strncmp(line, "utorrent_port=", 14)){
			wa->config.utorrent.port = atoi(line+14);
			//printf("ætorrent_port %i '%s'\n",wa->config.utorrent.port, line+14);

		}else if (!strncmp(line, "language_file=", 14)){
			if (wa->config.langpath != NULL)
				free(wa->config.langpath);
			#if defined (BUILDING_DLL)
			  wa->config.langpath = (char*)calloc(sizeof(char), strlen(line+14)+16);
			  strcpy(wa->config.langpath, "wvs/");
			  strcat(wa->config.langpath, line+14);
			#else
			  wa->config.langpath = (char*)strdup(line+14);
			#endif
			
			if (wa->config.langpath == NULL)
				wa->config.langpath = (char*)strdup(DEFAULTLANG);
							
		}else if (!strncmp(line, "lyrics_location=", 16)){
			if (wa->lyricsLocation != NULL)
				free(wa->lyricsLocation);
			wa->lyricsLocation = (char*)strdup(line+16);
			if (wa->lyricsLocation == NULL)
				wa->lyricsLocation = (char*)strdup("c:\\lyrics");
				
		}else if (!strncmp(line, "waveAudioDevice=", 16)){
			wa->mixer[VOLUME_WAVE].devicei = atoi(line+16);

		}else if (!strncmp(line, "rss_scrollPeriod=", 17)){
			sscanf(line+17, "%d", &wa->config.rssScrollPeriod);

		}else if (!strncmp(line, "rss_updatePeriod=", 17)){
			sscanf(line+17, "%d", &wa->config.rssUpdatePeriod);

		}else if (!strncmp(line, "mediaplayer_port=", 17)){
			sscanf(line+17, "%d", &wa->net.port);

		}else if (!strncmp(line, "masterAudioDevice=", 18)){
			wa->mixer[VOLUME_MASTER].devicei = atoi(line+18);
			
		}else if (!strncmp(line, "utorrent_username=", 18)){
			wa->config.utorrent.username = strdup(line+18);
			//printf("ætorrent_username '%s'\n", line+18);
			
		}else if (!strncmp(line, "utorrent_password=", 18)){
			wa->config.utorrent.password = strdup(line+18);
			//printf("ætorrent_password '%s'\n", line+18);
			
		}else if (!strncmp(line, "amsn_emailaddress=", 18)){
			strncpy(wa->amsn.email, line+18, MSNSIZE_EMAILADDRESS);

		}else if (!strncmp(line, "mediaplayer_address=", 20)){
			if (wa->net.address)
				free(wa->net.address);
			wa->net.address = (ubyte*)strdup(line+20);
			if (wa->net.address == NULL)
				wa->net.address = (ubyte*)strdup("127.0.0.1");

		}else if (!strncmp(line, "amsn_remotepassword=", 20)){
			strncpy(wa->amsn.remotePassword, line+20, MSNSIZE_REMOTEPASSWORD);

		}else if (!strncmp(line, "trackTitleScrollPeriod=", 23)){
			sscanf(line+23, "%d", &wa->config.trackTitleScrollPeriod);
		}
	}while(++i < al->tlines);

	freeASCIILINE(al);
	return 1;
}

static void killGraphTimer (TWINAMP *wa)
{
	if (wa->graph.timerID){
		timeKillEvent(wa->graph.timerID);
		wa->graph.timerID = 0;
		wa->misc.timerUpdateCounter = 0;
	}

	if (wa->misc.engineTimerID){
		timeKillEvent(wa->misc.engineTimerID);
		wa->misc.engineTimerID = 0;
	}

	if (wa->misc.IUTimerID){
		timeKillEvent(wa->misc.IUTimerID);
		wa->misc.IUTimerID = 0;
	}
	timeEndPeriod(5);
}


/*
static HANDLE hTimer = NULL;
static HANDLE hTimerQueue = NULL;

void CALLBACK EngineTickerCB2(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (lpParam){
		TWINAMP *wa = (TWINAMP *)lpParam;
		PostMessage(wa->hook.hwnd, 4010, 0, (LPARAM)wa);
	}
}
*/

static int initGraphTimer (TWINAMP *wa, int rate)
{
	if (rate < 10){
		//dbprintf("WVS: invalid rate supplied %i\n", rate);
		return 0;
	}

	timeBeginPeriod(5);
	wa->graph.fps = rate--;
	wa->graph.timerID = (int)timeSetEvent((1.0/(float)rate)*1000.0, 20, TitleTickerCB, (DWORD)wa, TIME_PERIODIC);
	//wa->graph.timerID = (int)SetTimer(wa->hook.hwnd, 4000, (1.0/(float)rate)*1000.0, (TIMERPROC)NULL);
	if (wa->graph.timerID == 0){
		//dbprintf("graph timer failed to start %i\n",(int)GetLastError());
		return 0;
	}

	if (!wa->misc.IUTimerID && !wa->config.mediaPlayDisabled)
		wa->misc.IUTimerID = (int)timeSetEvent(1000, 100, OneSecIntervalTickerCB, (DWORD)wa, TIME_PERIODIC);
	if (!wa->misc.engineTimerID)
		wa->misc.engineTimerID = (int)timeSetEvent((1.0/120.0)*1000.0, 10, EngineTickerCB, (DWORD)wa, TIME_PERIODIC);

	/*hTimerQueue = CreateTimerQueue();
    if (hTimerQueue){
		CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)EngineTickerCB2, wa, 1, (1.0/(float)90.0)*1000.0, 0);
	}*/
	//DeleteTimerQueueTimer()
	return wa->graph.timerID;
}

static int getUpdateSignal (TWINAMP *wa)
{
	return wa->misc.updateSignal;
}

static void setUpdateSignal (TWINAMP *wa)
{
	wa->misc.updateSignal = 0;
}

static void removeUpdateSignal (TWINAMP *wa)
{
	wa->misc.updateSignal = 1;
}

static int handleConnectionTimeout (TWINAMP *wa, unsigned int totalTime)
{
	int time = 0;
	int timeout = 0;
	int space;
	int found = 0;
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};

	lSetCharacterEncoding(wa->hw, CMT_ISO8859_15);
	unsigned int Time0 = GetTickCount();
	setUpdateSignal(wa);
	RefreshDisplay(wa, frame);

	do{
		time = GetTickCount()-Time0;
		timeout = (totalTime-time);
		if (timeout < 1) timeout = 0;

		space = lGetFontLineSpacing(wa->hw, wa->config.font.about);
		lSetFontLineSpacing(wa->hw, wa->config.font.about, space-3);
		lClearFrame(frame);
		lPrintEx(frame, &rect, wa->config.font.about, PF_WORDWRAP|PF_CLIPWRAP|PF_RESETXY, LPRT_CPY,\
		  "Connecting to %s:%d", wa->net.address, wa->net.port);
		lPrintEx(frame, &rect, LFT_DOTUMCHE24X24, PF_NEWLINE|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_OR, "%d", timeout/1000);
		lSetFontLineSpacing(wa->hw, wa->config.font.about, space);
		setUpdateSignal(wa);
		RefreshDisplay(wa, frame);

		/*
		if (wa->net.protocol){	// TCP
			wa->net.conn.socket.server = netConnect(&wa->net.conn, (char*)wa->net.address, wa->net.port, IPPROTO_TCP);
			if (wa->net.conn.socket.server > 0){
				wa->net.conn.socket.client = wa->net.conn.socket.server;
				if (!netPing(&wa->net.conn)){
					netCloseSocket(wa->net.conn.socket.server);
					found = 0;
				}else{
					return 1;
				}
			}
		}else{	// UDP
		*/
			// check if we've found a host
			netEmptyReadBuffer(&wa->net.conn);
			found = netPing(&wa->net.conn);
		//}

		if (!found){
			lSleep(100);
		}else{
			return 1;
		}
	}while((time < totalTime) && !isKeyPressed(VK_SHIFT) && !isKeyPressed(VK_ESCAPE) && !found);

	MessageBox(NULL, "WVS "MY_VERSIONSTR": Media player not found.\nEnsure address and port are correct and player is running.", "WVS: Host not found", MB_OK|MB_SYSTEMMODAL);
	return 0;
}

static void resetPlaylistScroller (TWINAMP *wa)
{
	// reset playlist scroller
	wa->playlist.highlightPos = wa->track.number.value;
	wa->playlist.start = wa->playlist.highlightPos;
	wa->playlist.enabled = 0;
	wa->playlist.track = -1;
}

static int doMediaPlayerQuit (TWINAMP *wa)
{
	wa->net.notFound = 1;
	if (wa->state) return 1;

	wa->misc.lastKnownPlayState = 0;	// track playback stopped
	cache_dump(&wa->net.cache);			// packet cache
	cache_Flush(&wa->cache);			// string cache, could do with an improved naming convention
	resetG15Leds(wa);

	wa->net.version.vMP = 0;
	wa->net.version.v1 = 0;
	wa->net.version.v2 = 0;
	wa->net.version.v3 = 0;

	wa->overlay.trackSeek = 0;
	wa->overlay.spectMarkers = 0;
	strcpy((char*)wa->net.mpName, "Media Player");

	memset(&wa->net.eq, 0, sizeof(wa->net.eq));
	memset(&wa->net.wave, 0, sizeof(wa->net.wave));
	memset(&wa->net.gen, 0, sizeof(wa->net.gen));
	//memset(wa->graph.spectrum, 0, sizeof(ubyte) * wa->graph.width);
	//memset(wa->graph.spectrum4, 0, sizeof(ubyte) * wa->graph.width);
	wa->net.gen.position = -1000;		// cmd_processGen adds 1000
	cmd_processGen(wa, &wa->net.gen);
	cmd_processWave(wa, &wa->net.wave);
	processSpectrumData(wa);

	seekEndTitleScroll(wa);
	wa->scroll.adler = 0;		// when player is found ensure track title is updated
	lDeleteFrame(wa->scroll.frame);
	wa->scroll.frame = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.titleScroll, "<WVS %s> ", MY_VERSIONSTR);
	if (wa->scroll.frame)
		setTitleScroll(wa);
	else
		dbprintf("doMediaPlayerQuit: wa->scroll.frame == NULL\n");
	return 1;
}

static int doMediaPlayerFound (TWINAMP *wa)
{
	wa->net.conn.sendCt = 0;
	wa->net.conn.readCt = 0;
	wa->net.notFound = 0;
	wa->net.foundOnce = 1;	// this is set once per session (once set it is not reset)

	netGetPluginVersion(&wa->net.conn, NULL, NULL, NULL);
	netGetMPVersion(&wa->net.conn, NULL);
	netGetMPName(&wa->net.conn, wa->net.mpName, sizeof(wa->net.mpName));
	netGetPlayState(&wa->net.conn, NULL);
	netGetTotalPlaylists(&wa->net.conn, &wa->misc.tPlaylists);
	getCurrentTrackDetails(wa);
	cache_Flush(&wa->cache);
	cacheTrackTitles(wa, 0, 6);
	cacheMetaTags(wa, 0, 0);
	resetPlaylistScroller(wa);
	return 1;
}

static int connectToMediaPlayer (TWINAMP *wa)
{
	if (getPageStatus(wa, DISF_EQ)
	  + getPageStatus(wa, DISF_META)
	  + getPageStatus(wa, DISF_MAIN)
	  + getPageStatus(wa, DISF_PLAYLIST)
	  + getPageStatus(wa, DISF_LYRICS)
	  + getPageStatus(wa, DISF_HITLIST) == 0)
	{
		wa->config.mediaPlayDisabled = 1;
		wa->net.notFound = 1;
		if (wa->config.volumeCtrl == VOLUME_MEDIAPLAYER)
			wa->config.volumeCtrl = VOLUME_MASTER;

		if (wa->config.startPage == DISF_EQ || wa->config.startPage == DISF_META
		 || wa->config.startPage == DISF_MAIN || wa->config.startPage == DISF_PLAYLIST
		 || wa->config.startPage == DISF_HITLIST || wa->config.startPage == DISF_LYRICS)
		{
			wa->config.startPage = wa->dis.index;
		}
		dbprintf("player is disabled\n");
		return 1;
	}
	if (wa->net.conn.socket.server != SOCKET_ERROR)
		netCloseSocket(wa->net.conn.socket.server);

	/*
	if (wa->net.protocol){  // TCP=1, UDP=0
		wa->net.conn.socket.server = 1;
		if (!wa->config.waitForPlayer){
			wa->net.conn.socket.server = netConnect(&wa->net.conn, (char*)wa->net.address, wa->net.port, IPPROTO_TCP);
			if (wa->net.conn.socket.server > 0)
				wa->net.conn.socket.client = wa->net.conn.socket.server;
		}
	}else{*/
		wa->net.conn.socket.server = netConnect(&wa->net.conn, (char*)wa->net.address, wa->net.port, IPPROTO_UDP);
	//}
	wa->net.conn.socket.client = wa->net.conn.socket.server;
	if ((int)wa->net.conn.socket.server < 0){
		if ((int)wa->net.conn.socket.server == -2){
			MessageBox(NULL, "WVS "MY_VERSIONSTR": Could not resolve address.\nEnsure host address is valid.", "WVS: Invalid address name supplied", MB_OK|MB_SYSTEMMODAL);
		}else if ((int)wa->net.conn.socket.server == -3){
			MessageBox(NULL, "WVS "MY_VERSIONSTR": Invalid protocol specified.\n", "WVS: Connect failed", MB_OK|MB_SYSTEMMODAL);
		}else if ((int)wa->net.conn.socket.server == -4){
			MessageBox(NULL, "WVS "MY_VERSIONSTR": Unable to create a socket. Verify address and port are correct\n", "WVS: Connect failed", MB_OK|MB_SYSTEMMODAL);
		}else {
			MessageBox(NULL, "WVS "MY_VERSIONSTR": Host not found.\nEnsure host is running.", "WVS: Connect failed", MB_OK|MB_SYSTEMMODAL);
			dbprintf("connecTo(): failed\n");
		}
		return 0;
	}else{
		if (wa->config.waitForPlayer){
			if (!handleConnectionTimeout(wa, MPSEARCHTIMEOUT)){
				wa->state = 1;
				wa->net.notFound = 1;
				wa->config.mediaPlayDisabled = 1;
				dbprintf("media player not found, exiting..\n");
				return 0;
			}
		}

		netEmptyReadBuffer(&wa->net.conn);
		if (netPing(&wa->net.conn)){
			dbprintf("media player found, starting..\n");
			doMediaPlayerFound(wa);
			return 1;
		}else{
			if (wa->config.waitForPlayer){
				MessageBox(NULL, "WVS "MY_VERSIONSTR": Media player not found.\nEnsure host is running.", "WVS: Connect failed", MB_OK|MB_SYSTEMMODAL);
				dbprintf("media player not found, exiting..\n");
				return 0;
			}else{
				dbprintf("starting WVS though media player was not found\n");
				return 1;
			}
		}
	}
}

static int packet_getStructType (TWVSNET *net, TWVSPACKET *packet)
{

	ubyte cmd = *(ubyte*)packet->buffer;
	if (!cmd) return 0;

	int ret = 0;
	if (cmd == CMD_GET_SPECTRUMDATA){
		if (packet->bufferSize <= sizeof(TMPCMDWAVE)){
			if (packet->wave->magic == MAGIC)
				ret = CMDSTRUCT_WAVE;
			else
				ret = -3;
		}
	}else if (cmd == CMD_GET_CURRENTTRKINFO){
		if (packet->bufferSize <= sizeof(TMPGENTRACKINFO)){
			if (packet->gen->magic == MAGIC)
				ret = CMDSTRUCT_GEN;
			else
				ret = -2;
		}
	}else if (cmd == CMD_SET_EQDATA || cmd == CMD_GET_EQDATA){
		if (packet->bufferSize <= sizeof(TMPCMDEQ)){
			if (packet->eq->magic == MAGIC)
				ret = CMDSTRUCT_EQ;
			else
				ret = -1;
		}
	}else if (cmd == CMD_HOSTEXIT){
		ret = CMD_HOSTEXIT;

	}else if (cmd <= CMD_TOTAL){
		if (packet->bufferSize <= sizeof(TMPCMD)){
			if (packet->cmd->magic == MAGIC)
				ret = CMDSTRUCT_CMD;
			else
				ret = -4;
		}
	}

	//dbprintf("net_getStructType %i %i\n", ret, cmd);
	return ret;
}

static int packet_extractWave (TWVSNET *net, TWVSPACKET *packet, TMPCMDWAVE *wave)
{

	/*size_t dataSize = sizeof(TMPCMDWAVE) - (sizeof(wave->data) - packet->wave->len);
	if (dataSize != packet->bufferSize){
		dbprintf("packet_extractWave buffer size mismatch: expecting:%i got:%i\n", dataSize, packet->bufferSize);
		return 0;
	}*/ // check isn't needed

	memcpy(wave, packet->wave, packet->bufferSize);
	packet_delete(&net->cache, packet);
	return 1;
}

static int packet_extractGen (TWVSNET *net, TWVSPACKET *packet, TMPGENTRACKINFO *gen)
{

	if (packet->bufferSize == sizeof(TMPGENTRACKINFO)){
		memcpy(gen, packet->gen, sizeof(TMPGENTRACKINFO));
		packet_delete(&net->cache, packet);
		return 1;
	}else{
		dbprintf("packet_extractGen buffer size mismatch: expecting:%i got:%i\n", sizeof(TMPGENTRACKINFO), packet->bufferSize);
	}
	packet_delete(&net->cache, packet);
	return 0;
}

static int cmd_processGen (TWINAMP *wa, TMPGENTRACKINFO *gen)
{
	wa->track.Channels.value = gen->channels;
	if (gen->totaltracks != wa->playlist.totalTracks){
		wa->playlist.track = -1;
		wa->playlist.enabled = 0;
		if (wa->playlist.start > gen->totaltracks)
			wa->playlist.start = 0;
	}

	if (gen->samplerate != wa->track.sampleRate.value){
		wa->track.sampleRate.value = gen->samplerate;
		lDeleteFrame(wa->track.sampleRate.text);
		wa->track.sampleRate.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.samRate, "%ikhz", wa->track.sampleRate.value);
		if (wa->track.sampleRate.text == NULL)
			dbprintf("wa->track.sampleRate.text == NULL\n");
	}

	if (gen->bitrate != wa->track.bitRate.value){
		wa->track.bitRate.value = gen->bitrate;
		lDeleteFrame(wa->track.bitRate.text);
		wa->track.bitRate.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.bitRate, "%ikb", wa->track.bitRate.value);
		if (wa->track.bitRate.text == NULL)
			dbprintf("wa->track.bitRate.text == NULL\n");
	}

	if ((gen->playlistpos != wa->track.number.value) || (gen->totaltracks != wa->playlist.totalTracks)){
		wa->track.number.value = gen->playlistpos;
		wa->playlist.totalTracks = gen->totaltracks;
		lDeleteFrame(wa->track.number.text);
		lSetCharacterEncoding(wa->hw, wa->lang->enc);
		wa->track.number.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackNumber, "%s %d/%d",\
		  lng_getString(wa->lang, LNG_TIT_TRK),\
		  MIN(wa->track.number.value+1, wa->playlist.totalTracks), wa->playlist.totalTracks);
		if (wa->track.number.text == NULL)
			dbprintf("wa->track.number.text == NULL\n");
	}

	if (gen->length != wa->track.length.value){
		wa->track.length.value = gen->length;
		lDeleteFrame(wa->track.length.text);
		wa->track.length.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackLength, "%i:%.2i",\
		  wa->track.length.value/60, wa->track.length.value%60);
		if (wa->track.length.text == NULL)
			dbprintf("wa->track.length..text == NULL\n");
	}

	if (gen->position != wa->track.position.value){
		wa->track.position.value = gen->position;
		lDeleteFrame(wa->track.position.text);
		wa->track.position.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackPosition,"%i:%.2i",\
		  wa->track.position.value/1000/60,(((wa->track.position.value)/1000)%60));
		if (wa->track.position.text == NULL)
			dbprintf("wa->track.position..text == NULL\n");
	}

	if (wa->track.Channels.value == 1)
		wa->overlay.channel2 = 0;
	else if (wa->config.overlay.channel2)
		wa->overlay.channel2 = 1;

	return 1;
}

static int packet_extractEq (TWVSNET *net, TWVSPACKET *packet, TMPCMDEQ *eq)
{

	if (packet->bufferSize == sizeof(TMPCMDEQ)){
		memcpy(eq, packet->eq, sizeof(TMPCMDEQ));
		packet_delete(&net->cache, packet);
		return 1;
	}else{
		dbprintf("packet_extractEq buffer size mismatch: expecting:%i got:%i\n", sizeof(TMPCMDEQ), packet->bufferSize);
	}

	packet_delete(&net->cache, packet);
	return 0;
}

static int cmd_processEq (TWINAMP *wa, TMPCMDEQ *eq)
{
	wa->eq.getnewdata = 1;
	renderFrame(wa, DISF_EQ);
	setUpdateSignal(wa);
	return 1;
}

static int packet_extractCmd (TWVSNET *net, TWVSPACKET *packet, TMPCMD *cmd)
{
	if (packet->bufferSize == sizeof(TMPCMD)){
		memcpy(cmd, packet->cmd, sizeof(TMPCMD));
		return 1;
	}else{
		dbprintf("packet_extractCmd buffer size mismatch: expecting:%i got:%i\n", sizeof(TMPCMD), packet->bufferSize);
	}
	packet_delete(&net->cache, packet);
	return 0;
}

static int cmd_processTotalPlaylists (TWINAMP *wa, TMPCMD *cmd)
{
	wa->misc.tPlaylists = cmd->data1;
	return 1;
}

static int cmd_processVolume (TWINAMP *wa, TMPCMD *cmd)
{
	int steps = 0;	// step size (0<n<256)
	volSensiIndexToLevel(wa->config.volumeSensi, NULL, &steps);

	wa->mixer[VOLUME_MEDIAPLAYER].currentVolume = cmd->data1;

	if (cmd->data2 == VOLUME_DECREASE){
		if (wa->mixer[VOLUME_MEDIAPLAYER].currentVolume > steps)
			wa->mixer[VOLUME_MEDIAPLAYER].currentVolume -= steps;
		else
			wa->mixer[VOLUME_MEDIAPLAYER].currentVolume = 0;
		netSetVolume(&wa->net.conn, wa->mixer[VOLUME_MEDIAPLAYER].currentVolume);

	}else if (cmd->data2 == VOLUME_INCREASE){
		if (wa->mixer[VOLUME_MEDIAPLAYER].currentVolume < 256-steps)
			wa->mixer[VOLUME_MEDIAPLAYER].currentVolume += steps;
		else
			wa->mixer[VOLUME_MEDIAPLAYER].currentVolume = 255;
		netSetVolume(&wa->net.conn, wa->mixer[VOLUME_MEDIAPLAYER].currentVolume);
	}
	return 1;
}

static int packet_extractString (TWVSNET *net, TWVSPACKET *packet, TMPCMD *cmd, void *buffer, size_t buffersize)
{
	if (cmd->data3){
		TWVSPACKET *packetNext = packet->next;
		if (packet->next == NULL)
			return -1;

		memcpy(buffer, packetNext->buffer, MIN(cmd->data3, packetNext->bufferSize));
		if (isWideStrCmd(cmd->command)){
			*((wchar_t*)(buffer+buffersize-sizeof(wchar_t))) = 0;
			//*((char*)buffer+(buffersize-1)) = 0;
		}else{
			*((char*)(buffer+buffersize-sizeof(char))) = 0;
		}

		packet_delete(&net->cache, packet);
		packet_delete(&net->cache, packetNext);
		return 1;
	}else{
		packet_delete(&net->cache, packet);
		return 0;
	}
}

static int ansi1252_lookup (int c)
{
	int i;
	for (i = 0; ansi1252table[i].wc; i++){
		if (ansi1252table[i].wc == c)
			return ansi1252table[i].c;
	}
	return '?';
}


static int utf81252_to_ansi (char *src, size_t srcLen, char *des)
{

	int i = 0;
	unsigned int wc = 0;
//	int c = 0;
	int chrs;

	while(i < srcLen){
		chrs = UTF8ToUTF32b((ubyte*)(src+i), &wc);
		if (chrs < 1) return 0;
		i += chrs;
		if (wc > 0xFF){
			*des++ = ansi1252_lookup(wc);
		}else{
			*des++ = wc;
		}
//		c++;
	}
	return i;
}

static int cmd_processString (TWINAMP *wa, TMPCMD *cmd, void *buffer)
{
	if (isWideStrCmd(cmd->command)){
		/*
		 if (cmd->command != 39 && cmd->command != 38){
			wdbprintf(L"W: %i %i %i: #%s#     ",cmd->command, cmd->data1, cmd->data2, (wchar_t*)buffer);
			dbprintf(" \n");
		}*/
		return cache_AddString(&wa->cache, buffer, cmd->command, cmd->data1, cmd->data2);
	}else{
		char *srcBuffer = buffer;
		char ansibuffer[4096];	// fix the magic numbers

		if (wa->config.autoUTF8){
			memset(ansibuffer, 0, sizeof(ansibuffer));
			int inlen = strlen(buffer);
			if (inlen == utf81252_to_ansi(buffer, inlen, ansibuffer))
				srcBuffer = ansibuffer;
		}
		/*
		if (cmd->command != 39 && cmd->command != 38)
			dbprintf("A: %i %i %i: #%s#\n",cmd->command, cmd->data1, cmd->data2, srcBuffer);
		*/
		return cache_AddString(&wa->cache, srcBuffer, cmd->command, cmd->data1, cmd->data2);
	}

	/*
	ubyte *text = (ubyte *)buffer;
	if (cmd->command == CMD_GET_TRACKTITLEA){
		int i = strlen((char*)text);
		int c = 0;
		while(c < i){
			dbprintf("%i: %i\n",c+1, text[c]);
			c++;
		}
	}*/
	//return cache_AddString(&wa->cache, buffer, cmd->command, cmd->data1, cmd->data2);
}

static int cmd_processPlayState (TWINAMP *wa, TMPCMD *cmd)
{
	wa->misc.lastKnownPlayState = cmd->data1;
	if (wa->overlay.g15Lights && wa->misc.lastKnownPlayState != 1)
		resetG15Leds(wa);
	return 1;
}

static int cmd_processMPPluginVersion (TWINAMP *wa, TMPCMD *cmd)
{
	wa->net.version.vMP = cmd->data1;
	return 1;
}

static int cmd_processPluginVersion (TWINAMP *wa, TMPCMD *cmd)
{
	wa->net.version.v1 = cmd->data1;
	wa->net.version.v2 = cmd->data2;
	wa->net.version.v3 = cmd->data3;

	if (!wa->net.version.v1 || !wa->net.version.v2 || !wa->net.version.v3){	// invalid packet
		dbprintf(buffer, "processPluginVersion(): invalid packet: 0x%X %i %i\n", cmd->data1, cmd->data2, cmd->data3);
		return 0;
	}
	if (wa->net.version.v1 != MY_VERSION1 || wa->net.version.v2 != MY_VERSION2 || wa->net.version.v3 != MY_VERSION3){
		wa->net.notFound = 1;
		wa->state = 1;
		char buffer[256];
		
		sprintf(buffer, "unsupported host version\nexpecting: 0x%X %i %i\nhost: 0x%X %i %i",\
		  MY_VERSION1, MY_VERSION2, MY_VERSION3, cmd->data1, cmd->data2, cmd->data3);
		dbprintf("%s\n", buffer);
		TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
		lPrintEx(frame, &rect, LFT_ARIAL, PF_CLIPWRAP|PF_WORDWRAP, LPRT_CPY, buffer);
		lRefresh(frame);

		MessageBox(NULL, "WVS "MY_VERSIONSTR": Old version of gen_wamp.dll detected\n\n"\
		  "Please download latest package from:\nhttp://mylcd.sourceforge.net\n\n", "WVS: Version check", MB_OK|MB_SYSTEMMODAL);
		PostQuitMessage(0);
		return 0;
		
	}else if (wa->net.notFound){
		dbprintf("player found\n");
		doMediaPlayerFound(wa);
		return 1;
	}else{
		return 1;
	}
}

static int cmd_processHostIPC (TWINAMP *wa, TMPCMD *cmd)
{
	//dbprintf("ipc: %i %i\n", cmd->data1, cmd->data2);

	if (cmd->data1 == IPC_PLAYLIST_MODIFIED){
		cache_Flush(&wa->cache);
		resetPlaylistScroller(wa);

	}else if (cmd->data1 == IPC_PLAYING_FILEW){
		cacheTrackTitlesNS(wa, wa->track.number.value, wa->track.number.value);

		wa->net.gen.playlistpos = cmd->data2;
		cmd_processGen(wa, &wa->net.gen);
		getCurrentTrackDetails(wa);
		resetPlaylistScroller(wa);

	}else if (cmd->data1 == IPC_CB_MISC){
		if (cmd->data2 == IPC_CB_MISC_TITLE){
			cacheTrackTitlesNS(wa, wa->track.number.value, wa->track.number.value);
			getCurrentTrackDetails(wa);
			resetPlaylistScroller(wa);

		}else if (cmd->data2 == IPC_CB_MISC_STATUS){
			netGetPlayState(&wa->net.conn, NULL);

		}else{
			dbprintf("not handled\n");
		}
	}else if (cmd->data1 == IPC_FILE_TAG_MAY_HAVE_UPDATEDW){
		cacheMetaTagsNS(wa, wa->track.number.value, wa->track.number.value);
	}

	return 1;
}

static int cmd_processCmd (TWINAMP *wa, TWVSPACKET *packet, TMPCMD *cmd)
{
	wchar_t buffer[4096+8];
	memset(buffer, 0, sizeof(buffer));

	switch(cmd->command){
	  case CMD_GET_METADATAW:
	  case CMD_GET_METADATAA:{
	  		//memset(buffer, 0, sizeof(buffer));
	  		int ret = packet_extractString(&wa->net, packet, cmd, buffer, sizeof(buffer));
			if (ret > 0)
				cmd_processString(wa, cmd, buffer);
			else if (ret == -1)
				return -1;
			break;
		}

	  case CMD_GET_TRACKFILENAMEW:
	  case CMD_GET_TRACKFILENAMEA:
	  case CMD_GET_TRACKTITLEW:
	  case CMD_GET_TRACKTITLEA:{
	  		//memset(buffer, 0, sizeof(buffer));
	  		int ret = packet_extractString(&wa->net, packet, cmd, buffer, sizeof(buffer));
			if (ret > 0){
				cmd_processString(wa, cmd, buffer);
				if (cmd->data2 == wa->track.number.value){
					if (updateTitleScroll(wa, 0)){		// returns 1 if title has been updated. (a track has changed)
						if (!wa->misc.seeked)		// don't seek past signature at startup
							seekScroll(wa, getFrame(wa, DISF_MAIN)->width-12);	// display full title on track change
						else
							wa->misc.seeked = 0;
						resetBarGraphScale(wa);		// scale is a per track setting
					}
				}
			}else if (ret == -1){
				return -1;
			}

			//if (cmd->command == CMD_GET_TRACKFILENAMEW)
				//dbprintf("W: #%s#\n",(char*)buffer);

			//if (cmd->command == CMD_GET_TRACKFILENAMEA)
				//dbprintf("A: #%s#\n",(char*)buffer);

			break;
		}

	  case CMD_GET_MPVOLUME:
	  		packet_delete(&wa->net.cache, packet);
			cmd_processVolume(wa, cmd);
			break;

	  case CMD_HOSTIPC:
	  		packet_delete(&wa->net.cache, packet);
	  		cmd_processHostIPC(wa, cmd);
	  		break;

	  case CMD_GET_TOTALPLAYLISTS:
	  		packet_delete(&wa->net.cache, packet);
			cmd_processTotalPlaylists(wa, cmd);
			break;

	  case CMD_GET_PLAYSTATUS:
	  	  	packet_delete(&wa->net.cache, packet);
	  		cmd_processPlayState(wa, cmd);
	  		break;

	  case CMD_GET_PLVERSION:
	  		packet_delete(&wa->net.cache, packet);
	  		cmd_processPluginVersion(wa, cmd);
	  		break;

	  case CMD_GET_MPVERSION:
	  		packet_delete(&wa->net.cache, packet);
	  		cmd_processMPPluginVersion(wa, cmd);
	  		break;

	  case CMD_GET_MPNAME:{
	  		int ret = packet_extractString(&wa->net, packet, cmd, wa->net.mpName, sizeof(wa->net.mpName));
			if (ret == -1)
				return -1;
			break;
	  	}

	  // cmd_hostexit should never reach here, but just in case.
	  case CMD_HOSTEXIT:
	  		packet_delete(&wa->net.cache, packet);
	  		doMediaPlayerQuit(wa);
	  		return 0;

	  default :
	  		packet_delete(&wa->net.cache, packet);
			dbprintf("unhandled cmd: %i\n", cmd->command);
	}
	return 0;
}

static int cmd_processWave (TWINAMP *wa, TMPCMDWAVE *wave)
{

	if (wa->net.wave.bpc){
		if (wa->config.g15LEDUpperRange > wa->net.wave.bpc)
			wa->config.g15LEDUpperRange = wa->net.wave.bpc;
		if (wa->config.g15LEDLowerRange > wa->net.wave.bpc)
			wa->config.g15LEDLowerRange = wa->net.wave.bpc>>1;
	}

	if (wave->bpc <= MAXCHANNELLENGTH){	// 576 = Winamp channel length
		wa->spectrum.channel1 = &wave->data[0];
		wa->spectrum.channel2 = &wave->data[wave->bpc];
		wa->wave.channel1 = &wave->data[wave->bpc<<1];
		wa->wave.channel2 = &wave->data[wave->bpc*3];
	}else{
		return 0;
	}

	if (wa->graph.barType == 1 || wa->graph.barType == 2)
		processSpectrumData(wa);
	else if (wa->graph.barType == 3 || wa->graph.barType == 4 || wa->graph.barType == 5)
		processOscData(wa);

	if (wa->graph.barType == 6 || wa->overlay.g15Lights){
		getLedLevel(wa); // for g15 and bargraph
		setG15LedLevels(wa);
	}
	return 1;
}

static int cache_processPacket (TWINAMP *wa, TWVSPACKET *packet)
{

	switch(packet_getStructType(&wa->net, packet)){
	  case CMDSTRUCT_WAVE:
		if (packet_extractWave(&wa->net, packet, &wa->net.wave) == 1)
			cmd_processWave(wa, &wa->net.wave);
		return 1;

	  case CMDSTRUCT_CMD:
		if (packet_extractCmd(&wa->net, packet, &wa->net.cmd) == 1){
			if (cmd_processCmd(wa, packet, &wa->net.cmd) == -1)
				return 0;
		}
		return 1;

	  case CMDSTRUCT_GEN:
		if (packet_extractGen(&wa->net, packet, &wa->net.gen) == 1)
			cmd_processGen(wa, &wa->net.gen);
		return 1;

	  case CMDSTRUCT_EQ:
		if (packet_extractEq(&wa->net, packet, &wa->net.eq) == 1)
			cmd_processEq(wa, &wa->net.eq);
		return 1;

	  case CMD_HOSTEXIT:
		dbprintf("host exiting\n");
  		doMediaPlayerQuit(wa);
  		return 0;

	  default:
	 	dbprintf("invalid packet, size: %i\n", packet->bufferSize);
	  	packet_delete(&wa->net.cache, packet);
	  	return 0;
	}
}

static int cache_processPackets (TWINAMP *wvs)
{

	if (!wvs->net.cache.packetsTotal || wvs->state)
		return 0;

	//int total = 0;
	int ret = 1;
	TWVSPACKET *packet;


	//if (wvs->net.cache.packetsTotal > 512){
	//	dbprintf("cache_processPackets(): packets queue %i\n", wvs->net.cache.packetsTotal);
	//}
	do{
		if (WaitForSingleObject(wvs->cacheLock, 50) == WAIT_OBJECT_0){
			if (!wvs->state){
				if ((packet = cache_getStream(&wvs->net.cache)))
					ret = cache_processPacket(wvs, packet);
			}
			ReleaseSemaphore(wvs->cacheLock, 1, NULL);
		}
		//total++;
	}while(ret && !wvs->state && wvs->net.cache.packetsTotal);

	//if (total > 2 && total != 29){
	//	dbprintf("cache_processPackets(): packets processed %i, %i\n", total, wvs->net.cache.packetsTotal);
	//}
	return ret;
}

static int packet_readPending (TWINAMP *wvs, TWVSNET *net, int pendingdata)
{
	do{
		int messageSize = netReadSocket(&net->conn, net->buffer, net->bufferSize);
		if (messageSize > 0){
			if (WaitForSingleObject(wvs->cacheLock, 2000) == WAIT_OBJECT_0){
				if (!wvs->state){
					if (!packet_add(&net->cache, net->buffer, messageSize)){
						dbprintf("packet_add failed, dropping. total in queue:%i max:%i\n", wvs->net.cache.packetsTotal, PACKETQUEUESIZE);
					}
				}
				ReleaseSemaphore(wvs->cacheLock, 1, NULL);
			}
			pendingdata -= messageSize;
		}else{
			return -1;
		}
	}while(pendingdata > 1 && !wvs->state);

	return 1;
}

static int processPending (TWINAMP *wa)
{
	int pendingdata;
	do{
		pendingdata = netIsPacketAvailable(&wa->net.conn);
		if (pendingdata){
			if (packet_readPending(wa, &wa->net, pendingdata) < 1){
				wa->net.notFound = 1;
				return 0;
			}
		}
	}while(pendingdata > 1 && !wa->state);
	return 	1;
}

unsigned int __stdcall packet_listener (TWINAMP *wvs)
{
	Sleep(100);
	dbprintf("packet_listener() started\n");
	if (wvs->net.conn.socket.server > 0){
		FD_SET Reader;
		FD_ZERO(&Reader);
		FD_SET(wvs->net.conn.socket.server, &Reader);

		do{
			select(0, &Reader, NULL, NULL, NULL);
			if (!wvs->state)
				processPending(wvs);
		}while(!wvs->state);
	}else{
		dbprintf("invalid socket %i\n",(int)wvs->net.conn.socket.server);
	}

	dbprintf("packet_listener() exited\n");
	_endthreadex(1);
	return 1;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//static int closeSignal = 0;
	//if (message != 4010)
	//	dbprintf("%i %i %i %i\n", closeSignal, message, wParam, (int)lParam);

	TWINAMP *wa = g_wvs; //(TWINAMP *)lParam;

    switch (message){
		case 4010: 
			if (!wa) break;
			if ((closeSignal == 2 && wa->config.exitWithPlayer) || closeSignal == 1){
				wa->state = 1;
				PostQuitMessage(0);
				return 0;
			}
			if (wa->net.notFound){
				if (wa->config.exitWithPlayer && wa->net.foundOnce){
					wa->state = 1;
					return 0;
       			}
       		}
       		      		
			if (wa->graph.updateCount != wa->misc.timerUpdateCounter){
				wa->graph.updateCount = wa->misc.timerUpdateCounter;
				setUpdateSignal(wa);
				requestWaveData(wa);
				advanceTitleScroll(wa, 0);
			}
			if (!wa->config.mediaPlayDisabled && isPageOpen(wa, DISF_MAIN))
				cache_processPackets(wa);
				
			if (!wa->state){
				doTimedEvents(wa);
        		if (!getUpdateSignal(wa))
					renderFrames(wa);
				updateDisplay(wa, frame);
			}
		
			return 0;

    	case 4000:
    		if (!wa) break;
     		if (!closeSignal){
				if (++wa->misc.timerUpdateCounter > 0xFFFF)
					wa->misc.timerUpdateCounter = 0;
				break;
			}else if (closeSignal == 2 && wa->config.exitWithPlayer){
				wa->state = 1;
				PostQuitMessage(0);
			}else if (closeSignal == 1){
				wa->state = 1;
				if (wa->net.conn.socket.server != SOCKET_ERROR)
					closeSocket(wa->net.conn.socket.server);
				wa->net.conn.socket.server = SOCKET_ERROR;
				PostQuitMessage(0);
			}
			closeSignal = 0;
			return 0;

		case 4020:
			if (!wa) break;
			if ((closeSignal == 2 && wa->config.exitWithPlayer) || closeSignal == 1){
				wa->state = 1;
				PostQuitMessage(0);
				return 0;
			}
			do1sIntervalUpdates(wa);
			return 0;

    	case 3000:	// hook.dll callback
    		if (!wa) break;
    		if (!wa->state){
    			resetPageSwapTime(wa);

	    		if (wa->config.reverseVolumeCtrl){
    				if (wParam == G15_WHEEL_ANTICLOCKWISE)
    					wParam = G15_WHEEL_CLOCKWISE;
    				else if (wParam == G15_WHEEL_CLOCKWISE)
	    				wParam = G15_WHEEL_ANTICLOCKWISE;
    			}
    			if (wParam == G15_WHEEL_ANTICLOCKWISE || wParam == G15_WHEEL_CLOCKWISE)
    				processKeyPress(wa, wParam);
    		}
    		return 0;

    	case 5000:	// g15 softkey callback
    		if (!wa) break;
   		
    		if (wParam && !wa->state){
				resetPageSwapTime(wa);
    			wa->state = processKeyPress(wa, wParam);
    		}
    		return 0;
    		
		case 5051:{
			if (!wa) break;
			int olddir = wa->dis.direction;
			gotoPrevPage(wa);
			if ((wa->config.pageSwapMode == 4 /*|| wa->config.pageSwapMode == 5*/) && wa->dis.direction != olddir && wa->misc.slideOffset)
				wa->misc.slideOffset = frame->width-wa->misc.slideOffset;
			onPageChange(wa);
			break;
		}
		case 5052:{
			if (!wa) break;
			int olddir = wa->dis.direction;
			gotoNextPage(wa);
			if ((wa->config.pageSwapMode == 4 /*|| wa->config.pageSwapMode == 5*/) && wa->dis.direction != olddir && wa->misc.slideOffset)
				wa->misc.slideOffset = frame->width-wa->misc.slideOffset;
			onPageChange(wa);
			break;
		}
		case 5053:{
			if (!wa) break;
			int olddir = wa->dis.direction;
			int page = gotoNextPage(wa);
			//if (page == DISF_ABOUT)
				//page = gotoNextPage(wa);
			if (page == DISF_CONFIG)
				gotoNextPage(wa);
			
			if ((wa->config.pageSwapMode == 4 /*|| wa->config.pageSwapMode == 5*/) && wa->dis.direction != olddir && wa->misc.slideOffset)
				wa->misc.slideOffset = frame->width-wa->misc.slideOffset;
			onPageChange(wa);
			break;
		}
    	case 6000:	// media keys
    		if (!wa) break;
    		resetPageSwapTime(wa);
    		if (wa->config.handleMediaKeys && !wa->state)
    			processKeyPress(wa, wParam);
    		return 0;

   		case 7000:
   			if (!wa) break;
   			registerOverlayFrame(wa, (TFRAMEOVER*)wParam);
   			return 0;

		case 8000:
			//dbprintf("gen_wamp.dll shutting down\n");
			closeSignal = 2;
			return 0;

    	case WM_KEYDOWN:	// from hook.dll callback
    		if (!wa) break;
    		//dbprintf("%i\n",wParam);
			resetPageSwapTime(wa);
			if (wParam == VK_LSHIFT){
  				processKeyPress(wa, VK_LSHIFT);

			}else if (wParam == VK_RSHIFT && getPageStatus(wa, DISF_HITLIST)){
				if (!wa->overlay.trackFav && !wa->net.notFound)
					wa->overlay.trackFav = 1;
  				processKeyPress(wa, VK_RSHIFT);

    		}else if (wParam >= VK_PRIOR && wParam <= VK_DELETE){
   				processKeyPress(wa, wParam|0x1000);

   			/*}else if (wParam == VK_LWIN || wParam == VK_RWIN){
    			kbHookOff();
    		*/
    		}else if (wParam == VK_OEM_2 && !kbHookGetStatus()){	// forward slash
    			processKeyPress(wa, VK_OEM_2);
    		}
    		//break;
			return 0;
    	case WM_KEYUP:
    		if (!wa) break;
    		if (wParam == VK_RSHIFT)
    			wa->overlay.trackFav = 0;

    		//break;
    		return 0;
    	case WM_CHAR: // message generated from hook.dll
    		if (!wa) break;
   			processKeyPress(wa, wParam);
			break;

		case WM_QUIT: //dbprintf("WM_QUIT\n");
			closeSignal = 1;
			DestroyWindow(hwnd);
			return 0;

		case WM_CLOSE: //dbprintf("WM_CLOSE\n");
     		closeSignal = 1;
  			return 0;

 		case WM_DESTROY: //dbprintf("WM_DESTROY\n");
 		    closeSignal = 1;
			PostQuitMessage(0);
	    	return 0;
    }
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static HANDLE initGUI (TWINAMP *wa)
{
    WNDCLASSEX wincl;
	HANDLE hThisInstance = GetModuleHandle(0);

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wincl.style = CS_DBLCLKS;
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    if (!RegisterClassEx (&wincl))
        return NULL;

    wa->hook.hwnd = CreateWindowEx(0, szClassName, szClassName, WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,\
	  5,5, HWND_DESKTOP, NULL, hThisInstance, NULL);

    ShowWindow(wa->hook.hwnd, SW_HIDE);
    return wa->hook.hwnd;
}


/*
	wchar_t dir[GetCurrentDirectoryW(0, NULL)+128];
	GetCurrentDirectoryW(sizeof(dir), dir);
	wcscat(dir, L"\\config.ini");

	int ret = WritePrivateProfileStringW(L"test", L"a key1", L"a string", dir);
	wdbprintf(L"%i -%s-\n",ret, dir);
*/


