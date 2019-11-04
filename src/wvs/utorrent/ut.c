
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


#include "utorrent.c"

typedef struct{
	int Weeks;
	int Days;
	int Hours;
	int Minutes;
	int Seconds;
}TTIME;


int updateTorrentList (TWINAMP *wa, TTORRENTS *torrents);
int renderTorrentList (TWINAMP *wa, TFRAME *frame, void *userPtr);
int renderTorrentDetailed (TWINAMP *wa, TFRAME *frame, void *userPtr);
int renderTorrentOverview (TWINAMP *wa, TFRAME *frame, void *userPtr);
void printSortMode (TWINAMP *wa, TFRAME *frame, int sortMode);
static void swapTorrent (TTORRENTENTRY *a, TTORRENTENTRY *b);

void sortTorrents (TTORRENTS *torrents, int sortMode);
static void sortByCompleted (TTORRENTENTRY *tor, int total);
static void sortByOrder (TTORRENTENTRY *tor, int total);
static void sortByUpRate (TTORRENTENTRY *tor, int total);
static void sortByDownRate (TTORRENTENTRY *tor, int total);
static void sortByETA (TTORRENTENTRY *tor, int total);
static void sortByName (TTORRENTENTRY *tor, int total);
static void sortByLabel (TTORRENTENTRY *tor, int total);


static TTORRENTS torrents;
static unsigned int utLastUpdateTime = 0;
static unsigned int utLastSortTime = 0;
static int utStartPos = 0;
static int utPage = 0;
static int utFont = 6;
static int utSort = 0;
static int utAlign = 1;


const static int alignedColumn[3][7] = {	// col, utFont
	{25, 26, 29, 29, 34, 37, 36},
	{53, 53, 56, 58, 65, 69, 63},
	{72, 72, 78, 81, 90, 98, 88}
};



int utPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	//dbprintf("utPageRender\n");
	
	if (updateTorrentList(wa, &torrents) < 1){
		TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
		lSetCharacterEncoding(wa->hw, wa->lang->enc);
		lClearFrame(frame);	
		lPrintEx(frame, &rect, LFTW_SNAP, PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, lng_getString(wa->lang, LNG_UT_UTORRENTUNAVAIL));
		return 0;
	}

	sortTorrents(&torrents, utSort);
		
	if (utPage == 0){
		renderTorrentList(wa, frame, userPtr);
		printSortMode(wa, frame, utSort);
	}else if (utPage == 1){
		renderTorrentDetailed(wa, frame, userPtr);
		printSortMode(wa, frame, utSort);
	}else{ /*if (utPage == 2)*/
		renderTorrentOverview(wa, frame, userPtr);
	}

	return 1;
}

void sortTorrents (TTORRENTS *torrents, int sortMode)
{
	switch (sortMode){
	  case 0: sortByOrder(torrents->list, torrents->totalTorrents); break;
	  case 1: sortByCompleted(torrents->list, torrents->totalTorrents); break;
	  case 2: sortByName(torrents->list, torrents->totalTorrents); break;
	  case 3: sortByDownRate(torrents->list, torrents->totalTorrents); break;
	  case 4: sortByUpRate(torrents->list, torrents->totalTorrents); break;
	  case 5: sortByETA(torrents->list, torrents->totalTorrents); break;
	  case 6: sortByLabel(torrents->list, torrents->totalTorrents); break;
	  default: break;
	}
}

