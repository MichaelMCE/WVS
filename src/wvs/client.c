
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


//#include <winsock2.h>
#include <stdio.h>
#include <unistd.h>

#include "winamp.h"
#include "net.h"
#include "config.h"
#include "client.h"
#include "net.c"

#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))

/* helper functions */

// no returned string can be more than 8190 bytes - 8192 including terminator
static int readStringW (TMPNET *net, TMPCMD *cmd, wchar_t *buffer, size_t buffersize)
{
	cmd->magic = 0;
	cmd->data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	memset(buffer, 0, buffersize);

	if (readSocket(net, cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading string reply\n");
		return 0;
	}
	if (cmd->data3 && (cmd->magic == MAGIC)){
		cmd->data3 = MIN(cmd->data3, buffersize);
		if (readSocket(net, buffer, cmd->data3) == SOCKET_ERROR){
			//printf("error reading readStringW, magic = 0x%X\n", cmd->magic);
			return 0;
		}else{
			return wcslen(buffer);
		}
	}
	return 0;
}

/* helper functions end*/


/*****************************/
/*   Multiple Playlist API   */

EXPORT int netGetPlaylistName (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYLISTNAME;
	cmd.data1 = playlist;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	int ret = readStringW(net, &cmd, buffer, buffersize);
	if (ret){
		//wprintf(L"##%s##", buffer);
		//printf("\n");
		return ret;
	}
	return 0;
	
}

EXPORT int netGetPlaylistTrackTitle (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYLISTTRKTITLE;
	cmd.data1 = playlist;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	int ret = readStringW(net, &cmd, buffer, buffersize);
	if (ret){
		//wprintf(L"##%s##", buffer);
		//printf("\n");
		return ret;
	}
	return 0;
}

EXPORT int netGetPlaylistTrackPath (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYLISTTRKPATH;
	cmd.data1 = playlist;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	int ret = readStringW(net, &cmd, buffer, buffersize);
	if (ret){
		//wprintf(L"##%s##", buffer);
		//printf("\n");
		return ret;
	}
	return 0;
}

EXPORT int netGetPlaylistTrackAlbum (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYLISTTRKALBUM;
	cmd.data1 = playlist;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	int ret = readStringW(net, &cmd, buffer, buffersize);
	if (ret){
		//wprintf(L"##%s##", buffer);
		//printf("\n");
		return ret;
	}
	return 0;
}

EXPORT int netGetPlaylistTrackArtist (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYLISTTRKARTIST;
	cmd.data1 = playlist;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	int ret = readStringW(net, &cmd, buffer, buffersize);
	if (ret){
		//wprintf(L"##%s##", buffer);
		//printf("\n");
		return ret;
	}
	return 0;
}

EXPORT int netGetTotalPlaylists (TMPNET *net, int *totalplaylist)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_TOTALPLAYLISTS;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (totalplaylist) *totalplaylist = 0;
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_TOTALPLAYLISTS reply\n");
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		if (totalplaylist) *totalplaylist = (int)cmd.data1;
		return 1;
	}
	return 0;
#else
	return 1;
#endif
}


EXPORT int netGetPlaylistTrackTotal (TMPNET *net, int playlist, int *totaltracks)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYLISTTRKTOTAL;
	cmd.data1 = playlist;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	
	cmd.magic = 0;
	cmd.data2 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_TOTALPLAYLISTS reply\n");
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		*totaltracks = (int)cmd.data2;
		return 1;
	}
	return 0;
}

/* Multiple Playlist API END */
/*****************************/

EXPORT int netEmptyReadBuffer (TMPNET *net)
{	

#if (IPPROTOCOL > 0)
#error client.c: netEmptyReadBuffer(): compile me with IPPROTOCOL=0
#endif

	//printf("emptying read buffer... ");
	unsigned long pendingdata = 0;
	int data;
	int ret = 1;
	
	Sleep(10);
	do{
		pendingdata = 0;
		ret = ioctlsocket(net->socket.server, FIONREAD, &pendingdata);
		if (pendingdata){
			if (netReadSocket(net, &data, sizeof(data)) == -1){
				//printf("connection error\n");
				return 0;
			}
		}
	}while(pendingdata && !ret);
	//printf("completed\n");
	return 1;
}

