
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.


#include <inttypes.h>

typedef struct{
	unsigned int tlines;
	char	**lines;
	char	*data;
}TUTASCIILINE;

typedef struct{
	char *label;
	int	total;
}TTORRENTLABEL;

typedef struct{
	char	*hash;
	int		status;
	char 	*name;
	u64		fileSize;
	int		have;
	u64		downloaded;
	u64		uploaded;
	int		ratio;
	u64		uploadRate;
	u64		downloadRate;
	int		ETA;			// ETA, seconds remaining
	char	*label;
	int		peersConnected;
	int		peersInSwarm;
	int		seedsConnected;
	int		seedsInSwarm;
	int		available;		// available/65536 = 1.234%
	int		order;
	u64		remaining;
}TTORRENTENTRY;

typedef struct{
	TTORRENTENTRY *list;
	TTORRENTLABEL *label;
	int	totalTorrents;
	int totalLabels;
}TTORRENTS;



int decomposeTorrentLine (TTORRENTENTRY *tor, char *line, int slen);
int decomposeLabelLine (TTORRENTLABEL *tor, char *line, int slen);
void dumpTorrent (TTORRENTENTRY *tor);
int getTorrentList (char *data, size_t dsize, char *username, char *password, int port);
void freeTorrents (TTORRENTS *torrents);
int getTorrents (TTORRENTS *torrents, char *username, char *password, int port);

int countChars (const char *buffer, size_t blen, const char theChar);		// get total occurrence of c within string
int countCharsTo (const char *buffer, size_t blen, const char theChar);		// get number of chars between up to c
int getInt (const char *buffer, size_t blen, int *pos);
u64 getU64 (const char *buffer, size_t blen, int *pos);
int getStrEnd (const char *buffer, size_t blen, int *pos, int *end);
char *getStr (const char *buffer, size_t blen, int *pos);

char *getHash (const char *buffer, size_t blen, int *pos);
int getStatus (const char *buffer, size_t blen, int *pos);
char *getName (const char *buffer, size_t blen, int *pos);
u64 getFileSize (const char *buffer, size_t blen, int *pos);
int getHave (const char *buffer, size_t blen, int *pos);
u64 getDownloaded (const char *buffer, size_t blen, int *pos);
u64 getUploaded (const char *buffer, size_t blen, int *pos);
int getRatio (const char *buffer, size_t blen, int *pos);
u64 getUploadRate (const char *buffer, size_t blen, int *pos);
u64 getDownloadRate (const char *buffer, size_t blen, int *pos);
char *getLabel (const char *buffer, size_t blen, int *pos);
int getPeersConnected (const char *buffer, size_t blen, int *pos);
int getPeersInSwarm (const char *buffer, size_t blen, int *pos);
int getSeedsConnected (const char *buffer, size_t blen, int *pos);
int getSeedsInSwarm (const char *buffer, size_t blen, int *pos);
int getETA (const char *buffer, size_t blen, int *pos);
int getAvailable (const char *buffer, size_t blen, int *pos);
int getOrder (const char *buffer, size_t blen, int *pos);
u64 getBytesRemaining (const char *buffer, size_t blen, int *pos);


int getUrl (char *url, char *buffer, long blen)
{
	memset(buffer, 0, blen);
	int status = 1;
	
	HINTERNET hSession = InternetOpen("httpGetFile", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);
	if (!hSession) return 0;
	HINTERNET hOpenUrl = InternetOpenUrl(hSession, url, 0, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_EXISTING_CONNECT, 0);
	//printf("InternetOpenUrl ret: %i %i\n", (int)hOpenUrl, (int)GetLastError());
	
	size_t total = 0;
	if (hOpenUrl){
		DWORD bytesread = 0;
		
		do{
			bytesread = 0;
			status = InternetReadFile(hOpenUrl, buffer+total, (blen-total)-1, &bytesread);
			//printf("InternetReadFile ret: %i %i\n", status, (int)GetLastError());
			total += bytesread;
		}while(status == 1 && bytesread && bytesread < blen);
		
		InternetCloseHandle(hOpenUrl);
		buffer[total] = 0;
	}
	InternetCloseHandle(hSession);
	return (int)total;
}

void freeALBuffer (TUTASCIILINE *al)
{
	if (al != NULL){
		free(al->lines);
		free(al->data);
		free(al);
	}
}

