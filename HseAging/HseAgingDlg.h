#include "ErcpTest.h"
// HseAgingDlg.h: 헤더 파일
//

#pragma once


// CHseAgingDlg 대화 상자
class CHseAgingDlg : public CDialogEx
{
// 생성입니다.
public:
	CHseAgingDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	ErcpTest m_dlgErcpTest;
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HSEAGING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnUdpReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUdpReceiveDio(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRs232Receive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateSystemInfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBcrRackIDInput(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:
	int m_nAgingStart[MAX_RACK];
	void Lf_writeRackMLog(int rack, CString sLog);
	void Lf_updateTowerLamp();

	void Lf_setAgingSTART_PID(int rack, int ch);
	void Lf_setAgingSTOP_PID(int rack);
	void Lf_setChannelUseUnuse_PID_ON(int rack, int ch);
	void Lf_setChannelUseUnuse_PID_OFF(int rack);

	CCimNetCommApi* pCimNet;

protected:
	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitButtonIcon();
	void Lf_InitDialogDesign();
	void Lf_InitCobmoRackModelList();

	void Lf_setAgingSTART(int rack);
	void Lf_setAgingSTOP(int rack);
	void Lf_setAgingFUSING(int rack);
	void Lf_setDoorOnOff(int rack);

	void Lf_updateSystemInfo();
	int  Lf_getChannelInfo(int ctrlID);
	int  Lf_getChannelStatus(int rack, int layer, int ch);

	void Lf_getMeasurePower();
	void Lf_getAgingStatus();

	void Lf_updateEthConnectInfo();
	void Lf_updateAgingStatus();
	
	void Lf_setDIOWrite(int outData, int mode);
	void Lf_setDIOBoardInitial();
	void Lf_getDIOStatus();

	void Lf_readSummaryIni(int rack);
	void Lf_getTemperature();
	void Lf_writeTempLog();
	void Lf_updateTempature();
	void Lf_saveTempMinMax();
	void Lf_parseSDR100Packet(char* szpacket);
	CString Lf_getLimitErrorString(int rack, int layer, int ch);
	int  Lf_getAlarmChannel(int layer, int ch);
	void Lf_checkPowerLimitAlarm();
	void Lf_getFirmawareVersion();
	void Lf_updateFirmwareMatching();
	BOOL Lf_checkAgingIDLEMode();
	void Lf_toggleChUseUnuse(int rack, int layer, int ch);
	void Lf_setChannelUseUnuse(int rack);
	void Lf_channelUseButtonShowHide(int rack);
	BOOL Lf_checkCableOpen(int rack);
	void Lf_checkDoorOpenClose();
	void Lf_checkBcrRackIDInput();

	void Lf_writeSensingLog();
	void Lf_checkComplete5MinOver();
	void Lf_savePowerMeasureMinMax(int rack, int layer, int ch);
	void Lf_flickerCompleteRackNumber();
	void Lf_AgingProgressLog();

	void Lf_rmsErcpSet();

	LPMODELINFO			lpModelInfo;
	LPSYSTEMINFO		lpSystemInfo;
	LPINSPWORKINFO		lpInspWorkInfo;
	CBitmapButton*		m_pBtnOnOff;

	int m_pSttRackCtrlID[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];
	CStatic* m_pSttRackState[MAX_RACK][MAX_LAYER][MAX_LAYER_CHANNEL];

	CStatic* m_pDoorState[MAX_RACK];
	CStatic* m_pSttRackNo[MAX_RACK];
	CComboBox* m_pCmbMaModel[MAX_RACK];
	CProgressCtrl* m_pCtrMaProgress[MAX_RACK];
	CStatic* m_pSttMaSetTime[MAX_RACK];
	CStatic* m_pSttMaRunTime[MAX_RACK];
	CListBox* m_pLstMLog[MAX_RACK];
	CStatic* m_pSttTempInfo[MAX_TEMP_SENSOR];
	CMFCButton m_mbtBuzzOff;
	CMFCButton* m_pBtnAgingStart[MAX_RACK];
	CMFCButton* m_pBtnAgingStop[MAX_RACK];
	CMFCButton* m_pBtnAgingFusing[MAX_RACK];
	CMFCButton* m_pBtnChUseUnuseSet[MAX_RACK];
	CButton* m_pChkChSelect[MAX_RACK];
	CStatic* m_pSttFWVersion[MAX_RACK];

	int m_nMeasureTick;
	int m_nAgingStatusTick;
	int m_nPowerMeasureTick;
	int m_nSensinglogTick;
	int m_nFirmwareRunCount;

	int m_nFirmwareTick;
	double m_nFirmwareTick_d;

	int m_nAgnOutFlag[MAX_RACK];
	int m_bOnOffFlag[MAX_RACK];
	int m_bMcuFwComapre[MAX_RACK];
	int	m_nTempLogWriteMin;
	int m_nSensingLogWriteMin;

	int m_nTempLogWriteHour; // 현재 시간
	int m_nSensingLogWriteHour; // 현재 시간

	int m_nProgressGauge[MAX_RACK];
	int m_nLastLoggerdProgress[MAX_RACK];

	/*CString m_nMainKeyInData;*/
	bool m_nUseKeyInData;
	CString m_nSendKeyInData;

	BOOL m_bFwMismatchNotified[MAX_RACK];

private:
	CFont m_Font[FONT_IDX_MAX];
	CBrush m_Brush[COLOR_IDX_MAX];
	CFont* m_pDefaultFont;
	
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



public:
	CBitmapButton m_btnIconUser;
	CBitmapButton m_btnIconMonitoring;
	CBitmapButton m_btnIconModel;
	CBitmapButton m_btnIconPIDInput;
	CBitmapButton m_btnIconFirmware;
	CBitmapButton m_btnIconSystem;
	CBitmapButton m_btnIconExit;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnMaUser();
	afx_msg void OnBnClickedBtnMaMonitoring();
	afx_msg void OnBnClickedBtnMaModel();
	afx_msg void OnBnClickedBtnMaSystem();
	afx_msg void OnBnClickedBtnMaPidInput();
	afx_msg void OnBnClickedBtnMaFirmware();
	afx_msg void OnBnClickedBtnMaExit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CGradientStatic m_sttMaUserName;
	CGradientStatic m_sttMaMLogRack1;
	CGradientStatic m_sttMaMLogRack2;
	CGradientStatic m_sttMaMLogRack3;
	CGradientStatic m_sttMaMLogRack4;
	CGradientStatic m_sttMaMLogRack5;
	CGradientStatic m_sttMaMLogRack6;
	CListBox m_lstMaMLogRack1;
	CListBox m_lstMaMLogRack2;
	CListBox m_lstMaMLogRack3;
	CListBox m_lstMaMLogRack4;
	CListBox m_lstMaMLogRack5;
	CListBox m_lstMaMLogRack6;
	CComboBox m_cmbMaModelRack1;
	CComboBox m_cmbMaModelRack2;
	CComboBox m_cmbMaModelRack3;
	CComboBox m_cmbMaModelRack4;
	CComboBox m_cmbMaModelRack5;
	CComboBox m_cmbMaModelRack6;
	CProgressCtrl m_ctrMaProgressRack1;
	CProgressCtrl m_ctrMaProgressRack2;
	CProgressCtrl m_ctrMaProgressRack3;
	CProgressCtrl m_ctrMaProgressRack4;
	CProgressCtrl m_ctrMaProgressRack5;
	CProgressCtrl m_ctrMaProgressRack6;
	CGradientStatic m_sttTempSensor;
	CGradientStatic m_sttTempSensor1T;
	CGradientStatic m_sttTempSensor2T;
	CGradientStatic m_sttTempSensor3T;
	CGradientStatic m_sttTempSensor4T;
	CGradientStatic m_sttTempSensor5T;
	CGradientStatic m_sttTempSensor6T;
	CGradientStatic m_sttConnectInfo;
	afx_msg void OnBnClickedMbcMaFusingRack1();
	afx_msg void OnBnClickedMbcMaFusingRack2();
	afx_msg void OnBnClickedMbcMaFusingRack3();
	afx_msg void OnBnClickedMbcMaFusingRack4();
	afx_msg void OnBnClickedMbcMaFusingRack5();
	afx_msg void OnBnClickedMbcMaFusingRack6();
	afx_msg void OnBnClickedMbcMaStartRack1();
	afx_msg void OnBnClickedMbcMaStartRack2();
	afx_msg void OnBnClickedMbcMaStartRack3();
	afx_msg void OnBnClickedMbcMaStartRack4();
	afx_msg void OnBnClickedMbcMaStartRack5();
	afx_msg void OnBnClickedMbcMaStartRack6();
	afx_msg void OnBnClickedMbcMaStopRack1();
	afx_msg void OnBnClickedMbcMaStopRack2();
	afx_msg void OnBnClickedMbcMaStopRack3();
	afx_msg void OnBnClickedMbcMaStopRack4();
	afx_msg void OnBnClickedMbcMaStopRack5();
	afx_msg void OnBnClickedMbcMaStopRack6();
	afx_msg void OnStnClickedChEnableDisable(UINT nID);
	afx_msg void OnBnClickedMbcMaChSetRack1();
	afx_msg void OnBnClickedMbcMaChSetRack2();
	afx_msg void OnBnClickedMbcMaChSetRack3();
	afx_msg void OnBnClickedMbcMaChSetRack4();
	afx_msg void OnBnClickedMbcMaChSetRack5();
	afx_msg void OnBnClickedMbcMaChSetRack6();
	afx_msg void OnBnClickedChkMaSelectRack1();
	afx_msg void OnBnClickedChkMaSelectRack2();
	afx_msg void OnBnClickedChkMaSelectRack3();
	afx_msg void OnBnClickedChkMaSelectRack4();
	afx_msg void OnBnClickedChkMaSelectRack5();
	afx_msg void OnBnClickedChkMaSelectRack6();
	afx_msg void OnBnClickedMbtMaBuzzOff();
	afx_msg void OnBnClickedButtonDoor1();
	afx_msg void OnBnClickedButtonDoor2();
	afx_msg void OnBnClickedButtonDoor3();
	afx_msg void OnBnClickedButtonDoor4();
	afx_msg void OnBnClickedButtonDoor5();
	afx_msg void OnBnClickedButtonDoor6();
};
