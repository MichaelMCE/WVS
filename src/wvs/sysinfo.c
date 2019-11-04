/************************************ 
* KneegoLCD with myLCD-library
* For KS0108 128x64
* Made by Hung Ki Chan
* One cloudy day in October, 2005
************************************/
/*----------------------------------
* Content:
* Availible space in Mb on C: drive
* Percentage use of total Memory
*----------------------------------*/
#include <winternl.h>
#include <wchar.h>
#include <locale.h>
#include <wctype.h>

#define SystemBasicInformation		0
#define	SystemPerformanceInformation	2
#define SystemTimeInformation		3

#define Li2Double(x)	((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
/*
typedef struct {
	DWORD	dwUnknown1;
	ULONG	uKeMaximumIncrement;
	ULONG	uPageSize;
	ULONG	uMmNumberOfPhysicalPages;
	ULONG	uMmLowestPhysicalPage;
	ULONG	UMmHighestPhysicalPage;
	ULONG	uAllocationGranularity;
	PVOID	pLowestUserAddress;
	PVOID	pMmHighestUserAddress;
	ULONG	uKeActiveProcessors;
	BYTE	bKeNumberProcessors;
	BYTE	bUnknown2;
	WORD	bUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct {
	LARGE_INTEGER	liIdleTime;
	DWORD		dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;
*/
typedef struct {
	LARGE_INTEGER	liKeBootTime;
	LARGE_INTEGER	liKeSystemTime;
	LARGE_INTEGER	liExpTimeZoneBias;
	ULONG			uCurrentTimeZoneID;
	DWORD			dwReserved;
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI) (UINT, PVOID, ULONG, PULONG);

typedef struct{
	unsigned int Weeks;
	unsigned int Days;
	unsigned int Hours;
	unsigned int Minutes;
	unsigned int Seconds;
}TUPT;


static PROCNTQSI _NtQuerySystemInformation = NULL;
static HANDLE hLibNTDLL = NULL;
static SYSTEM_BASIC_INFORMATION SysBaseInfo;
static SYSTEM_TIME_INFORMATION SysTimeInfo;
static SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
static LARGE_INTEGER liOldIdleTime = {{0,0}};
static LARGE_INTEGER liOldSystemTime = {{0,0}};
static MEMORYSTATUSEX stat;

static int GetDrives (TFRAME *frame, int x, int y, int unit, int showunit, int showremoveable);
float GetCPUUsage ();
static void GetUpTime (TUPT *upTime);
static int openQSI ();

int renderSysinfo (TWINAMP *wa, TFRAME *frame, void *data);
int sysinfoInit ();
void sysinfoCleanup ();

static const int textx = 46;
static const int pbarx = 68;
static const int pbarw = 42;
static const int pbarh = 8;
static const int pbardist = 9;


typedef struct {
	ubyte unit:1;
	ubyte showUnit:1;
	ubyte showRemovable:1;
	ubyte pad:5;
}TWVSSYSINFO;

static TWVSSYSINFO sysinfo;
static int sysInitOnce = 0;




int sysinfoPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	TWVSSYSINFO *si = (TWVSSYSINFO *)userPtr;
	si->unit = wa->config.sysInfoUnit;
	si->showUnit = wa->config.sysInfoShowUnit;
	si->showRemovable = wa->config.sysInfoShowRemovable;
	return renderSysinfo(wa, frame, si);
}

int sysinfoPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return -1;
}

int sysinfoPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("sysinfoPageEnter\n");
	return 1;
}

void sysinfoPageExit (void *userPtr)
{
	//dbprintf("sysinfoPageExit\n");
}

int sysinfoPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("sysinfoPageOpen()\n");
	return sysinfoInit();
}

void sysinfoPageClose (void *userPtr)
{
	//dbprintf("sysinfoPageClose()\n");
	sysinfoCleanup();
}

int renderSysinfo (TWINAMP *wa, TFRAME *frame, void *data)
{
	static TUPT ut;
	const int font = LFTW_5x7;
	int pbary = 0;
	TWVSSYSINFO *sysinfo = (TWVSSYSINFO*)data;

	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	lClearFrame(frame);
	float CPUuse = GetCPUUsage();
		
	GetDrives(frame, 0, 0, sysinfo->unit, sysinfo->showUnit, sysinfo->showRemovable);
	//lDrawPBar(frame, pbarx, pbary, pbarw, pbarh,(float)CPUuse,PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_SET);
	lPrintf(frame, textx, pbary+1, font, LPRT_XOR, "%s:    %.0f%%",\
	  lng_getString(wa->lang, LNG_SI_CPU), CPUuse);

	pbary += pbardist;
	GlobalMemoryStatusEx(&stat);
	//lDrawPBar(frame, pbarx, pbary, pbarw, pbarh,(float)stat.dwMemoryLoad,PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_SET);
	lPrintf(frame, textx , pbary+1, font, LPRT_XOR,"%s:    %.0f%%",\
	  lng_getString(wa->lang, LNG_MEM), (float)stat.dwMemoryLoad);

	pbary += pbardist;
	float totalv = 100.0-(float)((100.0/stat.ullTotalPageFile)*stat.ullAvailPageFile);
	//lDrawPBar(frame, pbarx, pbary, pbarw, pbarh,totalv ,PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_SET);
	lPrintf(frame, textx , pbary+1, font, LPRT_XOR, "%s:   %.0f%%",\
	  lng_getString(wa->lang, LNG_SI_PAGE), totalv);

	pbary += pbardist;	
	totalv = 100.0-(float)((100.0/stat.ullTotalVirtual)*stat.ullAvailVirtual);
	//lDrawPBar(frame, pbarx, pbary, pbarw, pbarh,totalv ,PB_BORDER_HBOX|PB_MARKER_HFILL, LSP_SET);
	lPrintf(frame, textx , pbary+1, font, LPRT_XOR,"%s:    %.0f%%",\
	  lng_getString(wa->lang, LNG_SI_VIRT), totalv);

	pbary += pbardist;
	GetUpTime(&ut);
	lPrintf(frame, textx , pbary, font, LPRT_OR, "%s:%d%s %d%s %d%s %d%s %d",\
	  lng_getString(wa->lang, LNG_SI_UPTIME),\
	  ut.Weeks, lng_getString(wa->lang, LNG_SI_W),\
	  ut.Days, lng_getString(wa->lang, LNG_SI_D),\
	  ut.Hours, lng_getString(wa->lang, LNG_SI_H),\
	  ut.Minutes,lng_getString(wa->lang, LNG_SI_M),\
	  ut.Seconds);
	return 1;
}

