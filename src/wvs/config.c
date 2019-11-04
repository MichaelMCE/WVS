
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


static void setConfigItemValue (TWINAMP *wa, int id, int value);
static TCONFIGOPT *getConfigItem (TWINAMP *wa, int id);
static int setConfigItemValues (TWINAMP *wa);
static int getConfigItemValues (TWINAMP *wa);
static float getConfigItemValueFloat (TWINAMP *wa, int id);
static ubyte *getConfigItemValueUChar (TWINAMP *wa, int id);
static int getConfigItemValueInt (TWINAMP *wa, int id);
static int getConfigItemCurrent (TWINAMP *wa, int id);
static int configItemNameToItem (TWINAMP *wa, int id, ubyte *name, size_t namelen, TCONFIGOPT **item);
static int configItemValueFindIndex (TCONFIGOPT *item, ubyte *name, int *index);
static int saveProfile (TWINAMP *wa, char *path);
int loadProfile (TWINAMP *wa, char *path);



//volatile static HANDLE thdListener = NULL;

static void toggleGroupStatus (TCONFIGPAGE *cfPage, int group)
{
	int i;
	for (i = group+1; cfPage->item[i].optTotal > 0; i++){
		if (!cfPage->item[i].flags)
			cfPage->item[i].flags = 1;
		else
			cfPage->item[i].flags = 0;
	}
}

static void getPrevConfigHL (TCONFIGPAGE *cfPage, int *start, int end, int *highlight)
{
	if (*highlight == 0){
		*start = 0;
		return;
	}
	(*highlight)--;
	if (*highlight < 0){
		*highlight = 0;
		
		if (*highlight < *start)
			(*start)--;
		return;
	}

	if ((cfPage->item[*highlight].optTotal && !cfPage->item[*highlight].flags) || (!getPageStatus(cfPage->wa, cfPage->item[*highlight].requires) && cfPage->item[*highlight].requires))
		getPrevConfigHL(cfPage, start, end, highlight);
	
	if (*highlight < *start)
		(*start)--;

	return;
}

static void getNextConfigHL (TCONFIGPAGE *cfPage, int *start, int end, int *highlight)
{
	int i;
	for (i = (*highlight)+1; cfPage->item[i].optTotal != -1; i++){
		if (getPageStatus(cfPage->wa, cfPage->item[i].requires) || !cfPage->item[i].requires){
			if (cfPage->item[i].flags || !cfPage->item[i].optTotal){
				*highlight = i;
				break;
			}
		}
	}
	if (*highlight >= end){
		for (i = (*start)+1; cfPage->item[i].optTotal != -1; i++){
			if (getPageStatus(cfPage->wa, cfPage->item[i].requires) || !cfPage->item[i].requires){
				if (cfPage->item[i].flags || !cfPage->item[i].optTotal){
					*start = i;
					return;
				}
			}
		}
	}
	return;
}

static void getPrevConfigOpt (TCONFIGPAGE *cfPage, int item)
{
	 if (cfPage->item[item].optTotal < 1)
	 	return;
	 if (cfPage->item[item].current > 0)
	 	cfPage->item[item].current--;
}

static void getNextConfigOpt (TCONFIGPAGE *cfPage, int item)
{
	 if (cfPage->item[item].optTotal < 1)
	 	return;
	 	
	 if (cfPage->item[item].current < cfPage->item[item].optTotal-1)
	 	cfPage->item[item].current++;

	 return;
}

static TCONFIGOPT *getConfigItem (TWINAMP *wa, int id)
{
	int i = 0;
	do{
		if (wa->cfPage.item[i].id == id)
			return &wa->cfPage.item[i];
	}while(wa->cfPage.item[++i].optTotal != -1);

	//printf("getConfigItem(): invalid id %i\n",id);
	return NULL;
}

static void setConfigItemValue (TWINAMP *wa, int id, int value)
{
	TCONFIGOPT *item =  getConfigItem(wa, id);
	if (item != NULL)
		item->current = value;
}

static int getConfigItemValueInt (TWINAMP *wa, int id)
{
	TCONFIGOPT *item = getConfigItem(wa, id);
	if (item)
		return atoi((char*)item->opt[item->current]);
	else
		return 0;
}

static char *getConfigItemValueChar (TWINAMP *wa, int id)
{
	TCONFIGOPT *item = getConfigItem(wa, id);
	if (item)
		return item->opt[item->current];
	else
		return " ";
}

static ubyte *getConfigItemValueUChar (TWINAMP *wa, int id)
{
	TCONFIGOPT *item = getConfigItem(wa, id);
	if (item)
		return (ubyte*)item->opt[item->current];
	else
		return (ubyte*)" ";
}

static float getConfigItemValueFloat (TWINAMP *wa, int id)
{
	TCONFIGOPT *item = getConfigItem(wa, id);
	if (item)
		return atof((char*)item->opt[item->current]);
	else
		return 0.0;
}

static int getConfigItemIsOnYesEn (TWINAMP *wa, int id)
{

	TCONFIGOPT *item = getConfigItem(wa, id);
	if (item)
		return isYes((char*)item->opt[item->current]);
	else
		return 0;
}

static int getConfigItemCurrent (TWINAMP *wa, int id)
{
	TCONFIGOPT *item = getConfigItem(wa, id);
	if (item)
		return item->current;
	else
		return 0;
}

