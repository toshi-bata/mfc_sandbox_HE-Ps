//

#include "stdafx.h"
#include "DeleteCompDlg.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(DeleteCompDlg, CDialogEx)

DeleteCompDlg::DeleteCompDlg(CHPSView* in_view, void* pProcess, ClickCompType deleteType, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELETE_COMP_DIALOG, pParent)
	, view(in_view)
	, m_eDeleteType(deleteType)
{
	Create(IDD_DELETE_COMP_DIALOG, pParent);

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ParasolidTopoBody;
#else
	m_pProcess = (ExProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ExchangeRIBRepModel;
#endif

	m_pCmdOp = new ClickEntitiesCmdOp(targetComp, view, m_pProcess, false, HPS::MouseButtons::ButtonLeft());
	m_pCmdOp->SetClickCompType(m_eDeleteType);

	view->GetCanvas().GetFrontView().GetOperatorControl().Push(m_pCmdOp);

}

DeleteCompDlg::~DeleteCompDlg()
{
	m_pCmdOp->DetachView();
}

BOOL DeleteCompDlg::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();

	UINT timerID = 1;
	UINT interval = 1 * 200;
	m_timerID = SetTimer(timerID, interval, NULL);

	if (ClickCompType::CLICK_PART == m_eDeleteType)
	{
		SetWindowText(_T("Delete Part"));
		((CStatic*)GetDlgItem(IDC_STATIC_COMP))->SetWindowTextW(_T("Part Name"));
	}

	return ret;
}

void DeleteCompDlg::OnOK()
{
	auto t0 = std::chrono::system_clock::now();

	m_pCmdOp->Unhighlight();

	HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

	for (int i = 0; i < selCompArr.size(); i++)
	{
		HPS::Component::DeleteMode deleteMode = HPS::Component::DeleteMode::Standard;
		if (ClickCompType::CLICK_BODY == m_eDeleteType)
		{
#ifdef USING_EXCHANGE_PARASOLID
			// Get owner BrepModel
			HPS::Exchange::Component ownerComp = view->GetOwnerBrepModel(selCompArr[i]);
			A3DRiBrepModel* pRiBrepModel = ownerComp.GetExchangeEntity();
			m_pProcess->RegisterDeleteBody(pRiBrepModel);

			int body = ((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity();
			view->InitPsBodyMap(body, selCompArr[i]);
#else
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity();
			m_pProcess->DeleteBody(pRiBrepModel);

			deleteMode = HPS::Component::DeleteMode::StandardAndExchange;
#endif
		}
		else if (ClickCompType::CLICK_PART == m_eDeleteType)
		{
#ifdef USING_EXCHANGE_PARASOLID
			A3DAsmProductOccurrence* pPO = ((HPS::Exchange::Component)selCompArr[i]).GetExchangeEntity();
			m_pProcess->RegisterDeletePart(pPO);
#else
			A3DAsmProductOccurrence* pPO = HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity();

			deleteMode = HPS::Component::DeleteMode::StandardAndExchange;
#endif
		}
		// Delete component
		selCompArr[i].Delete(deleteMode);
	}
	// Show process time
	auto t1 = std::chrono::system_clock::now();

	auto dur1 = t1 - t0;
	auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

	wchar_t wcsbuf[256];
	swprintf(wcsbuf, sizeof(wcsbuf), L"Process time: %d msec", msec1);
	view->ShowMessage(wcsbuf);

	view->GetCanvas().Update(HPS::Window::UpdateType::Complete);
	
	DestroyWindow();
}

void DeleteCompDlg::OnCancel()
{
	m_pCmdOp->Unhighlight();

	DestroyWindow();
}

void DeleteCompDlg::PostNcDestroy()
{
	delete this;
}

void DeleteCompDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BODY_TAG, m_bodyTagListBox);
}


BEGIN_MESSAGE_MAP(DeleteCompDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &DeleteCompDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()




void DeleteCompDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_pCmdOp->m_bUpdated)
	{
		int nCount = m_bodyTagListBox.GetCount();
		for (int i = nCount - 1; i > -1; i--)
			m_bodyTagListBox.DeleteString(i);

		HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

		for (int i = 0; i < selCompArr.size(); i++)
		{
			if (ClickCompType::CLICK_BODY == m_eDeleteType)
			{
				int iEnt = 0;

#ifdef USING_EXCHANGE_PARASOLID
				iEnt = ((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity();
#else
				A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity();
				iEnt = m_pProcess->GetEntityTag(pRiBrepModel, NULL, false);
#endif
				CString sEnt;
				if (0 < iEnt)
					sEnt.Format(_T("%d"), iEnt);
				else
					sEnt = "UNKNOWN";
				m_bodyTagListBox.AddString(sEnt);
			}
			else if (ClickCompType::CLICK_PART == m_eDeleteType)
			{
				HPS::UTF8 name = selCompArr[i].GetName();
				wchar_t wName[256];
				name.ToWStr(wName);
				m_bodyTagListBox.AddString(wName);
			}
		}

		CButton* okBtn = (CButton*)GetDlgItem(IDOK);
		if (0 < selCompArr.size())
			okBtn->EnableWindow(TRUE);
		else
			okBtn->EnableWindow(FALSE);

		m_pCmdOp->m_bUpdated = false;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void DeleteCompDlg::OnClose()
{
	if (m_timerID != 0)
	{
		BOOL err = KillTimer(m_timerID);
		m_timerID = 0;
	}

	CDialogEx::OnClose();
}


void DeleteCompDlg::OnBnClickedButtonClear()
{
	m_pCmdOp->ClearSelection();

	int nCount = m_bodyTagListBox.GetCount();
	for (int i = nCount - 1; i > -1; i--)
		m_bodyTagListBox.DeleteString(i);

	UpdateData(false);
}
