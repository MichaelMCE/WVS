
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


#include "msn.h"
#include "msn_lock.c"
#include "msn_net.c"
#include "msn_sendcmd.c"
#include "md5.c"
#include "editbox.c"


static char amsnAddress[] = "127.0.0.1";
static int amsnPort = 63251;
static TAMSN g_msn;
static int msnInitOnce = 0;
volatile static TFRAMEOVER msnOverlay1;		// msn message icon
volatile static TFRAMEOVER msnOverlay3;		// you've got mail icon
volatile static TFRAMEOVER msnOverlay2;		// new mail alert
volatile static TFRAMEOVER msnOverlay4;		// mail details
volatile static HANDLE haMSNConn = NULL;


unsigned int __stdcall connectionHandler (TAMSN *msn);


int displayInput_MSN (TWINAMP *wa, int key, void *data);
int renderMSN (TWINAMP *wa, TFRAME *frame, void *data);
int msnInit (TFRAME *frame, void *userPtr);
void msnCleanup ();
void aMSN_DoAutoConnect (TWINAMP *wa/*, TFRAME *frame*/);


int amsnPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderMSN(wa, frame, userPtr);
}

int amsnPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_MSN(wa, key, userPtr);
}

int amsnPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("amsnPageEnter\n");
	return 1;
}

void amsnPageExit (void *userPtr)
{
	//dbprintf("amsnPageExit\n");
}

int amsnPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("amsnPageOpen()\n");

	TWINAMP *wa = (TWINAMP*)userPtr;
	if (!strlen(wa->amsn.remotePassword) || !strlen(wa->amsn.email)){
		renderErrorPage(frame, P_OPNFAILED, "\naMSN - invalid config\n email or password not set or file is missing");
		return 0;
	}
	int ret = msnInit(frame, wa);
	if (ret)
		aMSN_DoAutoConnect(wa);
	return ret;
}

void amsnPageClose (void *userPtr)
{
	//dbprintf("amsnPageClose()\n");
	msnCleanup();
}

static void translateKey (char *key, char *pass, char *outHex)
{
	ubyte out[64];
	memset(out, ' ', sizeof(out));
	unsigned short *phex = (unsigned short *)outHex;
	
	md5_hmac((ubyte*)key, strlen(key), (ubyte*)pass, strlen(pass), out);
	int i = 0;
	for (i = 0; i < 16; i ++)
		sprintf((char*)&phex[i],"%x", out[i]);
}

static int msn_Authenticate (SOCKET fdsocket, char *password)
{
	char *key = NULL;
	char buffer[512];
	char out[512];
	memset(buffer, 0, sizeof(buffer));
	memset(out, 0, sizeof(out));
	
	size_t len = 6;
	msn_sendSocket(fdsocket, "auth\r\n", &len);
	len = msn_readSocket(fdsocket, buffer, sizeof(buffer));

	if (len > 12){
		if (!strncmp(buffer, "auth ", 5)){
			key = strtok(buffer+5, " ");
			translateKey(key, password, out);
	
			char *auth2 = (char*)calloc(sizeof(char), 512);
			if (auth2 != NULL){
				strcat(auth2, "auth2 ");
				strncat(auth2, out, strlen(out));
				strcat(auth2, "\r\n");
				
				len = strlen(auth2);
				msn_sendSocket(fdsocket, auth2, &len);
				msn_readSocket(fdsocket, buffer, sizeof(buffer));
				free(auth2);
			}
			return (strstr(buffer, "successfull") != NULL);
		}
	}
	return 0;
}

static int msn_GetConnectionPort (TAMSN *msn, int port)
{
	char buffer[128];

	SOCKET fdsocket = msn_connectTo(amsnAddress, port);
	if (fdsocket != SOCKET_ERROR){
		size_t len = strlen(msn->account.email);
		msn_sendSocket(fdsocket, msn->account.email, &len);
		len = 2;
		msn_sendSocket(fdsocket, "\r\n", &len);
		msn_readSocket(fdsocket, buffer, sizeof(buffer));
		msn_closeSocket(fdsocket);
		return atoi(buffer);
	}
	return -1;
}

static int collectPacket (TAMSN *msn)
{
    memset(msn->connection.recvBuffer, 0, msn->connection.recvBufferSize);
	if (msn->cmds.incomplete.size)
		memcpy(msn->connection.recvBuffer, msn->cmds.incomplete.text, msn->cmds.incomplete.size);
	
	msn->connection.lastMessageSize = msn_readSocket(msn->connection.socket, msn->connection.recvBuffer + msn->cmds.incomplete.size,\
	msn->connection.recvBufferSize-msn->cmds.incomplete.size-1);

	if (msn->connection.lastMessageSize > 0){
		msn->connection.lastMessageSize += msn->cmds.incomplete.size;
		msn->connection.recvTime = GetTickCount();
		return 1;
	}else{
		msn->connection.recvTime = 0;
		return 0;
	}
}

static int extractMessages (TAMSN *msn)
{
	msn->cmds.msgTotal = 0;
	int start = 0;
	int i;
	
	//printf("extracting... %i ", (int)msn->connection.lastMessageSize);
	//printf("\n\n###%s###\n",msn->connection.recvBuffer);
	
	for (i=0; i < msn->connection.lastMessageSize  && (msn->cmds.msgTotal < MSNSIZE_MESSAGEQUEUELIST); i++){
		if (msn->connection.recvBuffer[i] == '\r'){
			msn->connection.recvBuffer[i] = 0;
			//printf("%i ",msn->cmds.msgTotal);
			memcpy(msn->cmds.queue[msn->cmds.msgTotal++].text, msn->connection.recvBuffer+start, MIN((i-start)+1, MSNSIZE_MESSAGEQUEUE-1));
			if (msn->connection.recvBuffer[++i] == '\n')
				i++;
			start = i;
			

		}
	}
	if (i-start > 1){
		msn->cmds.incomplete.size = i-start; 
		memcpy(msn->cmds.incomplete.text, &msn->connection.recvBuffer[start], msn->cmds.incomplete.size);
		msn->cmds.incomplete.text[msn->cmds.incomplete.size] = 0;
	}else{
		msn->cmds.incomplete.size = 0;
	}

	//printf(" done %i\n", i);
	return 1;
}

static int msn_SendMsnUserMsgId (TAMSN *msn, int u, char *text)
{
	if (u && u <= msn->list.total){
		u--;
		//printf("-%s- -%s-\n",msn->list.user[u]->email, text);
		char sendbuffer[MSNSIZE_INPUTBUFFER*2];
		char *pbuffer = sendbuffer;
		
		int i = 0;
		while(text[i]){
			if (text[i] == '$' || text[i] == ';' || text[i] == '\"' || text[i] == '[' || text[i] == ']'){
				*pbuffer++ = '\\';
			}else if (text[i] == 3){
				*pbuffer++ = ':';
				text[i] = ')';
			}else if (text[i] == 4){
				*pbuffer++ = ':';
				text[i] = '(';
			}
							
			*pbuffer++ = text[i++];
		}
		*pbuffer = 0;
		//printf("-%s- -%s-\n",msn->list.user[u]->email, sendbuffer);
		return msn_SendCmdMsg(msn, msn->list.user[u]->email, sendbuffer);
	}else{
		return 0;
	}
}

static int msn_SendMsnUserMsg (TAMSN *msn, char *text)
{
	size_t len = strlen(text);
	if (len < 3)
		return 0;
		
	int u = 0;
	if (text[1] == ':'){
		if (len < 3)
			return 0;
		u = atoi(text);
		text += 2;
	}else if (text[2] == ':'){
		if (len < 4)
			return 0;
		u = atoi(text);
		text += 3;
	}else{
		return 0;
	}
	return msn_SendMsnUserMsgId(msn, u, text);
}

static int isHostAlive (TAMSN *msn)
{
	if (GetTickCount()-msn->connection.recvTime > MSN_CONNTIMEOUT){
		lSleep(100);
		return (msn_SendCmdCheckMail(msn) > 0);
		//return (msn_SendCmdPing(msn) > 0);
		//return (msn_SendCmdOnline(msn) > 0);
	}else{
		return 1;
	}
}

