/***************************
* Written by: Hung Ki Chan
* Cold winterday in December 2005
* A simple calendar
***************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


static int return_time  (int choice);
static void updatetime (char *s, int len, int format);

static int monOffset = 0;
static int yearOffset = 0;
static int calMonthOffset = 0;


int displayInput_Calendar (TWINAMP *wa, int key, void *data);
int renderCalendar (TWINAMP *wa, TFRAME *frame, void *data);



int calPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderCalendar(wa, frame, userPtr);
}

int calPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Calendar(wa, key, userPtr);
}

int calPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("calPageEnter\n");
	return 1;
}

void calPageExit (void *userPtr)
{
	//dbprintf("calPageExit\n");
}

int calPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("calPageOpen()\n");
	return 1;
}

void calPageClose (void *userPtr)
{
	//dbprintf("calPageClose()\n");
}

static char *getMonth (TWINAMP *wa)
{
	return lng_getString(wa->lang, LNG_CAL_JANUARY+return_time(5)+monOffset);
}

static char *getDay (TWINAMP *wa, int d)
{
	return lng_getString(wa->lang, LNG_CAL_MO+d);
}

int renderCalendar (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (return_time(5) < 0 || return_time(5) > 11)
		return 0;


	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	TLPRINTR rect2 = {95,-2,frame->width-1,frame->height-1,95,0,0,0};
	
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

	yearOffset = calMonthOffset / 12;
	monOffset = calMonthOffset % 12;
	if (monOffset+return_time(5) > 11){
		monOffset = (monOffset+return_time(5)) - 12;
		monOffset = monOffset - return_time(5);
		yearOffset++;
	}
		
   	tm.tm_year = return_time(6)+yearOffset;
   	tm.tm_mon = return_time(5)+monOffset;
   	tm.tm_mday = 1;						/* start on the first */
    
   	lClearFrame(frame);
   	int y = 0;

   	do{
		time_t tnow = mktime(&tm);		/* get the time_t for now */
		tm = *(localtime(&tnow));
	
		if (tm.tm_wday == 0)
			rect.sx = 2+(6*14);
		else
			rect.sx = 2+((tm.tm_wday-1)*14);
		rect.sy = y;

		lPrintEx(frame, &rect, LFTW_5x7, 0, LPRT_CPY, "%i",tm.tm_mday);
		if (!calMonthOffset && tm.tm_mday == return_time(4))
			lInvertArea(frame, rect.sx-1, rect.sy-1, (rect.ex-1),(rect.ey-1));

   		if (tm.tm_wday == 0){
   			y += 7; 			/* done sat, do sun = next row*/
   			if (y >= (5*7))
   				y = 0;
   		}

		tnow += 60*60*24;			/* go to the next day */
		tm = *(localtime(&tnow));	/* refresh time */
	}while(tm.tm_mon == (return_time(5)+monOffset));	/* do so to next month */
	
	char buff[16];
	memset(buff, 0, sizeof(buff));
	updatetime(buff, 9, wa->overlay.clockFormat);
	rect.sx = 2;
	rect.sy = rect.ey = 5*7;
	
	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	int x;
	for (x = 0; x < 7; x++)
		lPrintf(frame, 2+(x*14), rect.ey+2, LFTW_5x7, LPRT_CPY, getDay(wa, x));

	lDrawLineDotted(frame, 2, rect.ey, (7*14)-4, rect.ey, LSP_SET);
	lPrintEx(frame, &rect2, LFTW_WENQUANYI9PT, PF_RESETXY|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, getMonth(wa));
	lPrintEx(frame, &rect2, LFT_DOTUMCHE24X24, PF_NEWLINE|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, "%d", return_time(6)+1900+yearOffset);
	lPrintEx(frame, &rect2, LFTW_WENQUANYI9PT, PF_NEWLINE|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, buff);

	return 1;
}


static int return_time (int choice)
{
    time_t t = time(0);
    struct tm *tdate = localtime(&t);
    switch(choice)
    { 
    case 1: return tdate->tm_hour;   //return current hour
            break;
    case 2: return tdate->tm_min;    //return current minute
            break;
    case 3: return tdate->tm_sec;    //return current second
            break;
    case 4: return tdate->tm_mday;   //return day of month 1--31
            break;
    case 5: return tdate->tm_mon;    //return month 0--11
            break;
    case 6: return tdate->tm_year;   //return Year (calendar year minus 1900)
            break;
    case 7: return tdate->tm_wday;   //return weekday 0--6 0=sunday
            break;
    }
	return 0;
}

static void updatetime (char *s, int len, int format)     //digital clock
{
	time_t t = time(0);
	struct tm *tdate = localtime(&t);
	if (format)
		strftime(s, len, "%H:%M:%S", tdate);
	else
		strftime(s, len, "%I:%M:%S", tdate);
}

int displayInput_Calendar (TWINAMP *wa, int key, void *data)
{

	if (isKeyPressed(VK_LSHIFT)){
		return -1;
		
	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		if (--calMonthOffset < -return_time(5))
			calMonthOffset = -return_time(5);
		renderFrame(wa, DISF_CALENDAR);

	}else if (key == G15_WHEEL_CLOCKWISE){
		(calMonthOffset)++;
		renderFrame(wa, DISF_CALENDAR);

	}else if (key == G15_SOFTKEY_4){
		calMonthOffset = 0;
		renderFrame(wa, DISF_CALENDAR);

	}else{
		return -1;
	}
	return 0;
}