static int getConfigItemValues (TWINAMP *wa)
{
	setPageStatus(wa, DISF_PLAYLIST, getConfigItemIsOnYesEn(wa, CONF_PLAYLIST));
	setPageStatus(wa, DISF_HITLIST, getConfigItemIsOnYesEn(wa, CONF_HITLIST));	
	setPageStatus(wa, DISF_MAIN, getConfigItemIsOnYesEn(wa, CONF_MEDIAPLAYER));
	setPageStatus(wa, DISF_META, getConfigItemIsOnYesEn(wa, CONF_META));
	setPageStatus(wa, DISF_BANDWIDTH, getConfigItemIsOnYesEn(wa, CONF_BANDWIDTH));
	setPageStatus(wa, DISF_EQ, getConfigItemIsOnYesEn(wa, CONF_EQ));
	setPageStatus(wa, DISF_LYRICS, getConfigItemIsOnYesEn(wa, CONF_LYRICS));
	setPageStatus(wa, DISF_NETSTAT, getConfigItemIsOnYesEn(wa, CONF_NETSTAT));
	setPageStatus(wa, DISF_UTORRENT, getConfigItemIsOnYesEn(wa, CONF_UTORRENT));
	setPageStatus(wa, DISF_PROCLIST, getConfigItemIsOnYesEn(wa, CONF_PROCLIST));
	setPageStatus(wa, DISF_SYSINFO, getConfigItemIsOnYesEn(wa, CONF_SYSINFO));
	setPageStatus(wa, DISF_ATT, getConfigItemIsOnYesEn(wa, CONF_ATT));
	setPageStatus(wa, DISF_FRAPS, getConfigItemIsOnYesEn(wa, CONF_FRAPS));
	setPageStatus(wa, DISF_CALENDAR, getConfigItemIsOnYesEn(wa, CONF_CALENDAR));
	setPageStatus(wa, DISF_MYSTIFY, getConfigItemIsOnYesEn(wa, CONF_MYSTIFY));
	setPageStatus(wa, DISF_PARTICLES, getConfigItemIsOnYesEn(wa, CONF_PARTICLES));
	setPageStatus(wa, DISF_CONFIG, getConfigItemIsOnYesEn(wa, CONF_CONFIG));
	setPageStatus(wa, DISF_ABOUT, getConfigItemIsOnYesEn(wa, CONF_ABOUT));
	//setPageStatus(wa, DISF_ABOUT, 1); // about is always on
	setPageStatus(wa, DISF_PING, getConfigItemIsOnYesEn(wa, CONF_PING));
	setPageStatus(wa, DISF_MSN, getConfigItemIsOnYesEn(wa, CONF_MSN));
	
	if (!getPageStatus(wa, DISF_RSS) && getConfigItemIsOnYesEn(wa, CONF_RSS))
		signalPageOpen(wa, DISF_RSS);
	setPageStatus(wa, DISF_RSS, getConfigItemIsOnYesEn(wa, CONF_RSS));

	wa->config.volumeCtrl = getConfigItemCurrent(wa, CONF_VOLUMECTRL);
	wa->config.trackSkipTime = getConfigItemValueInt(wa, CONF_TRACKSEEK);
	wa->config.encoding = lEncodingAliasToID(wa->hw, getConfigItemValueChar(wa, CONF_ENCODING));
	wa->lyricsEncoding = lEncodingAliasToID(wa->hw, getConfigItemValueChar(wa, CONF_LYRICSENCODING));
	wa->overlay.trackPos = getConfigItemIsOnYesEn(wa, CONF_PROGBAR);
	wa->overlay.trackInfo = getConfigItemIsOnYesEn(wa, CONF_TRACKINFO);
	wa->overlay.playlistPos = getConfigItemIsOnYesEn(wa, CONF_TRKPLAYLISTPOS);
	wa->overlay.trackKhz = getConfigItemIsOnYesEn(wa, CONF_TRACKKHZ);
	wa->overlay.trackBps = getConfigItemIsOnYesEn(wa, CONF_TRACKBPS);
	wa->overlay.vupeak = getConfigItemIsOnYesEn(wa, CONF_VUPEAK);
	//wa->overlay.scroll = getConfigItemIsOnYesEn(wa, CONF_SCROLLTITLE);
	wa->overlay.invert = getConfigItemIsOnYesEn(wa, CONF_INVERT);
	wa->overlay.channel2 = getConfigItemIsOnYesEn(wa, CONF_CHANNEL2);
	wa->overlay.trackNumber = getConfigItemIsOnYesEn(wa, CONF_TRACKNUMBER);
	wa->playlist.fontType = getConfigItemCurrent(wa, CONF_FONTSIZE);
	wa->overlay.bwClock = getConfigItemIsOnYesEn(wa, CONF_BWCLOCK);
	wa->overlay.bwSplit = getConfigItemIsOnYesEn(wa, CONF_BWSPLITMODE);
	wa->graph.fallOff = getConfigItemCurrent(wa, CONF_PEAKFALLOFF)+12;
	wa->graph.height = getConfigItemCurrent(wa, CONF_GRAPHHEIGHT)+10;
	
	int fps = getConfigItemCurrent(wa, CONF_GRAPHFPS)+10;
	if (fps != wa->graph.fps){
		if (fps >= 10 && fps <= 40){
			killGraphTimer(wa);
			initGraphTimer(wa, fps);

		}
	}

	wa->playlist.scrollpercent = getConfigItemValueInt(wa, CONF_SCROLLPERCENT);
	wa->graph.barType = getConfigItemCurrent(wa, CONF_GRAPH)+1;
	wa->config.exitWithPlayer = getConfigItemIsOnYesEn(wa, CONF_EXITWITHPLAYER);
	wa->config.waitForPlayer = getConfigItemIsOnYesEn(wa, CONF_WAITFORPLAYER);
	wa->config.autoUTF16 = getConfigItemIsOnYesEn(wa, CONF_AUTOUTF16);
	wa->config.autoUTF8 = getConfigItemIsOnYesEn(wa, CONF_AUTOUTF8);
	wa->config.autoPageSwap = getConfigItemIsOnYesEn(wa, CONF_AUTOPAGESWAP);
	wa->overlay.clockFormat = getConfigItemCurrent(wa, CONF_CLOCKFORMAT);
	wa->config.startPage = getConfigItemCurrent(wa, CONF_DEFAULTPAGE)+1;
	wa->config.atitemp = getConfigItemCurrent(wa, CONF_ATTTEMP);
	wa->config.processPriority = getConfigItemCurrent(wa, CONF_PROCESSPRIORITY);
	if (wa->config.processPriority > 4) wa->config.processPriority = 0;
	
	SetPriorityClass(GetCurrentProcess(), WVSProcessPriority[wa->config.processPriority]);
	setFrapsFontType(wa, getConfigItemCurrent(wa, CONF_FRAPSFONT));

	wa->fraps.fps.value = -1; // force overlay to change font, if required
	wa->overlay.frapsFPS = getConfigItemCurrent(wa, CONF_FRAPSOVERLAY);
	wa->fraps.distination = getConfigItemCurrent(wa, CONF_FRAPSDEST);

	wa->fraps.position = getConfigItemCurrent(wa, CONF_FRAPSPOS);
	wa->fraps.copyMode = getConfigItemCurrent(wa, CONF_FRAPSCOPYMODE);
	if (wa->fraps.copyMode == 0)
		wa->fraps.copyMode = LCASS_CPY;
	else if (wa->fraps.copyMode == 1)
		wa->fraps.copyMode = LCASS_OR;
	else
		wa->fraps.copyMode = LCASS_XOR;


	aMSN_SetMailOvrTimeout(getConfigItemValueInt(wa, CONF_AMSNMAILTIMEOUT));
	aMSN_SetIconOvrTimeout(getConfigItemValueInt(wa, CONF_AMSNICONTIMEOUT));
	aMSN_SetOverlayFrame(getConfigItemCurrent(wa, CONF_AMSNOVRDEST));
	aMSN_SetInputKey(getConfigItemCurrent(wa, CONF_AMSNIMEKEY));
	g_msn.emoticons = getConfigItemIsOnYesEn(wa, CONF_AMSNSEMOTICONS);
	g_msn.autoConnect = getConfigItemIsOnYesEn(wa, CONF_AMSNAUTOCONN);
	g_msn.mailNotify = getConfigItemIsOnYesEn(wa, CONF_AMSNMAILNOTIFY);
	g_msn.iconOverlay = getConfigItemIsOnYesEn(wa, CONF_AMSNOVRICONS);
	g_msn.nameFormat = getConfigItemCurrent(wa, CONF_AMSNNAMEFORMAT);
	g_msn.encoding = lEncodingAliasToID(wa->hw, getConfigItemValueChar(wa, CONF_AMSNENCODING));
	wa->config.alwaysUseUTF8 = getConfigItemIsOnYesEn(wa, CONF_METAFILENAME);
	//wa->net.protocol = getConfigItemCurrent(wa, CONF_IPPROTOCOL);
	wa->config.sysInfoUnit = getConfigItemCurrent(wa, CONF_SYSINFOUNIT);
	wa->config.sysInfoShowUnit = getConfigItemIsOnYesEn(wa, CONF_SYSISHOWUNIT);
	wa->config.sysInfoShowRemovable = getConfigItemIsOnYesEn(wa, CONF_SYSISHOWREMOVE);
	wa->config.pageSwapMode = getConfigItemCurrent(wa, CONF_SWAPEFFECT);
	wa->config.rotateFlipStep = getConfigItemCurrent(wa, CONF_ROTATESTEP);
	wa->config.g15LEDScale = getConfigItemValueFloat(wa, CONF_LEDSCALE);
	wa->config.g15LEDUpperRange = getConfigItemValueFloat(wa, CONF_BWUPPER);
	wa->config.g15LEDLowerRange = getConfigItemValueFloat(wa, CONF_BWLOWER);
	wa->config.ledlevel[1] = getConfigItemValueFloat(wa, CONF_M1LEDL);
	wa->config.ledlevel[2] = getConfigItemValueFloat(wa, CONF_M2LEDL);
	wa->config.ledlevel[3] = getConfigItemValueFloat(wa, CONF_M3LEDL);
	wa->config.ledlevel[4] = getConfigItemValueFloat(wa, CONF_MRLEDL);
	wa->config.ledlevel[5] = getConfigItemValueFloat(wa, CONF_LCDBEATLEVEL);
	wa->config.ledkblevel[1] = getConfigItemValueFloat(wa, CONF_KBLEDL1);
	wa->config.ledkblevel[2] = getConfigItemValueFloat(wa, CONF_KBLEDL2);

	wa->overlay.lcdBeat = getConfigItemIsOnYesEn(wa, CONF_LCDBEAT);
	wa->config.handleMediaKeys = getConfigItemIsOnYesEn(wa, CONF_MEDIAKEYS);
	wa->config.reverseVolumeCtrl = getConfigItemIsOnYesEn(wa, CONF_VOLUMEINVERT);
	wa->config.volumeSensi = getConfigItemCurrent(wa, CONF_VOLUMESENSI);
	wa->lyricsFont = getConfigItemCurrent(wa, CONF_LYRICSFONTSIZE);
	wa->lyricsStampOffset = getConfigItemValueInt(wa, CONF_LYRICSSOFFSET);
	wa->overlay.lyricsOvrlay = getConfigItemIsOnYesEn(wa, CONF_LYRICSSOVERLAY);

	lyricsOverlaySetStatus(wa->overlay.lyricsOvrlay);

	wa->overlay.g15Lights = getConfigItemCurrent(wa, CONF_DANCINGLIGHTS);
	if (wa->overlay.g15Lights == 1)
		wa->overlay.g15Lights = 0x01|0x02;
	else if (wa->overlay.g15Lights == 2)
		wa->overlay.g15Lights = 0x01;
	else if (wa->overlay.g15Lights == 3)
		wa->overlay.g15Lights = 0x02;
	
	wa->overlay.g15LightsBackup = wa->overlay.g15Lights;
	if (wa->overlay.g15Lights && wa->g15.hDev == NULL){
		wa->g15.hDev = hid_g15_open(wa->g15.deviceNumber);
		//if (wa->g15.hDev == NULL)
			//wa->overlay.g15Lights = 0;
	}
	nsOption.itemFlags = getConfigItemValueInt(wa, CONF_NETSTATCFG);
	nsOption.ipLookup = getConfigItemIsOnYesEn(wa, CONF_NETSTATIPLOOKUP);
	int item = getConfigItemValueInt(wa, CONF_BWINTERFACE)-1;
	if (wa->config.bwinterface != item)
		bandwidthSetIntfIndex(item);
	wa->config.bwinterface = item;

	closeMixers(wa);
	initMixerDevice(wa, VOLUME_MASTER);
	initMixerDevice(wa, VOLUME_WAVE);
	if (getConfigItemCurrent(wa, CONF_PROFILE) == 1)
		saveProfile(wa, (char*)WAPROFILE);
	else if (getConfigItemCurrent(wa, CONF_PROFILE) == 2)
		return 1;

	return 0;
}