TUTASCIILINE *decomposeBuffer (char *data, size_t dlen)
{
	if (data == NULL)
		return NULL;
	else if (!*data)
		return NULL;
		
	TUTASCIILINE *al = (TUTASCIILINE *)malloc(sizeof(TUTASCIILINE));
	if (!al) return NULL;

	al->data = (char *)calloc(sizeof(char *), 4 + dlen);
	if (!al->data){
		free(al);
		return NULL;
	}
	memcpy(al->data, data, dlen);

	unsigned int i=0,c13=0,c10=0;
	do{
		if (al->data[i]==13) c13++;
		else if (al->data[i]==10) c10++;
	}while(++i < dlen);
	
	al->tlines = MAX(c10 , c13);
	al->lines = (char **)calloc(4+al->tlines, sizeof(char *));
	if (!al->lines){
		free(al->data);
		free(al);
		return NULL;
	}
	
	i=0;
	char *str = strtok((char*)al->data, "\12\15");

	do{
		al->lines[i++] = str;
		str = strtok(NULL, "\12\15");
	}while(str && (i <= al->tlines));
	
	al->tlines = i;
	return al;
}

int countChars (const char *buffer, size_t blen, const char theChar)
{
	int ct = 0;
	int i;
	for (i = 0; i < blen && buffer[i]; i++){
		if (buffer[i] == theChar)
			ct++;
	}
	return ct;
}

int countCharsTo (const char *buffer, size_t blen, const char theChar)
{
	int i;
	for (i = 0; i < blen && buffer[i]; i++){
		if (buffer[i] == theChar)
			return i+1;
	}
	return 0;
}

int decomposeLabelLine (TTORRENTLABEL *lab, char *line, int slen)
{
	memset(lab, 0, sizeof(TTORRENTLABEL));
	
	if ((line[0] != '[' || line[1] != '\"') && (line[0] != ',' || line[1] != '[' || line[2] != '\"'))
		return 0;

	int com = countChars(line, slen, ',');
	int openb = countChars(line, slen, '[');
	int closeb = countChars(line, slen, ']');
	int str = countChars(line, slen, '\"');
	if (com < 0 || com > 10 || !openb || !closeb || str < 2) 
		return 0;
		
	int pos = 0;
	if (line[0] == '[')
		pos = 1;
	else if (line[0] == ',')
		pos = 2;
	
	lab->label = getStr(&line[pos], slen, &pos);
	if (lab->label == NULL) return 0;
	lab->total = getInt(&line[pos], slen, &pos);
	//dbprintf("last pos: %i, slen: %i\n", pos, slen);
	return 1;
}

int decomposeTorrentLine (TTORRENTENTRY *tor, char *line, int slen)
{
	memset(tor, 0, sizeof(TTORRENTENTRY));

	if (slen < 44)		// 42 = minimum line length, 82 including hash
		return 0;
	if (line == NULL)
		return 0;
	if (line[0] != '[' || line[1] != '\"' || line[42] != '\"' || line[43] != ',' || !isdigit(line[44]))
		return 0;

	int com = countChars(line, slen, ',');
	int openb = countChars(line, slen, '[');
	int closeb = countChars(line, slen, ']');
	int str = countChars(line, slen, '\"');
	if (com < 18 || !openb || !closeb || str < 6) 
		return 0;
	int pos = 0;
	tor->hash = getHash(line, slen, &pos);
	if (tor->hash == NULL) return 0;
	tor->status = getStatus(&line[pos], slen, &pos);
	tor->name = getName(&line[pos], slen, &pos);
	if (tor->name == NULL){
		free(tor->hash);
		return 0;
	}
	tor->fileSize = getFileSize(&line[pos], slen, &pos);
	tor->have = getHave(&line[pos], slen, &pos);
	tor->downloaded = getDownloaded(&line[pos], slen, &pos);
	tor->uploaded = getUploaded(&line[pos], slen, &pos);
	tor->ratio = getRatio(&line[pos], slen, &pos);
	tor->uploadRate = getUploadRate(&line[pos], slen, &pos);
	tor->downloadRate = getDownloadRate(&line[pos], slen, &pos);
	tor->ETA = getETA(&line[pos], slen, &pos);
	tor->label = getLabel(&line[pos], slen, &pos);
	if (tor->label == NULL){
		free(tor->hash);
		free(tor->name);
		return 0;
	}
	tor->peersConnected = getPeersConnected(&line[pos], slen, &pos);
	tor->peersInSwarm = getPeersInSwarm(&line[pos], slen, &pos);
	tor->seedsConnected = getSeedsConnected(&line[pos], slen, &pos);
	tor->seedsInSwarm = getSeedsInSwarm(&line[pos], slen, &pos);
	tor->available = getAvailable(&line[pos], slen, &pos);
	tor->order = getOrder(&line[pos], slen, &pos);
	tor->remaining = getBytesRemaining(&line[pos], slen, &pos);
	
	//dbprintf("last pos: %i, slen: %i\n", pos, slen);
	return 1;
}

