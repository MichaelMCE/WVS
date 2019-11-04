
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
supported tags:

[al:aa]		album
[ar:aa]		artist
[ti:aa]		title
[au:aa]		author,song written by
[by:aa]		lyric file (this) creator
[encoding:aa]	character encoding
[re:aa]		the player or the editor that created this LRC file
[ve:aa]		version of program

[offset:nn]	lyric time offset in ms, can be negative
*/

/*

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <windows.h>
#include <conio.h>

#include <mylcd.h>
#include "utf8.c"
*/

#define TAGTYPE_INVALID			0
#define TAGTYPE_ID				1
#define TAGTYPE_LINETIME		2


#define	MIN_C_SEC				1	/*  "[mm:ss]"        */
#define	MIN_C_SEC_C_HD3			2	/*  "[mm:ss:xxx]"    */
#define	MIN_C_SEC_P_HD2			3	/*  "[mm:ss.xx]"     */
#define	MIN_C_SEC_P_HD3			4	/*  "[mm:ss.xxx]"    */
#define	HR_C_MIN_C_SEC_C_HD3	5	/*  "[hh:mm:ss:xxx]" */
#define	HR_C_MIN_C_SEC_P_HD3	6	/*  "[hh:mm:ss.xxx]" */
#define	HR_C_MIN_C_SEC_P_HD2	7	/*  "[hh:mm:ss.xx]"  */
#define	HR_C_MIN_C_SEC			8	/*  "[hh:mm:ss]"     */



//#define MLRECORDFILE	L"G:\\Documents and Settings\\Administrator\\Application Data\\MiniLyrics\\MLLyrics.S2L"


//#define u64 __int64

#define RECORDLENGTH	MAX_PATH


typedef struct{
	wchar_t	mediaPath[RECORDLENGTH];
	wchar_t	lyricPath[RECORDLENGTH];
}TMLRECORD;

typedef struct{
	char *album;
	char *artist;
	char *title;
	char *author;
	char *by;
	char *encoding;
	char *re;
	char *version;
	int offset;
}TLTAGS;

typedef struct{
	char *lyric;
	int	len;		// length of lyric excluding terminator
}TLLINE;

typedef struct{
	int stamp;
	unsigned int lyricIdx;
}TLTIME;

typedef struct{
	TLLINE *lines;
	TLTIME *time;
	TLTAGS tag;
		
	int lyricTotal;
	int timeTotal;
	
	int format;

	wchar_t	*shortwPath;
	wchar_t	*longwPath;
	char *longUtf8Path;
}TLYRICS;

//u64 lof (FILE *stream);
TMLRECORD *getRecords (wchar_t *path, int *total);
TMLRECORD *findRecordByTrackShort (TMLRECORD *records, int total, TLYRICS *lrc);
TMLRECORD *findRecordByTrackLong (TMLRECORD *records, int total, TLYRICS *lrc);
char *getFileContents (wchar_t *path, size_t *flen);
int isDataUTF16 (char *data);
int addLyric (TLYRICS *lrc, int lyricLineIdx, char *lyric, int lyricLen);
int addLyricTime(TLYRICS *lrc, int timeTagIdx, int lyricLineIdx, int time);
char *getNextLyric (TLYRICS *lrc, int *stamp);
char *getFirstLyric (TLYRICS *lrc, int *stamp);
void deleteRecords (TMLRECORD *records);

TLYRICS *newLyricsW (wchar_t *wpath);
TLYRICS *newLyricsU (char *upath);


// source path is in utf8 format
TLYRICS *newLyricsU (char *upath)
{
	size_t slen = strlen(upath);
	if (slen){
		wchar_t *wpath = (wchar_t*)calloc(1, (slen*sizeof(wchar_t))+sizeof(wchar_t));
		if (wpath){
			if (utf8_to_utf16(upath, slen, wpath)){
				TLYRICS *lrc = newLyricsW(wpath);
				free(wpath);
				return lrc;
			}
			free(wpath);
		}
	}
	return NULL;
}

