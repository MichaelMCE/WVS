
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


int lng_setEncoding (TLANG *lang, char *enc)
{
	//printf("lng_setEncoding %p\n", lang->hw);
	
	if (enc)
		lang->enc = lEncodingAliasToID(lang->hw, enc);
		
	if (enc == NULL || !lang->enc)
		lang->enc = CMT_ISO8859_15;
		
	return lang->enc;
}

int lng_getEncoding (TLANG *lang)
{
	return lang->enc;
}

char *lng_getItem (TLANG *lang, int lngIndex)
{
	if (lngIndex < lang->tItems){
		if (lang->itmTbl[lngIndex].item)
			return lang->itmTbl[lngIndex].item;
	}
	return "invalid lid";
}

char *lng_getString (TLANG *lang, int lngIndex)
{
	if (lngIndex < lang->tItems){
		if (lang->itmTbl[lngIndex].entry)
			return lang->itmTbl[lngIndex].entry;
	}
	return "invalid lid";
}

int lng_addItem (TLANG *lang, int lngIndex, char *item)
{
	if (lngIndex < lang->tItems){
		strncpy(lang->itmTbl[lngIndex].item, item, MAXITEMLENGTH);
		return 1;
	}
	return 0;
}

int lng_addString (TLANG *lang, int lngIndex, char *entry)
{
	if (lngIndex < lang->tItems && lngIndex >= 0 && entry){
		lang->itmTbl[lngIndex].entry = strdup(entry);
		return (lang->itmTbl[lngIndex].entry != NULL);
	}
	return 0;
}

TLANG *lng_createTable (int total)
{
	TLANG *lang = (TLANG*)calloc(1, sizeof(TLANG));
	if (lang != NULL){
		lang->tItems = total;
		lang->itmTbl = (TLNGITEM*)calloc(total, sizeof(TLNGITEM));
		if (lang->itmTbl != NULL)
			return lang;
		else
			free(lang);
	}
	return NULL;
}

void lng_freeTable (TLANG *lang)
{
	if (lang != NULL){
		if (lang->itmTbl != NULL){
			int i;
			for (i = 0; i < lang->tItems; i++){
				if (lang->itmTbl[i].entry)
					free(lang->itmTbl[i].entry);
			}
			free(lang->itmTbl);
		}
		free(lang);
	}
}

int lng_getItemIndex (TLANG *lang, char *item)
{
	int i;
	for (i = 0; i < lang->tItems; i++){
		if (!strncmp(lang->itmTbl[i].item, item, MAXITEMLENGTH)){
			return i;
		}
	}
	return -1;
}

int lng_decomposeLine (ubyte *line, char **item, char **entry)
{
	if (line){
		*entry = strchr((char*)line, '=');
		if (*entry){
			*entry[0] = 0;
			(*entry)++;
			if (*entry[0]){
				*item = (char*)line;
				return 1;
			}
		}
	}
	return 0;
}

//int lng_loadLangFile (TLANG *lang, wchar_t *filepath)
int lng_loadLangFile (TLANG *lang, char *filepath)
{
	TASCIILINE *al = readFileA(filepath);
	if (al == NULL){
		dbprintf("lng_loadLangFile(): '%s' not loaded\n", filepath);
		return 0;
	}
	
	ubyte *line = NULL;
	char *item = NULL;
	char *entry = NULL;
	char *tab = NULL;
	int i;

	for (i = 0; i < al->tlines; i++){
		line = al->lines[i];
		if (line == NULL) break;
		if (*line == ';' || *line == '=' || *line == ' ' || *line == '\t') continue;
		tab = strchr((char*)line, '\t');
		if (tab) *tab = 0;
		
		item = NULL; entry = NULL;
		if (!lng_decomposeLine(line, &item, &entry)) continue;
		//printf("decompose %i: '%s' '%s'\n", i, item, entry);
		int idx = lng_getItemIndex(lang, item);
		if (idx >= 0)
			lng_addString(lang, idx, entry);
	}
	
	freeASCIILINE(al);
	return (i > 0);
}