EXPORT int netGetMPName (TMPNET *net, void *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_MPNAME;
	cmd.data1 = 0;
	cmd.data2 = 0;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_MPNAME reply\n");
		return 0;
	}
			
	if (cmd.data3){
		char *tmpbuffer = malloc(cmd.data3);
		if (tmpbuffer){
			if (readSocket(net, tmpbuffer, cmd.data3) == SOCKET_ERROR){
				//printf("error reading mpName reply\n");
				free(tmpbuffer);
				return 0;
			}
			if (cmd.magic == MAGIC){
				size_t len = strlen(tmpbuffer) + 1;
				if (len > buffersize){
					free(tmpbuffer);
					return -1;
				}else{
					strncpy(buffer, tmpbuffer, len);
					free(tmpbuffer);
					return len;
				}
			}
			free(tmpbuffer);
		}
	}
	return 0;
#else
	return 1;
#endif
}

EXPORT int netGetMPHandle (TMPNET *net, HANDLE *handle)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_MPHANDLE;
	cmd.data1 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_MPHANDLE reply\n");
		return 0;
	}
	if (cmd.magic == MAGIC){
		*handle = (HANDLE)cmd.data1;
		return 1;
	
	}
	return 0;
#else
	return 1;
#endif
}

EXPORT int netGetTitle (TMPNET *net, int index, int type, void *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = type;
	cmd.data1 = 0;		// playlist
	cmd.data2 = index;	// track in playlist
	cmd.data3 = 0;		// length (returned)
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}

#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data3 = 0;

	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading netGetTitle reply\n");
		return 0;
	}
	if (cmd.data3){
		if (cmd.magic == MAGIC){
			cmd.data3 = MIN(cmd.data3, buffersize);
			if (readSocket(net, buffer, cmd.data3) == SOCKET_ERROR){
				//printf("error reading netGetTitle, magic = 0x%X\n", cmd.magic);
				return 0;
			}else{
				if (type == CMD_GET_TRACKTITLEW){
					return wcslen((wchar_t*)buffer);
				}else{
					return strlen((char*)buffer);
				}
			}
		}else{
			//printf("netGetTitle(): error reading title data, invalid magic (0x%X)\n", cmd.magic);
		}
	}else{
		//printf("netGetTitle(): media player could not retrieve title, magic = 0x%X\n", cmd.magic);
	}

	return 0;
#else
	return 1;
#endif
}

EXPORT int netGetCurrentTrackPosition (TMPNET *net)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_POSITION;
	cmd.data1 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_POSITION reply\n");
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		////printf("time %i\n", cmd.data1);
		return cmd.data1;
	}

	return 0;
#else
	return 1;
#endif
}


EXPORT size_t netIsPacketAvailable (TMPNET *net)
{
	unsigned long pendingdata = 0; 
	if (!ioctlsocket(net->socket.server, FIONREAD, &pendingdata))
		return (size_t)pendingdata;
	else
		return 0;
}


EXPORT int netGetPluginVersion (TMPNET *net, int *v1, int *v2, int *v3)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLVERSION;
	cmd.data1 = 0;
	cmd.data2 = 0;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending getplugin cmd\n");
		return 0;
	}
	
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) != cmdLen){
		//printf("netGetPluginVersion() reply length mismatch\n");
		return 0;
	}

	if (cmd.magic == MAGIC){
		if (v1) *v1 = cmd.data1;
		if (v2) *v2 = cmd.data2;
		if (v3) *v3 = cmd.data3;
		//printf("plugin version = 0x%x %i %i\n", cmd.data1, cmd.data2, cmd.data3);
		return 1;
	}else{
		//printf("bad magic\n");
	}

	return 0;