// source path is in wchar format
TLYRICS *newLyricsW (wchar_t *wpath)
{
	TLYRICS *lrc = NULL;
	if (wpath){
		lrc = (TLYRICS *)calloc(1, sizeof(TLYRICS));
		if (lrc){
			size_t wpathlen = wcslen(wpath);
			lrc->shortwPath = wcsdup(wpath);
			if (lrc->shortwPath){
				if (0 < GetShortPathNameW(wpath, lrc->shortwPath, wpathlen+sizeof(wchar_t))){
					size_t pathbuffer = wpathlen*6*sizeof(wchar_t);
					lrc->longwPath = calloc(1, pathbuffer);
					if (lrc->longwPath){
						size_t wlen = GetLongPathNameW(wpath, lrc->longwPath, pathbuffer/sizeof(wchar_t));
						if (wlen){
							lrc->longUtf8Path = calloc(sizeof(char), wlen*(sizeof(wchar_t)<<2));
							if (lrc->longUtf8Path){
								utf8_wcstombs((ubyte*)lrc->longUtf8Path, wlen*(sizeof(wchar_t)<<2), lrc->longwPath, wlen);
								return lrc;
							}
						}else{
							//if (GetLastError() == ERROR_PATH_NOT_FOUND || GetLastError() == ERROR_FILE_NOT_FOUND)
								//wprintf(L"lyric file not found: '%s'", wpath); dbprintf("\n");
						}
						free(lrc->longwPath);
					}
				}else{
					//if (GetLastError() == ERROR_PATH_NOT_FOUND || GetLastError() == ERROR_FILE_NOT_FOUND)
						//wprintf(L"lyric file not found: '%s'", wpath); dbprintf("\n");
				}
				free(lrc->shortwPath);
			}
			free(lrc);
		}
	}
	return NULL;
}

void deleteLyrics (TLYRICS *lrc)
{

	if (lrc == NULL) return;
	if (lrc->lines){
		int i;
		for (i = 0; i < lrc->lyricTotal; i++){
			if (lrc->lines[i].lyric){
				free(lrc->lines[i].lyric);
				lrc->lines[i].lyric = NULL;
			}
		}
		free(lrc->lines);
		lrc->lines = NULL;
	}
	if (lrc->time){
		free(lrc->time);
		lrc->time = NULL;
	}
	
	if (lrc->shortwPath)
		free(lrc->shortwPath);
	if (lrc->longwPath)
		free(lrc->longwPath);
	if (lrc->longUtf8Path)
		free(lrc->longUtf8Path);

		
	if (lrc->tag.album)
		free(lrc->tag.album);
	if (lrc->tag.artist)
		free(lrc->tag.artist);
	if (lrc->tag.title)
		free(lrc->tag.title);
	if (lrc->tag.author)
		free(lrc->tag.author);
	if (lrc->tag.by)
		free(lrc->tag.by);
	if (lrc->tag.re)
		free(lrc->tag.re);
	if (lrc->tag.version)
		free(lrc->tag.version);
	if (lrc->tag.encoding)
		free(lrc->tag.encoding);
	free(lrc);
}