void lng_registerStrings (TLANG *lang)
{
	//printf("tItems %i\n", lang->tItems);
		
	lng_addItem(lang, LNG_ENCODING, "encoding");
	
	lng_addItem(lang, LAG_MT_TITLE, "Title");
	lng_addItem(lang, LAG_MT_ARTIST, "Artist");
	lng_addItem(lang, LAG_MT_ALBUM, "Album");
	lng_addItem(lang, LAG_MT_TRACK, "Track");
	lng_addItem(lang, LAG_MT_YEAR, "Year");
	lng_addItem(lang, LAG_MT_GENRE, "Genre");
	lng_addItem(lang, LAG_MT_COMMENT, "Comment");
	lng_addItem(lang, LAG_MT_LENGTH, "Length");
	lng_addItem(lang, LAG_MT_BITRATE, "Bitrate");
	lng_addItem(lang, LAG_MT_SRATE, "SRate");
	lng_addItem(lang, LAG_MT_STEREO, "Stereo");
	lng_addItem(lang, LAG_MT_VBR, "VBR");
	lng_addItem(lang, LAG_MT_BPM, "BPM");
	lng_addItem(lang, LAG_MT_GAIN, "Gain");
	lng_addItem(lang, LAG_MT_ALBUMGAIN, "AlbumGain");
	lng_addItem(lang, LAG_MT_ENCODER, "Encoder");
	lng_addItem(lang, LAG_MT_ALBUMARTIST, "AlbumArtist");
	lng_addItem(lang, LAG_MT_ORIGINALARTIST, "OriginalArtist");
	lng_addItem(lang, LAG_MT_DISC, "Disc");
	lng_addItem(lang, LAG_MT_REMIXER, "Remixer");
	lng_addItem(lang, LAG_MT_MEDIA, "Media");
	lng_addItem(lang, LAG_MT_LYRICIST, "Lyricist");
	lng_addItem(lang, LAG_MT_SUBTITLE, "Subtitle");
	lng_addItem(lang, LAG_MT_MOOD, "Mood");
	lng_addItem(lang, LAG_MT_KEY, "Key");
	lng_addItem(lang, LAG_MT_COMPOSER, "Composer");
	lng_addItem(lang, LAG_MT_CONDUCTOR, "Conductor");
	lng_addItem(lang, LAG_MT_PUBLISHER, "Publisher");
	lng_addItem(lang, LAG_MT_COPYRIGHT, "Copyright");
	lng_addItem(lang, LAG_MT_URL, "URL");
	
	lng_addItem(lang, LNG_TIT_TRK, "Trk");
	lng_addItem(lang, LNG_VOL_MASTER, "Master");
	lng_addItem(lang, LNG_VOL_WAVE, "Wave");
	lng_addItem(lang, LNG_HIT_HATEIT, "Hateit");
	lng_addItem(lang, LNG_HIT_LIKEIT, "Likeit");
	lng_addItem(lang, LNG_HIT_ADDED, "Added");
	lng_addItem(lang, LNG_HIT_REMOVED, "Removed");
	lng_addItem(lang, LNG_EQ_EQ, "EQ");
	lng_addItem(lang, LNG_EQ_AUTO, "Auto");
	lng_addItem(lang, LNG_EQ_DB, "db");

	lng_addItem(lang, LNG_LYR_LYRICSUNAVAILABLE, "LyricsUnavailable");
	lng_addItem(lang, LNG_LYR_TITLE, "LYTitle");
	lng_addItem(lang, LNG_LYR_ARTIST, "LYArtist");
	lng_addItem(lang, LNG_LYR_ALBUM, "LYAlbum");
	lng_addItem(lang, LNG_LYR_AUTHOR, "LYAuthor");
	lng_addItem(lang, LNG_LYR_ENCODING, "LYEncoding");
	lng_addItem(lang, LNG_LYR_RE, "Re");
	lng_addItem(lang, LNG_LYR_VERSION, "Version");
	lng_addItem(lang, LNG_LYR_OFFSET, "Offset");
	lng_addItem(lang, LNG_LYR_LYRICS, "LYLyrics");
	lng_addItem(lang, LNG_LYR_STAMPS, "Stamps");

	lng_addItem(lang, LNG_MSN_CONTACTLIST, "ContactList");
	lng_addItem(lang, LNG_MSN_REVERSELIST, "ReverseList");
	lng_addItem(lang, LNG_MSN_ALLOWEDLIST, "AllowedList");
	lng_addItem(lang, LNG_MSN_BLOCKEDLIST, "BlockedList");
	lng_addItem(lang, LNG_MSN_PENDINGLIST, "PendingList");
	lng_addItem(lang, LNG_MSN_ALLCONTACTS, "AllContacts");
	lng_addItem(lang, LNG_MSN_INBOX, "Inbox");
	
	lng_addItem(lang, LNG_UT_STATUS, "Status");
	lng_addItem(lang, LNG_UT_COMPLETED, "Completed");
	lng_addItem(lang, LNG_UT_NAME, "Name");
	lng_addItem(lang, LNG_UT_DOWNRATE, "DownRate");
	lng_addItem(lang, LNG_UT_UPRATE, "UpRate");
	lng_addItem(lang, LNG_UT_ETA, "ETA");
	lng_addItem(lang, LNG_UT_LABEL, "Label");
	lng_addItem(lang, LNG_UT_UTORRENTUNAVAIL, "ęTorrentUnavail");
	
	lng_addItem(lang, LNG_UT_HAVE, "Have");
	lng_addItem(lang, LNG_UT_OF, "of");
	lng_addItem(lang, LNG_UT_SEEDS, "Seeds");
	lng_addItem(lang, LNG_UT_PEERS, "Peers");
	lng_addItem(lang, LNG_UT_COMPLETE, "Complete");
	lng_addItem(lang, LNG_UT_PAUSED, "Paused");
	lng_addItem(lang, LNG_UT_DOWNLOADING, "Downloading");
	lng_addItem(lang, LNG_UT_SEEDING, "Seeding");
	lng_addItem(lang, LNG_UT_DOWNLOADFORCED, "DownloadForced");
	lng_addItem(lang, LNG_UT_SEEDINGFORCED, "SeedingForced");
	lng_addItem(lang, LNG_UT_CHECKING, "Checking");
	lng_addItem(lang, LNG_UT_ERROR, "Error");
	lng_addItem(lang, LNG_UT_QUEUED, "Queued");
	lng_addItem(lang, LNG_UT_STOPPED, "Stopped");
	lng_addItem(lang, LNG_UT_TORRENTS, "Torrents");
	lng_addItem(lang, LNG_UT_R, "R");
	lng_addItem(lang, LNG_UT_A, "A");
	lng_addItem(lang, LNG_UT_DL, "DL");
	lng_addItem(lang, LNG_UT_UL, "UL");
	lng_addItem(lang, LNG_UT_IN, "In");
	lng_addItem(lang, LNG_UT_OUT, "Out");
	lng_addItem(lang, LNG_UT_RATIO, "Ratio");
	lng_addItem(lang, LNG_UT_AVAIL, "Avail");
	lng_addItem(lang, LNG_UT_SE, "Se");
	lng_addItem(lang, LNG_UT_DN, "Dn");
	lng_addItem(lang, LNG_UT_CP, "Cp");
	lng_addItem(lang, LNG_UT_PU, "Pu");
	lng_addItem(lang, LNG_UT_ST, "St");
	lng_addItem(lang, LNG_UT_ER, "Er");

	lng_addItem(lang, LNG_NS_PROT, "Prot");
	lng_addItem(lang, LNG_NS_LOIP, "LoIP");
	lng_addItem(lang, LNG_NS_REIP, "ReIP");
	lng_addItem(lang, LNG_NS_PT, "Pt");
	lng_addItem(lang, LNG_NS_PID, "Pid");
	lng_addItem(lang, LNG_NS_FILE, "File");
	lng_addItem(lang, LNG_NS_STATE, "State");
	lng_addItem(lang, LNG_NS_PROTOCOLTCPUDP, "ProtocolTCPUDP");
	lng_addItem(lang, LNG_NS_LOCALIP, "LocalIP");
	lng_addItem(lang, LNG_NS_LOCALPORT, "LocalPort");
	lng_addItem(lang, LNG_NS_REMOTEIP, "RemoteIP");
	lng_addItem(lang, LNG_NS_REMOTEPORT, "RemotePort");
	lng_addItem(lang, LNG_NS_PROCESSID, "ProcessID");
	lng_addItem(lang, LNG_NS_FILENAME, "Filename");
	lng_addItem(lang, LNG_NS_TCPCONNSTATE, "TCPConnState");
	lng_addItem(lang, LNG_NS_PROTOCOL, "Protocol");
	lng_addItem(lang, LNG_NS_ENDPOINT, "Endpoint");
	lng_addItem(lang, LNG_NS_LOCAL, "Local");
	lng_addItem(lang, LNG_NS_REMOTE, "Remote");
	lng_addItem(lang, LNG_NS_MODULE, "Module");
	lng_addItem(lang, LNG_NS_CONNECTIONSTATE, "ConnectionState");
	lng_addItem(lang, LNG_NS_UNKNOWN, "unknown");
	lng_addItem(lang, LNG_NS_CLOSED, "Closed");
	lng_addItem(lang, LNG_NS_LISTENING, "Listening");
	lng_addItem(lang, LNG_NS_SYNSENT, "SynSent");
	lng_addItem(lang, LNG_NS_SYNRECEIVED, "SynReceived");
	lng_addItem(lang, LNG_NS_ESTABLISHED, "Established");
	lng_addItem(lang, LNG_NS_FINWAIT1, "FinWait-1");
	lng_addItem(lang, LNG_NS_FINWAIT2, "FinWait-2");
	lng_addItem(lang, LNG_NS_CLOSEWAIT, "CloseWait");
	lng_addItem(lang, LNG_NS_CLOSING, "Closing");
	lng_addItem(lang, LNG_NS_LASTACKNOWLEDGE, "LastAcknowledge");
	lng_addItem(lang, LNG_NS_TIMEWAIT, "TimeWait");
	lng_addItem(lang, LNG_NS_DELETETCB, "DeleteTCB");
	lng_addItem(lang, LNG_NS_SYSTEM, "system");
	lng_addItem(lang, LNG_NS_YOURIP, "YourIP");	
	
	lng_addItem(lang, LNG_PL_PROCESS, "Process");
	lng_addItem(lang, LNG_PL_PID, "PID");
	lng_addItem(lang, LNG_PL_THS, "Ths");
	lng_addItem(lang, LNG_PL_MEMUSAGE, "MemUsage");
	lng_addItem(lang, LNG_PL_PEAKMEMUSAGE, "PeakMemUsage");
	lng_addItem(lang, LNG_PL_PAGEFAULTS, "PageFaults");

	lng_addItem(lang, LNG_SI_CPU, "CPU");
	lng_addItem(lang, LNG_SI_PAGE, "Page");
	lng_addItem(lang, LNG_SI_VIRT, "Virt");
	lng_addItem(lang, LNG_SI_UPTIME, "Uptime");
	lng_addItem(lang, LNG_SI_W, "W");
	lng_addItem(lang, LNG_SI_D, "D");
	lng_addItem(lang, LNG_SI_H, "H");
	lng_addItem(lang, LNG_SI_M, "M");

	lng_addItem(lang, LNG_ATT_ATTNOTFOUND, "ATTNotFound");
	lng_addItem(lang, LNG_ATT_GPU, "GPU");
	lng_addItem(lang, LNG_ATT_FPS, "FPS");
	lng_addItem(lang, LNG_ATT_ENV, "Env");
	lng_addItem(lang, LNG_ATT_MODE, "Mode");
	lng_addItem(lang, LNG_ATT_FAN, "Fan");
	lng_addItem(lang, LNG_ATT_OGL, "OGL");
	lng_addItem(lang, LNG_ATT_AA, "AA");
	lng_addItem(lang, LNG_ATT_AF, "AF");
	lng_addItem(lang, LNG_ATT_D3D, "D3D");

	lng_addItem(lang, LNG_FRP_FRAPSNOTFOUND, "FrapsNotFound");
	lng_addItem(lang, LNG_FRP_STARTFRAPSFIRST, "StartFrapsFirst");

	lng_addItem(lang, LNG_PNG_MS, "ms");

	lng_addItem(lang, LNG_CAL_JANUARY, "January");
	lng_addItem(lang, LNG_CAL_FEBRUARY, "February");
	lng_addItem(lang, LNG_CAL_MARCH, "March");
	lng_addItem(lang, LNG_CAL_APRIL, "April");
	lng_addItem(lang, LNG_CAL_MAY, "May");
	lng_addItem(lang, LNG_CAL_JUNE, "June");
	lng_addItem(lang, LNG_CAL_JULY, "July");
	lng_addItem(lang, LNG_CAL_AUGUST, "August");
	lng_addItem(lang, LNG_CAL_SEPTEMBER, "September");
	lng_addItem(lang, LNG_CAL_OCTOBER, "October");
	lng_addItem(lang, LNG_CAL_NOVEMBER, "November");
	lng_addItem(lang, LNG_CAL_DECEMBER, "December");
	lng_addItem(lang, LNG_CAL_MO, "Mo");
	lng_addItem(lang, LNG_CAL_TU, "Tu");
	lng_addItem(lang, LNG_CAL_WE, "We");
	lng_addItem(lang, LNG_CAL_TH, "Th");
	lng_addItem(lang, LNG_CAL_FR, "Fr");
	lng_addItem(lang, LNG_CAL_SA, "Sa");
	lng_addItem(lang, LNG_CAL_SU, "Su");

	lng_addItem(lang, LNG_ABT_COMPILEDWITH, "CompiledWith");

	lng_addItem(lang, LNG_ON, "On");
	lng_addItem(lang, LNG_OFF, "Off");
	lng_addItem(lang, LNG_ENABLED, "Enabled");
	lng_addItem(lang, LNG_DISABLED, "Disabled");
	lng_addItem(lang, LNG_MEM, "Mem");
	lng_addItem(lang, LNG_GB, "Gb");
	lng_addItem(lang, LNG_MB, "Mb");
	
	lng_addItem(lang, LNG_CFG_PAGESTATUS, "Pagestatus");
	lng_addItem(lang, LNG_CFG_METADATA, "Metadata");
	lng_addItem(lang, LNG_CFG_MEDIAPLAYER, "Mediaplayer");
	lng_addItem(lang, LNG_CFG_PLAYLIST, "Playlist");
	lng_addItem(lang, LNG_CFG_HITLIST, "Hitlist");
	lng_addItem(lang, LNG_CFG_EQUALIZER, "Equalizer");
	lng_addItem(lang, LNG_CFG_LYRICS, "Lyrics");
	lng_addItem(lang, LNG_CFG_AMSN, "aMSN");
	lng_addItem(lang, LNG_CFG_BANDWIDTH, "Bandwidth");
	lng_addItem(lang, LNG_CFG_UTORRENT, "ęTorrent");
	lng_addItem(lang, LNG_CFG_NETSTAT, "Netstat");
	lng_addItem(lang, LNG_CFG_PROCESSLIST, "Processlist");
	lng_addItem(lang, LNG_CFG_SYSTEMINFO, "Systeminfo");
	lng_addItem(lang, LNG_CFG_ATITRAYTOOLS, "ATITrayTools");
	lng_addItem(lang, LNG_CFG_FRAPS, "Fraps");
	lng_addItem(lang, LNG_CFG_RSS20, "RSS2.0");
	lng_addItem(lang, LNG_CFG_PING, "Ping");
	lng_addItem(lang, LNG_CFG_CALENDAR, "Calendar");
	lng_addItem(lang, LNG_CFG_MYSTIFY, "Mystify");
	lng_addItem(lang, LNG_CFG_PARTICLES, "Particles");
	lng_addItem(lang, LNG_CFG_CONFIG, "Config");
	lng_addItem(lang, LNG_CFG_AUTODETECTUTF16, "AutodetectUTF16");
	lng_addItem(lang, LNG_CFG_AUTODETECTUTF8, "AutodetectUTF8");
	lng_addItem(lang, LNG_CFG_VOLUMECONTROL, "Volumecontrol");
	lng_addItem(lang, LNG_CFG_VOLUMESENSITIVITY, "Volumesensitivity");
	lng_addItem(lang, LNG_CFG_UPDATERATE, "Updaterate");
	lng_addItem(lang, LNG_CFG_GRAPH, "Graph");
	lng_addItem(lang, LNG_CFG_SPECTRUMHEIGHT, "Spectrumheight");
	lng_addItem(lang, LNG_CFG_PEAKFALLOFFRATE, "Peakfalloffrate");
	lng_addItem(lang, LNG_CFG_TRACKSEEKTIME, "Trackseektime");
	lng_addItem(lang, LNG_CFG_CHANNEL2, "Channel2");
	lng_addItem(lang, LNG_CFG_PROGRESSBAR, "Progressbar");
	lng_addItem(lang, LNG_CFG_PLAYLISTPOS, "Playlistpos");
	lng_addItem(lang, LNG_CFG_TRACKTIME, "Tracktime");
	lng_addItem(lang, LNG_CFG_TRACKKHZ, "TrackKhz");
	lng_addItem(lang, LNG_CFG_TRACKBPS, "TrackBps");
	lng_addItem(lang, LNG_CFG_VUPEAKINDICATORS, "VUpeakindicators");
	lng_addItem(lang, LNG_CFG_UNICODEFILEPATHS, "Unicodefilepaths");
	lng_addItem(lang, LNG_CFG_TRACKNUMBER, "Tracknumber");
	lng_addItem(lang, LNG_CFG_SCROLLPERCENT, "Scrollpercent");
	lng_addItem(lang, LNG_CFG_FONT, "Font");
	lng_addItem(lang, LNG_CFG_MEDIAENCODING, "Mediaencoding");
	lng_addItem(lang, LNG_CFG_OVERLAYINMAIN, "OverlayinMain");
	lng_addItem(lang, LNG_CFG_LYRICSFONT, "Lyricsfont");
	lng_addItem(lang, LNG_CFG_GLOBALTIMEOFFSET, "Globaltimeoffset");
	lng_addItem(lang, LNG_CFG_LYRICENCODING, "Lyricencoding");
	lng_addItem(lang, LNG_CFG_AUTOCONNECT, "Autoconnect");
	lng_addItem(lang, LNG_CFG_MAILNOTIFICATION, "Mailnotification");
	lng_addItem(lang, LNG_CFG_MAILNOTIFYTIMEOUT, "Mailnotifytimeout");
	lng_addItem(lang, LNG_CFG_OVERLAYICONS, "OverlayIcons");
	lng_addItem(lang, LNG_CFG_OVERLAYPAGE, "Overlaypage");
	lng_addItem(lang, LNG_CFG_ICONOVERLAYTIMEOUT, "Iconoverlaytimeout");
	lng_addItem(lang, LNG_CFG_OPENEDITBOX, "Openeditbox");
	lng_addItem(lang, LNG_CFG_EMOTICONS, "Emoticons");
	lng_addItem(lang, LNG_CFG_USERNAMEFORMAT, "Usernameformat");
	lng_addItem(lang, LNG_CFG_LANGUAGE, "Language");
	lng_addItem(lang, LNG_CFG_INTERFACE, "Interface");
	lng_addItem(lang, LNG_CFG_SPLITSCREEN, "Splitscreen");
	lng_addItem(lang, LNG_CFG_CLOCK, "Clock");
	lng_addItem(lang, LNG_CFG_FLAGS, "Flags");
	lng_addItem(lang, LNG_CFG_IPLOOKUP, "IPlookup");
	lng_addItem(lang, LNG_CFG_FREESPACEUNIT, "Freespaceunit");
	lng_addItem(lang, LNG_CFG_DISPLAYUNITS, "Displayunits");
	lng_addItem(lang, LNG_CFG_LISTREMOVABLEDRIVES, "Listremovabledrives");
	lng_addItem(lang, LNG_CFG_TEMPERATURE, "Temperature");
	lng_addItem(lang, LNG_CFG_FRAPSOVERLAY, "Frapsoverlay");
	lng_addItem(lang, LNG_CFG_OVERLAY, "Overlay");
	lng_addItem(lang, LNG_CFG_PAGE, "OVRLYPage");
	lng_addItem(lang, LNG_CFG_POSITION, "Position");
	lng_addItem(lang, LNG_CFG_FPSFONT, "FPSFont");
	lng_addItem(lang, LNG_CFG_PRINTMODE, "Printmode");
	lng_addItem(lang, LNG_CFG_G15, "G15");
	lng_addItem(lang, LNG_CFG_DANCINGLIGHTS, "Dancinglights");
	lng_addItem(lang, LNG_CFG_LCDBEAT, "LCDbeat");
	lng_addItem(lang, LNG_CFG_REVERSEVOLUMECONTROL, "Reversevolumecontrol");
	lng_addItem(lang, LNG_CFG_HANDLEMEDIAKEYS, "Handlemediakeys");
	lng_addItem(lang, LNG_CFG_LOWERBWLIMIT, "LowerBWlimit");
	lng_addItem(lang, LNG_CFG_UPPERBWLIMIT, "UpperBWlimit");
	lng_addItem(lang, LNG_CFG_LEDSCALE, "LEDscale");
	lng_addItem(lang, LNG_CFG_M1LEDLEVEL, "M1LEDlevel");
	lng_addItem(lang, LNG_CFG_M2LEDLEVEL, "M2LEDlevel");
	lng_addItem(lang, LNG_CFG_M3LEDLEVEL, "M3LEDlevel");
	lng_addItem(lang, LNG_CFG_MRLEDLEVEL, "MRLEDlevel");
	lng_addItem(lang, LNG_CFG_LCDBEATLEVEL, "LCDbeatlevel");
	lng_addItem(lang, LNG_CFG_KBLEDLEVEL1, "KBLEDlevel1");
	lng_addItem(lang, LNG_CFG_KBLEDLEVEL2, "KBLEDlevel2");
	lng_addItem(lang, LNG_CFG_GENERAL, "General");
	lng_addItem(lang, LNG_CFG_DEFAULTPAGE, "Defaultpage");
	lng_addItem(lang, LNG_CFG_PAGESWAPEFFECT, "Pageswapeffect");
	lng_addItem(lang, LNG_CFG_AUTOSWAPWHENIDLE, "Autoswapwhenidle");
	lng_addItem(lang, LNG_CFG_ROTATIONDEGREE, "Rotationdegree");
	lng_addItem(lang, LNG_CFG_CLOCKFORMAT, "Clockformat");
	lng_addItem(lang, LNG_CFG_INVERTDISPLAY, "Invertdisplay");
	lng_addItem(lang, LNG_CFG_WAITFORPLAYER, "Waitforplayer");
	lng_addItem(lang, LNG_CFG_EXITWITHPLAYER, "Exitwithplayer");
	lng_addItem(lang, LNG_CFG_PROCESSPRIORITY, "Processpriority");
	lng_addItem(lang, LNG_CFG_PROFILE, "Profile");
	
}

void lng_dumpStrings (TLANG *lang)
{
	int i;
	for (i = 0; i < lang->tItems; i++){
		dbprintf("%i '%s'\n", i, lng_getString(lang, i));
	}
}

int lng_create (TWINAMP *wvs, char *langpath)
{
	wvs->lang = lng_createTable(LNG_TOTAL);
	if (wvs->lang){
		wvs->lang->hw = wvs->hw;
		
		printf("lang %p\n", wvs->lang->hw);
		
		lng_registerStrings(wvs->lang);
		if (lng_loadLangFile(wvs->lang, langpath)){
			lng_setEncoding(wvs->lang, lng_getString(wvs->lang, LNG_ENCODING));
			//lng_dumpStrings(wvs->lang);
			return 1;
		}
	}
	return 0;
}
