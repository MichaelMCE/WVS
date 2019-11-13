
// myLCD - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
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

#ifndef _DEMOS_H_
#define _DEMOS_H_


#if 0
# define dbprintf __mingw_printf
# define dbwprintf __mingw_wprintf
#else
# define dbprintf(X,...) 
# define dbwprintf(X,...) 
#endif


#include "wvs/file.c"

  
const int DWIDTH = 160;
const int DHEIGHT = 43;
const int DBPP = LFRM_BPP_1;


static THWD *hw = NULL;
static TFRAME *frame = NULL;
static TRECT display;
static wchar_t fontPath[MaxPath+4];
static wchar_t mapPath[MaxPath+4];


int initLibrary (wchar_t *path)
{

#if defined (BUILDING_DLL)
	if (path != NULL){
		dbwprintf(L"initLibrary: wvs location: '%s'\n",path);
		
		memset(fontPath, 0, sizeof(fontPath));
		__mingw_swprintf(fontPath, L"%s%s", path, L"wvs\\fonts\\");
		dbwprintf(L"\ninitLibrary: font path:'%s'\n", fontPath);
		
		memset(mapPath, 0, sizeof(mapPath));
		__mingw_swprintf(mapPath, L"%s%s", path, L"wvs\\mappings\\");
		dbwprintf(L"initLibrary: cmap path:'%s'\n", mapPath);
	}else{
		dbwprintf(L"\ninitLibrary: font path:'%s'\n", L"plugins\\wvs\\fonts\\");
		wcscpy(fontPath, L"plugins/wvs/fonts/");
		wcscpy(mapPath, L"plugins/wvs/mappings/");
	}
#else
	dbwprintf(L"\ninitLibrary: font path:'%s'\n", L"fonts/");
	wcscpy(fontPath, L"fonts/");
	wcscpy(mapPath, L"mappings/");
#endif

	dbprintf("initLibrary: starting mylcd.dll\n");

    if (!(hw=lOpen(fontPath, mapPath))){
    	dbprintf("initLibrary: lOpen() failed\n");
    	return 0;
    }else{
		dbprintf("initLibrary: display surface acquired, mylcd.dll fully initiated\n");
    	return 1;
    }
}

void demoCleanup()
{
	dbprintf("cleanup: mylcd.dll: deleting global display surface\n");
	if (frame){
		lDeleteFrame(frame);
		frame = NULL;
	}

	dbprintf("cleanup: mylcd.dll: closing device handle..\n");
	if (hw){
		lClose(hw);
		hw = NULL;
	}
	dbprintf("cleanup: mylcd.dll shutdown\n");

}

int myLCDInit (char *configfile, wchar_t *path)
{
	if (!initLibrary(path)){
		printf("initConfig: initLibrary failed\n");
		return 0;
	}
	
	display.left = 0;
	display.right = DWIDTH-1;
	display.top = 0;
	display.btm = DHEIGHT-1;
	
	int d = lSelectDevice(hw, "G15", "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (!d)
		d = lSelectDevice(hw, "G15:LIBUSB", "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (!d)
		d = lSelectDevice(hw, "G19", "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (!d)
		d = lSelectDevice(hw, "DDRAW", "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (d){
		frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
		if (frame){
			lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
			lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
			lSetCapabilities(hw, CAP_BACKBUFFER, CAP_STATE_OFF);
		}
	}else{
		printf("could not start a libmylcd driver\n");
	}
	return d && frame;
}

#endif