int extractIdTag (TLYRICS *lrc, char *buffer, size_t strLen)
{

	if (buffer[0] != '[' || !isalpha(buffer[1]) || strLen < 6)
		return 0;
		
	char *pbuffer = buffer+sizeof(char);
	
	if (!strncmp(pbuffer, "al:", 3)){
		lrc->tag.album = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.album, &buffer[4], strLen-4);
		lrc->tag.album[strLen-1] = 0;
		
	}else if (!strncmp(pbuffer, "ar:", 3)){
		lrc->tag.artist = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.artist, &buffer[4], strLen-4);
		lrc->tag.artist[strLen-1] = 0;
				
	}else if (!strncmp(pbuffer, "ti:", 3)){
		lrc->tag.title = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.title, &buffer[4], strLen-4);
		lrc->tag.title[strLen-1] = 0;
				
	}else if (!strncmp(pbuffer, "au:", 3)){
		lrc->tag.author = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.author, &buffer[4], strLen-4);
		lrc->tag.author[strLen-1] = 0;
				
	}else if (!strncmp(pbuffer, "by:", 3)){
		lrc->tag.by = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.by, &buffer[4], strLen-4);
		lrc->tag.by[strLen-1] = 0;
						
	}else if (!strncmp(pbuffer, "re:", 3)){
		lrc->tag.re = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.re, &buffer[4], strLen-4);
		lrc->tag.re[strLen-1] = 0;
		
	}else if (!strncmp(pbuffer, "ve:", 3)){
		lrc->tag.version = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.version, &buffer[4], strLen-4);
		lrc->tag.version[strLen-1] = 0;
		
	}else if (!strncmp(pbuffer, "encoding:", 9)){
		lrc->tag.encoding = calloc(sizeof(char), strLen);
		memcpy(lrc->tag.encoding, &buffer[10], strLen-10);
		lrc->tag.encoding[strLen-1] = 0;
		
	}else if (!strncmp(pbuffer, "offset:", 7)){
		lrc->tag.offset = atoi(pbuffer+7);
		
	}else{
		return 0;
	}

	return 1;
}

int get_timetag_type (char *tag, int *has_tone)
{
#define HAS_TONE( off, ret ) \
	do { if( ( (isdigit(tag[ off ])) \
		&& (isdigit(tag[ off + 1 ])) ) \
		&& (isdigit(tag[ off + 2])) \
		&& (tag[ off + 3 ] == ']') ) \
	{ \
		if( has_tone != NULL ) \
			*has_tone = TRUE; \
		return ret; \
	} } while (0)

	if( has_tone != NULL )
		*has_tone = FALSE;

	if( (tag[0] == '[') && (isdigit(tag[1])) && (isdigit(tag[2]) )
		&& (tag[3] == ':') && (isdigit(tag[4])) && (isdigit(tag[5])) )
	{
		switch( tag[6] ) {
		case '.':
			if( (isdigit(tag[7])) && (isdigit(tag[8])) ) {
				switch( tag[9] ) {
				case ']':
					return MIN_C_SEC_P_HD2;
				case '-':
					HAS_TONE( 10, 3 );
					break;
				default:
					if( (isdigit(tag[9]))
						&& (tag[10] == ']') )
						return MIN_C_SEC_P_HD3;
					break;
				}
			}
			break;
		case ':':
			if( ! ( (isdigit(tag[7])) && (isdigit(tag[8])) ) )
				return 0;
			switch( tag[9] ) {
			case '.':
				if( (isdigit(tag[10]))
					&& (isdigit(tag[11])) )
				{
					switch( tag[12] ) {
					case ']':
						return HR_C_MIN_C_SEC_P_HD2;
					case '-':
						HAS_TONE( 13, 7 );
						break;
					default:
						if( (isdigit(tag[12]))
							&& (tag[13] == ']') )
							return HR_C_MIN_C_SEC_P_HD3;
						break;
					}
				}
				break;
			case ':':
				if( ( (isdigit(tag[10]))
					&& (isdigit(tag[11])) )
					&& (isdigit(tag[12])) )
				{
					if(tag[13] == ']')
						return HR_C_MIN_C_SEC_C_HD3;
					else if(tag[13] == '-')
						HAS_TONE( 14, 5 );
				}
				break;
			case ']':
				return HR_C_MIN_C_SEC;
			case '-':
				HAS_TONE( 7, 8 );
				break;
			default:
				if( isdigit(tag[9]) ) {
					if( tag[10] == ']' )
						return MIN_C_SEC_C_HD3;
					else if(tag[10] == '-')
						HAS_TONE( 11, 2 );
				}
				break;
			}
			break;
		case ']':
			return MIN_C_SEC;
		case '-':
			HAS_TONE( 7, 1 );
			break;
		}
	}
	return 0;
}

int getTimeType (char *timeTag)
{
	int has_tone = 0;
	int type = get_timetag_type(timeTag, &has_tone);
	//if (has_tone)
		//dbprintf("%i '%s' ###### TONE #######\n", type, timeTag);
	return type;
}

