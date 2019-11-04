
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


#ifndef dbprintf
#define dbprintf(X,...)  
#endif


//static CRITICAL_SECTION netlock_read_cs;
//static CRITICAL_SECTION netlock_write_cs;



static void NET_SEND_LOCK ()
{
	//EnterCriticalSection(&netlock_write_cs);
}

static void NET_SEND_UNLOCK ()
{
	//LeaveCriticalSection(&netlock_write_cs);
}

static void NET_SEND_LOCK_CREATE ()
{
	//InitializeCriticalSection(&netlock_write_cs);
}

static void NET_SEND_LOCK_DELETE ()
{
	//DeleteCriticalSection(&netlock_write_cs);
}


static void NET_READ_LOCK ()
{
	//EnterCriticalSection(&netlock_read_cs);
}

static void NET_READ_UNLOCK ()
{
	//LeaveCriticalSection(&netlock_read_cs);
}

static void NET_READ_LOCK_CREATE ()
{
	//InitializeCriticalSection(&netlock_read_cs);
}

static void NET_READ_LOCK_DELETE ()
{
	//DeleteCriticalSection(&netlock_read_cs);
}

/*
* IPPROTOCOL: 0=udp, 1=tcp (defined in makefile via -DIPPROTOCOL=n)
* Client must be compiled with IPPROTOCOL=0
*/

int closeSocket (SOCKET socket)
{
	if (socket != SOCKET_ERROR){
#if (IPPROTOCOL > 0)
		shutdown(socket, SD_SEND);
#endif
		return closesocket(socket);
	}else{
		return SOCKET_ERROR;
	}
}

#if (IPPROTOCOL > 0)	// TCP
int sendSocket (TMPNET *net, void *buffer, size_t *bsize)
{
	if (net->socket.client == SOCKET_ERROR)
		return SOCKET_ERROR;

	int sent = 0;
	int total;
	for (total = 0; total < *bsize;){
		sent = send(net->socket.client, buffer+total, *bsize-total, 0);
		if ((sent == SOCKET_ERROR) || !sent){
			*bsize = total;
			return SOCKET_ERROR;
		}else{
			total += sent;
		}
	}
	*bsize = total;
	net->sendCt++;
	return total;
}

int readSocket (TMPNET *net, void *buffer, size_t bsize)
{
	char *pbuffer = (char *)buffer;
	size_t rtotal = 0;
	size_t len = 0;
	
	while (rtotal < bsize){
		len = recv(net->socket.client, pbuffer+rtotal, bsize-rtotal, 0);
		if ((len == SOCKET_ERROR) || !len){
			return SOCKET_ERROR;
		}else{
			rtotal += len;
		}
	}
	net->readCt++;
    return rtotal;
}

int readSocketPeek (TMPNET *net, void *buffer, size_t bsize)
{
	char *pbuffer = (char *)buffer;
	size_t rtotal = 0;
	size_t len = 0;
	
	while (rtotal < bsize){
		len = recv(net->socket.client, pbuffer+rtotal, bsize-rtotal, MSG_PEEK);
		if (len == SOCKET_ERROR || !len){
			return SOCKET_ERROR;
		}else{
			rtotal += len;
		}
	}
    return rtotal;
}

#else	// UDP

int sendSocket (TMPNET *net, void *buffer, size_t *bsize)
{
	if (net->socket.server == SOCKET_ERROR)
		return SOCKET_ERROR;

	NET_SEND_LOCK();
	int ret = sendto(net->socket.server, buffer, *bsize, 0, (SOCKADDR *)&net->sockaddrin,  sizeof(struct sockaddr_in));
	if (ret > 0)
		net->sendCt++;
	NET_SEND_UNLOCK();
	return ret;
}

int readSocket (TMPNET *net, void *buffer, size_t bsize)
{
	if (net->socket.server == SOCKET_ERROR)
		return SOCKET_ERROR;

	NET_READ_LOCK();
	int fromlen = sizeof(struct sockaddr_in);
	int ret = recvfrom(net->socket.server, buffer, bsize, 0, (SOCKADDR *)&net->sockaddrin,  &fromlen);
	if (ret > 0)
		net->readCt++;
	NET_READ_UNLOCK();
	return ret;
}

