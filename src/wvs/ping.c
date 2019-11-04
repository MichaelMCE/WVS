
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


//#include <ipexport.h>
#include "../include/icmp.h"

typedef HANDLE (WINAPI *Icmp_CreateFile) ();
typedef BOOL (WINAPI *Icmp_CloseHandle) (HANDLE IcmpHandle);
typedef DWORD (WINAPI *Icmp_SendEcho) (HANDLE IcmpHandle, struct in_addr,LPVOID RequestData,WORD RequestSize,LPIPINFO RequestOptns,LPVOID ReplyBuffer,DWORD ReplySize,DWORD Timeout);
static Icmp_CreateFile icmpCreateFile;
static Icmp_CloseHandle icmpCloseHandle;
static Icmp_SendEcho icmpSendEcho;

typedef struct {
	char *address;
	int echoPosition;
	int *echoReply;
	int bufferWidth;
}TPING;

volatile TPING ping;
volatile HANDLE hSem = INVALID_HANDLE_VALUE;
static HANDLE hLib = INVALID_HANDLE_VALUE;
static HANDLE hfile = INVALID_HANDLE_VALUE;
volatile HANDLE hPingThread = NULL;
static char sendData[] = "echo";
static int pingInitOnce = 0;

unsigned int __stdcall pingThread (void *ptr);
int getEcho (HANDLE hIcmpFile, char *address);
int pingStartup (TFRAME *frame, void *userPtr);
void pingClose ();
int renderPing (TWINAMP *wa, TFRAME *frame, void *data);



int pingPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderPing(wa, frame, userPtr);
}

int pingPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return -1;
}

int pingPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("pingPageEnter\n");
	return 1;
}

void pingPageExit (void *userPtr)
{
	//dbprintf("pingPageExit\n");
}

int pingPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("pingPageOpen()\n");
	return pingStartup(frame, userPtr);
}

void pingPageClose (void *userPtr)
{
	//dbprintf("pingPageClose()\n");
	pingClose();
}

int pingStartup (TFRAME *frame, void *userPtr)
{
	if (pingInitOnce) return 1;

	hLib = LoadLibrary("icmp.dll");
	if (hLib){
		icmpCreateFile = (Icmp_CreateFile)GetProcAddress(hLib, "IcmpCreateFile");
		icmpCloseHandle = (Icmp_CloseHandle)GetProcAddress(hLib, "IcmpCloseHandle");
		icmpSendEcho = (Icmp_SendEcho)GetProcAddress(hLib, "IcmpSendEcho");
		
		if (!(icmpSendEcho && icmpSendEcho && icmpSendEcho))
			return 0;
	}else{
		return 0;
	}

	hSem = CreateSemaphore(NULL, 1, 1, NULL);
	if (hSem == INVALID_HANDLE_VALUE){
		dbprintf("pingInit() CreateSemaphore() failed\n");
		return 0;
	}

	TWINAMP *wa = (TWINAMP *)userPtr;
	ping.address = (char*)wa->ping.address;
	ping.echoPosition = 1;
	ping.bufferWidth = frame->width;
	ping.echoReply = (int *)calloc(frame->width, sizeof(int));
	if (ping.echoReply == NULL) return 0;
	hfile = icmpCreateFile();
	if (hfile){
		unsigned int threadID = 0;
		hPingThread = (HANDLE)_beginthreadex(NULL, 0, (void*)pingThread, (void*)&ping, 0, &threadID);
		pingInitOnce = 1;
		return 1;
	}else{
		pingInitOnce = 0;
		return 1;
	}
}

