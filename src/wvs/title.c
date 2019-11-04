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


int displayInput_WinampMain (TWINAMP *wa, int key, void *data);
int renderMainFrame (TWINAMP *wa, TFRAME *frame, void *data);
int initStructs (TWINAMP *wa, TFRAME *frame);
static void drawScroll (TWINAMP *wa);
static void toggle_overlayTrackSeek (TWINAMP *wa);
static int drawVolume (TWINAMP *wa, TFRAME *frame, int mixer);
static void drawVolumeLevel (TWINAMP *wa, TFRAME *frame, int x, int y, int mixer);
static void drawVolumeLevelPercent (TWINAMP *wa, TFRAME *frame, int x, int y, int mixer);
static void drawWinampVolumeLevel (TWINAMP *wa, TFRAME *frame, int x, int y);
static void drawWinampVolumeLevelPercent (TWINAMP *wa, TFRAME *frame, int x, int y);

static int renderSpecGraph (TWINAMP *wa, TFRAME *frame, int x, int y);





int titlePageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	//dbprintf("titlePageRender\n");
	return renderMainFrame(wa, frame, userPtr);
}

int titlePageInput (TWINAMP *wa, int key, void *userPtr)
{
	//dbprintf("titlePageInput\n");
	return displayInput_WinampMain(wa, key, userPtr);
}

int titlePageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("titlePageEnter\n");
	return 1;
}

void titlePageExit (void *userPtr)
{
	//dbprintf("titlePageExit\n");
}

int titlePageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("titlePageOpen()\n");
	if (initStructs((TWINAMP*)userPtr, frame)){
		return 1;
	}else{
		renderErrorPage(frame, P_OPNFAILED, "\nTitle - startup failure");
		return 0;
	}
}

void titlePageClose (void *userPtr)
{
	//dbprintf("titlePageClose()\n");
}

static void toggle_overlayTrackSeek (TWINAMP *wa)
{
	if (!wa->overlay.trackSeek && !wa->overlay.spectMarkers){
		wa->overlay.trackSeek = 1;
	}else if (wa->overlay.trackSeek){
		wa->overlay.trackSeek = 0;
		wa->overlay.spectMarkers = 1 & (1^wa->lyricsDismode);
	}else{
		wa->overlay.trackSeek = 0;
		wa->overlay.spectMarkers = 0;
	}
}


static void drawWinampVolumeLevel (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	//float vol = (float)wa->mixer[VOLUME_MEDIAPLAYER].currentVolume;
	//lDrawPBar(frame, x, y, frame->width, 9,(100.0f/255.0f)*vol, PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_SET);
}

static void drawWinampVolumeLevelPercent (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	float vol = (float)wa->mixer[VOLUME_MEDIAPLAYER].currentVolume;
	TFRAME *pc = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackNumber, "%s %i%%",wa->net.mpName, (int)((100.0f/255.0f)*vol));
	if (pc != NULL){
		lCopyAreaEx(pc, frame, (wa->graph.width>>1)-(pc->width>>1), y, 0, 0, pc->width-1, pc->height-1, 1, 1, LCPY_XOR);
		lDeleteFrame(pc);
	}else{
		dbprintf("drawWinampVolumeLevelPercent(): pc == NULL\n");
	}
}

static void drawVolumeLevel (TWINAMP *wa, TFRAME *frame, int x, int y, int mixer)
{
	//float level = (100.0f/(float)(wa->mixer[mixer].range.max - wa->mixer[mixer].range.min))*wa->mixer[mixer].currentVolume;
	//lDrawPBar(frame, x, y, frame->width, 9, level, PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_SET);
}

static void drawVolumeLevelPercent (TWINAMP *wa, TFRAME *frame, int x, int y, int mixer)
{
	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	float level = (100.0f/(float)(wa->mixer[mixer].range.max - wa->mixer[mixer].range.min))*wa->mixer[mixer].currentVolume;
	TFRAME *pc = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackNumber, "%s %i%%", wa->mixer[mixer].name, (int)level);
	if (pc){
		lCopyAreaEx(pc, frame, (wa->graph.width>>1)-(pc->width>>1), y, 0, 0, pc->width-1, pc->height-1, 1, 1, LCPY_XOR);
		lDeleteFrame(pc);
	}else{
		dbprintf("drawVolumeLevelPercent(): pc == NULL\n");
	}
}

