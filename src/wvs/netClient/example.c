
// WVS
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


#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <winsock2.h>

typedef unsigned char ubyte;

#define IPC_PLAYING_FILEW 13003 
#define IPC_PLAYLIST_MODIFIED 3002 
#define IPC_CB_MISC 603
    #define IPC_CB_MISC_TITLE 0
    #define IPC_CB_MISC_VOLUME 1 // volume/pan
    #define IPC_CB_MISC_STATUS 2
    #define IPC_CB_MISC_EQ 3
    #define IPC_CB_MISC_INFO 4
    #define IPC_CB_MISC_VIDEOINFO 5
#define IPC_FILE_TAG_MAY_HAVE_UPDATEDW 3005 

#include "../net.h"
#include "../client.h"


/*
	You can make one or multiple requests without pulling (reading) the data from the previous requests.
	Requested data can return to you out of order.
		
	Or alternatively you can rely on the media player to post events to you.
	Such as track change, title change or modification, playlist modified (files added, tracks moved or deleted), playback started
	or if play status has changed.
	
	netPing() is the only function that blocks.

	Example relies upon gen_wamp.dll/xmp-wvs.dll having already been initiated by Winamp
	gen_wamp.dll//xmp-wvs.dll is not dependant on vis_wamp.dll/vis_wvs.svp
	netGetWaveData()/CMD_GET_SPECTRUMDATA is the only command that requires vis_wamp.dll//vis_wvs.svp

	compile with: gcc example.c libnetclient.a -s
*/



#define METATAGTOTAL 30
ubyte metaTags[METATAGTOTAL][16]={
	"Title",	// index 0
	"Artist",
	"Album",
	"Track",
	"Year",
	"Genre",
	"Comment",
	"Length",
	"Bitrate",
	"SRate",
	"Stereo",
	"VBR",
	"BPM",
	"Gain",
	"AlbumGain",
	"Encoder",
	"AlbumArtist",
	"OriginalArtist",
	"Disc",
	"Remixer",
	"Media",
	"Lyricist",
	"Subtitle",
	"Mood",
	"Key",
	"Composer",
	"Conductor",
	"Publisher",
	"Copyright",
	"URL",
};



//  UDP connection example
int main (int argc, char* argv[])
{
	static int version[3];
	static char buffer[4096];
	

	TMPGENTRACKINFO gen;
	TMPNET mpnet;
	memset(&mpnet, 0, sizeof(TMPNET));
	memset(&gen, 0, sizeof(TMPGENTRACKINFO));

	netInit();
	mpnet.socket.server = netConnect(&mpnet, "127.0.0.1", MY_PORT, IPPROTO_UDP);

	netEmptyReadBuffer(&mpnet); // clean the pipe
	if (netPing(&mpnet)){	// check if host is alive. returns TRUE when pong'd
		netGetPluginVersion(&mpnet, NULL, NULL, NULL);
		netGetMPName(&mpnet, NULL, 0);
		netGetMPVersion(&mpnet, NULL);
		netGetPlayState(&mpnet, NULL);
		netGetTotalPlaylists(&mpnet, NULL);
		netGetCurrentTrackInfo(&mpnet, NULL);
		netGetVolume(&mpnet, NULL);

		Sleep(50);
		processSocketCmds(&mpnet);
		Sleep(50);
		processSocketCmds(&mpnet);
			
		printf("\npackets sent:%i\npackets received:%i\n\n", mpnet.sendCt, mpnet.readCt);

	}else{
		// host is dead
		printf("host not found\n");
	}

	netShutdown();
	return EXIT_SUCCESS;
}

static int isWideStrCmd (int cmd)
{
	return (cmd == CMD_GET_TRACKTITLEW || cmd == CMD_GET_TRACKFILENAMEW || cmd == CMD_GET_METADATAW);
}