static int encodingToIndex (int encoding)
{
	int i;
	for (i = 0; i < WVSENCODINGLISTTOTAL; i++){
		if (EncTable[i] == encoding)
			return i;
	}
	return encodingToIndex(CMT_ISO8859_1);
}

static int trackSkipTToIndex (int time)
{
	int i;
	for (i = 0; i < 12; i++){
		if (time <= TrackSkipTable[i])
			return i;
	}
	return 0;
}

static int setConfigItemValues (TWINAMP *wa)
{
	setConfigItemValue(wa, CONF_PLAYLIST, getPageStatus(wa, DISF_PLAYLIST));
	setConfigItemValue(wa, CONF_HITLIST, getPageStatus(wa, DISF_HITLIST));
	setConfigItemValue(wa, CONF_META, getPageStatus(wa, DISF_META));
	setConfigItemValue(wa, CONF_MEDIAPLAYER, getPageStatus(wa, DISF_MAIN));
	setConfigItemValue(wa, CONF_BANDWIDTH, getPageStatus(wa, DISF_BANDWIDTH));
	setConfigItemValue(wa, CONF_EQ, getPageStatus(wa, DISF_EQ));
	setConfigItemValue(wa, CONF_LYRICS, getPageStatus(wa, DISF_LYRICS));
	setConfigItemValue(wa, CONF_PROCLIST, getPageStatus(wa, DISF_PROCLIST));
	setConfigItemValue(wa, CONF_NETSTAT, getPageStatus(wa, DISF_NETSTAT));
	setConfigItemValue(wa, CONF_UTORRENT, getPageStatus(wa, DISF_UTORRENT));
	setConfigItemValue(wa, CONF_SYSINFO, getPageStatus(wa, DISF_SYSINFO));
	setConfigItemValue(wa, CONF_ATT, getPageStatus(wa, DISF_ATT));
	setConfigItemValue(wa, CONF_FRAPS, getPageStatus(wa, DISF_FRAPS));
	setConfigItemValue(wa, CONF_CALENDAR, getPageStatus(wa, DISF_CALENDAR));
	setConfigItemValue(wa, CONF_RSS, getPageStatus(wa, DISF_RSS));
	setConfigItemValue(wa, CONF_MYSTIFY, getPageStatus(wa, DISF_MYSTIFY));
	setConfigItemValue(wa, CONF_PARTICLES, getPageStatus(wa, DISF_PARTICLES));
	setConfigItemValue(wa, CONF_CONFIG, getPageStatus(wa, DISF_CONFIG));
	setConfigItemValue(wa, CONF_ABOUT, getPageStatus(wa, DISF_ABOUT));
	setConfigItemValue(wa, CONF_PING, getPageStatus(wa, DISF_PING));
	setConfigItemValue(wa, CONF_MSN, getPageStatus(wa, DISF_MSN));
	setConfigItemValue(wa, CONF_TRACKSEEK, trackSkipTToIndex(wa->config.trackSkipTime));
	setConfigItemValue(wa, CONF_ENCODING, encodingToIndex(wa->config.encoding));
	
	setConfigItemValue(wa, CONF_LYRICSENCODING, encodingToIndex(wa->lyricsEncoding));
	
	setConfigItemValue(wa, CONF_VOLUMECTRL, wa->config.volumeCtrl);
	setConfigItemValue(wa, CONF_GRAPH, wa->graph.barType-1);
	setConfigItemValue(wa, CONF_PROGBAR, wa->overlay.trackPos);
	setConfigItemValue(wa, CONF_TRACKINFO, wa->overlay.trackInfo);
	setConfigItemValue(wa, CONF_TRKPLAYLISTPOS, wa->overlay.playlistPos);
	
	setConfigItemValue(wa, CONF_TRACKKHZ, wa->overlay.trackKhz);
	setConfigItemValue(wa, CONF_TRACKBPS, wa->overlay.trackBps);
	setConfigItemValue(wa, CONF_VUPEAK, wa->overlay.vupeak);
	//setConfigItemValue(wa, CONF_SCROLLTITLE, wa->overlay.scroll);
	setConfigItemValue(wa, CONF_INVERT, wa->overlay.invert);
	setConfigItemValue(wa, CONF_CHANNEL2, wa->overlay.channel2);
	setConfigItemValue(wa, CONF_PEAKFALLOFF, wa->graph.fallOff-12);
	setConfigItemValue(wa, CONF_GRAPHHEIGHT, wa->graph.height-10);
	
	setConfigItemValue(wa, CONF_GRAPHFPS, wa->graph.fps-10);
	setConfigItemValue(wa, CONF_SCROLLPERCENT, wa->playlist.scrollpercent-1);
	setConfigItemValue(wa, CONF_TRACKNUMBER, wa->overlay.trackNumber);
	setConfigItemValue(wa, CONF_FONTSIZE, wa->playlist.fontType);
	setConfigItemValue(wa, CONF_BWCLOCK, wa->overlay.bwClock);
	setConfigItemValue(wa, CONF_BWSPLITMODE, wa->overlay.bwSplit);
	setConfigItemValue(wa, CONF_EXITWITHPLAYER, wa->config.exitWithPlayer);
	setConfigItemValue(wa, CONF_WAITFORPLAYER, wa->config.waitForPlayer);
	setConfigItemValue(wa, CONF_AUTOUTF16, wa->config.autoUTF16);
	setConfigItemValue(wa, CONF_AUTOUTF8, wa->config.autoUTF8);
	setConfigItemValue(wa, CONF_AUTOPAGESWAP, wa->config.autoPageSwap);
	setConfigItemValue(wa, CONF_DEFAULTPAGE, wa->config.startPage-1);
	setConfigItemValue(wa, CONF_CLOCKFORMAT, wa->overlay.clockFormat);
	setConfigItemValue(wa, CONF_ATTTEMP, wa->config.atitemp);
	setConfigItemValue(wa, CONF_FRAPSFONT, getFrapsFontType(wa));
	wa->fraps.fps.value = -1; // force overlay to change font, if required
	setConfigItemValue(wa, CONF_FRAPSOVERLAY, wa->overlay.frapsFPS);
	setConfigItemValue(wa, CONF_FRAPSDEST, wa->fraps.distination);
	setConfigItemValue(wa, CONF_FRAPSPOS, wa->fraps.position);
	
	setConfigItemValue(wa, CONF_AMSNMAILTIMEOUT, aMSN_OvrTimeoutToIndex(aMSN_GetMailOvrTimeout()));
	setConfigItemValue(wa, CONF_AMSNICONTIMEOUT, aMSN_OvrTimeoutToIndex(aMSN_GetIconOvrTimeout()));
	setConfigItemValue(wa, CONF_AMSNOVRDEST, aMSN_GetOverlayFrame());
	setConfigItemValue(wa, CONF_AMSNIMEKEY, aMSN_GetInputKey());
	setConfigItemValue(wa, CONF_AMSNSEMOTICONS, g_msn.emoticons);
	setConfigItemValue(wa, CONF_AMSNAUTOCONN, g_msn.autoConnect);
	setConfigItemValue(wa, CONF_AMSNMAILNOTIFY, g_msn.mailNotify);
	setConfigItemValue(wa, CONF_AMSNOVRICONS, g_msn.iconOverlay); 
	setConfigItemValue(wa, CONF_AMSNNAMEFORMAT, g_msn.nameFormat);
	setConfigItemValue(wa, CONF_AMSNENCODING, encodingToIndex(g_msn.encoding));
	
	setConfigItemValue(wa, CONF_LYRICSSOFFSET, lyricOffsetToIdx(wa->lyricsStampOffset));
	
	if (wa->fraps.copyMode == LCASS_CPY)
		setConfigItemValue(wa, CONF_FRAPSCOPYMODE, 0);
	else if (wa->fraps.copyMode == LCASS_OR)
		setConfigItemValue(wa, CONF_FRAPSCOPYMODE, 1);
	else
		setConfigItemValue(wa, CONF_FRAPSCOPYMODE, 2);


	setConfigItemValue(wa, CONF_METAFILENAME, wa->config.alwaysUseUTF8);
	//setConfigItemValue(wa, CONF_IPPROTOCOL, wa->net.protocol);
	setConfigItemValue(wa, CONF_SYSINFOUNIT, wa->config.sysInfoUnit);
	setConfigItemValue(wa, CONF_SYSISHOWUNIT, wa->config.sysInfoShowUnit);
	setConfigItemValue(wa, CONF_SYSISHOWREMOVE, wa->config.sysInfoShowRemovable);
	setConfigItemValue(wa, CONF_SWAPEFFECT, wa->config.pageSwapMode);
	setConfigItemValue(wa, CONF_VOLUMESENSI, wa->config.volumeSensi);
	setConfigItemValue(wa, CONF_ROTATESTEP, wa->config.rotateFlipStep);
	
	setConfigItemValue(wa, CONF_LYRICSFONTSIZE, wa->lyricsFont);
	
	setConfigItemValue(wa, CONF_LCDBEAT, wa->overlay.lcdBeat);
	setConfigItemValue(wa, CONF_MEDIAKEYS, wa->config.handleMediaKeys);
	setConfigItemValue(wa, CONF_VOLUMEINVERT, wa->config.reverseVolumeCtrl);
	setConfigItemValue(wa, CONF_LEDSCALE, ((int)wa->config.g15LEDScale/10)-1);
	memset(wa->g15.upperBand, 0, sizeof(LightUpperBand));
	memset(wa->g15.lowerBand, 0, sizeof(LightLowerBand));
	itoa(wa->config.g15LEDUpperRange, wa->g15.upperBand, 10);
	itoa(wa->config.g15LEDLowerRange, wa->g15.lowerBand, 10);
	
	setConfigItemValue(wa, CONF_LYRICSSOVERLAY, wa->overlay.lyricsOvrlay);

	int g15 = 0;
	if (wa->overlay.g15LightsBackup == 0x03)
		g15 = 1;
	else if (wa->overlay.g15LightsBackup == 0x01)
		g15 = 2;
	else if (wa->overlay.g15LightsBackup == 0x02)
		g15 = 3;
	setConfigItemValue(wa, CONF_DANCINGLIGHTS, g15);

	
	memset(NetStatFlags, 0, sizeof(NetStatFlags));
	itoa(nsOption.itemFlags, NetStatFlags, 10);
	
	setConfigItemValue(wa, CONF_NETSTATIPLOOKUP, nsOption.ipLookup);
	setConfigItemValue(wa, CONF_M1LEDL, (((int)wa->config.ledlevel[1]-10)/5));
	setConfigItemValue(wa, CONF_M2LEDL, (((int)wa->config.ledlevel[2]-15)/5));
	setConfigItemValue(wa, CONF_M3LEDL, (((int)wa->config.ledlevel[3]-20)/5));
	setConfigItemValue(wa, CONF_MRLEDL, (((int)wa->config.ledlevel[4]-25)/5));
	setConfigItemValue(wa, CONF_LCDBEATLEVEL, (((int)wa->config.ledlevel[5]-70)/10));
	setConfigItemValue(wa, CONF_KBLEDL1, (((int)wa->config.ledkblevel[1]-25)/5));
	setConfigItemValue(wa, CONF_KBLEDL2, (((int)wa->config.ledkblevel[2]-25)/5));
	setConfigItemValue(wa, CONF_PROCESSPRIORITY, wa->config.processPriority);
	setConfigItemValue(wa, CONF_BWINTERFACE, wa->config.bwinterface);

	if (bandwidthGetTotalInf() <= 10)	// CONF_BWINTERFACE item contains 10 values
		getConfigItem(wa, CONF_BWINTERFACE)->optTotal = bandwidthGetTotalInf();
	else
		getConfigItem(wa, CONF_BWINTERFACE)->optTotal = 10;

	setConfigItemValue(wa, CONF_PROFILE, 0);
	return 1;
}

