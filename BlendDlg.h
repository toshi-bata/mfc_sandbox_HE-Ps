#pragma once
#include "Resource.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ClickEntitiesCmdOp.h"


class BlendDlg : public CDialogEx
{
	DECLARE_DYNAMIC(BlendDlg)

public:
	BlendDlg(CHPSView* in_view, void* pProcess, CWnd* pParent = nullptr);
	virtual ~BlendDlg();
	
	virtual BOOL OnInitDialog();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual void PostNcDestroy() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BLEND_DIALOG };
#endif
private:
	CHPSView* view;
	UINT m_timerID;
	HPS::SegmentKey m_coordRootSK;

#ifdef USING_EXCHANGE_PARASOLID
	ExPsProcess* m_pProcess;
#else
	ExProcess* m_pProcess;
#endif
	ClickEntitiesCmdOp* m_pCmdOp;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	int m_iBlendType;
	double m_dBlendR;
	double m_dBlendC2;

	void UpdateDlg();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonClear();
	CListBox m_psTagListBox;
	afx_msg void OnClickedRadioBlendTypeR();
	afx_msg void OnBnClickedRadioBlendTypeC();
	afx_msg void OnBnClickedCheckChamferFlip();
	BOOL m_bFlip;
};