static int MIN (int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

static int cmd_readString (TMPNET *net, TMPCMD *cmd, void *buffer, size_t buffersize)
{
	//printf("string: %i %i %i (%i)\n",cmd->data1, cmd->data2, cmd->data3, cmd->command);
	
	if (cmd->data3){
		if (cmd->magic == MAGIC){
			cmd->data3 = MIN(cmd->data3, buffersize);
			if (netReadSocket(net, buffer, cmd->data3) == SOCKET_ERROR){
				printf("error reading cmd_readString, magic = 0x%X\n", cmd->magic);
				return 0;
			}else{
				if (isWideStrCmd(cmd->command)){
					return cmd->data3/sizeof(wchar_t);
				}else{
					return cmd->data3/sizeof(char);
				}
			}
		}else{
			printf("cmd_readString(): error reading buffer, invalid magic (0x%X)\n", cmd->magic);
		}
	}else{
		printf("cmd_readString(): could not retrieve string, magic = 0x%X\n", cmd->magic);
	}

	return 0;
}

int cmd_processHostIPC (TMPNET *net, TMPCMD *cmd)
{
	printf("ipc: %i %i\n", cmd->data1, cmd->data2);

	if (cmd->data1 == IPC_PLAYLIST_MODIFIED){
		// request a title or two
		
	}else if (cmd->data1 == IPC_PLAYING_FILEW){
		//cururent track = cmd->data2;

	}else if (cmd->data1 == IPC_CB_MISC){
		if (cmd->data2 == IPC_CB_MISC_TITLE){
			// title of current track has changed

		}else if (cmd->data2 == IPC_CB_MISC_STATUS){
			netGetPlayState(net, NULL);
		}
	}else if (cmd->data1 == IPC_FILE_TAG_MAY_HAVE_UPDATEDW){
		// do something
	}

	return 1;
}

int cmd_processCmd (TMPNET *net, TMPCMD *cmd)
{
		
	switch(cmd->command){
	  case CMD_GET_METADATAW: {
	  		wchar_t buffer[4096]; 
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				printf("MetaW: playlist position:%i, tag name:\"%s\" = ", cmd->data2, metaTags[cmd->data1]);
				wprintf(L"\"%s\" %i %i %i", buffer, cmd->data1, cmd->data2, cmd->data3);
				printf("\n");
			}
			break;
		}	  
	  case CMD_GET_METADATAA:{
	  		char buffer[4096];
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				printf("MetaA: playlist position:%i, tag name:\"%s\" = ", cmd->data2, metaTags[cmd->data1]);
				printf("\"%s\" %i %i %i\n", buffer, cmd->data1, cmd->data2, cmd->data3);
			}
			break;
		}
	  case CMD_GET_TRACKFILENAMEW:
	  case CMD_GET_TRACKTITLEW: {
	  		wchar_t buffer[4096]; 
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				wprintf(L"TrackTitleW: playlist %i, postion %i: \"%s\" %i", cmd->data1, cmd->data2, buffer, cmd->data3);
				printf("\n");
			}
			break;
		}
	  case CMD_GET_TRACKFILENAMEA:
	  case CMD_GET_TRACKTITLEA: {
	  		char buffer[4096];
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				printf("TrackTitleA: playlist %i, postion %i: \"%s\" %i", cmd->data1, cmd->data2, buffer, cmd->data3);
			}
			break;
		}
	  case CMD_GET_MPVOLUME: 
			printf("Media player volume is: %i (0....255)\n", cmd->data1);
			break;

	  case CMD_GET_MPNAME:{
	  		wchar_t buffer[4096]; 
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				printf("Media player name: \"%s\"\n", buffer, cmd->data1, cmd->data2, cmd->data3);
			}
			break;
		}
	  case CMD_HOSTIPC:
	  		cmd_processHostIPC(net, cmd);
	  		break;
			
	  case CMD_GET_TOTALPLAYLISTS:
			printf("Total playlists: %i\n", cmd->data1);
			break;

	  case CMD_GET_PLAYSTATUS:
	  		printf("Play state: %i\n", cmd->data1);
	  		break;
	  		
	  case CMD_GET_PLVERSION:
	  		printf("Plugin version: 0x%x %i %i\n", cmd->data1, cmd->data2, cmd->data3);
	  		break;

	  case CMD_GET_MPVERSION:
	  		printf("Media player version: 0x%x\n", cmd->data1);
	  		break;	  		

	  default :
			printf("unhandled cmd: %i\n", cmd->command);
	}
	return 0;
}

static ubyte getCmdIndex (TMPNET *net)
{
	ubyte cmd = 0;
	netReadSocketPeek(net, &cmd, sizeof(unsigned char));
	return cmd;
}

