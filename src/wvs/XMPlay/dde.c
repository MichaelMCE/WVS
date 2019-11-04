

typedef struct _XMPlayData {
    DWORD instID;
    HSZ hszService;
    HSZ hszTopic;
} XMPlayData, *pXMPlayData;



HDDEDATA CALLBACK DdeCallback (UINT uType, UINT uFmt, HCONV hconv, HSZ hszTopic, HSZ hszItem, HDDEDATA hdata, DWORD dwData1, DWORD dwData2)
{
    return (HDDEDATA)DDE_FNOTPROCESSED;
}

static void exeDDECmd (char *cmd)
{

	XMPlayData s_data;
	s_data.instID = 0;
	
    if (DdeInitialize(&s_data.instID, DdeCallback, APPCMD_CLIENTONLY | MF_ERRORS | MF_LINKS | MF_CONV, 0) == DMLERR_NO_ERROR){
        s_data.hszService = DdeCreateStringHandle(s_data.instID, "XMPlay", CP_WINANSI);
        s_data.hszTopic = DdeCreateStringHandle(s_data.instID, cmd, CP_WINANSI);
    }

	HCONV hconv = DdeConnect(s_data.instID, s_data.hszService, s_data.hszTopic, 0);
	HDDEDATA hdata = DdeClientTransaction(cmd, strlen(cmd)+1, hconv, s_data.hszTopic, CF_TEXT, XTYP_EXECUTE, 3000, 0);

	DdeFreeDataHandle(hdata);
	DdeDisconnect(hconv);
	
    if (s_data.instID){
        DdeFreeStringHandle(s_data.instID, s_data.hszService);
        DdeFreeStringHandle(s_data.instID, s_data.hszTopic);
        DdeUninitialize(s_data.instID);
    }
}


