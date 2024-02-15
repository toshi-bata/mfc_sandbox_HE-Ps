#pragma once
#include "Resource.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "MirrorBodyOp.h"

class MirrorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MirrorDlg)

public:
	MirrorDlg(CHPSView* in_view, void* pProcess, CWnd* pParent = nullptr);
	virtual ~MirrorDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MIRROR_DIALOG };
#endif
private:
	CHPSView* view;
	UINT m_timerID;

#ifdef USING_EXCHANGE_PARASOLID
	ExPsProcess* m_pProcess;
#else
	ExProcess* m_pProcess;
#endif
	MirrorBodyOp* m_pCmdOp = nullptr;
	int m_iActiveCtrl;
	CBrush m_activeBrush;
	CBrush m_inactiveBrush;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	CString m_cTargetBody;
	CString m_cMirrorPlane;

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void PostNcDestroy();
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSetfocusEditTarget();
	afx_msg void OnSetfocusEditMirrorPlane();
	double m_dXL;
	double m_dYL;
	double m_dZL;
	double m_dXV;
	double m_dYV;
	double m_dZV;
	BOOL m_bCopyBody;
	BOOL m_bMergeBodies;
};
