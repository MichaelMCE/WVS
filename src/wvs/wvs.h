
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


#include "langids.h"


#if defined (BUILDING_DLL)
#define WACONFIG	"wvs/wvs.cfg"
#define WAPROFILE	"wvs/profile.cfg"
#define MYLCDCONFIG	"wvs/mylcd.cfg"
#define HITLISTNAME	"wvs/hitlist"
#define DEFAULTLANG	"wvs/lang/english.txt"

#else

#define WACONFIG	"wvs.cfg"
#define WAPROFILE	"profile.cfg"
#define MYLCDCONFIG	"mylcd.cfg"
#define HITLISTNAME	"hitlist"
#define DEFAULTLANG	"lang/english.txt"
#endif

#define HITLISTEXT	"m3u"


#define MPSEARCHTIMEOUT 60000
#define KEEPALIVETIMER	4000

#define G15_SOFTKEY_1 1025
#define G15_SOFTKEY_2 1026
#define G15_SOFTKEY_3 1028
#define G15_SOFTKEY_4 1032

#define VOLUME_DECREASE 1
#define VOLUME_INCREASE 2

#define METATAGTOTAL 30

char szClassName[] = "you_shouldn't_see_me";

#define NO_SOURCE ((MIXERLINE_COMPONENTTYPE_SRC_LAST + 1))

#define MSNSIZE_EMAILADDRESS		256
#define MSNSIZE_REMOTEPASSWORD		64

#define NETREADBUFFERSIZE 65536

#define SWAPSTARTTIME	40000
#define SWAPINTERVAL	7000

#define RENDER_FOCUS	1		/*render page only when it has focus*/
#define RENDER_ALWAYS	2		/*render page whether it has focus or not*/

#define NEWSTRINGTYPE	LFRM_BPP_32
#define LCPY_OR	LCASS_OR
#define LCPY_CPY LCASS_CPY
#define LCPY_XOR LCASS_XOR


#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))


enum _frapsPosition {
	FRAPSPOS_TOPLEFT = 0,
	FRAPSPOS_TOPRIGHT,
	FRAPSPOS_BOTTOMRIGHT,
	FRAPSPOS_BOTTOMLEFT
};

enum _mixer {
	VOLUME_MASTER = 0,
	VOLUME_WAVE,
	VOLUME_MEDIAPLAYER
};

#define MIXER_TOTAL	2		// master and wave, do not use VOLUME_MEDIAPLAYER as an index

enum _disframe {
	DISF_ACTIVEFRAME = 0,
	DISF_FIRST = 1,
	DISF_META = DISF_FIRST,
	DISF_MAIN,
	DISF_PLAYLIST,
	DISF_HITLIST,
	DISF_EQ,
	DISF_LYRICS,
	DISF_MSN,
	DISF_BANDWIDTH,
	DISF_UTORRENT,
	DISF_NETSTAT,
	DISF_PROCLIST,
	DISF_SYSINFO,
	DISF_ATT,
	DISF_FRAPS,
	DISF_RSS,
	DISF_PING,
	DISF_CALENDAR,
	DISF_MYSTIFY,
	DISF_PARTICLES,
	DISF_ABOUT,
	DISF_CONFIG,
	DISF_TOTAL = DISF_CONFIG
};

enum _configitem {
	CONF_FIRST = 1,
	CONF_META = CONF_FIRST,	// page status
	CONF_MEDIAPLAYER,
	CONF_PLAYLIST,
	CONF_HITLIST,
	CONF_EQ,
	CONF_LYRICS,
	CONF_MSN,
	CONF_BANDWIDTH,
	CONF_UTORRENT,
	CONF_NETSTAT,
	CONF_PROCLIST,
	CONF_SYSINFO,
	CONF_ATT,
	CONF_FRAPS,
	CONF_RSS,
	CONF_PING,
	CONF_CALENDAR,
	CONF_MYSTIFY,
	CONF_PARTICLES,
	CONF_ABOUT,
	CONF_CONFIG,
	
	CONF_METAFILENAME,			// meta tag page

	CONF_AUTOUTF16,				// title page
	CONF_AUTOUTF8,
	CONF_VOLUMECTRL,
	CONF_VOLUMESENSI,
	CONF_GRAPH,
	CONF_PROGBAR,
	CONF_TRACKSEEK,
	CONF_TRACKINFO,
	CONF_TRKPLAYLISTPOS,
	CONF_TRACKKHZ,
	CONF_TRACKBPS,
	CONF_VUPEAK,
	CONF_SCROLLTITLE,
	CONF_CHANNEL2,
	CONF_PEAKFALLOFF,
	CONF_GRAPHHEIGHT,
	CONF_GRAPHFPS,
	
	CONF_TRACKNUMBER,			// playlist page
	CONF_SCROLLPERCENT,
	CONF_FONTSIZE,
	CONF_ENCODING,
	
	CONF_LYRICSFONTSIZE,		// lyrics page
	CONF_LYRICSSOFFSET,
	CONF_LYRICSSOVERLAY,
	CONF_LYRICSENCODING,
	
	CONF_AMSNAUTOCONN,			// aMSN page
	CONF_AMSNMAILNOTIFY,
	CONF_AMSNMAILTIMEOUT,
	CONF_AMSNOVRICONS,
	CONF_AMSNOVRDEST,
	CONF_AMSNICONTIMEOUT,
	CONF_AMSNIMEKEY,
	CONF_AMSNSEMOTICONS,
	CONF_AMSNNAMEFORMAT,
	CONF_AMSNENCODING,
		
	CONF_BWINTERFACE,			// bandwidth page
	CONF_BWCLOCK,
	CONF_BWSPLITMODE,
	
	CONF_NETSTATCFG,			// netstat page
	CONF_NETSTATIPLOOKUP,
	
	CONF_ATTTEMP,				// ati tray tools page
	
	CONF_FRAPSOVERLAY,			// Fraps overlay config
	CONF_FRAPSDEST,
	CONF_FRAPSFONT,
	CONF_FRAPSPOS,
	CONF_FRAPSCOPYMODE,

	CONF_SYSINFOUNIT,			// system info page
	CONF_SYSISHOWUNIT,
	CONF_SYSISHOWREMOVE,
	
