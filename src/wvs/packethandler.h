
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



typedef struct _TWVSPACKET {
	union{
		void *buffer;
		TMPGENTRACKINFO *gen;
		TMPCMDWAVE *wave;
		TMPCMDEQ *eq;
		TMPCMD *cmd;
	};
	int isValid;
	unsigned int id;			//id=0: invalid packet and buffer is NULL'd
	size_t bufferSize;			
	struct _TWVSPACKET *next;
}TWVSPACKET;


#define PACKETQUEUESIZE 128

typedef struct{
	TWVSPACKET *list;
	TWVSPACKET *root;
	TWVSPACKET *last;
	
	int packetsTotal;	// total unprocessed packets
	int slotsTotal;		// total slots
	int slotsRemaining;	// total slots remaining
	unsigned int idSrc;
}TWVSPAKCACHE;

