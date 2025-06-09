#pragma once


// CAutoFirmware 대화 상자

#define MAX_FILE_SIZE		(1024*1024*2)		// 2MByte

#define FW_TARGET_PG		0
#define FW_TARGET_DIO		1

class CAutoFirmware : public CDialog
{
	DECLARE_DYNAMIC(CAutoFirmware)

public:
	CAutoFirmware(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CAutoFirmware();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTO_FIRMWARE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:


protected:
	LPMODELINFO		lpModelInfo;
	LPSYSTEMINFO	lpSystemInfo;
	LPINSPWORKINFO	lpInspWorkInfo;
	CStatic*		m_pStaticVer[MAX_RACK][MAX_LAYER];

	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();

	void Lf_disableButton();
	void Lf_enableButton();

	void Lf_clearVersionStatus(int rack);

	BOOL Lf_checkRackSelect();
	void Lf_setCheckFwRack(int rack);

	void Lf_loadFirmwareFile();
	void Lf_readFirmwareFile(CString strFilePath);
	void Lf_parseDataRecord(CString strRecord, BYTE* pData);

	void Lf_readyInitialize();

	BOOL Lf_firmwareDownloadStartPG(int rack);
	BOOL Lf_checkDownloadReady1PG(int rack);
	BOOL Lf_checkDownloadReady2PG(int rack);
	BOOL Lf_sendFirmwareFilePG(int rack);
	BOOL Lf_sendDownloadCompletePG(int rack);

	BOOL Lf_firmwareDownloadStartDIO();
	BOOL Lf_checkDownloadReady1DIO();
	BOOL Lf_checkDownloadReady2DIO();
	BOOL Lf_sendFirmwareFileDIO();
	BOOL Lf_sendDownloadCompleteDIO();

	void Lf_readFirmwareVersion();

	BOOL Lf_isRackEthConnect(int rack);


	int nTestCnt;
	int m_nTargetBoard;
	int	m_nSelRack;

	CString		m_sFirmwareVersionInfo;
	BYTE* m_pFirmwareData;
	int			m_nFirmwareDataLen;

private:
	CFont m_Font[FONT_IDX_MAX];
	CBrush m_Brush[COLOR_IDX_MAX];
	CFont* m_pDefaultFont;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////



public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CMFCButton m_mbcAfFileLoad;
	CMFCButton m_mbcAfStart;
	CMFCButton m_mbcAfVerRead;
	CMFCButton m_mbcAfCancel;
	afx_msg void OnBnClickedMbcAfFileLoad();
	afx_msg void OnBnClickedMbcAfStart();
	afx_msg void OnBnClickedMbcAfCancel();
	CProgressCtrl m_ctrAfProgress;
	afx_msg void OnBnClickedMbcAfVerRead();
	afx_msg void OnBnClickedChkAfRackAll();
	afx_msg void OnBnClickedChkAfRack1();
	afx_msg void OnBnClickedChkAfRack2();
	afx_msg void OnBnClickedChkAfRack3();
	afx_msg void OnBnClickedChkAfRack4();
	afx_msg void OnBnClickedChkAfRack5();
	afx_msg void OnBnClickedChkAfRack6();
	afx_msg void OnBnClickedChkAfDioBoard();
	CButton m_chkAfRackAll;
	CButton m_chkAfRack1;
	CButton m_chkAfRack2;
	CButton m_chkAfRack3;
	CButton m_chkAfRack4;
	CButton m_chkAfRack5;
	CButton m_chkAfRack6;
	CButton m_chkAfDioBoard;
};
