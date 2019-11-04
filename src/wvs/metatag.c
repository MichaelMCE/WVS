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


static int displayInput_Metadata (TWINAMP *wa, int key, void *data);
static int renderMetaData (TWINAMP *wa, TFRAME *frame, void *data);



int metatagPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderMetaData(wa, frame, userPtr);
}

int metatagPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Metadata(wa, key, userPtr);
}

int metatagPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("metatagPageEnter\n");
	TWINAMP *wa = (TWINAMP*)userPtr;
	wa->meta.track = wa->track.number.value;
	return 1;
}

void metatagPageExit (void *userPtr)
{
	//dbprintf("metatagPageExit\n");
}

int metatagPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("metatagPageOpen()\n");
	return 1;
}

void metatagPageClose (void *userPtr)
{
	//dbprintf("metatagPageClose()\n");
}


static int displayInput_Metadata (TWINAMP *wa, int key, void *userPtr)
{

	TMETAPAGE *meta = &wa->meta;

	if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE){ // scroll list forwards
		if (++meta->track > wa->playlist.totalTracks-1)
			meta->track = wa->playlist.totalTracks-1;
		cacheMetaTags(wa, meta->track, meta->track+1);
		cacheTrackFilenames(wa, meta->track, meta->track+1);
		renderFrame(wa, DISF_META);

	}else if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE){	//scroll list backwards
		if (--meta->track < 0 )
			meta->track = 0;
		cacheMetaTags(wa, meta->track-1 , meta->track);
		cacheTrackFilenames(wa, meta->track-1 , meta->track);
		renderFrame(wa, DISF_META);

	}else if (isKeyPressed(VK_LSHIFT)){	// global volume handling
		return -1;

	}else if (key == G15_WHEEL_ANTICLOCKWISE){	// hightlight up/previous track
		if (--meta->tag < 0)
			meta->tag = 0;
		renderFrame(wa, DISF_META);

	}else if (key == G15_WHEEL_CLOCKWISE){	// highlight down/next track
		if (++meta->tag > METATAGTOTAL+1)
			meta->tag = METATAGTOTAL+1;
		renderFrame(wa, DISF_META);

	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_3){
		if (!WVSFonts[++wa->playlist.fontType])
			wa->playlist.fontType = 0;
		wa->playlist.highlightPos = wa->playlist.start;
		renderFrame(wa, DISF_META);

	}else if (key == G15_SOFTKEY_3){	// play current track
		netPlayTrackIndex(&wa->net.conn, meta->track, 0);

	}else if (isKeyPressed(VK_MENU) && key == G15_SOFTKEY_4){
		if (wa->config.encoding == CMT_BIG5)
			wa->config.encoding = CMT_GBK;
		else
			wa->config.encoding = CMT_BIG5;

	}else{
		return -1;
	}
	return 0;
}

char *getTagName (TWINAMP *wa, int i)
{
	return lng_getString(wa->lang, LAG_MT_TITLE+i);
}

static int renderMetaData (TWINAMP *wa, TFRAME *frame, void *data)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	
	lClearFrame(frame);
	if (wa->net.notFound){
		drawPlayerNotFound(frame, getRandomFont());
		return 1;
	}

	if (wa->config.encoding == CMT_UTF16)
		lSetCharacterEncoding(wa->hw, CMT_UTF8);
	else
		lSetCharacterEncoding(wa->hw, wa->config.encoding);
	int enc = lGetCharacterEncoding(wa->hw);

	const int font = WVSFonts[wa->playlist.fontType];
	int space = lGetFontLineSpacing(wa->hw, font);
	if (font == LFTW_WENQUANYI9PT)
		lSetFontLineSpacing(wa->hw, font, space-2);
	else if (font == LFTW_SNAP)
		lSetFontLineSpacing(wa->hw, font, space-3);

	int i, printed = 1;
	for (i=wa->meta.tag; (i<METATAGTOTAL) && (rect.ey < frame->height-1) && printed; i++){
		int e = getMetaTag(wa, buffer, sizeof(buffer), i, wa->meta.track, enc);
		lSetCharacterEncoding(wa->hw, wa->lang->enc);
		printed = lPrintEx(frame, &rect, font, PF_RESETX|PF_NEWLINE|PF_CLIPWRAP|PF_WORDWRAP, LPRT_OR, "%s:", getTagName(wa, i));
		lSetCharacterEncoding(wa->hw, e);
		lPrintEx(frame, &rect, font, PF_USELASTX|PF_CLIPWRAP|PF_WORDWRAP, LPRT_OR, "%s", buffer);

		if (font == LFT_SMALLFONTS7X7 || font == LFT_COMICSANSMS7X8 || font == LFT_ARIAL)
			rect.ey++;
	}

	if (i == METATAGTOTAL){
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];

		if (wa->config.alwaysUseUTF8)
			enc = CMT_UTF8;
		if (getPlaylistFilename(wa, buffer, sizeof(buffer), wa->meta.track, enc)){
			_splitpath(buffer, drive, dir, NULL, NULL);
			sprintf(buffer, "%s%s", drive, dir);
			lPrintEx(frame, &rect, font, PF_NEWLINE|PF_CLIPTEXTV|PF_CLIPWRAP|PF_CLIPTEXTH|PF_DONTFORMATBUFFER, LPRT_OR, buffer);
		}
	}

	char name[_MAX_FNAME];
	char ext[_MAX_EXT];

	if (wa->config.alwaysUseUTF8)
		enc = CMT_UTF8;
	if (getPlaylistFilename(wa, buffer, sizeof(buffer), wa->meta.track, enc)){
		_splitpath(buffer, NULL, NULL, name, ext);
		sprintf(buffer, "%s%s", name, ext);
		if (font == LFT_SMALLFONTS7X7 || font == LFT_COMICSANSMS7X8 || font == LFT_ARIAL)
			rect.ey++;
		if (rect.ey < frame->height-4)
			lPrintEx(frame, &rect, font, PF_NEWLINE|PF_CLIPTEXTV|PF_CLIPWRAP|PF_CLIPTEXTH|PF_DONTFORMATBUFFER, LPRT_OR, buffer);
	}
	lSetFontLineSpacing(wa->hw, font, space);
	//lSaveImage(frame, "meta.bmp", IMG_BMP, frame->width, frame->height);
	return 1;
}

