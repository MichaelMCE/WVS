
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

#if !defined (BUILDING_DLL)
#include "../utf8.c"
#include "../1252to88591.h"
#endif



static XMPFUNC_STATUS *xmp_status = NULL;



static int CP1252BlockBackToANSI (int c)
{
	int i;
	for (i = 0; ansi1252table[i].wc; i++){
		if (ansi1252table[i].wc == c)
			return ansi1252table[i].c;
	}
	return '?';
}

static int utf8_to_ansi (char *src, size_t srcLen, char *des)
{
	
	int i = 0;
	unsigned int wc = 0;
	int c = 0;
	while(i < srcLen){
		i += UTF8ToUTF32a(src+i, &wc);
		if (wc > 0xFF){
			//*des++ = '?';
			*des++ = CP1252BlockBackToANSI(wc);
		}else{
			*des++ = wc;
		}
		c++;
	}
	return c;
}

int xmp_sendTrackFilenameW (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKFILENAMEW;
	
	size_t cmdLen = sizeof(TMPCMD);
	char *titlesrc = (char*)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTFILE);
	if (titlesrc){
		size_t itemLen = strlen(titlesrc);
		if (itemLen){
			char *title = (char*)calloc(itemLen+sizeof(char), sizeof(char));
			if (title){
				strncpy(title, titlesrc, itemLen);
				memset(mp->buffer.WChar, 0, SENDBUFFERSIZE);
				int ret = MultiByteToWideChar(CP_UTF8, 0, title, /*itemLen*/-1, mp->buffer.WChar, SENDBUFFERSIZE/sizeof(wchar_t));	
				if (ret > 0 && ret < SENDBUFFERSIZE/sizeof(wchar_t)){
					if (sendString(mp, cmd, mp->buffer.WChar, ret*sizeof(wchar_t))){
						free(title);
						return 1;
					}
				}
				free(title);
			}
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int xmp_sendTrackTitleW (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKTITLEW;
	
	size_t cmdLen = sizeof(TMPCMD);
	char *titlesrc = (char*)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTTITLE);
	if (titlesrc){
		size_t itemLen = strlen(titlesrc);
		if (itemLen){
			char *title = (char*)calloc(itemLen+sizeof(char), sizeof(char));
			if (title){
				strncpy(title, titlesrc, itemLen);
				memset(mp->buffer.WChar, 0, SENDBUFFERSIZE);
				int ret = MultiByteToWideChar(CP_UTF8, 0, title, /*itemLen*/-1, mp->buffer.WChar, SENDBUFFERSIZE/sizeof(wchar_t));	
				if (ret > 0 && ret < SENDBUFFERSIZE/sizeof(wchar_t)){
					if (sendString(mp, cmd, mp->buffer.WChar, ret*sizeof(wchar_t))){
						free(title);
						return 1;
					}
				}
				free(title);
			}
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

/*
	data2 = track index
	data3 = title length (bytes)
*/
int xmp_sendTrackFilenameA (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKFILENAMEA;
	
	size_t cmdLen = sizeof(TMPCMD);
	char *titlesrc = (char*)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTFILE);
	if (titlesrc){
		size_t itemLen = strlen(titlesrc);
		if (itemLen){
			char *title = (char*)calloc(itemLen+sizeof(char), sizeof(char));
			if (title){
				strncpy(title, titlesrc, itemLen);
				memset(mp->buffer.WChar, 0, SENDBUFFERSIZE);
				int ret = utf8_to_ansi(title, itemLen, mp->buffer.AChar);
				if (ret > 0 && ret < SENDBUFFERSIZE/sizeof(char)){
					if (sendString(mp, cmd, mp->buffer.AChar, ++ret*sizeof(char))){
						free(title);
						return 1;
					}
				}
				free(title);
			}
		}
	}

	sendNullString(mp, cmd);
	return 0;
}


/*
	data2 = track index
	data3 = title length (bytes)
*/
int xmp_sendTrackTitleA (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKTITLEA;
	
	size_t cmdLen = sizeof(TMPCMD);
	char *titlesrc = (char*)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTTITLE);
	if (titlesrc){
		size_t itemLen = strlen(titlesrc);
		if (itemLen){
			char *title = (char*)calloc(itemLen+sizeof(char), sizeof(char));
			if (title){
				strncpy(title, titlesrc, itemLen);
				memset(mp->buffer.WChar, 0, SENDBUFFERSIZE);
				int ret = utf8_to_ansi(title, itemLen, mp->buffer.AChar);
				if (ret > 0 && ret < SENDBUFFERSIZE/sizeof(char)){
					if (sendString(mp, cmd, mp->buffer.AChar, ++ret*sizeof(char))){
						free(title);
						return 1;
					}
				}
				free(title);
			}
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int xmp_mediaCtrlFastForward (TMP *mp, TMPCMD *cmd)
{
	exeDDECmd("key82");
	return 1;
}

int xmp_mediaCtrlRewind (TMP *mp, TMPCMD *cmd)
{
	exeDDECmd("key83");
	return 1;
}

int xmp_sendMPVersion (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);
	if (!cmd->data1)
		cmd->data1 = cmd->data1 = 0x3040;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}


/*
	only send TMPGENTRACKINFO in reply
*/
int xmp_sendGeneralTrackInfo (TMP *mp, TMPCMD *cmd)
{
	TMPGENTRACKINFO gen;
	gen.command = cmd->command;
	gen.magic = cmd->magic;
	gen.length = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);
	gen.playlistpos = (int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
	gen.position = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
	gen.bitrate = 0; //(short)SendMessage(mp->hwndParent, WM_WA_IPC, 1, IPC_GETINFO);
	gen.samplerate = 0; //(ubyte)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETINFO);
	mp->totalTracks = gen.totaltracks = (int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	gen.channels = 0;

	if (xmp_status){
		const XMPFORMAT *format = xmp_status->GetFormat(0);
		if (format){
			gen.channels = format->chan;
			gen.samplerate = (format->rate/1000)&0xFF;
		}
	}

	if (gen.channels < 1)
		gen.channels = 2;
		
	size_t genLen = sizeof(TMPGENTRACKINFO);	
	if (sendSocket(&mp->net, &gen, &genLen) != SOCKET_ERROR)
		return 1;
	return 0;
}
