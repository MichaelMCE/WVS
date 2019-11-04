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



#include <wininet.h>
#include <objbase.h>
#include "../rss/rss.c"


static int updateFeed (TRSS **feed);
static int renderText (TFRAME *frame, ubyte *buffer, int flags, int font);
static void renderFeed (TFRAME *frame, TRSS *rss, int font, ubyte *scratchpad);
static int createScroll (TLSCROLLEX **scroll, TFRAME *srcframe, TFRAME *desframe);
static void setEncoding (THWD *hw, TRSS *rss);
static void rssReset ();
static void rssDrawScroll ();

static TLSCROLLEX *rssScroll = NULL;
static TRSS *rss = NULL;
static ubyte *rssbuffer = NULL;
static ubyte *rssurl = NULL;
static TFRAME *srcFrame = NULL;
static unsigned int lastTickCt = 0;
static int rssUpdatePeriod = 60000;
static ubyte URL[] = "http://newsrss.bbc.co.uk/rss/newsonline_uk_edition/front_page/rss.xml";
static int initonce = 0;
static void rssCleanup ();
static unsigned int lastUpdateTime = 0;
static unsigned int paused = 0;

int displayInput_RSS (TWINAMP *wa, int key, void *data);
int renderRSS (TWINAMP *wa, TFRAME *frame, void *data);
void rssCleanup ();
int rssInit (ubyte *url, TFRAME *frame, int font, int updateperiod);



int rssPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderRSS(wa, frame, userPtr);
}

int rssPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_RSS(wa, key, userPtr);
}

int rssPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("rssPageEnter\n");
	return 1;
}

void rssPageExit (void *userPtr)
{
	//dbprintf("rssPageExit\n");
}

int rssPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("rssPageOpen()\n");
	
	TWINAMP *wa = (TWINAMP *)userPtr;
	return rssInit(wa->config.rss.url, frame, wa->font.rss, wa->config.rssUpdatePeriod);
}

void rssPageClose (void *userPtr)
{
	//dbprintf("rssPageClose()\n");
	rssCleanup();
}

static char *my_strcpy (ubyte *a, ubyte *b)
{
	return (char *)strcpy((char*)a,(char*)b);
}

static int my_strstr (ubyte *a, char *c)
{
	return (int)strstr((char*)a,c);
}

int rssInit (ubyte *url, TFRAME *frame, int font, int updateperiod)
{
	if (initonce)
		rssCleanup();
	
	if (url == NULL)
		rssurl = URL;
	else
		rssurl = url;
		
	if (updateperiod > 600)
		rssUpdatePeriod = updateperiod*1000;
	else
		rssUpdatePeriod = 600000;

	if (!initonce){
		if (font == LFTW_ROUGHT18){
			lMergeFontW(frame->hw, font, LFTW_HELVR18);
			lSetFontLineSpacing(frame->hw, font, lGetFontLineSpacing(frame->hw, font)-2);
		}
		initonce = 1;
	}
	srcFrame = lCloneFrame(frame);
	rssReset();
	int ret = updateFeed(&rss);
	if (!renderRSS(NULL, frame, NULL) || !ret){
		//dbprintf("RSS feed unavailable\n");
		//lPrintf(frame, 0, 0, font, LPRT_CPY, "RSS feed unavailable");
		return 0;
	}else{
		//dbprintf("RSS available\n");
		return 1;
	}
}

void rssCleanup ()
{
	if (rss) freeRSS(rss);
	if (rssbuffer) free(rssbuffer);
	if (srcFrame) lDeleteFrame(srcFrame);
	if (rssScroll) lDeleteScroll(rssScroll);
	
	rss = NULL;
	rssScroll = NULL;
	rssbuffer = NULL;
	srcFrame = NULL;
}

static void rssReset ()
{
	if (GetTickCount()-lastTickCt > 5000)
		lastTickCt = 0;
}
static void rssAdvanceScroll ()
{
	//dbprintf("%i %i %i %i\n",rssScroll->pos, rssScroll->srcRect->y1, rssScroll->srcRect->y2, rssScroll->desRect->y2);
	if (rssScroll && !paused)
		lUpdateScroll(rssScroll);
}