static int configDecomposeLine (ubyte *line, ubyte **name, ubyte **value)
{
	*name = (ubyte*)strtok((char*)line, "=");
	if (*name){
		*value = (ubyte*)strtok(NULL, "\0");
		if (*value){
			return 1;
		}
	}
	return 0;
}

static int configItemNameToItem (TWINAMP *wa, int id, ubyte *name, size_t namelen, TCONFIGOPT **item)
{
	if (!namelen)
		return 0;

	*item = getConfigItem(wa, id);
	if (*item){
		if (namelen == strlen((char*)(*item)->name)){
			if (!strnicmp((char*)(*item)->name, (char*)name, namelen))
				return 1;
		}
	}
	return 0;
}

static int configItemValueFindIndex (TCONFIGOPT *item, ubyte *name, int *index)
{
	int len = strlen((char*)name);
	for (*index = 0; *index < item->optTotal; (*index)++){
		if (!strnicmp((char*)item->opt[*index], (char*)name, len))
			return 1;
	}
	return 0;
}

/*
unsigned int __stdcall rssLoad (TWINAMP *wa)
{
	signalPageOpen(wa, DISF_RSS);
	thdListener	= NULL;
	_endthreadex(1);
	return 1;
}*/

// awful function, must rewrite config handling
int loadProfile (TWINAMP *wa, char *path)
{
	
#if defined(__DEBUG__)	
	dbprintf("loading profile: '%s'\n", path);
#endif
	
	TASCIILINE *al = readFileA(path);
	if (al == NULL){
		dbprintf("WVS: profile (%s) not found\n", path);
		return 0;
	}
	
	ubyte *line = NULL;
	ubyte *name = NULL;
	ubyte *value = NULL;
	TCONFIGOPT *item = NULL;
	int index = 0;
	int i = 0;
	size_t namelen;
	

	do{
		line = al->lines[i];
		if (line == NULL)
			break;

		if (*line == '-') break;

		if ((*line != '[') && (*line != ';')){
			if (configDecomposeLine(line, &name, &value)){
				if (name)
					namelen = strlen((char*)name);
				else
					namelen = 0;

				if (configItemNameToItem(wa, CONF_PLAYLIST, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_PLAYLIST, index);

				}else if (configItemNameToItem(wa, CONF_HITLIST, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_HITLIST, index);

				}else if (configItemNameToItem(wa, CONF_META, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_META, index);

				}else if (configItemNameToItem(wa, CONF_MEDIAPLAYER, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_MAIN, index);

				}else if (configItemNameToItem(wa, CONF_BANDWIDTH, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_BANDWIDTH, index);

				}else if (configItemNameToItem(wa, CONF_EQ, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_EQ, index);

				}else if (configItemNameToItem(wa, CONF_LYRICS, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_LYRICS, index);

				}else if (configItemNameToItem(wa, CONF_PROCLIST, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_PROCLIST, index);

				}else if (configItemNameToItem(wa, CONF_SYSINFO, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_SYSINFO, index);

				}else if (configItemNameToItem(wa, CONF_NETSTAT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_NETSTAT, index);
						
				}else if (configItemNameToItem(wa, CONF_UTORRENT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_UTORRENT, index);

				}else if (configItemNameToItem(wa, CONF_FRAPS, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_FRAPS, index);

				}else if (configItemNameToItem(wa, CONF_ATT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_ATT, index);
				
				}else if (configItemNameToItem(wa, CONF_CALENDAR, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_CALENDAR, index);		

				}else if (configItemNameToItem(wa, CONF_RSS, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index)){
						//if (!getPageStatus(wa, DISF_RSS) && index){
							//if (thdListener == NULL){
							//	//unsigned int threadID = 0;
							//	//thdListener = (HANDLE)_beginthreadex(NULL, 0, (void*)rssLoad, (void*)wa, 0, &threadID);
							//	signalPageOpen(wa, DISF_RSS);
							//}
						//}
						setPageStatus(wa, DISF_RSS, index);
					}
				}else if (configItemNameToItem(wa, CONF_MYSTIFY, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_MYSTIFY, index);
						
				}else if (configItemNameToItem(wa, CONF_PARTICLES, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_PARTICLES, index);
				
				}else if (configItemNameToItem(wa, CONF_ABOUT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_ABOUT, index);
				
				}else if (configItemNameToItem(wa, CONF_PING, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_PING, index);
						
				}else if (configItemNameToItem(wa, CONF_MSN, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_MSN, index);
						
				}else if (configItemNameToItem(wa, CONF_CONFIG, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setPageStatus(wa, DISF_CONFIG, index);

				}else if (configItemNameToItem(wa, CONF_AMSNIMEKEY, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						aMSN_SetInputKey(index);
						
				}else if (configItemNameToItem(wa, CONF_VOLUMECTRL, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->config.volumeCtrl = index;

				}else if (configItemNameToItem(wa, CONF_TRACKSEEK, name, namelen, &item)){
					if (value)
						wa->config.trackSkipTime = atoi((char*)value);

				}else if (configItemNameToItem(wa, CONF_LYRICSENCODING, name, namelen, &item)){
					if (value){
						wa->lyricsEncoding = lEncodingAliasToID(wa->hw, (char*)value);
					}
					
				}else if (configItemNameToItem(wa, CONF_ENCODING, name, namelen, &item)){
					if (value){
						wa->config.encoding = lEncodingAliasToID(wa->hw, (char*)value);
						lSetCharacterEncoding(wa->hw, wa->config.encoding);
					}

				}else if (configItemNameToItem(wa, CONF_AMSNMAILTIMEOUT, name, namelen, &item)){
					if (value)
						aMSN_SetMailOvrTimeout(atoi((char*)value));

				}else if (configItemNameToItem(wa, CONF_AMSNICONTIMEOUT, name, namelen, &item)){
					if (value)
						aMSN_SetIconOvrTimeout(atoi((char*)value));

				}else if (configItemNameToItem(wa, CONF_AMSNENCODING, name, namelen, &item)){
					if (value){
						g_msn.encoding = lEncodingAliasToID(wa->hw, (char*)value);
						//lSetCharacterEncoding(wa->hw, g_msn.encoding);
					}

				}else if (configItemNameToItem(wa, CONF_GRAPH, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->graph.barType = index+1;
				
				}else if (configItemNameToItem(wa, CONF_PROGBAR, name, namelen, &item)){
					if (value)
						wa->overlay.trackPos = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_TRKPLAYLISTPOS, name, namelen, &item)){
					if (value)
						wa->overlay.playlistPos = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_TRACKINFO, name, namelen, &item)){
					if (value)
						wa->overlay.trackInfo = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_TRACKKHZ, name, namelen, &item)){
					if (value)
						wa->overlay.trackKhz = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_VUPEAK, name, namelen, &item)){
					if (value)
						wa->overlay.vupeak = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_TRACKBPS, name, namelen, &item)){
					if (value)
						wa->overlay.trackBps = isYes((char*)value);
						
				/*
				}else if (configItemNameToItem(wa, CONF_SCROLLTITLE, name, namelen, &item)){
					if (value)
						wa->overlay.scroll = isYes((char*)value);
				*/	
				}else if (configItemNameToItem(wa, CONF_INVERT, name, namelen, &item)){
					if (value)
						wa->overlay.invert = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_CHANNEL2, name, namelen, &item)){
					if (value)
						wa->overlay.channel2 = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_SCROLLPERCENT, name, namelen, &item)){
					if (value)
						wa->playlist.scrollpercent = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_GRAPHHEIGHT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->graph.height = index+10;

				}else if (configItemNameToItem(wa, CONF_GRAPHFPS, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index)){
						int fps = index+10;
						if (fps != wa->graph.fps){
							if (fps >= 10 && fps <= 40){
								//wa->graph.fps = fps;
								killGraphTimer(wa);
								initGraphTimer(wa, fps);
							}
						}
					}

				}else if (configItemNameToItem(wa, CONF_PEAKFALLOFF, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->graph.fallOff = index+12;

				}else if (configItemNameToItem(wa, CONF_TRACKNUMBER, name, namelen, &item)){
					if (value)
						wa->overlay.trackNumber = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_FONTSIZE, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->playlist.fontType = index;

				}else if (configItemNameToItem(wa, CONF_BWSPLITMODE, name, namelen, &item)){
					if (value)
						wa->overlay.bwSplit = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_BWCLOCK, name, namelen, &item)){
					if (value)
						wa->overlay.bwClock = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_DEFAULTPAGE, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->config.startPage = index+1;

				}else if (configItemNameToItem(wa, CONF_PROCESSPRIORITY, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index)){
						if (index > 4) index = 0;
						wa->config.processPriority = index;
						SetPriorityClass(GetCurrentProcess(), WVSProcessPriority[index]);
					}

				}else if (configItemNameToItem(wa, CONF_CLOCKFORMAT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->overlay.clockFormat = index;

				}else if (configItemNameToItem(wa, CONF_AMSNAUTOCONN, name, namelen, &item)){
					if (value)
						g_msn.autoConnect = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_AMSNMAILNOTIFY, name, namelen, &item)){
					if (value)
						g_msn.mailNotify = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_AMSNOVRICONS, name, namelen, &item)){
					if (value)
						g_msn.iconOverlay = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_AMSNSEMOTICONS, name, namelen, &item)){
					if (value)
						g_msn.emoticons = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_AUTOPAGESWAP, name, namelen, &item)){
					if (value)
						wa->config.autoPageSwap = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_AUTOUTF8, name, namelen, &item)){
					if (value)
						wa->config.autoUTF8 = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_AUTOUTF16, name, namelen, &item)){
					if (value)
						wa->config.autoUTF16 = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_EXITWITHPLAYER, name, namelen, &item)){
					if (value)
						wa->config.exitWithPlayer = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_WAITFORPLAYER, name, namelen, &item)){
					if (value)
						wa->config.waitForPlayer = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_SYSISHOWREMOVE, name, namelen, &item)){
					if (value)
						wa->config.sysInfoShowRemovable = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_METAFILENAME, name, namelen, &item)){
					if (value)
						wa->config.alwaysUseUTF8 = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_SYSISHOWUNIT, name, namelen, &item)){
					if (value)
						wa->config.sysInfoShowUnit = isYes((char*)value);
				/*
				}else if (configItemNameToItem(wa, CONF_IPPROTOCOL, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->net.protocol = index;
						if (wa->net.cfgProto < 0)
							wa->net.cfgProto = wa->net.protocol;
				*/
				}else if (configItemNameToItem(wa, CONF_SYSINFOUNIT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->config.sysInfoUnit = index;

				}else if (configItemNameToItem(wa, CONF_ATTTEMP, name, namelen, &item)){
					if (value){
						if (*value == 'c' || *value == 'C')
							wa->config.atitemp = 0;
					}

				}else if (configItemNameToItem(wa, CONF_FRAPSOVERLAY, name, namelen, &item)){
					if (value)
						wa->overlay.frapsFPS = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_FRAPSFONT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						setFrapsFontType(wa, index);

				}else if (configItemNameToItem(wa, CONF_FRAPSDEST, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->fraps.distination = index;

				}else if (configItemNameToItem(wa, CONF_FRAPSPOS, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->fraps.position = index;

				}else if (configItemNameToItem(wa, CONF_FRAPSCOPYMODE, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index)){
						if (index == 0)
							wa->fraps.copyMode = LCASS_CPY;
						else if (index == 1)
							wa->fraps.copyMode = LCASS_OR;
						else
							wa->fraps.copyMode = LCASS_XOR;
					}

				}else if (configItemNameToItem(wa, CONF_LYRICSFONTSIZE, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->lyricsFont = index;
	
				}else if (configItemNameToItem(wa, CONF_VOLUMESENSI, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->config.volumeSensi = index;
					
				}else if (configItemNameToItem(wa, CONF_SWAPEFFECT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->config.pageSwapMode = index;

				}else if (configItemNameToItem(wa, CONF_ROTATESTEP, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						wa->config.rotateFlipStep = index;

				}else if (configItemNameToItem(wa, CONF_LYRICSSOFFSET, name, namelen, &item)){
					if (value)
						wa->lyricsStampOffset = atoi((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_LYRICSSOVERLAY, name, namelen, &item)){
					if (value){
						wa->overlay.lyricsOvrlay = isYes((char*)value);
						lyricsOverlaySetStatus(wa->overlay.lyricsOvrlay);
					}

				}else if (configItemNameToItem(wa, CONF_LCDBEAT, name, namelen, &item)){
					if (value)
						wa->overlay.lcdBeat = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_AMSNNAMEFORMAT, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						g_msn.nameFormat = index;

				}else if (configItemNameToItem(wa, CONF_AMSNOVRDEST, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index))
						aMSN_SetOverlayFrame(index);
						
				}else if (configItemNameToItem(wa, CONF_VOLUMEINVERT, name, namelen, &item)){
					if (value)
						wa->config.reverseVolumeCtrl = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_MEDIAKEYS, name, namelen, &item)){
					if (value)
						wa->config.handleMediaKeys = isYes((char*)value);

				}else if (configItemNameToItem(wa, CONF_NETSTATIPLOOKUP, name, namelen, &item)){
					if (value)
						nsOption.ipLookup = isYes((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_DANCINGLIGHTS, name, namelen, &item)){
					if (configItemValueFindIndex(item, value, &index)){
						wa->overlay.g15Lights = index&0x03;
						if (wa->overlay.g15Lights == 1)
							wa->overlay.g15Lights = 0x01|0x02;
						else if (wa->overlay.g15Lights == 2)
							wa->overlay.g15Lights = 0x01;
						else if (wa->overlay.g15Lights == 3)
							wa->overlay.g15Lights = 0x02;
					
						wa->overlay.g15LightsBackup = wa->overlay.g15Lights;
						if (wa->overlay.g15Lights && wa->g15.hDev == NULL){
							wa->g15.hDev = hid_g15_open(wa->g15.deviceNumber);
							//if (wa->g15.hDev == NULL)
								//wa->overlay.g15Lights = 0;
						}
					}
				}else if (configItemNameToItem(wa, CONF_BWLOWER, name, namelen, &item)){
					if (value)
						wa->config.g15LEDLowerRange = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_BWUPPER, name, namelen, &item)){
					if (value)
						wa->config.g15LEDUpperRange = atof((char*)value);
											
				}else if (configItemNameToItem(wa, CONF_LEDSCALE, name, namelen, &item)){
					if (value)
						wa->config.g15LEDScale = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_M1LEDL, name, namelen, &item)){
					if (value)
						wa->config.ledlevel[1] = atof((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_M2LEDL, name, namelen, &item)){
					if (value)
						wa->config.ledlevel[2] = atof((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_M3LEDL, name, namelen, &item)){
					if (value)
						wa->config.ledlevel[3] = atof((char*)value);
						
				}else if (configItemNameToItem(wa, CONF_MRLEDL, name, namelen, &item)){
					if (value)
						wa->config.ledlevel[4] = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_LCDBEATLEVEL, name, namelen, &item)){
					if (value)
						wa->config.ledlevel[5] = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_KBLEDL1, name, namelen, &item)){
					if (value)
						wa->config.ledkblevel[1] = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_KBLEDL2, name, namelen, &item)){
					if (value)
						wa->config.ledkblevel[2] = atof((char*)value);

				}else if (configItemNameToItem(wa, CONF_NETSTATCFG, name, namelen, &item)){
					if (value)
						nsOption.itemFlags = atoi((char*)value);

				}else if (configItemNameToItem(wa, CONF_BWINTERFACE, name, namelen, &item)){
					if (value){
						int item = atoi((char*)value)-1;
						if (item < bandwidthGetTotalInf()){
							if (item != wa->config.bwinterface)
								bandwidthSetIntfIndex(item);
							wa->config.bwinterface = item;
						}
					}
				}
			}
		}
	}while(++i < al->tlines);

	freeASCIILINE(al);
	return 1;
}

