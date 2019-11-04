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



#include <tlhelp32.h>
#include <psapi.h>



typedef struct {
	char module[MAX_PATH];
	int	pid;
	int	thrds;
}TPROCMOD;



int displayInput_Proclist (TWINAMP *wa, int key, void *data);
int renderProcList (TWINAMP *wa, TFRAME *frame, void *data);



int proclistPageRender (TWINAMP *wa, TFRAME *frame, void *userPtr)
{
	return renderProcList(wa, frame, userPtr);
}

int proclistPageInput (TWINAMP *wa, int key, void *userPtr)
{
	return displayInput_Proclist(wa, key, userPtr);
}

int proclistPageEnter (TFRAME *frame, void *userPtr)
{
	//dbprintf("proclistPageEnter\n");
	return 1;
}

void proclistPageExit (void *userPtr)
{
	//dbprintf("proclistPageExit\n");
}

int proclistPageOpen (TFRAME *frame, void *userPtr)
{
	//dbprintf("proclistPageOpen()\n");
	return 1;
}

void proclistPageClose (void *userPtr)
{
	//dbprintf("proclistPageClose()\n");
}

int getProcTotal (HANDLE hSnap)
{
	int total = 0;
	PROCESSENTRY32 proc;
	proc.dwSize = sizeof(PROCESSENTRY32);

	Process32First(hSnap, &proc);
	while(Process32Next(hSnap, &proc))
		total++;
		
	return total;
}

static inline int getProcList (HANDLE hSnap, TPROCMOD *procList)
{
	int i = 0;
	PROCESSENTRY32 proc;
	proc.dwSize = sizeof(PROCESSENTRY32);
	
	Process32First(hSnap, &proc);
	while(Process32Next(hSnap, &proc)){
		strncpy(procList->module, proc.szExeFile, strlen(proc.szExeFile)+1);
		
		procList->pid = (int)proc.th32ProcessID;
		procList->thrds = (int)proc.cntThreads;
		procList++;
		i++;
	}
	return i;
}

int SetPrivilege (HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege) 
{ 
	TOKEN_PRIVILEGES tp = { 0 }; 
	// Initialize everything to zero 
	LUID luid; 
	DWORD cb = sizeof(TOKEN_PRIVILEGES); 
	if (!LookupPrivilegeValue(NULL, Privilege, &luid))
		return FALSE; 
	tp.PrivilegeCount = 1; 
	tp.Privileges[0].Luid = luid; 
	if (bEnablePrivilege){ 
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	}else{ 
		tp.Privileges[0].Attributes = 0; 
	} 
	AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL); 
	if (GetLastError() != ERROR_SUCCESS) 
		return FALSE; 

	return TRUE;
}

static int killProcessOpenThread (int pid)
{

	HANDLE hToken = NULL;
	HANDLE hProcess = NULL;
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)){
		if (GetLastError() == ERROR_NO_TOKEN){
			if (!ImpersonateSelf(SecurityImpersonation))
				return 0;

			if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken))
            	return 0;

		}else{
            return 0;
		}
	}
    if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)){
		CloseHandle(hToken);
        return 0;
    }
    if ((hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid)) == NULL)
        return 0;

    SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);

    if (!TerminateProcess(hProcess, 0xffffffff))
        return 0;

    CloseHandle(hToken);
    CloseHandle(hProcess);
    return 1;
}

static int killProcessRemoteThread (int pid)
{
	HANDLE hProcess = OpenProcess(0x000F0000 | 0x00100000 | 0xFFF, 1, pid);
	if (hProcess){
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,(void*)GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"ExitProcess"),0,0, &dwThreadId);
		if (hThread){
			WaitForSingleObject(hThread, 6);
			CloseHandle(hThread);
			CloseHandle(hProcess);
			return 1;
		}else{
			if (TerminateProcess(hProcess, 0xffffffff)){
				CloseHandle(hProcess);
				return 1;
			}
			CloseHandle(hProcess);
			return 0;
		}
	}
	return 0;
}

static int killProcessWindow (DWORD pid)
{
	DWORD test_pid;
	HANDLE hwnd = FindWindow(0,0);
	while (hwnd){
 		if (!GetParent(hwnd)){
			GetWindowThreadProcessId(hwnd, &test_pid);
     		if (test_pid == pid){
     			DWORD proc = 0;
				if (IsWindow(hwnd)){
	     			PostThreadMessage(GetWindowThreadProcessId(hwnd, &proc), WM_QUIT, 0, 0);
	     			Sleep(1);
	     		}
	     		if (IsWindow(hwnd)){
     				PostMessage(hwnd, WM_DESTROY , 0, 0);
     				Sleep(1);
     			}
     			if (IsWindow(hwnd)){
     				PostMessage(hwnd, WM_QUIT , 0, 0);
     				Sleep(1);
     			}
     			if (IsWindow(hwnd)){
     				PostMessage(hwnd, WM_CLOSE , 0, 0);
     				Sleep(1);
     			}
     			if (IsWindow(hwnd)){
     				PostMessage(hwnd, SC_CLOSE , 0, 0);
     				Sleep(1);
     			}
     			if (IsWindow(hwnd)){
     				CloseWindow(hwnd);
     				Sleep(1);
     			}
				return IsWindow(hwnd);
     		}
  		}
  		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
 	}
 	return 0;
}

