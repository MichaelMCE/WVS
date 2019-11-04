
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



//#include <semaphore.h>


//static sem_t semTMSNLIST;
//static pthread_mutex_t muteTMSNLIST = PTHREAD_MUTEX_INITIALIZER;
static CRITICAL_SECTION msnlist_cs;
volatile static HANDLE msnlist_hSemLock = INVALID_HANDLE_VALUE;



static void TMSNLIST_LOCK_CREATE ()
{
	/*
	if (pthread_mutex_init(&muteTMSNLIST, 0))
		exit(EXIT_SUCCESS);
	sem_init(&semTMSNLIST, 0, 1);
	*/
	InitializeCriticalSection(&msnlist_cs);
	msnlist_hSemLock = CreateSemaphore(NULL, 1, 1, NULL);
	if (msnlist_hSemLock == INVALID_HANDLE_VALUE){
		dbprintf("WVS: TMSNLIST_LOCK_CREATE() failed: %s\n",strerror(GetLastError()));
	}
}

static void TMSNLIST_LOCK_DELETE ()
{
	/*
	sem_destroy(&semTMSNLIST);
	pthread_mutex_destroy(&muteTMSNLIST);
	*/
	CloseHandle(msnlist_hSemLock);
	DeleteCriticalSection(&msnlist_cs);
	msnlist_hSemLock = INVALID_HANDLE_VALUE;
}

static void TMSNLIST_LOCK ()
{
	/*
	pthread_mutex_lock(&muteTMSNLIST);
	while(sem_wait(&semTMSNLIST) != 0)
		lSleep(1);
	*/
	EnterCriticalSection(&msnlist_cs);
	if (WaitForSingleObject(msnlist_hSemLock, 20000) != WAIT_OBJECT_0)
		printf("TMSNLIST_LOCK(): WaitForSingleObject() Failed to lock: %s\n",strerror(GetLastError()));
}

static void TMSNLIST_UNLOCK ()
{
	/*
	sem_post(&semTMSNLIST);
	pthread_mutex_unlock(&muteTMSNLIST);
	*/
	ReleaseSemaphore(msnlist_hSemLock, 1, NULL);
	LeaveCriticalSection(&msnlist_cs);
}


