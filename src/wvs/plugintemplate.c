// myLCD
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2007  Michael McElligott
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


/*
	flags:
	TWP_RENDER_ACTIVE		// request render only when active/in view
	TWP_RENDER_NONACTIVE	// request render whether active or not
	
	TWP_TIMER_1SECOND		// requet 1 second render interval
	TWP_TIMER_2SECOND		// requet 2 second render interval
	TWP_TIMER_SYNC			// request render call period in sync with internal engine update rate.
	
	TWP_VU
	TWP_SPECTRUM			// neither 3 are on by default
	TWP_WAVE
*/

#include <mylcd.h>

#define TWP_RENDER_ACTIVE		0x0001
#define TWP_RENDER_NONACTIVE	0x0002
#define TWP_TIMER_1SECOND		0x0004
#define TWP_TIMER_2SECOND		0x0008
#define TWP_TIMER_SYNC			0x0010
#define TWP_VU					0x0020
#define TWP_SPECTRUM			0x0040
#define TWP_WAVE				0x0080

typedef struct{
	size_t	size;		// size of this struct
	int		flags;		// determine behaviour of plugin
	void	*userData;	// 
	int (*name) (TFRAME *frame, int font, void *userData);				// used to render title of plugin.
	int (*key) (TFRAME *frame, int key, int data, void *userData);		// input callback
	int (*render) (TFRAME *frame, void *userData);						// render your frame here
	int (*init) (TFRAME *frame, void *userData);						// start your plugin here
	void (*shutdown) (void *userData);									// shutdown here. init() may still be called after a shutdown() call.
}TWPLUGIN;

typedef struct{
	size_t	size;		// size of this struct
	int		flags;		// determine behaviour of plugin
	void	*userData;	// 
	int (*name) (TFRAME *frame, int font, void *userData);				// used to render title of plugin.
	int (*key) (TFRAME *frame, int key, int data, void *userData);		// input callback
	int (*render) (TFRAME *frame, void *userData);						// render your frame here
	int (*init) (TFRAME *frame, void *userData);						// start your plugin here
	void (*shutdown) (void *userData);									// shutdown here. init() may still be called after a shutdown() call.
	
	int (*vu) (int which, int level, void *userData);					// 
	int	(*spectrum) (void *buffer, size_t buffersize, int nchannels, void *userData);
	int	(*wave) (void *buffer, size_t buffersize, int nchannels, void *userData);
	void *net;
}TWPLUGINEX;

static TWPLUGIN plugin;

int name (TFRAME *frame, int font, void *userData);
int key (TFRAME *frame, int key, int data, void *userData);
int render (TFRAME *frame, void *userData);
int init (TFRAME *frame, void *userData);
void shutdown (void *userData);



#ifdef __cplusplus
extern "C" {
#endif

__declspec (dllexport) TWPLUGIN *wPlugin (int which)
{
	if (!which){
		plugin.size = sizeof(TWPLUGIN);
		plugin.flags = TWP_RENDER_ACTIVE|TWP_TIMER_1SECOND;
		plugin.userData	= NULL;
		plugin.name = name;
		plugin.key = key;
		plugin.render = render;
		plugin.init = init;
		plugin.shutdown = shutdown;
		/*plugin.vu = NULL;
		plugin.spectrum	= NULL;
		plugin.wave	= NULL;
		plugin.net = NULL;
		return &plugin;*/
	}else{
		return NULL;
	}
}

#ifdef __cplusplus
}
#endif


int name (TFRAME *frame, int font, void *userData)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lPrintEx(frame, &rect, font, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,(ubyte*)"Plugin Template");
	return 1;
}

int key (TFRAME *frame, int key, int data, void *userData)
{
	return -1;
}

int render (TFRAME *frame, void *userData)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	lClearFrame(frame);
	lPrintEx(frame, &rect, LFTW_SNAP, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,(ubyte*)"Hello World.");
	lDrawRectangle(frame, 0, 0, frame->width, frame->height, LSP_SET);
	return 1;
}

int init (TFRAME *frame, void *userData)
{
	
	/*
	Width of display = frame->width			//
	Height of display = frame->height		// 	Never assume these to be fixed.
	*/
	return 1;
}

void shutdown (void *userData)
{
	return;
}

