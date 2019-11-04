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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.


#include <winternl.h>
#include <wchar.h>
#include <locale.h>
#include <wctype.h>

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define NT_HANDLE_LIST		16
#define MAX_HANDLE_LIST_BUF	0x200000
#define Protocol			0x01
#define LocalIP				0x02
#define LocalPort			0x04
#define RemoteIP			0x08
#define RemotePort			0x10
#define PID					0x20
#define FilePath			0x40
#define State				0x80

#define TOTALPORTS	512
#define TOTALPIDS	96


static int highlightedItem = 0;
static unsigned int displayMode = 4;		// 2 = view all, 1 = cfg, 3 = view by connection, 4 = view by process
static int startIndex = 0;
static int addressOnce = 0;
static int nsInitOnce = 0;



#ifndef PUNICODE_STRING
#define PUNICODE_STRING void*
#endif

#ifndef __OBJECT_ATTRIBUTES_DEFINED
#define __OBJECT_ATTRIBUTES_DEFINED
typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;
#endif





typedef struct{
	int port;
	char ip[128];
}TCONNINFO;

typedef struct{
	int id;
	char module[512];
}TPROCESSINFO;

typedef struct{
	int proto;		// 1=tcp, 2=udp
	int state;		// tcp conn state
	TCONNINFO local;
	TCONNINFO remote;
	TPROCESSINFO process;
}TNETSTAT;

unsigned int itemlookup[8] = {Protocol, LocalIP, LocalPort, RemoteIP, RemotePort, PID, FilePath, State};
ubyte protocollookup[3][4] = {" ", "TCP", "UDP"};


typedef struct{
	int pid;
	int port;
	int	proto;	// 1 = tcp, 2 = udp
}TPROCESSPORTS;


typedef struct {
	unsigned int itemFlags;
	TPROCESSPORTS pids[TOTALPIDS+1];
	TNETSTAT netstat[TOTALPORTS+1];
	int total;
	int	flags;
	int ipLookup;
	char ipAddress[128];
}TNSOPTION;
static TNSOPTION nsOption;


typedef struct _HandleInfo{
	USHORT dwPid;
	USHORT CreatorBackTraceIndex;
	BYTE ObjType;
	BYTE HandleAttributes;
	USHORT HndlOffset;
	DWORD dwKeObject;
	ULONG GrantedAccess;
}HANDLEINFO, *PHANDLEINFO;

typedef DWORD (CALLBACK* NTQUERYSYSTEMINFORMATION)( DWORD, PDWORD, DWORD, PVOID);
NTQUERYSYSTEMINFORMATION Nt_QuerySystemInformation;


typedef void (CALLBACK* RTLINITUNICODESTRING)(PUNICODE_STRING, PCWSTR);
RTLINITUNICODESTRING Rtl_InitUnicodeString;

typedef DWORD (CALLBACK* ZWOPENSECTION)(PVOID, DWORD,POBJECT_ATTRIBUTES);
ZWOPENSECTION Zw_OpenSection;

//typedef VOID(CALLBACK* INITIALIZEOBJECTATTRIBUTES)(POBJECT_ATTRIBUTES,PUNICODE_STRING,ULONG,HANDLE,PSECURITY_DESCRIPTOR);
//INITIALIZEOBJECTATTRIBUTES InitializeObjectAttributes;

typedef DWORD (CALLBACK* ZWOPENFILE)(PHANDLE,DWORD,POBJECT_ATTRIBUTES,PIO_STATUS_BLOCK,ULONG,ULONG);
ZWOPENFILE Zw_OpenFile;

static HMODULE hNtdll = NULL;
	
/*
char states[13][24]={
	"unknown",
	"Closed",
	"Listening",
	"Syn Sent",
	"Syn Received",
	"Established",
	"Fin Wait-1",
	"Fin Wait-2",
	"Close Wait",
	"Closing",
	"Last Acknowledge",
	"Time Wait",
	"Delete TCB",
};
*/


int displayInput_Netstat (TWINAMP *wa, int key, void *data);
int renderNetstat (TWINAMP *wa, TFRAME *frame, void *data);
void closeNetstat ();
int initNetstat (TWINAMP *wa, TFRAME *frame);


int nsPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderNetstat(wa, frame, userPtr);
}

int nsPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Netstat(wa, key, userPtr);
}

int nsPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("nsPageEnter\n");
	return 1;
}

void nsPageExit (void *userPtr)
{
	//dbprintf("nsPageExit\n");
}

int nsPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("nsPageOpen()\n");
	return 1;
}

void nsPageClose (void *userPtr)
{
	//dbprintf("nsPageClose()\n");
	closeNetstat();
}

DWORD getmap (PHANDLEINFO get1, LPVOID addr, HANDLE pm, char *buf)
{

	if (addr == NULL)
		return 0;
	
	DWORD readset;
	LPVOID pmaddr1;
	int i;
	
	readset = (get1->dwKeObject>>0x16);
	readset = *((LPDWORD)((DWORD)addr + 4*readset));
	if ((readset&0x000000ff) < 1)
		return 0;

	if ((readset&0x000000ff)<0x80){
		pmaddr1 = MapViewOfFile(pm,4,0,readset&0xfffff000,0x1000);
		if (!pmaddr1)
			return 0;

		readset = (get1->dwKeObject>>0x0c) & 0x3ff;
		readset = *((LPDWORD)((DWORD)pmaddr1 + 4*readset));
		UnmapViewOfFile(pmaddr1);
		readset = readset & 0x0FFFFF000;
	}else{
		readset = (readset&0xfffff000)+(get1->dwKeObject&0x003ff000);
	}
	pmaddr1 = MapViewOfFile(pm,4,0,readset,0x1000);
	if (pmaddr1 != NULL){
		readset = get1->dwKeObject&0x00000fff;
		readset = (DWORD)pmaddr1+readset;

		for(i=0;i<0x70;i++)
			buf[i] = *((char *)(readset + i));
		UnmapViewOfFile(pmaddr1);
		return readset;
	}else{
		return 0;
	}

}

