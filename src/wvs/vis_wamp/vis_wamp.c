
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
#include <mylcd.h>

#include "../winamp.h"
#include "../net.h"
#include "../config.h"
#include "../plugin_about.h"


void config (winampVisModule *mod);
int init (winampVisModule *mod);
int render (winampVisModule *mod);
void quit (winampVisModule *mod);
winampVisModule *getModule (int which);

static winampVisModule Module = {"WVS", NULL, NULL, 0, 0, 0, renderUpdatePeriod, 2, 2, {0,}, {0,}, config, init, render, quit, NULL};
static winampVisHeader VisHdr = {VIS_HDRVER, "WVS: mylcd.sourceforge.net", getModule};

TMP *mpp = NULL;
static HMODULE hModule = NULL;
volatile int state = 0;

typedef void *(*getMPStructPtr) ();
static getMPStructPtr getMPP = NULL;


static HINSTANCE hMod = NULL;
static winampVisHeader *childHeader = NULL;
static winampVisModule *ModuleChild = NULL;


typedef winampVisHeader* (*VGH_t)();
static VGH_t _VGH = NULL;



void loadChild ()
{
	if (hMod == NULL){
		hMod = LoadLibraryA("wvs/child_vis.dll");
		if (!hMod)
			hMod = LoadLibraryA("child_vis.dll");
		if (!hMod)
			hMod = LoadLibraryA("plugins/wvs/child_vis.dll");
			
		if (hMod){
			_VGH = (VGH_t)GetProcAddress(hMod, "winampVisGetHeader");
			if (_VGH){
				childHeader = _VGH();
				if (childHeader){
					if (childHeader->getModule){
						Module.delayMs = renderUpdatePeriodWithChild;
						ModuleChild = childHeader->getModule(0);
					}
				}
			}
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif

__declspec (dllexport) winampVisHeader *winampVisGetHeader ()
{
	loadChild();
	return &VisHdr;
}

#ifdef __cplusplus
}
#endif

winampVisModule *getModule (int which)
{
	if (which == 0)
		return &Module;
	else if (ModuleChild)
		return childHeader->getModule(which-1);
	else
		return NULL;
}

void config (winampVisModule *mod)
{
	if (ModuleChild)
		ModuleChild->Config(mod);
		
#if (BUILD_WINAMP)
	if (!state)
		return;
	
	if (mpp == NULL)
		init(mod);

	plugin_about(mpp, "WVS: vis_wamp.dll");
#endif
}

int init (winampVisModule *mod)
{
	HWND mlhwin = FindWindow(NULL, "MiniLyrics (Evaluation Version)");
	if (mlhwin != NULL)
		PostMessage(mlhwin, WM_CLOSE, 0, 0);

	if (ModuleChild)
		ModuleChild->Init(mod);
		
	if (hModule == NULL){
		hModule = GetModuleHandle("gen_wamp.dll");
		if (hModule == NULL)
			hModule = GetModuleHandle("plugins\\gen_wamp.dll");	
	}

	if (hModule == NULL){
		#if (BUILD_WINAMP)
		  MessageBox(mod->hwndParent, "gen_wamp.dll not found.", "WVS: vis_wamp.dll", MB_OK);
		#else
		  return 0;
		#endif
		
	}else{
		if (getMPP == NULL)
			getMPP = (getMPStructPtr)GetProcAddress(hModule, "getMPStructPtr");
		if (getMPP != NULL){
			mpp = (TMP *)getMPP();
			if (mpp != NULL){
				mpp->bpc = MAXCHANNELLENGTH;
				mpp->mod = mod;
				mod->userData = (void*)mpp;
				state = 1;
				return 0;
			}
		}
		MessageBox(mod->hwndParent, "Old version of gen_wamp.dll found, please update.", "WVS: vis_wamp.dll", MB_OK);
	}
	return 1;
}

void quit (winampVisModule *mod)
{
	if (ModuleChild)
		ModuleChild->Quit(mod);
		
	state = 0;
	if (mpp != NULL)
		mpp->mod = NULL;

	if (hMod)
		FreeLibrary(hMod);

	hMod = NULL;
	mpp = NULL;
	hModule = NULL;
	getMPP = NULL;
}


int render (winampVisModule *mod)
{

	#if 0
	static int fps = 0;
	static unsigned int t0 = 0;
	
	fps++;
	if (GetTickCount()-t0 >= 1000){
		t0 = GetTickCount();
		printf("%i %i %i\n", fps, mod->latencyMs, mod->delayMs);
		fps = 0;
	}
	#endif

	if (ModuleChild)
		ModuleChild->Render(mod);

		
#if (BUILD_WINAMP)
	if (mpp != NULL){
		if (mpp->wavedataLock != NULL){
			if (WaitForSingleObject(mpp->wavedataLock, 500) == WAIT_OBJECT_0){
				if (mpp->control&0x01)
					memcpy(mpp->spectrum, mod->spectrumData, 576*mod->nCh);
				if (mpp->control&0x02)
					memcpy(mpp->wave, mod->waveformData, 576*mod->nCh);
				ReleaseSemaphore(mpp->wavedataLock, 1, NULL);
			}
		}
	}
#endif
	return 0;
}