unsigned int GetMemUsage (int pid)
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (NULL == hProcess) return 0;
    
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))){
    	CloseHandle(hProcess);
    	return pmc.WorkingSetSize;
    }else{
    	CloseHandle(hProcess);
    	return 0;
	}
    
}

void PrintMemoryInfo (TWINAMP *wa, TFRAME *frame, int font, TPROCMOD *mod, int pidindex, int start, int *modtotal)
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
	TLPRINTR trect = {0,0,frame->width-1, frame->height-1,0,0,0,0};
   	lPrintEx(frame, &trect, font, PF_RESETXY, LPRT_OR, "%i: %i %i %s", pidindex, mod->pid, mod->thrds, (char*)mod->module);
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, mod->pid);
    if (NULL == hProcess) return;
    
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))){
        int i;
        for (i = start; i < 9; i++){
        	if (i==1){
		        lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "%s: %.2fmb",\
		          lng_getString(wa->lang, LNG_PL_MEMUSAGE), (float)((float)pmc.WorkingSetSize/1024.0/1024.0));
			}
        	if (i==2){
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "%s: %.2fmb",\
        		  lng_getString(wa->lang, LNG_PL_PEAKMEMUSAGE), (float)((float)pmc.PeakWorkingSetSize/1024.0/1024.0));
			}
        	if (i==3){
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "%s: %i",\
        		  lng_getString(wa->lang, LNG_PL_PAGEFAULTS), (int)pmc.PageFaultCount);
        	}
        	if (i==4)
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "PagedPoolUsage: %.2fk", (float)((float)pmc.QuotaPagedPoolUsage/1024.0));
        	if (i==5)
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "PeakPagedPoolUsage: %.2fk", (float)((float)pmc.QuotaPeakPagedPoolUsage/1024.0));
        	if (i==6)
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "NonPagedPoolUsage: %.2fk", (float)((float)pmc.QuotaNonPagedPoolUsage/1024.0));
        	if (i==7)
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "PeakNonPagedPoolUsage: %.2fk", (float)((float)pmc.QuotaPeakNonPagedPoolUsage/1024.0));
        	if (i==8)
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "PagefileUsage: %.0fk",(float)((float)pmc.PagefileUsage/1024.0)); 
        	if (i==9)
        		lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_OR, "PeakPagefileUsage: %.0fk", (float)((float)pmc.PeakPagefileUsage/1024.0));
	        if (trect.ey >= frame->height-1)
				break;
        }
    }
    
    *modtotal = 9;
    CloseHandle(hProcess);
}

BOOL ListProcessModules (TFRAME *frame, int font, TPROCMOD *mod, int pidindex, int start, int *modtotal)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	TLPRINTR trect = {0,0,frame->width-1, frame->height-1,0,0,0,0};
	lPrintEx(frame, &trect, font, PF_RESETXY, LPRT_OR, "%i: %i %i %s", pidindex, mod->pid, mod->thrds, (char*)mod->module);

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, mod->pid);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
		return 0;

	me32.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(hModuleSnap, &me32)){
		CloseHandle(hModuleSnap); 
		return 0;
	}

	int i = 1;
	if (start < 1) start = 1;
	
	int space = lGetFontLineSpacing(frame->hw, font);
	lSetFontLineSpacing(frame->hw, font, space-2);

	do{
		//if ((i >= start) || (i == 1)){
		if (i >= start){
			//if (strcasecmp("ntdll.dll", me32.szModule)){			// don't list a common to all module
				//if (strcasecmp("kernel32.dll", me32.szModule)){	// don't list a common to all module
					//if (strcasecmp(mod->module, me32.szModule)){	// don't list self module
						if (trect.ey < frame->height-1)
							lPrintEx(frame, &trect, font, PF_NEWLINE|PF_WORDWRAP|PF_CLIPWRAP, LPRT_OR, "%s", (char*)me32.szExePath);
					//}
				//}
			//}
		}
		i++;
	}while(Module32Next(hModuleSnap, &me32));

	lSetFontLineSpacing(frame->hw, font, space);
	*modtotal = i;
	CloseHandle(hModuleSnap);
	return 1;
}