#else
	return 1;
#endif
}


EXPORT int netPing (TMPNET *net)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLVERSION;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending getplugin cmd\n");
		return 0;
	}
	
	cmd.magic = 0;
	if (readSocket(net, &cmd, cmdLen) != cmdLen)
		return 0;

	if (cmd.magic == MAGIC)
		return 1;
	return 0;
}

EXPORT int netGetMPVersion (TMPNET *net, int *version)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_MPVERSION;
	cmd.data1 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_MPVERSION reply\n");
		//printf("mp version = %x\n", cmd.data1);
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		//printf("mp version = %x\n", cmd.data1);
		if (version) *version = cmd.data1;
		return 1;
	}
	return 0;
#else
	return 1;
#endif
}

EXPORT int netGetPlayState (TMPNET *net, int *state)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_PLAYSTATUS;
	cmd.data1 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_PLAYSTATUS reply\n");
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		if (state) *state = cmd.data1;
		return 1;
	}else{
		return 0;
	}
#else
	return 1;
#endif

}

EXPORT int netGetVolumeEx (TMPNET *net, int *volume, int userData)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_MPVOLUME;
	cmd.data1 = 0;
	cmd.data2 = userData;	// userdata is posted back to you in the CMD_GET_MPVOLUME reply
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_MPVOLUME reply\n");
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		*volume = (int)cmd.data1;
		return 1;
	}
	return 0;
#else
	return 1;
#endif
	
}

EXPORT int netGetVolume (TMPNET *net, int *volume)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_MPVOLUME;
	cmd.data1 = 0;
	cmd.data2 = 0;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data1 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading CMD_GET_MPVOLUME reply\n");
		return 0;
	}
	
	if (cmd.magic == MAGIC){
		*volume = (int)cmd.data1;
		return 1;
	}
	return 0;
#else
	return 1;
#endif
}

EXPORT int netSetVolume (TMPNET *net, unsigned int volume)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_SET_MPVOLUME;
	cmd.data1 = volume;
	cmd.data2 = 0;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netMCtrlStop (TMPNET *net)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_MCTL_STOP;
	cmd.data1 = 0;
	cmd.data2 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netMCtrlNext (TMPNET *net)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_MCTL_NEXT;
	cmd.data1 = 0;
	cmd.data2 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netMCtrlPrevious (TMPNET *net)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_MCTL_PREVIOUS;
	cmd.data1 = 0;
	cmd.data2 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netMCtrlPlay (TMPNET *net)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_MCTL_PLAY;
	cmd.data1 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netPlayTrackIndex (TMPNET *net, unsigned int trackIndex, unsigned int playlistIndex)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_PLAYTRACKINDEX;
	cmd.data1 = trackIndex;
	cmd.data2 = playlistIndex;		// playlist index. >0 is only used with MM
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netTrackRewind (TMPNET *net, unsigned int time)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_MCTL_REWIND;
	cmd.data1 = time;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netTrackForward (TMPNET *net, unsigned int time)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_MCTL_FORWARD;
	cmd.data1 = time;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}else{
		return 1;
	}
}

EXPORT int netGetWaveData (TMPNET *net, int control, TMPCMDWAVE *wave)
{

	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.data1 = control;
	cmd.data2 = 0;
	cmd.data3 = 0;
	cmd.command = CMD_GET_SPECTRUMDATA;
	wave->magic = 0;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending wave data cmd\n");
		return 0;
	}

