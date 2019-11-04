
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

#include "wa.c"


void initConsole ();
void config ();
int init ();
void quit ();
volatile winampGeneralPurposePlugin plugin = {GPPHDR_VER, "WVS client: mylcd.sourceforge.net", init, config, quit, 0, 0};

volatile int initWVSOnce = 0;
static TWINAMP *wvs = NULL;
volatile HANDLE hThread = NULL;

static HINSTANCE hInstance;
static wchar_t szPath[MAX_PATH];



__declspec (dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin ()
{
	return (winampGeneralPurposePlugin*)&plugin;
}

wchar_t *getWinampPathW ()
{	
	memset(szPath, 0, sizeof(szPath));
	GetModuleFileNameW(hInstance, szPath, sizeof(szPath)-sizeof(wchar_t));
	//wprintf(L"#%s#\n",szPath);
	return szPath;
}

/*
char *getWinampPath ()
{	
	memset(szPath, 0, sizeof(szPath));
	GetModuleFileName(hInstance, szPath, sizeof(szPath)-sizeof(char));
	//printf("#%s#\n",szPath);
	return szPath;
}
*/

static TWINAMP mywvs;
static HANDLE quitEvent;


int init ()
{
	//initConsole();
	wchar_t drive[_MAX_DRIVE+1];
	wchar_t dir[_MAX_DIR+1];
	
	if (initWVSOnce)
		return 0;
	else
		initWVSOnce = 1;

	if (wvs == NULL)
		wvs = &mywvs; //(TWINAMP *)calloc(1, sizeof(TWINAMP));
	memset(wvs, 0, sizeof(TWINAMP));

	quitEvent = CreateEvent(NULL, 0, 0, "WVSQUITEVENT");
	//printf("gen_wvs: quitEvent %p\n",quitEvent);

	
	_wsplitpath(getWinampPathW(), drive, dir, NULL, NULL);
	swprintf(szPath, L"%s%s", drive, dir);
	//wprintf(L"-%s-\n", (wchar_t*)szPath);
	SetCurrentDirectoryW(szPath);

	wvs->state = 0;
	wvs->wvsPath = szPath;	
	unsigned int threadID = 0;
	hThread = (HANDLE)_beginthreadex(NULL, 0, (void*)DoWVS, (void*)wvs, 0, &threadID);
	return 0;
}

void config ()
{

}

void quit ()
{
	//printf("gen_wvs() quit\n");
	SetEvent(quitEvent);
	
	if (!initWVSOnce)
		return;

	if (wvs){
		wvs->state = 1;
	
		HWND hwin = FindWindow(NULL, "you_shouldn't_see_me");
		if (hwin != NULL)
			PostMessage(hwin, 8000, 0, 0);
		
		if (hThread){
			WaitForSingleObject(hThread, 60000);
			CloseHandle(hThread);
		}
		//free((void*)wvs);
	}

	//wvs = NULL;
	hThread = NULL;
	initWVSOnce = 0;
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


__declspec(dllexport) BOOL APIENTRY DllMain (HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			//initConsole();
			hInstance = hDLL;
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return 1;
}