int extractTimeTag (char *timeTag)
{

	unsigned int hr = 0, min = 0, sec = 0, hd = 0;
	
	switch(getTimeType(timeTag)){
		case MIN_C_SEC:
			sscanf(timeTag, "[%u:%u.%u]", &min, &sec, &hd);
			break;
			
		case MIN_C_SEC_C_HD3:
			sscanf(timeTag, "[%u:%u:%u]", &min, &sec, &hd);
			break;
			
		case MIN_C_SEC_P_HD3:
		case MIN_C_SEC_P_HD2:
			sscanf(timeTag, "[%u:%u.%u]", &min, &sec, &hd);
			break;
			
		case HR_C_MIN_C_SEC_C_HD3:
			sscanf(timeTag, "[%u:%u:%u:%u]", &hr, &min, &sec, &hd);
			break;
			
		case HR_C_MIN_C_SEC_P_HD3:
		case HR_C_MIN_C_SEC_P_HD2:
			sscanf(timeTag, "[%u:%u:%u.%u]", &hr, &min, &sec, &hd);
			break;
			
		case HR_C_MIN_C_SEC:
			sscanf(timeTag, "[%u:%u:%u]", &hr, &min, &sec);
			break;
		default :
			dbprintf("invalid time tag: #%s#\n", timeTag);
			return 0;
	}

	if (hd < 100) hd *= 10;
	if (hd > 999) hd = 999;
	return (int)((hr * 60*60*1000) + (min * 60*1000) + (sec * 1000) + hd);
}

static int isOpenTag (char tag)
{
	return (tag == '[');
}

static int isCloseTag (char tag)
{
	return (tag == ']');
}

size_t getLrcTag (char *buffer, char *tag, size_t inTagLen)
{
	size_t tagLen = 0;
	
	while (++tagLen < inTagLen){
		if (buffer[tagLen] != 0){
			if (isCloseTag(buffer[tagLen])){
				strncpy(tag, &buffer[0], tagLen+1);
				tag[tagLen+1] = 0;
				return tagLen;
			}
		}
	}
	*tag = 0;
	return 0;
}

static int getTagType (char *tag)
{
	//dbprintf("getTagType() #%s#\n", tag);
	
	if (isdigit(tag[1]))
		return TAGTYPE_LINETIME;
	else if (isalpha(tag[1]))
		return TAGTYPE_ID;
	else
		return TAGTYPE_INVALID;
}

