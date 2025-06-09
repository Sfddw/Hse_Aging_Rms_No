#pragma once


// CCommand

class CCommand : public CWnd
{
	DECLARE_DYNAMIC(CCommand)

public:
	CCommand();
	virtual ~CCommand();

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
public:
	int		getPowerOffAckWaitTime();
	int		getPwrSeqIndex(int cmbIndex);
	CString makeSystemFusingData();

	BOOL Gf_setFusingSystemInfo(int rack);
	BOOL Gf_getCableOpenCheck(int rack);
	BOOL Gf_setChannelUseUnuse(CString ip, int* chUseInfo);
	BOOL Gf_setPowerSequenceOnOff(int rack, BOOL onoff, int bAck=ACK);
	BOOL Gf_getAllPowerMeasure(int rack);
	BOOL Gf_getPowerMeasureAllGroup();

	BOOL Gf_getAgingStatusAllGroup(int* skipGroup);

	BOOL Gf_setAgingSTART(int rack);
	BOOL Gf_setAgingSTOP(int rack);
	

	BOOL Gf_getMainBoardFwVersion(int rack);
	BOOL Gf_getMainBoardFwVersionAll();
	BOOL Gf_setGoToBootSection(int rack);
	BOOL Gf_setDownloadData(int rack, int len, char* packet);
	BOOL Gf_setDownloadComplete(int rack);

	BOOL Gf_dio_setDIOBoardInitial();
	BOOL Gf_dio_setDIOWriteOutput(int color, int mode);
	BOOL Gf_dio_getDIOReadStatus();
	BOOL Gf_dio_getDIOBoardFwVersion();
	BOOL Gf_dio_setGoToBootSection();
	BOOL Gf_dio_setDownloadData(int len, char* packet);
	BOOL Gf_dio_setDownloadComplete();

protected:
	LPMODELINFO		lpModelInfo;
	LPSYSTEMINFO	lpSystemInfo;
	LPINSPWORKINFO	lpInspWorkInfo;

/////////////////////////////////////////////////////////////////////////

protected:
	DECLARE_MESSAGE_MAP()


};


