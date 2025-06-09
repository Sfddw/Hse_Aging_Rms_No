#pragma once


// CModelInfo 대화 상자

class CModelInfo : public CDialog
{
	DECLARE_DYNAMIC(CModelInfo)

public:
	CModelInfo(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CModelInfo();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODEL_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////
// 사용자 정의 Function
///////////////////////////////////////////////////////////////////////////
public:


protected:
	LPSYSTEMINFO lpSystemInfo;
	LPMODELINFO lpModelInfo;

	CString curLoadingModel;

	void Lf_InitLocalValue();
	void Lf_InitFontset();
	void Lf_InitColorBrush();
	void Lf_InitDialogDesign();
	void Lf_InitComboModelList(int setSel = 0);

	void Lf_loadModelData();
	void Lf_deleteModelData();
	void Lf_reloadControlData();
	void Lf_saveModelData();

	void Lf_calcAgingTimeMinute();
	void Lf_calcHorResolution();
	void Lf_calcVerResolution();
	void Lf_calcVSync();

	BOOL Lf_checkAgingTempInfoChange();

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
	afx_msg void OnBnClickedBtnMiSave();
	afx_msg void OnBnClickedBtnMiCancel();
	CComboBox m_cmbMiSignalType;
	CComboBox m_cmbMiPixelType;
	CComboBox m_cmbMiOddEven;
	CComboBox m_cmbMiSignalBit;
	CComboBox m_cmbMiBitSwap;
	CComboBox m_cmbMiLvdsRsSel;
	CComboBox m_cmbMiDimmingSel;
	CComboBox m_cmbMiCableOpen;
	CComboBox m_cmbMiPowerOnSeq1;
	CComboBox m_cmbMiPowerOnSeq2;
	CComboBox m_cmbMiPowerOnSeq3;
	CComboBox m_cmbMiPowerOnSeq4;
	CComboBox m_cmbMiPowerOnSeq5;
	CComboBox m_cmbMiPowerOnSeq6;
	CComboBox m_cmbMiPowerOnSeq7;
	CComboBox m_cmbMiPowerOnSeq8;
	CComboBox m_cmbMiPowerOnSeq9;
	CComboBox m_cmbMiPowerOnSeq10;
	CComboBox m_cmbMiPowerOffSeq1;
	CComboBox m_cmbMiPowerOffSeq2;
	CComboBox m_cmbMiPowerOffSeq3;
	CComboBox m_cmbMiPowerOffSeq4;
	CComboBox m_cmbMiPowerOffSeq5;
	CComboBox m_cmbMiPowerOffSeq6;
	CComboBox m_cmbMiPowerOffSeq7;
	CComboBox m_cmbMiPowerOffSeq8;
	CComboBox m_cmbMiPowerOffSeq9;
	CComboBox m_cmbMiPowerOffSeq10;
	CComboBox m_cmbMiLoadModel;
	CMFCButton m_mbtMiModelLoad;
	CMFCButton m_mbtMiModelDelete;
	CComboBox m_cmbMiAgingEndWaitTime;
	CComboBox m_cmbMiTemperatureUse;
	CComboBox m_cmbMiDoorUse;
	CButton m_btnMiSave;
	CButton m_btnMiCancel;
	afx_msg void OnEnChangeEdtMiAgingTimeHh();
	afx_msg void OnEnChangeEdtMiAgingTimeMm();
	afx_msg void OnEnChangeEdtMiHorActive();
	afx_msg void OnEnChangeEdtMiHorWidth();
	afx_msg void OnEnChangeEdtMiHorBp();
	afx_msg void OnEnChangeEdtMiHorFp();
	afx_msg void OnEnChangeEdtMiVerActive();
	afx_msg void OnEnChangeEdtMiVerWidth();
	afx_msg void OnEnChangeEdtMiVerBp();
	afx_msg void OnEnChangeEdtMiVerFp();
	afx_msg void OnBnClickedMbtMiModelLoad();
	afx_msg void OnBnClickedMbtMiModelDelete();
};