static void drawTrackProgressBar (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.trackPos && !wa->lyricsDismode){
		//lDrawPBar(frame, x, y, frame->width, 9, (100.0/(float)wa->track.length.value)*(wa->track.position.value*0.001), PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_XOR);
	
	}else if (wa->overlay.trackPos && wa->lyricsDismode){
		//lDrawPBar(frame, x, y-1, frame->width, 3, (100.0/(float)wa->track.length.value)*(wa->track.position.value*0.001), PB_MARKER_HFILL, LSP_XOR);
		lSetPixel(frame, x, y, LSP_SET);
		//lSetPixel(frame, frame->width-1, y, LSP_SET);
	}
}

static void renderFavOverlay (TWINAMP *wa, TFRAME *frame)
{
	if (frame){
		lSetCharacterEncoding(wa->hw, wa->lang->enc);
		lClearFrame(frame);
		lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, LSP_SET);
		lDrawLine(frame, 1, frame->height-1, frame->width-2, frame->height-1, LSP_CLEAR);

		if (wa->overlay.trackFav == 1){
			lPrintf(frame, 2, 0, LFTW_SNAP, LPRT_OR, "%s", lng_getString(wa->lang, LNG_HIT_HATEIT));
			lPrintf(frame, (frame->width/2)+16, 0, LFTW_SNAP, LPRT_OR, "%s", lng_getString(wa->lang, LNG_HIT_LIKEIT));
			lPrintf(frame, (frame->width/2)-2, 0, LFTW_SNAP, LPRT_OR, "%i", wa->hitlist.type);
			wa->misc.hitlistr = wa->hitlist.type;
		}else if (wa->overlay.trackFav == 2){
			lPrintf(frame, 35, 0, LFTW_SNAP, LPRT_OR, lng_getString(wa->lang, LNG_HIT_ADDED));
		}else if (wa->overlay.trackFav == 3){
			lPrintf(frame, 30, 0, LFTW_SNAP, LPRT_OR, lng_getString(wa->lang, LNG_HIT_REMOVED));
		}
	}
}

static void drawTrackFavOverlay (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (!wa->overlay.trackFav){
		return;
	}else if (wa->overlay.trackFav == 1){
		if (wa->hitlist.type != wa->misc.hitlistr){
			renderFavOverlay(wa, wa->track.favorite.text);
		}
	}else{
		if (wa->misc.hitlistr != -wa->overlay.trackFav){
			wa->misc.hitlistr = -wa->overlay.trackFav;  //force a call to renderFavOverlay() to update wa->track.favorite.text
			renderFavOverlay(wa, wa->track.favorite.text);
		}
	}
	lCopyAreaEx(wa->track.favorite.text, frame, x, y, 0, 0, wa->track.favorite.text->width-1, wa->track.favorite.text->height-1, 1, 1, LCPY_CPY);
}


static int indexToFreq (TWINAMP *wa, int i)
{
	return (double)((double)((double)wa->net.wave.sRate/(double)wa->net.wave.bpc)*(double)i)/2.0;
}

static void drawBandwidthMarkers (TWINAMP *wa, TFRAME *frame, int lower, int upper)
{
	if (wa->overlay.spectMarkers){
		const int y2 = frame->height-1;
		const float factor = (float)(frame->width-1)/(float)wa->net.wave.bpc;
		lDrawLine(frame, factor*(float)lower, 27, factor*(float)lower, y2, LSP_XOR);
		lDrawLine(frame, factor*(float)upper, 27, factor*(float)upper, y2, LSP_XOR);

		TLPRINTR rect = {0,frame->height-12,frame->width-1,frame->height-1,0,0,0,0};
		lPrintEx(frame, &rect, wa->font.trackNumber, PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_OR,\
		  "%i  %i", indexToFreq(wa, lower), indexToFreq(wa, upper));
		lDrawRectangle(frame, rect.sx-2, rect.sy-2, rect.ex/*-rect.sx*/, 1+rect.ey/*-rect.sy*/, LSP_XOR);
	}
}

static void drawTrackSeekIcon (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.trackSeek)
		lCopyAreaEx(wa->track.seekPosition.text, frame, x, y, 0, 0, wa->track.seekPosition.text->width-1, wa->track.seekPosition.text->height-1, 1, 1, LCPY_XOR);
}

static void drawTrackNumber (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.playlistPos && !wa->lyricsDismode)
		lCopyAreaEx(wa->track.number.text, frame, x, y, 0, 0, wa->track.number.text->width-1, wa->track.number.text->height-1, 1, 1, LCPY_OR);
}