	CONF_DANCINGLIGHTS,
	CONF_LCDBEAT,
	CONF_LEDSCALE,
	CONF_BWUPPER,
	CONF_BWLOWER,	
	CONF_M1LEDL,
	CONF_M2LEDL,
	CONF_M3LEDL,
	CONF_MRLEDL,
	CONF_KBLEDL1,
	CONF_KBLEDL2,
	CONF_LCDBEATLEVEL,
	CONF_VOLUMEINVERT,
	CONF_MEDIAKEYS,

	CONF_EXITWITHPLAYER,			// general page
	CONF_WAITFORPLAYER,
	CONF_DEFAULTPAGE,
	CONF_AUTOPAGESWAP,
	CONF_SWAPEFFECT,
	CONF_ROTATESTEP,
	CONF_CLOCKFORMAT,
	CONF_INVERT,
	CONF_IPPROTOCOL,
	CONF_PROCESSPRIORITY,
	CONF_PROFILE,
	CONF_LAST = CONF_PROFILE
};


typedef struct{
	unsigned int trackPos:1;
	unsigned int trackInfo:1;
	unsigned int trackKhz:1;
	unsigned int trackBps:1;
	unsigned int trackSeek:1;
	unsigned int playlistPos:1;
	unsigned int volumeLevel:1;
	unsigned int vupeak:1;
	unsigned int scroll:1;
	unsigned int channel2:1;
	unsigned int invert:1;
	unsigned int spectMarkers:1;
	unsigned int trackNumber:1;
	unsigned int bwClock:1;
	unsigned int bwSplit:1;
	unsigned int trackFav:2;
	unsigned int frapsFPS:1;
	unsigned int lcdBeat:1;
	unsigned int clockFormat:1;
	unsigned int g15Lights:2;
	unsigned int g15LightsBackup:2;
	unsigned int lyricsOvrlay:1;
	unsigned int filler:7;
}TDISSTATE;

typedef struct{
	T2POINT *chan1;
	//T2POINT *chan2;
}TOSC;

typedef struct{
	TOSC osc;
	ubyte *spectrum;
	ubyte *spectrum4;
	unsigned int updateCount;
	int	barType;
	int	pixelMode;
	int	x;
	int	y;
	int	width;
	int	height;
	int	fallOff;
	volatile int timerID;
	int	fps;		// update rate
	int chMax[2];
}TWAGRAPH;

typedef struct{
	int	samRate;
	int bitRate;
	int trackNumber;
	int trackPosition;
	int trackLength;
	int titleScroll;
	int bandwidthClock;
	int bandwidthValue;
	int	procList;
	int rss;
	int about;
	int frapsFPS;
}TFONTS;

typedef struct{
	//int	hour;
	//int	minute;
	//int	second;
	//int	lastsecond;
	clock_t	tmillisecond;
}TCLOCK;

typedef struct{
	TFRAME	*text;
	int		value;
}TTEXT;

typedef struct{
	TTEXT	length;		// length of current track, 5:38
	TTEXT	position;	// play position, 3:45
	TTEXT	number;		// track n of , 'track n/34'
	TTEXT	bitRate;	// bit rate, '256kbps'
	TTEXT	sampleRate;	// sample rate, '44khz'
	TTEXT	seekPosition;	// eg, "< 55% >"
	TTEXT	favorite;	// favorite overlay in main
	TTEXT	Channels;
}TTRACK;

typedef struct{
	//char		text[4096];
	uint32_t	adler;		// checksum of title
	TFRAME		*frame;
	TLSCROLLEX	*s;
	clock_t		time;
}TWASCROLL;

typedef struct{
	ubyte	*url;
	clock_t	time;
}TWARSS;

typedef struct{
	char *username;
	char *password;
	int port;
}TUTORRENTCFG;

typedef struct{
	TFONTS		font;
	int			encoding;	// gb2312, euc-jp, etc..
	int			scrollX;	// title scroll x offset
	int			scrollY;	// title scroll y offset
	int			graphH;		// graph height
	TDISSTATE	overlay;	// UI state
	TWARSS		rss;
	int			mystifyNodes;
	int			rssScrollPeriod;
	int			trackTitleScrollPeriod;
	int			rssUpdatePeriod;
	float		rotateFlipStep;
	int			tparticles;
	int			trackSkipTime;
	int			volumeTimeout;
	int			pageSwapMode;
	int			exitWithPlayer;
	int			waitForPlayer;
	int			atitemp;
	int			sysInfoUnit;
	int			g15LEDLowerRange;
	int			g15LEDUpperRange;
	int			volumeCtrl;
	int			volumeSensi;
	int			bwinterface;
	float		graphAudioScale;	// bargraph sensitivity
	float		g15LEDScale;		// led scale
	float		ledlevel[6];		// 
	float		ledkblevel[3];
	int			startPage;
	int			sysInfoShowUnit;
	int			sysInfoShowRemovable;
	int			processPriority;
	int			autoUTF16;
	int			autoUTF8;
	int			mediaPlayDisabled;		// 1=media player pages disabled
	int			handleMediaKeys;
	int			reverseVolumeCtrl;
	int 		alwaysUseUTF8;	// always use utf8 for filepaths in meta tag page
	int			autoPageSwap;
	TUTORRENTCFG utorrent;
	char		*langpath;
}TWACONFIG;

struct _TWINAMP;

typedef struct {
	int			start;
	int			end;
	int			highlightPos;	// currently highlighted item in playlist
	int			fontType;		// 0/small, 1/medium or 2/large(cjkv)
	int			scrollpercent;
	int			totalTracks;	// number of tracks in playlist
	
	int			type;			// list type. 0=mediaplayer playlist, 1=hitlist 1 ... 9=hitlist 9
	TASCIILINE	*al;
	FILE		*hFile;

	// playlist scroller	
	int enabled;
	TFRAME *frame;
	TLSCROLLEX	*scroll;
	struct _TWINAMP *wa;
	
	/* cached scroller info*/
	uint32_t adler;
	unsigned int track;
	unsigned int ttracks;
	unsigned int time0;
	int font;
	int enc;
	/* end */
}TPLAYLIST;

typedef struct{
	int (*input) (struct _TWINAMP *wa, int key, void *userPtr);
	int (*render) (struct _TWINAMP *wa, TFRAME *frame, void *userPtr);
	int (*open) (TFRAME *frame, void *userPtr);	// page initialization 
	void (*close) (void *userPtr);		// page shutdown
	int (*enter) (TFRAME *frame, void *userPtr);		// called once before entering page
	void (*exit) (void *userPtr);		// called once just after leaving page
	int flags;
	unsigned int uperiod;	// time between render proc calls
}TPAGEFUNCS;

