

int mix_Open (HMIXER *mixerHandle, int audioDevice);	// device index
int mix_Close (HMIXER mixerHandle);
int mix_GetControlValue (HMIXER mixerHandle, DWORD dwControlID, int *value);
int mix_SetControlValue (HMIXER mixerHandle, DWORD dwControlID, int value);
int mix_GetControlDetails (HMIXER mixerHandle, DWORD dwLineID, int *min, int *max, int *step);




