
// Winamp EQ

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



// #define IPC_GETEQDATA 127
/* (requires Winamp 2.05+)
** int data=SendMessage(hwnd_winamp,WM_WA_IPC,pos,IPC_GETEQDATA);
** IPC_GETEQDATA queries the status of the EQ. 
** The value returned depends on what 'pos' is set to:
** Value      Meaning
** ------------------
** 0-9        The 10 bands of EQ data. 0-63 (+20db - -20db)
** 10         The preamp value. 0-63 (+20db - -20db)
** 11         Enabled. zero if disabled, nonzero if enabled.
** 12         Autoload. zero if disabled, nonzero if enabled.
*/

#define MAXEQ		63.0


int displayInput_EQ (TWINAMP *wa, int key, void *data);
int renderEQ (TWINAMP *wa, TFRAME *frame, void *data);



int eqPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderEQ(wa, frame, userPtr);
}

int eqPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_EQ(wa, key, userPtr);
}

int eqPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("eqPageEnter\n");
	return 1;
}

void eqPageExit (void *userPtr)
{
	//dbprintf("eqPageExit\n");
}

int eqPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("eqPageOpen()\n");
	return 1;
}

void eqPageClose (void *userPtr)
{
	//dbprintf("eqPageClose()\n");

}

int renderEQ (TWINAMP *wa, TFRAME *frame, void *data)
{
	TEQ *eqctl = (TEQ *)data;
	int mode = eqctl->mode;
	int bar = eqctl->bar;
	int direction = eqctl->direction;

	int const barwidth = 6;
	//float dataf = 0;
	int i, x = 36;
	//int y = 0;
	

	lClearFrame(frame);
	if (wa->net.notFound){
		drawPlayerNotFound(frame,  getRandomFont());
		return 1;
	}

	wa->net.eq.first = 1;
	wa->net.eq.last = 13;	// 1 to 11 = eq data, 12 and 13 are eq settings

	if (!wa->net.notFound && !eqctl->getnewdata){
		netGetEQData(&wa->net.conn, &wa->net.eq);
	}else{
		eqctl->getnewdata = 0;
	}
	
	for (i = 0; i < 11; i++){
		if (i == bar){
			if (mode == 1){
				if (!direction){
					if (wa->net.eq.band[i] > 1){
						wa->net.eq.band[i]--;
						if (!netSetEQData(&wa->net.conn, &wa->net.eq))
							return 0;
					}
				}else{
					if (wa->net.eq.band[i] < MAXEQ){
						wa->net.eq.band[i]++;
						if (!netSetEQData(&wa->net.conn, &wa->net.eq))
							return 0;
					}
				}
				if (!netGetEQData(&wa->net.conn, &wa->net.eq))
					return 0;
			}
			lPrintf(frame, 125, frame->height-6, LFTW_5x7, LPRT_CPY, "%d", 32-wa->net.eq.band[i]);
			if (i == 10){
				lDrawLine(frame, 0, 20, 0, 23, LSP_SET);
				lDrawLine(frame, barwidth+3, 20, barwidth+3, 23, LSP_SET);
			}else{
				lDrawLine(frame, x-2, 20, x-2, 23, LSP_SET);
				lDrawLine(frame, x+barwidth+1, 20, x+barwidth+1, 23, LSP_SET);
				/*
				lDrawLine(frame, x-3, 18, x-1, 20, LSP_SET);
				lDrawLine(frame, x-3, 25, x-1, 23, LSP_SET);
				lDrawLine(frame, x+barwidth, 20, x+barwidth+2, 18, LSP_SET);
				lDrawLine(frame, x+barwidth, 23, x+barwidth+2, 25, LSP_SET);
				*/
			}
		}

		if (i == 10) x = 2;
		//dataf = (float)(100.0-((100.0/MAXEQ)*(float)wa->net.eq.band[i]));
		//lDrawPBar(frame, x, y, barwidth, 43, dataf, PB_BORDER_VLINE|PB_MARKER_VBAR, LSP_SET);
		x += barwidth + 3;
	}

	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	
	lPrintf(frame, 8, 0 , LFTW_5x7, LPRT_OR, "+12%s", lng_getString(wa->lang, LNG_EQ_DB));
	lPrintf(frame, 8, 19, LFTW_5x7, LPRT_OR, " 0%s", lng_getString(wa->lang, LNG_EQ_DB));
	lPrintf(frame, 8, 37, LFTW_5x7, LPRT_OR, "-12%s", lng_getString(wa->lang, LNG_EQ_DB));
	lPrintf(frame, 135, 0, LFTW_5x7, LPRT_OR, "%s:", lng_getString(wa->lang, LNG_EQ_EQ));
	lPrintf(frame, 132, 21, LFTW_5x7, LPRT_OR, "%s:", lng_getString(wa->lang, LNG_EQ_AUTO));

	if (!wa->net.eq.band[11])
		lPrintf(frame, 135, 9, LFTW_5x7, LPRT_OR, lng_getString(wa->lang, LNG_OFF));
	else
		lPrintf(frame, 135, 9, LFTW_5x7, LPRT_OR, lng_getString(wa->lang, LNG_ON));

	if (!wa->net.eq.band[12])
		lPrintf(frame, 135, 30, LFTW_5x7, LPRT_OR, lng_getString(wa->lang, LNG_OFF));
	else
		lPrintf(frame, 135, 30, LFTW_5x7, LPRT_OR, lng_getString(wa->lang, LNG_ON));

	return 1;	
}

int displayInput_EQ (TWINAMP *wa, int key, void *data)
{
	TEQ *eq = (TEQ*)data;

	if (isKeyPressed(VK_LSHIFT)){
		return -1;
		
	}else if ((isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE) || key == G15_SOFTKEY_3){
		if (--eq->bar < 0)
			eq->bar = 10;
		eq->mode = 0; eq->direction = 0;
		renderFrame(wa, DISF_EQ);
		
	}else if ((isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE) || key == G15_SOFTKEY_4){
		if (++eq->bar > 10)
			eq->bar = 0;
		eq->mode = 0; eq->direction = 0;
		renderFrame(wa, DISF_EQ);
		
	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		setUpdateSignal(wa);
		eq->mode = 1; eq->direction = 1;
		renderFrameBlock(wa, DISF_EQ);
		eq->mode = 0; eq->direction = 0;
		
	}else if (key == G15_WHEEL_CLOCKWISE){
		setUpdateSignal(wa);
		eq->mode = 1; eq->direction = 0;
		renderFrameBlock(wa, DISF_EQ);
		eq->mode = 0; eq->direction = 0;
	}else{
		return -1;
	}
	return 0;
}

