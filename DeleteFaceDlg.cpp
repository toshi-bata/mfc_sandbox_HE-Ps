//

#include "stdafx.h"
#include "DeleteFaceDlg.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(DeleteFaceDlg, CDialogEx)

DeleteFaceDlg::DeleteFaceDlg(CHPSView* in_view, void* pProcess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELETE_FACE_DIALOG, pParent)
	, view(in_view)
{
	Create(IDD_DELETE_FACE_DIALOG, pParent);

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ParasolidTopoFace;
#else
	m_pProcess = (ExProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ExchangeTopoFace;
#endif

	m_pCmdOp = new ClickEntitiesCmdOp(targetComp, view, m_pProcess, false, HPS::MouseButtons::ButtonLeft());

	view->GetCanvas().GetFrontView().GetOperatorControl().Push(m_pCmdOp);

}

DeleteFaceDlg::~DeleteFaceDlg()
{
	m_pCmdOp->DetachView();
}

BOOL DeleteFaceDlg::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();

	UINT timerID = 1;
	UINT interval = 1 * 200;
	m_timerID = SetTimer(timerID, interval, NULL);

	return ret;
}

void DeleteFaceDlg::OnOK()
{
	auto t0 = std::chrono::system_clock::now();

	m_pCmdOp->Unhighlight();

	HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

	// Get owner BrepModel
	HPS::Component ownerComp = view->GetOwnerBrepModel(selCompArr[0]);
	A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

#ifdef USING_EXCHANGE_PARASOLID
	// Get selected facs
	std::vector<PK_FACE_t>faces;
	for (int i = 0; i < selCompArr.size(); i++)
	{
		if (ownerComp == view->GetOwnerBrepModel(selCompArr[i]))
			faces.push_back(((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity());
	}

	if (m_pProcess->DeleteFaces(faces.size(), faces.data(), pRiBrepModel))
	{
		// PsComponent mapper
		HPS::Component psBodyComp = view->GetOwnerPSBodyCompo(selCompArr[0]);
		int body = ((HPS::Parasolid::Component)psBodyComp).GetParasolidEntity();
		view->InitPsBodyMap(body, psBodyComp);

		// Tessellate
		HPS::Parasolid::FacetTessellationKit ftk = HPS::Parasolid::FacetTessellationKit::GetDefault();
		HPS::Parasolid::LineTessellationKit ltk = HPS::Parasolid::LineTessellationKit::GetDefault();
		HPS::Parasolid::Component(psBodyComp).Tessellate(ftk, ltk);
	}
#else
	// Get selected faces
	std::vector<A3DTopoFace*> pTopoFaces;
	for (int i = 0; i < selCompArr.size(); i++)
	{
		if (ownerComp == view->GetOwnerBrepModel(selCompArr[i]))
			pTopoFaces.push_back(HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity());
	}

	if (0 < pTopoFaces.size())
	{
		if (m_pProcess->DeleteFaces(pRiBrepModel, pTopoFaces.size(), pTopoFaces.data()))
		{
			// Reload
			HPS::Exchange::ReloadNotifier notifier = HPS::Exchange::Component(ownerComp).Reload();
			notifier.Wait();
		}
	}

#endif
	// Show process time
	auto t1 = std::chrono::system_clock::now();

	auto dur1 = t1 - t0;
	auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

	wchar_t wcsbuf[256];
	swprintf(wcsbuf, sizeof(wcsbuf), L"Process time: %d msec", msec1);
	view->ShowMessage(wcsbuf);

	view->GetCanvas().Update();
	
	DestroyWindow();
}

void DeleteFaceDlg::OnCancel()
{
	m_pCmdOp->Unhighlight();

	DestroyWindow();
}

void DeleteFaceDlg::PostNcDestroy()
{
	delete this;
}

void DeleteFaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PS_TAG, m_psTagListBox);
}


BEGIN_MESSAGE_MAP(DeleteFaceDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &DeleteFaceDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()




void DeleteFaceDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_pCmdOp->m_bUpdated)
	{
		int nCount = m_psTagListBox.GetCount();
		for (int i = nCount - 1; i > -1; i--)
			m_psTagListBox.DeleteString(i);

		HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

		for (int i = 0; i < selCompArr.size(); i++)
		{
			int iEnt = 0;

#ifdef USING_EXCHANGE_PARASOLID
			iEnt = ((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity();
#else
			// Get owner BrepModel
			HPS::Component ownerComp = view->GetOwnerBrepModel(selCompArr[0]);
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

			HPS::Component comp = selCompArr[i];
			A3DEntity* pEntity = HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity();
			iEnt = m_pProcess->GetEntityTag(pRiBrepModel, pEntity, false);
#endif
			CString sEnt;
			if (0 < iEnt)
				sEnt.Format(_T("%d"), iEnt);
			else
				sEnt = "UNKNOWN";
			m_psTagListBox.AddString(sEnt);
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


void DeleteFaceDlg::OnClose()
{
	if (m_timerID != 0)
	{
		BOOL err = KillTimer(m_timerID);
		m_timerID = 0;
	}

	CDialogEx::OnClose();
}


void DeleteFaceDlg::OnBnClickedButtonClear()
{
	m_pCmdOp->ClearSelection();
	UpdateData(false);
}