#if (REQUESTPULLHANDLER)
	size_t dataSize = sizeof(TMPCMDWAVE) - sizeof(wave->data); // header length
	int ret = readSocketPeek(net, wave, dataSize);
	if (ret == 1)	// has has signaled its closing connection
		return 0;
	else if (ret > 1 && ret != dataSize)
		return -1;
	else if (/*ret == -1 && */wave->magic != MAGIC)
		return 0;

	if (wave->len > sizeof(wave->data)/* || wave->magic != MAGIC*/){
		//printf("error reading wave data. data too large or invalid magic: %i > %i, magic = 0x%X, ret = %i\n", wave->len, sizeof(wave->data), wave->magic, ret);
		netEmptyReadBuffer(net);
		return -1;
	}

	dataSize = sizeof(TMPCMDWAVE) - (sizeof(wave->data)-wave->len);
	ret = readSocket(net, wave, dataSize);
	if (ret != dataSize || wave->magic != MAGIC){
		//printf("error reading wave data. length mismatch; expected:%i but received:%i, magic = 0x%X\n", dataSize, ret, wave->magic);
		if (ret == -1){		// host has been terminated without sending shutdown signal
			return 0;
		}else{
			//netEmptyReadBuffer(net);
			return -1;
		}
	}else{
		return 1;
	}
#else
	return -1;
#endif
}

EXPORT int netGetMetaData (TMPNET *net, int metaindex, int metacmd, int playlistindex, void *meta, void *buffer, size_t *buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = metacmd;
	cmd.data1 = metaindex;
	cmd.data2 = playlistindex;	// track index in active playlist
	size_t cmdLen = sizeof(TMPCMD);

	if (metacmd == CMD_GET_METADATAW){
		cmd.data3 = (wcslen((wchar_t*)meta)*sizeof(wchar_t))+sizeof(wchar_t);
		//wprintf(L"getting tag #%s#", meta);
		//printf("\n");
	}else{
		cmd.data3 = (strlen((char*)meta)*sizeof(char))+sizeof(char);
		//printf("getting tag #%s#\n", meta);
	}
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending meta cmd\n");
		return 0;
	}
	size_t dataSize = (size_t)cmd.data3;
	if (sendSocket(net, meta, &dataSize) == SOCKET_ERROR){
		//printf("error sending meta tag\n");
		return 0;
	}

#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data3 = 0;
	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading meta reply\n");
		return 0;
	}

	if (cmd.data3  && cmd.magic == MAGIC){
		cmd.data3 = MIN(cmd.data3, *buffersize);
		if (readSocket(net, buffer, cmd.data3) == SOCKET_ERROR){
			//printf("error reading netGetMetaData, magic = 0x%X\n", cmd.magic);
			return 0;
		}else{
			if (cmd.command == CMD_GET_METADATAW){
				*buffersize = wcslen((wchar_t*)buffer);
			}else{
				*buffersize = strlen((char*)buffer);
			}
			return 1;
		}
	}
	//printf("netGetMetaData(): error reading data, no data or invalid magic (0x%X)\n", cmd.magic);
	return 0;
#else
	return 1;
#endif
}

EXPORT int netEnqueueFile (TMPNET *net, int type, char *path, int play)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = type;
	cmd.data2 = play;

	if (cmd.command == CMD_ENQUEUEFILEW)
		cmd.data1 = (wcslen((wchar_t*)path)*sizeof(wchar_t))+sizeof(wchar_t);
	else
		cmd.data1 = (strlen((char*)path)*sizeof(char))+sizeof(char);

	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	size_t dataSize = cmd.data1;
	if (sendSocket(net, (void*)path, &dataSize) == SOCKET_ERROR){
		//printf("error sending data\n");
		return 0;
	}	
	return 1;
}

EXPORT int netGetFilename (TMPNET *net, int playlistindex, int type, void *buffer, size_t buffersize)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = type;
	cmd.data1 = 0;
	cmd.data2 = playlistindex;
	cmd.data3 = 0;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}

