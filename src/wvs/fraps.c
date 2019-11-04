
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



typedef struct  {
   DWORD sizeOfStruct;
   DWORD currentFPS;
   DWORD totalFrames;
   DWORD timeOfLastFrame;
   char gameName[32];
}FRAPS_SHARED_DATA;
FRAPS_SHARED_DATA *(WINAPI *FrapsSharedData)();

static HMODULE frapsDLL = NULL;
volatile static TFRAMEOVER overlay;
static int frpInitOnce = 0;

int renderFraps (TWINAMP *wa, TFRAME *frame, void *data);
int frapsInit (TFRAME *frame, void *userPtr);
void frapsClose ();



int frapsPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderFraps(wa, frame, userPtr);
}

int frapsPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return -1;
}

int frapsPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("frapsPageEnter\n");
	return 1;
}

void frapsPageExit (void *userPtr)
{
	//dbprintf("frapsPageExit\n");
}

int frapsPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("frapsPageOpen()\n");
	frapsInit(frame, userPtr);
	return 1;	
}

void frapsPageClose (void *userPtr)
{
	//dbprintf("frapsPageClose()\n");
	frapsClose();
}

int frapsInit (TFRAME *frame, void *userPtr)
{
	if (frpInitOnce) return 1;

	TWINAMP *wa = (TWINAMP*)userPtr;
	
	if (wa->fraps.fps.text == NULL){
		wa->fraps.fps.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.frapsFPS, "0");
		if (wa->fraps.fps.text == NULL){
			dbprintf("frapsInit(): wa->fraps.fps.text == NULL\n");
			return 0;
		}
		wa->fraps.fps.value = -99;
	}
	
	overlay.ownerSetsT0 = 0;
	overlay.t0 = 0;
	overlay.x = frame->width+10; // render to bottom right
	overlay.y = frame->height+10; // let wvs calculate x,y
	overlay.frame = wa->fraps.fps.text;
	overlay.requestedTime = 1200;
	overlay.ownerCleans = 1;
	overlay.copyMode = wa->fraps.copyMode; //LCASS_XOR;
	overlay.distination = DISF_ACTIVEFRAME;
	overlay.reuse = 1;
	overlay.status = 0;

	//int ret = (int)GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, "FRAPS.DLL", &frapsDLL);
	frapsDLL = GetModuleHandle("FRAPS.DLL");
	if (!frapsDLL) {
		//printf("fraps.dll not found\n");
		return 0;
	}

	FrapsSharedData = (void *)GetProcAddress(frapsDLL, "FrapsSharedData");
	if (!FrapsSharedData) {
		//printf("fraps 1.9c or later required\n");
		return 0;
	}
	frpInitOnce = 1;
	return 1;
}

int getFrapsFPS (TWINAMP *wa, TFRAME *frame)
{
	if (!frpInitOnce){
		if (!frapsInit(frame, wa))
			return 0;
	}

	if (frapsDLL != NULL && GetModuleHandle("FRAPS.DLL")){
		FRAPS_SHARED_DATA *fsd = FrapsSharedData();
		if (fsd != NULL)
			return fsd->currentFPS;
	}
	frapsDLL = NULL;
	return 0;
}

char *getFrapsGameName ()
{
	if (frpInitOnce){
		if (frapsDLL != NULL && GetModuleHandle("FRAPS.DLL")){
			FRAPS_SHARED_DATA *fsd = FrapsSharedData();
			if (fsd != NULL){
				if (strlen(fsd->gameName))
					return fsd->gameName;
				else
					return "<>";
			}
		}
		frapsDLL = NULL;
	}
	return "<>";
}

void frapsClose ()
{
	frpInitOnce = 0;
	frapsDLL = NULL;
}

int renderFraps (TWINAMP *wa, TFRAME *frame, void *data)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);
	
	int fps = getFrapsFPS(wa, frame);
	
	if (wa->overlay.frapsFPS && frapsDLL != NULL){
		if (wa->fraps.fps.value != fps){
			wa->fraps.fps.value = fps;
			lDeleteFrame(wa->fraps.fps.text);
			wa->fraps.fps.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.frapsFPS, "%d", fps);
			if (wa->fraps.fps.text == NULL){
				dbprintf("renderFraps(): wa->fraps.fps.text == NULL\n");
				return 0;
			}
			overlay.frame = wa->fraps.fps.text;

			if (wa->fraps.position == FRAPSPOS_TOPLEFT){
				overlay.x = 0;
				overlay.y = 0;
			}else if (wa->fraps.position == FRAPSPOS_TOPRIGHT){
				overlay.x = frame->width+10;
				overlay.y = 0;
			}else if (wa->fraps.position == FRAPSPOS_BOTTOMRIGHT){
				overlay.x = frame->width+10;
				overlay.y = frame->height+10;
			}else if (wa->fraps.position == FRAPSPOS_BOTTOMLEFT){
				overlay.x = 0;
				overlay.y = frame->height+10;
			}
		}
		overlay.copyMode = wa->fraps.copyMode;
		PostMessage(wa->hook.hwnd, 7000, (WPARAM)&overlay, (LPARAM)wa);
	}
	
	if (frapsDLL == NULL){
		lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY, "%s\n\n%s",\
		  lng_getString(wa->lang, LNG_FRP_FRAPSNOTFOUND),\
		  lng_getString(wa->lang, LNG_FRP_STARTFRAPSFIRST));
		  
		frpInitOnce = 0;
		return 0;
	}else{
		lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY, "%s", getFrapsGameName());
		lPrintEx(frame, &rect, LFTW_ROUGHT18, PF_MIDDLEJUSTIFY|PF_NEWLINE|PF_CLIPWRAP, LPRT_OR, "%d", fps);
		return 1;
	}
}