int enumPids (TPROCESSPORTS *pids)
{

	int totalPids = 0;
	DWORD readset1;
	DWORD readset2;
	DWORD readset3;
	OVERLAPPED la;
	la.hEvent = NULL;
	DWORD dwNumEntries = 0;
	PHANDLEINFO pHandleInfo = NULL;
	HANDLE htcp = NULL;
	HANDLE pmy = NULL;
	HANDLE hudp = NULL;
	HANDLE myhand = NULL;
	HANDLE h1 = NULL;
	DWORD status;
	LPVOID pmaddr = NULL;
	TOKEN_PRIVILEGES NewState;
	DWORD dwNumBytes = MAX_HANDLE_LIST_BUF;
	PDWORD pdwHandleList = NULL;
	DWORD dwNumBytesRet;
	HANDLE hToken = NULL;
	BOOL isok;
	UNICODE_STRING dn;
	IO_STATUS_BLOCK ch3;
	int port1;
	int port2;
	HANDLE hProc = NULL;
	wchar_t * ch1 = L"\\Device\\Tcp";
	wchar_t * ch2 = L"\\Device\\Udp";
	OBJECT_ATTRIBUTES ofs;
	DWORD i;
	DWORD p=0;
	char buf1[0x70];
	char buf2[0x70];
	char buf3[0x70];
	char in[0x18];
	char in1[0x18];
	char out[0x38];
	char out1[0x30];
	PHANDLEINFO tcpdnum = NULL;
	PHANDLEINFO udpdnum = NULL;
	

	Rtl_InitUnicodeString(&dn, L"\\Device\\PhysicalMemory");
	
	OBJECT_ATTRIBUTES udm = {
		sizeof(OBJECT_ATTRIBUTES),// Length
		NULL, // RootDirectory
		&dn, // ObjectName
		0, // Attributes
		NULL, // SecurityDescriptor
		NULL, // SecurityQualityOfService
	};

	//InitializeObjectAttributes(&udm, &dn, OBJ_CASE_INSENSITIVE|OBJ_INHERIT|OBJ_OPENIF, NULL, NULL);

	status = Zw_OpenSection(&h1, SECTION_MAP_READ, &udm);
	if (status == 0){
		pmaddr = MapViewOfFile(h1, FILE_MAP_READ, 0, 0x30000, 0x1000);
		
		pmy = GetCurrentProcess();
		NewState.PrivilegeCount=1;
		NewState.Privileges[0].Attributes=2;
		NewState.Privileges[0].Luid.HighPart=0;
		NewState.Privileges[0].Luid.LowPart=0;
		isok = LookupPrivilegeValue(0,SE_DEBUG_NAME,&NewState.Privileges[0].Luid);
		isok = OpenProcessToken(pmy, 0x20, &hToken);
		isok = AdjustTokenPrivileges(hToken,0,&NewState,0x10,0,0);
		CloseHandle(hToken);
	
		Rtl_InitUnicodeString(&dn,ch1);
		ofs.SecurityDescriptor = 0;
		ofs.ObjectName = &dn;
		ofs.Length =0x18;
		ofs.RootDirectory = 0;
		ofs.Attributes =0x40;
		ofs.SecurityQualityOfService =0;
		
		status = Zw_OpenFile(&htcp, 0x100000, &ofs, &ch3, 3, 0);
		Rtl_InitUnicodeString(&dn,ch2);
		ofs.ObjectName = &dn;
		status = Zw_OpenFile(&hudp, 0x100000, &ofs, &ch3, 3, 0);

		pdwHandleList = (PDWORD)malloc(dwNumBytes);
		if (pdwHandleList == NULL)
			return 0;		
		dwNumBytesRet = 0x10;
	
		isok = Nt_QuerySystemInformation(0x10, pdwHandleList, dwNumBytes, &dwNumBytesRet);
		//dbprintf("isok %x %i total handles:%i mem:%i\n",(int)isok, (int)GetLastError(), (int)pdwHandleList[0], (int)dwNumBytesRet);
		if (!isok){
			dwNumEntries = pdwHandleList[0];
			pHandleInfo = (PHANDLEINFO)(pdwHandleList+1);

			for (i = 0; i < dwNumEntries; i++, pHandleInfo++){
				if (pHandleInfo->dwPid == GetCurrentProcessId() && pHandleInfo->HndlOffset == (int)htcp){
					tcpdnum = pHandleInfo;
					break;
				}
			}

			pHandleInfo = (PHANDLEINFO)( pdwHandleList + 1 );
			for (i = 0; i < dwNumEntries; i++, pHandleInfo++){
				if (pHandleInfo->dwPid == GetCurrentProcessId() && pHandleInfo->HndlOffset == (int)hudp){
					udpdnum = pHandleInfo;
					break;
				}
			}

			ZeroMemory(buf1,0x70);
			ZeroMemory(buf2,0x70);
			readset1 = getmap(tcpdnum, pmaddr, h1, buf1);
			if (!readset1){
				//dbprintf("map tcp fail\n");
				return 0;
			}
		
			readset2 = getmap(udpdnum, pmaddr, h1, buf2);
			if (!readset2){
				//dbprintf("map udp fail\n");
				return 0;
			}
			
			la.hEvent = CreateEvent(0,1,0,0);
			la.Internal = 0;
			la.InternalHigh=0;
			la.Offset = 0;
			la.OffsetHigh = 0;
			pHandleInfo = (PHANDLEINFO)(pdwHandleList + 1);
	
			for (i = 0; i < dwNumEntries; i++, pHandleInfo++){
				ZeroMemory(buf3, 0x70);
				if (pHandleInfo->ObjType == tcpdnum->ObjType){
					readset3 = getmap(pHandleInfo,pmaddr,h1,buf3);
					if (readset3 == 0){
						continue;
					}
					if (buf3[4]==buf1[4] && buf3[5]==buf1[5]&& buf3[6]==buf1[6]&& buf3[7]==buf1[7]){
						if ((buf3[16]==1 || buf3[16]==2) && buf3[17]==0 && buf3[18]==0 && buf3[19]==0){
							if (hProc != NULL){
								CloseHandle(hProc);
								hProc = NULL;
							}
							
							hProc = OpenProcess(0x40, 0, pHandleInfo->dwPid);
							if (hProc == NULL){
								continue;
							}
							if (myhand != NULL){
								CloseHandle(myhand);
								myhand = NULL;
							}
								
							if (DuplicateHandle(hProc, (HANDLE)(int)(pHandleInfo->HndlOffset), pmy, &myhand, 0, 0, 2)){
								if (myhand == NULL){
									CloseHandle(hProc);
									hProc = NULL;
									continue;
								}
								ZeroMemory(out1, 0x30);
								ZeroMemory(out, 0x38);
								ZeroMemory(in, 0x18);
								ZeroMemory(in1, 0x18);
								in[0x10] = 4;
								in1[0x10] = 3;
							
								if (buf3[16] == 2){
									p = 0;
									isok = DeviceIoControl(myhand, 0x210012, &in, 0x18, &out, 0x38, &p, &la);
									if (isok == FALSE){
										CloseHandle(hProc);
										hProc = NULL;
										CloseHandle(myhand);
										myhand = NULL;
										continue;
									}
								}

								isok = DeviceIoControl(myhand, 0x210012, &in1, 0x18, &out1, 0x30, &p, &la);
								if (isok){
									port1 = out1[12];
									if (port1 < 0)
										port1 = 256 + port1;
									port1 = port1*256;
									port2 = out1[13];
									
									if (port2 < 0)
										port1 = port1 + 256 + port2;
									else
									port1 = port1 + port2;
								//	dbprintf("TCP PID:%d; PORT:%d\n",pHandleInfo->dwPid, port1);
									if (pids != NULL){
										pids->pid = pHandleInfo->dwPid;
										pids->port = port1;
										pids->proto = 1;
										pids++;
									}
									totalPids++;
									if (totalPids > TOTALPIDS)
										break;
								}
								CloseHandle(hProc);
								hProc = NULL;
								CloseHandle(myhand);
								myhand = NULL;
							}
						}
					}
				}
			}
			pHandleInfo = (PHANDLEINFO)(pdwHandleList + 1);

			for (i = 0; i < dwNumEntries; i++, pHandleInfo++){
				ZeroMemory(buf3,0x70);
				if (pHandleInfo->ObjType == udpdnum->ObjType){
					readset3 = getmap(pHandleInfo,pmaddr,h1,buf3);
					if (readset3==0){
						continue;
					}
					if (buf3[4]==buf2[4] && buf3[5]==buf2[5]&& buf3[6]==buf2[6]&& buf3[7] == buf2[7]){
						if ((buf3[16]==1 || buf3[16]==2) && buf3[17]==0 && buf3[18]==0 && buf3[19] == 0){
							if (hProc != NULL){
								CloseHandle(hProc);
								hProc = NULL;
							}
							
							hProc = OpenProcess(0x40, 0, pHandleInfo->dwPid);
							if (hProc == NULL){
								continue;
							}
							if (myhand != NULL){
								CloseHandle(myhand);
								myhand = NULL;
							}
								
							if (DuplicateHandle(hProc, (HANDLE)(int)(pHandleInfo->HndlOffset), pmy, &myhand, 0, 0, 2)){
								if (myhand == NULL){
									CloseHandle(hProc);
									hProc = NULL;
									continue;
								}
								ZeroMemory(out1,0x30);
								ZeroMemory(out,0x38);
								ZeroMemory(in,0x18);
								ZeroMemory(in1,0x18);
								in[0x10] = 4;
								in1[0x10] = 3;
							
								if (buf3[16] == 2){
									p = 0;
									isok = DeviceIoControl(myhand,0x210012,&in,0x18,&out,0x38,&p,&la);
									if (isok == FALSE){
										CloseHandle(hProc);
										hProc = NULL;
										CloseHandle(myhand);
										myhand = NULL;
										continue;
									}
								}
								isok = DeviceIoControl(myhand,0x210012,&in1,0x18,&out1,0x30,&p,&la);
								if (isok){
									port1 = out1[12];
									if (port1 < 0)
										port1 = 256 + port1;
									port1 = port1*256;
									port2 = out1[13];
									if (port2<0)
										port1 = port1 + 256 + port2;
									else
										port1 = port1 + port2;
										
								//	dbprintf("UDP PID:%d; PORT:%d\n",pHandleInfo->dwPid,(unsigned short)port1);
									if (pids != NULL){
										pids->pid = pHandleInfo->dwPid;
										pids->port = port1;
										pids->proto = 2;
										pids++;
									}
									totalPids++;
									if (totalPids > TOTALPIDS)
										break;
								}
								CloseHandle(hProc);
								hProc = NULL;
								CloseHandle(myhand);
								myhand = NULL;
							}
						}
					}
				}
			}
		}else{
			dbprintf("netstat: error opening handlelist\n");
		}
	}else{
		dbprintf("netstat: error accessing memory\n");
	}

	if (la.hEvent)
		CloseHandle(la.hEvent);
	if (pdwHandleList)
		free(pdwHandleList);
	if (h1)
		CloseHandle(h1);
	if (pmy)
		CloseHandle(pmy);
	if (htcp)
		CloseHandle(htcp);
	if (hudp)
		CloseHandle(hudp);
	if (pmaddr)
		UnmapViewOfFile(pmaddr);
	//if (hNtdll)
		//FreeLibrary(hNtdll);
	return totalPids;
}

