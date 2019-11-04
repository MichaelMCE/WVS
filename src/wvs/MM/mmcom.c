
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




void CloseConnection ()
{
	OleUninitialize();
}

IDispatch *OpenConnection (wchar_t *pdbname)
{
	// Get the CLSID for Word's Application Object
	CLSID clsid;
	
	// Initialize OLE Libraries.
	OleInitialize(NULL);

	// Get CLSID from registry.
	CLSIDFromProgID(pdbname, &clsid);

	// Create an instance of the Word application and obtain the pointer
	// to the application's IUnknown interface
	IUnknown* pUnk;
	IDispatch *m_pDispApp;
	HRESULT hr = CoCreateInstance(&clsid, NULL, CLSCTX_LOCAL_SERVER, &IID_IUnknown, (void**)&pUnk);
	if (FAILED(hr)) {
		//printf("%i\n",(int)hr);
		return NULL;
	}

	// Query IUnknown to retrieve a pointer to the IDispatch interface
	IDispatch_QueryInterface(pUnk, &IID_IDispatch, (void**)&m_pDispApp);
	return m_pDispApp;
}
	
IDispatch *SetTable (IDispatch *pDispApp, wchar_t *pinterface)
{
	// Get pointer to interface.
	DISPPARAMS dp = {NULL, NULL, 0, 0};  
	DISPID dispID;
	LPOLESTR szDoc = pinterface;   
	VARIANT varRetVal;
	EXCEPINFO excepInfo;
	
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispID);
	hr = IDispatch_Invoke(pDispApp, dispID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dp, &varRetVal, &excepInfo, NULL);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}

	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *SetTable_Disp (IDispatch *pDispApp, wchar_t *pinterface, IDispatch *pDisp)
{
	// Get pointer to interface.
	//DISPPARAMS dp = {NULL, NULL, 0, 0}; 
	VARIANT varRetVal;
	VARIANTARG varg;
	EXCEPINFO excepInfo;
	DISPPARAMS dpOpen; // = {&varg, NULL, 0, 0};

	varg.vt = VT_DISPATCH;
	varg.pdispVal = pDisp;

	dpOpen.cArgs = 1;
	dpOpen.rgvarg = &varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
		
	DISPID dispPrintID;
	LPOLESTR szPrintDoc = pinterface;
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szPrintDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispPrintID);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispPrintID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	
	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *SetTable_Var (IDispatch *pDispApp, wchar_t *pinterface, VARIANTARG *varg, int cArgs)
{
	// Get pointer to interface.
	//DISPPARAMS dp = {NULL, NULL, 0, 0}; 
	VARIANT varRetVal;
	EXCEPINFO excepInfo;
	
	DISPPARAMS dpOpen; // = {&varg, NULL, 0, 0};

	dpOpen.cArgs = cArgs;
	dpOpen.rgvarg = varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
		
	DISPID dispPrintID;
	LPOLESTR szPrintDoc = pinterface;
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szPrintDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispPrintID);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispPrintID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	
	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *SetTable_Int (IDispatch *pDispApp, wchar_t *pinterface, int data1)
{
	// Get pointer to interface.
	//DISPPARAMS dp = {NULL, NULL, 0, 0}; 
	VARIANT varRetVal;
	EXCEPINFO excepInfo;
	VARIANTARG varg;
	DISPPARAMS dpOpen; // = {&varg, NULL, 0, 0};
	
	varg.vt = VT_I4;
	varg.lVal = data1;

	dpOpen.cArgs = 1;
	dpOpen.rgvarg = &varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
		
	DISPID dispPrintID;
	LPOLESTR szPrintDoc = pinterface;
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szPrintDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispPrintID);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispPrintID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}

	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *SetTable_Str (IDispatch *pDispApp, wchar_t *pinterface, wchar_t *Str)
{
	// Get pointer to interface.
	//DISPPARAMS dp = {NULL, NULL, 0, 0}; 
	VARIANT varRetVal;
	EXCEPINFO excepInfo;
	VARIANTARG varg;
	DISPPARAMS dpOpen = {&varg, NULL, 0, 0};
	
	varg.vt = VT_BSTR;
	varg.bstrVal = Str;

	dpOpen.cArgs = 1;
	dpOpen.rgvarg = &varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
		
	DISPID dispPrintID;
	LPOLESTR szPrintDoc = pinterface;
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szPrintDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispPrintID);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispPrintID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET|DISPATCH_METHOD, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (FAILED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	
	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *SetItem_Bool (IDispatch *pDispApp, wchar_t *pinterface, short data1, int type)
{
	// Get pointer to interface.
	EXCEPINFO excepInfo;
	DISPPARAMS dpOpen;
	static VARIANTARG varg;

	varg.vt = type; //VT_BOOL;
	varg.cVal = data1;

	dpOpen.cArgs = 1;
	dpOpen.rgvarg = &varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
	
	DISPID dispID;
	LPOLESTR szDoc = pinterface;   
	VARIANT varRetVal;
	
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispID);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT|DISPATCH_METHOD, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *GetItem_Int (IDispatch *pDispApp, wchar_t *pinterface, int *data1)
{
	// Get pointer to interface.
	EXCEPINFO excepInfo;
	DISPPARAMS dpOpen;
	VARIANTARG varg;

	dpOpen.cArgs = 0;
	dpOpen.rgvarg = &varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
	
	DISPID dispID;
	LPOLESTR szDoc = pinterface;   
	VARIANT varRetVal;
	
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispID);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	if (varRetVal.vt == VT_I4)
		*data1 = varRetVal.intVal;

	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	return varRetVal.pdispVal;
}

