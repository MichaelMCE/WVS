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


int renderPlayList (TWINAMP *wa, TFRAME *frame,  void *data);
int displayInput_Playlist (TWINAMP *wa, int key, void *data);

void playlistPlayHighlightedTrack (TWINAMP *wa, TPLAYLIST *list);
void playlistHighlightNextItem (TWINAMP *wa, TPLAYLIST *list);
void playlistHighlightPreviousItem (TWINAMP *wa, TPLAYLIST *list);



int playlistPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	if (wa->playlist.enabled)
		lUpdateScroll(wa->playlist.scroll);
	return renderPlayList(wa, frame, userPtr);
}

int playlistPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Playlist(wa, key, userPtr);
}

int playlistPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("playlistPageEnter\n");
	TPLAYLIST *list = (TPLAYLIST*)userPtr;
	list->highlightPos = list->wa->track.number.value;
	list->start = list->highlightPos;
	return 1;
}

void playlistPageExit (void *userPtr)
{
	//dbprintf("playlistPageExit\n");
}

int playlistPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("playlistPageOpen()\n");
	return 1;
}

void playlistPageClose (void *userPtr)
{
	//dbprintf("playlistPageClose()\n");
}


void playlistPlayHighlightedTrack (TWINAMP *wa, TPLAYLIST *list)
{
	netPlayTrackIndex(&wa->net.conn, list->highlightPos, 0);
}

void playlistHighlightPreviousItem (TWINAMP *wa, TPLAYLIST *list)
{
	list->highlightPos--;
	if (list->highlightPos < 0)
		list->highlightPos = 0;

	if (list->highlightPos < list->start)
		list->start--;

	if (list->start < 0)
		list->start = 0;

	renderFrame(wa, getActivePage(wa));
}

void playlistHighlightNextItem (TWINAMP *wa, TPLAYLIST *list)
{
	list->highlightPos++;
	if (list->highlightPos > list->totalTracks-1)
		list->highlightPos = list->totalTracks-1;

	if (list->highlightPos >= list->end)
		list->start++;

	if (list->start > list->totalTracks-1)
		list->start = list->totalTracks-1;

	renderFrameBlock(wa, getActivePage(wa));

	if (list->highlightPos >= list->end){
		list->start++;
		if (list->start > list->totalTracks-1)
			list->start = list->totalTracks-1;
		renderFrame(wa, getActivePage(wa));
	}
}

static void drawHitlistNotFound (TFRAME *frame, int font, TPLAYLIST *list)
{
	TLPRINTR rect = {1,-1,frame->width-1,frame->height-1,0,0,0,0};
	char name[128];
	snprintf(name, sizeof(name), "%s%i.%s", HITLISTNAME, list->type, HITLISTEXT);
	lPrintEx(frame, &rect, font, PF_CLIPWRAP|PF_WORDWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, "%s\nnot found or empty", name);
	list->enabled = 0;
	list->track = -1;
}

static int drawPlaylistItem (TWINAMP *wa, TPLAYLIST *list, TFRAME *frame, TLPRINTR *rect, int font, int index, char *item)
{
	if (list->type)
		index--;

	if (wa->track.number.value == index && !list->type){
		return lPrintEx(frame, rect, font, PF_CLIPDRAW|PF_RESETX|PF_NEWLINE, LPRT_OR, ">>%s<<", item);
	}else if (!wa->overlay.trackNumber){
		return lPrintEx(frame, rect, font, PF_CLIPDRAW|PF_RESETX|PF_NEWLINE|PF_DONTFORMATBUFFER, LPRT_OR, item);
	}else{
		return lPrintEx(frame, rect, font, PF_CLIPDRAW|PF_RESETX|PF_NEWLINE, LPRT_OR, "%i:%s", index+1, item);
	}
}

