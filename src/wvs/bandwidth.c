
// network bandwidth and scrolling clock

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


#include <iphlpapi.h>
#include <time.h>

#define s64 signed __int64
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

typedef struct{
	MIB_IFTABLE *pTable;
	DWORD iftableSize;
	DWORD iNum;   		//numberOfInterfaces
	int table;
	s64 downTotal;		// down total since last update
	s64 upTotal;		// up total since last update
	s64 downSpeed;
	s64 upSpeed;	
	s64 dataDownMax;
	s64 dataUpMax;
	s64 dataMax;
	s64 *dataDown;
	s64 *dataUp;
	s64 TotalDown;		// sum of total down 
	s64	TotalUp;		// sum of total up
	int tdata;
}TNetSpeed;


static double btm;
static double top;
static double left;
static double right;
static char clockstr[32];
static int width;
static int movPosition;
static T2POINT *Up = NULL;
static T2POINT *Dn = NULL;
static TNetSpeed *netSpeed = NULL;
static int nameTimeOut = 0;
static int bwInitOnce = 0;

int displayInput_Bandwidth (TWINAMP *wa, int key, void *data);
static int renderBandwidth (TWINAMP *wa, TFRAME *frame, void *data);
static int bandwidthInit (TFRAME *frame, int interfacei);
static void bandwidthCleanup ();


int bandwidthIntfIndex = 0;



int bwPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderBandwidth(wa, frame, userPtr);
}

int bwPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Bandwidth(wa, key, userPtr);
}

int bwPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("bwPageEnter\n");
	return 1;
}

void bwPageExit (void *userPtr)
{
	//dbprintf("bwPageExit\n");
}

int bwPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("bwPageOpen()\n");
	return bandwidthInit(frame, bandwidthIntfIndex);
}

void bwPageClose (void *userPtr)
{
	//dbprintf("bwPageClose()\n");
	bandwidthCleanup();
}

void bandwidthSetIntfIndex (int index)
{
	bandwidthIntfIndex = index;
	bandwidthInit(frame, index);
}

static void update_speeds (TNetSpeed *ns)
{
	if (ns->pTable == NULL) return;
	
	s64 down = ns->downTotal;
	s64 up = ns->upTotal;

	if (!ns->downTotal || !ns->upTotal){
		ns->downTotal = (s64)ns->pTable->table[netSpeed->table].dwInOctets;
		ns->upTotal = (s64)ns->pTable->table[netSpeed->table].dwOutOctets;
		ns->dataDownMax = 1;
		ns->dataUpMax = 1;
		return;
	}

	GetIfTable(ns->pTable, &ns->iftableSize, TRUE);
	ns->downTotal = (s64)ns->pTable->table[ns->table].dwInOctets;
	ns->upTotal = (s64)ns->pTable->table[ns->table].dwOutOctets;
	ns->downSpeed = (s64)ns->downTotal - down;
	ns->upSpeed = (s64)(ns->upTotal - up);
	
	if (!ns->TotalDown)
		ns->TotalDown = ns->downTotal;
	else
		ns->TotalDown += ns->downSpeed;

	if (!ns->TotalUp)
		ns->TotalUp = ns->upTotal;
	else
		ns->TotalUp += ns->upSpeed;
	
	if (ns->downSpeed > ns->dataDownMax)
		ns->dataDownMax = ns->downSpeed;
	if (ns->upSpeed > ns->dataUpMax)
		ns->dataUpMax = ns->upSpeed;
	
	int i;
	ns->dataDownMax = 1;
	ns->dataUpMax = 1;

	for (i=ns->tdata-1;i>0;i--){
		ns->dataDown[i] = ns->dataDown[i-1];
		if (ns->dataDown[i] > ns->dataDownMax)
			ns->dataDownMax = ns->dataDown[i];
	}
	ns->dataDown[0] = ns->downSpeed;

	for (i=ns->tdata-1;i>0;i--){
		ns->dataUp[i] = ns->dataUp[i-1];
		if (ns->dataUp[i] > ns->dataUpMax)
			ns->dataUpMax = ns->dataUp[i];
	}
	ns->dataUp[0] = ns->upSpeed;
	
	if (ns->dataUpMax > ns->dataDownMax)
		ns->dataMax = ns->dataUpMax;
	else
		ns->dataMax = ns->dataDownMax;
}