IDispatch *GetItem_Str (IDispatch *pDispApp, wchar_t *pinterface, wchar_t *buffer, size_t buffersize)
{
	// Get pointer to interface.
	EXCEPINFO excepInfo;
	DISPPARAMS dpOpen;
	VARIANTARG varg;

	dpOpen.cArgs = 0;
	dpOpen.rgvarg = &varg;
	dpOpen.cNamedArgs = 0;
	dpOpen.rgdispidNamedArgs = NULL;
	
	DISPID dispID;
	LPOLESTR szDoc = pinterface;   
	VARIANT varRetVal;
	
	memset(buffer, 0, buffersize);
	HRESULT hr = IDispatch_GetIDsOfNames(pDispApp, &IID_NULL, &szDoc, 1, LOCALE_SYSTEM_DEFAULT, &dispID);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}
	
	hr = IDispatch_Invoke(pDispApp, dispID, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dpOpen, &varRetVal, &excepInfo, NULL);
	if (!SUCCEEDED(hr)){
		//printf("%x\n",(int)hr);
		return NULL;
	}

	//printf("%i %i\n",(int)varRetVal.vt, (int)varRetVal.intVal);
	if (varRetVal.puiVal != NULL)
		wcscpy(buffer, varRetVal.bstrVal);

	return varRetVal.pdispVal;
}

int getNowPlaying_SongData (int trackindex, wchar_t *data, wchar_t *buffer, size_t buffersize)
{
	int ret = 0;
	IDispatch *pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (pRoot){
		IDispatch *pPlayer = SetTable(pRoot, L"Player");
		if (pPlayer){
			IDispatch *pSongData = SetTable_Int(pPlayer, L"PlaylistItems", trackindex);
			if (pSongData){
				GetItem_Str(pSongData, data, buffer, sizeof(buffer));
				ret = 1;
			}
		}
		CloseConnection();
	}
		
	return ret;	
}

int getPlaylist_SongData (int playlist, int trackindex, wchar_t *data, wchar_t *buffer, size_t buffersize)
{
	int ret = 0;
	IDispatch *pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (pRoot){
		IDispatch *pPlayList = SetTable_Str(pRoot, L"PlaylistByTitle", L"");
		if (pPlayList){
			IDispatch *pPlayLists = SetTable(pPlayList, L"ChildPlaylists");
			if (pPlayLists){
				pPlayList = SetTable_Int(pPlayLists, L"Item", playlist);
				if (pPlayList){
					IDispatch *pSongList = SetTable(pPlayList, L"Tracks");
					if (pSongList){
						IDispatch *pSongData = SetTable_Int(pSongList, L"Item", trackindex);
						if (pSongData){
							GetItem_Str(pSongData, data, buffer, sizeof(buffer));
							ret = 1;
						}
					}
				}
			}
		}
		CloseConnection();
	}
	
	return ret;	
}