static int isMsgPong (char *text)
{
	if (!strncmp(text,"18| ", 4)){
		if (strstr(text," pong "))
			return 1;
	}
	return 0;
}

static int isMsgUserStatus (char *text)
{
	if (!strncmp(text,"19| ", 4)){
		if (strstr(text,"@")){
			if (strstr(text,".")){
				if (strstr(text," - "))
					return 1;
			}
		}
	}
	return 0;
}

static int isMsgMail1 (char *text)
{
	if (!strncmp(text,"28| ", 4)){
		if (strstr(text," mailmsg1"))
			return 1;
	}
	return 0;
}

static int isMsgMail2 (char *text)
{
	if (!strncmp(text,"29| ", 4)){
		if (strstr(text," mailmsg2"))
			return 1;
	}
	return 0;
}

static int isMsgMail3 (char *text)
{
	if (!strncmp(text,"30| ", 4)){
		if (strstr(text," mailmsg3"))
			return 1;
	}
	return 0;
}

static int isMsgOnline (char *text)
{
	if (!strncmp(text,"6| ", 3)){
		if (strstr(text,"@")){
			if (strstr(text,".")){
				if (strstr(text," - ")){
					if (strstr(text," ---:| "))
						return 1;
				}
			}
		}
	}
	return 0;
}

static int isMsgStatus (char *text)
{
	if (!strncmp(text,"** ", 3)){
		if (strstr(text,"(")){
			if (strstr(text,"@")){
				if (strstr(text,".")){
					if (strstr(text,")"))
						return 1;
				}
			}
		}
	}
	return 0;
}

static int isMsgTo (char *text)
{
	if (!strncmp(text,"To ", 3)){
		if (strstr(text,"@")){
			if (strstr(text,".")){
				if (strstr(text," : ")){
					if (strstr(text," msgsent"))
						return 1;
				}
			}
		}
	}
	return 0;
}

static int isMsgFrom (char *text)
{
	if (!strncmp(text,"From ", 5)){
		if (strstr(text,"@")){
			if (strstr(text,".")){
				if (strstr(text," : ")){
					if (strstr(text," msgrcv"))
						return 1;
				}
			}
		}
	}
	return 0;
}

static int isMsgMyState (char *text)
{
	if (!strncmp(text,"7| ", 3)){
		//if (strstr(text," : ")){
			if (strstr(text," normal"))
				return 1;
		//}	
	}
	return 0;
}

static int getMsgComponentsUserStatus (char *text, char **email, char **status)
{
	char *tmp = strstr(text," ");
	if (tmp){
		*email = tmp+1;
		tmp = strstr(*email," - ");
		if (tmp){
			*tmp = 0;
			*status = tmp+3;
			tmp = strstr(*status," normal");
			if (tmp){
				*tmp = 0;
				return 1;
			}
		}
	}
	return 0;
}

static int getMsgComponentsMyState (char *text, char **status)
{
	char *tmp = strstr(text," ");
	if (*tmp){
		*status = tmp+1;
		tmp = strstr(text," normal");
		if (tmp){
			*tmp = 0;
			return 1;
		}
	}
	return 0;
}

static int getMsgComponentsMailMsg1 (char *text, int *tmails)
{
	if (strlen(text) > 4){
		*tmails = (int)atoi(text+4);
		return 1;
	}
	return 0;
	
}

static int getMsgComponentsMailMsg2 (char *text, char **alert)
{
	char *tmp = strstr(text," ");
	if (*tmp){
		*alert = tmp+1;
		tmp = strstr(text," mailmsg2");
		if (tmp){
			*tmp = 0;
			return 1;
		}
	}
	return 0;
}

static void freeInbox (TAMSN *msn)
{
	if (msn->inbox.mail)
		free(msn->inbox.mail);
	msn->inbox.mail = NULL;
	msn->inbox.total = 0;
}

static int addInboxMail (TAMSN *msn, char *name, char *email)
{
	//if (tusers > msn->inbox.total){
		if (msn->inbox.mail == NULL){
			msn->inbox.mail = calloc(1, sizeof(TAMSNINBOXMAIL));
			if (msn->inbox.mail){
				if (name)
					strncpy(msn->inbox.mail[0].name, name, MSNSIZE_USERNAME);
				if (email)
					strncpy(msn->inbox.mail[0].email, email, MSNSIZE_EMAIL);

				msn->inbox.total = 1;
				return 1;
			}
		}else{
			msn->inbox.mail = realloc(msn->inbox.mail, (1+msn->inbox.total) * sizeof(TAMSNINBOXMAIL));
			if (msn->inbox.mail){
				if (name)
					strncpy(msn->inbox.mail[msn->inbox.total].name, name, MSNSIZE_USERNAME);
				if (email)
					strncpy(msn->inbox.mail[msn->inbox.total].email, email, MSNSIZE_EMAIL);
					
				msn->inbox.total++;
				return 1;
			}
		}
	//}
	return 0;
}

static int getMsgComponentsInboxDetails (TAMSN *msn, char *text)
{
	int count = 0;
	char *ptext = text;
	char *name = NULL;
	char *email = NULL;

	char *tmp = strstr(ptext,"30| ");
	if (*tmp){
		ptext = tmp+4;
		while(ptext){
			if (*ptext){
				ptext = strstr(ptext,"| ");
				if (ptext){
					name = ptext+2;
					ptext = strstr(name," @ ");
					if (ptext){
						*ptext = 0;
						email = ptext+3;
						ptext = strstr(email," ");
						if (ptext){
							*ptext = 0;
							ptext++;
							count++;
							addInboxMail(msn, name, email);
						}
					}
				}
			}else{
				break;
			}
		}
	}
	return count;
}

// mailmsg3
static int getMsgComponentsInboxDetailsLast (char *ptext, char **pname, char **pemail)
{
	int count = 0;
	char *name = NULL;
	char *email = NULL;
	
	char *tmp = strstr(ptext,"30| ");
	if (*tmp){
		ptext = tmp+4;
		while(ptext){
			if (*ptext){
				ptext = strstr(ptext,"| ");
				if (ptext){
					name = ptext+2;
					ptext = strstr(name," @ ");
					if (ptext){
						*ptext = 0;
						email = ptext+3;
						ptext = strstr(email," ");
						if (ptext){
							*ptext = 0;
							ptext++;
							count++;
							*pname = name;
							*pemail = email;
						}
					}
				}
			}else{
				break;
			}
		}
	}

	return count;
}

static int msn_setMyState (TAMSN *msn, char *status)
{
	if (status){
		if (*status){
			strncpy(msn->me.status, status, MSNSIZE_STATUS);
			return 1;
		}
	}
	//printf("msn_setMyState(): invalid status\n");
	return 0;
}
				
static int isMsgMyDetails (char *text, char *email)
{
	if (!strncmp(text,"14| ", 4)){
		//if (strstr(text,"@")){
		if (strstr(text, email)){
			//if (strstr(text,".")){
				if (strstr(text," : ")){
					if (strstr(text," normal"))
						return 1;
				}
			//}
		}
	}
	return 0;
}

	
static int getMsgComponentsMyDetails (char *text, char **email, char **username)
{
	char *tmp = strstr(text," : ");
	if (tmp){
		*username = tmp+3;
		tmp = strstr(text, " normal");
		if (tmp){
			*tmp = 0;
			*email = strtok(text+4, " ");
			if (*email)
				return 1;
		}
	}
	return 0;
}

static int msn_setMyDetails (TAMSN *msn, char *email, char *username)
{
	if (email && username){
		strncpy(msn->me.email, email, MSNSIZE_EMAIL);
		strncpy(msn->me.username, username, MSNSIZE_USERNAME);
		return 1;
	}else{
		//printf("msn_setMyDetails(): invalid details supplied\n");
		return 0;
	}
}

