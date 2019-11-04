
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



static HANDLE thandle = NULL;
static TCMDP command[CMD_TOTAL+1];
static int handleClientCommand (TMP *mp, TMPCMD *cmd);
int sendPlaylistTotal (TMP *mp, TMPCMD *cmd);
int playTrackIndex (TMP *mp, TMPCMD *cmd);

/* helper functions */

static void clearSendBuffer (TMP *mp)
{
	memset(mp->buffer.AChar, 0, SENDBUFFERSIZE);
}

int sendString (TMP *mp, TMPCMD *cmd, void *text, size_t len)
{
	cmd->data3 = (unsigned short)len;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR){
		if (sendSocket(&mp->net, text, &len) != SOCKET_ERROR)
			return 1;
	}	
	return 0;
}

int sendNullString (TMP *mp, TMPCMD *cmd)
{
#if (REQUESTPULLHANDLER)
	cmd->data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	sendSocket(&mp->net, cmd, &cmdLen);
	return 0;
#else
	return 1;
#endif
}

/* end of helper functions */


/* multiple playlist api */

int sendTotalPlaylists (TMP *mp, TMPCMD *cmd)
{
	cmd->data1 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

int sendPlaylistName (TMP *mp, TMPCMD *cmd)
{
	sendNullString(mp, cmd);
	return 0;	
}

int sendPlaylistTrackTotal (TMP *mp, TMPCMD *cmd)
{
	return sendPlaylistTotal (mp, cmd);
}

int sendPlaylistTrackTitle (TMP *mp, TMPCMD *cmd)
{
	sendNullString(mp, cmd);
	return 0;	
}

int sendPlaylistTrackArtist (TMP *mp, TMPCMD *cmd)
{
	sendNullString(mp, cmd);
	return 0;	
}

int sendPlaylistTrackAlbum (TMP *mp, TMPCMD *cmd)
{
	sendNullString(mp, cmd);
	return 0;	
}

int sendPlaylistTrackPath (TMP *mp, TMPCMD *cmd)
{
	sendNullString(mp, cmd);
	return 0;	
}

/* multi playlist api ends*/ 


/*
	CMD_ENQUEUEFILEW
	data1 size of inbound data in bytes including trailing null(s)
	play track if data2 == 1.
	
*/
int enqueueFileW (TMP *mp, TMPCMD *cmd)
{
	if (cmd->data1){
		wchar_t *buffer = (wchar_t*)calloc(cmd->data1, sizeof(char)); // data1 = total number of bytes
		if (buffer == NULL) return 0;
		if (readSocket(&mp->net, buffer, cmd->data1) != SOCKET_ERROR){
			COPYDATASTRUCT cds;
			cds.dwData = IPC_PLAYFILEW;
			cds.lpData = (void*)buffer;
			cds.cbData = cmd->data1;
			int ret = SendMessage(mp->hwndParent, WM_COPYDATA, 0, (LPARAM)&cds);
			if (cmd->data2 == 1 && ret){
				cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH)-1;
				playTrackIndex(mp, cmd);
			}
			free(buffer);
			return 1;
		}
		free(buffer);
	}
	return 0;
}


/*
	CMD_ENQUEUEFILEA
	data1 size of inbound data in bytes including trailing null(s)
	play track if data2 == 1.
*/
int enqueueFileA (TMP *mp, TMPCMD *cmd)
{
	if (cmd->data1){
		char *buffer = (char*)calloc(cmd->data1, sizeof(char));
		if (buffer == NULL) return 0;
		if (readSocket(&mp->net, buffer, cmd->data1) != SOCKET_ERROR){
			COPYDATASTRUCT cds;
			cds.dwData = IPC_ENQUEUEFILE;
			cds.lpData = (void*)buffer;
			cds.cbData = strlen(buffer)+sizeof(char);
			int ret = SendMessage(mp->hwndParent, WM_COPYDATA, 0, (LPARAM)&cds);
			if (cmd->data2 == 1 && ret){
				cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH)-1;
				playTrackIndex(mp, cmd);
			}
			free(buffer);
			return 1;
		}
		free(buffer);
	}
	return 0;
}


/*
	data2 = file pos in playlist
	data3 = size of buffer required for incoming meta field
*/
int sendMetaDataA (TMP *mp, TMPCMD *cmd)
{

	if (!cmd->data3) return 0;
	extendedFileInfoStruct ext;
	cmd->command = CMD_GET_METADATAA;
		
	char *meta = calloc(cmd->data3, sizeof(char));
	if (meta == NULL) return 0;

	clearSendBuffer(mp);
	if (readSocket(&mp->net, meta, cmd->data3) != SOCKET_ERROR){
		ext.filename = (char *)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTFILE);
		ext.metadata = meta;
		ext.ret = mp->buffer.AChar; 
		ext.retlen = SENDBUFFERSIZE-sizeof(char);
		SendMessage(mp->hwndParent,WM_WA_IPC, (DWORD)&ext, IPC_GET_EXTENDED_FILE_INFO);
		
		//printf("A, %i :%s: :%s:", cmd->data2, meta, mp->buffer.AChar);
		//printf("\n");
		
		size_t dataSize = 0;
		if (!strlen(ext.ret)){
			dataSize = (strlen("  ") + sizeof(char)) * sizeof(char);
			if (sendString(mp, cmd, "  ", dataSize)){
				free(meta);
				return 1;
			}
		}else{
			dataSize = (strlen(ext.ret) + sizeof(char)) * sizeof(char);
			if (sendString(mp, cmd, ext.ret, dataSize)){
				free(meta);
				return 1;
			}
		}
	}

	free(meta);
	return 0;
}