int pidToModule (TWINAMP *wa, int pid, char *path)
{
	if (pid){
		MODULEENTRY32 me32;
		me32.dwSize = sizeof(MODULEENTRY32);

		HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if (hModuleSnap != INVALID_HANDLE_VALUE){
			if (Module32First(hModuleSnap, &me32)){
				strncpy(path, me32.szExePath, strlen(me32.szExePath)+1);
				//dbprintf("\t%i %s\n", pid, path);
				CloseHandle(hModuleSnap);
				return 1;
			}
			CloseHandle(hModuleSnap);
			//strncpy(path, "unknown", 8);
			strncpy(path, lng_getString(wa->lang, LNG_NS_UNKNOWN), 512);
			return 0;
		}else{
			if (pid == 8){
				strncpy(path, lng_getString(wa->lang, LNG_NS_SYSTEM), 512);
				return 0;
			}
		}
	}
	strncpy(path, lng_getString(wa->lang, LNG_NS_UNKNOWN), 512);
	return 0;

}

int portToPid (TPROCESSPORTS *pids, int total, int port, int proto)
{
	int i;
	for (i = 0; i < total; i++){
		//dbprintf("%i %i: %i %i\n", i, total, pids->port, pids->proto);
		if (port == pids->port && proto == pids->proto){
			if (pids->pid)
				return pids->pid;
		}
		pids++;
	}
	return 0;
}

