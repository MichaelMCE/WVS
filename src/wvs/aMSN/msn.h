
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




#if defined (BUILDING_DLL)
# define ICON_MSG	L"wvs/images/msn1.tga"
# define ICON_MAIL	L"wvs/images/mail.tga"
#else
# define ICON_MSG	L"images/msn1.tga"
# define ICON_MAIL	L"images/mail.tga"
#endif




#define MSNSIZE_EMAIL				64
#define MSNSIZE_USERNAME			128
#define MSNSIZE_STATUS				16
#define MSNSIZE_USERLIST			96		// room for n users in list

#define MSNSIZE_USERMSG				384
#define MSNSIZE_USERMSGLIST			16		// store n messages then overright oldest

#define MSNSIZE_RECVBUFFER			8192
#define MSNSIZE_MESSAGEQUEUE 		MSNSIZE_RECVBUFFER
#define MSNSIZE_MESSAGEQUEUELIST	64

#define MSNMSG_ORIGINFROM			0
#define MSNMSG_ORIGINTO				1

#define MSNERROR_NONE				0
#define MSNERROR_REMOTEDISABLED		1
#define MSNERROR_BADALLOC			2
#define MSNERROR_AUTHFAILED			3
#define MSNERROR_AMSNNOTFOUND		4
#define MSNERROR_DISCONNECTED		5
#define MSNERROR_NOTCONNECTED		6
#define MSNERROR_CONNECTINGAMSN		7
#define MSNERROR_CONNECTINGACT		8
#define MSNERROR_CONNECTED			9

#define MSN_CONNTIMEOUT				8000



#define MSNSIZE_CONTACTLIST			6		// FL, RL, AL, BL, PL and All

#define MSN_CONTACTLIST_FL			0
#define MSN_CONTACTLIST_RL			1
#define MSN_CONTACTLIST_AL			2
#define MSN_CONTACTLIST_BL			3
#define MSN_CONTACTLIST_PL			4
#define MSN_CONTACTLIST_ALL			5

/*
static ubyte contactlist[6][16]={
	"Contact list",
	"Reverse list",
	"Allowed list",
	"Blocked list",
	"Pending list",
	"All contacts"
};
*/

typedef struct {
	char text[MSNSIZE_MESSAGEQUEUE+4];			// a single unprocessed message
	unsigned int size;			// message length
}TMESSAGE;

typedef struct{
	int	lastMessageSize;
	size_t	recvBufferSize;
	char	*recvBuffer;
	volatile SOCKET	socket;
	unsigned long recvTime; // time of last recv
}TCONNECTION;

typedef struct{
	int	msgTotal;			// number of messages extracted from packet
	int	queueTotal;			// total queue size
	TMESSAGE queue[MSNSIZE_MESSAGEQUEUELIST];		// storage space for incomming '\r\n' seperated messages
	TMESSAGE incomplete;	// storage space for an incomplete message when required
}TMESSAGEQUEUE;

typedef struct{
	char message[MSNSIZE_USERMSG+4];
	u64	timestamp;
	int origin;	// 0=from, 1=to
}TMSNUSERMSG;

typedef struct{
	char email[MSNSIZE_EMAIL+4];
	char username[MSNSIZE_USERNAME+4];
	char status[MSNSIZE_STATUS+4];
	TMSNUSERMSG msg[MSNSIZE_USERMSGLIST];
	int	state;
}TAMSNUSER;

typedef struct{
	int total;
	TAMSNUSER *user[MSNSIZE_USERLIST];
	int lastUser;
	int lastMsg;
}TAMSNUSERLIST;

typedef struct{
	char email[MSNSIZE_EMAIL+4];
	char username[MSNSIZE_USERNAME+4];
	char status[MSNSIZE_STATUS+4];
}TAMSNME;


typedef struct{
	unsigned int showStatus;
	unsigned int displayMode;
	int displayPage;
}TMSNCONFIG;

typedef struct{
	int startLine;
	int totalLines;
}TMSNDISPLAY;


typedef struct{
	char email[MSNSIZE_EMAIL+4];
}TAMSNCONTACT;

typedef struct{
	int total;
	TAMSNCONTACT *user;
}TAMSNCONTACTLIST;

typedef struct{
	char email[MSNSIZE_EMAIL+4];
	char name[MSNSIZE_USERNAME+4];
}TAMSNINBOXMAIL;

typedef struct{
	int total;
	TAMSNINBOXMAIL *mail;
}TAMSNINBOX;

typedef struct{
	TWINAMP *wa;
	TCONNECTION connection;
	TMESSAGEQUEUE cmds;
	TAMSNUSERLIST list;
	TAMSNINBOX inbox;
	TAMSNME me;
	TWVSINPUT input;
	TMSNCONFIG config;
	TMSNDISPLAY display;
	TAMSNACCOUNT account;
	TAMSNCONTACTLIST cl[MSNSIZE_CONTACTLIST];
	char nameBuffer[MSNSIZE_USERNAME+4];

	volatile int listeningState;	
	volatile int status;
	volatile int authed;
	HANDLE haMSNConn;
	int mailTotal;
	
	// user configurable values
	int encoding;				// 
	int inputKey;				// 
	ubyte autoConnect:1;		// 
	ubyte mailNotify:1;			// enable mail alert message
	ubyte iconOverlay:1;		// enable icons
	ubyte emoticons:1;			// enable emoticons
	ubyte mailAlertOvr:1;		// used as a signal by mail1 to enable a mail3 overlay
	ubyte nameFormat:3;			// 
}TAMSN;


static ubyte msnemsg[10][48]={
	"No error",
	"aMSN not found or remote control is disabled",
	"Bad Alloc",
	"Auth Failed",
	"aMSN not found",
	"Disconnected from aMSN",
	"Not connected to aMSN",
	"Connecting to aMSN..",
	"Connecting your account...",
	"Connected to aMSN"
};

int aMSNTimeoutTable[14] = {
	0,
	2000,
	5000,
	7000,
	10000,
	15000,
	20000,
	30000,
	60000,
	90000,
	120000,
	300000,
	600000
};