static int getMsgComponentsTo (char *text, char **email, char **message)
{
	char *tmp = text+3;
	*email = strtok(tmp, " ");
	if (*email){
		*message = tmp + strlen(*email)+3;
		int i = strlen(*message);
		if (i){
			*(*message+(i-8)) = 0;
			return 1;
		}
	}
	return 0;
}

static int getMsgComponentsFrom (char *text, char **email, char **message)
{
	char *tmp = text+5;
	*email = strtok(tmp, " ");
	if (*email){
		*message = tmp + strlen(*email)+3;
		int i = strlen(*message);
		if (i){
			*(*message+(i-7)) = 0;
			return 1;
		}
	}
	return 0;
}

static int getMsgComponentsStatus (char *text, char **email, char **username, char **status)
{
	char *tmp = text;
	if (tmp){
		*username = tmp+3;
		tmp = strchr(text, '(');
		while(tmp){
			if (tmp)
				*email = tmp+1;
			tmp = strchr(*email, '(');
		};
		
		if (*email){
			*(*email-2) = 0;
			*email = strtok(*email, ")");
			if (*email){
				*status = *email+strlen(*email)+2;
				int i = strlen(*status);
				tmp = *status + i-1;
				int ct = 0;

				if (i){
					for (; i>=0; i--, tmp--){
						if (*tmp == ' '){
							if (!ct){
								ct = 1;
								*tmp = 0;
							}else{
								*tmp = 0;
								*status = tmp+1;
								return 1;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}


static int allocateContactList (TAMSN *msn, int listid, int tusers, char *user)
{
	if (tusers > msn->cl[listid].total){
		if (msn->cl[listid].user == NULL){
			msn->cl[listid].user = calloc(tusers, sizeof(TAMSNCONTACT));
			if (msn->cl[listid].user){
				msn->cl[listid].total = tusers;
				if (user){
					strncpy(msn->cl[listid].user[tusers-1].email, user, MSNSIZE_EMAIL);
					//printf("%i #%s#\n", listid, msn->cl[listid].user[tusers-1].email);
				}
				return 1;
			}
		}else{
			msn->cl[listid].user = realloc(msn->cl[listid].user, tusers * sizeof(TAMSNCONTACT));
			if (msn->cl[listid].user){
				msn->cl[listid].total = tusers;
				if (user)
					strncpy(msn->cl[listid].user[tusers-1].email, user, MSNSIZE_EMAIL);
				return 1;
			}
		}
	}
	return 0;
}

static void freeContactList (TAMSN *msn, int listid)
{
	if (msn->cl[listid].user != NULL)
		free(msn->cl[listid].user);
	 msn->cl[listid].user = NULL;
	 msn->cl[listid].total = 0;
}

static int addContactList (TAMSN *msn, int listid, char *text)
{
	freeContactList(msn, listid);
	int total = 0;

	char *user = strtok(text, " ");
	while(user){
		if (strchr(user, '@'))
			allocateContactList(msn, listid, ++total, user);
		user = strtok(NULL, " ");
	}
	//printf("list %i = %i\n", listid, total);
	return total;
}

static int isMsgListFL (char *text)
{
	return (0 == strncmp(text,"31| ", 4) && strstr(text," contactlist"));
}

static int isMsgListRL (char *text)
{
	return (0 == strncmp(text,"32| ", 4) && strstr(text," contactlist"));
}

static int isMsgListAL (char *text)
{
	return (0 == strncmp(text,"33| ", 4) && strstr(text," contactlist"));
}

static int isMsgListBL (char *text)
{
	return (0 == strncmp(text,"34| ", 4) && strstr(text," contactlist"));
}

static int isMsgListPL (char *text)
{
	return (0 == strncmp(text,"35| ", 4) && strstr(text," contactlist"));
}

static int isMsgListAll (char *text)
{
	return (0 == strncmp(text,"36| ", 4) && strstr(text," contactlist"));
}

static int isMsgConnecting (char *text)
{
	if (!strncmp(text,"3| connecting ... ", 18))
		return 1;
	else
		return 0;
}

static int isMsgConnected (char *text)
{
	if (!strncmp(text,"1| connected... ", 16))
		return 1;
	else
		return 0;
}

static int getMsgComponentsOnline (char *text, char **email, char **username, char **status)
{

	char *tmp = strtok(text, " ");	// skip "6| "
	if (tmp){
		*email = strtok(NULL, " ");
		if (*email){
			*username = *email+strlen(*email)+3;
			if (*username){
				tmp = strstr(*username," ---:| ");
				if (tmp){
					*tmp = 0;
					tmp += 7;
					*status = tmp;
					tmp = strstr(*status, " normal");
					if (tmp){
						*tmp = 0;
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static int msn_deleteAllusers (TAMSN *msn)
{
	int i;
	for (i = 0; i < msn->list.total; i++){
		if (msn->list.user[i]){
			free(msn->list.user[i]);
			msn->list.user[i] = NULL;
		}
	}
	msn->list.total = 0;
	return 1;
}

static int msn_isUserInList (TAMSN *msn, char *email)
{
	int i;
	size_t len = strlen(email);

	for (i = 0; i < msn->list.total; i++){
		if (!strncmp(msn->list.user[i]->email, email, len))
			return i;
	}
	return -1;
}

static int msn_setUserStatus (TAMSN *msn, char *email, char *status, int state)
{
	int i = msn_isUserInList(msn, email);
	if (i != -1){
		if (status == NULL)
			*msn->list.user[i]->status = 0;
		else
			strncpy(msn->list.user[i]->status, status, MSNSIZE_STATUS);
		if (state != -1)
			msn->list.user[i]->state = state;
		return 1;
	}else{
		//printf("msn_setUserStatus(): user not found -%s- %i\n",email, state);
		return 0;
	}
}

static int msn_addUser (TAMSN *msn, char *email, char *username, char *status)
{
	int i = msn_isUserInList(msn, email);
	if (i != -1){
		// user is already in list so just update displayname and status
		//strncpy(msn->list.user[i]->email, email, MSNSIZE_EMAIL);
		if (username)
			strncpy(msn->list.user[i]->username, username, MSNSIZE_USERNAME);
		if (status)
			strncpy(msn->list.user[i]->status, status, MSNSIZE_STATUS);
		msn->list.user[i]->state = 1;
		return 1;
	}else{
		if (msn->list.total < MSNSIZE_USERLIST){
			TAMSNUSER *user = (TAMSNUSER*)calloc(1, sizeof(TAMSNUSER));
			if (user != NULL){
				strncpy(user->email, email, MSNSIZE_EMAIL);
				strncpy(user->username, username, MSNSIZE_USERNAME);
				strncpy(user->status, status, MSNSIZE_STATUS);
				user->state = 1;
				msn->list.user[msn->list.total++] = user;
				return 1;
			}
		}else{
			//printf("msn_addUser(): user not added, too many users (%s)\n", email);
			return 0;
		}
	}
	return 0;
}

static u64 msn_getNextMessageUserIndex (TAMSNUSER *user, u64 stampBase, char **username, char **message, int *origin)
{
	u64 stampMin = rdtsc();
	if (username)
		*username = NULL;
	if (message)
		*message = NULL;
	int m;
	for (m = 0; m < MSNSIZE_USERMSGLIST; m++){	
		if (user->msg[m].timestamp > stampBase && user->msg[m].timestamp <= stampMin){
			if (message)
				*message = user->msg[m].message;
			if (origin)
				*origin = user->msg[m].origin;
			if (username)
				*username = user->username;
			stampMin = user->msg[m].timestamp;
		}
	}
	return stampMin;
}

static u64 msn_getNextMessageTime (TAMSNUSERLIST *list, u64 stampBase, char **username, char **message, int *origin)
{
	u64 stampMin = rdtsc();
	int u,m;
	if (username)
		*username = NULL;
	if (message)
		*message = NULL;

	for (u = 0; u < list->total; u++){
		for (m = 0; m < MSNSIZE_USERMSGLIST; m++){	
			if (list->user[u]->msg[m].timestamp > stampBase && list->user[u]->msg[m].timestamp <= stampMin){
				if (message)
					*message = list->user[u]->msg[m].message;
				if (origin)
					*origin = list->user[u]->msg[m].origin;
				if (username)
					*username = list->user[u]->username;
				stampMin = list->user[u]->msg[m].timestamp;
			}
		}
	}
	return stampMin;
}

static int msn_getMessageSlot (TMSNUSERMSG *msg)
{
	u64 stamp = rdtsc();
	int slot = 0;
	int i;
	for (i = 0; i < MSNSIZE_USERMSGLIST; i++){
		if (!msg[i].timestamp){
			return i;
		}else if (msg[i].timestamp < stamp){
			stamp = msg[i].timestamp;
			slot = i;
		}
	}
	return slot;
}

static int msn_addMessage (TAMSN *msn, char *email, char *message, int origin)
{
	int i = msn_isUserInList(msn, email);	
	if (i != -1){
		int slot = msn_getMessageSlot(msn->list.user[i]->msg);
		strncpy(msn->list.user[i]->msg[slot].message, message, MSNSIZE_USERMSG);
		msn->list.user[i]->msg[slot].origin = origin;
		msn->list.user[i]->msg[slot].timestamp = rdtsc();
		msn->list.lastUser = i;
		msn->list.lastMsg = slot;
	}else{
		//printf("msn_addMessage(): user not found -%s-\n",email);
	}
	return i;
}

static int msn_appendToLastMessage (TAMSN *msn, char *message)
{
	// remove trailing message ident, if any
	int origin;
	char *tmp = strstr(message, " msgrcv");
	if (tmp){
		*tmp = 0;
		origin = MSNMSG_ORIGINFROM;
	}else{
		origin = MSNMSG_ORIGINTO;
		tmp = strstr(message, " msgsent");
		if (tmp)
			*tmp = 0;
	}
	
	#if 1
	 msn_addMessage(msn, msn->list.user[msn->list.lastUser]->email, message, origin);
	#else
	 strncat(msn->list.user[msn->list.lastUser]->msg[msn->list.lastMsg].message, "\n", MSNSIZE_USERMSG);
	 strncat(msn->list.user[msn->list.lastUser]->msg[msn->list.lastMsg].message, message, MSNSIZE_USERMSG);
	#endif
	return 1;
}

static int dispatchMessages (TAMSN *msn)
{
	TMSNLIST_LOCK();

	int i;
	for (i = 0; i < msn->cmds.msgTotal; i++){
		//printf("##%s##\n",msn->cmds.queue[i].text);
		 if (isMsgOnline(msn->cmds.queue[i].text)){
			//printf("Online: %s\n",msn->cmds.queue[i].text);
			char *email = NULL;
			char *username = NULL;
			char *status = NULL;
			if (getMsgComponentsOnline(msn->cmds.queue[i].text, &email, &username, &status)){
				//printf("user online:%i: #%s# #%s# #%s#\n", msn->list.total, email, username, status);
				msn_addUser(msn, email, username, status);
			}
		}else if (isMsgStatus(msn->cmds.queue[i].text)){
			char *email = NULL;
			char *username = NULL;
			char *status = NULL;
			if (getMsgComponentsStatus(msn->cmds.queue[i].text, &email, &username, &status)){
				//printf("status: #%s# #%s# #%s#\n", email, username, status);
				if (!msn_setUserStatus(msn, email, status, 0))
					msn_addUser(msn, email, username, status);
				msn_SendCmdGetUserStatus(msn, email);
				msn_SendCmdOnline(msn);
				msn_SendCmdGetState(msn);
			}
		}else if (isMsgMail1(msn->cmds.queue[i].text)){	// total unread mails
			int unread = 0;
			if (getMsgComponentsMailMsg1(msn->cmds.queue[i].text, &unread)){
				//printf("unread mails %i\n",unread);
				if (unread != msn->mailTotal && msn->iconOverlay){
					msn->mailTotal = unread;
					msn_SendCmdGetMailAlert(msn);
					msn->mailAlertOvr = 1;
				}
				if (unread && msn->iconOverlay){
					if (!msnOverlay1.status)
						msnOverlay3.x = frame->width+10;
					else
						msnOverlay3.x = (frame->width-1 - msnOverlay3.frame->width) - 12;
					PostMessage(msn->wa->hook.hwnd, 7000, (WPARAM)&msnOverlay3, (LPARAM)msn->wa);
				}else{
					msnOverlay3.status = 0;
				}
				msn_SendCmdGetMailDetails(msn);
			}
		}else if (isMsgMail2(msn->cmds.queue[i].text)){	// mail alert message
			char *alert = NULL;
			if (getMsgComponentsMailMsg2(msn->cmds.queue[i].text, &alert)){
				if (!msnOverlay2.status && msn->mailNotify){ // don't update overlay until/unless its expired
					lSetCharacterEncoding(msn->wa->hw, g_msn.encoding);
					REFRESH_LOCK();
					msnOverlay2.frame = lNewString(frame->hw, NEWSTRINGTYPE, PF_INVERTTEXT, LFT_COMICSANSMS7X8, " %s", alert);
					REFRESH_UNLOCK();
					if (msnOverlay2.frame != NULL)
						PostMessage(msn->wa->hook.hwnd, 7000, (WPARAM)&msnOverlay2, (LPARAM)msn->wa);
					else
						dbprintf("aMSN_dispatchMessages(): msnOverlay2.frame == NULL\n");
				}
			}			
		}else if (isMsgMail3(msn->cmds.queue[i].text)){	// mail details
			//printf("mail3: #%s# %p\n",msn->cmds.queue[i].text, msn->cmds.queue[i].text);
			char *name = NULL;
			char *email = NULL;
			if (getMsgComponentsInboxDetailsLast(msn->cmds.queue[i].text, &name, &email)){
				freeInbox(msn);
				getMsgComponentsInboxDetails(msn, msn->cmds.queue[i].text);
				if (msn->mailAlertOvr && !msnOverlay4.status && msn->mailNotify){ // don't update overlay until/unless its expired
					msn->mailAlertOvr = 0;
					lSetCharacterEncoding(msn->wa->hw, g_msn.encoding);
					REFRESH_LOCK();
					msnOverlay4.frame = lNewString(frame->hw, NEWSTRINGTYPE, PF_INVERTTEXT, LFT_COMICSANSMS7X8, " %s %s", name, email);
					REFRESH_UNLOCK();
					if (msnOverlay4.frame != NULL)
						PostMessage(msn->wa->hook.hwnd, 7000, (WPARAM)&msnOverlay4, (LPARAM)msn->wa);
					else
						dbprintf("aMSN_dispatchMessages(): msnOverlay4.frame == NULL\n");
				}
			}else if (msn->inbox.total){
				freeInbox(msn);
			}
		}else if (isMsgFrom(msn->cmds.queue[i].text)){
			//printf("<From : #%s#\n", msn->cmds.queue[i].text);
			char *email = NULL;
			char *message = NULL;
			if (getMsgComponentsFrom(msn->cmds.queue[i].text, &email, &message)){
				msn_addMessage(msn, email, message, MSNMSG_ORIGINFROM);

				if (msn->iconOverlay){
					if (msnOverlay3.status)
						msnOverlay3.x = (frame->width-1 - msnOverlay3.frame->width) - 12;
					PostMessage(msn->wa->hook.hwnd, 7000, (WPARAM)&msnOverlay1, (LPARAM)msn->wa);
				}
			}
		}else if (isMsgTo(msn->cmds.queue[i].text)){	
			//printf(">To : %s\n", msn->cmds.queue[i].text);
			char *email = NULL;
			char *message = NULL;
			if (getMsgComponentsTo(msn->cmds.queue[i].text, &email, &message)){
				//printf("To: #%s# #%s#\n", email, message);
				msn_addMessage(msn, email, message, MSNMSG_ORIGINTO);
			}
		}else if (isMsgMyDetails(msn->cmds.queue[i].text, msn->account.email)){
			char *email = NULL;
			char *username = NULL;
			if (getMsgComponentsMyDetails(msn->cmds.queue[i].text, &email, &username)){
				msn_setMyDetails(msn, email, username);
			}
		}else if (isMsgMyState(msn->cmds.queue[i].text)){
			char *status = NULL;
			if (getMsgComponentsMyState(msn->cmds.queue[i].text, &status)){
				msn_setMyState(msn, status);
				//printf("status '%s'\n", status);
			}
		}else if (isMsgUserStatus(msn->cmds.queue[i].text)){
			char *email = NULL;
			char *status = NULL;
			if (getMsgComponentsUserStatus(msn->cmds.queue[i].text, &email, &status)){
				//printf("status for: %s %s\n",email, status);
				if (!msn_setUserStatus(msn, email, status, -1))
					msn_addUser(msn, email, email, status);
			}
		}else if (isMsgConnecting(msn->cmds.queue[i].text)){
			msn->status = MSNERROR_CONNECTINGACT;
			//printf("* Connecting to your account...\n");

		}else if (isMsgConnected(msn->cmds.queue[i].text)){
			msn->status = MSNERROR_CONNECTED;
			//printf("* Connected to your account\n");
			freeInbox(msn);
			msn_SendCmdGetState(msn);
			msn_SendCmdWhatIs(msn, msn->account.email);
			msn_SendCmdOnline(msn);
			msn_SendCmdStatus(msn);
			msn_SendCmdCheckMail(msn);
			msn_SendCmdGetContactList(msn);
			msn_SendCmdGetAcceptList(msn);
			msn_SendCmdGetBlockList(msn);
			msn_SendCmdGetPendingList(msn);
			msn_SendCmdGetReverseList(msn);
			msn_SendCmdGetAllContacts(msn);

		}else if (isMsgListFL(msn->cmds.queue[i].text)){
		//	printf("\nFL:%s\n", msn->cmds.queue[i].text+4);
			addContactList(msn, MSN_CONTACTLIST_FL, msn->cmds.queue[i].text+4);
			
		}else if (isMsgListRL(msn->cmds.queue[i].text)){
		//	printf("\nRL:%s\n", msn->cmds.queue[i].text+4);
			addContactList(msn, MSN_CONTACTLIST_RL, msn->cmds.queue[i].text+4);
			
		}else if (isMsgListAL(msn->cmds.queue[i].text)){
		//	printf("\nAL:%s\n", msn->cmds.queue[i].text+4);
			addContactList(msn, MSN_CONTACTLIST_AL, msn->cmds.queue[i].text+4);
			
		}else if (isMsgListBL(msn->cmds.queue[i].text)){
		//	printf("\nBL:%s\n", msn->cmds.queue[i].text+4);
			addContactList(msn, MSN_CONTACTLIST_BL, msn->cmds.queue[i].text+4);
			
		}else if (isMsgListPL(msn->cmds.queue[i].text)){
		//	printf("\nPL:%s\n", msn->cmds.queue[i].text+4);
			addContactList(msn, MSN_CONTACTLIST_PL, msn->cmds.queue[i].text+4);
						
		}else if (isMsgListAll(msn->cmds.queue[i].text)){
		//	printf("\nAll:%s\n", msn->cmds.queue[i].text+4);
			addContactList(msn, MSN_CONTACTLIST_ALL, msn->cmds.queue[i].text+4);
			
		}else if (isMsgPong(msn->cmds.queue[i].text)){
			// do nothing
			
		}else if (strstr(msn->cmds.queue[i].text, " msgrcv")){ //  appened to last time stampped message
			//printf("< %s\n", msn->cmds.queue[i].text);
			msn_appendToLastMessage(msn, msn->cmds.queue[i].text);
			
		}else if (strstr(msn->cmds.queue[i].text, " msgsent")){ //  appened to last time stampped message
			//printf("> %s\n", msn->cmds.queue[i].text);
			msn_appendToLastMessage(msn, msn->cmds.queue[i].text);
			
		}else{
			//printf("unhandled message: #%s#\n", msn->cmds.queue[i].text);
		}
		
	}
	TMSNLIST_UNLOCK();
	return 1;
}

static int isPacketAvailable (SOCKET socket)
{
	unsigned long pendingdata = 0; 
	int iocret = ioctlsocket(socket, FIONREAD, &pendingdata);
	return (!iocret && pendingdata);
}

unsigned int __stdcall connectionHandler (TAMSN *msn)
{
	lSleep(100);
	msn_SendCmdConnect(msn);

	do{
		if (isPacketAvailable(msn->connection.socket)){
			if (collectPacket(msn)){
				extractMessages(msn);
				dispatchMessages(msn);
			}else{
				msn->listeningState = 0;
			}
		}else if (msn->listeningState){
			if (isHostAlive(msn))
				lSleep(40);
			else
				msn->listeningState = 0;
		}
	}while(msn->listeningState);
	
	TMSNLIST_LOCK();
	msn->authed = 0;
	msn->status = MSNERROR_DISCONNECTED;
	if (msn->connection.socket != SOCKET_ERROR)
		msn_closeSocket(msn->connection.socket);
	msn->connection.socket = SOCKET_ERROR;
	msn->listeningState = 0;
	TMSNLIST_UNLOCK();
	
	//printf("msn connectionHandler exited\n");
	_endthreadex(1);
	return 1;
}

static int msn_CreateConnection (TAMSN *msn)
{
	msn->status = MSNERROR_CONNECTINGAMSN;
	int port2 = msn_GetConnectionPort(msn, amsnPort);
	if (!port2){
		msn->status = MSNERROR_REMOTEDISABLED;
		return 0;
	}else if (port2 < 0){
		msn->status = MSNERROR_REMOTEDISABLED;
		return 0;
	}

	msn->config.displayMode = 0;
	msn->display.startLine = 0;
	msn->authed = 0;
	msn->list.total = 0;
	msn->cmds.msgTotal = 0;
	msn->cmds.queueTotal = 32;
	msn->cmds.incomplete.size = 0;
	msn->cmds.incomplete.text[0] = 0;
	msn->connection.lastMessageSize = 0;
	msn->connection.recvTime = GetTickCount();

	msn->connection.socket = msn_connectTo(amsnAddress, port2);
	if (msn->connection.socket != SOCKET_ERROR){
		int retry = 20;
		do{
			msn->authed = msn_Authenticate(msn->connection.socket, msn->account.remotePassword);
			if (!msn->authed)
				lSleep(10);
		}while(--retry && !msn->authed);
		
		if (msn->authed){
			TMSNLIST_LOCK();
			msn->listeningState = 1;
			unsigned int threadID = 0;
			haMSNConn = (HANDLE)_beginthreadex(NULL, 0, (void*)connectionHandler, msn, 0, &threadID);
			TMSNLIST_UNLOCK();
			return 1;
		}else{
			msn->status = MSNERROR_AUTHFAILED;
		}
	}else{
		msn->status = MSNERROR_REMOTEDISABLED;
	}
	return 0;
}

static int aMSN_OvrTimeoutToIndex (int time)
{
	//printf("%i\n",time);
	int i;
	for (i = 0; i < 14; i++){
		if (time <= aMSNTimeoutTable[i])
			return i;
	}
	return 0;
}

int aMSN_GetOverlayFrame ()
{
	return msnOverlay1.distination;
}

void aMSN_SetOverlayFrame (int frameid)
{
	msnOverlay1.distination = frameid;
	msnOverlay2.distination = frameid;
	msnOverlay3.distination = frameid;
	msnOverlay4.distination = frameid;
}

int aMSN_GetInputKey ()
{
	//printf("aMSN_GetInputKey(): %i\n",g_msn.inputKey);
	return g_msn.inputKey;
}

void aMSN_SetInputKey (int keyconfig)
{
	g_msn.inputKey = keyconfig;
	//printf("aMSN_SetInputKey(): %i\n",g_msn.inputKey);
}

int aMSN_GetMailOvrTimeout ()
{
	return msnOverlay2.requestedTime;
}

int aMSN_GetIconOvrTimeout ()
{
	return msnOverlay1.requestedTime;
}

void aMSN_SetMailOvrTimeout (unsigned int time)
{
	time *= 1000;
	if (!time)
		time = 0xFFFFFFF;
	msnOverlay2.requestedTime = time;
	msnOverlay4.requestedTime = time;
}

void aMSN_SetIconOvrTimeout (unsigned int time)
{
	time *= 1000;
	if (!time)
		time = 0xFFFFFFF;
	msnOverlay1.requestedTime = time;
	msnOverlay3.requestedTime = time;
}

int msnInit (TFRAME *frame, void *userPtr)
{
	if (msnInitOnce) return 1;
	if (userPtr == NULL) return 0;
	TWINAMP *wa = (TWINAMP*)userPtr;
	
	TMSNLIST_LOCK_CREATE();
	memcpy(&g_msn.account, &wa->amsn, sizeof(TAMSNACCOUNT));

	msnOverlay1.frame = lNewFrame(frame->hw, 8, 8, LFRM_BPP_1);
	if (!lLoadImage(msnOverlay1.frame, ICON_MSG)){
		wprintf(L"%s not found\n", ICON_MSG);
		//msnOverlay1.frame = lNewString(frame->hw, NEWSTRINGTYPE, 0, LFT_COMICSANSMS7X8, "M");
	}

	msnOverlay1.ownerSetsT0 = 0;
	msnOverlay1.t0 = 0;
	msnOverlay1.requestedTime = 300*1000;
	msnOverlay1.x = frame->width+10; // render to bottom right
	msnOverlay1.y = frame->width+10;
	msnOverlay1.ownerCleans = 1;
	msnOverlay1.copyMode = LCASS_CPY;
	msnOverlay1.distination = DISF_ACTIVEFRAME;
	msnOverlay1.reuse = 1;
	msnOverlay1.status = 0;
	
	msnOverlay2.ownerSetsT0 = 0;
	msnOverlay2.t0 = 0;
	msnOverlay2.requestedTime = 7*1000;
	msnOverlay2.x = frame->width+10; // render to bottom right
	msnOverlay2.y = 0; // let wvs calculate x,y
	msnOverlay2.frame = NULL;
	msnOverlay2.ownerCleans = 0;
	msnOverlay2.copyMode = LCASS_CPY;
	msnOverlay2.distination = DISF_ACTIVEFRAME;
	msnOverlay2.reuse = 1;
	msnOverlay2.status = 0;

	msnOverlay3.frame = lNewFrame(frame->hw, 8, 8, LFRM_BPP_1);
	if (!lLoadImage(msnOverlay3.frame, ICON_MAIL)){
		wprintf(L"%s not found\n", ICON_MAIL);
		//msnOverlay3.frame = lNewString(frame->hw, NEWSTRINGTYPE, 0, LFT_COMICSANSMS7X8, "M");
	}

	msnOverlay3.ownerSetsT0 = 0;
	msnOverlay3.t0 = 0;
	msnOverlay3.requestedTime = 30*1000;
	msnOverlay3.x = frame->width+10;
	msnOverlay3.y = frame->height+10; // render to bottom right
	msnOverlay3.ownerCleans = 1;
	msnOverlay3.copyMode = LCASS_CPY;
	msnOverlay3.distination = DISF_ACTIVEFRAME;
	msnOverlay3.reuse = 1;
	msnOverlay3.status = 0;

	msnOverlay4.frame = NULL;
	msnOverlay4.ownerSetsT0 = 0;
	msnOverlay4.t0 = 0;
	msnOverlay4.requestedTime = msnOverlay2.requestedTime;
	msnOverlay4.x = frame->width+10; // render to bottom right
	msnOverlay4.y = 9; //frame->width+10; // let wvs calculate x,y
	msnOverlay4.ownerCleans = 0;
	msnOverlay4.copyMode = LCASS_CPY;
	msnOverlay4.distination = DISF_ACTIVEFRAME;
	msnOverlay4.reuse = 1;
	msnOverlay4.status = 0;
	
	
	/* default/failsafe config values
	g_msn.autoConnect = 1;
	g_msn.inputKey = 0;
	g_msn.nameFormat = 2;
	g_msn.emoticons = 1;
	g_msn.config.displayMode = 0;
	g_msn.encoding = 0;
	*/

	g_msn.mailTotal = -1;
	g_msn.wa = wa;
	g_msn.listeningState = 0;
	g_msn.connection.recvBufferSize = MSNSIZE_RECVBUFFER;
	g_msn.connection.recvBuffer = (char*)calloc(sizeof(ubyte), g_msn.connection.recvBufferSize+8);
	if (g_msn.connection.recvBuffer == NULL){
		g_msn.status = MSNERROR_BADALLOC;
		msnInitOnce = 0;
		return 0;
	}else{
		g_msn.status = MSNERROR_NOTCONNECTED;
		msnInitOnce = 1;
		return 1;
	}
}

void aMSN_DoAutoConnect (TWINAMP *wa/*, TFRAME *frame*/)
{
/*	if (!g_msn.listeningState && g_msn.autoConnect){
		if (!msnInitOnce){
			if (!msnInit(frame, wa))
				return;
		}
		msn_CreateConnection(&g_msn);
	}
*/

	if (!g_msn.listeningState && g_msn.autoConnect)
		msn_CreateConnection(&g_msn);
}

void msnCleanup ()
{
	if (!msnInitOnce) return;
	msnInitOnce = 0;

	g_msn.listeningState = 0;
	if (g_msn.connection.socket != SOCKET_ERROR)
		msn_closeSocket(g_msn.connection.socket);
	g_msn.connection.socket = SOCKET_ERROR;
	lSleep(50);
	if (haMSNConn){
		//printf("waiting for connectionHandler\n");
		WaitForSingleObject(haMSNConn, 5000);
		//printf("waiting for connectionHandler done\n");
		CloseHandle(haMSNConn);
	}
	haMSNConn = NULL;
	
	TMSNLIST_LOCK();	
	g_msn.authed = 0;
	g_msn.status = MSNERROR_NOTCONNECTED;
	msn_deleteAllusers(&g_msn);
	lSleep(1);
	TMSNLIST_LOCK_DELETE();

	if (g_msn.connection.recvBuffer)
		free(g_msn.connection.recvBuffer);
	g_msn.connection.recvBuffer = NULL;

	freeInbox(&g_msn);
	freeContactList(&g_msn, MSN_CONTACTLIST_FL);
	freeContactList(&g_msn, MSN_CONTACTLIST_AL);
	freeContactList(&g_msn, MSN_CONTACTLIST_BL);
	freeContactList(&g_msn, MSN_CONTACTLIST_PL);
	freeContactList(&g_msn, MSN_CONTACTLIST_RL);
	freeContactList(&g_msn, MSN_CONTACTLIST_ALL);
	
	if (msnOverlay1.frame)
		lDeleteFrame(msnOverlay1.frame);
	msnOverlay1.frame = NULL;

	if (msnOverlay2.frame)
		lDeleteFrame(msnOverlay2.frame);
	msnOverlay2.frame = NULL;
	
	if (msnOverlay3.frame)
		lDeleteFrame(msnOverlay3.frame);
	msnOverlay3.frame = NULL;

	if (msnOverlay4.frame)
		lDeleteFrame(msnOverlay4.frame);
	msnOverlay4.frame = NULL;
}

// get used slots, one slot per user.
static int getTotalUsers (TAMSN *msn)
{
	return msn->list.total;
}

static int getTotalOnlineUsers (TAMSN *msn)
{
	int total = 0;
	int i;
	for (i = 0; i < getTotalUsers(msn); i++){
		if (msn->list.user[i]->state)
			total++;
	}
	return total;
}

static char *formatName (TAMSN *msn, char *name)
{
	switch(msn->nameFormat){
		case 0: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "%s: ", name); break;
		case 1: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "%s, ", name); break;
		case 2: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "(%s) ", name); break;
		case 3: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "<%s> ", name); break;
		case 4: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "{%s} ", name); break;
		case 5: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "[%s] ", name); break;
		case 6: snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "%s - ", name); break;
		default :
			//printf("invalid name format specified %i\n",msn->nameFormat);
			snprintf(msn->nameBuffer, MSNSIZE_USERNAME, "%s ", name);
	}
	return msn->nameBuffer;
}

static int processEmoticons (char *src, char *des, size_t srclen)
{
	//if (!srclen) return 0;
	char *pbuffer = des;
	int i;

	for (i = 0; i < srclen; i++){
		if (src[i] == ':' && src[i+1] == ')'){
			*pbuffer++ = 3;
			i++;
		}else if (src[i] == ':' && src[i+1] == '('){
			*pbuffer++ = 4;
			i++;
		}else{
			*pbuffer++ = src[i];
		}
	}
	*pbuffer++ = 0;
	return (pbuffer-des)-1;
}

static int drawUserChat (TAMSN *msn, TFRAME *frame, int userIndex)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	int origin = 0;
	u64 stamp = 1;
	char *user = NULL;
	char *message = NULL;
	char *tmpUser;
	char buffer[1024];
	char *pbuffer = buffer;
	const int ptflags = PF_CLIPWRAP|PF_WORDWRAP|PF_NEWLINE|PF_RESETX;
	msn->display.totalLines = 0;


	if (!msn->emoticons)
		pbuffer = msn->list.user[userIndex]->username;
	else
		processEmoticons(msn->list.user[userIndex]->username, pbuffer, strlen(msn->list.user[userIndex]->username));
 
	if (!msn->config.showStatus){
		lPrintEx(frame, &rect, LFTW_5x7, PF_NEWLINE|PF_RESETX, LPRT_CPY,"%s (%s)",\
		 pbuffer, msn->list.user[userIndex]->email);
	}else{
		lPrintEx(frame, &rect, LFTW_5x7, PF_NEWLINE|PF_RESETX, LPRT_CPY,"%s: %s (%s)",\
		  msn->list.user[userIndex]->status, pbuffer, msn->list.user[userIndex]->email);
	}
	rect.sy = ++rect.ey;

	do{
		stamp = msn_getNextMessageUserIndex(msn->list.user[userIndex], stamp, &user, &message, &origin);
		if (user){
			if (msn->display.totalLines++ >= msn->display.startLine){
				tmpUser = (origin == MSNMSG_ORIGINFROM) ? formatName(msn, user) : formatName(msn, msn->me.username);
				if (msn->emoticons){
					int len = processEmoticons(tmpUser, buffer, strlen(tmpUser));
					processEmoticons(message, buffer+len, strlen(message));
					lPrintEx(frame, &rect, LFTW_5x7, ptflags|PF_DONTFORMATBUFFER, LPRT_CPY,buffer);
				}else{
					lPrintEx(frame, &rect, LFTW_5x7, ptflags, LPRT_CPY,"%s%s", tmpUser, message);
				}
			}
		}
	}while(user != NULL /*&& rect.ey < frame->height-4*/);
	
	lDrawLineDotted(frame, 0, rect.ey+3, frame->width-1, rect.ey+3, LSP_SET);
	if (msn->display.startLine > msn->display.totalLines)
		msn->display.startLine = msn->display.totalLines-1;
	return 1;
}

static int drawAllUsersChat (TAMSN *msn, TFRAME *frame)
{
	TLPRINTR rect = {0, 0, frame->width-1, frame->height-1, 0, 0, 0, 0};
	int i;
	u64 stamp = 1;
	char *user = NULL;
	char *message = NULL;
	int origin = 0;
	msn->display.totalLines = 0;
	char *tmpUser;
	int charswritten = 0;
	const int ptflags = PF_CLIPWRAP|PF_WORDWRAP|PF_NEWLINE|PF_RESETX;
	char buffer[1024];
						
	int listwidth;
	if (!msn->config.showStatus)
		listwidth = 50;
	else
		listwidth = 76;

	if (msn->config.displayMode){
		rect.bx1 = frame->width-listwidth;
		rect.bx2 = frame->width-1;
	}

	for (i = 0; i < msn->list.total; i++){
		if (msn->display.totalLines++ >= msn->display.startLine){
			if (msn->list.user[i]->state){
				*buffer = 0;
				charswritten = 0;
				if (msn->config.showStatus)
					charswritten = sprintf(buffer, "%i:%s: ", i+1, msn->list.user[i]->status);
				
				if (!msn->emoticons)	
					charswritten += sprintf(buffer+charswritten, msn->list.user[i]->username);
				else
					charswritten += processEmoticons(msn->list.user[i]->username, buffer+charswritten, strlen(msn->list.user[i]->username));
				
				if (!msn->config.displayMode && msn->config.showStatus)
					charswritten += sprintf(buffer+charswritten, ": %s", msn->list.user[i]->email);

				lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_NEWLINE|PF_RESETX, LPRT_CPY, buffer);
			}
		}
	}
		
	if (!msn->config.displayMode){
		if (msn->display.totalLines++ > msn->display.startLine){
			lDrawLine(frame, 0, rect.ey+2, frame->width-1, rect.ey+2, LSP_SET);
			lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPWRAP, LPRT_CPY,"\n");
		}
	}else{
		msn->display.totalLines = 0;
		rect.bx1 = 0;
		rect.bx2 = frame->width-listwidth-2;
		rect.sy = rect.ey = 0;
		lDrawLine(frame, rect.bx2, 0, rect.bx2, frame->height-1, LSP_SET);
	}

	do{
		stamp = msn_getNextMessageTime(&msn->list, stamp, &user, &message, &origin);
		if (user){
			if (msn->display.totalLines++ >= msn->display.startLine){
				//tmpUser = (origin == MSNMSG_ORIGINFROM) ? user : msn->me.username;
				tmpUser = (origin == MSNMSG_ORIGINFROM) ? formatName(msn, user) : formatName(msn, msn->me.username);
				if (msn->emoticons){
					int len = processEmoticons(tmpUser, buffer, strlen(tmpUser));
					processEmoticons(message, buffer+len, strlen(message));
					lPrintEx(frame, &rect, LFTW_5x7, ptflags|PF_DONTFORMATBUFFER, LPRT_CPY, buffer);
				}else{
					lPrintEx(frame, &rect, LFTW_5x7, ptflags, LPRT_CPY,"%s%s", tmpUser, message);
				}
			}
		}
	}while(user != NULL /*&& rect.ey < frame->height-4*/);
	
	lDrawLineDotted(frame, 0, rect.ey+3, rect.bx2-1, rect.ey+3, LSP_SET);
	return 1;
}

static void drawContactList (TAMSN *msn, TFRAME *frame, int listid)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	msn->display.totalLines = 0;
	
	if (!msn->display.startLine){
		int enc = lSetCharacterEncoding(msn->wa->hw, msn->wa->lang->enc);
		lPrintEx(frame, &rect, LFTW_5x7, PF_WORDWRAP|PF_CLIPWRAP|PF_CLIPTEXTV, LPRT_CPY, "%s (%i)",\
		  lng_getString(msn->wa->lang, LNG_MSN_CONTACTLIST+listid), msn->cl[listid].total);
		lSetCharacterEncoding(msn->wa->hw, enc);
		lDrawLine(frame, 0, rect.ey+2, frame->width-1, rect.ey+2, LSP_SET);
		lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPWRAP, LPRT_CPY,"\n");
		msn->display.totalLines++;
	}
	
	int i;
	for(i = 0; i< msn->cl[listid].total && rect.ey < frame->height-4; i++){
		if (msn->display.totalLines++ >= msn->display.startLine){
			lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_NEWLINE|PF_RESETX, LPRT_CPY,\
		 	  /*"%i:%s", i+1, msn->cl[listid].user[i].email);*/\
		 	  msn->cl[listid].user[i].email);
		}
	}
	
}

