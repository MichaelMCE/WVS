

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


static void toggleCaret (TMSNINPUT *input)
{
	if (input->caretChar == 5)	// custom chars added to 5x7.bdf
		input->caretChar = 6;
	else
		input->caretChar = 5;
}

static int addCaret (TMSNINPUT *input, char *src, char *des, size_t desSize)
{
	toggleCaret(input);
	size_t srcLen = strlen(src);
	
	if (!srcLen){
		des[0] = input->caretChar;
		des[1] = 0;
	}else if (input->caretPos >= input->tKeys){
		strncpy(des, src, srcLen);
		des[srcLen] = input->caretChar;
		des[srcLen+1] = 0;
	}else{
		memset(des, 0, MSNSIZE_INPUTBUFFER);
		strncpy(des, src, input->caretPos);
		des[input->caretPos] = input->caretChar;
		strncpy(&des[input->caretPos+1], src+input->caretPos, strlen(src+input->caretPos));
	}
	return 1;
}

static void drawEditBox (TMSNINPUT *input, TFRAME *frame, int x, int y, char *ptext, unsigned int *offset)
{
	
	TLPRINTR rect = {x+2,y+2,frame->width-3,frame->height-5,x,y,x,y+7};
	TLPRINTR rect2; //metrics rect
	memcpy(&rect2, &rect, sizeof(TLPRINTR));
	const int renderFlags = PF_DONTFORMATBUFFER|PF_WORDWRAP|PF_CLIPWRAP|PF_CLIPTEXTV;
	char *text = ptext+*offset;

	lPrintEx(frame, &rect2, LFTW_5x7, renderFlags|PF_DONTRENDER, LPRT_CPY, text);
	if (!*input->workingBuffer){
		rect2.ex = x+1;	// we've got a rect now factor in a border
		rect2.ey += 2;
	}

	if (rect2.ey < 20 || !input->tKeys){
		if (!input->tKeys)
			lClearFrameArea(frame, x, y, rect2.ex+2, rect2.ey-1);
		else
			lClearFrameArea(frame, x, y, rect2.ex, rect2.ey);
	}else{
		lClearFrameArea(frame, x, y, frame->width-1, rect2.ey+1);
	}
		
	int tabTotal = 0;
	int i = 0;
	int charMax = lPrintEx(frame, &rect, LFTW_5x7, renderFlags, LPRT_CPY, text);

	while(text[i] && i <= charMax+*offset){
		if (text[i] == '\t')
			tabTotal++;
		i++;
	}
	
	charMax = charMax - (tabTotal*3);
	if ((input->caretPos > charMax+*offset-10) && input->tKeys >= charMax+*offset){
		(*offset)++;
		drawEditBox(input, frame, x, y, ptext, offset);
	}
	if (!*input->workingBuffer){
		rect.ex = x+1;
		rect.ey += 2;
	}
	//drawVLineDotted(frame, rect.ex, rect.sy, rect.ey);
	if (rect.ey < 20 || !input->tKeys){
		if (!input->tKeys)
			lDrawRectangle(frame, x, y, rect.ex+3, rect.ey-1, LSP_SET);
		else
			lDrawRectangle(frame, x, y, rect.ex, rect.ey+1, LSP_SET);
	}else{
		lDrawRectangle(frame, x, y, frame->width-1, rect.ey+1, LSP_SET);
	}
}

static void drawCharTotal (TMSNINPUT *input, TFRAME *frame)
{
	TFRAME *total = lNewString(frame->hw, NEWSTRINGTYPE, 0, LFTW_5x7, "%i", strlen(input->workingBuffer));
	if (total != NULL){
		lDrawRectangleFilled(frame, frame->width - total->width, 0, frame->width-1, total->height-1, LSP_SET);
		lCopyAreaEx(total, frame, frame->width-total->width+1, 1, 0, 0, total->width-1, total->height-1, 1, 1, LCASS_XOR);
		lDeleteFrame(total);
	}else{
		dbprintf("editbox_drawCharTotal(): total == NULL\n");
	}
}

static int addKey (TMSNINPUT *input, int key, int position)
{
	//printf("addkey %i\n",key);
	
	if (/*key == '[' || key == '{' ||*/ key == '{' || key == '}'){
		//printf("invalid key %i\n",key);
		key = 32;
	}

	if (input->tKeys < MSNSIZE_INPUTBUFFER){
		if (position == input->tKeys){
			input->caretPos++;
			input->workingBuffer[input->tKeys++] = key;
			return 1;
		}else{
			char *src = input->workingBuffer;
			char *des = input->caretBuffer;
			
			memset(des, 0, MSNSIZE_INPUTBUFFER);
			strncpy(des, src, position);
			des[position] = key;
			strncpy(&des[position+1], src+position, strlen(src+position));
			strncpy(src, des, strlen(input->caretBuffer));
			
			input->caretPos++;
			input->tKeys++;
			return 1;
		}
	}
	return 0;

}

static void addKeyTab (TMSNINPUT *input)
{
	addKey(input, '\t', input->caretPos);
}

static int deleteKey (TMSNINPUT *input, int position)
{
	char *src = input->workingBuffer;
	char *des = input->caretBuffer;
	
	memset(des, 0, MSNSIZE_INPUTBUFFER);
	strncpy(des, src, input->caretPos-1);
	strncpy(&des[input->caretPos-1], src+input->caretPos, strlen(src+input->caretPos));
	memset(src, 0, MSNSIZE_INPUTBUFFER);
	strncpy(src, des, strlen(input->caretBuffer));
	
	if (input->iOffset) input->iOffset--;
	if (input->caretPos) input->caretPos--;
	if (input->tKeys) input->tKeys--;
	return 1;
	
}