static int saveProfile (TWINAMP *wa, char *path)
{
	FILE *hFile = fopen(path, "wb");
	if (hFile == NULL){
		//printf("WVS: profile %s not saved\n", path);
		return 0;
	}
	fprintf(hFile,"; All fields and values are case sensitive\r\n");
	
	TCONFIGOPT *item = &wa->cfPage.item[0];
	do{
		if (!item->optTotal)
			fprintf(hFile,"\r\n[%s]\r\n",item->name);
		else
			fprintf(hFile,"%s=%s\r\n",item->name, (char*)getConfigItemValueUChar(wa, item->id));

		item++;
	}while(item->optTotal != -1 && (item->id != CONF_LAST));
	
	fclose(hFile);
	return 1;
}

char *cfgStrN (TWINAMP *wa, TCONFIGPAGE *cfPage, int index)
{
	return lng_getString(wa->lang, cfPage->item[index].lid);
}

int renderConfig (TWINAMP *wa, TFRAME *frame, void *data)
{
	TCONFIGPAGE *cfPage = (TCONFIGPAGE *)data;
	int start = cfPage->start;
	TLPRINTR rect = {1,-1,frame->width-1,frame->height-1,0,0,0,0};
	const int font = LFTW_SNAP;
	
	lClearFrame(frame);

	do{
		if (getPageStatus(wa, cfPage->item[start].requires) || !cfPage->item[start].requires){
			lSetCharacterEncoding(wa->hw, wa->lang->enc);
		if (!cfPage->item[start].optTotal){			// group title
				if (!lPrintEx(frame, &rect, font, PF_NEWLINE|PF_RESETX|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, cfgStrN(wa, cfPage, start)))
					break;
			}else if (cfPage->item[start].flags){	// item name
				if (!lPrintEx(frame, &rect, font, PF_NEWLINE|PF_RESETX, LPRT_CPY, cfgStrN(wa, cfPage, start)))
					break;
			}

			// highlight selected item
			if ((start == cfPage->highlight) && !cfPage->hlmode)
				lDrawRectangleFilled(frame, rect.sx, rect.sy+1, rect.ex, rect.ey, LSP_XOR);

			if (cfPage->item[start].optTotal && cfPage->item[start].flags){
				if (cfPage->item[start].current > cfPage->item[start].optTotal-1)
					cfPage->item[start].current = cfPage->item[start].optTotal-1;

				lSetCharacterEncoding(wa->hw, CMT_CP850); // MSDOS code page needed for the micro/'u' symbol
				lPrintEx(frame, &rect, font, PF_RIGHTJUSTIFY|PF_CLIPWRAP, LPRT_CPY, cfPage->item[start].opt[cfPage->item[start].current]);
				if (start == cfPage->highlight && cfPage->hlmode)
					lDrawRectangleFilled(frame, rect.sx, rect.sy+1, rect.ex, rect.ey, LSP_XOR);
				rect.ey++; // add another line between rows
			}
		}
		start++;
	}while((rect.ey < frame->height-8) && (cfPage->item[start].optTotal != -1));

	cfPage->end = start;
	return 1;
}