int renderRSS (TWINAMP *wa, TFRAME *desFrame, void *data)
{	
	if (!rss)
		return 0;

	int ret = 0;
	if (((GetTickCount() - lastTickCt) > rssUpdatePeriod) || !lastTickCt) {
		lastTickCt = GetTickCount();
		//WVSHookOff(wa);
		ret = updateFeed(&rss);
		//WVSHookOn(wa);
		setEncoding(wa->hw, rss);
		renderFeed(srcFrame, rss, LFTW_ROUGHT18, rssbuffer);
		createScroll(&rssScroll, srcFrame, desFrame);
	}
	
	rssAdvanceScroll();
	rssDrawScroll();
	return ret;
}

static void rssDrawScroll ()
{
	if (rssScroll)
		lDrawScroll(rssScroll);
}

static void rssForward ()
{
	if (rssScroll->dir == SCR_DOWN){
		rssScroll->dir = SCR_UP;
		rssScroll->pos = (rssScroll->srcRect->y2 - rssScroll->pos) + rssScroll->desRect->y2+1;
	}else{
		int i;
		for (i = 0; i<4; i++)
			rssAdvanceScroll();
	}
}

static void rssBack ()
{
	if (rssScroll->dir == SCR_UP){
		rssScroll->dir = SCR_DOWN;
		rssScroll->pos = (rssScroll->srcRect->y2 - rssScroll->pos) + rssScroll->desRect->y2+1;
	}else{
		int i;
		for (i = 0; i<4; i++)
			rssAdvanceScroll();
	}
}

static void rssBackFast ()
{
	int i;
	for (i = 0; i<6; i++)
		rssBack();
}

static void rssForwardFast ()
{
	int i;
	for (i = 0; i<6; i++)
		rssForward();
}

static void setEncoding (THWD *hw, TRSS *rss)
{
	if (my_strstr(rss->XMLtag,"GB2312") || my_strstr(rss->XMLtag,"gb2312"))
		lSetCharacterEncoding(hw, CMT_GB18030);
	else if (my_strstr(rss->XMLtag,"GBK") || my_strstr(rss->XMLtag,"gbk"))
		lSetCharacterEncoding(hw, CMT_GBK);
	else if (my_strstr(rss->XMLtag,"big5") || my_strstr(rss->XMLtag,"BIG5"))
		lSetCharacterEncoding(hw, CMT_BIG5);
	else if (my_strstr(rss->XMLtag,"utf-8") || my_strstr(rss->XMLtag,"UTF-8"))
		lSetCharacterEncoding(hw, CMT_UTF8);
	else
		lSetCharacterEncoding(hw, CMT_ISO8859_1);
}

static int createScroll (TLSCROLLEX **scroll, TFRAME *srcFrame, TFRAME *desFrame)
{
	if (*scroll) 
		lDeleteScroll(*scroll);

	*scroll = lNewScroll(srcFrame, desFrame);
	if (*scroll){
		(*scroll)->desRect->x2 = desFrame->width-1;
		(*scroll)->desRect->y2 = desFrame->height-1;
		(*scroll)->flags = SCR_LOOP;
		(*scroll)->dir = SCR_UP;
		(*scroll)->loopGapWidth = 16;	// 16 pixels between bottom and top of repeating scroll
		return 1;
	}else{
		return 0;
	}
}