void update_time (char *s, size_t len, int format)
{
	time_t t = time(0);
	struct tm *tdate = localtime(&t);
	if (format)
		strftime(s, len, "%H:%M", tdate);	// 24hr clock
	else
		strftime(s, len, "%I:%M", tdate);	// 12hr
}

static void bandwidthCleanup ()
{
	if (!bwInitOnce) return;
	bwInitOnce = 0;
		
	if (Up)
		free(Up);
	if (Dn)
		free(Dn);
	if (netSpeed){
		if (netSpeed->dataDown)
			free(netSpeed->dataDown);
		if (netSpeed->dataUp)
			free(netSpeed->dataUp);
		if (netSpeed->pTable)
			FREE(netSpeed->pTable);
		free(netSpeed);
	}

	netSpeed = NULL;
	Dn = NULL;
	Up = NULL;
}

static int bandwidthGetTotalInf ()
{
	DWORD total = 0;
	GetNumberOfInterfaces(&total);
	return total;
}

static int bandwidthInit (TFRAME *frame, int interfacei)
{
	
	//dbprintf("bandwidthInit %i\n", interfacei);
	if (netSpeed != NULL)
		bandwidthCleanup();

	netSpeed = (TNetSpeed *)malloc(sizeof(TNetSpeed));
	memset (netSpeed,0,sizeof(TNetSpeed));
	netSpeed->tdata = frame->width+1;
	netSpeed->dataDown = malloc(netSpeed->tdata*sizeof(s64));
	if (netSpeed->dataDown == NULL) return 0;
	netSpeed->dataUp = malloc(netSpeed->tdata*sizeof(s64));
	if (netSpeed->dataUp == NULL) return 0;
	memset(netSpeed->dataDown,0,netSpeed->tdata*sizeof(s64));
	memset(netSpeed->dataUp,0,netSpeed->tdata*sizeof(s64));

	netSpeed->iftableSize = 0;
	if ((GetIfTable(NULL/*netSpeed->pTable*/, &netSpeed->iftableSize, TRUE)) == ERROR_INSUFFICIENT_BUFFER){
		FREE(netSpeed->pTable);
		netSpeed->pTable = MALLOC(netSpeed->iftableSize);
		if (netSpeed->pTable)
			GetIfTable(netSpeed->pTable, &netSpeed->iftableSize, TRUE);
	}

	netSpeed->iNum = bandwidthGetTotalInf();
	/*int in = netSpeed->iNum;
	while(in--)
		printf("%i %s\n",in ,netSpeed->pTable->table[in].bDescr);	*/

	if (interfacei < netSpeed->iNum)
		netSpeed->table = interfacei;
	else if (netSpeed->iNum > 0)
		netSpeed->table = 1;
	else
		netSpeed->table = 0;

	btm = frame->height-1-9;
	top = 2;
	left = 0;
	right = frame->width;
	width = (right-left)+1;
	Up = (T2POINT *)malloc(width * sizeof(T2POINT));
	if (Up == NULL) return 0;
	Dn = (T2POINT *)malloc(width * sizeof(T2POINT));
	if (Dn == NULL) return 0;
	
	int i;
	for (i = left; i < width; i++){
		Dn[i].x = i;
		Up[i].x = i;
	}
	
	movPosition = 0;
	nameTimeOut = 3;		// display name when rendering for n seconds
	bwInitOnce = 1;
	return netSpeed->iNum;
}

