
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

#include <shlobj.h>
#include "ml.c"

#define WVSSTAMPOFFSET 1500
#define WVSSTRINGBUFFER 4096


static uint32_t lastTrackAddler = 1;
static int total = 0;
static TMLRECORD *records = NULL;
static TLYRICS *lrc = NULL;
static int lyricsAvailable = 0;
//static int stamp = 0;
static unsigned int lastRecCheck = 0;
static char *buffer = NULL;
static wchar_t *recordFile = NULL;
static int timeOffset = 0;
static int stampModifiedTime = 0;
static int lyPage = 1;
static uint32_t crc = 0;
static char *lastLyricPtr = NULL;
volatile static TFRAMEOVER lycOvr;
static int lyInitOnce = 0;

int MLyricsInit (TFRAME *frame);
void MLyricsClose ();
int renderLyrics (TWINAMP *wa, TFRAME *frame, void *data);
int inputLyrics (TWINAMP *wa, int key, void *data);



int lyricsPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderLyrics(wa, frame, userPtr);
}

int lyricsPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return inputLyrics(wa, key, userPtr);
}

int lyricPageEnter (TFRAME *frame, void *userPtr)
{
	lastLyricPtr = NULL;
	lastTrackAddler = 0;
	//dbprintf("lyricPageEnter\n");
	return 1;
}

void lyricPageExit (void *userPtr)
{
	//dbprintf("lyricPageExit\n");
}

int lyricsPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("lyricsPageOpen()\n");
	
	
	TWINAMP *wa = (TWINAMP*)userPtr;
	if (wa->lyricsLocation == NULL)
		wa->lyricsLocation = strdup("c:\\lyrics");	// default location
	/*
	wa->lyricsStampOffset = 0;
	wa->lyricsFont = 0;
	wa->lyricsEncoding = CMT_ISO8859_1;
	wa->lyricsDismode = 0;
	wa->config.overlay.lyricsOvrlay = 1;
	*/
	
	return MLyricsInit(frame);
}

void lyricsPageClose (void *userPtr)
{
	//dbprintf("MLyricsClose()\n");
	MLyricsClose();
}

int inputLyrics (TWINAMP *wa, int key, void *data)
{
	if (!lyInitOnce) return -1;
	
	if (isKeyPressed(VK_LSHIFT)){	// global volume handling
		return -1;
		
	}else if (isKeyPressed(VK_CONTROL) && key == G15_WHEEL_CLOCKWISE){
		timeOffset += 250;
		lastLyricPtr = NULL;	// force a redraw
		renderFrame(wa, DISF_LYRICS);
		setUpdateSignal(wa);
		stampModifiedTime = GetTickCount();
		
	}else if (isKeyPressed(VK_CONTROL) && key == G15_WHEEL_ANTICLOCKWISE){
		timeOffset -= 250;
		lastLyricPtr = NULL;	// force a redraw
		renderFrame(wa, DISF_LYRICS);
		setUpdateSignal(wa);
		stampModifiedTime = GetTickCount();

	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_4){		// force lyrics to reload
		lastTrackAddler = 0;		
		lyPage = 1;
		setUpdateSignal(wa);
		renderFrame(wa, DISF_LYRICS);
				
	}else if (key == G15_SOFTKEY_3){		// select page to display (lyrics or tag)
		lyPage = (1+lyPage)&0x01;
		lastLyricPtr = NULL;
		setUpdateSignal(wa);
		renderFrame(wa, DISF_LYRICS);
		
	}else if (key == G15_SOFTKEY_4){		// reset time offset
		timeOffset = WVSSTAMPOFFSET;
		lastLyricPtr = NULL;		// force a redraw
		renderFrame(wa, DISF_LYRICS);
		setUpdateSignal(wa);
		stampModifiedTime = GetTickCount();		
		
	}else{
		return -1;
	}
	return 0;
}