static void drawTrackLength (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.trackInfo  && !wa->lyricsDismode)
		lCopyAreaEx(wa->track.length.text, frame, x, y, 0, 0, wa->track.length.text->width-1, wa->track.length.text->height-1, 1, 1, LCPY_OR);
}

static void drawTrackPosition (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.trackInfo  && !wa->lyricsDismode)
		lCopyAreaEx(wa->track.position.text, frame, x, y, 0, 0, wa->track.position.text->width-1, wa->track.position.text->height-1, 1, 1, LCPY_OR);
}

static void drawTrackKHZ (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.trackKhz){
		if (wa->overlay.trackSeek)
			x -= wa->track.seekPosition.text->width-2;
		lCopyAreaEx(wa->track.sampleRate.text, frame, x, y, 0, 0, wa->track.sampleRate.text->width-1, wa->track.sampleRate.text->height-1, 1, 1, LCPY_OR);
	}
}

static void drawTrackBPS (TWINAMP *wa, TFRAME *frame, int x, int y)
{
	if (wa->overlay.trackBps){
		if (wa->overlay.trackSeek)
			x -= wa->track.seekPosition.text->width-2;
		lCopyAreaEx(wa->track.bitRate.text, frame, x, y, 0, 0, wa->track.bitRate.text->width-1, wa->track.bitRate.text->height-1, 1, 1, LCPY_OR);
	}
}

static int drawVolume (TWINAMP *wa, TFRAME *frame, int mixer)
{
	if (mixer == VOLUME_MEDIAPLAYER){
		if (wa->overlay.volumeLevel){
			if (GetTickCount() - wa->misc.winampVolumeTime < wa->config.volumeTimeout){
				drawWinampVolumeLevel(wa, frame, 0, 18);
				drawWinampVolumeLevelPercent(wa, frame, -1, 19);
				return 1;
			}else{
				wa->overlay.volumeLevel = 0;
				return 0;
			}
		}
	}else if (!wa->mixer[mixer].status && wa->overlay.volumeLevel){
		if (GetTickCount() - wa->mixer[mixer].volumeTime < wa->config.volumeTimeout){
			drawVolumeLevel(wa, frame, 0, 18, mixer);
			if (1) drawVolumeLevelPercent(wa, frame, -1 , 19, mixer);
			return 1;
		}else{
			wa->overlay.volumeLevel = 0;
			return 0;
		}
	}
	wa->overlay.volumeLevel = 0;
	return 0;
}