#if (REQUESTPULLHANDLER)
	cmd.magic = 0;
	cmd.data3 = 0;

	if (readSocket(net, &cmd, cmdLen) == SOCKET_ERROR){
		//printf("error reading netGetFilename reply\n");
		return 0;
	}
	if (cmd.data3 && (cmd.magic == MAGIC)){
		cmd.data3 = MIN(cmd.data3, buffersize);
		if (readSocket(net, buffer, cmd.data3) == SOCKET_ERROR){
			//printf("error reading netGetFilename, magic = 0x%X\n", cmd.magic);
			return 0;
		}else{
			if (cmd.command == CMD_GET_TRACKFILENAMEW){
				return wcslen((wchar_t*)buffer);
			}else{
				return strlen((char*)buffer);
			}
		}
	}
	//printf("netGetFilename(): error reading data, no data or invalid magic (0x%X)\n", cmd.magic);
	return 0;
#else
	return 1;
#endif
}

EXPORT int netSetEQData (TMPNET *net, TMPCMDEQ *eq)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.data1 = eq->first;
	cmd.data2 = eq->last;
	cmd.command = CMD_SET_EQDATA;
	eq->command = CMD_SET_EQDATA;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	cmdLen = sizeof(TMPCMDEQ);
	if (sendSocket(net, eq, &cmdLen) == SOCKET_ERROR){
		//printf("error sending eq data\n");
		return 0;
	}
	return 1;
}

EXPORT int netGetEQData (TMPNET *net, TMPCMDEQ *eq)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_EQDATA;
	cmd.data1 = eq->first;
	cmd.data2 = eq->last;
	size_t cmdLen = sizeof(TMPCMD);

	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}

#if (REQUESTPULLHANDLER)
	if (readSocket(net, eq, sizeof(TMPCMDEQ)) == SOCKET_ERROR){
		//printf("error reading eq data reply\n");
		return 0;
	}
	if (eq->magic == MAGIC){
		return 1;
	}else{
		//printf("error reading eq data, invalid magic 0x%X\n",eq->magic);
		return 0;
	}
#else
	return 1;
#endif

}

EXPORT int netGetCurrentTrackInfo (TMPNET *net, TMPGENTRACKINFO *gen)
{
	TMPCMD cmd;
	cmd.magic = MAGIC;
	cmd.command = CMD_GET_CURRENTTRKINFO;
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(net, &cmd, &cmdLen) == SOCKET_ERROR){
		//printf("error sending cmd\n");
		return 0;
	}
	
#if (REQUESTPULLHANDLER)
	gen->magic = 0;
	if (readSocket(net, gen, sizeof(TMPGENTRACKINFO)) != sizeof(TMPGENTRACKINFO)){
		//printf("error reading geninfo reply\n");
		return 0;
	}
	if ((gen->magic == MAGIC) && (gen->command == CMD_GET_CURRENTTRKINFO))
		return 1;
	else
		return 0;
#else
	return 1;
#endif
}


EXPORT int netCloseSocket (SOCKET socket)
{
	return closeSocket(socket);
}

EXPORT int netSendSocket (TMPNET *net, void *buffer, size_t *bsize)
{
	return sendSocket(net, buffer, bsize);
}

EXPORT int netReadSocket (TMPNET *net, void *buffer, size_t bsize)
{
	return readSocket(net, buffer, bsize);
}

EXPORT int netReadSocketPeek (TMPNET *net, void *buffer, size_t bsize)
{
	return readSocketPeek(net, buffer, bsize);
}

EXPORT int netBindPort (TMPNET *net, int port)
{
	return bindPort(net, port);
}

SOCKET netWaitForConnection (SOCKET server, SOCKADDR_IN *sockaddrin)
{
	return waitForConnection (server, sockaddrin);
}

EXPORT void netShutdown ()
{
	shutdownSocket();
}

EXPORT void netInit ()
{
	initSocket();
}

EXPORT SOCKET netConnect (TMPNET *net, char *addr, int port, int proto)
{
	return connectTo (net, addr, port, proto);
}

/*
BOOL WINAPI DllMain (HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	DisableThreadLibraryCalls(hInstDLL);
	return TRUE;
}
*/