int MLyricsInit (TFRAME *frame)
{
	if (lyInitOnce) return 1;


	buffer = (char*)calloc(sizeof(char), WVSSTRINGBUFFER);
	if (buffer == NULL) return 0;
	recordFile = (wchar_t*)calloc(sizeof(wchar_t), WVSSTRINGBUFFER);
	if (recordFile == NULL) return 0;
	
	if (S_OK == SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (void*)recordFile)){
		wcscat(recordFile, L"\\MiniLyrics\\MLLyrics.S2L");
		//wprintf(L"record file: '%s'\n",recordFile);
	}

	lycOvr.ownerSetsT0 = 0;
	lycOvr.t0 = 0;
	lycOvr.x = 0;
	lycOvr.y = 19;
	lycOvr.frame = lNewFrame(frame->hw, frame->width, frame->height-lycOvr.y, LFRM_BPP_1);
	lycOvr.requestedTime = 60000;
	lycOvr.ownerCleans = 1;
	lycOvr.copyMode = LCASS_OR;
	lycOvr.distination = DISF_MAIN;
	lycOvr.reuse = 1;
	lycOvr.status = 1;

	lastTrackAddler = 0;
	lastRecCheck = 0;
	lyricsAvailable = 0;
	lyPage = 1;
	timeOffset = WVSSTAMPOFFSET;
	lyInitOnce = 1;
	
	return 1;
}

static int getLyricOvrPosY (TWINAMP *wa)
{
	if (!wa->lyricsDismode)
		return 27;
	else
		return 19;
}

void MLyricsClose ()
{
	if (records != NULL){
		deleteRecords(records);
		records = NULL;
	}
	if (lrc != NULL){
		deleteLyrics(lrc);
		lrc = NULL;
	}
	if (buffer != NULL){
		free(buffer);
		buffer = NULL;
	}
	if (recordFile != NULL){
		free(recordFile);
		recordFile = NULL;
	}
	if (lycOvr.frame != NULL){
		REFRESH_LOCK();
		lDeleteFrame(lycOvr.frame);
		lycOvr.frame = NULL;
		REFRESH_UNLOCK();
	}
	lyInitOnce = 0;
}

