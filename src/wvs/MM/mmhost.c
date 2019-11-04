
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



/*
	control = content type
	data = spectrum and/or wave data
*/
int MM_sendSpectrumData (TMP *mp, TMPCMD *cmd)
{
	TMPCMDWAVE wave;
	wave.command = cmd->command;
	wave.control = cmd->data1;
	wave.len = 0;
	wave.channels = 0;
	wave.bpc = 0;
	wave.magic = MAGIC;

	if (mp->mm.visMap != NULL){
		wave.sRate = 44100; //mp->mod->sRate;
		wave.bpc = MAXCHANNELLENGTH;

		//if (WaitForSingleObject(mp->wavedataLock, 2000) == WAIT_OBJECT_0){
			mp->control = cmd->data1;
			if (cmd->data1&0x01){
				wave.len = wave.bpc*2; //mp->mod->nCh;
				memcpy(&wave.data[0], mp->mm.visMap, wave.len);
				wave.channels = 2; //mp->mod->nCh;
			}
			if (cmd->data1&0x02){
				memcpy(&wave.data[wave.len], (mp->mm.visMap+1152), wave.len);
				wave.len += wave.len;
				wave.channels += 2; //mp->mod->nCh;
			}
			//ReleaseSemaphore(mp->wavedataLock, 1, NULL);
		//}
	}

	size_t cmdLen = sizeof(TMPCMDWAVE) - (sizeof(wave.data)-wave.len);
	if (sendSocket(&mp->net, &wave, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

#if 0
/*
	data1 = file pos in playlist
	data2 = size of buffer required for incoming meta field
*/
int MM_sendMetaDataW (TMP *mp, TMPCMD *cmd)
{
	wchar_t buffer[4096];
	memset(buffer, 0, sizeof(buffer));

	wchar_t *meta = calloc(cmd->data2, sizeof(char));	// cmd->data2 length includes terminator
	if (meta == NULL) return 0;

	if (readSocket(&mp->net, meta, cmd->data2) != SOCKET_ERROR){
		mp->mm.pSongList = SetTable(mp->mm.pPlayer, L"CurrentSongList");		// highlighted tracks
		if (mp->mm.pSongList){
			mp->mm.pSongData = SetTable_Int(mp->mm.pSongList, L"Item", cmd->data1);
			if (mp->mm.pSongData){
				GetItem_Str(mp->mm.pSongData, meta, buffer, sizeof(buffer));
			}
		}		
		if (sendString(mp, cmd, buffer, (wcslen(buffer)* sizeof(wchar_t))+sizeof(wchar_t))){
			free(meta);
			return 1;
		}
	}
	free(meta);
	return 0;
}
#endif

/*
	data1 = track index
	data2 = title length (bytes)
*/
int MM_sendTrackFilenameW (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistTrackPath(0, cmd->data2, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

/*
	data1 = track index
	data2 = title length (bytes)
*/
int MM_sendTrackTitleW (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistTrackTitle(0, cmd->data2, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}


/*************************/
/* Multiple Playlist API */

int MM_sendPlaylistName (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistName(cmd->data1, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int MM_sendTotalPlaylists (TMP *mp, TMPCMD *cmd)
{
	cmd->data1 = GetTotalPlaylists();
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

int MM_sendPlaylistTrackTotal (TMP *mp, TMPCMD *cmd)
{
	cmd->data2 = 0;
	GetPlaylistTotalTracks(cmd->data1, &cmd->data2);
	size_t cmdLen = sizeof(TMPCMD);
	if (sendSocket(&mp->net, cmd, &cmdLen) != SOCKET_ERROR)
		return 1;
	return 0;
}

int MM_sendPlaylistTrackTitle (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistTrackTitle(cmd->data1, cmd->data2, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int MM_sendPlaylistTrackArtist (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistTrackArtist(cmd->data1, cmd->data2, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int MM_sendPlaylistTrackAlbum (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistTrackAlbum(cmd->data1, cmd->data2, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}

int MM_sendPlaylistTrackPath (TMP *mp, TMPCMD *cmd)
{
	if (GetPlaylistTrackPath(cmd->data1, cmd->data2, mp->buffer.WChar, SENDBUFFERSIZE)){
		size_t itemLen = wcslen(mp->buffer.WChar);
		if (itemLen){
			if (sendString(mp, cmd, mp->buffer.WChar, ++itemLen*sizeof(wchar_t)))
				return 1;
		}
	}

	sendNullString(mp, cmd);
	return 0;
}