void printSortMode (TWINAMP *wa, TFRAME *frame, int sortMode)
{
	if (GetTickCount()-utLastSortTime < 1000){
		char *text = NULL;
		switch (sortMode){
		  case 0: text = lng_getString(wa->lang, LNG_UT_STATUS); break;
		  case 1: text = lng_getString(wa->lang, LNG_UT_COMPLETED); break;
		  case 2: text = lng_getString(wa->lang, LNG_UT_NAME); break;
		  case 3: text = lng_getString(wa->lang, LNG_UT_DOWNRATE); break;
		  case 4: text = lng_getString(wa->lang, LNG_UT_UPRATE); break;
		  case 5: text = lng_getString(wa->lang, LNG_UT_ETA); break;
		  case 6: text = lng_getString(wa->lang, LNG_UT_LABEL); break;
		  default: return;
		}		
		int enc = lSetCharacterEncoding(wa->hw, wa->lang->enc);
		TFRAME *sort = lNewString(frame->hw, NEWSTRINGTYPE, PF_DONTFORMATBUFFER, LFTW_ROUGHT18, text);
		if (sort != NULL){
			lDrawRectangleFilled(frame, frame->width - sort->width, 0, frame->width-1, sort->height-2, LSP_SET);
			lCopyAreaEx(sort, frame, frame->width-sort->width+1, 0, 0, 1, sort->width-1, sort->height-1, 1, 1, LCASS_XOR);
			lDeleteFrame(sort);
		}
		lSetCharacterEncoding(wa->hw, enc);
	}
}

int utPageInput (TWINAMP *wa, int key, void *userPtr)
{
	if (isKeyPressed(VK_LSHIFT)){
		return -1;
	}else if (isKeyPressed(VK_RSHIFT)){
		utLastSortTime = GetTickCount();
		renderFrame(wa, DISF_UTORRENT);
		
	}else if (key == G15_WHEEL_ANTICLOCKWISE && torrents.totalTorrents){
		if (--utStartPos < 0)
			utStartPos = 0;
		renderFrame(wa, DISF_UTORRENT);
			
	}else if (key == G15_WHEEL_CLOCKWISE && torrents.totalTorrents){
		if (++utStartPos > torrents.totalTorrents-1)
			utStartPos = torrents.totalTorrents-1;
		renderFrame(wa, DISF_UTORRENT);
		
	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_3){	// change font
		++utFont;
		if (!WVSFonts[utFont] || WVSFonts[utFont] == LFTW_B16B)
			utFont = 0;
		renderFrame(wa, DISF_UTORRENT);

	}else if (key == G15_SOFTKEY_3){
		if (++utPage > 2)
			utPage = 0;
		renderFrame(wa, DISF_UTORRENT);
		
	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_4 && utPage < 2){
		utAlign ^= 1;
		renderFrame(wa, DISF_UTORRENT);
		
	}else if (key == G15_SOFTKEY_4 && utPage < 2){
		if (++utSort > 6)
			utSort = 0;
		utStartPos = 0;
		utLastSortTime = GetTickCount();
		renderFrame(wa, DISF_UTORRENT);
	}else{
		return -1;
	}
	return 0;
}

int utPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("utPageEnter\n");
	utLastUpdateTime = 0;
	return 1;
}

void utPageExit (void *userPtr)
{
	//dbprintf("utPageExit\n");
}

int utPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("utPageOpen()\n");
	TWINAMP *wa = (TWINAMP *)userPtr;
	if (wa->config.utorrent.password == NULL || wa->config.utorrent.username == NULL || !wa->config.utorrent.port){
		//dbprintf("utPageOpen(): invalid config\n");
		renderErrorPage(frame, P_OPNFAILED, "\næTorrent - Invalid config");
		//dbprintf("-%s- -%s- -%i- \n", wa->config.utorrent.password, wa->config.utorrent.username, wa->config.utorrent.port);
		return 0;
	}else{
		memset(&torrents, 0, sizeof(TTORRENTS));
		utLastUpdateTime = 0;
		return 1;
	}
}

void utPageClose (void *userPtr)
{
	//dbprintf("utPageClose()\n");
	if (torrents.totalTorrents)
		freeTorrents(&torrents);
}
	
int updateTorrentList (TWINAMP *wa, TTORRENTS *torrents)
{
	if (GetTickCount()-utLastUpdateTime < 2500){
		return torrents->totalTorrents;
	}else{
		utLastUpdateTime = GetTickCount();
	}

	if (torrents->totalTorrents)
		freeTorrents(torrents);
	int total = getTorrents(torrents, wa->config.utorrent.username, wa->config.utorrent.password, wa->config.utorrent.port);
#if 0
	if (total > 0){
		TTORRENTENTRY *tor = torrents->list;
		int i;
		for (i = 0; i < total; i++){
			dumpTorrent(tor++);
		}
	}
#endif
	
	return total;//torrents->totalTorrents;
}