int renderProcList (TWINAMP *wa, TFRAME *frame, void *data)
{
	TPROCLIST *pl = (TPROCLIST *)data;

	int *highlight = &pl->highlight;
	int modstart = pl->modStart;
	int *modtotal = &pl->modTotal;
	int killp = pl->killp;
	int columnMode = pl->columnMode;
	int subDisplay = pl->subMode;
	TLPRINTR trect = {0,0,frame->width-1, frame->height-1,0,0,0,0};
	int flags = 0;
	
	const int font = LFTW_5x7;
	const int fontb = LFTW_WENQUANYI9PT;
		
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != NULL){
		lSetCharacterEncoding(wa->hw, wa->lang->enc);
		lClearFrame(frame);
		
#if 1
		TPROCMOD procList[getProcTotal(hSnap)+8];
		memset(procList, 0, sizeof(procList));
#else
		TPROCMOD procList[128];
#endif
		int processTotal = getProcList(hSnap, procList);

		if (*highlight < 1)
			*highlight = 1;
		else if (*highlight > processTotal)
			*highlight = processTotal;

		int start = processTotal-(*highlight)+2;
		if (start > processTotal-1)
			start = processTotal-1;
		else if (start < 2)
			start = 2;

		if (!subDisplay && columnMode){
			trect.sx = 1;
			lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_CPY, lng_getString(wa->lang, LNG_PL_PID));
			trect.sx = 21;
			lPrintEx(frame, &trect, font, 0, LPRT_CPY, lng_getString(wa->lang, LNG_PL_THS));
			trect.sx = 38;
			lPrintEx(frame, &trect, font, 0, LPRT_CPY, lng_getString(wa->lang, LNG_MEM));
			trect.sx = 67;
			lPrintEx(frame, &trect, font, flags, LPRT_CPY, lng_getString(wa->lang, LNG_PL_PROCESS));
		}
		
		int i;
		for (i = start; i >= 0; i--){
			if (i == processTotal-*highlight){
				if (!killp){
					if ((subDisplay == 1) || (subDisplay == 2)){
						if (!columnMode)
							lPrintf(frame, frame->width-12, -3, fontb, LPRT_OR, "&#8678;");	// left arrow
						else
							lPrintf(frame, frame->width-11, -1, fontb, LPRT_OR, "&#8681;");	// down arrow
					
						if (subDisplay == 1){
							ListProcessModules(frame, font, &procList[i], i, modstart, modtotal);
							return *highlight;
	
						}else{
							PrintMemoryInfo(wa, frame, font, &procList[i], i, modstart, modtotal);
							return *highlight;
						}
					}
					flags = PF_INVERTTEXTRECT;
				}else{
					killProcessWindow(procList[i].pid);
					killProcessOpenThread(procList[i].pid);
					killProcessRemoteThread(procList[i].pid);
				}
			}else{
				flags = 0;
			}

			if (!subDisplay){
				trect.sx = 0;
				lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_CPY, "%i", procList[i].pid);
				trect.sx = 22;
				lPrintEx(frame, &trect, font, 0, LPRT_CPY, "%i", procList[i].thrds);
				trect.sx = 37;
				lPrintEx(frame, &trect, font, 0, LPRT_CPY, "%.2f", (float)((float)GetMemUsage(procList[i].pid)/1024.0/1024.0));
				trect.sx = 66;
				lPrintEx(frame, &trect, font, flags, LPRT_CPY, "%s", procList[i].module);

				if (trect.ey >= frame->height-1)
					break;
			}
		}
		CloseHandle(hSnap);
	//	lPrintf(frame, frame->width-12, -3, fontb, LPRT_OR, "&#8678;");	// left arrow
		return *highlight;
	}else{
		return 0;
	}
}

static int iskeyPressed (int vk)
{
	return GetKeyState(vk)&0x80;
}

int displayInput_Proclist (TWINAMP *wa, int key, void *data)
{

	TPROCLIST *proclist = (TPROCLIST *)data;

	if (isKeyPressed(VK_LSHIFT)){
		return -1;
		
	}else if (key == G15_WHEEL_ANTICLOCKWISE){
		if (!proclist->columnMode || !proclist->subMode) {
			proclist->highlight--;
		}else{
			proclist->modStart--;
			if (proclist->modStart < 1)
				proclist->modStart = 1;
		}
		renderFrame(wa, DISF_PROCLIST);

	}else if (key == G15_WHEEL_CLOCKWISE){
		if (!proclist->columnMode || !proclist->subMode) {
			proclist->highlight++;
		}else{
			proclist->modStart++;
			if (proclist->modStart > proclist->modTotal-3)
				proclist->modStart = proclist->modTotal-3;
		}
		renderFrame(wa, DISF_PROCLIST);

	}else if (iskeyPressed(VK_CONTROL) && key == G15_SOFTKEY_3){
		proclist->killp = 1;
		renderFrameBlock(wa, DISF_PROCLIST);		// drawproclist(..,1)  kill highlighted process
		proclist->killp = 0;
		renderFrameBlock(wa, DISF_PROCLIST);		// update frame

	}else if (key == G15_SOFTKEY_3){
		if (++proclist->subMode > 2)
			proclist->subMode = 0;
		proclist->modStart = 1;
		renderFrame(wa, DISF_PROCLIST);
		
	}else if (key == G15_SOFTKEY_4){
		proclist->columnMode ^= 1;
		if (!proclist->columnMode)
			proclist->modStart = 1;
		renderFrame(wa, DISF_PROCLIST);
	}else{
		return -1;
	}
	return 0;
}