int displayInput_WinampMain (TWINAMP *wa, int key, void *data)
{

	if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE){
		if (++wa->hitlist.type > 9)
			wa->hitlist.type = 9;

	}else if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE){
		if (--wa->hitlist.type < 1)
			wa->hitlist.type = 1;

	}else if (isKeyPressed(VK_LSHIFT)){	// global volume handling
		return -1;

	}else if (isKeyPressed(VK_RCONTROL) && key == G15_WHEEL_CLOCKWISE && wa->overlay.spectMarkers){
		wa->config.g15LEDUpperRange +=2;
		if (wa->config.g15LEDUpperRange > wa->net.wave.bpc)
			wa->config.g15LEDUpperRange = wa->net.wave.bpc;

	}else if (isKeyPressed(VK_RCONTROL) && key == G15_WHEEL_ANTICLOCKWISE && wa->overlay.spectMarkers){
		if (wa->config.g15LEDUpperRange > wa->config.g15LEDLowerRange+5){
			wa->config.g15LEDUpperRange -=2;
			if (wa->config.g15LEDUpperRange < 2)
				wa->config.g15LEDUpperRange = 2;
		}
	}else if (isKeyPressed(VK_LCONTROL) && key == G15_WHEEL_CLOCKWISE && wa->overlay.spectMarkers){
		if (wa->config.g15LEDLowerRange < wa->config.g15LEDUpperRange-5){
			wa->config.g15LEDLowerRange +=2;
			if (wa->config.g15LEDLowerRange > wa->net.wave.bpc-5)
				wa->config.g15LEDLowerRange = wa->net.wave.bpc-5;
		}
	}else if (isKeyPressed(VK_LCONTROL) && key == G15_WHEEL_ANTICLOCKWISE && wa->overlay.spectMarkers){
			wa->config.g15LEDLowerRange -=2;
			if (wa->config.g15LEDLowerRange < 1)
				wa->config.g15LEDLowerRange = 1;

	}else if (key == G15_WHEEL_ANTICLOCKWISE && wa->overlay.spectMarkers){
		wa->config.g15LEDLowerRange -=2;
		if (wa->config.g15LEDLowerRange < 1)
			wa->config.g15LEDLowerRange = 1;

		if (wa->config.g15LEDUpperRange > wa->config.g15LEDLowerRange+5){
			wa->config.g15LEDUpperRange -=2;
			if (wa->config.g15LEDUpperRange < 2)
				wa->config.g15LEDUpperRange = 2;
		}
	}else if (key == G15_WHEEL_CLOCKWISE && wa->overlay.spectMarkers){
		wa->config.g15LEDUpperRange +=2;
		if (wa->config.g15LEDUpperRange > wa->net.wave.bpc)
			wa->config.g15LEDUpperRange = wa->net.wave.bpc;

		if (wa->config.g15LEDLowerRange < wa->config.g15LEDUpperRange-5){
			wa->config.g15LEDLowerRange +=2;
			if (wa->config.g15LEDLowerRange > wa->net.wave.bpc-5)
				wa->config.g15LEDLowerRange = wa->net.wave.bpc-5;
		}
	}else if (key == G15_WHEEL_ANTICLOCKWISE && wa->overlay.trackSeek){
			track_Rewind(wa);

	}else if (key == G15_WHEEL_CLOCKWISE && wa->overlay.trackSeek){
			track_Forward(wa);

	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_1){
		if (++wa->config.volumeCtrl > 2)
			wa->config.volumeCtrl = 0;
		volumeSetActive(wa, wa->config.volumeCtrl);

	}else if (isKeyPressed(VK_RSHIFT) && (key == G15_SOFTKEY_2)){		// add track
		ubyte filename[4096];
		if (getPlaylistFilename(wa, (char*)filename, sizeof(filename), wa->track.number.value, CMT_UTF8)){
			hitlistTrackRemove(wa, wa->hitlist.type, filename);
			wa->overlay.trackFav = 3;
		}
	}else if (isKeyPressed(VK_RSHIFT) && (key == G15_SOFTKEY_3)){		// remove track
		ubyte filename[4096];
		if (getPlaylistFilename(wa, (char*)filename, sizeof(filename), wa->track.number.value, CMT_UTF8)){
			hitlistTrackAdd(wa, wa->hitlist.type, filename);
			wa->overlay.trackFav = 2;
		}
	}else if (isKeyPressed(VK_MENU) && key == G15_SOFTKEY_4){
		if (wa->config.encoding == CMT_BIG5)
			wa->config.encoding = CMT_GBK;
		else
			wa->config.encoding = CMT_BIG5;
		if (updateTitleScroll(wa, 1))
			seekScroll(wa, getFrame(wa, DISF_MAIN)->width-12);

	}else if (key == G15_SOFTKEY_3 && !wa->net.notFound){	// toggle graph type
		if (++wa->graph.barType > 7)
			wa->graph.barType = 1;

	}else if (key == G15_SOFTKEY_4 && !wa->net.notFound){
		toggle_overlayTrackSeek(wa);	// selects track seek or LED BW calibration mode

	}else{
		return -1;
	}
	return 0;
}

static void drawScroll (TWINAMP *wa)
{
	if (wa->overlay.scroll)
		lDrawScroll(wa->scroll.s);
}

int renderMainFrame (TWINAMP *wa, TFRAME *frame, void *data)
{
	lClearFrame(frame);
	drawScroll(wa);

	if (!drawVolume(wa, frame, wa->config.volumeCtrl)){
		drawTrackBPS(wa, frame, frame->width - wa->track.bitRate.text->width, 28);
		drawTrackKHZ(wa, frame, frame->width - wa->track.sampleRate.text->width, 36);
		drawTrackNumber(wa, frame, (wa->graph.width>>1)-(wa->track.number.text->width>>1), 19);
		drawTrackLength(wa, frame, frame->width - wa->track.length.text->width - 3, 19);
		drawTrackPosition(wa, frame, 3, 19);
		drawTrackProgressBar(wa, frame, 0, 18);
		drawTrackSeekIcon(wa, frame, frame->width-18, frame->height-16);
		drawBandwidthMarkers(wa, frame, wa->config.g15LEDLowerRange, wa->config.g15LEDUpperRange);
	}
	renderSpecGraph(wa, frame, 0, frame->height - wa->graph.height);
	drawTrackFavOverlay(wa, frame, 29, frame->height-10);

	/*
	addCaret(&wa->input, wa->input.workingBuffer, wa->input.caretBuffer, MSNSIZE_INPUTBUFFER-1);
	drawEditBox(&wa->input, frame, 0, 7, wa->input.caretBuffer, &wa->input.iOffset);
	drawCharTotal(&wa->input, frame);
	*/
	return 1;
}