static void etaToTime (int eta, TTIME *e)
{
	memset(e, 0, sizeof(TTIME));
	if (eta < 1) return;

	e->Seconds = eta % 60;
	e->Minutes = (eta / 60) % 60;
	e->Hours = ((eta / 60) / 60) % 24;
	e->Days = (((eta / 60) / 60) / 24) % 7;
    e->Weeks = ((((eta / 60) / 60) / 24) / 7) % 52;
}   

char *sizeToA (u64 bytes, char *buffer, size_t blen)
{
	if (bytes > (u64)1024*1024*1024*100){
		snprintf(buffer, blen, "%.0fG", bytes/1024.0/1024.0/1024.0);
		return buffer;
	}else if (bytes > (u64)1024*1024*1024){
		snprintf(buffer, blen, "%.1fG", bytes/1024.0/1024.0/1024.0);
		return buffer;
	}else if (bytes > (u64)1024*1024*100){
		snprintf(buffer, blen, "%.0fM", bytes/1024.0/1024.0);
		return buffer;
	}else if (bytes > (u64)1024*1024){
		snprintf(buffer, blen, "%.1fM", bytes/1024.0/1024.0);
		return buffer;
	}else if (bytes > (u64)1024*100){
		snprintf(buffer, blen, "%.0fK", bytes/1024.0);
		return buffer;
	}else if (bytes > (u64)1024){
		snprintf(buffer, blen, "%.1fK", bytes/1024.0);
		return buffer;
	}else{
		snprintf(buffer, blen, "%d", (int)bytes);
		return buffer;
	}
}

char *etaToA (int eta, char *buffer, size_t blen)
{
	if (eta < 1){
		snprintf(buffer, blen, " &#8734;");
		return buffer;
	}else{
		*buffer = 0;
	}
	
	int total = 0;
	TTIME e;
	etaToTime(eta, &e);

	if (e.Weeks)
		total = snprintf(buffer, blen, " %dw", e.Weeks);
		
	if (e.Days && total >= 0)
		total += snprintf(buffer+total, blen-total, " %dd", e.Days);
		
	if (e.Hours && total >= 0)
		total += snprintf(buffer+total, blen-total, " %dh", e.Hours);
		
	if (total >= 0 && !e.Weeks)
		total += snprintf(buffer+total, blen-total, " %dm", e.Minutes);
	
	if (total >= 0 && !e.Weeks && !e.Days)
		snprintf(buffer+total, blen-total, " %ds", e.Seconds);

	return buffer;
}

static int isSeeding (int status, u64 remaining)
{
	return status&0x01 && !(status&0x20) && !remaining;
}

static int isDownloading (int status, u64 remaining)
{
	return status&0x01 && !(status&0x20) && (remaining);
}

static int isComplete (int status, u64 remaining)
{
	return !(status&0x40) && !(status&0x10) && !(status&0x02) && !(status&0x01) && (!remaining);
}

static int isPaused (int status)
{
	return status&0x20 && status&0x01;
}

static int isError (int status)
{
	return !(status&0x02) && !(status&0x01) && status&0x10;
}

static int isStopped (int status, u64 remaining)
{
	return !(status&0x40) && !(status&0x10) && !(status&0x02) && !(status&0x01) && (remaining);
}

char *utStatusToA (int status, u64 remaining)
{
	if (status&0x01){	// transferring
		if (status&0x20){
			return "Paused";

		}else if (status&0x40){
			if (remaining)
				return "Downloading";
			else
				return "Seeding";
		}else{
			if (remaining)
				return "Download forced";
			else
				return "Seeding forced";
		}
	}else if (status&0x02){
		return "Checking";
		
	}else if (status&0x10){
		return "Error";
		
	}else if (status&0x40){
		return "Queued";
		
	}else if (!remaining){
		return "Complete";
		
	}else{
		return "Stopped";
	}
}

