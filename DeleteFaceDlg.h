#pragma once
#include "Resource.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ClickEntitiesCmdOp.h"

class DeleteFaceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(DeleteFaceDlg)

public:
	DeleteFaceDlg(CHPSView* in_view, void* pProcess, CWnd* pParent = nullptr);
	virtual ~DeleteFaceDlg();

	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual void PostNcDestroy() override;
	virtual BOOL OnInitDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DELETE_FACE_DIALOG };
#endif
private:
	CHPSView* view;
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
	afx_msg void OnBnClickedButtonClear();
	CListBox m_psTagListBox;
};
