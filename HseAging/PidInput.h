#pragma once


// CPidInput 대화 상자

class CPidInput : public CDialog
{
	DECLARE_DYNAMIC(CPidInput)

public:
	CPidInput(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CPidInput();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PID_INPUT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:
	int m_nMesAutoRackNo;
	int m_nMesAutoDMOU;
	int m_nTargetRack;

protected:
	LPSYSTEMINFO lpSystemInfo;
	LPMODELINFO lpModelInfo;
	LPINSPWORKINFO lpInspWorkInfo;

	CMFCButton* m_pBtnPidRack[MAX_RACK];
	CStatic* m_pSttChInfo[MAX_LAYER][MAX_LAYER_CHANNEL];
	CButton* m_pChkChannelUse[MAX_LAYER][MAX_LAYER_CHANNEL];
	CEdit* m_pedtPannelID[MAX_LAYER][MAX_LAYER_CHANNEL];

	CEdit* m_pedtBcrChID[MAX_LAYER][MAX_LAYER_CHANNEL]; // 채널 바코드 ID

	CEdit* m_pEdtFocusCtrl[MAX_LAYER][MAX_LAYER_CHANNEL];
	BOOL m_bPIDRuleError[MAX_LAYER][MAX_LAYER_CHANNEL];
	int	m_nedtDlgCtrlID[MAX_LAYER][MAX_LAYER_CHANNEL];
	int m_nSelRack;
	int m_nSelLayer;
	int m_nSelChannel;

	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();
	void Lf_addMessage(CString msg);

	void Lf_ChangeColorRackButton(int selectRack);
	void Lf_updatePanelID();
	void Lf_setFocus();
	void Lf_loadMesStatusInfo();
	void Lf_saveMesStatusInfo();
	void Lf_clearMesStatusInfo();

	int Lf_checkPIDMesInfo(int ctrl_id);
	int Lf_checkPIDErrorInfo(int ctrl_id);
	int Lf_checkPIDValidInfo(int ctrl_id);
	int Lf_checkPIDFocusInfo(int ctrl_id);
	void Lf_InvalidateWindow();
	BOOL Lf_isExistErrorChannel();
	void Lf_AllChannelSelect(int onoff);

	BOOL Lf_setExecuteMesAGNIN();
	BOOL Lf_setExecuteMesPCHK();
	BOOL Lf_setExecuteMesAGNOUT();

	CString m_nMainKeyInData; // 키 입력값 저장
	CString m_nSubKeyInData; // 서브키 입력값 저장
	void Lf_checkBcrRackChIDInput(CString RackID, CString ChID); // 바코드+채널 값 읽는 함수
	void Lf_Send_checkBcrRackChIDInput(CString RackID, CString ChID); // 바코드+채널 값 읽는 함수
	BOOL Pchk_TF = TRUE;
	int Lf_PchkInfo(int ctrl_id);

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
	afx_msg void OnBnClickedMbcPiRack1();
	afx_msg void OnBnClickedMbcPiRack2();
	afx_msg void OnBnClickedMbcPiRack3();
	afx_msg void OnBnClickedMbcPiRack4();
	afx_msg void OnBnClickedMbcPiRack5();
	afx_msg void OnBnClickedMbcPiRack6();
	CGradientStatic m_sttPiRackID;
	CStatic m_sttPiLayer1;
	CStatic m_sttPiLayer2;
	CStatic m_sttPiLayer3;
	CStatic m_sttPiLayer4;
	CStatic m_sttPiLayer5;
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch1();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch2();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch3();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch4();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch5();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch6();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch7();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch8();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch9();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch10();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch11();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch12();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch13();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch14();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch15();
	afx_msg void OnEnSetfocusEdtPiPidLayer1Ch16();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch1();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch2();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch3();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch4();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch5();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch6();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch7();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch8();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch9();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch10();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch11();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch12();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch13();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch14();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch15();
	afx_msg void OnEnSetfocusEdtPiPidLayer2Ch16();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch1();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch2();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch3();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch4();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch5();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch6();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch7();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch8();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch9();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch10();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch11();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch12();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch13();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch14();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch15();
	afx_msg void OnEnSetfocusEdtPiPidLayer3Ch16();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch1();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch2();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch3();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch4();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch5();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch6();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch7();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch8();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch9();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch10();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch11();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch12();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch13();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch14();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch15();
	afx_msg void OnEnSetfocusEdtPiPidLayer4Ch16();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch1();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch2();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch3();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch4();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch5();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch6();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch7();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch8();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch9();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch10();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch11();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch12();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch13();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch14();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch15();
	afx_msg void OnEnSetfocusEdtPiPidLayer5Ch16();
	afx_msg void OnBnClickedMbcPiPidClear();
	CListBox m_lstPiMesMessage;
	afx_msg void OnBnClickedBtnPiMesAgnin();
	afx_msg void OnBnClickedBtnPiMesPchk();
	afx_msg void OnBnClickedBtnPiMesAgnout();
	afx_msg void OnBnClickedBtnPiSaveExit();
	afx_msg void OnBnClickedBtnPiSaveExit_B();
	afx_msg void OnBnClickedBtnPiCancel();
	CButton m_btnPiSaveExit;
	CButton m_btnPiCancel;
	afx_msg void OnBnClickedMbcPiChAllSelect();
	afx_msg void OnBnClickedMbcPiChAllClear();
};