static void drawInbox (TAMSN *msn, TFRAME *frame)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	msn->display.totalLines = 0;
	
	if (!msn->display.startLine){
		int enc = lSetCharacterEncoding(msn->wa->hw, msn->wa->lang->enc);
		lPrintEx(frame, &rect, LFTW_5x7, PF_WORDWRAP|PF_CLIPWRAP|PF_CLIPTEXTV, LPRT_CPY, "%s (%i)",\
		   lng_getString(msn->wa->lang, LNG_MSN_INBOX), msn->mailTotal);
		lSetCharacterEncoding(msn->wa->hw, enc);
		lDrawLine(frame, 0, rect.ey+2, frame->width-1, rect.ey+2, LSP_SET);
		lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPWRAP, LPRT_CPY,"\n");
		msn->display.totalLines++;
	}
	
	int i;
	for(i = 0; i < msn->inbox.total && rect.ey < frame->height-4; i++){
		if (msn->display.totalLines++ >= msn->display.startLine){
			lPrintEx(frame, &rect, LFTW_5x7, PF_CLIPDRAW|PF_NEWLINE|PF_RESETX, LPRT_CPY,\
			  "%i:%s (%s)", i+1, msn->inbox.mail[i].name, msn->inbox.mail[i].email);
		}
	}
}

