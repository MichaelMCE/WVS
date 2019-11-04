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


int displayInput_Hitlist (TWINAMP *wa, int key, void *data);



int hitlistPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	if (wa->hitlist.enabled)
		lUpdateScroll(wa->hitlist.scroll);
	return renderPlayList(wa, frame, userPtr);
}

int hitlistPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Hitlist(wa, key, userPtr);
}

int hitlistPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("hitlistPageEnter\n");
	TPLAYLIST *list = (TPLAYLIST*)userPtr;
	list->start = list->highlightPos;
	return 1;
}

void hitlistPageExit (void *userPtr)
{
	//dbprintf("hitlistPageExit\n");
}

int hitlistPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("hitlistPageOpen()\n");
	return 1;
}

void hitlistPageClose (void *userPtr)
{
	//dbprintf("hitlistPageClose()\n");

}

static int hitlistTrackRemove (TWINAMP *wa, int num, ubyte *mp3path)
{
	hitlistOpen(wa, num);
	int ret = hitlistWrite(wa, num, mp3path, 0);
	hitlistClose(wa);
	return ret;
}

static int hitlistTrackAdd (TWINAMP *wa, int num, ubyte *mp3path)
{
	hitlistOpen(wa, num);
	int ret = hitlistWrite(wa, num, mp3path, 1);
	hitlistClose(wa);
	return ret;
}

int displayInput_Hitlist (TWINAMP *wa, int key, void *data)
{
	TPLAYLIST *list = (TPLAYLIST*)data;

	if ((key == G15_WHEEL_CLOCKWISE || key == G15_WHEEL_ANTICLOCKWISE) && !isKeyPressed(VK_LSHIFT))
		list->time0 = GetTickCount();

	if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE){ 	// scroll forward playlist by n%
		list->highlightPos += ((list->totalTracks/100.0)*(float)list->scrollpercent);
		list->start = list->highlightPos;
		playlistHighlightNextItem(wa, list);
		if (list->highlightPos < 1) list->highlightPos = 1;

	}else if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE){	//scroll back playlist by n%
		list->highlightPos -= ((list->totalTracks/100.0)*(float)list->scrollpercent);
		list->start = list->highlightPos;
		playlistHighlightPreviousItem(wa, list);
		if (list->highlightPos < 1) list->highlightPos = 1;

	}else if (isKeyPressed(VK_LSHIFT)){
		return -1;

	}else if (key == G15_WHEEL_ANTICLOCKWISE){	// hightlight up/previous track
		playlistHighlightPreviousItem(wa, list);
		if (list->highlightPos < 1) list->highlightPos = 1;
		if (list->start < 1) list->start = 1;

	}else if (key == G15_WHEEL_CLOCKWISE){	// highlight down/next track
		playlistHighlightNextItem(wa, list);
		if (list->highlightPos < 1) list->highlightPos = 1;
		if (list->start < 1) list->start = 1;

	}else if (isKeyPressed(VK_CONTROL) && (key == G15_SOFTKEY_3)){	// cycle font
		if (!WVSFonts[++list->fontType])
			list->fontType = 0;
		list->highlightPos = list->start;
		renderFrame(wa, DISF_HITLIST);

	}else if (isKeyPressed(VK_MENU) && (key == G15_SOFTKEY_3)){		// alt+b3, remove track from hitlist
		char filename[4096];
		hitlistIndexToFile(wa, list->highlightPos, list->type, filename, sizeof(filename));
		hitlistTrackRemove(wa, list->type, (ubyte*)filename);

	}else if (isKeyPressed(VK_SHIFT) && (key == G15_SOFTKEY_3)){	// enqueue track to media player
		hitlistEnqueueFile(wa, list->highlightPos, list->type, 0);

	}else if (key == G15_SOFTKEY_3){		// play this track
		hitlistEnqueueFile(wa, list->highlightPos, list->type, 1);

	}else if (isKeyPressed(VK_MENU) && key == G15_SOFTKEY_4){	// swap between big5 and gbk encoding
		if (wa->config.encoding == CMT_BIG5)
			wa->config.encoding = CMT_GBK;
		else
			wa->config.encoding = CMT_BIG5;

	}else if (key == G15_SOFTKEY_4){		// toggle track number display
		toggle_overlayTrackNumber(wa);
		renderFrame(wa, DISF_HITLIST);

	}else{
		return -1;
	}
	return 0;
}


