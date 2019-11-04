
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


#include <winsock2.h>


int msn_closeSocket (SOCKET socket)
{
	if (socket != SOCKET_ERROR){
		shutdown(socket, SD_SEND);
		return closesocket(socket);
	}else{
		return SOCKET_ERROR;
	}
}

int msn_sendSocket (SOCKET socket, void *buffer, size_t *bsize)
{
	if (socket == SOCKET_ERROR)
		return SOCKET_ERROR;

	int sent = 0;
	int total;
	for (total = 0; total < *bsize;){
		sent = send(socket, buffer+total, *bsize-total, 0);
		if ((sent == SOCKET_ERROR) || !sent){
			*bsize = total;
			return SOCKET_ERROR;
		}else{
			total += sent;
		}
	}
	*bsize = total;
	return total;
}

int msn_readSocketPeek (SOCKET socket, void *buffer, size_t bsize)
{
	char *pbuffer = (char *)buffer;
	size_t rtotal = 0;
	size_t len = 0;
	
//	while (rtotal < bsize){
		len = recv(socket, pbuffer+rtotal, bsize-rtotal, MSG_PEEK);
		if ((len == SOCKET_ERROR) || !len){
			return SOCKET_ERROR;
		}else{
			rtotal += len;
		}
//	}
    return rtotal;
}

int msn_readSocket (SOCKET socket, void *buffer, size_t bsize)
{
	char *pbuffer = (char *)buffer;
	size_t rtotal = 0;
	size_t len = 0;
	
//	while (rtotal < bsize){
		len = recv(socket, pbuffer+rtotal, bsize-rtotal, 0);
		if ((len == SOCKET_ERROR) || !len){
			return SOCKET_ERROR;
		}else{
			rtotal += len;
		}
//	}
    return rtotal;
}

/*
void initSocket ()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
}
*/

SOCKET msn_connectTo (char *addr, int port)
{
    struct hostent *hostPtr = NULL;
    struct sockaddr_in serverName = {0};
    SOCKET fdsocket = SOCKET_ERROR;

	if (port > 0xFFFF || !addr){
		dbprintf("connectTo(): invalid address/port\n");
		return SOCKET_ERROR;
	}

	fdsocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fdsocket == SOCKET_ERROR) {
        dbprintf("connectTo(): socket error\n");
        return SOCKET_ERROR;
    }

    hostPtr = gethostbyname(addr);
    if (hostPtr == NULL) {
        hostPtr = gethostbyaddr(addr, strlen(addr), PF_INET);
        if (hostPtr == NULL) {
        	dbprintf("connectTo(): unable to resolve '%s' to an address\n",addr);
        	return -2;
        }
    }

    serverName.sin_family = AF_INET;
    serverName.sin_port = htons(port);
    memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);
    if (connect(fdsocket,(struct sockaddr*)&serverName, sizeof(serverName)) == SOCKET_ERROR){
    	msn_closeSocket(fdsocket);
        return SOCKET_ERROR;
	}else{
    	return fdsocket;
    }
}