void pingClose ()
{
	if (!pingInitOnce) return;
	pingInitOnce = 0;
	
	if (hfile != INVALID_HANDLE_VALUE)
		icmpCloseHandle(hfile);	
	hfile = INVALID_HANDLE_VALUE;
	lSleep(50);

	if (WaitForSingleObject(hSem, 1000) == WAIT_OBJECT_0)
		CloseHandle(hSem);
	hSem = INVALID_HANDLE_VALUE;
	if (hPingThread){
		Sleep(10);
		WaitForSingleObject(hPingThread, 5000);
		CloseHandle(hPingThread);
	}

	if (hLib != INVALID_HANDLE_VALUE) FreeLibrary(hLib);
    if (ping.echoReply != NULL) free(ping.echoReply);

	ping.echoReply = NULL;
	hPingThread = NULL;
	hLib = INVALID_HANDLE_VALUE;

}

unsigned int __stdcall pingThread (void *ptr)
{
	static char address[256];
	TPING *ping = (TPING*)ptr;
	strncpy(address, ping->address, sizeof(address));
	int reply = 0;
	
	do{
		int i = 10;
		while (i-- && (hSem != INVALID_HANDLE_VALUE))
			lSleep(50);
			
		if (hSem != INVALID_HANDLE_VALUE){
			reply = getEcho(hfile, address);
			if (WaitForSingleObject(hSem, 5000) == WAIT_OBJECT_0){
				if (++ping->echoPosition > ping->bufferWidth-1)
					ping->echoPosition = 0;
				if (ping->echoReply != NULL)
					ping->echoReply[ping->echoPosition] = reply;
				ReleaseSemaphore(hSem, 1, NULL);
			}
		}
	}while(hSem != INVALID_HANDLE_VALUE);
	_endthreadex(1);
	return 1;
}

int renderPing (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (!pingInitOnce){
		if (!pingStartup(frame, data))
			return 0;
	}

	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);
	if (WaitForSingleObject(hSem, 500) == WAIT_OBJECT_0){
		lPrintf(frame, 0, 0, LFT_COURIERNEWCE8, LPRT_CPY, "%s = %i%s",\
		  ping.address, ping.echoReply[ping.echoPosition],\
		  lng_getString(wa->lang, LNG_PNG_MS));

		int pos = ping.echoPosition+1;
		double max = 1.0;
		double y = 0;
		int x = 0;

		for (x = 1; x < ping.bufferWidth; x++){
			if ((double)ping.echoReply[x] > max)
				max = (double)ping.echoReply[x];
		}
		double GraphFactor = (double)(frame->height-1.0 - 12.0) / (max*1.1);
		for (x = 1; x < ping.bufferWidth;  x++){
			if (pos > ping.bufferWidth-1)
				pos = 0;
			y = (double)frame->height-1.0 - (double)(GraphFactor * (double)ping.echoReply[pos++]);
			lDrawLine(frame, x, frame->height-1, x, y, LSP_SET);
		}
		ReleaseSemaphore(hSem, 1, NULL);
		lPrintEx(frame, &rect, LFT_COURIERNEWCE8, PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_OR,"%i",(int)max);
	}else{
		return 0;
	}
	return 1;
}

int getEcho (HANDLE hIcmpFile, char *address)
{

	struct hostent *hostPtr = NULL;
	hostPtr = gethostbyname(address);
	if (hostPtr == NULL) {
		hostPtr = gethostbyaddr(address, strlen(address), PF_INET);
		if (hostPtr == NULL) {
			printf("getEcho(): unable to resolve '%s' to an address\n",address);
    		return SOCKET_ERROR;
		}
	}
	struct sockaddr_in serverName;
	memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

	void *ReplyBuffer = (void*)malloc(sizeof(ICMP_ECHO_REPLY) + sizeof(sendData));
	if (ReplyBuffer){
    	DWORD dwRetVal = icmpSendEcho(hIcmpFile, serverName.sin_addr, sendData, sizeof(sendData), NULL, ReplyBuffer, sizeof(sendData) + sizeof(ICMP_ECHO_REPLY), 1000);
    	if (dwRetVal != 0){
			PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        	int triptime = (int)pEchoReply->RoundTripTime;
        	free(ReplyBuffer);
        	return triptime;
        }
    	free(ReplyBuffer);
        return -2;
    }
    return -3;
}
