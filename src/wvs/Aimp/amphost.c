
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



static int asktoutf16 (ubyte *r, size_t r_n, wchar_t *wc, size_t wc_n)
{
	int i;
	ubyte *pp = (ubyte *)wc;

	for (i = 0; i < r_n && *r; i++){
		*pp++ = *r++;
		*pp++ = 0;
	}
	wc[i] = 0;
	return i;
}

int amp_sendTrackTitleW (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKTITLEW;
	
	size_t cmdLen = sizeof(TMPCMD);
	char *titlesrc = (char*)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTTITLE);
	if (titlesrc){
		size_t itemLen = strlen(titlesrc);
		if (itemLen){
			wchar_t *title = (wchar_t*)calloc(itemLen+sizeof(wchar_t), sizeof(wchar_t));
			if (title){
				asktoutf16(titlesrc, itemLen++, title, 0);
				if (sendString(mp, cmd, title, itemLen*sizeof(wchar_t))){
					free(title);
					return 1;
				}
				free(title);
			}
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int amp_sendTrackFilenameW (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKFILENAMEW;
	
	size_t cmdLen = sizeof(TMPCMD);
	char *titlesrc = (char*)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTFILE);
	if (titlesrc){
		size_t itemLen = strlen(titlesrc);
		if (itemLen){
			wchar_t *title = (wchar_t*)calloc(itemLen+sizeof(wchar_t), sizeof(wchar_t));
			if (title){
				asktoutf16(titlesrc, itemLen++, title, 0);
				if (sendString(mp, cmd, title, itemLen*sizeof(wchar_t))){
					free(title);
					return 1;
				}
				free(title);
			}
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

