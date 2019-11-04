
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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <windows.h>
#include <math.h>

typedef unsigned char ubyte;
#include "../winamp.h"
#include "../net.h"
#include "../config.h"
#include "../plugin_about.h"
#include "vis.h"

void initialize(void);
BOOL render(unsigned long* Video, int width, int height, int pitch, VisData* pVD);
BOOL savesettings(char* FileName);
BOOL opensettings(char* FileName);



TMP *mpp = NULL;
static HMODULE hModule = NULL;
static int state = 0;

typedef void *(*getMPStructPtr) ();
static getMPStructPtr getMPP = NULL;
static winampVisModule mod;
static int spectrum[4096];
		


VisInfo plugin = 
{
	0,
	"WVS: mylcd.sourceforge.net",
	VI_WAVEFORM|VI_SPECTRUM|SONIQUEVISPROC,
	&initialize,
	&render,
	&savesettings,
	&opensettings
};


__declspec( dllexport ) VisInfo* QueryModule (void)
{
	return &plugin;
}

void initialize (void)
{
	
	//printf("vis: initialize\n");
	state = 0;
	
	if (mpp != NULL)
		mpp->mod = NULL;

	mpp = NULL;
	hModule = NULL;
	getMPP = NULL;

	if (hModule == NULL){
		hModule = GetModuleHandle("gen_wamp.dll");
		if (hModule == NULL)
			hModule = GetModuleHandle("plugins\\gen_wamp.dll");	
	}

	if (hModule == NULL){
		hModule = GetModuleHandle("xmp-wvs.dll");
		if (hModule == NULL)
			hModule = GetModuleHandle("plugins\\xmp-wvs.dll");	
	}

	if (hModule == NULL){
		  MessageBox(0, "WVS plugin not found.", "WVS: vis_wvs.svp", MB_OK);
		  return;
	}else{
		if (getMPP == NULL)
			getMPP = (getMPStructPtr)GetProcAddress(hModule, "getMPStructPtr");

		if (getMPP != NULL){
			mpp = (TMP *)getMPP();
			if (mpp != NULL){
				mpp->bpc = 256;
				mpp->mod = &mod;
				mod.sRate = 44100;
				mod.nCh = 2;
				mod.userData = (void*)mpp;
				state = 1;
				return;
			}
		}
		MessageBox(0, "Old version of WVS plugin found, please update.", "WVS: vis_wvs.svp", MB_OK);
	}

	return;
}

static int MAX (const int a, const int b)
{
	if (a > b)
		return a;
	else
		return b;
}

static void drawLineV (unsigned long *Video, int x, int height, int pitch, int winH)
{
	int y;
	if (height < 0)
		height = 0;
	else if (height > winH-1)
		height = winH-1;
		
	height = winH-1 - height;
	for (y = height; y < winH; y++)
		Video[y*pitch+x] = 0xFFFFFFFF;

}

BOOL render (unsigned long *Video, int width, int height, int pitch, VisData *pVD)
{
	if (mpp != NULL && state){
		if (mpp->wavedataLock){
			if (WaitForSingleObject(mpp->wavedataLock, 200) == WAIT_OBJECT_0){
				if (mpp->control&0x01){
					memcpy((void*)&mpp->spectrum[0][0], &pVD->Spectrum[0], 256);
					memcpy((void*)&mpp->spectrum[0][256], &pVD->Spectrum[1], 256);
				}
				if (mpp->control&0x02){
					memcpy((void*)&mpp->wave[0], pVD->Waveform, 256);
					memcpy((void*)&mpp->wave[0][256], pVD->Waveform, 256);
				}
				ReleaseSemaphore(mpp->wavedataLock, 1, NULL);
			}
		}
	}

	unsigned int x, k;
	float value;
	const int falloff = 15;
	const float yfactor = (float)height/256.0f;
	
	
	if (width == 266)
		width = 256;
	
	for (x = 0; x < width; x++){
		value = 0.0;
		for (k = x; k < x+1; k++)
			value += ((float)pVD->Spectrum[0][k&255] + (float)pVD->Spectrum[1][k&255]);
		
		value += (value*logf(x*x+1));
		spectrum[x] = MAX(spectrum[x]-falloff, value/2.0);
		drawLineV(Video, x, (float)((float)spectrum[x]*yfactor), pitch, height);
	}

	return TRUE;
}

BOOL savesettings (char* FileName)
{
	//printf("vis: savesettings\n");
	mpp->mod = NULL;
	mpp = NULL;
	state = 0;
	return TRUE;
}

BOOL opensettings (char* FileName)
{
	//printf("vis: opensettings\n");
	
	//plugin_about(mpp, "WVS: vis_wvs.svp");
	return TRUE;
}