void dumpTorrent (TTORRENTENTRY *tor)
{
	//printf("\n");
	dbprintf("name: >%s<\n", tor->name);
	dbprintf("status: %i\n", tor->status);
	dbprintf("hash: >%s<\n", tor->hash);
	dbprintf("fileSize: %"PRId64"\n", tor->fileSize);
	dbprintf("have: %i\n", tor->have);
	dbprintf("downloaded: %"PRId64"\n", tor->downloaded);
	dbprintf("uploaded: %"PRId64"\n", tor->uploaded);
	dbprintf("ratio: %i\n", tor->ratio);
	dbprintf("uploadRate: %"PRId64"\n", tor->uploadRate);
	dbprintf("downloadRate: %"PRId64"\n", tor->downloadRate);
	dbprintf("ETA: %i\n", tor->ETA);
	dbprintf("label: >%s<\n", tor->label);
	dbprintf("peersConnected: %i\n", tor->peersConnected);
	dbprintf("peersInSwarm: %i\n", tor->peersInSwarm);
	dbprintf("seedsConnected: %i\n", tor->seedsConnected);
	dbprintf("seedsInSwarm: %i\n", tor->seedsInSwarm);
	dbprintf("available: %i\n", tor->available);
	dbprintf("order: %i\n", tor->order);
	dbprintf("remaining: %"PRId64"\n", tor->remaining);
}

int getTorrentList (char *data, size_t dsize, char *username, char *password, int port)
{
	const char url[] = {"127.0.0.1"};
	const char location[] = {"/gui/?list=1"};
	char buffer[4096];
	*data = 0;

	snprintf(buffer, sizeof(buffer)-1, "http://%s:%s@%s:%i%s", username, password, url, port, location);
	return getUrl(buffer, data, dsize); 
}

int getTorrents (TTORRENTS *torrents, char *username, char *password, int port)
{

	memset(torrents, 0, sizeof(TTORRENTS));	
	int dlen = 0;
	int blen = 8192;
	char *buffer = malloc(blen*sizeof(char));
	if (buffer == NULL) return -1;
	
	do{
		blen <<= 1;
		buffer = realloc(buffer, blen*sizeof(char));
		if (buffer != NULL)
			dlen = getTorrentList(buffer, blen, username, password, port);
	}while(dlen >= blen-1 && buffer && dlen > 10);

	//printf("dlen %i 0x%p\n",dlen, buffer);
	
	if (dlen < 20 || !*buffer){
		free(buffer);
		return -2;
	}

	TUTASCIILINE *al = decomposeBuffer(buffer, dlen);
	if (al){
		if (al->tlines){
			torrents->totalTorrents = 0;
			torrents->totalLabels = 0;
			torrents->list = (TTORRENTENTRY*)calloc(al->tlines, sizeof(TTORRENTENTRY));
			torrents->label = (TTORRENTLABEL*)calloc(al->tlines, sizeof(TTORRENTLABEL));

			int i;
			for (i = 0; i < al->tlines; i++){
				TTORRENTENTRY *tor = &torrents->list[torrents->totalTorrents];
				if (decomposeTorrentLine(tor, al->lines[i], strlen(al->lines[i]))){
					torrents->totalTorrents++;
				}else{
					TTORRENTLABEL *lab = &torrents->label[torrents->totalLabels];
					if (decomposeLabelLine(lab, al->lines[i], strlen(al->lines[i]))){
						torrents->totalLabels++;
					}
				}
			}
		}else{
			//printf("al->tlines is null\n");
		}
		freeALBuffer(al);		
	}else{
		//printf("al is null\n");
	}
	
	free(buffer);
	//printf("torrents: %i, labels: %i\n", torrents->totalTorrents, torrents->totalLabels);
	
	if (!torrents->totalLabels){
		if (torrents->label)
			free(torrents->label);
		torrents->label = NULL;
	}
	if (!torrents->totalTorrents){
		freeTorrents(torrents);
		return -3;
	}else{
		return torrents->totalTorrents;
	}
}

