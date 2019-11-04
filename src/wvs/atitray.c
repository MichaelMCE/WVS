

// ATI Tray Tools plugin

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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <windows.h>


typedef struct tagTATTData {
	DWORD CurGPU;      //Current GPU Speed
	DWORD CurMEM;      //Current MEM Speed
	DWORD isGameActive; //If game from profile is active, this field will be 1 or 0 if not.
	DWORD is3DActive;  //1=3D mode, 0=2D mode
	DWORD isTempMonSupported; //1 - if temperature monitoring supported by ATT
	DWORD GPUTemp;            //GPU Temperature 
	DWORD ENVTemp;            //ENV Temperature 
	DWORD FanDuty;            //FAN Duty
	DWORD MAXGpuTemp;         //MAX GPU Temperature
	DWORD MINGpuTemp;         //MIN GPU Temperature
	DWORD MAXEnvTemp;         //MAX ENV Temperature
	DWORD MINEnvTemp;         //MIN ENV Temperature
	DWORD CurD3DAA;           //Direct3D Antialiasing value
	DWORD CurD3DAF;           //Direct3D Anisotropy value
	DWORD CurOGLAA;           //OpenGL Antialiasing value
	DWORD CurOGLAF;           //OpenGL Anisotropy value
	
	DWORD IsActive;           //is 3d application active
	DWORD CurFPS;             // current FPS
	DWORD FreeVideo;  		  //Free Video Memory
	DWORD FreeTexture;		  //Free Texture Memory
	DWORD Cur3DApi;			  // API used
	DWORD MemUsed;
}TATTData;


static TATTData *data = NULL;
static HANDLE hMapObject = NULL;
static int atiInitOnce = 0;

void attClose ();
int attInit ();
int displayInput_ATT (TWINAMP *wa, int key, void *data);
int renderATT (TWINAMP *wa, TFRAME *frame, void *data);



int attPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderATT(wa, frame, userPtr);
}

int attPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_ATT(wa, key, userPtr);
}

int attPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("attPageEnter\n");
	return 1;
}

void attPageExit (void *userPtr)
{
	//dbprintf("attPageExit\n");
}

int attPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("attPageOpen()\n");
	attInit();
	return 1;
}

void attPageClose (void *userPtr)
{
	//dbprintf("attPageClose()\n");
	attClose();
}

int attInit ()
{
   	if (hMapObject && data && atiInitOnce)
   		return 1;

	hMapObject = OpenFileMapping(FILE_MAP_ALL_ACCESS,TRUE,"ATITRAY_SMEM");	
	if (hMapObject != NULL){
		data = (TATTData*)MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0, 0);
		if (data != NULL){
			atiInitOnce = 1;
			return 1;
		}else{
			atiInitOnce = 0;
			return 0;
		}
	}else{
		data = NULL;
		//printf ("ATI Tray Tools not found\n");
		return 0;
	}	
}

void attClose ()
{
	if (data)
		UnmapViewOfFile(data);
	if (hMapObject)
		CloseHandle(hMapObject);
		
	hMapObject = NULL;
	data = NULL;
	atiInitOnce = 0;
}

static int ctof (int c)
{
	return ((float)c/(float)(5.0/9.0))+32.0;
}

int renderATT (TWINAMP *wa, TFRAME *frame, void *udata)
{
	int mainfont = LFT_COURIERNEWCE8;
	int gpufont = LFTW_ROUGHT18;
	int tUnit = *(int *)udata;
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	
	lClearFrame(frame);
	if (!attInit()){
		lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
		  lng_getString(wa->lang, LNG_ATT_ATTNOTFOUND));
		return 0;
	}

	if (!atiInitOnce) return 0;

	if (!tUnit){
		lPrintf(frame, 0, -2, gpufont, LPRT_CPY, "%s:%dc",\
		  lng_getString(wa->lang, LNG_ATT_GPU), (int)data->GPUTemp);
		lPrintf(frame, 0, 15, mainfont, LPRT_CPY,"%s:%dc",\
		  lng_getString(wa->lang, LNG_ATT_ENV), (int)data->ENVTemp);
	}else{
		lPrintf(frame, 0, -2, gpufont, LPRT_CPY, "%s:%df",\
		  lng_getString(wa->lang, LNG_ATT_GPU), (int)ctof(data->GPUTemp));
		lPrintf(frame, 0, 15, mainfont, LPRT_CPY,"%s:%df",\
		  lng_getString(wa->lang, LNG_ATT_ENV), (int)ctof(data->ENVTemp));
	}
	
	lPrintf(frame, 0, 25, mainfont, LPRT_CPY,"%s:%dmhz",\
	  lng_getString(wa->lang, LNG_ATT_GPU), (int)data->CurGPU);
	lPrintf(frame, 0, 35, mainfont, LPRT_CPY,"%s:%dmhz",\
	  lng_getString(wa->lang, LNG_MEM), (int)data->CurMEM);
	lPrintf(frame, 70, 25, mainfont, LPRT_CPY, lng_getString(wa->lang, LNG_ATT_OGL));
	lPrintf(frame, 70, 35, mainfont, LPRT_CPY, lng_getString(wa->lang, LNG_ATT_D3D));
	lPrintf(frame, 96, 25, mainfont, LPRT_CPY,"%s:%d",\
	  lng_getString(wa->lang, LNG_ATT_AA), (int)data->CurOGLAA);
	lPrintf(frame, 96, 35, mainfont, LPRT_CPY,"%s:%d",\
	  lng_getString(wa->lang, LNG_ATT_AA), (int)data->CurD3DAA);
	lPrintf(frame, 130, 25, mainfont, LPRT_CPY,"%s:%d",\
	  lng_getString(wa->lang, LNG_ATT_AF), (int)data->CurOGLAF);
	lPrintf(frame, 130, 35, mainfont, LPRT_CPY,"%s:%d",\
	  lng_getString(wa->lang, LNG_ATT_AF), (int)data->CurD3DAF);
	lPrintf(frame, 120, 15, mainfont, LPRT_CPY,"%s:%d%%",\
	  lng_getString(wa->lang, LNG_ATT_FAN), (int)data->FanDuty);

	if (data->is3DActive || data->IsActive){
		lPrintf(frame, 70, 15, mainfont, LPRT_CPY,"%s:3D",\
		  lng_getString(wa->lang, LNG_ATT_MODE));
	}else{
		lPrintf(frame, 70, 15, mainfont, LPRT_CPY,"%s:2D",\
		  lng_getString(wa->lang, LNG_ATT_MODE));
	}

	lPrintf(frame, 70, -2, gpufont, LPRT_CPY, "%s:%d",\
	  lng_getString(wa->lang, LNG_ATT_FPS), (int)data->CurFPS);
	return 1;
}


int displayInput_ATT (TWINAMP *wa, int key, void *data)
{
	return -1;
}