/*
	data1 = file pos in playlist
	data2 = size of buffer required for incoming meta field
*/
int sendMetaDataW (TMP *mp, TMPCMD *cmd)
{
	if (!cmd->data3) return 0;
	extendedFileInfoStructW ext;
	cmd->command = CMD_GET_METADATAW;
	
	wchar_t *meta = calloc(cmd->data3, sizeof(char));
	if (meta == NULL) return 0;
	
	clearSendBuffer(mp);
	if (readSocket(&mp->net, meta, cmd->data3) != SOCKET_ERROR){
		ext.filename = (wchar_t *)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTFILEW);
		ext.metadata = meta;
		ext.ret = mp->buffer.WChar;
		ext.retlen = SENDBUFFERSIZE-sizeof(wchar_t);
		SendMessage(mp->hwndParent, WM_WA_IPC, (DWORD)&ext, IPC_GET_EXTENDED_FILE_INFOW);

		size_t dataSize = 0;
		if (!wcslen(ext.ret)){
			size_t dataSize = (wcslen(L"  ") + sizeof(wchar_t)) * sizeof(wchar_t);
			if (sendString(mp, cmd, L" ", dataSize)){
				free(meta);
				return 1;
			}
		}else{
			dataSize = (wcslen(ext.ret) + sizeof(wchar_t)) * sizeof(wchar_t);
			if (sendString(mp, cmd, ext.ret, dataSize)){
				free(meta);
				return 1;
			}
		}
	}
	free(meta);
	return 0;
}

/*
	send EQ bands to client
	first band may also be the last band

	in:
	cmd->data1 = fist band in set (1 to 11)
	cmd->data2 = last band in set (1 to 11)
	
	out:
	eq.first: actual fist band in set, 1 to 11 or from data1 to data2
	eq.last: actual fist band in set, 1 to 11 or from data1 to data2
	
	if client requests band 23 but the maximum the media player supports is 15 then band 
	23 is replaced with 15, eq.last should indicate 15 as max
*/
int sendEQData (TMP *mp, TMPCMD *cmd)
{
	TMPCMDEQ eq;
	int i;
	for (i = cmd->data1-1; i < cmd->data2; i++)
		eq.band[i] = (ubyte)SendMessage(mp->hwndParent, WM_WA_IPC, i, IPC_GETEQDATA);

	eq.command = CMD_GET_EQDATA;
	eq.magic = MAGIC;
	eq.first = cmd->data1;
	eq.last = cmd->data2;
	size_t cmdLen = sizeof(TMPCMDEQ);
	if (sendSocket(&mp->net, &eq, &cmdLen) != SOCKET_ERROR)
		return 1;
	else
		return 0;
}

/*
	set EQ data from client
	data1 = fist band to set (1 to 11)
	data2 = last band to set (1 to 11)
	refer to winamp sdk wa_icp.h
*/
int setEQData (TMP *mp, TMPCMD *cmd)
{
	int i;
	TMPCMDEQ eq;
	size_t cmdLen = sizeof(TMPCMDEQ);
	unsigned int value;
	if (readSocket(&mp->net, &eq, cmdLen) != SOCKET_ERROR){
		if (eq.magic == MAGIC){
			for (i = cmd->data1-1; i < cmd->data2 && i < 16; i++){
				//PostMessage(mp->hwndParent, WM_WA_IPC, i, IPC_GETEQDATA);
				//value = (unsigned short)eq.band[i];				
				value = 0xDB<<24|i<<16|(unsigned short)eq.band[i];
				PostMessage(mp->hwndParent, WM_WA_IPC, value, IPC_SETEQDATA);
			}
			return 1;
		}
	}
	return 0;
}

