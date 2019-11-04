
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

#ifndef _PLUGINABOUT_H_
#define _PLUGINABOUT_H_


static void plugin_about (TMP *mpp, char *title)
{
	if (mpp == NULL || title == NULL)
		return;
	
	char message[2048];
	memset(message, 0, sizeof(message));
	
	if (mpp->net.serverState)
		sprintf(message, "Listening on 127.0.0.1:%i\n",TCPPORT);
	
	if (!mpp->net.clientState){
		sprintf(message, "%sAwaiting connection...      \n", message);
	}else{
		if (inet_ntoa(mpp->net.sockaddrin.sin_addr) != NULL)
			sprintf(message, "%sLast packet received from: %s:%i ", message, inet_ntoa(mpp->net.sockaddrin.sin_addr), ntohs(mpp->net.sockaddrin.sin_port));
	}

	sprintf(message, "%s\nPackets sent: %i\nPackets received: %i\n\n%s\n0x%X %i %i",\
	  message, mpp->net.sendCt, mpp->net.readCt,\
	  MY_NAME, MY_VERSION1, MY_VERSION2, MY_VERSION3);

	if (IPPROTOCOL > 0)
		sprintf(message, "%s TCP", message);
	else
		sprintf(message, "%s UDP", message);
	
	MessageBox(mpp->hwndParent, message, title, MB_OK);
	
}


#endif