static int getStructType (TMPNET *net)
{
	ubyte cmd = getCmdIndex(net);
	if (!cmd)
		return 0;
	
	if (cmd == CMD_SET_EQDATA || cmd == CMD_GET_EQDATA)
		return CMDSTRUCT_EQ;
	else if (cmd == CMD_GET_CURRENTTRKINFO)
		return CMDSTRUCT_GEN;
	else if (cmd == CMD_GET_SPECTRUMDATA)
		return CMDSTRUCT_WAVE;
	else if (cmd == CMD_HOSTEXIT)
		return CMD_HOSTEXIT;
	else if (cmd <= CMD_TOTAL)
		return CMDSTRUCT_CMD;

	return 0;
}

static int cmd_readWave (TMPNET *net, TMPCMDWAVE *wave)
{
	size_t dataSize = sizeof(TMPCMDWAVE) - sizeof(wave->data); // header length
	int ret = netReadSocketPeek(net, wave, dataSize);
	if (ret == 1)	// has has signaled its closing connection
		return 0;
	else if (ret > 1 && ret != dataSize)
		return -1;
	else if (wave->magic != MAGIC)
		return 0;

	if (wave->len > sizeof(wave->data)/* || wave->magic != MAGIC*/){
		printf("error reading wave data. data too large or invalid magic: %i > %i, magic = 0x%X, ret = %i\n",\
		  wave->len, sizeof(wave->data), wave->magic, ret);
		netEmptyReadBuffer(net);
		return -1;
	}
	
	dataSize = sizeof(TMPCMDWAVE) - (sizeof(wave->data)-wave->len);
	ret = netReadSocket(net, wave, dataSize);
	if (ret != dataSize || wave->magic != MAGIC){
		printf("error reading wave data. length mismatch; expected:%i but received:%i, magic = 0x%X\n",\
		  dataSize, ret, wave->magic);
		if (ret == -1){		// host has been terminated without sending shutdown signal
			return 0;
		}else{
			return -1;
		}
	}else{
		return 1;
	}
}

static int cmd_readEq (TMPNET *net, TMPCMDEQ *eq)
{
	return netReadSocket(net, eq, sizeof(TMPCMDEQ));
}

static int cmd_readCmd (TMPNET *net, TMPCMD *cmd)
{
	return netReadSocket(net, cmd, sizeof(TMPCMD));
}

static int cmd_readGen (TMPNET *net, TMPGENTRACKINFO *gen)
{
	return netReadSocket(net, gen, sizeof(TMPGENTRACKINFO));
}

int processSocketCmds (TMPNET *net)
{
	size_t pendingdata = 0;
	pendingdata = netIsPacketAvailable(net);
	if (!pendingdata)
		return 0;

	// use static to zero the structs
	static TMPCMD cmd;
	static TMPCMDWAVE wave;
	static TMPGENTRACKINFO gen;
	static TMPCMDEQ eq;
	

	do{
		switch(getStructType(net)){
		  case CMDSTRUCT_WAVE:
			cmd_readWave(net, &wave);
			//cmd_processWave(&wave); do with it what you want
			break;
			
		  case CMDSTRUCT_CMD:
			cmd_readCmd(net, &cmd);
			cmd_processCmd(net, &cmd);
			break;	

		  case CMDSTRUCT_GEN:
			cmd_readGen(net, &gen);
			//cmd_processGen(&gen);
			printf("\nCurrent track details:\n");
			printf(" Length:%i\n Position:%i\n Bitrate:%i\n Channels:%i\n Samplerate:%ikhz\n Playlist position:%i (0....n)\n Total tracks:%i\n\n",\
			  gen.length, gen.position, gen.bitrate, gen.channels, gen.samplerate, gen.playlistpos, gen.totaltracks);

			netGetMetaData(net, 1/*tag index*/, CMD_GET_METADATAW, gen.playlistpos, L"Artist", NULL, 0);
			netGetMetaData(net, 1/*tag index*/, CMD_GET_METADATAA, gen.playlistpos, "Artist", NULL, 0);
			netGetTitle(net, gen.playlistpos, CMD_GET_TRACKTITLEW, NULL, 0);
			netGetTitle(net, gen.playlistpos, CMD_GET_TRACKTITLEA, NULL, 0);
			break;
		
		  case CMDSTRUCT_EQ:
			cmd_readEq(net, &eq);
			//cmd_processEq(&eq);
			break;
		  	
		  case CMD_HOSTEXIT:
			printf("host exiting\n");
  			return 0;
	  		
		  default:
	 		printf("invalid packet\n");
	  		return 0;
		}
		pendingdata = netIsPacketAvailable(net);
	}while(pendingdata > 1);
	return 1;
}
