#pragma once
#include "Resource.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ClickEntitiesCmdOp.h"

class DeleteCompDlg : public CDialogEx
{
	DECLARE_DYNAMIC(DeleteCompDlg)

public:
	DeleteCompDlg(CHPSView* in_view, void* pProcess, ClickCompType deleteType, CWnd* pParent = nullptr);
	virtual ~DeleteCompDlg();

	virtual BOOL OnInitDialog();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual void PostNcDestroy() override;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_COMP_DIALOG };
#endif
private:
	CHPSView* view;
	ClickCompType m_eDeleteType;
	UINT m_timerID;

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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	CListBox m_bodyTagListBox;
	afx_msg void OnBnClickedButtonClear();
};