static void renderFeed (TFRAME *frame, TRSS *rss, int font, ubyte *scratchpad)
{
	if (!frame||!rss||!scratchpad) return;
	
	ubyte *rssbufferposition = scratchpad;
	int i;

	my_strcpy(rssbufferposition, rss->Channel->Title);
	rssbufferposition += strlen((char*)rss->Channel->Title);
	*rssbufferposition++ = '\n';
	*rssbufferposition++ = '\n';
	
	for (i=0; i<rss->Channel->totalItems; i++){
		my_strcpy(rssbufferposition, (ubyte*)" &bull; ");
		rssbufferposition += 8;
		
		if (rss->Channel->Item[i].Title){
			my_strcpy(rssbufferposition, rss->Channel->Item[i].Title);
			rssbufferposition += strlen((char*)rss->Channel->Item[i].Title);
			*rssbufferposition++ = ':';
			*rssbufferposition++ = '\n';
		}

		if (rss->Channel->Item[i].Descr){
			my_strcpy(rssbufferposition, rss->Channel->Item[i].Descr);
			rssbufferposition += strlen((char*)rss->Channel->Item[i].Descr);
			*rssbufferposition++ = '\n';

		}else if (rss->Channel->Item[i].Comments){
			my_strcpy(rssbufferposition, rss->Channel->Item[i].Comments);
			rssbufferposition += strlen((char*)rss->Channel->Item[i].Comments);
			*rssbufferposition++ = '\n';

		}else if (rss->Channel->Item[i].Link){
			my_strcpy(rssbufferposition, rss->Channel->Item[i].Link);
			rssbufferposition += strlen((char*)rss->Channel->Item[i].Link);
			*rssbufferposition++ = '\n';
		}
	}

	// terminate sting buffer and remove last newline
	*(rssbufferposition-sizeof(ubyte)) = 0;

	lHTMLCharRefEnable();
	lClearFrame(frame);
	renderText(frame, scratchpad, PF_WORDWRAP|PF_CLIPWRAP|PF_RESETXY, font);

	// top of page marker
	lDrawLineDotted(frame,0,0,frame->width-1,0, LSP_SET);
	// bottom of page marker
	lDrawLineDotted(frame,0,frame->height-1,frame->width-1,frame->height-1, LSP_SET);
	//lSaveImage(frame,"rss.bmp", IMG_BMP, frame->width, frame->height);
}

static int renderText (TFRAME *frame, ubyte *buffer, int flags, int font)
{
	int total = lCountCharacters(frame->hw, (char*)buffer);
	if (!total) return 0;
	unsigned int *glist = (unsigned int *)malloc(4+(sizeof(unsigned int)*total));
	if (!glist) return 0;

	lDecodeCharacterBuffer(frame->hw, (char*)buffer, glist, total);
	lCacheCharacterBuffer(frame->hw, glist, total, font);
	
	// limit metrics to a max width of frame
	TLPRINTR rect = {0, 0, frame->width-1, 0, 0, 0, 0, 0};
printf("rssfeed a\n");
	lGetTextMetricsList(frame->hw, glist, 0, total-1, flags, font, &rect);
printf("rssfeed b\n");
	lResizeFrame(frame, frame->width, rect.by2+1, 0);
	rect.bx2 = frame->width-1;
	lPrintList(frame, glist, 0, total, &rect, font, flags, LPRT_CPY);
	free(glist);
	return total;
}

static int updateFeed (TRSS **rss)
{
	
	if (GetTickCount()-lastUpdateTime < 10000)
		return 1;

	//long blen = 512*1024; // theres nothing like a good hack
	if (rssbuffer)
		free(rssbuffer);
	//
	//rssbuffer = (ubyte*)calloc(sizeof(ubyte),blen);
	//if (rssbuffer == NULL)
	//	return 0;
	//
	//if (!GetUrl(rssurl, rssbuffer, blen))
	//	return 0;
	
	size_t len;
	rssbuffer = (ubyte*)GetUrl((char*)rssurl, &len);
	if (!rssbuffer) return 0;
	
	//if (*rss)
	//	freeRSS(*rss);

	*rss = newRSS(rssbuffer, len);
	if (!*rss)
		return 0;

	lastUpdateTime = GetTickCount();
	if (!parseRSS(*rss)){
		freeRSS(*rss);
		*rss = NULL;
		return 0;
	}else{
		return 1;
	}
}

static int IsKeyPressed (int vk)
{
	return GetKeyState(vk)&0x80;
}

int displayInput_RSS (TWINAMP *wa, int key, void *data)
{
	if (!initonce) return -1;
	
	if (IsKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE){	
		rssBackFast();
		rssDrawScroll();
	}else if (IsKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE){	
		rssForwardFast();
		rssDrawScroll();
	}else if (isKeyPressed(VK_LSHIFT)){
		return -1;
	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		rssBack();
		rssDrawScroll();
	}else if (key == G15_WHEEL_CLOCKWISE){
		rssForward();
		rssDrawScroll();
	}else if (key == G15_SOFTKEY_3){
		paused ^= 1;
	}else if (key == G15_SOFTKEY_4){
		rssReset();
	}else{
		return -1; //handleCommonKeys(wa, key, data);
	}
	return 0;
}