void printTagPage (TWINAMP *wa, TFRAME *frame)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	const int fonttags = LFTW_5x7;
	
	if (lyricsAvailable){
		if (isFrameActive(wa, DISF_LYRICS)){
			int enc = lSetCharacterEncoding(wa->hw, wa->lang->enc);
			lClearFrame(frame);
			
			if (getLyricTitle(lrc)){
				lPrintEx(frame, &rect, fonttags, 0, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_TITLE));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX, LPRT_OR, "%s", getLyricTitle(lrc));
			}
				
			if (getLyricArtist(lrc)){
				lSetCharacterEncoding(wa->hw, wa->lang->enc);
				lPrintEx(frame, &rect, fonttags, PF_RESETX|PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_ARTIST));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX|PF_CLIPWRAP, LPRT_OR, "%s", getLyricArtist(lrc));
			}

			if (getLyricAlbum(lrc)){
				lSetCharacterEncoding(wa->hw, wa->lang->enc);
				lPrintEx(frame, &rect, fonttags, PF_RESETX|PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_ALBUM));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX|PF_CLIPWRAP, LPRT_OR, "%s", getLyricAlbum(lrc));
			}

			if (getLyricAuthor(lrc)){
				lSetCharacterEncoding(wa->hw, wa->lang->enc);
				lPrintEx(frame, &rect, fonttags, PF_RESETX|PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_AUTHOR));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX|PF_CLIPWRAP, LPRT_OR, "%s", getLyricAuthor(lrc));
			}

			if (getLyricEncoding(lrc)){
				lSetCharacterEncoding(wa->hw, wa->lang->enc);
				lPrintEx(frame, &rect, fonttags, PF_RESETX|PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_ENCODING));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX|PF_CLIPWRAP, LPRT_OR, "%s", getLyricEncoding(lrc));
			}

			if (getLyricRe(lrc)){
				lSetCharacterEncoding(wa->hw, wa->lang->enc);
				lPrintEx(frame, &rect, fonttags, PF_RESETX|PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_RE));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX|PF_CLIPWRAP, LPRT_OR, "%s", getLyricRe(lrc));
			}

			if (getLyricVersion(lrc)){
				lSetCharacterEncoding(wa->hw, wa->lang->enc);
				lPrintEx(frame, &rect, fonttags, PF_RESETX|PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s: ", lng_getString(wa->lang, LNG_LYR_VERSION));
				lSetCharacterEncoding(wa->hw, enc);
				lPrintEx(frame, &rect, fonttags, PF_USELASTX|PF_CLIPWRAP, LPRT_OR, "%s", getLyricVersion(lrc));
			}

			lSetCharacterEncoding(wa->hw, wa->lang->enc);	
			lPrintEx(frame, &rect, fonttags, PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s:%i, %s:%i, %s:%i",\
			  lng_getString(wa->lang, LNG_LYR_OFFSET), getLyricOffset(lrc),\
			  lng_getString(wa->lang, LNG_LYR_LYRICS), getLyricTotalLyrics(lrc),\
			  lng_getString(wa->lang, LNG_LYR_STAMPS), getLyricTotalStamps(lrc));
			  

		}else if (isFrameActive(wa, DISF_MAIN) && wa->overlay.lyricsOvrlay){
			REFRESH_LOCK();
			lClearFrame(lycOvr.frame);
			REFRESH_UNLOCK();
			
			char *lyric = NULL;
			if (getLyricTitle(lrc))
				lyric = getLyricTitle(lrc);
			else if (getLyricArtist(lrc))
				lyric = getLyricArtist(lrc);
			else if (getLyricAlbum(lrc))
				lyric = getLyricAlbum(lrc);
			else if (getLyricAuthor(lrc))
				lyric = getLyricAuthor(lrc);
			else
				lyric = "~";
				
			if (lyric != NULL){
				TLPRINTR rectrt = {0,0,lycOvr.frame->width-1, lycOvr.frame->height-1,0,0,0,0};
				lycOvr.y = getLyricOvrPosY(wa);
				REFRESH_LOCK();
				lPrintEx(lycOvr.frame, &rectrt, LFTW_HELVR12, PF_MIDDLEJUSTIFY|PF_CLIPWRAP|PF_DONTFORMATBUFFER, LPRT_OR, lyric);
				REFRESH_UNLOCK();
				PostMessage(wa->hook.hwnd, 7000, (WPARAM)&lycOvr, (LPARAM)wa);
			}
		}
	}else{
		/*if (isFrameActive(wa, DISF_MAIN)){
			lClearFrame(lycOvr.frame);
			lPrintEx(lycOvr.frame, &rect, LFTW_HELVR12, PF_MIDDLEJUSTIFY|PF_CLIPWRAP|PF_DONTFORMATBUFFER, LPRT_CPY, "Lyrics Unavailable");
			PostMessage(wa->hook.hwnd, 7000, (WPARAM)&lycOvr, (LPARAM)wa);
		}else*/ if (isFrameActive(wa, DISF_LYRICS)){		
			lClearFrame(frame);
			lPrintEx(frame, &rect, LFTW_5x7, 0, LPRT_OR, "%i: %i", wa->track.number.value+1, wa->track.position.value);
			lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPWRAP|PF_NEWLINE|PF_DONTFORMATBUFFER, LPRT_OR, buffer);
		}
	}
}
 