int getNowPlayingName (wchar_t *buffer, size_t buffersize)
{
	wcscpy(buffer, L"Now Playing");
	return 1;
}

int getNowPlayingTotalTracks (int *tTracks)
{
	int ret = 0;
	IDispatch *pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (pRoot){
		IDispatch *pPlayer = SetTable(pRoot, L"Player");
		if (pPlayer){
			GetItem_Int(pPlayer, L"PlaylistCount", tTracks);
			ret = 1;
		}
		CloseConnection();
	}
		
	return ret;	
}

int GetTotalPlaylists ()
{
	int count = 0;
	IDispatch *pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (pRoot){
		IDispatch *pPlayList = SetTable_Str(pRoot, L"PlaylistByTitle", L"");
		if (pPlayList){
			IDispatch *pPlayLists = SetTable(pPlayList, L"ChildPlaylists");
			if (pPlayLists){
				GetItem_Int(pPlayLists, L"Count", &count);
				count++; // include "Now Playing" as playlist index 0
			}
		}
		CloseConnection();
	}
	
	return count;
}

int GetPlaylistName (int playlist, wchar_t *buffer, size_t buffersize)
{
	if (!playlist)
		return getNowPlayingName(buffer, buffersize);
	else
		playlist--;
	
	int ret = 0;
	IDispatch *pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (pRoot){
		IDispatch *pPlayList = SetTable_Str(pRoot, L"PlaylistByTitle", L"");
		if (pPlayList){
			IDispatch *pPlayLists = SetTable(pPlayList, L"ChildPlaylists");
			if (pPlayLists){
				pPlayList = SetTable_Int(pPlayLists, L"Item", playlist);
				if (pPlayList){
					GetItem_Str(pPlayList, L"Title", buffer, buffersize);
					ret = 1;
				}
			}
		}
		CloseConnection();
	}
	
	return ret;	
}

int GetPlaylistTotalTracks (int playlist, int *tTracks)
{
	if (!playlist)
		return getNowPlayingTotalTracks(tTracks);
	else
		playlist--;

	int ret = 0;
	IDispatch *pRoot = OpenConnection(L"SongsDB.SDBApplication");
	if (pRoot){
		IDispatch *pPlayList = SetTable_Str(pRoot, L"PlaylistByTitle", L"");
		if (pPlayList){
			IDispatch *pPlayLists = SetTable(pPlayList, L"ChildPlaylists");
			if (pPlayLists){
				pPlayList = SetTable_Int(pPlayLists, L"Item", playlist);
				if (pPlayList){
					IDispatch *pSongList = SetTable(pPlayList, L"Tracks");
					if (pSongList){
						GetItem_Int(pSongList, L"Count", tTracks);
						ret = 1;
					}
				}
			}
		}
		CloseConnection();
	}

	return ret;	
}

int GetPlaylistTrackTitle (int playlist, int trackindex, wchar_t *buffer, size_t buffersize)
{
	if (!playlist)
		return getNowPlaying_SongData(trackindex, L"Title", buffer, buffersize);
	else
		return getPlaylist_SongData(--playlist, trackindex, L"Title", buffer, buffersize);
}

int GetPlaylistTrackPath (int playlist, int trackindex, wchar_t *buffer, size_t buffersize)
{
	if (!playlist)
		return getNowPlaying_SongData(trackindex, L"Path", buffer, buffersize);
	else
		return getPlaylist_SongData(--playlist, trackindex, L"Path", buffer, buffersize);
}

int GetPlaylistTrackArtist (int playlist, int trackindex, wchar_t *buffer, size_t buffersize)
{
	if (!playlist)
		return getNowPlaying_SongData(trackindex, L"ArtistName", buffer, buffersize);
	else
		return getPlaylist_SongData(--playlist, trackindex, L"ArtistName", buffer, buffersize);
}

int GetPlaylistTrackAlbum (int playlist, int trackindex, wchar_t *buffer, size_t buffersize)
{
	if (!playlist)
		return getNowPlaying_SongData(trackindex, L"AlbumName", buffer, buffersize);
	else
		return getPlaylist_SongData(--playlist, trackindex, L"AlbumName", buffer, buffersize);
}

