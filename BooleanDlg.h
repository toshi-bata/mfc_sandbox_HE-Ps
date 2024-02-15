#pragma once
#include "Resource.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "BooleanOp.h"


class BooleanDlg : public CDialogEx
{
	DECLARE_DYNAMIC(BooleanDlg)

public:
	BooleanDlg(CHPSView* in_view, void* pProcess, CWnd* pParent = nullptr);
	virtual ~BooleanDlg();

	virtual BOOL OnInitDialog();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual void PostNcDestroy() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BOOLEAN_DIALOG };
#endif
private:
	CHPSView* view;
	UINT m_timerID;

#ifdef USING_EXCHANGE_PARASOLID
	ExPsProcess* m_pProcess;
#else
	ExProcess* m_pProcess;
#endif
	BooleanOp* m_pCmdOp;
	int m_iActiveCtrl;
	CBrush m_activeBrush;
	CBrush m_inactiveBrush;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    

	DECLARE_MESSAGE_MAP()
public:
	int m_iTBoolType;
	CString m_cTargetBody;
	CListBox m_toolBodyListBox;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLbnSetfocusListBoolTool();
	afx_msg void OnEnSetfocusEditBoolTarget();
};