static void GetUpTime (TUPT *ut)
{
	unsigned long lintTicks = GetTickCount();
	ut->Seconds = (lintTicks / 1000) % 60;
	ut->Minutes = ((lintTicks / 1000) / 60) % 60;
	ut->Hours = (((lintTicks / 1000) / 60) / 60) % 24;
	ut->Days = ((((lintTicks / 1000) / 60) / 60) / 24) % 7;
    ut->Weeks = (((((lintTicks / 1000) / 60) / 60) / 24) / 7) % 52;
}   

float GetCPUUsage ()
{
	float dbIdleTime = 0.0f;

	if (_NtQuerySystemInformation){
		LONG status = _NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
		if (status == NO_ERROR){ // should be STATUS_SUCCESS?
			status = _NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
			if (status == NO_ERROR){
				status = _NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
				if (status == NO_ERROR){
					status = _NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
					if (status == NO_ERROR){
						float dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
						if (dbSystemTime > 0.0)
							dbIdleTime = (Li2Double(SysPerfInfo.IdleTime) - Li2Double(liOldIdleTime)) / dbSystemTime;
						dbIdleTime = 100.0 - dbIdleTime * 100.0 / (float)SysBaseInfo.NumberOfProcessors;
						_NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
						_NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
						liOldIdleTime = SysPerfInfo.IdleTime;
						liOldSystemTime = SysTimeInfo.liKeSystemTime;
					}
				}
			}
		}
	}
	if (dbIdleTime < 0.0){
		return 0.0;
	}else{
		if (dbIdleTime > 100.0)
			return 100.0;
		else
			return dbIdleTime;
	}
}

static u64 GetFreeDriveSpace (ubyte *DirectoryName)
{
	u64 lpFreeBytesAvailable;
	u64 lpTotalNumberOfBytes;
	u64 lpTotalNumberOfFreeByte;
	GetDiskFreeSpaceEx((char*)DirectoryName,(PULARGE_INTEGER)&lpFreeBytesAvailable,(PULARGE_INTEGER)&lpTotalNumberOfBytes,(PULARGE_INTEGER)&lpTotalNumberOfFreeByte);
	return lpTotalNumberOfFreeByte;
}

static int GetDrives (TFRAME *frame, int x, int y, int unit, int showunit, int showRemovable)
{
	int i,ct=0;
	int dtype;
	int row = y;
	ubyte buffer[] = {0,':',0};
	int len = 4096;
	char drives[len];
	memset(drives, 0, len);
	int printthis = 0;
	int total = GetLogicalDriveStringsA(len, drives);
	
	for (i=0;i<total;i++){
		if (*(drives+i) == 0){
			dtype = GetDriveType(drives+i-3);
			if (dtype != DRIVE_CDROM){
				printthis = 1;
				if (!showRemovable && dtype == DRIVE_REMOVABLE)
				 	printthis = 0;
			}
			if (printthis){
				printthis = 0;
				buffer[0] = *(drives+i-3);

				if (!unit){
					if (showunit)
						lPrintf(frame, x, row, LFT_SMALLFONTS7X7, LPRT_CPY,"%s %iMb", buffer, (int)(GetFreeDriveSpace(buffer)/1024/1024));
					else
						lPrintf(frame, x, row, LFT_SMALLFONTS7X7, LPRT_CPY,"%s %i", buffer, (int)(GetFreeDriveSpace(buffer)/1024/1024));
				}else{
					if (showunit)
						lPrintf(frame, x, row, LFT_SMALLFONTS7X7, LPRT_CPY,"%s %.1fGb", buffer, (float)((float)GetFreeDriveSpace(buffer)/1024.0/1024.0/1024.0));
					else
						lPrintf(frame, x, row, LFT_SMALLFONTS7X7, LPRT_CPY,"%s %.1f", buffer, (float)((float)GetFreeDriveSpace(buffer)/1024.0/1024.0/1024.0));
				}
				ct++;
				row += 7;
				if (row >= frame->height-1){
					row = y;
					x = 112;
				}
			}
			*(drives+i) = 32;
		}
	}
	return ct;
}

int sysinfoInit ()
{
	if (openQSI()){
		_NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
		_NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
		liOldIdleTime = SysPerfInfo.IdleTime;
		liOldSystemTime = SysTimeInfo.liKeSystemTime;
		sysInitOnce = 1;
		return 1;
	}else{
		sysInitOnce = 0;
		return 0;
	}
}

static int openQSI ()
{
	hLibNTDLL = GetModuleHandle("ntdll");
	if (hLibNTDLL){
		stat.dwLength = sizeof(MEMORYSTATUSEX);
		_NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(hLibNTDLL, "NtQuerySystemInformation");
		return 1;
	}
	return 0;
}

void sysinfoCleanup ()
{
	if (hLibNTDLL){
		//CloseHandle(hLibNTDLL);
		hLibNTDLL = NULL;
	}
	sysInitOnce = 0;
}

              