void freeTorrents (TTORRENTS *torrents)
{
	if (torrents->totalTorrents){
		TTORRENTENTRY *tor = torrents->list;

		while(torrents->totalTorrents--){
			if (tor->name)
				free(tor->name);
			if (tor->hash)
				free(tor->hash);
			if (tor->label)
				free(tor->label);
			tor++;
		}
	}
	if (torrents->list)
		free(torrents->list);
	torrents->list = NULL;
	
	if (torrents->totalLabels){
		TTORRENTLABEL *lab = torrents->label;

		while(torrents->totalLabels--){
			if (lab->label)
				free(lab->label);
			lab++;
		}
		free(torrents->label);
		torrents->label = NULL;
	}
	if (torrents->label)
		free(torrents->label);
	torrents->label = NULL;
	torrents->totalTorrents = 0;
	torrents->totalLabels = 0;
}

u64 getBytesRemaining (const char *buffer, size_t blen, int *pos)
{
	return getU64(buffer, blen, pos);
}

int getOrder (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

int getAvailable (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

int getETA (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

int getSeedsInSwarm (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

int getSeedsConnected (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

int getPeersInSwarm (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

int getPeersConnected (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

char *getLabel (const char *buffer, size_t blen, int *pos)
{
	if (buffer[0] != '\"')
		return NULL;
	else
		return getStr(buffer, blen, pos);
}

u64 getDownloadRate (const char *buffer, size_t blen, int *pos)
{
	return getU64(buffer, blen, pos);
}

u64 getUploadRate (const char *buffer, size_t blen, int *pos)
{
	return getU64(buffer, blen, pos);
}

int getRatio (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

u64 getUploaded (const char *buffer, size_t blen, int *pos)
{
	return getU64(buffer, blen, pos);
}

u64 getDownloaded (const char *buffer, size_t blen, int *pos)
{
	return getU64(buffer, blen, pos);
}

int getHave (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}

u64 getFileSize (const char *buffer, size_t blen, int *pos)
{
	return getU64(buffer, blen, pos);
}

char *getName (const char *buffer, size_t blen, int *pos)
{

	if (buffer[0] != '\"')
		return NULL;
	else
		return getStr(buffer, blen, pos);
}

int getStatus (const char *buffer, size_t blen, int *pos)
{
	return getInt(buffer, blen, pos);
}
	
char *getHash (const char *buffer, size_t blen, int *pos)
{
	if (blen < 44)		// 42 = minimum line length, 82 including hash
		return NULL;
	if (buffer == NULL)
		return NULL;
		
	if (buffer[1] != '\"' || buffer[42] != '\"' || buffer[43] != ',' || !isdigit(buffer[44]))
		return NULL;
	else
		return getStr(buffer, blen, pos);
}

int getInt (const char *buffer, size_t blen, int *pos)
{
	*pos += countCharsTo(buffer, blen, ',');
	return atoi(buffer);
}

u64 getU64 (const char *buffer, size_t blen, int *pos)
{
	*pos += countCharsTo(buffer, blen, ',');
	u64 fsize = 0;
	sscanf(buffer, "%"PRId64, &fsize);
	return fsize;
}

int getStrEnd (const char *buffer, size_t blen, int *pos, int *end)
{
	if (!buffer[0]){
		return 0;
	}else if (buffer[0] == '\"' && buffer[1] == ',' && isdigit(buffer[2])){
		return *end;
	}else{
		++(*end);
		getStrEnd(++buffer, blen, pos, end);
		return *end;
	}
}

char *getStr (const char *buffer, size_t blen, int *pos)
{
	int end = 0;
	end = getStrEnd(++buffer, blen, pos, &end);
	if (end){
		char *name= (char*)calloc(sizeof(char), end+1);	// hash length is 40 plus 1 for terminator
		if (name != NULL){
			memcpy(name, buffer, end);
			name[end] = 0;
			*pos += end+3;
			return name;
		}
	}else{
		if (buffer[0] == '\"' && buffer[1] == ','){
			char *name = (char*)calloc(sizeof(char), 2);
			if (name != NULL){
				name[0] = 0;
				name[1] = 0;
				*pos += 3;
				return name;
			}
		}
	}
	return NULL;
}