typedef struct{
	TFRAME *frame;					// display surface, front
	TFRAME *work;					// display surface, back
	char *title;					// page title
	void *userPtr;					// somewhere for page to user its data.
	TPAGEFUNCS proc;
	int pageID;
	int dependsID;					// page requires or depends upon dependsID being activated before this
	unsigned int lastRenderTime;	// time of last render
	unsigned int isEnabled:1;		// 1 if page is enabled - proc's are valid
	unsigned int isOpen:1;			// 1 if open proc has been called
	unsigned int isClosed:1;		// 1 if close proc has been called
	unsigned int isRegistered:1;
	unsigned int padding:28;
}TDISPAGE;

typedef struct{
	float		 angle;
	int			 direction;		// rotate direction, 1 = forward/right, 2 = backward/left. 0 = not moving/current
	int			 directionLast; // last direction
	int			 total;
	unsigned int index;
	unsigned int indexLast;
	unsigned int lastSwapTime;
	unsigned int curSwapTime;
	TDISPAGE	 page[DISF_TOTAL+1];
}TFRAMEREG;

typedef struct{
	int			min;
	int			max;
	int			step;
}TWAMIXRANGE;

typedef struct{
	int			status;			// 1=enabled, 0=disabled
	int			devicei;		// hardware device index, defaults to first found
	MSG			messages;
	HMIXER		mixerHandle;
	DWORD		dwControlID;
	DWORD 		dwLineID;
	int			currentVolume;
	TWAMIXRANGE range;
	unsigned int volumeTime;	// ms time since last switched on
	ubyte		name[64];
}TWAMIXER;

typedef struct{
	int		highlight;
	int		columnMode;
	int		modStart;
	int		modTotal;
	int		subMode;
	int		killp;
}TPROCLIST;

typedef struct{
	int			status;			// is set to 1 if g15 is connected and in use
	lDISPLAY	id;				// mylcd id
	HANDLE		hDev;			// HID handle
	int			total[2];		// wave form level sum per channel
	float		ledlevel[2];	// audio bar level per channel
	int			lastledlevel;	// last bar level
	int			lastkblevel;	// last kb level
	int			beatXOffset;
	int			beatYOffset;
	int			deviceNumber;	// g15 device to open, 1'st, 2'nd, 3'rd, etc..
	char		*upperBand;
	char		*lowerBand;
	char		MLeds;			// somewhere to store user led preferences
	char		KBLeds;
	char		LCDC;
	volatile unsigned int key;	// contains last unhandled softkey (press)
	char		states[8];
}TG15GENERAL;

typedef struct{
	unsigned int winampVolumeTime;
	unsigned int lag;
	int slideOffset;
	int hitlistr;
	volatile int updateSignal;
	int seeked;
	unsigned int keyCheckToggle;
	volatile unsigned int timerUpdateCounter;
	int lastKnownPlayState;

	volatile int engineTimerID;
	volatile int IUTimerID;

	int tPlaylists;
}TMISC;

typedef struct{
	char *name;
	int	lid;			// lang id
	unsigned short requires;
	unsigned short flags;
	int id;
	unsigned int optTotal;
	unsigned int current;	// highlighted, active or selected item
	char *opt[32];
}TCONFIGOPT;

typedef struct{
	int start;
	int end;
	int highlight;
	int hlmode;
	TCONFIGOPT *item;
	struct _TWINAMP *wa;
}TCONFIGPAGE;

typedef struct {
	int v1;
	int v2;
	int v3;
	int vMP;
}TVERSION;

typedef struct {
	ubyte	*address;
	int		port;
	HANDLE	mpHandle;
	ubyte	mpName[128];
	TMPNET conn;
	TVERSION version;
	int aLiveTimer;
	int protocol;
	int cfgProto;	// records protocol config setting on first load
	volatile int notFound;	// 1=media player not available, 0=media player alive and well
	int foundOnce;	// is set to 1 once media player has been found, is not reset thereafter
	HANDLE thdListener;
	char *buffer;
	int bufferSize;
	
	TWVSPAKCACHE cache;
	TMPCMDWAVE wave;
	TMPCMD cmd;
	TMPGENTRACKINFO gen;
	TMPCMDEQ eq;

	unsigned int readCt;
	unsigned int timeOut;
}TWVSNET;	

typedef struct{
	ubyte *channel1;
	ubyte *channel2;
}TWAVEDATA;

typedef struct{
	int track;
	int tag;
}TMETAPAGE;

typedef struct{
	HANDLE	hwnd;
    MSG		messages;
}TKEYHOOK;

typedef struct {
	TTEXT	fps;
	ubyte	position;		// overlay position in frame; top left, rop right, etc..
	ubyte	copyMode;		// LCASS_xxx  (LCASS_OR or LCASS_CPY)
	ubyte	distination;	// DISF_xxx or 0, 0 = all
	ubyte	fontType;		// 0,1,2
	int		FPS;
}TFRAPS;

typedef struct {
	ubyte *address;
}TWVSPING;

typedef struct {
	int mode;
	int bar;
	int direction;
	int getnewdata;
}TEQ;

typedef struct {
	char email[MSNSIZE_EMAILADDRESS+4];
	char remotePassword[MSNSIZE_REMOTEPASSWORD+4];
}TAMSNACCOUNT;

typedef struct {
	volatile unsigned int t0;		// start time in ms.
	volatile unsigned int requestedTime; // total time in ms.
	TFRAME *frame;
	volatile int x;
	volatile int y;					// x,y; display location within distination frame
	volatile ubyte distination;		// distination page. 0=active page only, 1=first page only, ..., 255=display in all pages
	volatile ubyte copyMode;		// copy type
	volatile ubyte ownerCleans:1;	// who handles frame deletion. 1=creator, 0=auto delete after timeout
	volatile ubyte status:1;		// 1=struct is valid, 0=invalid struct - can be [re]used
	volatile ubyte reuse:1;			// 1=struct contains new overlay, 0=update existing overlay
	volatile ubyte ownerSetsT0:1;	// 1=caller sets t0 time (GetTickCount()), 0=wvs sets base time.
	volatile ubyte pad:4;
}TFRAMEOVER;

typedef struct {
	volatile TFRAMEOVER *overlay[16];
	int count;
}TPOPUP;

#define MSNSIZE_WORKINGBUFFERS		10
#define MSNSIZE_INPUTBUFFER			512