int parseLyricsBuffer (TLYRICS *lrc, char *buffer, size_t bufferLen)
{
	size_t bufferPos = 0;
	int state = 0;
	size_t tagLen;
	int ttime;
	int lyricLen = 0;
	int timeTagTotal = 0;
	int lyricLineTotal = 0;
	int lyricState = 0;
	char tag[512];		// one tag
	char lyric[4096];	// one line
	memset(lyric, 0, sizeof(lyric));
	
	
	lrc->lyricTotal = 0;
	lrc->timeTotal = 0;
	
	do{
		if (isOpenTag(buffer[bufferPos])){
			memset(tag, 0, 512);
			
			tagLen = getLrcTag(&buffer[bufferPos], tag, sizeof(tag));
			if (tagLen){
				if (getTagType(tag) == TAGTYPE_LINETIME){
					if (lyricLen && timeTagTotal){
						//dbprintf("lyric %i: %i, %i: #%s#\n\n", lyricLineTotal, bufferPos, lyricLen, lyric);
						addLyric(lrc, lyricLineTotal, lyric, lyricLen);
						memset(lyric, 0, sizeof(lyric));
						lyricLen = 0;
						lyricLineTotal++;
					}
					ttime = extractTimeTag(tag);
					//dbprintf("time tag %i for lyric %i: %s, %i  %i\n", timeTagTotal, lyricLineTotal, tag, ttime, getTimeType(tag));
					addLyricTime(lrc, timeTagTotal, lyricLineTotal, ttime);
					timeTagTotal++;
					lyricState = 1;
					
				}else if (getTagType(tag) == TAGTYPE_ID){
					extractIdTag(lrc, tag, tagLen);

				}
				bufferPos += tagLen;
			}else{
				dbprintf("getLrcTag(): invalid tag\n");
				state = 1;
			}
		}else{
		
			if (buffer[bufferPos] == '\r' || buffer[bufferPos] == '\n'){
				if (!lyricLen && lyricState && lyricLen < sizeof(lyric)-3){
					strcpy(lyric, "~");
					lyricLen = 1;
				}
				lyricState = 0;
			}
		
			if (lyricState && lyricLen < sizeof(lyric)-3){
				if ((buffer[bufferPos] == '\r' || buffer[bufferPos] == '\n') && !lyricLen){
					if (timeTagTotal){
						lyric[lyricLen++] = '~';
						lyric[lyricLen] = 0;
					}
				}else if (buffer[bufferPos] != '\r' && buffer[bufferPos] != '\n' && buffer[bufferPos] != 0){
					lyric[lyricLen++] = buffer[bufferPos];
					lyric[lyricLen] = 0;
				}
			}
		}
		bufferPos++;
	}while((bufferPos < bufferLen) && !state);

	if (timeTagTotal){
		if (!lyricLen){
			strcpy(lyric, "~");
			lyricLen = 1;
		}
		//dbprintf("lyric %i: %i, %i: #%s#\n\n", lyricLineTotal, bufferPos, lyricLen, lyric);
		addLyric(lrc, lyricLineTotal, lyric, lyricLen);
		lyricLen = 0;
		lyricLineTotal++;
	}

	#if 0
	//dbprintf("\nparseLyricsBuffer exited at offset %i of %i\n",bufferPos, bufferLen);
	//printf("%i time tags\n%i lyric lines\n", timeTagTotal, lyricLineTotal);

	if (lrc->tag.album)
		dbprintf("album: #%s#\n", lrc->tag.album);
		
	if (lrc->tag.artist)
		dbprintf("artist: #%s#\n", lrc->tag.artist);
		
	if (lrc->tag.title)
		dbprintf("title: #%s#\n", lrc->tag.title);
		
	if (lrc->tag.author)
		dbprintf("author: #%s#\n", lrc->tag.author);
		
	if (lrc->tag.by)
		dbprintf("by: #%s#\n", lrc->tag.by);
		
	if (lrc->tag.re)
		dbprintf("re: #%s#\n", lrc->tag.re);
		
	if (lrc->tag.version)
		dbprintf("version: #%s#\n", lrc->tag.version);
		
	if (lrc->tag.encoding)
		dbprintf("encoding: #%s#\n", lrc->tag.encoding);
		
	if (lrc->tag.offset)
		dbprintf("offset: %i\n", lrc->tag.offset);
		
	//dbprintf("\nlyrics added: %i\n", lrc->lyricTotal);
	//dbprintf("time stamps added: %i\n\n", lrc->timeTotal);
	
	#endif
	
	//int i;
	#if 0
	for (i = 0; i < lrc->lyricTotal; i++){
		dbprintf("%i: #%s#\n", i, lrc->lines[i].lyric);
		
	}
	#endif

	#if 0
	for (i = 0; i < lrc->timeTotal; i++){
		dbprintf("%i: %i, %i: %s\n", i, lrc->time[i].lyricIdx, lrc->time[i].stamp, lrc->lines[lrc->time[i].lyricIdx].lyric);
		
	}
	#endif

	return 1;
}

char *getLyricAlbum (TLYRICS *lrc)
{
	return lrc->tag.album;
}

char *getLyricArtist (TLYRICS *lrc)
{
	return lrc->tag.artist;
}

char *getLyricTitle (TLYRICS *lrc)
{
	return lrc->tag.title;
}
char *getLyricAuthor (TLYRICS *lrc)
{
	if (lrc->tag.author)
		return lrc->tag.author;
	else
		return lrc->tag.by;
}

char *getLyricRe (TLYRICS *lrc)
{
	return lrc->tag.re;
}

char *getLyricVersion (TLYRICS *lrc)
{
	return lrc->tag.version;
}

char *getLyricEncoding (TLYRICS *lrc)
{
	return lrc->tag.encoding;
}

