
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




int renderAbout (TWINAMP *wa, TFRAME *frame, void *data);
static unsigned int page = 0;





int abtPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderAbout(wa, frame, userPtr);
}

int abtPageInput (TWINAMP *wa, int key, void *userPtr)
{
	if (key == G15_SOFTKEY_4){
		page ^= 1;
		renderFrame(wa, DISF_ABOUT);
	}else{
		return -1;
	}
	return 0;
}

int abtPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("abtPageEnter\n");
	return 1;
}

void abtPageExit (void *userPtr)
{
	//dbprintf("abtPageExit\n");
}

int abtPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("abtPageOpen()\n");
	return 1;
}

void abtPageClose (void *userPtr)
{
	//dbprintf("abtPageClose()\n");

}

int renderAbout (TWINAMP *wa, TFRAME *frame, void *data)
{
	TLPRINTR rect = {0,2,frame->width-1,frame->height-1,0,0,0,0};
	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	lClearFrame(frame);
	
	if (!page){
		lPrintEx(frame, &rect, LFTW_SNAP, PF_MIDDLEJUSTIFY|PF_CLIPWRAP|PF_RESETXY, LPRT_CPY,\
		  "%s myLCD v%s\nmylcd.sourceforge.net\n%s",\
		  lng_getString(wa->lang, LNG_ABT_COMPILEDWITH), (char*)lVersion(), mySELF);
		  
		lDrawRectangleDotted(frame,0,0,frame->width-1, frame->height-1, LSP_SET);
	}else{
		rect.by1 = 0;
		lPrintEx(frame, &rect, LFT_COMICSANSMS7X8, PF_CLIPWRAP|PF_RESETXY, LPRT_CPY,\
		  "host: %s:%i\n"
		  "host version: %i 0x%X %i %i 0x%X\n"
		  "packets sent: %i\n"
		  "packets received: %i",
		  wa->net.address, wa->net.port,
		  wa->net.cfgProto, wa->net.version.v1, wa->net.version.v2, wa->net.version.v3, wa->net.version.vMP,
		  wa->net.conn.sendCt, wa->net.conn.readCt);
	}
	return 1;
}