typedef struct{
	char buffers[MSNSIZE_WORKINGBUFFERS][MSNSIZE_INPUTBUFFER+4];
	char caretBuffer[MSNSIZE_INPUTBUFFER+4];
	char workingBuffer[MSNSIZE_INPUTBUFFER+4];
	
	int historyBufferi;
	int caretChar;
	int caretPos;
	size_t tKeys;
	size_t iOffset;
}TWVSINPUT, TMSNINPUT;
	
typedef struct {
	int valid;		// 1=this ocntains valid data, 0=invalid
	int playlist;	// playlist index (0 to totalplaylists-1)
	int track;		// index in to playlist (0 to totaltracks-1)
	int	type;		// 1=wchar_t*, 2=char*
	char *buffer;
	u64	time;
}TCACHEITEM;

#define CACHEITEMSIZE	512

typedef struct {
	TCACHEITEM item[CACHEITEMSIZE];
}TDATACACHE;

#define MAXITEMLENGTH	32

typedef struct{
	int id;
	char item[MAXITEMLENGTH];
	char *entry;
}TLNGITEM;

typedef struct{
	int enc;		// myLCD CMT_xxxx encoding reference except utf16/32
	char *filepath;
	TLNGITEM *itmTbl;
	int	tItems;
	
	THWD *hw;
}TLANG;

typedef struct _TWINAMP {
	TPOPUP		popup;
	TWASCROLL	scroll;
	TCLOCK		clock;
	TTRACK		track;
	TFONTS		font;	//legacy
	TWAGRAPH	graph;
	TDISSTATE	overlay;
	TWACONFIG	config;
	TPLAYLIST	playlist;
	TPLAYLIST	hitlist;
	TPROCLIST	proclist;
	TFRAMEREG	dis;
	TWAMIXER	mixer[MIXER_TOTAL+10];
	TG15GENERAL	g15;
	TCONFIGPAGE cfPage;
	TMISC		misc;
	TKEYHOOK	hook;
	TWAVEDATA	spectrum;
	TWAVEDATA	wave;
	TMETAPAGE	meta;
	TFRAPS		fraps;
	TWVSPING	ping;
	TEQ			eq;
	TAMSNACCOUNT amsn;
	TLANG		*lang;

	volatile int state;		// runtime state
	TWVSNET		net;
	TDATACACHE	cache;
	HANDLE cacheLock;
	TFRAME *clone;
	wchar_t *wvsPath;
	
	char *lyricsLocation;	// UTF8 path, location of lyrics folder without trailing slash. eg: g:\lyrics
	int lyricsFont;
	int lyricsStampOffset;
	int lyricsEncoding;
	int lyricsDismode;
	
	THWD *hw;
}TWINAMP;


#define WVSFONTSSTRSTOTAL	8
//                  	2016   1001   1003   2037   2043    1013     2001          2010
#define WVSFONTSSTRS	"5x7", "7x6", "8x8", "8x9", "8x16", "10x12", "14x16(CJK)", "18x18(CJK)"

#define WVSPAGELISTENG "Meta tags", "Title", "Playlist", "Hitlist", "Equalizer", "Lyrics", "aMSN", "Bandwidth",  "ÊTorrent", "Netstat", "Proclist",     "Sysinfo", "ATT", "Fraps", "RSS", "Ping", "Calendar", "Mystify", "Particles", "About", "Config"
#define WVSPAGELISTTOTAL DISF_TOTAL

#define WVSENCODINGLISTTOTAL 26
#define WVSENCODINGLIST "BIG5", "GBK", "GB-2312", "ZH_CN", "EUC-JP", "SJIS", "EUC-KR",\
						"UTF8", "CP1250", "CP1251", "TIS-620", "8859-1", "8859-2",\
						"8859-3", "8859-4", "8859-5", "8859-6", "8859-7", "8859-8", "8859-9",\
						"8859-10", "8859-11", "8859-13", "8859-14", "8859-15", "8859-16"
						

char CENABLED[] = "Enabled";
char CDISABLED[] = "Disabled";
char CON[] = "On";
char COFF[] = "Off";
char CYES[] = "Yes";
char CNO[] = "No";


char LightUpperBand[8];
char LightLowerBand[8];
char NetStatFlags[8];