int printLyricPage (TWINAMP *wa, TFRAME *frame)
{
	
	if (lyricsAvailable){	// lyrics aavailable so display them
		int stamp = wa->track.position.value + timeOffset + getLyricOffset(lrc) + wa->lyricsStampOffset;
		char *slyric = getLyricInTime(lrc, &stamp);
		if (slyric){
			// if a lyric is repeated then ignore then 2'nd, 3'rd ..., repeat of the lyric
			if (lastLyricPtr == slyric)	// only update when something has changed
				return 1;
			else
				lastLyricPtr = slyric;
			
			// if title screen is active then display then lyric below the volume/track position bar 
			if (isFrameActive(wa, DISF_MAIN) && wa->overlay.lyricsOvrlay){
				REFRESH_LOCK();
				lClearFrame(lycOvr.frame);
				
				{	// display lyric in title screen (calculate height and position of text via a fake render)
				TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
				lPrintEx(lycOvr.frame, &rect, WVSFonts[wa->lyricsFont], PF_MIDDLEJUSTIFY|PF_CLIPWRAP|PF_WORDWRAP|PF_DONTFORMATBUFFER|PF_DONTRENDER, LPRT_CPY, slyric);
				REFRESH_UNLOCK();
				if (rect.ey > 20)
					wa->lyricsDismode = 1;
				else
					wa->lyricsDismode = 0;
				lycOvr.y = getLyricOvrPosY(wa) + (((frame->height-getLyricOvrPosY(wa)) - rect.ey)/2);
				if (lycOvr.y < getLyricOvrPosY(wa)) lycOvr.y = getLyricOvrPosY(wa);

				if (LFTW_5x7 == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFTW_5x7, lGetFontLineSpacing(wa->hw, LFTW_5x7)-1);
				else if (LFTW_HELVR12 == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFTW_HELVR12, lGetFontLineSpacing(wa->hw, LFTW_HELVR12)-4);
				else if (LFT_ARIAL == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFT_ARIAL, lGetFontLineSpacing(wa->hw, LFT_ARIAL)-2);
				else if (LFTW_WENQUANYI9PT == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFTW_WENQUANYI9PT, lGetFontLineSpacing(wa->hw, LFTW_WENQUANYI9PT)-2);
				}	
					
				{
				REFRESH_LOCK();
				TLPRINTR rect = {0,0,lycOvr.frame->width-1, lycOvr.frame->height-1,0,0,0,0};
				lPrintEx(lycOvr.frame, &rect, WVSFonts[wa->lyricsFont], PF_MIDDLEJUSTIFY|PF_CLIPWRAP|PF_WORDWRAP|PF_DONTFORMATBUFFER, LPRT_CPY, slyric);
				REFRESH_UNLOCK();
				}
				
				if (LFTW_5x7 == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFTW_5x7, lGetFontLineSpacing(wa->hw, LFTW_5x7)+1);
				else if (LFTW_HELVR12 == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFTW_HELVR12, lGetFontLineSpacing(wa->hw, LFTW_HELVR12)+4);
				else if (LFT_ARIAL == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFT_ARIAL, lGetFontLineSpacing(wa->hw, LFT_ARIAL)+2);
				else if (LFTW_WENQUANYI9PT == WVSFonts[wa->lyricsFont])
					lSetFontLineSpacing(wa->hw, LFTW_WENQUANYI9PT, lGetFontLineSpacing(wa->hw, LFTW_WENQUANYI9PT)+2);

				PostMessage(wa->hook.hwnd, 7000, (WPARAM)&lycOvr, (LPARAM)wa);

			}else if (isFrameActive(wa, DISF_LYRICS)){
				lClearFrame(frame);
				TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
				lPrintEx(frame, &rect, WVSFonts[wa->lyricsFont], PF_CLIPWRAP|PF_WORDWRAP|PF_DONTFORMATBUFFER|PF_MIDDLEJUSTIFY, LPRT_CPY, slyric);

				//printf("%i\n",rect.ey);
				if (WVSFonts[wa->lyricsFont] == LFTW_5x7){
					if (rect.ey < 28) rect.ey = rect.sy = 28;
				}else if (WVSFonts[wa->lyricsFont] == LFTW_WENQUANYI9PT){
					if (rect.ey < 28) rect.ey = rect.sy = 28;
				}else if (WVSFonts[wa->lyricsFont] == LFTW_B16B){
					if (rect.ey < 21) rect.ey = rect.sy = 21;
					else if (rect.ey > 30) return 1;
				}else if (WVSFonts[wa->lyricsFont] == LFTW_HELVR12){
					if (rect.ey <= 30) rect.ey = rect.sy = 29;
				}else/* if (font == LFT_SMALLFONTS7X7){
					if (rect.ey < 29) rect.ey = rect.sy = 29;
				}else if (font == LFT_COMICSANSMS7X8){
					if (rect.ey < 29) rect.ey = rect.sy = 29;
				}else if (font == LFTW_SNAP){
					if (rect.ey < 29) rect.ey = rect.sy = 29;
				}else if (font == LFT_ARIAL)*/{
					if (rect.ey < 29) rect.ey = rect.sy = 29;
				}
				
				if ((slyric=getNextLyric(lrc, &stamp)) && rect.sy < frame->height-7){
					lPrintEx(frame, &rect, WVSFonts[wa->lyricsFont], PF_CLIPWRAP|PF_WORDWRAP|PF_DONTFORMATBUFFER|PF_MIDDLEJUSTIFY, LPRT_CPY, slyric);
					//if ((slyric=getNextLyric(lrc, &stamp)) && rect.sy < frame->height-7)
						//lPrintEx(frame, &rect, font, PF_CLIPWRAP|PF_CLIPDRAW|PF_CLIPTEXTV|PF_NEWLINE|PF_MIDDLEJUSTIFY, LPRT_OR, "\n%s", slyric);
				}
			}else{
				return 0;
			}
		}else{
			printTagPage(wa, frame);
		}
	}else{	// lyrics unavailable
	
		// recheck for a lyrics file ever 1.5 seconds
		if (GetTickCount()-lastRecCheck > 1500)	
			lastTrackAddler = 0;
			
		// notify title screen of that a lyric file can not be found
		if (isFrameActive(wa, DISF_MAIN) && wa->overlay.lyricsOvrlay){
			REFRESH_LOCK();
			TLPRINTR rectrt = {0,0,lycOvr.frame->width-1, lycOvr.frame->height-1,0,0,0,0};
			lSetCharacterEncoding(wa->hw, wa->lang->enc); 
			lPrintEx(lycOvr.frame, &rectrt, LFTW_HELVR12, PF_MIDDLEJUSTIFY|PF_CLIPWRAP|PF_WORDWRAP|PF_DONTFORMATBUFFER, LPRT_CPY, lng_getString(wa->lang, LNG_LYR_LYRICSUNAVAILABLE));
			lycOvr.y = frame->height-16;
			REFRESH_UNLOCK();
			PostMessage(wa->hook.hwnd, 7000, (WPARAM)&lycOvr, (LPARAM)wa);
		}
		// can't display a blank screen so just print some general info
		TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
		if (isFrameActive(wa, DISF_LYRICS)){
			lClearFrame(frame);
			lPrintEx(frame, &rect, LFTW_5x7, 0, LPRT_OR, "%i: %i", wa->track.number.value+1, wa->track.position.value);
		}
		// [re]check lyrics file availability
		if (getPlaylistTrack(wa, buffer, WVSSTRINGBUFFER, wa->track.number.value, CMT_UTF8)){
			char *upath = (char*)calloc(1, strlen(buffer)+WVSSTRINGBUFFER);
			if (upath != NULL){
				sprintf(upath, "%s\\%s.lrc", wa->lyricsLocation, buffer);
				//dbprintf("upath: '%s'\n", upath);
				
				if (isFrameActive(wa, DISF_LYRICS))
					lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPWRAP|PF_NEWLINE, LPRT_OR, "%s", buffer);

				if (!lastTrackAddler){
					if (lrc != NULL)
						deleteLyrics(lrc);
					if ((lrc=newLyricsU(upath))){
						lyricsAvailable = loadLyricFile(lrc, lrc->longwPath);
						if (!lyricsAvailable)
							lyricsAvailable = loadLyricFile(lrc, lrc->shortwPath);
					}
					if (lyricsAvailable){
						lastTrackAddler = crc;
						lastRecCheck = GetTickCount();
					}
				}
				free(upath);
			}
		}
	}

	return 1;
}
	