int renderTorrentDetailed (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	char buffer[2][128];
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);	
	lSetCharacterEncoding(wa->hw, CMT_UTF8);

	if (utStartPos > torrents.totalTorrents-1)
		utStartPos = torrents.totalTorrents-1;
	TTORRENTENTRY *tor = &torrents.list[utStartPos];

	lPrintEx(frame, &rect, WVSFonts[utFont], PF_CLIPDRAW|PF_NEWLINE|PF_DONTFORMATBUFFER, LPRT_CPY, tor->name);
	if (utFont > 2) rect.ey += 2;
	
	lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_NEWLINE, LPRT_CPY, "&#9661;%s &#9651;%s",\
	  sizeToA(tor->downloadRate, buffer[0], sizeof(buffer[0])), sizeToA(tor->uploadRate, buffer[1], sizeof(buffer[1])));
	  
	lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_USELASTX|PF_RIGHTJUSTIFY|PF_CLIPWRAP, LPRT_CPY, "&dArr;%s &uArr;%s",\
	  sizeToA(tor->downloaded, buffer[0], sizeof(buffer[0])), sizeToA(tor->uploaded, buffer[1], sizeof(buffer[1])));
	 
	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	lPrintEx(frame, &rect, LFTW_5x7, PF_RESETX|PF_CLIPDRAW|PF_NEWLINE, LPRT_CPY, "%s: %.1f%% %s %s",\
	  lng_getString(wa->lang, LNG_UT_HAVE), (float)tor->have/10.0,\
	  lng_getString(wa->lang, LNG_UT_OF), sizeToA(tor->fileSize, buffer[0], sizeof(buffer[0])));
	  
	if (utFont > 2){
		lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_USELASTX|PF_RIGHTJUSTIFY|PF_CLIPWRAP, LPRT_CPY,"%s:%.2f",\
		  lng_getString(wa->lang, LNG_UT_A), tor->available/65536.0);
	}
		
	lPrintEx(frame, &rect, LFTW_5x7, PF_RESETX|PF_CLIPDRAW|PF_NEWLINE, LPRT_CPY, "%s:%i(%i) %s:%i(%i)",\
	  lng_getString(wa->lang, LNG_UT_SEEDS), tor->seedsConnected, tor->seedsInSwarm,\
	  lng_getString(wa->lang, LNG_UT_PEERS), tor->peersConnected, tor->peersInSwarm);
	  
	if (utFont > 2){
		lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_USELASTX|PF_RIGHTJUSTIFY|PF_CLIPWRAP, LPRT_CPY,"%s:%.2f",\
		  lng_getString(wa->lang, LNG_UT_R), tor->ratio/1000.0);
	}
	lPrintEx(frame, &rect, LFTW_5x7,  PF_RESETX|PF_CLIPDRAW|PF_NEWLINE, LPRT_CPY, "%s:%s\n",\
	  lng_getString(wa->lang, LNG_UT_ETA), etaToA(tor->ETA, buffer[0], sizeof(buffer[0])));
	
	lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_USELASTX|PF_RIGHTJUSTIFY|PF_CLIPWRAP|PF_DONTFORMATBUFFER, LPRT_CPY, utStatusToA(tor->status, tor->remaining));
	if (utFont < 3){
		lPrintEx(frame, &rect, LFTW_5x7, PF_RESETX|PF_CLIPDRAW|PF_NEWLINE, LPRT_CPY, "%s: %.3f %s: %.3f",\
		  lng_getString(wa->lang, LNG_UT_RATIO), tor->ratio/1000.0,\
		  lng_getString(wa->lang, LNG_UT_AVAIL), tor->available/65536.0);
	}
	return 1;
}

static void swapTorrent (TTORRENTENTRY *a, TTORRENTENTRY *b)
{
	TTORRENTENTRY tor;
	memcpy(&tor, a, sizeof(TTORRENTENTRY));
	memcpy(a, b, sizeof(TTORRENTENTRY));
	memcpy(b, &tor, sizeof(TTORRENTENTRY));
}