/*
	each track is seperated by \0
	buffer is terminated with \0
	including NULL's from last track, last 2 bytes in buffer should = \0\0
	data1 = total tracks in buffer
	data2 = length of buffer
*/
int sendCompletePlaylistTitleA (TMP *mp, TMPCMD *cmd)
{
	unsigned int i;
	int total = 0;
	ubyte *item = NULL;
	size_t itemLen = 0;
	size_t totalLen = 0;
	ubyte *playlist = (ubyte *)calloc(8, sizeof(ubyte));
	if (playlist == NULL) return 0;
	int totalTracks = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);

	for(i = 0; i < totalTracks; i++){
		item = (ubyte*)SendMessage(mp->hwndParent, WM_WA_IPC, i, IPC_GETPLAYLISTTITLE);
		if (item){
			itemLen = (size_t)strlen(item);
			if (itemLen){
				playlist = (ubyte*)realloc(playlist, totalLen+8);
				if (playlist){
					memcpy(&playlist[totalLen], item, totalLen+itemLen);
					totalLen += itemLen;
					playlist[totalLen++] = 0;
					total++;
				}else{
					break;
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}

	playlist[totalLen++] = 0;
	int ret = 0;
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = total;
	cmd->data2 = totalLen;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR){
		if (sendSocket(&mp->net, playlist, &totalLen) != SOCKET_ERROR)
			ret = 1;
	}
	if (playlist)
		free(playlist);
	return ret;
}

/*
	each track is seperated by \0
	buffer is also terminated with \0
	including NULL's from last track, last 4 bytes in buffer should = \0\0
	data1 = total tracks in buffer
	data2 = length of buffer
*/
int sendCompletePlaylistFilenameA (TMP *mp, TMPCMD *cmd)
{
	unsigned int i;
	int total = 0;
	ubyte *item = NULL;
	size_t itemLen = 0;
	size_t totalLen = 0;
	ubyte *playlist = (ubyte *)calloc(8, sizeof(ubyte));
	if (playlist == NULL) return 0;
	int totalTracks = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);

	for(i = 0; i < totalTracks; i++){
		item = (ubyte*)SendMessage(mp->hwndParent, WM_WA_IPC, i, IPC_GETPLAYLISTFILE);
		if (item){
			itemLen = (size_t)strlen(item);
			if (itemLen){
				playlist = (ubyte*)realloc(playlist, totalLen+8);
				if (playlist){
					memcpy(&playlist[totalLen], item, totalLen+itemLen);
					totalLen += itemLen;
					playlist[totalLen++] = 0;
					total++;
				}else{
					break;
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}

	playlist[totalLen++] = 0;
	int ret = 0;
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = total;
	cmd->data2 = totalLen;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR){
		if (sendSocket(&mp->net, playlist, &totalLen) != SOCKET_ERROR)
			ret = 1;
	}
	if (playlist)
		free(playlist);
	return ret;
}

/*
	each track is seperated by \0\0
	buffer is also terminated with \0\0
	including NULL from last track, last 4 bytes in buffer should = \0\0\0\0
	data1 = total tracks in buffer
	data2 = length of buffer including NULL's
*/
int sendCompletePlaylistFilenameW (TMP *mp, TMPCMD *cmd)
{
	unsigned int i;
	int total = 0;
	wchar_t *item = NULL;
	size_t itemLen = 0;
	size_t totalLen = 0;
	ubyte *playlist = (ubyte *)calloc(8, sizeof(ubyte));
	if (playlist == NULL) return 0;
	int totalTracks = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);

	for(i = 0; i < totalTracks; i++){
		item = (wchar_t*)SendMessage(mp->hwndParent, WM_WA_IPC, i, IPC_GETPLAYLISTFILEW);
		if (item){
			itemLen = (size_t)wcslen(item)*sizeof(wchar_t);
			if (itemLen){
				playlist = (ubyte*)realloc(playlist, totalLen+8);
				if (playlist){
					memcpy(&playlist[totalLen], item, totalLen+itemLen);
					totalLen += itemLen;
					playlist[totalLen++] = 0;
					playlist[totalLen++] = 0;
					total++;
				}else{
					break;
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}
	playlist[totalLen++] = 0;
	playlist[totalLen++] = 0;
	int ret = 0;
	
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = total;
	cmd->data2 = totalLen;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR){
		if (sendSocket(&mp->net, playlist, &totalLen) != SOCKET_ERROR)
			ret = 1;
	}
	if (playlist)
		free(playlist);
	return ret;
}

/*
	each track is seperated by \0\0
	buffer is also terminated with \0\0
	including NUL's from last track, last 4 bytes in buffer should = \0\0\0\0
	data1 = total tracks in buffer
	data2 = length of buffer including NUL's
*/
int sendCompletePlaylistTitleW  (TMP *mp, TMPCMD *cmd)
{
	unsigned int i;
	int total = 0;
	wchar_t *item = NULL;
	size_t itemLen = 0;
	size_t totalLen = 0;
	ubyte *playlist = (ubyte *)calloc(8, sizeof(ubyte));
	if (playlist == NULL) return 0;
	int totalTracks = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	
	for(i = 0; i < totalTracks; i++){
		item = (wchar_t*)SendMessage(mp->hwndParent, WM_WA_IPC, i, IPC_GETPLAYLISTTITLEW);
		if (item){
			itemLen = (size_t)wcslen(item)*sizeof(wchar_t);
			if (itemLen){
				playlist = (ubyte*)realloc(playlist, totalLen+8);
				if (playlist){
					memcpy(&playlist[totalLen], item, totalLen+itemLen);
					totalLen += itemLen;
					playlist[totalLen++] = 0;
					playlist[totalLen++] = 0;
					total++;
				}else{
					break;
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}
	playlist[totalLen++] = 0;
	playlist[totalLen++] = 0;
	int ret = 0;
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = total;
	cmd->data2 = totalLen;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR){
		if (sendSocket(&mp->net, playlist, &totalLen) != SOCKET_ERROR)
			ret = 1;
	}
	if (playlist)
		free(playlist);
	return ret;
}

/*
	data1 = track index
	data2 = filename length (bytes)
*/
int sendTrackFilenameW (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKFILENAMEW;
	
	size_t cmdLen = sizeof(TMPCMD);
	wchar_t *titlesrc = (wchar_t *)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTFILEW);
	if (titlesrc){
		size_t itemLen = wcslen(titlesrc);
		if (itemLen){
			wchar_t *title = (wchar_t*)calloc(itemLen+1, sizeof(wchar_t));
			if (title){
				wcsncpy(title, titlesrc, itemLen);
				title[itemLen++] = 0;
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

/*
	data1 = track index
	data2 = filename length (bytes)
*/
int sendTrackFilenameA (TMP *mp, TMPCMD *cmd)
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
				itemLen++;
				if (sendString(mp, cmd, title, itemLen*sizeof(char))){
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


/*
	data1 = 
	data2 = 
	data3 = title length (bytes)
*/
int sendTrackTitleW (TMP *mp, TMPCMD *cmd)
{
	cmd->command = CMD_GET_TRACKTITLEW;
	
	size_t cmdLen = sizeof(TMPCMD);
	wchar_t *titlesrc = (wchar_t *)SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data2, IPC_GETPLAYLISTTITLEW);
	
	//wprintf(L"Wide, %i :%s:", cmd->data2, titlesrc);
	//printf("\n");
	
	
	if (titlesrc){
		size_t itemLen = wcslen(titlesrc);
		if (itemLen){
			wchar_t *title = (wchar_t*)calloc(itemLen+1, sizeof(wchar_t));
			if (title){
				wcsncpy(title, titlesrc, itemLen);
				title[itemLen++] = 0;
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

/*
	data1 =
	data2 = 
	data3 = title length (bytes)
*/
int sendTrackTitleA (TMP *mp, TMPCMD *cmd)
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
				itemLen++;
				if (sendString(mp, cmd, title, itemLen*sizeof(char))){
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

/*
	data2 = size of name in bytes
*/
int sendPluginName (TMP *mp, TMPCMD *cmd)
{
	return sendString(mp, cmd, MY_NAME, sizeof(MY_NAME));
}

/*
	data1/2/3 = version
*/
int sendPluginVersion (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = MY_VERSION1;
	cmd->data2 = MY_VERSION2;
	cmd->data3 = MY_VERSION3;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

/*
	data2 = name length
*/
int sendMPName (TMP *mp, TMPCMD *cmd)
{
	return sendString(mp, cmd, MP_NAME,  sizeof(MP_NAME));
}

/*
	data1 = version
*/
int sendMPVersion (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

/*
	data1 = current track position time
*/
int sendCurrentTrackPosition (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = length of current[ly playing] track
*/
int sendCurrentTrackLength (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = bite rate of current track
*/
int sendCurrentTrackBitrate (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 1, IPC_GETINFO);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = audio channels of current track
*/
int sendCurrentTrackChannels (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 2, IPC_GETINFO);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = sample rate of current track
*/
int sendCurrentTrackSampleRate (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETINFO);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}


/*
	data1 = playlist position
*/
int sendPlaylistPosition (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = number of tracks in playlist
*/
int sendPlaylistTotal (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = track position to set (ms)
*/
int setTrackPosition (TMP *mp, TMPCMD *cmd)
{
	PostMessage(mp->hwndParent, WM_WA_IPC, cmd->data1, IPC_JUMPTOTIME);
	return 1;
}

/*
	data1 = track time to skip forward by (ms)
*/
int mediaCtrlFastForward (TMP *mp, TMPCMD *cmd)
{
	int pos = SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
	PostMessage(mp->hwndParent,WM_WA_IPC,pos+cmd->data1,IPC_JUMPTOTIME);
	return 1;
}

/*
	data1 = track time to skip back by (ms)
*/
int mediaCtrlRewind (TMP *mp, TMPCMD *cmd)
{
	int pos = SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
	PostMessage(mp->hwndParent,WM_WA_IPC,pos-cmd->data1,IPC_JUMPTOTIME);
	return 1;
}

/*
	data1 = volume level to set
*/
int setVolume (TMP *mp, TMPCMD *cmd)
{
	PostMessage(mp->hwndParent, WM_WA_IPC, cmd->data1, IPC_SETVOLUME);	
	return 1;
}

/*
	data1 = media player volume level
*/
int sendVolume (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)SendMessage(mp->hwndParent, WM_WA_IPC, -666, IPC_SETVOLUME);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 = window handle of media player, if available
*/
int sendMPHandle (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (unsigned int)mp->hwndParent;
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	control = type of contents
	data = spectrum and/or wave data
*/
int sendSpectrumData (TMP *mp, TMPCMD *cmd)
{
	TMPCMDWAVE wave;
	wave.command = cmd->command;
	wave.control = cmd->data1;
	wave.len = 0;
	wave.channels = 0;
	wave.bpc = 0;
	wave.magic = MAGIC;

	if (mp->mod != NULL){
		wave.sRate = mp->mod->sRate;
		wave.bpc = mp->bpc;	
		
		if (WaitForSingleObject(mp->wavedataLock, 500) == WAIT_OBJECT_0){
			mp->control = cmd->data1;
			if (cmd->data1&0x01 && mp->mod != NULL){
				wave.channels = mp->mod->nCh;
				wave.len = wave.bpc*mp->mod->nCh;
				memcpy(&wave.data[0], mp->spectrum, wave.len);
			}
			if (cmd->data1&0x02 && mp->mod != NULL){
				wave.channels += mp->mod->nCh;
				memcpy(&wave.data[wave.len], mp->wave, wave.len);
				wave.len += wave.len;
			}
			ReleaseSemaphore(mp->wavedataLock, 1, NULL);
		}
	}

	size_t cmdLen = sizeof(TMPCMDWAVE) - (sizeof(wave.data)-wave.len);
	if (sendSocket(&mp->net, &wave, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

/*
	data1 = current play state. 1 = playing, 0 = stopped, 3 = paused.
*/
int sendPlayState (TMP *mp, TMPCMD *cmd)
{
	size_t cmdLen = sizeof(TMPCMD);
	cmd->data1 = (int)SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;	
}

/*
	data1 should contain a playlist index of track to start
*/
int playTrackIndex (TMP *mp, TMPCMD *cmd)
{
	SendMessage(mp->hwndParent, WM_WA_IPC, cmd->data1, IPC_SETPLAYLISTPOS);
	if (SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 3)	// if is paused then unpause
		SendMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2,0),0);
	PostMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2,0),0); // send play command
	return 1;
}

/*
	play button has been pressed so do something
*/
int mediaCtrlPlay (TMP *mp, TMPCMD *cmd)
{
	if (1 != SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING))
		PostMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2,0),0); // is not playing so start
	else
		PostMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON3,0),0); // is playing so pause
	return 1;
}

/*
	stop button was pressed, stop the track
*/
int mediaCtrlStop (TMP *mp, TMPCMD *cmd)
{

	PostMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON4,0),0);
	return 1;
}

/*
	Next button was pressed, skip to next track
*/
int mediaCtrlNext (TMP *mp, TMPCMD *cmd)
{
	PostMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON5,0),0);
	return 1;
}

/*
	Previous button was pressed, go back to last track
*/
int mediaCtrlPrevious (TMP *mp, TMPCMD *cmd)
{
	PostMessage(mp->hwndParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON1,0),0);
	return 1;
}


int hack_SendMessage(TMP *mp, int a, int b, int c)
{
	if (!mp->Shuttingdown){
		if (WaitForSingleObject(mp->quitEvent, 0) == WAIT_OBJECT_0){
			//printf("hack_SendMessage() wait_object_0\n");
			mp->Shuttingdown = 1;
		}
	}

	if (!mp->Shuttingdown)
		return SendMessage(mp->hwndParent, a, b, c);
	else
		return 0;
}

/*
	only send a TMPGENTRACKINFO struct reply
*/
int sendGeneralTrackInfo (TMP *mp, TMPCMD *cmd)
{
	NET_SEND_LOCK();

	TMPGENTRACKINFO gen;
	gen.command = cmd->command;
	gen.magic = cmd->magic;
	gen.length = (unsigned int)hack_SendMessage(mp, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);
	gen.playlistpos = (int)hack_SendMessage(mp, WM_WA_IPC, 0, IPC_GETLISTPOS);
	gen.position = (unsigned int)hack_SendMessage(mp, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
	gen.bitrate = (short)hack_SendMessage(mp, WM_WA_IPC, 1, IPC_GETINFO);
	gen.channels = (ubyte)hack_SendMessage(mp, WM_WA_IPC, 2, IPC_GETINFO);
	gen.samplerate = (ubyte)hack_SendMessage(mp, WM_WA_IPC, 0, IPC_GETINFO);
	mp->totalTracks = gen.totaltracks = (int)hack_SendMessage(mp, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	size_t genLen = sizeof(TMPGENTRACKINFO);
	if (sendSocket(&mp->net, &gen, &genLen) != SOCKET_ERROR){
		NET_SEND_UNLOCK();
		return 1;
	}else{
		NET_SEND_UNLOCK();
		return 0;
	}
}

int mpHostIPC (TMP *mp, TMPCMD *cmd)
{
	return 1;
}

int mpshutdown (TMP *mp, TMPCMD *cmd)
{
	return 0;
}

void closeSockets (TMP *mp)
{
#if (IPPROTOCOL > 0)
	if (mp->net.socket.client != SOCKET_ERROR){
		closeSocket(mp->net.socket.client);
		mp->net.socket.client = SOCKET_ERROR;
	}
#endif
	if (mp->net.socket.server != SOCKET_ERROR){
		closeSocket(mp->net.socket.server);
		mp->net.socket.server = SOCKET_ERROR;
	}
}

void serverShutdown (TMP *mp)
{
	mp->net.serverState = 0;
	mp->net.clientState = 0;
	mp->Shuttingdown = 1;
	
	if (WaitForSingleObject(mp->sendLock, 60000) == WAIT_OBJECT_0){
		mp->net.serverState = 0;
		ReleaseSemaphore(mp->sendLock, 1, NULL);
	}

	int ret = CMD_HOSTEXIT;
	int i = 2;
	while(i--){
		size_t btotal = 1;
		sendSocket(&mp->net, &ret, &btotal);
	}
	Sleep(50);
	closeSockets(mp);
	mp->net.serverState = 0;
	WaitForSingleObject(thandle, 60000);
	CloseHandle(thandle);
	thandle = NULL;
}

SOCKET newSocket (TMP *mp)
{
#if (IPPROTOCOL > 0)
	mp->net.socket.server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	mp->net.socket.server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
	
	if (mp->net.socket.server == INVALID_SOCKET){
		MessageBox(mp->hwndParent, "socket() returned an INVALID_SOCKET\nvis_wamp.dll failed to start\n\n"MP_NAME" must be restarted","WVS Host: gen_wamp.dll", MB_OK);
		return INVALID_SOCKET;
	}	

	if (bindPort(&mp->net, TCPPORT) == SOCKET_ERROR){
		MessageBox(mp->hwndParent, "gen_wamp.dll was unable to bind port "TCPPORTSTR" as it's already in use by another application.\n"
		  "Please close any application using port "TCPPORTSTR" then restart "MP_NAME".","WVS Host: gen_wamp.dll", MB_OK);
		closeSockets(mp);
		return INVALID_SOCKET;
	}

#if (IPPROTOCOL > 0)
	if (listen(mp->net.socket.server, 128) != 0){
		MessageBox(mp->hwndParent, "Failied to put socket in to listen mode\n\n"MP_NAME" must be restarted","WVS Host: gen_wamp.dll", MB_OK);
		closeSockets(mp);
		return INVALID_SOCKET;
	}
#endif
	return mp->net.socket.server;
}


void emptyReadBuffer (TMPNET *net)
{
#if (IPPROTOCOL > 0)
	return;
#endif

	//printf("emptying read buffer... ");
	unsigned long pendingdata = 0;
	int data;
	int ret = 1;
	
	do{
		pendingdata = 0;
		ret = ioctlsocket(net->socket.server, FIONREAD, &pendingdata);
		if (pendingdata){
			if (readSocket(net, &data, sizeof(data)) == -1){
				//printf("connection error\n");
				return;
			}
		}
	}while(pendingdata && !ret);
	//printf("completed\n");
	return;
}

int __stdcall UDPConnectionHandler (TMP *mp)
{
	Sleep(30);		// let parent thread exit function before starting
	size_t btotal;
	FD_SET Reader;	
	mp->net.clientState = 0;
	//TMPCMD cmd;

	initSocket();
	if (newSocket(mp) == INVALID_SOCKET){
		mp->net.serverState = 0;
		return 0;
	}

	emptyReadBuffer(&mp->net);
	FD_ZERO(&Reader);
	FD_SET(mp->net.socket.server, &Reader);
		
	do{
		if (select(0, &Reader, NULL, NULL, NULL) == SOCKET_ERROR){
			mp->net.serverState = 0;
			break;
		}
		if (!mp->net.serverState)
			break;

		if (WaitForSingleObject(mp->sendLock, 100) == WAIT_OBJECT_0){
			if (mp->net.serverState){
				memset(&mp->cmd, 0, sizeof(TMPCMD));
				btotal = readSocket(&mp->net, &mp->cmd, sizeof(TMPCMD));
				if (mp->cmd.magic == MAGIC && btotal == sizeof(TMPCMD)){
					//NET_SEND_LOCK();
					//NET_READ_LOCK();
					//memcpy(&mp->cmd, &cmd, sizeof(TMPCMD));
					handleClientCommand(mp, &mp->cmd);
					//NET_READ_UNLOCK();
					//NET_SEND_UNLOCK();
					mp->net.clientState = 1;
				}//else
					//emptyReadBuffer(&mp->net);
			}
			ReleaseSemaphore(mp->sendLock, 1, NULL);
		}
	}while(mp->net.serverState);
	
	// ensure client knows we've shutdown
	int ret = CMD_HOSTEXIT;
	int i = 6;
	while(i--){
		btotal = 1;
		sendSocket(&mp->net, &ret, &btotal);
	}

	mp->net.serverState = 0;
	mp->net.clientState = 0;
	mp->Shuttingdown = 2;
	closeSockets(mp);
	_endthreadex(1);
	return 1;
}


#if (IPPROTOCOL > 0)
void __cdecl TCPConnectionHandler (TMP *mp)
{
	Sleep(30);
	size_t btotal;
	mp->net.clientState = 0;
	
	initSocket();
	if (newSocket(mp) == INVALID_SOCKET){
		mp->net.serverState = 0;
		return;
	}

	do{
		mp->net.socket.client = waitForConnection(mp->net.socket.server, &mp->net.sockaddrin);
		if (mp->net.socket.client == INVALID_SOCKET){
			closeSocket(mp->net.socket.server);
			Sleep(100);
			if (newSocket(mp) == INVALID_SOCKET){
				mp->net.clientState = 0;
				mp->net.serverState = 0;
			}
		}else{
			mp->net.clientState = 1;
		}

		while(mp->net.clientState){
			memset(&mp->cmd, 0, sizeof(TMPCMD));
			if (WaitForSingleObject(mp->sendLock, 500) == WAIT_OBJECT_0){
				btotal = readSocket(&mp->net, &mp->cmd, sizeof(TMPCMD));
				if ((mp->cmd.magic == MAGIC) && (btotal == sizeof(TMPCMD)))
					handleClientCommand(mp, &mp->cmd);
				ReleaseSemaphore(mp->sendLock, 1, NULL);
			}
		};

		closeSocket(mp->net.socket.client);
		mp->net.socket.client = SOCKET_ERROR;
	}while(mp->net.serverState);

	mp->net.serverState = 0;
	mp->net.clientState = 0;
	closeSockets(mp);
}
#endif

static int handleClientCommand (TMP *mp, TMPCMD *cmd)
{
	//if (cmd->command != 40)
		//printf("command:%i, data1:%i data2:%i\n", (int)cmd->command, (int)cmd->data1, (int)cmd->data2);
		
	if (cmd->command < 1 || cmd->command > CMD_TOTAL){
		return 0;
	}else{
		return command[cmd->command].address(mp, cmd);
	}
}

int serverStart (TMP *mp)
{

	command[CMD_GET_PLAYLISTTITLEA].address = sendCompletePlaylistTitleA;
	command[CMD_GET_PLAYLISTFILENAMEA].address = sendCompletePlaylistFilenameA;
	command[CMD_GET_METADATAA].address = sendMetaDataA;
	command[CMD_GET_TRACKTITLEA].address = sendTrackTitleA;
	command[CMD_GET_TRACKFILENAMEA].address = sendTrackFilenameA;
	command[CMD_ENQUEUEFILEA].address = enqueueFileA;

	// MM returns version 0x99DB
	// Unicode(UTF16) was added to Winamp in v5.30
	// Aimp returns 0x2099. AIMP does not support the Winamp WideChar(UTF16) API
	// Just to spice things up XMPlay returns UTF8 everywhere, returns (or does not return at all..) Version 0x0000
	if (SendMessage(mp->hwndParent, WM_WA_IPC, 0, IPC_GETVERSION) >= 0x5030){
		command[CMD_GET_PLAYLISTTITLEW].address = sendCompletePlaylistTitleW;
		command[CMD_GET_PLAYLISTFILENAMEW].address = sendCompletePlaylistFilenameW;
		command[CMD_ENQUEUEFILEW].address = enqueueFileW;
		command[CMD_GET_METADATAW].address = sendMetaDataW;
		
#if (BUILD_MEDIAMONKEY)
		command[CMD_GET_TRACKTITLEW].address = MM_sendTrackTitleW;
		command[CMD_GET_TRACKFILENAMEW].address = MM_sendTrackFilenameW;
		//command[CMD_GET_METADATAW].address = MM_sendMetaDataW;
#else
		command[CMD_GET_TRACKTITLEW].address = sendTrackTitleW;
		command[CMD_GET_TRACKFILENAMEW].address = sendTrackFilenameW;
		//command[CMD_GET_METADATAW].address = sendMetaDataW;
#endif
	}else{
		command[CMD_GET_PLAYLISTTITLEW].address = sendCompletePlaylistTitleA;
		command[CMD_GET_PLAYLISTFILENAMEW].address = sendCompletePlaylistFilenameA;
		command[CMD_GET_METADATAW].address = sendMetaDataA;
		command[CMD_GET_TRACKTITLEW].address = sendTrackTitleA;
		command[CMD_GET_TRACKFILENAMEW].address = sendTrackFilenameA;
		command[CMD_ENQUEUEFILEW].address = enqueueFileA;
	}

	command[CMD_GET_EQDATA].address = sendEQData;
	command[CMD_SET_EQDATA].address = setEQData;
	command[CMD_GET_CURRENTTRKINFO].address = sendGeneralTrackInfo;
	command[CMD_GET_POSITION].address = sendCurrentTrackPosition;
	command[CMD_GET_TRACKLENGTH].address = sendCurrentTrackLength;
	command[CMD_GET_BITRATE].address = sendCurrentTrackBitrate;
	command[CMD_GET_CHANNELS].address = sendCurrentTrackChannels;
	command[CMD_GET_SAMPLERATE].address = sendCurrentTrackSampleRate;
	command[CMD_SET_POSITION].address = setTrackPosition;
	command[CMD_GET_PLAYLISTPOSITION].address = sendPlaylistPosition;
	command[CMD_GET_PLAYLISTTOTAL].address = sendPlaylistTotal;
	command[CMD_PLAYTRACKINDEX].address = playTrackIndex;
	command[CMD_GET_MPHANDLE].address = sendMPHandle;
	command[CMD_GET_MPVOLUME].address = sendVolume;
	command[CMD_SET_MPVOLUME].address = setVolume;
	command[CMD_GET_PLAYSTATUS].address = sendPlayState;
	command[CMD_MCTL_FORWARD].address = mediaCtrlFastForward;
	command[CMD_MCTL_REWIND].address = mediaCtrlRewind;
	command[CMD_MCTL_PLAY].address = mediaCtrlPlay;
	command[CMD_MCTL_STOP].address = mediaCtrlStop;
	command[CMD_MCTL_NEXT].address = mediaCtrlNext;
	command[CMD_MCTL_PREVIOUS].address = mediaCtrlPrevious;
	command[CMD_GET_PLNAME].address = sendPluginName;
	command[CMD_GET_PLVERSION].address = sendPluginVersion;
	command[CMD_GET_MPNAME].address = sendMPName;
	command[CMD_GET_MPVERSION].address = sendMPVersion;
	command[CMD_SHUTDOWN].address = mpshutdown;
	command[CMD_HOSTIPC].address = mpHostIPC;


	// XMPlay as of version 3.4.2 returns UTF8 instead of ANSI and UTF16
	// its internal ANSI to WideChar encoding is slightly broken
#if (BUILD_XMPLAY)
	command[CMD_GET_TRACKTITLEW].address = xmp_sendTrackTitleW;
	command[CMD_GET_TRACKTITLEA].address = xmp_sendTrackTitleA;
	command[CMD_GET_TRACKFILENAMEW].address = xmp_sendTrackFilenameW;
	command[CMD_GET_TRACKFILENAMEA].address = xmp_sendTrackFilenameA;
	command[CMD_MCTL_FORWARD].address = xmp_mediaCtrlFastForward;
	command[CMD_MCTL_REWIND].address = xmp_mediaCtrlRewind;
	command[CMD_GET_MPVERSION].address = xmp_sendMPVersion;
	command[CMD_GET_CURRENTTRKINFO].address = xmp_sendGeneralTrackInfo;

#endif
	
#if (BUILD_SVP)

	command[CMD_GET_TRACKTITLEW].address = amp_sendTrackTitleW;
	command[CMD_GET_TRACKFILENAMEW].address = amp_sendTrackFilenameW;
	command[CMD_GET_METADATAW].address = sendMetaDataA;
#endif
	
#if (BUILD_MEDIAMONKEY)
	command[CMD_GET_SPECTRUMDATA].address = MM_sendSpectrumData;

	command[CMD_GET_TOTALPLAYLISTS].address = MM_sendTotalPlaylists;
	command[CMD_GET_PLAYLISTNAME].address = MM_sendPlaylistName;
	command[CMD_GET_PLAYLISTTRKTOTAL].address = MM_sendPlaylistTrackTotal;
	command[CMD_GET_PLAYLISTTRKTITLE].address = MM_sendPlaylistTrackTitle;
	command[CMD_GET_PLAYLISTTRKARTIST].address = MM_sendPlaylistTrackArtist;
	command[CMD_GET_PLAYLISTTRKALBUM].address = MM_sendPlaylistTrackAlbum;
	command[CMD_GET_PLAYLISTTRKPATH].address = MM_sendPlaylistTrackPath;
#else
	command[CMD_GET_SPECTRUMDATA].address = sendSpectrumData;
	
	command[CMD_GET_TOTALPLAYLISTS].address = sendTotalPlaylists;
	command[CMD_GET_PLAYLISTNAME].address = sendPlaylistName;
	command[CMD_GET_PLAYLISTTRKTOTAL].address = sendPlaylistTrackTotal;
	command[CMD_GET_PLAYLISTTRKTITLE].address = sendPlaylistTrackTitle;
	command[CMD_GET_PLAYLISTTRKARTIST].address = sendPlaylistTrackArtist;
	command[CMD_GET_PLAYLISTTRKALBUM].address = sendPlaylistTrackAlbum;
	command[CMD_GET_PLAYLISTTRKPATH].address = sendPlaylistTrackPath;
#endif
	

	mp->control = 0x01;		// set default spectrum/wave data control flag
	mp->net.serverState = 1;
	mp->Shuttingdown = 0;
	
	unsigned int tid = 0;
	
#if (IPPROTOCOL > 0)
	thandle = (HANDLE)_beginthreadex(NULL, 0, (void*)TCPConnectionHandler, (void*)mp, 0, &tid);
#else
	thandle = (HANDLE)_beginthreadex(NULL, 0, (void*)UDPConnectionHandler, (void*)mp, 0, &tid);
	//_beginthread((void*)UDPConnectionHandler, 0, (void*)mp);
#endif
	return 1;
}