int renderLyrics (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (wa->net.notFound){
		lClearFrame(frame);
		drawPlayerNotFound(frame, getRandomFont());
		return 1;
	}

	if (!lyInitOnce){
		if (!MLyricsInit(frame))
			return 0;
	}
	if (!isFrameActive(wa, DISF_LYRICS) && !isFrameActive(wa, DISF_MAIN))
		return 1;

	if (!getPlaylistFilename(wa, buffer, WVSSTRINGBUFFER, wa->track.number.value, CMT_UTF8))
		return 0;

	crc = getAddler((ubyte*)buffer, strlen(buffer));
	if (lastTrackAddler != crc){
		lastTrackAddler = crc;
		lastRecCheck = GetTickCount();
		lyricsAvailable = 0;
		timeOffset = WVSSTAMPOFFSET;
		lastLyricPtr = NULL;
		//printf("track: %s\n", buffer);
		if (lycOvr.frame){
			REFRESH_LOCK();
			lClearFrame(lycOvr.frame);
			REFRESH_UNLOCK();
		}
		if (records != NULL){
			deleteRecords(records);
			records = NULL;
		}
		if (lrc != NULL){
			deleteLyrics(lrc);
			lrc = NULL;
		}

		if (recordFile[0])
			records = getRecords(recordFile, &total);
		if (records){
			lrc = newLyricsU(buffer);
			if (lrc)
				lyricsAvailable = loadLyricRecord(records, total, lrc);
		}else{
			wprintf(L"record file not found: '%%APPDATA%%\\%s'\n", L"MLLyrics.S2L");
		}
	}

	int lspace = 0;
	if (WVSFonts[wa->lyricsFont] == LFTW_ROUGHT18){
		lspace = lGetFontLineSpacing(wa->hw, LFTW_ROUGHT18);
		lSetFontLineSpacing(wa->hw, LFTW_ROUGHT18, lspace-1);
	}

	if (lyricsAvailable == 2)
		lSetCharacterEncoding(wa->hw, CMT_UTF8);
	else
		lSetCharacterEncoding(wa->hw, wa->lyricsEncoding);
	
	if (!lyPage){
		printTagPage(wa, frame);
	}else{
		printLyricPage(wa, frame);
	}

	if (WVSFonts[wa->lyricsFont])
		lSetFontLineSpacing(wa->hw, LFTW_ROUGHT18, lspace);

	// print offset
	if (isFrameActive(wa, DISF_LYRICS)){
		unsigned int ticks = GetTickCount();
		if (ticks-stampModifiedTime < 3000){
			TFRAME *total;
			if ((timeOffset-WVSSTAMPOFFSET) == 0)
				total = lNewString(frame->hw, NEWSTRINGTYPE, PF_DONTFORMATBUFFER, LFTW_ROUGHT18, "0");
			else
				total = lNewString(frame->hw, NEWSTRINGTYPE, 0, LFTW_ROUGHT18, "%+i", timeOffset-WVSSTAMPOFFSET);
			if (total != NULL){
				lDrawRectangleFilled(frame, frame->width-total->width, 0, frame->width-1, total->height-3, LSP_SET);
				lCopyAreaEx(total, frame, frame->width-total->width+1, 0, 0, 2, total->width-1, total->height-1, 1, 1, LCASS_XOR);
				lDeleteFrame(total);
			}
		}else if (ticks-stampModifiedTime < 4000){
			lastLyricPtr = NULL;	// force a redraw
		}
	}

	return 1;
}

static int lyricOffsetToIdx (int stamp)
{
	int i = 15;
	while(i--){
		if (lyricOffsetTable[i] == stamp)
			return i;
	}
	return 7; // index 7 = time 0ms
}

static void lyricsOverlayEnable ()
{
	lycOvr.requestedTime = 60000;
	lycOvr.status = 1;
}


static void lyricsOverlayDisable ()
{
	lycOvr.requestedTime = 0;
	lycOvr.status = 0;
}

static void lyricsOverlaySetStatus (int status)
{
	if (!status)
		lyricsOverlayDisable();
	else
		lyricsOverlayEnable();
}