static void sortByDownRate (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (tor[i].downloadRate < tor[i+1].downloadRate)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static void sortByETA (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (tor[i].ETA < tor[i+1].ETA)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static void sortByLabel (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (strcmp(tor[i].label, tor[i+1].label) > 0)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static void sortByName (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (strcmp(tor[i].name, tor[i+1].name) > 0)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static void sortByUpRate (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (tor[i].uploadRate < tor[i+1].uploadRate)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static void sortByOrder (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (tor[i].order < tor[i+1].order)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static void sortByCompleted (TTORRENTENTRY *tor, int total)
{
	int i, j = total;
	while (j--){
		for (i = 0; i < total-1; i++){
			if (tor[i].have < tor[i+1].have)
				swapTorrent(&tor[i], &tor[i+1]);
		}
	}
}

static int getColumn (int col, int font, int offSetZero)
{
	if (alignedColumn[col][font] && !offSetZero)
		return alignedColumn[col][font] + 5;
	else	
		return alignedColumn[col][font];
}

int renderTorrentList (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	
	char buffer[2][128];
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	const int font = WVSFonts[utFont];
	lClearFrame(frame);	
	lSetCharacterEncoding(wa->hw, CMT_UTF8);

	if (utStartPos > torrents.totalTorrents-1)
		utStartPos = torrents.totalTorrents-1;
	TTORRENTENTRY *tor = &torrents.list[utStartPos];
	
	int i;
	if (!utAlign){
		for (i = utStartPos; i < torrents.totalTorrents && (rect.ey < frame->height-5); i++, tor++){
			lPrintEx(frame, &rect, font, PF_CLIPDRAW|PF_NEWLINE, LPRT_CPY, "%.1f%% %s %s %s",\
	  		  (float)tor->have/10.0, sizeToA(tor->downloadRate, buffer[0], sizeof(buffer[0])), sizeToA(tor->uploadRate, buffer[1], sizeof(buffer[1])), tor->name);
		}
	}else{
		for (i = utStartPos; i < torrents.totalTorrents && (rect.ey < frame->height-5); i++, tor++){
			lPrintEx(frame, &rect, font, PF_CLIPDRAW|PF_RESETX|PF_NEWLINE, LPRT_CPY, "%.1f%%", (float)tor->have/10.0);
			rect.ex = getColumn(0, utFont, (tor->downloadRate));
			lPrintEx(frame, &rect, font, PF_CLIPDRAW|PF_USELASTX, LPRT_CPY, "%s", sizeToA(tor->downloadRate, buffer[0], sizeof(buffer[0])));
			rect.ex = getColumn(1, utFont, (tor->uploadRate));
			lPrintEx(frame, &rect, font, PF_CLIPDRAW|PF_USELASTX, LPRT_CPY, "%s", sizeToA(tor->uploadRate, buffer[0], sizeof(buffer[0])));
			rect.ex = getColumn(2, utFont, 0);
			lPrintEx(frame, &rect, font, PF_CLIPDRAW|PF_USELASTX, LPRT_CPY, "%s", tor->name);
		}
	}
	return 1;
}

static u64 getTotalUpRate (TTORRENTENTRY *tor, int total)
{
	u64 rate = 0;
	while (total--){
		rate += tor->uploadRate;
		tor++;
	}
	return rate;
}

static u64 getTotalDownRate (TTORRENTENTRY *tor, int total)
{
	u64 rate = 0;
	while (total--){
		rate += tor->downloadRate;
		tor++;
	}
	return rate;
}

static u64 getTotalUp (TTORRENTENTRY *tor, int total)
{
	u64 rate = 0;
	while (total--){
		rate += tor->uploaded;
		tor++;
	}
	return rate;
}

static u64 getTotalDown (TTORRENTENTRY *tor, int total)
{
	u64 rate = 0;
	while (total--){
		rate += tor->downloaded;
		tor++;
	}
	return rate;
}

static int getTotalPeers (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += tor->peersConnected;
		tor++;
	}
	return rate;
}

static int getTotalSeeds (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += tor->seedsConnected;
		tor++;
	}
	return rate;
}

static float getTotalRatio (TTORRENTENTRY *tor, int total)
{
	return (float)getTotalUp(tor, torrents.totalTorrents)/(float)getTotalDown(tor, torrents.totalTorrents);
}

static int getTotalSeeding (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += isSeeding(tor->status, tor->remaining);
		tor++;
	}
	return rate;
}

static int getTotalDownloading (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += isDownloading(tor->status, tor->remaining);
		tor++;
	}
	return rate;
}

static int getTotalCompleted (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += isComplete(tor->status, tor->remaining);
		tor++;
	}
	return rate;
}

static int getTotalPaused (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += isPaused(tor->status);
		tor++;
	}
	return rate;
}