char *tcpStateToText (TWINAMP *wa, unsigned int state)
{
	if (state < 13)
		return lng_getString(wa->lang, LNG_NS_UNKNOWN+state);  //states[state];
	else
		return " ";
}

int gettcptable (TWINAMP *wa, TFRAME *frame, TPROCESSPORTS *pids, int total, int portTotal, TNETSTAT *netstat)
{
	int sort = 0;

    // Declare and initialize variables
    PMIB_TCPTABLE pTcpTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    char name[_MAX_FNAME];
	char ext[_MAX_EXT];
    struct in_addr IpAddr;
    int i = 0;
    int tentry = 0;

    pTcpTable = (MIB_TCPTABLE *) MALLOC(sizeof (MIB_TCPTABLE));
    if (pTcpTable == NULL) {
        dbprintf("netstat: Error allocating memory\n");
        return 0;
    }

    dwSize = 0;//sizeof (MIB_TCPTABLE);
// Make an initial call to GetTcpTable to
// get the necessary size into the dwSize variable
    if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, sort)) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pTcpTable);
        pTcpTable = (MIB_TCPTABLE *) MALLOC(dwSize);
        if (pTcpTable == NULL) {
            dbprintf("netstat: Error allocating memory\n");
            return 0;
        }
    }
// Make a second call to GetTcpTable to get
// the actual data we require
    if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, sort)) == NO_ERROR) {
        //dbprintf("\tNumber of entries: %d\n", (int) pTcpTable->dwNumEntries);
        tentry = (int)pTcpTable->dwNumEntries;
        
        for (i = 0; i < tentry && i+portTotal < TOTALPORTS; i++, netstat++) {
            IpAddr.S_un.S_addr = (u_long) pTcpTable->table[i].dwLocalAddr;
			strncpy(netstat->local.ip, inet_ntoa(IpAddr), 128);
			netstat->local.port = ntohs(pTcpTable->table[i].dwLocalPort);

			if (pTcpTable->table[i].dwState == MIB_TCP_STATE_LISTEN){
				strncpy(netstat->remote.ip, "0.0.0.0", 8);
				netstat->remote.port = 0;
			}else{
            	IpAddr.S_un.S_addr = (u_long) pTcpTable->table[i].dwRemoteAddr;
            	strncpy(netstat->remote.ip, inet_ntoa(IpAddr), 128);
            	netstat->remote.port = ntohs(pTcpTable->table[i].dwRemotePort);
            }

      		netstat->proto = 1; // 1 = tcp
      		netstat->state = pTcpTable->table[i].dwState;
			netstat->process.id = portToPid(pids, total, netstat->local.port, 1);

			if (pidToModule(wa, netstat->process.id, netstat->process.module)){
				_splitpath(netstat->process.module, NULL, NULL, name, ext);
				sprintf(netstat->process.module, "%s%s", name, ext);
			}
        }
    }else{
        dbprintf("\tGetTcpTable failed with %i\n", (int)dwRetVal);
     	return 0;
    }

	if (pTcpTable != NULL)
		FREE(pTcpTable);
    return i;
}

