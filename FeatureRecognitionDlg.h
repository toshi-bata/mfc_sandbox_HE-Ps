#pragma once
#include "Resource.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ClickEntitiesCmdOp.h"


class FeatureRecognitionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(FeatureRecognitionDlg)

public:
	FeatureRecognitionDlg(CHPSView* in_view, void* pProcess, CWnd* pParent = nullptr);
	virtual ~FeatureRecognitionDlg();

	virtual void OnCancel() override;
	virtual void PostNcDestroy() override;
	virtual BOOL OnInitDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FR_DIALOG };
#endif
private:
	CHPSView* view;
	HPS::HighlightOptionsKit m_highlight_options;
	UINT m_timerID;
	HPS::Component m_selComp;

#ifdef USING_EXCHANGE_PARASOLID
	ExPsProcess* m_pProcess;
#else
	ExProcess* m_pProcess;
#endif
	ClickEntitiesCmdOp* m_pCmdOp;

	void clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	int m_iFRType;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_iSelFace;
	afx_msg void OnClose();
	CListBox m_detectedFaceListBox;
	afx_msg void OnBnClickedRadioFrBoss();
	afx_msg void OnBnClickedRadioFrConcentric();
	afx_msg void OnBnClickedRadioFrCoplanar();
};