int getLyricOffset (TLYRICS *lrc)
{
	return lrc->tag.offset;
}

int getLyricTotalLyrics (TLYRICS *lrc)
{
	return lrc->lyricTotal;
}

int getLyricTotalStamps (TLYRICS *lrc)
{
	return lrc->timeTotal;
}

char *getLyricInTime (TLYRICS *lrc, int *stamp)
{
	unsigned int s;
	char *lyric = NULL;
	int delta = 0;
	int minStamp = 999999999;
	int maxDelta= -999999999;

	for (s = 0; s < lrc->timeTotal; s++){
		//dbprintf("%u %u %i\n", lrc->time[s].stamp, *stamp, (int)lrc->time[s].stamp - *stamp);
		delta = lrc->time[s].stamp - *stamp;
		if (delta <= 0){
			if (delta > maxDelta){
				maxDelta = delta;
				minStamp = lrc->time[s].stamp;
				lyric = lrc->lines[lrc->time[s].lyricIdx].lyric;
			}
		}
	}
	if (minStamp != 999999999){
		*stamp = minStamp;
		return lyric;
	}else{
		*stamp = 0;
		return "~";
	}
}

char *getLyricByTimeRange (TLYRICS *lrc, int *stamp, int minStamp)
{
	unsigned int s;
	char *lyric = NULL;

	for (s = 0; s < lrc->timeTotal; s++){
		if (lrc->time[s].stamp > *stamp){
			if (lrc->time[s].stamp <= minStamp){
				minStamp = lrc->time[s].stamp;
				lyric = lrc->lines[lrc->time[s].lyricIdx].lyric;
			}
		}
	}
	if (minStamp != 999999999){
		*stamp = minStamp;
		return lyric;
	}else{
		*stamp = 0;
		return "~";
	}
}

char *getNextLyric (TLYRICS *lrc, int *stamp)
{
	return getLyricByTimeRange(lrc, stamp, 999999999);
}

char *getFirstLyric (TLYRICS *lrc, int *stamp)
{
	*stamp = 0;
	return getNextLyric(lrc, stamp);
}

int addLyricTime (TLYRICS *lrc, int timeTagIdx, int lyricLineIdx, int stamp)
{
	if (lrc->time == NULL){
		lrc->timeTotal = 1;
		lrc->time = (TLTIME*)calloc(1, sizeof(TLTIME));
		if (lrc->time == NULL)
			return 0;
	}
	
	if (timeTagIdx >= lrc->timeTotal){
		lrc->time = (TLTIME*)realloc(lrc->time, (1+timeTagIdx) * sizeof(TLTIME));
		int i;
		for (i = lrc->timeTotal; i <= timeTagIdx; i++){
			lrc->time[i].stamp = 0;
			lrc->time[i].lyricIdx = 0;
		}
		lrc->timeTotal = timeTagIdx+1;
	}
	
	lrc->time[timeTagIdx].stamp = stamp;
	lrc->time[timeTagIdx].lyricIdx = lyricLineIdx;
	
	
	return 1;
}

int addLyric (TLYRICS *lrc, int lyricLineIdx, char *lyric, int lyricLen)
{
	if (lrc->lines == NULL){
		lrc->lyricTotal = 1;
		lrc->lines = (TLLINE*)calloc(1, sizeof(TLLINE));
		if (lrc->lines == NULL)
			return 0;
	}
	
	if (lyricLineIdx >= lrc->lyricTotal){
		lrc->lines = (TLLINE*)realloc(lrc->lines, (1+lyricLineIdx) * sizeof(TLLINE));
		int i;
		for (i = lrc->lyricTotal; i <= lyricLineIdx; i++){
			lrc->lines[i].lyric = NULL;
			lrc->lines[i].len = 0;
		}
		lrc->lyricTotal = lyricLineIdx+1;
	}
		
	lrc->lines[lyricLineIdx].lyric = strdup(lyric);
	lrc->lines[lyricLineIdx].len = lyricLen;

	return (lrc->lines[lyricLineIdx].lyric != NULL);
}