int initStructs (TWINAMP *wa, TFRAME *frame)
{
	
	strcpy((char*)wa->net.mpName, "Media Player");
	
	wa->misc.winampVolumeTime = GetTickCount();
	
	wa->track.number.value = -1;
	wa->track.length.value = -1;
	wa->track.sampleRate.value = -1;
	wa->track.position.value = -1;
	wa->track.bitRate.value = -1;
	wa->track.favorite.value = -1;

	wa->graph.spectrum = (ubyte*)calloc(sizeof(ubyte), wa->graph.width);
	if (wa->graph.spectrum == NULL) return 0;
	
	wa->graph.spectrum4 = (ubyte*)malloc(sizeof(ubyte)*wa->graph.width);
	if (wa->graph.spectrum4 == NULL) return 0;
	memset(wa->graph.spectrum4, getFrame(wa, DISF_MAIN)->height, sizeof(ubyte)*wa->graph.width);
	
	wa->graph.osc.chan1 = (T2POINT*)calloc(sizeof(T2POINT), wa->graph.width);
	if (wa->graph.osc.chan1 == NULL) return 0;
	
	if (wa->track.sampleRate.text == NULL)
		wa->track.sampleRate.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.samRate, "0khz");
	if (wa->track.sampleRate.text == NULL) return 0;
	
	if (wa->track.bitRate.text == NULL)
		wa->track.bitRate.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.bitRate, "0k");
	if (wa->track.bitRate.text == NULL) return 0;
	
	if (wa->track.number.text == NULL)
		wa->track.number.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackNumber, "0/0");
	if (wa->track.number.text == NULL) return 0;
	
	if (wa->track.length.text == NULL)
		wa->track.length.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackLength, "0:0");
	if (wa->track.length.text == NULL) return 0;
	
	if (wa->track.position.text == NULL)
		wa->track.position.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.trackPosition, "0:0");
	if (wa->track.position.text == NULL) return 0;

	if (wa->track.seekPosition.text == NULL)
		wa->track.seekPosition.text = lNewString(frame->hw, NEWSTRINGTYPE, 0, wa->font.titleScroll, "&#9664;&#9654;");
	if (wa->track.seekPosition.text == NULL) return 0;
	
	if (wa->track.favorite.text == NULL)	
		wa->track.favorite.text = lNewFrame(frame->hw, 102, 10, NEWSTRINGTYPE);
	if (wa->track.favorite.text == NULL) return 0;

	if (wa->scroll.frame == NULL)
		wa->scroll.frame = lNewString(frame->hw, NEWSTRINGTYPE, PF_DONTFORMATBUFFER, wa->font.titleScroll, "<WVS "MY_VERSIONSTR"> ");
	if (wa->scroll.frame == NULL) return 0;
	
	//lSaveImage(wa->scroll.frame, L"title.bmp", IMG_BMP, 0, 0);
	
	if (wa->scroll.s == NULL)
		wa->scroll.s = lNewScroll(wa->scroll.frame, frame);
	if (wa->scroll.s == NULL) return 0;
	wa->scroll.s->desRect->x1 = wa->config.scrollX;
	wa->scroll.s->desRect->y1 = wa->config.scrollY;
	wa->scroll.s->desRect->x2 = frame->width-1;
	wa->scroll.s->desRect->y2 = wa->scroll.s->desRect->y1 + wa->scroll.frame->height-1;
	wa->scroll.s->srcRect->x2 = wa->scroll.s->srcFrm->width-1;
	wa->scroll.s->srcRect->y2 = wa->scroll.s->srcFrm->height-1;
	wa->scroll.s->flags = SCR_LOOP;
	wa->scroll.s->dir = SCR_LEFT;
	wa->scroll.s->loopGapWidth = 16;
	wa->scroll.adler = 0;
	wa->scroll.time = 0;
	
	return 1;
}