int displayInput_Config (TWINAMP *wa, int key, void *data)
{
	TCONFIGPAGE *cfPage = (TCONFIGPAGE *)data;

	if (isKeyPressed(VK_LSHIFT)){
		return -1;
		
	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		if (!cfPage->hlmode)
			getPrevConfigHL(cfPage, &cfPage->start, cfPage->end, &cfPage->highlight);
		else
			getPrevConfigOpt(cfPage, cfPage->highlight);
		renderFrame(wa, DISF_CONFIG);

	}else if (key == G15_WHEEL_CLOCKWISE){
		if (!cfPage->hlmode)
			getNextConfigHL(cfPage, &cfPage->start, cfPage->end, &cfPage->highlight);
		else
			getNextConfigOpt(cfPage, cfPage->highlight);
		renderFrame(wa, DISF_CONFIG);

	}else if (key == G15_SOFTKEY_3){
		if (!cfPage->item[cfPage->highlight].optTotal){
			toggleGroupStatus(cfPage, cfPage->highlight);
		}else{
			if (!cfPage->hlmode)
				cfPage->hlmode = 1;
			else
				cfPage->hlmode = 0;
		}
		renderFrame(wa, DISF_CONFIG);

	}else if (key == G15_SOFTKEY_4){
		if (getConfigItemValues(wa)){
			loadProfile(wa, (char*)WAPROFILE);
			setConfigItemValues(wa);
			getConfigItemValues(wa);
		}
	}else{
		return -1;
	}
	return 0;
}


int cfgPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	//dbprintf("cfgPageRender\n");
	return renderConfig(wa, frame, userPtr);
}

int cfgPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Config(wa, key, userPtr);
}

int cfgPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("cfgPageEnter\n");
	TCONFIGPAGE *cfPage = (TCONFIGPAGE*)userPtr;
	setConfigItemValues(cfPage->wa);
	return 1;
}

void cfgPageExit (void *userPtr)
{
	//dbprintf("cfgPageExit\n");
}

int cfgPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("cfgPageOpen()\n");
	return 1;
}

void cfgPageClose (void *userPtr)
{
	//dbprintf("cfgPageClose()\n");
}
