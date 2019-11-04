
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


#include <windows.h>

LRESULT CALLBACK HookProc (int nCode, WPARAM wParam, LPARAM lParam);
static HANDLE hNotifyWnd = NULL;
static HANDLE hHook = NULL;
static HANDLE hDllInstance = NULL;
static void *lptr = NULL;
static int kbhook = 0;


/*
BOOL WINAPI _DllMainCRTStartup (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	//DisableThreadLibraryCalls(hInst);
	return TRUE;
}
*/

BOOL WINAPI DllMain (HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hDllInstance = hInstDLL;
//	DisableThreadLibraryCalls(hInstDLL);
	return TRUE;
}

__declspec (dllexport) BOOL SetHook (HWND hWnd, void *userPtr)
{
	if (hWnd == NULL)
		return FALSE;

	if (hNotifyWnd != NULL)
		return FALSE;

	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)HookProc, hDllInstance, 0);
	if (hHook != NULL){
		kbhook = 0;
		lptr = userPtr;
		hNotifyWnd = hWnd;
		return TRUE;
	}else{
		return FALSE;
	}
}

__declspec (dllexport) BOOL UnHook (HWND hWnd)
{
	BOOL unHooked = TRUE;
	if (hNotifyWnd != NULL && hHook != NULL)
		unHooked = UnhookWindowsHookEx(hHook);

	hHook = NULL;
	hNotifyWnd = NULL;
	kbhook = 0;
	return unHooked;
}

__declspec (dllexport) int kbHookGetStatus ()
{
	return (kbhook && hHook);
}

__declspec (dllexport) void kbHookOn ()
{
	kbhook = 1;
}

__declspec (dllexport) void kbHookOff ()
{
	kbhook = 0;
}


LRESULT CALLBACK HookProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && hNotifyWnd){
		LPKBDLLHOOKSTRUCT kbhs = (LPKBDLLHOOKSTRUCT)lParam;
		if ((kbhs->vkCode == VK_VOLUME_DOWN || kbhs->vkCode == VK_VOLUME_UP) && wParam == WM_KEYDOWN){
			PostMessage(hNotifyWnd, 3000, kbhs->vkCode, (LPARAM)lptr);
			return 1;
			
		}else if (kbhs->vkCode == VK_LSHIFT || kbhs->vkCode == VK_RSHIFT){
			PostMessage(hNotifyWnd, wParam, kbhs->vkCode, (LPARAM)lptr);
			
		}else if ((kbhs->vkCode == VK_MEDIA_PREV_TRACK || kbhs->vkCode == VK_MEDIA_STOP ||
			  kbhs->vkCode == VK_MEDIA_PLAY_PAUSE || kbhs->vkCode == VK_MEDIA_NEXT_TRACK) && wParam == WM_KEYDOWN){
			PostMessage(hNotifyWnd, 6000, kbhs->vkCode, (LPARAM)lptr);
			
		}else if ((kbhs->vkCode == VK_BACK || kbhs->vkCode == VK_RETURN || kbhs->vkCode == VK_OEM_2/* forward slash */) && !kbhook){
			PostMessage(hNotifyWnd, WM_KEYDOWN, kbhs->vkCode, (LPARAM)lptr);
			
		}else if (kbhook){
			if (kbhs->vkCode != VK_LWIN && kbhs->vkCode != VK_RWIN && kbhs->vkCode != VK_APPS && kbhs->vkCode != VK_LMENU && kbhs->vkCode != VK_RMENU){
				if (wParam == WM_KEYDOWN){
					PostMessage(hNotifyWnd, WM_KEYDOWN, kbhs->vkCode, (LPARAM)lptr);
					return 1;
				}else if (wParam == WM_KEYUP && kbhs->vkCode != VK_OEM_2 && kbhs->vkCode != VK_RCONTROL && kbhs->vkCode != VK_LCONTROL && kbhs->vkCode != VK_CONTROL){
					return 1;
				}
			}
		}
	}
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