static int renderBandwidth (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (!bwInitOnce){
		if (!bandwidthInit(frame, wa->config.bwinterface))
			return 0;
		else
			bwInitOnce = 1;
	}

	update_speeds(netSpeed);
	if (!isFrameActive(wa, DISF_BANDWIDTH))
		return 1;

	const int bandwidthFont = LFTW_5x7;
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);

	if (!nameTimeOut){
		rect.sy = frame->height-7;
		lPrintEx(frame, &rect, bandwidthFont, PF_CLIPWRAP|PF_LEFTJUSTIFY, LPRT_OR, "&#9661;%"PRId64"k", (s64)(netSpeed->downSpeed/1024.0));
		lPrintEx(frame, &rect, bandwidthFont, PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_OR,"&#9651;%"PRId64"k", (s64)(netSpeed->upSpeed/1024.0));
	}else{
		nameTimeOut--;
		lPrintf(frame, 0, frame->height-8, bandwidthFont, LPRT_CPY, (char*)netSpeed->pTable->table[netSpeed->table].bDescr);
	}

	if (wa->overlay.bwClock){
		update_time(clockstr, sizeof(clockstr), wa->overlay.clockFormat);
		lPrint(frame, clockstr, 14, 13, LFT_INTERDIMENSIONAL16, LPRT_CPY);
		lMoveArea(frame, 0, 13, frame->width-1,31,movPosition, LMOV_LOOP, LMOV_LEFT);
		if (++movPosition == width)
			movPosition = 0;
	}

	s64 range;
	double h;
	double y1;
	double down,up;
	int x = left;
	int i;

	if (wa->overlay.bwSplit){
		range = ((double)netSpeed->dataDownMax*1.18)/1024.0;
		if (range < 1.0) range = 1.0;
		h = (double)range / (btm-top);
		lPrintEx(frame, &rect, bandwidthFont, PF_RESETXY|PF_CLIPWRAP|PF_LEFTJUSTIFY, LPRT_OR, "%"PRId64"k  &dArr;%.1fmb", range, (double)(netSpeed->TotalDown/1024.0/1024.0));

		for (i=0;i<width>>1;i++){
			down = (double)netSpeed->dataDown[i];
			y1 = btm-((down/1024.0)/h);
			if (y1 < top) y1 = top;
			else if (y1>btm) y1=btm;
			Dn[i].y = y1;
			Dn[i].x = x;
			x++;
		}

		range = ((double)netSpeed->dataUpMax*1.18)/1024.0;
		if (range < 1.0) range = 1.0;
		h = (double)range / (btm-top);
		lPrintEx(frame, &rect, bandwidthFont, PF_USELASTX|PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_OR, "&uArr;%.1fmb  %"PRId64"k",(double)(netSpeed->TotalUp/1024.0/1024.0), range);

		for (i=0;i<width>>1;i++){
			up = (double)netSpeed->dataUp[i];
			y1 = btm-((up/1024.0)/h);
			if (y1 < top) y1 = top;
			else if (y1>btm) y1=btm;
			Up[i].y = y1;
			Up[i].x = x;
			x++;
		}
		lDrawPolyLineTo(frame, Dn, (width>>1)-1, LSP_SET);
		lDrawPolyLineTo(frame, Up, (width>>1), LSP_SET);

	}else{

		range = ((double)netSpeed->dataMax*1.18)/1024.0;
		if (range < 1) range = 1;	
		h = (double)range / (btm-top);

		lPrintEx(frame, &rect, bandwidthFont, PF_RESETXY|PF_CLIPWRAP|PF_LEFTJUSTIFY, LPRT_OR, "&dArr;%.1fmb", (double)(netSpeed->TotalDown/1024.0/1024.0));
		lPrintEx(frame, &rect, bandwidthFont, PF_USELASTX|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_OR, "%"PRId64"k", range);
		lPrintEx(frame, &rect, bandwidthFont, PF_USELASTX|PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_OR, "&uArr;%.1fmb",(double)(netSpeed->TotalUp/1024.0/1024.0));

		for (i=0;i<width;i++){
			down = (double)netSpeed->dataDown[i];
			y1 = btm-((down/1024.0)/h);
			if (y1 < top) y1 = top;
			else if (y1>btm) y1=btm;
			Dn[x].y = y1;

			up = (double)netSpeed->dataUp[i];
			y1 = btm-((up/1024.0)/h);
			if (y1 < top) y1 = top;
			else if (y1>btm) y1=btm;
			Up[x].y = y1;
			Up[x].x = x;
			x++;
		}
		lDrawPolyLineDottedTo(frame, Up, width, LSP_SET);
		lDrawPolyLineTo(frame, Dn, width, LSP_SET);
	}
	return 1;
}

static void toggle_overlayBWClock (TWINAMP *wa)
{
	wa->overlay.bwClock ^= 1;
}

int displayInput_Bandwidth (TWINAMP *wa, int key, void *data)
{
	if (!bwInitOnce)
		return -1;

	if (key == G15_SOFTKEY_3){
		if (++wa->config.bwinterface >= bandwidthGetTotalInf())
			wa->config.bwinterface = 0;
		bandwidthSetIntfIndex(wa->config.bwinterface);
		
	}else if (key == G15_SOFTKEY_4){
		toggle_overlayBWClock(wa);
	}else{
		return -1;
	}
	return 0;	
}