int getudptable (TWINAMP *wa, TFRAME *frame, TPROCESSPORTS *pids, int total, int portTotal, TNETSTAT *netstat)
{

	int sort = 0;

    // Declare and initialize variables
    PMIB_UDPTABLE pUdpTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];
    struct in_addr IpAddr;
	int tentry = 0;
    int i = 0;

/*
    pUdpTable = (MIB_UDPTABLE *) MALLOC(sizeof (MIB_UDPTABLE));
    if (pUdpTable == NULL) {
        dbprintf("Error allocating memory\n");
        return 0;
    }
*/

    dwSize = 0;//sizeof (MIB_TCPTABLE);
// Make an initial call to GetTcpTable to
// get the necessary size into the dwSize variable
    if ((dwRetVal = GetUdpTable(NULL, &dwSize, sort)) == ERROR_INSUFFICIENT_BUFFER) {
  //      FREE(pUdpTable);
        pUdpTable = (MIB_UDPTABLE *)MALLOC(dwSize);
        if (pUdpTable == NULL) {
            dbprintf("netstat: Error allocating memory\n");
            return 0;
        }
    }
// Make a second call to GetTcpTable to get
// the actual data we require
	dwRetVal = GetUdpTable(pUdpTable, &dwSize, sort);
    if (dwRetVal == NO_ERROR){
        //dbprintf("\tNumber of entries: %d\n", (int) pTcpTable->dwNumEntries);
        tentry = pUdpTable->dwNumEntries;
        
        for (i = 0; i < tentry && i+portTotal < TOTALPORTS; i++, netstat++) {
      		netstat->proto = 2; // 2 = udp
      		netstat->state = 0; // udp is stateless
            IpAddr.S_un.S_addr = (u_long)pUdpTable->table[i].dwLocalAddr;
			strncpy(netstat->local.ip, inet_ntoa(IpAddr), 128);
			
      		netstat->local.port = ntohs(pUdpTable->table[i].dwLocalPort);
			netstat->process.id = portToPid(pids, total, netstat->local.port, 2);
			
			if (pidToModule(wa, netstat->process.id, netstat->process.module)){
				_splitpath(netstat->process.module, NULL, NULL, name, ext);
				sprintf(netstat->process.module, "%s%s", name, ext);
			}
		}
	}
	
	if (pUdpTable != NULL)
		FREE(pUdpTable);
	return i;
}          