static int getHitlistTrack (TWINAMP *wa, char *buffer, size_t buffersize, int item, int enc)
{
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];

	if (wa->hitlist.al->lines[item]){
		_splitpath((char*)wa->hitlist.al->lines[item], NULL, NULL, name, ext);
		sprintf(buffer, "%s%s", name, ext);

		if (enc != CMT_UTF8){
			wchar_t track[4096];
			int ret = MultiByteToWideChar(CP_UTF8, 0, buffer, buffersize, track, 4095);
			if (ret > 0 && ret < 4095){
				int i;
				for (i = 0; i < ret; i++)
					buffer[i] = (ubyte)(track[i]&0xFF);
				buffer[i] = 0;
			}
		}
		return 1;
	}
	return 0;
}

static int getTextHeight (TFRAME *frame, char *text, int font)
{
	int height = 0;
printf("playlist a\n");
	lGetTextMetrics(frame->hw, text, PF_CLIPDRAW, font, NULL, &height);
printf("playlist b\n");
	return height;
}

int displayInput_Playlist (TWINAMP *wa, int key, void *data)
{
	TPLAYLIST *list = (TPLAYLIST*)data;

	if ((key == G15_WHEEL_CLOCKWISE || key == G15_WHEEL_ANTICLOCKWISE) && !isKeyPressed(VK_LSHIFT))
		list->time0 = GetTickCount(); // reset playlist scroller

	if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE){ 	// scroll forward playlist by n%
		list->highlightPos += ((list->totalTracks/100.0)*(float)list->scrollpercent);
		list->start = list->highlightPos;

		if (!list->type){
			cacheTrackTitles(wa, list->start-1, list->start+6);
			lSleep(1);
		}
		setUpdateSignal(wa);
		playlistHighlightNextItem(wa, list);

	}else if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE){	//scroll back playlist by n%
		list->highlightPos -= ((list->totalTracks/100.0)*(float)list->scrollpercent);
		list->start = list->highlightPos;

		if (!list->type){
			cacheTrackTitles(wa, list->start-1, list->start+6);
			lSleep(1);
		}
		setUpdateSignal(wa);
		playlistHighlightPreviousItem(wa, list);

	}else if (isKeyPressed(VK_LSHIFT)){	// global volume handling
		return -1;

	}else if (key == G15_WHEEL_ANTICLOCKWISE){	// hightlight up/previous track
		playlistHighlightPreviousItem(wa, list);
		if (!list->type){
			cacheTrackTitles(wa, list->start-6, list->start);
			setUpdateSignal(wa);
			renderFrame(wa, DISF_PLAYLIST);
		}
	}else if (key == G15_WHEEL_CLOCKWISE){	// highlight down/next track
		playlistHighlightNextItem(wa, list);
		if (!list->type){
			cacheTrackTitles(wa, list->start, list->start+6);
			setUpdateSignal(wa);
			renderFrame(wa, DISF_PLAYLIST);
		}
	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_3){	// change font
		if (!WVSFonts[++list->fontType])
			list->fontType = 0;
		list->highlightPos = list->start;
		renderFrame(wa, getActivePage(wa));

	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_4){		// flush string cache
		if (!list->type){
			cache_Flush(&wa->cache);
			cacheTrackTitles(wa, list->start-6, list->start);
			Sleep(10);
			setUpdateSignal(wa);
			renderFrame(wa, DISF_PLAYLIST);
		}
	}else if (isKeyPressed(VK_MENU) && key == G15_SOFTKEY_4){
		if (wa->config.encoding == CMT_BIG5)
			wa->config.encoding = CMT_GBK;
		else
			wa->config.encoding = CMT_BIG5;

	}else if (key == G15_SOFTKEY_3){		// play highlighted track
		playlistPlayHighlightedTrack(wa, list);

	}else if (key == G15_SOFTKEY_4){		// toggle track number display
		toggle_overlayTrackNumber(wa);
		renderFrame(wa, DISF_PLAYLIST);

	}else{
		return -1;
	}
	return 0;
}