static int getTotalError (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += isError(tor->status);
		tor++;
	}
	return rate;
}

static int getTotalStopped (TTORRENTENTRY *tor, int total)
{
	int rate = 0;
	while (total--){
		rate += isStopped(tor->status, tor->remaining);
		tor++;
	}
	return rate;
}

int renderTorrentOverview (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	
	char buffer[2][128];
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);	
	const int font = LFTW_5x7;
	const int flags = PF_CLIPDRAW|PF_CLIPWRAP|PF_NEWLINE|PF_MIDDLEJUSTIFY;
	TTORRENTENTRY *tor = &torrents.list[0];

	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	
	lPrintEx(frame, &rect, font, flags, LPRT_CPY, "%s: %i",\
	  lng_getString(wa->lang, LNG_UT_TORRENTS), torrents.totalTorrents);
	  
	lPrintEx(frame, &rect, font, flags, LPRT_CPY, "%s: %s  %s: %s",\
	  lng_getString(wa->lang, LNG_UT_DL),\
	  sizeToA(getTotalDownRate(tor, torrents.totalTorrents), buffer[0], sizeof(buffer[0])),\
	  lng_getString(wa->lang, LNG_UT_UL),\
	  sizeToA(getTotalUpRate(tor, torrents.totalTorrents), buffer[1], sizeof(buffer[1])));
	  
	lPrintEx(frame, &rect, font, flags, LPRT_CPY, "%s: %sB  %s: %sB",\
	  lng_getString(wa->lang, LNG_UT_IN),\
	  sizeToA(getTotalDown(tor, torrents.totalTorrents), buffer[0], sizeof(buffer[0])),\
	  lng_getString(wa->lang, LNG_UT_OUT),\
	  sizeToA(getTotalUp(tor, torrents.totalTorrents), buffer[1], sizeof(buffer[1])));
	  
	lPrintEx(frame, &rect, font, flags, LPRT_CPY, "%s: %.2f",\
	  lng_getString(wa->lang, LNG_UT_RATIO),\
	  getTotalRatio(tor, torrents.totalTorrents));
	  
	lPrintEx(frame, &rect, font, flags, LPRT_CPY, "%s: %i  %s: %i",\
	  lng_getString(wa->lang, LNG_UT_SEEDS),\
	  getTotalSeeds(tor, torrents.totalTorrents),\
	  lng_getString(wa->lang, LNG_UT_PEERS),\
	  getTotalPeers(tor, torrents.totalTorrents));
	  
	lPrintEx(frame, &rect, font, flags, LPRT_CPY, "%s:%i  %s:%i  %s:%i  %s:%i  %s:%i  %s:%i",\
	  lng_getString(wa->lang, LNG_UT_SE),\
	  getTotalSeeding(tor, torrents.totalTorrents),\
	  lng_getString(wa->lang, LNG_UT_DN),\
	  getTotalDownloading(tor, torrents.totalTorrents),\
	  lng_getString(wa->lang, LNG_UT_CP),\
	  getTotalCompleted(tor, torrents.totalTorrents),\
	  lng_getString(wa->lang, LNG_UT_PU),\
	  getTotalPaused(tor, torrents.totalTorrents),\
	  lng_getString(wa->lang, LNG_UT_ST),\
	  getTotalStopped(tor, torrents.totalTorrents),\
	  lng_getString(wa->lang, LNG_UT_ER),\
	  getTotalError(tor, torrents.totalTorrents));

	return 1;
}


