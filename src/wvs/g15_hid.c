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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <setupapi.h>
#if 0
#include <ddk/hidclass.h>
#include <ddk/hidsdi.h>
#include <ddk/hidpi.h>
#include <ddk/hidusage.h>
#else
#include <hidclass.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <hidusage.h>
#endif

#define led1		0x01
#define led2		0x02
#define led3		0x04
#define led4		0x08
#define led_allon	0x00
#define led_alloff	(led1|led2|led3|led4)

static SP_DEVICE_INTERFACE_DATA DID;
static SP_DEVICE_INTERFACE_DETAIL_DATA *DIDD = NULL;
static char DevName[MAX_PATH];
static char CurDeviceSerialNumber[MAX_PATH];
static char devSignG15v1[] = "\\\\?\\hid#vid_046d&pid_c222&col02#";
static char devSignG15v2[] = "\\\\?\\hid#vid_046d&pid_c227&col02#";

static GUID HidGuid;
static HIDD_ATTRIBUTES Attributes;
static HDEVINFO *hDevInfo = NULL;



HANDLE hid_g15_open (int deviceCount)
{

 	HANDLE hDev;
 	int contLoop = 0;
	int memberIndex = 0;
	DWORD RequiredSize = 0;
	DWORD Size = 0;
	DID.cbSize = sizeof(DID);

 	if (deviceCount < 1)
 		deviceCount = 1;
 	else if (deviceCount > 8)
 		deviceCount = 8;

	HidD_GetHidGuid(&HidGuid);
	hDevInfo = SetupDiGetClassDevsA(&HidGuid, 0, 0, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	if (hDevInfo == NULL)
		return NULL;

	do{
    	contLoop = (int)SetupDiEnumDeviceInterfaces(hDevInfo, 0, &HidGuid, memberIndex, &DID);
		if (contLoop){
    		SetupDiGetDeviceInterfaceDetailA(hDevInfo, &DID, 0, 0, &RequiredSize, 0);
    		if (DIDD) free(DIDD);
			DIDD = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, RequiredSize);
			if (DIDD == NULL) return NULL;
			DIDD->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    		Size = RequiredSize;
    		SetupDiGetDeviceInterfaceDetailA(hDevInfo, &DID, DIDD, Size, &RequiredSize, 0);

			if (!strncmp(devSignG15v1, DIDD->DevicePath, strlen(devSignG15v1))){
				//printf("G15v1 found: %s\n", DIDD->DevicePath);
				if (--deviceCount == 0){
					strcpy(DevName, DIDD->DevicePath);
					break;
				}
			}else if (!strncmp(devSignG15v2, DIDD->DevicePath, strlen(devSignG15v2))){
				//printf("G15v2 found \"%s\"\n", DIDD->DevicePath);
				if (--deviceCount == 0){
					strcpy(DevName, DIDD->DevicePath);
					break;
				}
    		}
    	}
		memberIndex++;
	}while(contLoop);

	if (DIDD)
    	free(DIDD);
    DIDD = NULL;
	SetupDiDestroyDeviceInfoList(hDevInfo);

	if (*DevName){
		//printf("using: %s\n", DevName);
		hDev = CreateFileA(DevName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (hDev){
			Attributes.Size = sizeof(Attributes);
			HidD_GetAttributes(hDev, &Attributes);
			HidD_GetProductString(hDev, CurDeviceSerialNumber, MAX_PATH);
			dbwprintf(L"0x%.4X 0x%.4X 0x%.4X \"%s\"\n", Attributes.VendorID, Attributes.ProductID, Attributes.VersionNumber, CurDeviceSerialNumber);
			return hDev;
		}
	}
	return NULL;
}

void hid_g15_close (HANDLE hDev)
{
	if (hDev != NULL)
		CloseHandle(hDev);
}

void hid_g15_setKBBackLightlevel (HANDLE hDev, int level)
{
	if (hDev == NULL) return;

	char kbbl[] = {2, 1, 0, 0, 0, 0, 0, 0} ;		// 0 1 2
	if (!level){
		kbbl[2] = 0;

	}else if (level == 1){
		kbbl[2] = 1;

	}else if (level == 2){
		kbbl[2] = 2;
	}
	HidD_SetFeature(hDev, kbbl, 4);
}

void hid_g15_setLCDBackLightlevel (HANDLE hDev, int level)
{
	if (hDev == NULL) return;

	char lcdbl[] = {2, 2, 0x10, 0, 0, 0, 0, 0};		// 0x00 0x10 0x20
	if (!level){
		lcdbl[2] = 0x00;

	}else if (level == 1){
		lcdbl[2] = 0x10;

	}else if (level == 2){
		lcdbl[2] = 0x20;
	}
	HidD_SetFeature(hDev, lcdbl, 4);
}


void hid_g15_setContrastLevel (HANDLE hDev, int level)
{
	if (hDev == NULL) return;

	char lcdc[] = {2, 32, 129, 36, 0, 0, 0, 0};		// 16 18 26 36
	if (!level){
		lcdc[3] = 18;

	}else if (level == 1){
		lcdc[3] = 22;

	}else if (level == 2){
		lcdc[3] = 26;

	}
	HidD_SetFeature(hDev, lcdc, 4);
}

void hid_g15_setMLEDs (HANDLE hDev, int mleds)
{
	if (hDev == NULL) return;

	char leds[] = {2, 4, mleds, 0, 0, 0, 0, 0};
	HidD_SetFeature(hDev, leds, 4);		// bits 1,2,3,4
}

// m button leds
void hid_g15_setLEDLevel (HANDLE hDev, int level)
{
	if (hDev == NULL) return;

	char leds[] = {2, 4, led_alloff, 0, 0, 0, 0, 0};
	if (!level){
		leds[2] = led_alloff;

	}else if (level == 4){
		leds[2] = led_allon;

	}else if (level == 3){
		leds[2] = led4;

	}else if (level == 2){
		leds[2] = led3|led4;

	}else if (level == 1){
		leds[2] = led2|led3|led4;
	}
	HidD_SetFeature(hDev, leds, 4);
}

char hid_g15_getLEDLevel (HANDLE hDev)
{
	if (hDev == NULL)
		return 0;

	char leds[] = {2, 4, 0, 0, 0, 0, 0, 0};
	HidD_GetFeature(hDev, leds, 4);
	return leds[3];
}

char hid_g15_getLCDBackLightlevel (HANDLE hDev)
{
	if (hDev == NULL)
		return 0;

	char lcdc[] = {2, 1, 0, 0, 0, 0, 0, 0};
	HidD_GetFeature(hDev, lcdc, 4);
	return lcdc[2];
}

char hid_g15_getKBBackLightlevel (HANDLE hDev)
{
	if (hDev == NULL)
		return 0;

	char leds[] = {2, 2, 0, 0, 0, 0, 0, 0};
	HidD_GetFeature(hDev, leds, 4);
	return leds[1];
}

int hid_g15_getState (HANDLE hDev, char buffer[8])
{
	if (hDev == NULL)
		return 0;

	memset(buffer, 0, sizeof(buffer[8]));
	buffer[0] = 2;
	buffer[3] = 0;
	HidD_GetFeature(hDev, buffer, 4);
	return 1;
}

int hid_g15_setState (HANDLE hDev, char buffer[8])
{
	if (hDev == NULL)
		return 0;

	buffer[0] = 2;
	buffer[3] = 0;
	HidD_SetFeature(hDev, buffer, 4);
	return 1;
}