static void _swapInt (int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

static void InvertRegion (TFRAME *frame, int x1, int y1, int x2, int y2)
{
	x1--; x2--;
	y1--; y2--;
	if (x2 < x1) _swapInt(&x2,&x1);
	if (y2 < y1) _swapInt(&y2,&y1);
	y2++;
	int y;
	for (y = y1; y<y2; y++)
		lDrawLine(frame, x1, y, x2, y, LSP_XOR);
}

static void drawBox (TFRAME *frame, int x1, int y1, int x2, int y2)
{
	lDrawRectangle(frame, x1, y1, x2, y2, LSP_SET);
}

static void drawCFGText (TFRAME *frame, TLPRINTR *rect, int invert, int selected, char *text, char *help)
{
	lPrintEx(frame, rect, LFTW_5x7, PF_USELASTX, LPRT_OR, text);

	if (invert)
		InvertRegion(frame, rect->sx, rect->sy, rect->ex, rect->ey);

	if (selected){
		drawBox(frame, rect->sx-2, rect->sy-2, rect->ex, rect->ey);

		TLPRINTR rect2;
		memcpy(&rect2, rect, sizeof(TLPRINTR));
		rect2.ey += 2;
		lClearFrameArea(frame, 0, rect2.ey, frame->width-1, rect2.ey+7);
		lPrintEx(frame, &rect2, LFTW_5x7, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY, help);
		
		rect2.ey++;
		lDrawLine(frame, 0, rect2.ey, frame->width-1, rect2.ey, LSP_SET);
	}
}

static unsigned int isHighlighted (unsigned int item)
{
	return item&itemlookup[highlightedItem];
}

static unsigned int isSelected (TNSOPTION *opt, unsigned int item)
{
	return (item&opt->itemFlags);
}

static void addSelection (TNSOPTION *opt, unsigned int item)
{
	opt->itemFlags |= itemlookup[item];
}

static void removeSelection (TNSOPTION *opt, unsigned int item)
{
	opt->itemFlags &= ~itemlookup[item];
}

static void drawCFG (TWINAMP *wa, TFRAME *frame, TNSOPTION *opt, TLPRINTR *rect)
{

	rect->sy = 2;
	rect->ex = 2;
	drawCFGText(frame, rect, isSelected(opt, Protocol), isHighlighted(Protocol),\
	  lng_getString(wa->lang, LNG_NS_PROT), lng_getString(wa->lang, LNG_NS_PROTOCOLTCPUDP));
	  
	drawCFGText(frame, rect, 0, 0, " ", "");
	drawCFGText(frame, rect, isSelected(opt, LocalIP), isHighlighted(LocalIP),\
	  lng_getString(wa->lang, LNG_NS_LOIP), lng_getString(wa->lang, LNG_NS_LOCALIP));
	  
	drawCFGText(frame, rect, 0, 0, ":", "");
	drawCFGText(frame, rect, isSelected(opt, LocalPort), isHighlighted(LocalPort),\
	  lng_getString(wa->lang, LNG_NS_PT), lng_getString(wa->lang, LNG_NS_LOCALPORT));
	  
	drawCFGText(frame, rect, 0, 0, " ", "");
	drawCFGText(frame, rect, isSelected(opt, RemoteIP), isHighlighted(RemoteIP),\
	  lng_getString(wa->lang, LNG_NS_REIP), lng_getString(wa->lang, LNG_NS_REMOTEIP));
	  
	drawCFGText(frame, rect, 0, 0, ":", "");
	drawCFGText(frame, rect, isSelected(opt, RemotePort), isHighlighted(RemotePort),\
	  lng_getString(wa->lang, LNG_NS_PT), lng_getString(wa->lang, LNG_NS_REMOTEPORT));
	  
	drawCFGText(frame, rect, 0, 0, " ", "");
	drawCFGText(frame, rect, isSelected(opt, PID), isHighlighted(PID),\
	  lng_getString(wa->lang, LNG_NS_PID), lng_getString(wa->lang, LNG_NS_PROCESSID));
	  
	drawCFGText(frame, rect, 0, 0, " ", "");
	drawCFGText(frame, rect, isSelected(opt, FilePath), isHighlighted(FilePath),\
	  lng_getString(wa->lang, LNG_NS_FILE), lng_getString(wa->lang, LNG_NS_FILENAME));
	  
	drawCFGText(frame, rect, 0, 0, " ", "");
	drawCFGText(frame, rect, isSelected(opt, State), isHighlighted(State),\
	  lng_getString(wa->lang, LNG_NS_STATE), lng_getString(wa->lang, LNG_NS_TCPCONNSTATE));
}


int updateTables (TWINAMP *wa)
{
	if (!nsOption.flags){
		int totalpids = enumPids(nsOption.pids);
		nsOption.total = gettcptable(wa, frame, nsOption.pids, totalpids, 0, &nsOption.netstat[0]);
		nsOption.total += getudptable(wa, frame, nsOption.pids, totalpids, nsOption.total, &nsOption.netstat[nsOption.total]);
	}else{
		nsOption.flags = 0;
	}
	return nsOption.total;
}

int initNetstat (TWINAMP *wa, TFRAME *frame)
{
	if (nsInitOnce) return 1;
	nsInitOnce = 1;
	addressOnce = 0;

	//nsOption.itemFlags = Protocol|LocalPort|RemoteIP|RemotePort|FilePath;
	//nsOption.flags = 0;
	//nsOption.total = 0;
	//nsOption.ipLookup = 0;
	
	if (hNtdll == NULL){
		hNtdll = LoadLibrary("ntdll.dll");
	if (!hNtdll){
			dbprintf( "netstat: LoadLibrary() failed, GetLastError:%d\n",(int)GetLastError());
			return 0;
		}
	}

	Nt_QuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(hNtdll, "NtQuerySystemInformation");
	Rtl_InitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress(hNtdll, "RtlInitUnicodeString");
	Zw_OpenSection = (ZWOPENSECTION)GetProcAddress(hNtdll, "ZwOpenSection");
	//InitializeObjectAttributes = (INITIALIZEOBJECTATTRIBUTES)GetProcAddress(hNtdll, "InitializeObjectAttributes");
	Zw_OpenFile = (ZWOPENFILE)GetProcAddress(hNtdll, "ZwOpenFile");
	
	if (((int)Nt_QuerySystemInformation && (int)Rtl_InitUnicodeString && (int)Zw_OpenSection && (int)Zw_OpenFile) == 0){
		dbprintf("netstat: a function handle was not found\n");
		return 0;
	}

	updateTables(wa);
	return 1;
}

void closeNetstat ()
{
	nsInitOnce = 0;
	if (hNtdll)
		FreeLibrary(hNtdll);
	hNtdll = NULL;
}

static void sortDescending (int *list, int total)
{
	if (!total) return;
	int i = --total;
	int j;
	
	while(i--){
		for (j = 0; j < total; j++){
			if (list[j] < list[j+1])
				_swapInt(&list[j], &list[j+1]);
		}
	}
}

int isPidInList (int *list, unsigned int count, int pid)
{
	int j;
	for (j = 0; j < count; j++){
		if (list[j] == pid)
			return 1;
	}
	return 0;
}

int getExternalIPAddress (char *url, char *searchOpen, char *searchClose, char *ip)
{

	//char buffer[2048+64];
	//memset(buffer, 0, sizeof(buffer));
	*ip = 0;

	size_t totalRead = 0;
	
	char *buffer = GetUrl(url, &totalRead);
	if (buffer){
		char *found = strstr(buffer, searchOpen);
		if (found){
			int solen = strlen(searchOpen);
			if ((found-buffer)+solen < sizeof(buffer)-32){
				char *pip = strtok(found+solen, searchClose);
				if (pip){
					size_t len = strlen(pip);
					if (len){
						strcpy(ip, pip);
						//printf("-%s-\n",pip);
						free(buffer);
						return len;
					}
				}
			}
		}
		free(buffer);
	}
	return 0;
}

static void drawConnections (TWINAMP *wa, TFRAME *frame, TNSOPTION *opt, TLPRINTR *rect)
{
	TNETSTAT *pnetstat = NULL;
	int total = updateTables(wa);
	
	if (displayMode == 1 || displayMode == 2){
		if (startIndex > total-1)
			startIndex = total-1;
		int flags = PF_USELASTX;
		int i;
		
		for (i = startIndex; i < total; i++){
			rect->sx = rect->ex = 0;
			pnetstat = &nsOption.netstat[i];

			if (isSelected(opt, Protocol))
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%s ",protocollookup[pnetstat->proto]);
			
			if (isSelected(opt, LocalIP))
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%s",pnetstat->local.ip);

			if (isSelected(opt, LocalIP) && isSelected(opt, LocalPort))
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, ":");

			if (isSelected(opt, LocalPort))
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%i",pnetstat->local.port);
			
			if (isSelected(opt, LocalIP) || isSelected(opt, LocalPort))
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, " ");
		
			if (isSelected(opt, RemoteIP) && pnetstat->proto == 1  && pnetstat->state != MIB_TCP_STATE_LISTEN)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%s",pnetstat->remote.ip);
			
			if (isSelected(opt, RemoteIP) && isSelected(opt, RemotePort) && pnetstat->proto == 1  && pnetstat->state != MIB_TCP_STATE_LISTEN)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, ":");
			
			if (isSelected(opt, RemotePort) && pnetstat->proto == 1  && pnetstat->state != MIB_TCP_STATE_LISTEN)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%i", pnetstat->remote.port);
			
			if ((isSelected(opt, RemoteIP) || isSelected(opt, RemotePort)) && pnetstat->proto == 1 && rect->ex < frame->width-6 && pnetstat->state != MIB_TCP_STATE_LISTEN)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, " ");
			
			if (isSelected(opt, PID) && rect->ex < frame->width-6)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%i ", pnetstat->process.id);
			
			if (isSelected(opt, FilePath) /*&& pnetstat->proto == 1*/ && rect->ex < frame->width-10)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%s ", pnetstat->process.module);
			
			if (isSelected(opt, State) && pnetstat->proto == 1 && rect->ex < frame->width-10)
				lPrintEx(frame, rect, LFTW_5x7, flags, LPRT_CPY, "%s ", tcpStateToText(wa, pnetstat->state));

			rect->sy += 7;
			if (rect->sy > frame->height-4)
				break;
		}
	}else if (displayMode == 3){
		if (startIndex > total-1)
			startIndex = total-1;
		pnetstat = &nsOption.netstat[startIndex];

		lPrintEx(frame, rect, LFTW_5x7, PF_RESETXY|PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_CPY, "%s %i/%i",\
		  lng_getString(wa->lang, LNG_NS_ENDPOINT), startIndex+1, total);

		if (pnetstat->proto == 1){
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETXY, LPRT_CPY, "%s: TCP", lng_getString(wa->lang, LNG_NS_PROTOCOL));
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %s:%i",\
			  lng_getString(wa->lang, LNG_NS_LOCAL), pnetstat->local.ip, pnetstat->local.port);
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %s:%i",\
			  lng_getString(wa->lang, LNG_NS_REMOTE), pnetstat->remote.ip, pnetstat->remote.port);
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %i",\
			  lng_getString(wa->lang, LNG_NS_PROCESSID), pnetstat->process.id);
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %s",\
			  lng_getString(wa->lang, LNG_NS_MODULE), pnetstat->process.module);
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %s",\
			  lng_getString(wa->lang, LNG_NS_CONNECTIONSTATE), tcpStateToText(wa, pnetstat->state));

		}else if (pnetstat->proto == 2){
		//	rect->ey = rect->sy = 0;
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETXY, LPRT_CPY, "%s: UDP",\
			  lng_getString(wa->lang, LNG_NS_PROTOCOL));
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %s:%i",\
			  lng_getString(wa->lang, LNG_NS_LOCAL), pnetstat->local.ip, pnetstat->local.port);
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %i",\
			  lng_getString(wa->lang, LNG_NS_PROCESSID), pnetstat->process.id);			
			lPrintEx(frame, rect, LFTW_5x7, PF_RESETX|PF_NEWLINE, LPRT_CPY, "%s: %s",\
			  lng_getString(wa->lang, LNG_NS_MODULE), pnetstat->process.module);
		}
	}else if (displayMode == 4){
		int list[total];
		memset(list, 0, sizeof(list));
		unsigned int count = 0;
		int i;
		for (i = 0; i < total; i++){
			if (!isPidInList(list, count, nsOption.netstat[i].process.id)){
				list[count++] = nsOption.netstat[i].process.id;
			}
		}
		int end = ((count<<1)+total)-(6-(opt->ipLookup*2));
		if (startIndex > end)
			startIndex = end;
			
		rect->sy = -(startIndex*7);
		sortDescending(list, total);
		int tmppid = -1;
		int pid;
		int ip = 0;

		for (pid = 0; pid < count; pid++){
			pnetstat = nsOption.netstat;

			if (opt->ipLookup){
				if (rect->sy > frame->height-1)
					return;
				if (!ip){
					ip = 1; // only render ip address once
					if (rect->sy >= 0){
						lPrintEx(frame, rect, LFTW_5x7, PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, "%s: %s",\
						  lng_getString(wa->lang, LNG_NS_YOURIP), opt->ipAddress);
					}
					rect->ey = rect->sy += 14;
				}
			}
			for (i = 0; i < total; i++){
				if (list[pid] == pnetstat->process.id){
					if (rect->sy > frame->height-1)
						return;

					if (tmppid != pid){
						tmppid = pid;	// only render module name once per pass
						if (rect->sy >= 0){
							if (!pnetstat->process.id){
								lPrintEx(frame, rect, LFTW_5x7, PF_RESETX, LPRT_CPY, "%i: %s (TCP)",\
								  pid+1, pnetstat->process.module);
							}else{
								lPrintEx(frame, rect, LFTW_5x7, PF_RESETX, LPRT_CPY, "%i: %i %.1fmb %s",\
								  pid+1, pnetstat->process.id, (float)((float)GetMemUsage(pnetstat->process.id)/1024.0/1024.0), pnetstat->process.module);
							}
						}
						rect->ey = rect->sy += 7;
					}
					if (rect->sy > frame->height-1)
						return;
						
					if (rect->sy >= 0){
						if (pnetstat->proto == 1){
							if (pnetstat->state != MIB_TCP_STATE_LISTEN){
								if (!pnetstat->process.id){
									lPrintEx(frame, rect, LFTW_5x7, PF_RESETX, LPRT_CPY, "%s:%i %s:%i %s",\
									  pnetstat->local.ip, pnetstat->local.port, pnetstat->remote.ip, pnetstat->remote.port, tcpStateToText(wa, pnetstat->state));
								}else{
									lPrintEx(frame, rect, LFTW_5x7, PF_RESETX, LPRT_CPY, "TCP %s:%i %s:%i %s",\
									  pnetstat->local.ip, pnetstat->local.port, pnetstat->remote.ip, pnetstat->remote.port, tcpStateToText(wa, pnetstat->state));
								}
							}else{
								lPrintEx(frame, rect, LFTW_5x7, PF_RESETX, LPRT_CPY, "TCP %s:%i %s",\
								  pnetstat->local.ip, pnetstat->local.port, tcpStateToText(wa, pnetstat->state));
							}

						}else if (pnetstat->proto == 2){
							lPrintEx(frame, rect, LFTW_5x7, PF_RESETX, LPRT_CPY, "UDP %s:%i *:*",\
							  pnetstat->local.ip, pnetstat->local.port);
						}						
					}
					rect->ey = rect->sy += 7;
				}
				pnetstat++;
			}
			rect->ey = rect->sy += 7;
		}
	}
}



