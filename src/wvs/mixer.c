
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

// 	http://www.borg.com/~jglatt/tech/mixer.htm



int mix_Close (HMIXER mixerHandle)
{
	return (int)mixerClose(mixerHandle);
}

DWORD mix_GetVolumeHandle (HMIXER m_HMixer, DWORD destType, DWORD srcType, DWORD ControlType)
{
	if (mixerGetNumDevs() < 1)
		return 0;

	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwComponentType = destType;
	
	if (mixerGetLineInfo((HMIXEROBJ)m_HMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR)
		return 0;
	if (srcType != NO_SOURCE){
		DWORD DstIndex = mxl.dwDestination;
		unsigned int nconn = mxl.cConnections;
		unsigned int j;
		for (j = 0; j < nconn; j++){
			mxl.cbStruct = sizeof(MIXERLINE);
			mxl.dwSource = j;
			mxl.dwDestination = DstIndex;
			if (mixerGetLineInfo((HMIXEROBJ)m_HMixer, &mxl, MIXER_GETLINEINFOF_SOURCE) != MMSYSERR_NOERROR)
				return 0;
			if (mxl.dwComponentType == srcType)
				break;
		}
	}
	
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = ControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if (mixerGetLineControls((HMIXEROBJ)m_HMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
		return 0;
	else
		return mxc.dwControlID;
}

int mix_GetControlDetails (HMIXER mixerHandle, DWORD dwLineID, int *min, int *max, int *step)
{
	MIXERCONTROL mixerControl;
	MIXERLINECONTROLS  mixerLineControls;
	MMRESULT err;

	mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
	mixerLineControls.cControls = 1;
	mixerLineControls.dwLineID = dwLineID;
	mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mixerLineControls.pamxctrl = &mixerControl;
	mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);

	if ((err = mixerGetLineControls((HMIXEROBJ)mixerHandle, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE))){
    	return 0;
	}else{
		*min = (int)mixerControl.Bounds.lMinimum;
		*max = (int)mixerControl.Bounds.lMaximum;
		*step = (int)mixerControl.Metrics.cSteps;
		return 1;
	}
}

int mix_SetControlValue (HMIXER mixerHandle, DWORD dwControlID, int newvalue)
{
	MIXERCONTROLDETAILS_UNSIGNED value;
	MIXERCONTROLDETAILS mixerControlDetails;
	MMRESULT err;
	
	mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixerControlDetails.dwControlID = dwControlID;
	mixerControlDetails.cChannels = 1;
	mixerControlDetails.cMultipleItems = 0;
	mixerControlDetails.paDetails = &value;
	mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

	if (mixerGetControlDetails((HMIXEROBJ)mixerHandle, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE))
		return 0;
		
	if (newvalue < 0)
		newvalue = 0;
	value.dwValue = (DWORD)newvalue;	

	if ((err = mixerSetControlDetails((HMIXEROBJ)mixerHandle, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE))){
    	return 0;
	}else{
		return 1;
	}
}

int mix_GetControlValue (HMIXER mixerHandle, DWORD dwControlID, int *curvalue)
{
	if (mixerHandle == NULL)
		return 0;

	MIXERCONTROLDETAILS_UNSIGNED value;
	MIXERCONTROLDETAILS mixerControlDetails;
	MMRESULT err;

	mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixerControlDetails.dwControlID = dwControlID;
	mixerControlDetails.cChannels = 1;
	mixerControlDetails.cMultipleItems = 0;
	mixerControlDetails.paDetails = &value;
	mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

	if ((err = mixerGetControlDetails((HMIXEROBJ)mixerHandle, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE))){
    	return 0;
	}else{
		*curvalue = (int)value.dwValue;
    	return 1;
	}
}

int mix_Open (HMIXER *mixerHandle, int audioDevice)
{
	return ~mixerOpen(mixerHandle, audioDevice, 0, 0, CALLBACK_NULL);
}