int loadLyricFile (TLYRICS *lrc, wchar_t *path)
{
	int ret = 0;
	if (path){
		size_t flen = 0;
		char *lyricsData = getFileContents(path, &flen);
		if (lyricsData){
			//dbprintf("lyric file length: %i\n", flen);
			
			if (isDataUTF16(lyricsData)){
				//dbprintf("lyric file is in UTF16 (BOM:%X %X)\n", (ubyte)lyricsData[0], (ubyte)lyricsData[1]);
				lrc->format = 1;
				ret = 0;
							
				// convert utf16 to utf8
				size_t wlen = wcslen((wchar_t*)lyricsData);
				if (wlen){
					char *utf8str = calloc(sizeof(char), wlen*(sizeof(wchar_t)<<2));
					if (utf8str){
						utf8_wcstombs((ubyte*)utf8str, wlen*(sizeof(wchar_t)<<2), (wchar_t*)lyricsData, wlen);
						parseLyricsBuffer(lrc, utf8str, strlen(utf8str));
						free(utf8str);
						ret = 2; //lrc->lyricTotal;
					}
				}
			}else{
				//dbprintf("lyric file is not UTF16\n");
				lrc->format = 2;
				parseLyricsBuffer(lrc, lyricsData, flen);
				ret = 1; // lrc->lyricTotal;
			}
			free(lyricsData);
		}else{
			//wprintf(L"lyric file is not found: '%s'", path); dbprintf("\n");
		}
	}else{
		//wprintf(L"\ninvalid path supplied\n");
	}
	return ret;
}

int loadLyricRecord (TMLRECORD *records, int total, TLYRICS *lrc)
{

	TMLRECORD *record = findRecordByTrackShort(records, total, lrc);
	if (record == NULL)
		record = findRecordByTrackLong(records, total, lrc);
	if (record){
		//wprintf(L"\n\n%s\n", record->mediaPath);
		//wprintf(L"%s\n", record->lyricPath);
		return loadLyricFile(lrc, record->lyricPath);
	}else{
		return 0;
	}
}

void deleteRecords (TMLRECORD *records)
{
	if (records)
		free(records);
}

int isDataUTF16 (char *data)
{
	return ((unsigned char)data[0] == 0xFF && (unsigned char)data[1] == 0xFE) \
		|| ((unsigned char)data[0] == 0xFE && (unsigned char)data[1] == 0xFF);
		
}

char *getFileContents (wchar_t *path, size_t *flen)
{
	FILE *file = _wfopen(path, L"rb");
	if (file){
		*flen = lof(file);
		if (*flen){
			char *data = (char *)malloc(*flen);
			if (data){
				size_t readlen = fread(data, 1, *flen, file);
				fclose(file);
				if (readlen == *flen){
					return data;
				}else{
					free(data);
					return NULL;
				}
			}
		}
		fclose(file);
	}
	if (flen) *flen = 0;
	return NULL;
}

TMLRECORD *getRecords (wchar_t *path, int *total)
{
	size_t flen = 0;
	TMLRECORD *records = (TMLRECORD *)getFileContents(path, &flen);
	if (records){
		*total = flen/sizeof(TMLRECORD);
		return records;
	}else{
		*total = 0;
		return NULL;
	}
}

TMLRECORD *findRecordByTrackLong (TMLRECORD *records, int total, TLYRICS *lrc)
{
	int i;
	for (i = 0; i < total; i++){
		if (!wcscmp(records[i].mediaPath, lrc->longwPath)){
			return &records[i];
		}
	}
	return NULL;
}

TMLRECORD *findRecordByTrackShort (TMLRECORD *records, int total, TLYRICS *lrc)
{
	int i;
	for (i = 0; i < total; i++){
		if (!wcscmp(records[i].mediaPath, lrc->shortwPath)){
			return &records[i];
		}
	}
	return NULL;
}

/*
u64 lof (FILE *stream)
{
	fpos_t pos;
	fgetpos(stream, &pos);
	fseek(stream, 0, SEEK_END);
	u64 fl = (u64)ftell(stream);
	fsetpos(stream, &pos);
	return fl;
}

*/