static void _swapint (int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

static void invertRegion (TFRAME *frame, int x1, int y1, int x2, int y2)
{
	if (x2 < x1) _swapint(&x2,&x1);
	if (y2 < y1) _swapint(&y2,&y1);
	y2++;
	int y;
	for (y = y1; y<y2; y++)
		lDrawLine(frame, x1, y, x2, y, LSP_XOR);
}

static void invertTextRegion (TFRAME *frame, int font, int x1, int y1, int x2, int y2)
{
	if (font == LFT_SMALLFONTS7X7 || font == LFT_COMICSANSMS7X8 || font == LFTW_5x7 || font == LFT_ARIAL)
		y2--;
	invertRegion(frame, x1, y1, x2, y2);
}

static void seekPlaylistScroll (TLSCROLLEX *scroll, int pos)
{
	scroll->flags &= ~SCR_LOOP;
	int i;
	//pos = MIN(pos, scroll->srcFrm->width-2);

	for (i = 0; i < pos; i++)
		lUpdateScroll(scroll);
	scroll->flags |= SCR_LOOP;
}

static int createPlaylistScroll (TPLAYLIST *list, TFRAME *des, TLPRINTR *rect, int font, char *text)
{
	if (list->frame != NULL)
		lDeleteFrame(list->frame);

	list->frame = lNewString(des->hw, NEWSTRINGTYPE, PF_DONTFORMATBUFFER, font, text);
	if (list->frame == NULL){
		list->enabled = 0;
		dbprintf("createPlaylistScroll(): list->frame == NULL\n");
		return 0;
	}

	if (list->scroll != NULL)
		lDeleteScroll(list->scroll);

	list->scroll = lNewScroll(list->frame, des);
	if (list->scroll == NULL){
		list->enabled = 0;
		dbprintf("createPlaylistScroll(): list->scroll == NULL\n");
		return 0;
	}

	list->scroll->desRect->x1 = 0;
	list->scroll->desRect->y1 = rect->sy;
	list->scroll->desRect->x2 = des->width-1;
	list->scroll->srcRect->x2 = list->scroll->srcFrm->width-1;
	list->scroll->srcRect->y2 = list->scroll->srcFrm->height-1;
	list->scroll->flags = SCR_LOOP;
	list->scroll->dir = SCR_LEFT;
	list->enabled = 1;
	if (font == LFT_SMALLFONTS7X7 || font == LFT_COMICSANSMS7X8 || font == LFTW_5x7)
		list->scroll->desRect->y2 = rect->ey-1;
	else
		list->scroll->desRect->y2 = rect->ey;
	return 1;
}

static int addPlaylistTrackScroll (TPLAYLIST *list, TFRAME *frame, TLPRINTR *rect, int font, int index, int enc, char *item)
{
	uint32_t adler = getAddler((uint8_t*)item, strlen(item));
	if (!list->enabled || list->adler != adler || list->enc != enc || list->scroll->desRect->y1 != rect->sy || list->track != index || list->font != font || list->ttracks != list->totalTracks){
		list->ttracks = list->totalTracks;
		list->track = index;
		list->adler = adler;
		list->font = font;
		list->enc = enc;

		if (createPlaylistScroll(list, frame, rect, font, item)){
			seekPlaylistScroll(list->scroll, frame->width);
			lDrawScroll(list->scroll);
		}
	}else{	// copy scroll to surface, no not advance or update
		lDrawScroll(list->scroll);
	}
	return 1;
}

int renderPlayList (TWINAMP *wa, TFRAME *frame, void *data)
{
	TPLAYLIST *list = (TPLAYLIST *)data;
	if (list == NULL) return 0;

	TLPRINTR rect = {1,-1,frame->width-1,frame->height-1,0,0,0,0};
	int font = WVSFonts[list->fontType];
	int index = list->start;
	char item[4096];
	

	if (list->highlightPos > list->totalTracks)
		list->highlightPos = 0;

	if (wa->config.encoding == CMT_UTF16)
		lSetCharacterEncoding(wa->hw, CMT_UTF8);	// utf16 encodings are handled internally as utf8, to make life easy
	else									// getTrackTitle() converts utf16 to utf8
		lSetCharacterEncoding(wa->hw, wa->config.encoding);
	int enc = lGetCharacterEncoding(wa->hw);

	lClearFrame(frame);
	if (wa->net.notFound){
		drawPlayerNotFound(frame, getRandomFont());
		resetPlaylistScroller(wa);
		return 1;
	}

	if (list->type){
		if (!hitlistOpen(wa, list->type)){
			drawHitlistNotFound(frame, getRandomFont(), list);
			return 1;
		}
		list->totalTracks = wa->hitlist.al->tlines;
		if (list->totalTracks < 2){ // first track is always the utf8 encoding marker
			drawHitlistNotFound(frame, getRandomFont(), list);
			return 1;
		}
		// skip past UTF8 encoding marker
		if (list->highlightPos < 1) list->highlightPos = 1;
		if (list->start < 1) index = list->start = 1;
	}

	do{
		if (index > list->totalTracks-1){
			index--;
			break;
		}
		if (index < 0) index = 0;

		if (!list->type){
			if (!getPlaylistTrack(wa, item, sizeof(item), index, enc)){
				sprintf(item, " ... #%i", index+1);
				//cacheTrackTitles(wa, index, index);
			}
		}else{
			if (!getHitlistTrack(wa, item, sizeof(item), index, enc)){
				index--;
				break;
			}
		}

		if (getTextHeight(frame, item, font)+rect.ey > frame->height/*-1*/){
			break;
		}

		if (!drawPlaylistItem(wa, list, frame, &rect, font, index, item)){
			break;
		}

		if (index == list->highlightPos){
			if (GetTickCount()-list->time0 > 2000){
				addPlaylistTrackScroll(list, frame, &rect, font, index, enc, item);
			}else{
				invertTextRegion(frame, font, rect.sx-1, rect.sy, rect.ex-1, rect.ey);
				list->enabled = 0;
			}
		}

		if (font == LFT_ARIAL)
			rect.ey++;
		index++;
	}while((rect.ey < frame->height-4) && (index < list->totalTracks));

	if (list->type)
		hitlistClose(wa);

	list->end = index;
	//lSaveImage(frame, "playlist.tga", IMG_TGA, frame->width, frame->height);
	return 1;
}


#if 0
// testing new playlist api
// root playlist. displays root nodes
static int renderRootPlaylists (TWINAMP *wa, TFRAME *frame, void *data)
{
	TPLAYLIST *list = (TPLAYLIST *)data;
	TLPRINTR rect = {1,-1,frame->width-1,frame->height-1,0,0,0,0};

	if (list->type || !wa->misc.tPlaylists)
		return renderPlayList(wa, frame, data);
	else
		list->totalTracks = wa->misc.tPlaylists;

	//dbprintf("%i\n",wa->misc.tPlaylists);

	wchar_t buffer16[4096];
	char buffer8[4096*sizeof(wchar_t)];

	if (list->highlightPos > list->totalTracks)
		list->highlightPos = 0;

	if (list->start > list->totalTracks)
		list->start = 0;

	int index = list->start;

	lClearFrame(frame);
	lSetCharacterEncoding(wa->hw, CMT_UTF8);
	int font = WVSFonts[list->fontType];

		do{
		if (index > list->totalTracks-1){
			index--;
			break;
		}

		if (netGetPlaylistName(&wa->net.conn, index, buffer16, sizeof(buffer16))){
			if (utf8_wcstombs((ubyte*)buffer8, sizeof(buffer8), buffer16, wcslen(buffer16))){
				if (getTextHeight(frame, buffer8, font)+rect.ey > frame->height/*-1*/)
					break;
				lPrintEx(frame, &rect, font, PF_CLIPDRAW|PF_RESETX|PF_NEWLINE, LPRT_OR, "+ %s", buffer8);
				if (index == list->highlightPos)
					invertTextRegion(frame, font, rect.sx, rect.sy, rect.ex, rect.ey);
			}
		}

		index++;
	}while((rect.ey < frame->height-4) && (index < list->totalTracks));

	list->end = index;
	return 0;
}
#endif

