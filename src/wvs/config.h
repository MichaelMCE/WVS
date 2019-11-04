
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

#ifndef _CONFIG_H_
#define _CONFIG_H_


#define WVSMODULE "wvs.exe" /* used by gen_wamp.c */

#if (BUILD_MEDIAMONKEY)
# define MP_NAME "MediaMonkey"
#elif (BUILD_SVP)
#  define MP_NAME "AIMP"
#elif (BUILD_XMPLAY)
#  define MP_NAME "XMPlay"
#else
#  define MP_NAME "Winamp"
#endif

#define MY_NAME "myLCD "MP_NAME" plugin by Michael McElligott\nokio@users.sourceforge.net"


#define REQUESTPULLHANDLER 0
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
const int SENDBUFFERSIZE = 8192;		// size of string send buffer (bytes)
const int SOCKETBUFFERSIZE = 65536;

#if (BUILD_MEDIAMONKEY)
typedef struct {
	IDispatch *pRoot;
	IDispatch *pPlayer;
	IDispatch *pSongList;
	IDispatch *pSongData;
	unsigned char *visMap;
}TMPMM;
#endif

typedef struct {
	TMPNET net;
	TMPCMD cmd;
	winampVisModule	*mod;
	winampGeneralPurposePlugin *plugin;
	
	volatile HANDLE hwndParent;
	volatile HANDLE wavedataLock;
	volatile HANDLE sendLock;
	volatile unsigned int control;
	volatile unsigned char spectrum[2][MAXCHANNELLENGTH];
	volatile unsigned char wave[2][MAXCHANNELLENGTH];
	volatile unsigned short bpc;	// bytes per channel

	union{
		wchar_t *WChar;
		char *AChar;
	}buffer;

#if (BUILD_MEDIAMONKEY)
	TMPMM mm;
#endif

	int totalTracks; //total tracks in currently displayed list
	int Shuttingdown;	// will go high when exiting
	HANDLE quitEvent;
}TMP;

typedef struct{
	int (*address) (TMP *mp, TMPCMD *cmd);
}TCMDP;



#endif
