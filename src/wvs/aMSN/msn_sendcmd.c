
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


/*
	Forward list		FL
	Reverse List		RL
	Accept List			AL
	Block list			BL
	Pending list 		PL
*/


int msn_SendCmdGetReverseList (TAMSN *msn)
{
	size_t btotal = 7;
	return msn_sendSocket(msn->connection.socket, "getRL\r\n", &btotal);
}

int msn_SendCmdGetAcceptList (TAMSN *msn)
{
	size_t btotal = 7;
	return msn_sendSocket(msn->connection.socket, "getAL\r\n", &btotal);
}

int msn_SendCmdGetBlockList (TAMSN *msn)
{
	size_t btotal = 7;
	return msn_sendSocket(msn->connection.socket, "getBL\r\n", &btotal);
}

int msn_SendCmdGetPendingList (TAMSN *msn)
{
	size_t btotal = 7;
	return msn_sendSocket(msn->connection.socket, "getPL\r\n", &btotal);
}

int msn_SendCmdGetContactList (TAMSN *msn)
{
	size_t btotal = 7;
	return msn_sendSocket(msn->connection.socket, "getFL\r\n", &btotal);
}

int msn_SendCmdGetMailDetails (TAMSN *msn)
{
	size_t btotal = 16;
	return msn_sendSocket(msn->connection.socket, "getmaildetails\r\n", &btotal);
}

int msn_SendCmdGetAllContacts (TAMSN *msn)
{
	size_t btotal = 16;
	return msn_sendSocket(msn->connection.socket, "getallcontacts\r\n", &btotal);
}

int msn_SendCmdCheckMail (TAMSN *msn)
{
	size_t btotal = 11;
	return msn_sendSocket(msn->connection.socket, "checkmail\r\n", &btotal);
}

int msn_SendCmdGetMailAlert (TAMSN *msn)
{
	size_t btotal = 17;
	return msn_sendSocket(msn->connection.socket, "getmailalertmsg\r\n", &btotal);
}

int msn_SendCmdConnect (TAMSN *msn)
{
	size_t btotal = 9;
	return msn_sendSocket(msn->connection.socket, "connect\r\n", &btotal);
}

int msn_SendCmdOnline (TAMSN *msn)
{
	size_t btotal = 8;
	return msn_sendSocket(msn->connection.socket, "online\r\n", &btotal);
}

int msn_SendCmdWhatIs (TAMSN *msn, char *email)
{
	size_t btotal = 7;
	msn_sendSocket(msn->connection.socket, "whatis ", &btotal);
	btotal = strlen(email);
	msn_sendSocket(msn->connection.socket, email, &btotal);
	btotal = 2;
	return msn_sendSocket(msn->connection.socket, "\r\n", &btotal);
}

int msn_SendCmdGetUserStatus (TAMSN *msn, char *email)
{
	size_t btotal = 14;
	msn_sendSocket(msn->connection.socket, "getuserstatus ", &btotal);
	btotal = strlen(email);
	msn_sendSocket(msn->connection.socket, email, &btotal);
	btotal = 2;
	return msn_sendSocket(msn->connection.socket, "\r\n", &btotal);
}

int msn_SendCmdPing (TAMSN *msn)
{
	size_t btotal = 6;
	return msn_sendSocket(msn->connection.socket, "ping\r\n", &btotal);
}

int msn_SendCmdGetState (TAMSN *msn)
{
	size_t btotal = 10;
	return msn_sendSocket(msn->connection.socket, "getstate\r\n", &btotal);
}

int msn_SendCmdLogout (TAMSN *msn)
{
	size_t btotal = 8;
	return msn_sendSocket(msn->connection.socket, "logout\r\n", &btotal);
}

int msn_SendCmdQuit (TAMSN *msn)
{
	size_t btotal = 6;
	return msn_sendSocket(msn->connection.socket, "quit\r\n", &btotal);
}

int msn_SendCmdStatus (TAMSN *msn)
{
	size_t btotal = 8;
	return msn_sendSocket(msn->connection.socket, "status\r\n", &btotal);
}

int msn_SendCmdMsg (TAMSN *msn, char *email, char *text)
{
	size_t btotal = 4;
	msn_sendSocket(msn->connection.socket, "msg ", &btotal);
	btotal = strlen(email);
	msn_sendSocket(msn->connection.socket, email, &btotal);
	btotal = 1;
	msn_sendSocket(msn->connection.socket, " ", &btotal);
	btotal = strlen(text);
	msn_sendSocket(msn->connection.socket, text, &btotal);
	btotal = 2;
	return msn_sendSocket(msn->connection.socket, "\r\n", &btotal);
}