int renderMSN (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (!msnInitOnce){
		if (!msnInit(frame, wa))
			return 0;
	}
	TAMSN *msn = &g_msn;
	
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	int space = lGetFontLineSpacing(wa->hw, LFTW_5x7);
	lSetFontLineSpacing(wa->hw, LFTW_5x7, 0);
	lSetCharacterEncoding(msn->wa->hw, g_msn.encoding);
	lClearFrame(frame);

	TMSNLIST_LOCK();
	if (msn->authed && getTotalOnlineUsers(msn)){
		if (!msn->config.displayPage){
			if (msn->config.displayMode < 2){
				drawAllUsersChat(msn, frame);
			}else if (msn->config.displayMode < 8){
				drawContactList(msn, frame, msn->config.displayMode-2);
			}else if (msn->config.displayMode == 8){
				drawInbox(msn, frame);
			}
		}else{
			drawUserChat(msn, frame, msn->config.displayPage-1);
		}
	}else{
		lPrintEx(frame, &rect, LFTW_SNAP, PF_DONTFORMATBUFFER|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP|PF_RESETXY,\
		  LPRT_CPY, (char*)msnemsg[msn->status]);
	}

	if (kbHookGetStatus()){
		if (msn->input.iOffset > msn->input.caretPos-1)
			msn->input.iOffset = msn->input.caretPos;
		addCaret(&msn->input, msn->input.workingBuffer, msn->input.caretBuffer, MSNSIZE_INPUTBUFFER-1);
		drawEditBox(&msn->input, frame, 0, 7, msn->input.caretBuffer, &msn->input.iOffset);
		drawCharTotal(&msn->input, frame);
	}
	TMSNLIST_UNLOCK();
	lSetFontLineSpacing(wa->hw, LFTW_5x7, space);	
	return 1;
}