static int getPreviousKey (TMSNINPUT *input)
{
	if (input->caretPos < MSNSIZE_INPUTBUFFER && input->caretPos > 0)
		return input->workingBuffer[input->caretPos-1];
	else
		return 0;
}


void pasteClipBoardText (HWND hwnd, TMSNINPUT *input, int emoticons)
{
	if (OpenClipboard(hwnd)){
		HANDLE handle = GetClipboardData(CF_TEXT);
		if (handle){
			ubyte *buffer = (ubyte*)GlobalLock(handle);
			if (buffer){
				while(*buffer){
					if (*buffer == VK_TAB){
						addKeyTab(input);
					}else if (*buffer < 32){
						addKey(input, ' ', input->caretPos);
					}else if (emoticons){
						if (*buffer == '(' && getPreviousKey(input) == ':'){
							addKey(input, 4, input->caretPos-1);
							deleteKey(input, input->caretPos);
						}else if (*buffer == ')' && getPreviousKey(input) == ':'){
							addKey(input, 3, input->caretPos-1);
							deleteKey(input, input->caretPos);
						}else{
							addKey(input, *buffer, input->caretPos);
						}
					}else{
						addKey(input, *buffer, input->caretPos);
					}
					buffer++;
				}
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
}

static void clearWorkingBuffer (TMSNINPUT *input)
{
	memset(input->workingBuffer, 0, MSNSIZE_INPUTBUFFER);
	input->iOffset = input->caretPos = input->tKeys = 0;
}

static int nextHistoryBuffer (TMSNINPUT *input)
{
	if (++input->historyBufferi >= MSNSIZE_WORKINGBUFFERS)
		input->historyBufferi = 0;
	return input->historyBufferi;
}

static int previousHistoryBuffer (TMSNINPUT *input)
{
	if (--input->historyBufferi < 0)
		input->historyBufferi = MSNSIZE_WORKINGBUFFERS-1;
	return input->historyBufferi;
}

static void addWorkingBuffer (TMSNINPUT *input)
{
	memcpy(input->buffers[input->historyBufferi], input->workingBuffer, MSNSIZE_INPUTBUFFER);
	//strncpy(input->buffers[input->historyBufferi], input->workingBuffer, MSNSIZE_INPUTBUFFER);
}

static void addHistoryBuffer (TMSNINPUT *input)
{
	memcpy(input->workingBuffer, input->buffers[input->historyBufferi], MSNSIZE_INPUTBUFFER);
	//strncpy(input->workingBuffer, input->buffers[input->historyBufferi], MSNSIZE_INPUTBUFFER);
	input->caretPos = input->tKeys = strlen(input->workingBuffer);
	input->iOffset = 0;
}


int editBoxInputProc (TMSNINPUT *input, HWND hwnd, int key, int emoticons)
{
	if (key == VK_LSHIFT || key == VK_SHIFT || key == VK_RSHIFT)
		return 0;
			
	//printf("%i\n",key);

	if (key&0x1000){
		key &= ~0x1000;
			
		if (key >= VK_PRIOR && key <= VK_DELETE){
			//printf("%i\n",key);
			if (key == VK_LEFT){
				if (--input->caretPos < 1){
					input->caretPos = 0;
					input->iOffset = 0;
				}
			}else if (key == VK_RIGHT){
				if (++input->caretPos > input->tKeys)
					input->caretPos = input->tKeys;

			}else if (key == VK_DELETE){
				if (++input->caretPos > input->tKeys){
					input->caretPos = input->tKeys;
				}else{
					deleteKey(input, input->caretPos);
				}
			}else if (key == VK_UP){
				previousHistoryBuffer(input);
				addHistoryBuffer(input);
					
			}else if (key == VK_DOWN){
				nextHistoryBuffer(input);
				addHistoryBuffer(input);
					
			}else if (key == VK_HOME){
				input->caretPos = 0;
				input->iOffset = 0;
					
			}else if (key == VK_END){
				input->caretPos = input->tKeys;
				input->iOffset = 0;
					
			}else{
				return 0;
			}
			return 1;
		}
		return 0;
	 		
	}else if (key == 22){			// control+v
		pasteClipBoardText(hwnd, input, emoticons);
		
	}else if (key == VK_RETURN){
		kbHookOff();
		return 2;
			
	}else if (key == VK_ESCAPE){		// softkeys 1/2/3 and esc
		kbHookOff();
		return 1;
			
	}else if (key == VK_TAB){
		addKeyTab(input);
			
	}else if (key == VK_BACK){
	 	if (input->tKeys && input->caretPos)
	 		deleteKey(input, input->caretPos);

	}else if (key == G15_SOFTKEY_1 || key == G15_SOFTKEY_2 || key == G15_SOFTKEY_3 || key == G15_SOFTKEY_4){
		kbHookOff();
		return -1;

	}else if (key < 31){
		// do nothing
			
	}else if (emoticons){	// do emoticons
		if (key == '(' && getPreviousKey(input) == ':'){
			addKey(input, 4, input->caretPos-1);
			deleteKey(input, input->caretPos);
		}else if (key == ')' && getPreviousKey(input) == ':'){
			addKey(input, 3, input->caretPos-1);
			deleteKey(input, input->caretPos);
		}else{
			addKey(input, key, input->caretPos);
		}
	}else{
		addKey(input, key, input->caretPos);
	}

	return 1;

}

