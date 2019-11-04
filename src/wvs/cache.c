
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


MYLCD_EXPORT u64 MYLCD_APICALL rdtsc();


#include "utf8.c"



static int isWideStrCmd (int cmd)
{
	return (cmd == CMD_GET_TRACKTITLEW || cmd == CMD_GET_TRACKFILENAMEW || cmd == CMD_GET_METADATAW);
}

static int cache_InsertItem (TCACHEITEM *item, void *buffer, int type, int playlist, int track)
{
	item->valid = 1;
	item->playlist = playlist;
	item->track = track;
	item->type = type;
	item->time = rdtsc();
	
	if (item->buffer != NULL)
		free(item->buffer);
		
	if (isWideStrCmd(type)){
		size_t wlen = wcslen(buffer);
		if (wlen){
			char *utf8str = calloc(sizeof(char), wlen*(sizeof(wchar_t)<<2));
			if (utf8str){
				utf8_wcstombs((ubyte*)utf8str, wlen*(sizeof(wchar_t)<<2), buffer, wlen);
				item->buffer = strdup(utf8str);
				free(utf8str);
				return (item->buffer != NULL);
			}
		}
		item->valid = 0;
		return 0;
	}else{
		item->buffer = strdup(buffer);
		return (item->buffer != NULL);
	}
}

static TCACHEITEM *cache_GetOldestItem (TDATACACHE *cache)
{
	TCACHEITEM *item = cache->item;
	TCACHEITEM *old = item;
	int i;
	for (i = 0; i < CACHEITEMSIZE; i++, item++){
		if (item->time < old->time && item->time)
			old = item;
	}
	return old;
}

static void cache_FreeAll (TDATACACHE *cache)
{
	TCACHEITEM *item = cache->item;
	int i;

	for (i = 0; i < CACHEITEMSIZE; i++, item++){
		if (item->valid){
			item->valid = 0;
			if (item->buffer != NULL)
				free(item->buffer);
			item->buffer = NULL;
			item->playlist = -1;
			item->track = -1;
			item->type = -1;
		}
	}
}

static void cache_Flush (TDATACACHE *cache)
{
	cache_FreeAll(cache);
}

static char *cache_FindString (TDATACACHE *cache, int type, int playlist, int track) 
{
	int i;
	TCACHEITEM *item = cache->item;

	// find duplicate, replace
	for (i = 0; i < CACHEITEMSIZE; i++, item++){
		if (item->valid){
			if (item->playlist == playlist && item->track == track && item->type == type){
				return item->buffer;
			}
		}
	}
	return NULL;
}


static int cache_AddString (TDATACACHE *cache, void *buffer, int type, int playlist, int track) 
{
	int i;
	TCACHEITEM *item = cache->item;

	// find duplicate, replace
	for (i = 0; i < CACHEITEMSIZE; i++, item++){
		if (item->valid){
			if (item->playlist == playlist && item->track == track && item->type == type){
				cache_InsertItem(item, buffer, type, playlist, track);
				//printf("insert dup item %i: %i %i %i\n", i, type, playlist, track);
				return 1;
			}
		}
	}
		
	item = cache->item;
	// check if theres a free slot then add
	for (i = 0; i < CACHEITEMSIZE; i++, item++){
		if (!item->valid){
			cache_InsertItem(item, buffer, type, playlist, track);
			//printf("insert free item %i: %i %i %i\n", i, type, playlist, track);
			return 1;
		}
	}

	// replace with oldest item
	//printf("insert old item %i %i %i\n", type, playlist, track);
	return cache_InsertItem(cache_GetOldestItem(cache), buffer, type, playlist, track);
}