int readSocketPeek (TMPNET *net, void *buffer, size_t bsize)
{
	if (net->socket.server == SOCKET_ERROR)
		return SOCKET_ERROR;

	NET_READ_LOCK();
	int fromlen = sizeof(struct sockaddr_in);
	int ret = recvfrom(net->socket.server, buffer, bsize, MSG_PEEK, (SOCKADDR *)&net->sockaddrin,  &fromlen);
	NET_READ_UNLOCK();
	return ret;
}

#endif

void shutdownSocket ()
{
	WSACleanup();
	NET_SEND_LOCK_DELETE();
	NET_READ_LOCK_DELETE();
}

void initSocket ()
{
	NET_SEND_LOCK_CREATE();
	NET_READ_LOCK_CREATE();
	WSADATA wsa;
	int ret = (int)WSAStartup(MAKEWORD(2,2),&wsa);
	if (ret)
		dbprintf("WSAStartup() failed: ret = %i: %s\n", ret, strerror(GetLastError()));
}

SOCKET connectTo (TMPNET *net, char *addr, int port, int proto)
{
    struct hostent *hostPtr = NULL;
    SOCKET fdsocket = SOCKET_ERROR;

	//port &= 0xFFFF;
	if (port > 0xFFFF || !addr){
		dbprintf("connectTo(): invalid address/port\n");
		return -4;
	}

	if (proto == IPPROTO_UDP)
		fdsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else if (proto == IPPROTO_TCP)
		fdsocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		return -3;
	
	if (fdsocket == SOCKET_ERROR){
        dbprintf("connectTo(): socket error\n");
        return -4;
    }

	ULONG x = SOCKETBUFFERSIZE;
	setsockopt(fdsocket, SOL_SOCKET, SO_SNDBUF, (char *)&x, sizeof(x));
	x = SOCKETBUFFERSIZE;
	setsockopt(fdsocket, SOL_SOCKET, SO_RCVBUF, (char *)&x, sizeof(x));

    hostPtr = gethostbyname(addr);
    if (hostPtr == NULL) {
        hostPtr = gethostbyaddr(addr, strlen(addr), AF_INET);
        if (hostPtr == NULL) {
        	dbprintf("connectTo(): unable to resolve '%s' to an address\n",addr);
        	return -2;
        }
    }

	if (net->port != port)
		net->port = port;

    net->sockaddrin.sin_family = AF_INET;
    net->sockaddrin.sin_port = htons(port);
    (void)memcpy(&net->sockaddrin.sin_addr, hostPtr->h_addr, hostPtr->h_length);
    
    if (proto == IPPROTO_UDP){
    	return fdsocket;
    }else{
    	if (connect(fdsocket,(struct sockaddr*)&net->sockaddrin, sizeof(net->sockaddrin)) == SOCKET_ERROR){
    		closeSocket(fdsocket);
    		dbprintf("connect() failed\n");
        	return SOCKET_ERROR;
		}else{
	    	return fdsocket;
    	}
    }
    
}

int bindPort (TMPNET *net, int port)
{
	net->port = port;
	net->sockaddrin.sin_family = AF_INET;
	net->sockaddrin.sin_port = htons(port);
	//net->sockaddrin.sin_addr.s_addr = htonl(INADDR_ANY);
	net->sockaddrin.sin_addr.s_addr = inet_addr("127.0.0.1");

	DWORD dwBytesReturned = 0;
	int x = 1;
	//setsockopt(net->socket.server, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&x, sizeof(x));
	setsockopt(net->socket.server, SOL_SOCKET, SO_REUSEADDR, (char*)&x, sizeof(x));
	x = SOCKETBUFFERSIZE;
	setsockopt(net->socket.server, SOL_SOCKET, SO_SNDBUF, (char*)&x, sizeof(x));
	x = SOCKETBUFFERSIZE;
	setsockopt(net->socket.server, SOL_SOCKET, SO_RCVBUF, (char*)&x, sizeof(x));
	x = 0;
	WSAIoctl(net->socket.server, SIO_UDP_CONNRESET, &x, sizeof(x), NULL, 0, &dwBytesReturned, NULL, NULL);
		
	if (bind(net->socket.server, (struct sockaddr*)&net->sockaddrin, sizeof(net->sockaddrin)) != 0)
		return SOCKET_ERROR;
	else
		return 1;
}

SOCKET waitForConnection (SOCKET server, SOCKADDR_IN *sockaddrin)
{
	int length = sizeof(SOCKADDR_IN);
	return accept(server, (struct sockaddr *)sockaddrin, &length);
}