int renderNetstat (TWINAMP *wa, TFRAME *frame, void *data)
{
	if (!nsInitOnce){
		if (initNetstat(wa, frame))
			return 0;
	}
		
	TNSOPTION *opt = (TNSOPTION *)data;

	if (!addressOnce && opt->ipLookup){
		addressOnce = 1;
		if (getExternalIPAddress("http://whatsmyip.org/index.php", "Your IP is ", "<", opt->ipAddress)){
			//printf("-%s-\n",opt->ipAddress);
		}
	}
	
	TLPRINTR rect = {0, 0, frame->width-1,frame->height-1,0,0,0,0};
	lSetCharacterEncoding(wa->hw, wa->lang->enc);
	lClearFrame(frame);
	
	if (displayMode == 1){
		drawCFG(wa, frame, opt, &rect);
		rect.ey += 12;
		rect.sy = rect.ey;
		rect.sx = 0;
	}
	drawConnections(wa, frame, opt, &rect);
	return 1;
}

int displayInput_Netstat (TWINAMP *wa, int key, void *data)
{
	if (!nsInitOnce){
		if (initNetstat(wa, frame))
			return -1;
	}

	TNSOPTION *opt = (TNSOPTION *)data;
	opt->flags = 1;

	if (isKeyPressed(VK_LSHIFT)){
		return -1;
		
	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		if (displayMode == 1){
			if (--highlightedItem < 0)
				highlightedItem = 7;
			renderFrame(wa, DISF_NETSTAT);

		}else{
			if (--startIndex < 0)
				startIndex = 0;
			renderFrame(wa, DISF_NETSTAT);
		}
	}else if (key == G15_WHEEL_CLOCKWISE){
		if (displayMode == 1){
			if (++highlightedItem > 7)
				highlightedItem = 0;
			renderFrame(wa, DISF_NETSTAT);

		}else{
			startIndex++;
			renderFrame(wa, DISF_NETSTAT);
		}
	}else if (key == G15_SOFTKEY_4){
		if (displayMode == 1){
			if (isSelected(opt, itemlookup[highlightedItem]))
				removeSelection(opt, highlightedItem);
			else
				addSelection(opt, highlightedItem);
			renderFrame(wa, DISF_NETSTAT);
		}
	}else if (key == G15_SOFTKEY_3){
		if (++displayMode > 4)
			displayMode = 1;
		renderFrame(wa, DISF_NETSTAT);

	}else{
		return -1;
	}
	return 0;
}