int displayInput_MSN (TWINAMP *wa, int key, void *data)
{
	if (!msnInitOnce) return 0;
	
	TAMSN *msn = &g_msn;
	//dbprintf("msn key: %i %i \n",key, isKeyPressed(VK_OEM_2));
	
	msnOverlay1.status = 0; // switch off message notification on key press
	if (msnOverlay3.status)
		msnOverlay3.x = frame->width+10;
	
	if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_CLOCKWISE){
		TMSNLIST_LOCK();
		if (++msn->config.displayPage > getTotalUsers(msn))
			msn->config.displayPage = 0;
		TMSNLIST_UNLOCK();
		renderFrame(wa, DISF_MSN);	
			
	}else if (isKeyPressed(VK_RSHIFT) && key == G15_WHEEL_ANTICLOCKWISE){
		TMSNLIST_LOCK();
		if (--msn->config.displayPage < 0)
			msn->config.displayPage = getTotalUsers(msn);
		TMSNLIST_UNLOCK();
		renderFrame(wa, DISF_MSN);

	}else if (aMSN_GetInputKey() == 0 && isKeyPressed(VK_CONTROL) && key == 10 /*ctrl+enter*/ && !kbHookGetStatus()){
		kbHookOn();
		renderFrame(wa, DISF_MSN);
		
	}else if (aMSN_GetInputKey() == 1 && isKeyPressed(VK_CONTROL) && key == 127 /*ctrl+back*/ && !kbHookGetStatus()){
		kbHookOn();
		renderFrame(wa, DISF_MSN);	
		
	}else if (aMSN_GetInputKey() == 2 && isKeyPressed(VK_RCONTROL) && isKeyPressed(VK_RSHIFT) && !kbHookGetStatus()){
		kbHookOn();
		renderFrame(wa, DISF_MSN);
		
	}else if (aMSN_GetInputKey() == 3 && isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_4 && !kbHookGetStatus()){
		kbHookOn();
		renderFrame(wa, DISF_MSN);

	}else if (aMSN_GetInputKey() == 4 && isKeyPressed(VK_CONTROL) && key == VK_OEM_2 && !kbHookGetStatus()){
		kbHookOn();
		renderFrame(wa, DISF_MSN);

	}else if (isKeyPressed(VK_LSHIFT) && !kbHookGetStatus()){
		return -1;

	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		if (--msn->display.startLine < 0)
			msn->display.startLine = 0;
		renderFrame(wa, DISF_MSN);
		
	}else if (key == G15_WHEEL_CLOCKWISE){
		if (msn->display.startLine < msn->display.totalLines-1)
			msn->display.startLine++;
		renderFrame(wa, DISF_MSN);
		
	}else if (isKeyPressed(VK_CONTROL) && key == G15_SOFTKEY_3 && !kbHookGetStatus()){
		if (msn->listeningState){
			msn->listeningState = 0;
		}else{
			msn_CreateConnection(msn);
		}
	}else if (kbHookGetStatus()){
		if (key == VK_LSHIFT || key == VK_SHIFT || key == VK_RSHIFT)
			return 0;

		int ret = editBoxInputProc(&msn->input, wa->hook.hwnd, key, msn->emoticons);
		if (!ret){	// we don't have this key
			return 0;

		}else if (ret == 1){ // key handled, update frame
			renderFrame(wa, DISF_MSN);
			return 0;
			
		}else if (ret == -1){ // edit box closed without hitting enter
			renderFrame(wa, DISF_MSN);
			return -1;
			
		}else if (ret == 2){ // edit box closed via return/enter
			TMSNLIST_LOCK();
			int ret = msn_SendMsnUserMsg(msn, msn->input.workingBuffer);
			TMSNLIST_UNLOCK();
			
			if (ret > 0){ 	// message has been sent
				addWorkingBuffer(&msn->input);
				nextHistoryBuffer(&msn->input);

				int u = atoi(msn->input.workingBuffer);
				clearWorkingBuffer(&msn->input);
				sprintf(msn->input.workingBuffer, "%i:", u);
				msn->input.caretPos = msn->input.tKeys = strlen(msn->input.workingBuffer);
			}else{
				if (msn->config.displayPage){
					TMSNLIST_LOCK();
					ret = msn_SendMsnUserMsgId(msn, msn->config.displayPage, msn->input.workingBuffer);
					TMSNLIST_UNLOCK();
					if (ret > 0){
						addWorkingBuffer(&msn->input);
						nextHistoryBuffer(&msn->input);
						clearWorkingBuffer(&msn->input);
					}
				}
			}
			renderFrame(wa, DISF_MSN);
		}

	}else if (key == G15_SOFTKEY_3){
		if (++msn->config.displayMode > 8)
			msn->config.displayMode = 0;
		msn->config.displayPage = 0;
		msn->display.startLine = 0;
		renderFrame(wa, DISF_MSN);

	}else if (key == G15_SOFTKEY_4){
		msn->config.showStatus ^= 1;
		renderFrame(wa, DISF_MSN);
	}else{
		return -1;
	}
	return 0;
}