TCONFIGOPT configopt[] = {
	{"Page status", LNG_CFG_PAGESTATUS, 0, 0, 0, 0, 0},
	{"Meta data", LNG_CFG_METADATA, 0, 0, CONF_META, 2, 0, {CDISABLED, CENABLED}},
	{"Media player", LNG_CFG_MEDIAPLAYER, 0, 0, CONF_MEDIAPLAYER, 2, 0, {CDISABLED, CENABLED}},
	{"Playlist", LNG_CFG_PLAYLIST, 0, 0, CONF_PLAYLIST, 2, 0, {CDISABLED, CENABLED}},
	{"Hitlist", LNG_CFG_HITLIST, 0, 0, CONF_HITLIST, 2, 0, {CDISABLED, CENABLED}},
	{"Equalizer", LNG_CFG_EQUALIZER, 0, 0, CONF_EQ, 2, 0, {CDISABLED, CENABLED}},
	{"Lyrics", LNG_CFG_LYRICS, 0, 0, CONF_LYRICS, 2, 0, {CDISABLED, CENABLED}},
	{"aMSN", LNG_CFG_AMSN, 0, 0, CONF_MSN, 2, 0, {CDISABLED, CENABLED}},
	{"Bandwidth", LNG_CFG_BANDWIDTH, 0, 0, CONF_BANDWIDTH , 2, 0, {CDISABLED, CENABLED}},
	{"ÊTorrent", LNG_CFG_UTORRENT, 0, 0, CONF_UTORRENT , 2, 0, {CDISABLED, CENABLED}},
	{"Netstat", LNG_CFG_NETSTAT, 0, 0, CONF_NETSTAT, 2, 0, {CDISABLED, CENABLED}},
	{"Process list", LNG_CFG_PROCESSLIST, 0, 0, CONF_PROCLIST, 2, 0, {CDISABLED, CENABLED}},
	{"System info", LNG_CFG_SYSTEMINFO, 0, 0, CONF_SYSINFO, 2, 0, {CDISABLED, CENABLED}},
	{"ATI Tray Tools", LNG_CFG_ATITRAYTOOLS, 0, 0, CONF_ATT, 2, 0, {CDISABLED, CENABLED}},
	{"Fraps", LNG_CFG_FRAPS, 0, 0, CONF_FRAPS, 2, 0, {CDISABLED, CENABLED}},
	{"RSS 2.0", LNG_CFG_RSS20, 0, 0, CONF_RSS, 2, 0, {CDISABLED, CENABLED}},
	{"Ping", LNG_CFG_PING, 0, 0, CONF_PING, 2, 0, {CDISABLED, CENABLED}},
	{"Calendar", LNG_CFG_CALENDAR, 0, 0, CONF_CALENDAR, 2, 0, {CDISABLED, CENABLED}},
	{"Mystify", LNG_CFG_MYSTIFY, 0, 0, CONF_MYSTIFY , 2, 0, {CDISABLED, CENABLED}},
	{"Particles", LNG_CFG_PARTICLES, 0, 0, CONF_PARTICLES, 2, 0, {CDISABLED, CENABLED}},
	{"About", LNG_CFG_ABOUT, 0, 0, CONF_ABOUT, 2, 0, {CDISABLED, CENABLED}},
	{"Config", LNG_CFG_CONFIG, 0, 0, CONF_CONFIG, 2, 0, {CDISABLED, CENABLED}},

	{"Media Player", LNG_CFG_MEDIAPLAYER, DISF_MAIN, 0, 0, 0, 0},
	{"Auto detect UTF16", LNG_CFG_AUTODETECTUTF16, DISF_MAIN, 0, CONF_AUTOUTF16, 2, 0, {CNO, CYES}},
	{"Auto detect UTF8", LNG_CFG_AUTODETECTUTF8, DISF_MAIN, 0, CONF_AUTOUTF8, 2, 0, {CNO, CYES}},
	{"Volume control", LNG_CFG_VOLUMECONTROL, DISF_MAIN, 0, CONF_VOLUMECTRL , 3, 0, {"Master", "Wave", "Player"}},
	{"Volume sensitivity", LNG_CFG_VOLUMESENSITIVITY, DISF_MAIN, 0, CONF_VOLUMESENSI , 3, 0, {"Low", "Medium", "High"}},
	{"Update rate", LNG_CFG_UPDATERATE, DISF_MAIN, 0, CONF_GRAPHFPS, 31, 0, {"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31" ,"32" ,"33" ,"34" ,"35" ,"36" ,"37" ,"38" ,"39" ,"40" }},
	{"Graph", LNG_CFG_GRAPH, DISF_MAIN, 0, CONF_GRAPH , 7, 0, {"Spectrum", "Bar", "Osc 1", "Osc 2", "Osc 3", "VU", "None"}},
	{"Spectrum height", LNG_CFG_SPECTRUMHEIGHT, DISF_MAIN, 0, CONF_GRAPHHEIGHT , 21, 0, {"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30"}},
	{"Peak falloff rate", LNG_CFG_PEAKFALLOFFRATE, DISF_MAIN, 0, CONF_PEAKFALLOFF , 21, 0, {"12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"}},
	{"Track seek time", LNG_CFG_TRACKSEEKTIME, DISF_MAIN, 0, CONF_TRACKSEEK , 12, 0, {"500ms", "1000ms", "1500ms", "2000ms", "3000ms", "4000ms", "5000ms", "6000ms", "7000ms", "10000ms", "20000ms", "60000ms"}},
	{"Channel 2", LNG_CFG_CHANNEL2, DISF_MAIN, 0, CONF_CHANNEL2 , 2, 0, {COFF, CON}},
	{"Progress bar", LNG_CFG_PROGRESSBAR, DISF_MAIN, 0, CONF_PROGBAR, 2, 0, {COFF, CON}},
	{"Playlist pos", LNG_CFG_PLAYLISTPOS, DISF_MAIN, 0, CONF_TRKPLAYLISTPOS, 2, 0, {COFF, CON}},
	{"Track time", LNG_CFG_TRACKTIME, DISF_MAIN, 0, CONF_TRACKINFO, 2, 0, {COFF, CON}},
	{"Track Khz", LNG_CFG_TRACKKHZ, DISF_MAIN, 0, CONF_TRACKKHZ, 2, 0, {COFF, CON}},
	{"Track Bps", LNG_CFG_TRACKBPS, DISF_MAIN, 0, CONF_TRACKBPS , 2, 0, {COFF, CON}},
	{"VU peak indicators", LNG_CFG_VUPEAKINDICATORS, DISF_MAIN, 0, CONF_VUPEAK , 2, 0, {COFF, CON}},
	//{"Scrolling title", DISF_MAIN, 0, CONF_SCROLLTITLE , 2, 0, {COFF, CON}},
	
	{"Meta data", LNG_CFG_METADATA, DISF_META, 0, 0, 0, 0},
	{"Unicode filepaths", LNG_CFG_UNICODEFILEPATHS, DISF_META, 0, CONF_METAFILENAME, 2, 0, {CDISABLED, CENABLED}},

	{"Playlist", LNG_CFG_PLAYLIST, DISF_PLAYLIST, 0, 0, 0, 0},
	{"Track number", LNG_CFG_TRACKNUMBER, DISF_PLAYLIST, 0, CONF_TRACKNUMBER , 2, 0, {COFF, CON}},
	{"Scroll percent", LNG_CFG_SCROLLPERCENT, DISF_PLAYLIST, 0, CONF_SCROLLPERCENT , 15, 0, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"}},
	{"Font", LNG_CFG_FONT, DISF_PLAYLIST, 0, CONF_FONTSIZE, WVSFONTSSTRSTOTAL, 0, {WVSFONTSSTRS}},
	{"Media encoding", LNG_CFG_MEDIAENCODING, DISF_PLAYLIST, 0, CONF_ENCODING , WVSENCODINGLISTTOTAL, 0, {WVSENCODINGLIST}},

	{"Lyrics", LNG_CFG_LYRICS, DISF_LYRICS, 0, 0, 0, 0},
	{"Overlay in Main", LNG_CFG_OVERLAYINMAIN, DISF_LYRICS, 0, CONF_LYRICSSOVERLAY, 2, 0, {CDISABLED, CENABLED}},
	{"Lyrics font", LNG_CFG_LYRICSFONT, DISF_LYRICS, 0, CONF_LYRICSFONTSIZE, WVSFONTSSTRSTOTAL, 0, {WVSFONTSSTRS}},
	{"Global time offset", LNG_CFG_GLOBALTIMEOFFSET, DISF_LYRICS, 0, CONF_LYRICSSOFFSET, 15, 0, {"-3000", "-2500", "-2000", "-1500", "-1000", "-500", "-250", "0", "250", "500", "1000", "1500", "2000", "2500", "3000"}},
	{"Lyric encoding", LNG_CFG_LYRICENCODING, DISF_LYRICS, 0, CONF_LYRICSENCODING , WVSENCODINGLISTTOTAL, 0, {WVSENCODINGLIST}},

	{"aMSN", LNG_CFG_AMSN, DISF_MSN, 0, 0, 0, 0},
	{"Auto connect", LNG_CFG_AUTOCONNECT, DISF_MSN, 0, CONF_AMSNAUTOCONN, 2, 0, {CNO, CYES}},
	{"Mail notification", LNG_CFG_MAILNOTIFICATION, DISF_MSN, 0, CONF_AMSNMAILNOTIFY, 2, 0, {CDISABLED, CENABLED}},
	{"Mail notify timeout", LNG_CFG_MAILNOTIFYTIMEOUT, DISF_MSN, 0, CONF_AMSNMAILTIMEOUT, 14, 0, {"None", "2", "5", "7", "10", "15", "20", "30", "60", "90", "120", "300", "600"}},
	{"Overlay Icons", LNG_CFG_OVERLAYICONS, DISF_MSN, 0, CONF_AMSNOVRICONS, 2, 0, {CDISABLED, CENABLED}},
	{"Overlay page", LNG_CFG_OVERLAYPAGE, DISF_MSN, 0, CONF_AMSNOVRDEST, 1+WVSPAGELISTTOTAL, 0, {"All", WVSPAGELISTENG}},
	{"Icon overlay timeout", LNG_CFG_ICONOVERLAYTIMEOUT, DISF_MSN, 0, CONF_AMSNICONTIMEOUT, 14, 0, {"None", "2", "5", "7", "10", "15", "20", "30", "60", "90", "120", "300", "600"}},
	{"Open edit box (IME)", LNG_CFG_OPENEDITBOX, DISF_MSN, 0, CONF_AMSNIMEKEY, 5, 0, {"Ctrl+Enter", "Ctrl+Back", "RCtrl+RShift", "Ctrl+B4", "Ctrl+/"}},
	{"Emoticons", LNG_CFG_EMOTICONS, DISF_MSN, 0, CONF_AMSNSEMOTICONS, 2, 0, {CDISABLED, CENABLED}},
	{"Username format", LNG_CFG_USERNAMEFORMAT, DISF_MSN, 0, CONF_AMSNNAMEFORMAT, 7, 0, {"name: ", "name, ", "(name) ", "<name> ", "{name} ", "[name] ", "name - "}},
	{"Language", LNG_CFG_LANGUAGE, DISF_MSN, 0, CONF_AMSNENCODING, WVSENCODINGLISTTOTAL, 0, {WVSENCODINGLIST}},
	
	{"Bandwidth", LNG_CFG_BANDWIDTH, DISF_BANDWIDTH, 0, 0, 0, 0},
	{"Interface", LNG_CFG_INTERFACE, DISF_BANDWIDTH, 0, CONF_BWINTERFACE, 10, 0, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"}},
	{"Split screen", LNG_CFG_SPLITSCREEN, DISF_BANDWIDTH, 0, CONF_BWSPLITMODE, 2, 0, {CDISABLED, CENABLED}},
	{"Clock", LNG_CFG_CLOCK, DISF_BANDWIDTH, 0, CONF_BWCLOCK, 2, 0, {COFF, CON}},

	{"Netstat", LNG_CFG_NETSTAT, DISF_NETSTAT, 0, 0, 0, 0},
	{"Flags", LNG_CFG_FLAGS, DISF_NETSTAT, 0, CONF_NETSTATCFG, 1, 0, {NetStatFlags}},
	{"IP lookup", LNG_CFG_IPLOOKUP, DISF_NETSTAT, 0, CONF_NETSTATIPLOOKUP, 2, 0, {CDISABLED, CENABLED}},

	{"System info", LNG_CFG_SYSTEMINFO, DISF_SYSINFO, 0, 0, 0, 0},
	{"Free space unit", LNG_CFG_FREESPACEUNIT, DISF_SYSINFO, 0, CONF_SYSINFOUNIT, 2, 0, {"Megabyte", "Gigabyte"}},
	{"Display units", LNG_CFG_DISPLAYUNITS, DISF_SYSINFO, 0, CONF_SYSISHOWUNIT, 2, 0, {CNO, CYES}},
	{"List removable drives", LNG_CFG_LISTREMOVABLEDRIVES, DISF_SYSINFO, 0, CONF_SYSISHOWREMOVE, 2, 0, {CNO, CYES}},
	
	{"Ati Tray Tools", LNG_CFG_ATITRAYTOOLS, DISF_ATT, 0, 0, 0, 0},
	{"Temperature", LNG_CFG_TEMPERATURE, DISF_ATT, 0, CONF_ATTTEMP, 2, 0, {"Celsius", "Fahrenheit"}},

	{"Fraps overlay", LNG_CFG_FRAPSOVERLAY, DISF_FRAPS, 0, 0, 0, 0},
	{"Overlay", LNG_CFG_OVERLAY, DISF_FRAPS, 0, CONF_FRAPSOVERLAY, 2, 0, {CDISABLED, CENABLED}},
	{"Page", LNG_CFG_PAGE, DISF_FRAPS, 0, CONF_FRAPSDEST, 1+WVSPAGELISTTOTAL, 0, {"All", WVSPAGELISTENG}},
	{"Position", LNG_CFG_POSITION, DISF_FRAPS, 0, CONF_FRAPSPOS, 4, 0, {"Top Left", "Top Right", "Lower Right", "Lower Left"}},
	{"FPS Font", LNG_CFG_FPSFONT, DISF_FRAPS, 0, CONF_FRAPSFONT, 4, 0, {"5x7", "8x9", "14x16", "16x17"}},
	{"Print mode", LNG_CFG_PRINTMODE, DISF_FRAPS, 0, CONF_FRAPSCOPYMODE, 3, 0, {"Topmost", "Layered", "Inverted"}}, // Opaque, OR and XOR
	
	{"G15", LNG_CFG_G15, 0, 0, 0, 0, 0},
	{"Dancinglights", LNG_CFG_DANCINGLIGHTS, 0, 0, CONF_DANCINGLIGHTS, 4, 0, {"Off", "Keyboard+MKeys", "Keyboard", "MKeys"}},
	{"LCD beat", LNG_CFG_LCDBEAT, 0, 0, CONF_LCDBEAT, 2, 0, {CDISABLED, CENABLED}},
	{"Reverse volume control", LNG_CFG_REVERSEVOLUMECONTROL, 0, 0, CONF_VOLUMEINVERT, 2, 0, {CNO, CYES}}, 
	{"Handle media keys", LNG_CFG_HANDLEMEDIAKEYS, 0, 0, CONF_MEDIAKEYS, 2, 0, {CNO, CYES}},
	{"Lower BW limit", LNG_CFG_LOWERBWLIMIT, 0, 0, CONF_BWLOWER, 1, 0, {LightLowerBand}},
	{"Upper BW limit", LNG_CFG_UPPERBWLIMIT, 0, 0, CONF_BWUPPER, 1, 0, {LightUpperBand}},
	{"LED scale", LNG_CFG_LEDSCALE, 0, 0, CONF_LEDSCALE, 20, 0, {"10", "20", "30", "40", "50", "60", "70", "80", "90", "100", "110", "120", "130", "140", "150", "160", "170", "180", "190", "200"}},
	{"M1 LED level", LNG_CFG_M1LEDLEVEL, 0, 0, CONF_M1LEDL, 21, 0, {"10", "15", "20", "25", "30", "35", "40", "45", "50", "55", "60", "65", "70", "75", "80", "85", "90", "95", "100", "105", "110"}},
	{"M2 LED level", LNG_CFG_M2LEDLEVEL, 0, 0, CONF_M2LEDL , 21, 0, {"15", "20", "25", "30", "35", "40", "45", "50", "55", "60","65", "70", "75", "80", "85", "90", "95", "100", "105", "110", "115"}},
	{"M3 LED level", LNG_CFG_M3LEDLEVEL, 0, 0, CONF_M3LEDL, 21, 0, {"20", "25", "30", "35", "40", "45", "50", "55", "60","65", "70", "75", "80", "85", "90", "95", "100", "105", "110", "115", "120"}},
	{"MR LED level", LNG_CFG_MRLEDLEVEL, 0, 0, CONF_MRLEDL, 21, 0, {"25", "30", "35", "40", "45", "50", "55", "60","65", "70", "75", "80", "85", "90", "98", "100", "105", "110", "115", "120", "125"}},
	{"LCD beat level", LNG_CFG_LCDBEATLEVEL, 0, 0, CONF_LCDBEATLEVEL, 14, 0, {"70", "80", "90", "100", "110", "120", "130", "140", "150", "160", "170", "180", "190", "200"}},
	{"KB LED level 1", LNG_CFG_KBLEDLEVEL1, 0, 0, CONF_KBLEDL1, 21, 0, {"25", "30", "35", "40", "45", "50", "55", "60", "65", "70", "75", "80", "85", "90", "95", "100", "105", "110", "115", "120", "125"}},
	{"KB LED level 2", LNG_CFG_KBLEDLEVEL2, 0, 0, CONF_KBLEDL2, 21, 0, {"25", "30", "35", "40", "45", "50", "55", "60", "65", "70", "75", "80", "85", "90", "95", "100", "105", "110", "115", "120", "125"}},

	{"General", LNG_CFG_GENERAL, 0, 0, 0, 0, 0},
	{"Default page", LNG_CFG_DEFAULTPAGE, 0, 0, CONF_DEFAULTPAGE, WVSPAGELISTTOTAL, 0, {WVSPAGELISTENG}},
	{"Page swap effect", LNG_CFG_PAGESWAPEFFECT, 0, 0, CONF_SWAPEFFECT , 7, 0, {"Rotate 1", "Line fade", "Direct", "Rotate 2", "Slide left" ,"Slide up" ,"Drag"}},
	{"Auto swap when idle", LNG_CFG_AUTOSWAPWHENIDLE, 0, 0, CONF_AUTOPAGESWAP, 2, 0, {CNO, CYES}},
	{"Rotation degree", LNG_CFG_ROTATIONDEGREE, 0, 0, CONF_ROTATESTEP, 20, 0, {"0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0","1.1","1.2","1.3","1.4","1.5","1.6","1.7","1.8","1.9","2.0"}},
	{"Clock format", LNG_CFG_CLOCKFORMAT, 0, 0, CONF_CLOCKFORMAT, 2, 0, {"12hr","24hr"}},
	{"Invert display", LNG_CFG_INVERTDISPLAY, DISF_MAIN, 0, CONF_INVERT, 2, 0, {CDISABLED, CENABLED}},
	{"Wait for player", LNG_CFG_WAITFORPLAYER, 0, 0, CONF_WAITFORPLAYER, 2, 0, {CNO, CYES}},
	{"Exit with player",LNG_CFG_EXITWITHPLAYER, 0, 0, CONF_EXITWITHPLAYER, 2, 0, {CNO, CYES}},
	//{"IP Protocol", 0, 0, CONF_IPPROTOCOL, 2, 0, {"UDP", "TCP"}},
	{"Process priority", LNG_CFG_PROCESSPRIORITY, 0, 0, CONF_PROCESSPRIORITY, 5, 0, {"Idle", "Below N.", "Normal", "Above N.", "High"}},
	{"Profile", LNG_CFG_PROFILE, 0, 0, CONF_PROFILE, 3, 0, {"->", "Save now", "Reload"}},
	{"", 0, 0, 0, 0, -1, 0, {"", ""}}
};


ubyte metaTags[METATAGTOTAL][16]={
	"Title",
	"Artist",
	"Album",
	"Track",
	"Year",
	"Genre",
	"Comment",
	"Length",
	"Bitrate",
	"SRate",
	"Stereo",
	"VBR",
	"BPM",
	"Gain",
	"AlbumGain",
	"Encoder",
	"AlbumArtist",
	"OriginalArtist",
	"Disc",
	"Remixer",
	"Media",
	"Lyricist",
	"Subtitle",
	"Mood",
	"Key",
	"Composer",
	"Conductor",
	"Publisher",
	"Copyright",
	"URL",
};

int WVSFonts[9] = {
	LFTW_5x7,
	LFT_SMALLFONTS7X7,
	LFT_COMICSANSMS7X8,
	LFTW_SNAP,
	LFTW_HELVR12,
	LFT_ARIAL,
	LFTW_WENQUANYI9PT,
	LFTW_B16B,
	0
};

int TrackSkipTable[12] = {
	500,
	1000,
	1500,
	2000,
	3000,
	4000,
	5000,
	6000,
	7000,
	10000,
	20000,
	60000
};

int lyricOffsetTable[15] = {
	-3000,
	-2500,
	-2000,
	-1500,
	-1000,
	-500,
	-250,
	0,
	250,
	500,
	1000,
	1500,
	2000,
	2500,
	3000
};
	

int EncTable[WVSENCODINGLISTTOTAL] = {
	CMT_BIG5,
	CMT_GBK,
	CMT_HZ_GB2312,
	CMT_EUC_CN,
	CMT_EUC_JP,
	CMT_SHIFTJIS,
	CMT_EUC_KR,
	CMT_UTF8,
	CMT_CP1250,
	CMT_CP1251,
	CMT_TIS620,
	CMT_ISO8859_1,
	CMT_ISO8859_2,		// Latin alphabet no. 2 (Latin-2)
	CMT_ISO8859_3,		// South European (iso-ir-109, Latin-3)
	CMT_ISO8859_4,		// North European (iso-ir-110, Latin-3)
	CMT_ISO8859_5,		// Cyrillic languages (iso-ir-144)
	CMT_ISO8859_6,		// Arabic (RTL (bidi)text is unsupported within myLCD)
	CMT_ISO8859_7,		// Greek (iso-ir-126, ELOT 928)
	CMT_ISO8859_8,		// Hebrew (RTL (bidi) text unsupported within myLCD)
	CMT_ISO8859_9,		// Turkish (iso-ir-148, latin5)
	CMT_ISO8859_10,		// Nordic languages (iso-ir-157, latin6)
	CMT_ISO8859_11,		// Thai (tis620+nbsp)
	CMT_ISO8859_13,		// Baltic languages (Baltic rim, Latin-7)
	CMT_ISO8859_14,		// Celtic languages (iso-ir-199, Latin-8)
	CMT_ISO8859_15,		// (updated table similar to iso-8859-1, includes 'euro', Latin-9)
	CMT_ISO8859_16		// South-Eastern European, French, Italian and Irish Gaelic (iso-ir-226, Latin-10)
};

	
DWORD WVSProcessPriority[5] = {
	IDLE_PRIORITY_CLASS,
	BELOW_NORMAL_PRIORITY_CLASS,
	NORMAL_PRIORITY_CLASS,
	ABOVE_NORMAL_PRIORITY_CLASS,
	HIGH_PRIORITY_CLASS,
};

static void renderFrameBlock (TWINAMP *wa, int frameid);
static void renderFrame (TWINAMP *wa, int frameid);
static TFRAME *getFrame (TWINAMP *wa, int frameid);
static int isKeyPressed (int vk);

static int isYes (char *text);
static void setPageStatus (TWINAMP *wa, int frameid, int status);
static int getPageStatus (TWINAMP *wa, int frameid);
static int initGraphTimer (TWINAMP *wa, int rate);
static void killGraphTimer (TWINAMP *wa);
static void setFrapsFontType (TWINAMP *wa, int type);
static int getFrapsFontType (TWINAMP *wa);
static int registerOverlayFrame (TWINAMP *wa, TFRAMEOVER *ovr);
static void drawPlayerNotFound (TFRAME *frame, int font);
static int getRandomFont ();

static void closeMixers (TWINAMP *wvs);
static int initMixerDevice (TWINAMP *wvs, int mixer);

static int getPlaylistFilename (TWINAMP *wa, char *buffer, size_t buffersize, int index, int enc);
static int getPlaylistTrack (TWINAMP *wa, char *buffer, size_t buffersize, int start, int enc);


static void setUpdateSignal (TWINAMP *wa);
static int isFrameActive (TWINAMP *wa, int frameNumber);
static int track_Forward (TWINAMP *wa);
static int track_Rewind (TWINAMP *wa);
static void volumeSetActive (TWINAMP *wa, int ctrl);
static int hitlistTrackRemove (TWINAMP *wa, int num, ubyte *mp3path);
static int hitlistTrackAdd (TWINAMP *wa, int num, ubyte *mp3path);

static int updateTitleScroll (TWINAMP *wa, int forceUpdate);

static void advanceTitleScroll (TWINAMP *wa, int mode);
static void seekScroll (TWINAMP *wa, int pos);
static void seekEndTitleScroll (TWINAMP *wa);

static void toggle_overlayTrackNumber (TWINAMP *wa);

static void cacheTrackTitles (TWINAMP *wa, int start, int end);
static void cacheTrackTitlesNS (TWINAMP *wa, int start, int end);
static void cacheMetaTagNS (TWINAMP *wa, int metaIndex, int trackIndex);
static void cacheMetaTagsNS (TWINAMP *wa, int trackStart, int trackEnd);
static void cacheMetaTags (TWINAMP *wa, int trackStart, int trackEnd);
static void cacheMetaTag (TWINAMP *wa, int metaIndex, int trackIndex);
static void cacheTrackFilenames (TWINAMP *wa, int start, int end);
static int getMetaTag (TWINAMP *wa, char *buffer, size_t buffersize, int tagIndex, int trackIndex, int enc);
static int cache_processPackets (TWINAMP *wa);
static int getActivePage (TWINAMP *wa);
static void resetPlaylistScroller (TWINAMP *wa);
static int hitlistWrite (TWINAMP *wa, int num, ubyte *mp3path, int action);
static int hitlistOpen (TWINAMP *wa, int num);
static int hitlistClose (TWINAMP *wa);
static int hitlistEnqueueFile (TWINAMP *wa, int hitlistIndex, int hitlist, int play);
static int hitlistIndexToFile (TWINAMP *wa, int hitlistIndex, int hitlist, char *buffer, size_t buffersize);
static int signalPageEnter (TWINAMP *wa, int pageIndex);
static void signalPageExit (TWINAMP *wa, int pageIndex);
static int signalPageOpen (TWINAMP *wa, int pageIndex);
static int signalPageClose (TWINAMP *wa, int pageIndex);
int renderErrorPage (TFRAME *frame, int err, char *msg);

static void WVSHookOn (TWINAMP *wvs);
static void WVSHookOff (TWINAMP *wvs);

inline void REFRESH_LOCK ();
inline void REFRESH_UNLOCK ();

#define P_OPNFAILED		1
#define P_RNDFAILED		2
#define P_ENTFAILED		3
#define P_EXTFAILED		4
#define P_CLSFAILED		5
#define P_INPFAILED		6


