
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



static int cache_create (TWVSPAKCACHE *cache)
{
	cache->list = (TWVSPACKET*)calloc(PACKETQUEUESIZE, sizeof(TWVSPACKET));
	if (cache->list == NULL)
		return 0;
	cache->slotsTotal = PACKETQUEUESIZE;
	cache->slotsRemaining = cache->slotsTotal;
	cache->root = NULL;
	cache->last = NULL;
	cache->idSrc = 0;
	cache->packetsTotal = 0;
	return (cache->list != NULL);
}

static void cache_resize (TWVSPAKCACHE *cache, int newTotal)
{
	cache->list = (TWVSPACKET*)realloc(cache->list, (1+newTotal) * sizeof(TWVSPACKET));
	if (cache->list == NULL)
		return;

	int i;
	for (i = cache->slotsTotal; i < newTotal; i++)
		cache->list[i].isValid = 0;

	for (i = 0; i < newTotal; i++)
		cache->list[i].next = &cache->list[i+1];

	cache->root = &cache->list[0];
	cache->last = &cache->list[cache->slotsTotal-1];
	cache->slotsRemaining += (newTotal-cache->slotsTotal);
	cache->slotsTotal = newTotal;
}

static void cache_dump (TWVSPAKCACHE *cache)
{
	int i;
	for (i = 0; i < cache->slotsTotal; i++){
		if (cache->list[i].isValid){
			cache->list[i].isValid = 0;
			free(cache->list[i].buffer);
		}
	}

	cache->slotsRemaining = cache->slotsTotal;
	cache->last = NULL;
	cache->root = NULL;
	cache->packetsTotal = 0;
	return;
}

static void cache_delete (TWVSPAKCACHE *cache)
{
	cache_dump(cache);
	if (cache->list != NULL)
		free(cache->list);
	cache->list = NULL;
	return;
}

static unsigned int packet_createID (TWVSPAKCACHE *cache)
{
	return ++cache->idSrc;
}

static TWVSPACKET *packet_create (TWVSPAKCACHE *cache, size_t bufferSize)
{
	if (bufferSize < 8)
		bufferSize = 8;

	int i;
	for (i = 0; i < cache->slotsTotal; i++){
		if (!cache->list[i].isValid){
			cache->list[i].buffer = (void *)malloc(bufferSize);
			if (cache->list[i].buffer != NULL){
				cache->list[i].bufferSize = bufferSize;
				cache->list[i].id = packet_createID(cache);
				cache->list[i].next = NULL;
				cache->list[i].isValid = 1;
				cache->slotsRemaining--;
				cache->packetsTotal++;
				return &cache->list[i];
			}
		}
	}
	return NULL;
}

static int packet_add (TWVSPAKCACHE *cache, void *buffer, size_t bufferSize)
{
	if (cache->slotsRemaining < 1)
		cache_resize(cache, cache->slotsTotal+128);
	
	TWVSPACKET *packet = packet_create(cache, bufferSize);
	if (packet != NULL){
		memcpy(packet->buffer, buffer, bufferSize);
		
		if (cache->root == NULL)
			cache->root = packet;
			
		if (cache->last != NULL)
			cache->last->next = packet;
			
		cache->last = packet;
		if (packet == packet->next)
			packet->next = NULL;
		return 1;
	}

	return 0;
}

static int packet_delete (TWVSPAKCACHE *cache, TWVSPACKET *packet)
{
	if (packet != NULL){
		if (packet->isValid){
			packet->isValid = 0;
			cache->slotsRemaining++;
			cache->packetsTotal--;
			if (cache->root->id == packet->id)
				cache->root = packet->next;
			free(packet->buffer);
		}
	}
	return 1;
}

static TWVSPACKET *cache_getStream (TWVSPAKCACHE *cache)
{
	// root will always point to the beginning of the queue
	if (cache->packetsTotal){
		return cache->root;
	}else{
		printf("cache_getStream: no packets\n");
		return NULL;
	}
		
}
