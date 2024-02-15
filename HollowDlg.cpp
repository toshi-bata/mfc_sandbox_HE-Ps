// HollowDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "HollowDlg.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(HollowDlg, CDialogEx)

HollowDlg::HollowDlg(CHPSView* in_view, void* pProcess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOLLOW_DIALOG, pParent)
	, view(in_view)
	, m_dEditTick(1)
	, m_bHollowOutside(FALSE)
{
	Create(IDD_HOLLOW_DIALOG, pParent);

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

HollowDlg::~HollowDlg()
{
	m_pCmdOp->DetachView();
}

BOOL HollowDlg::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();

	UINT timerID = 1;
	UINT interval = 1 * 200;
	m_timerID = SetTimer(timerID, interval, NULL);

	return ret;
}

void HollowDlg::OnOK()
{
	auto t0 = std::chrono::system_clock::now();

	m_pCmdOp->Unhighlight();

	UpdateData(true);

	double dThick = -m_dEditTick;
	if (m_bHollowOutside)
		dThick = m_dEditTick;

	HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

	// Get owner BrepModel
	HPS::Component ownerComp = view->GetOwnerBrepModel(selCompArr[0]);
	A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

#ifdef USING_EXCHANGE_PARASOLID
	// Get body
	HPS::Parasolid::Component bodyCompo = view->GetOwnerPSBodyCompo(selCompArr[0]);
	PK_BODY_t body = bodyCompo.GetParasolidEntity();

	// Get selected facs
	std::vector<PK_FACE_t>faces;
	for (int i = 0; i < selCompArr.size(); i++)
	{
		if (ownerComp == view->GetOwnerBrepModel(selCompArr[i]))
			faces.push_back(((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity());
	}

	if (m_pProcess->Hollow(dThick, body, faces.size(), faces.data(), pRiBrepModel))
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
		if (m_pProcess->Hollow(dThick, pRiBrepModel, pTopoFaces.size(), pTopoFaces.data()))
		{
			// Reload
			HPS::Exchange::ReloadNotifier notifier = HPS::Exchange::Component(ownerComp).Reload();
			notifier.Wait();
		}
	}
#endif
	view->GetCanvas().Update();

	// Show process time
	auto t1 = std::chrono::system_clock::now();

	auto dur1 = t1 - t0;
	auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

	wchar_t wcsbuf[256];
	swprintf(wcsbuf, sizeof(wcsbuf), L"Process time: %d msec", (int)msec1);
	view->ShowMessage(wcsbuf);

	DestroyWindow();
}

void HollowDlg::OnCancel()
{
	m_pCmdOp->Unhighlight();

	DestroyWindow();
}

void HollowDlg::PostNcDestroy()
{
	delete this;
}

void HollowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_T, m_dEditTick);
	DDX_Control(pDX, IDC_LIST_PS_TAG, m_psTagListBox);
	DDX_Check(pDX, IDC_CHECK_HOLLOW_OUTSIDE, m_bHollowOutside);
}


BEGIN_MESSAGE_MAP(HollowDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &HollowDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()


void HollowDlg::OnTimer(UINT_PTR nIDEvent)
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
			A3DTopoEdge* pTopoEdge = HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity();
			iEnt = m_pProcess->GetEntityTag(pRiBrepModel, pTopoEdge, false);
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


void HollowDlg::OnClose()
{
	if (m_timerID != 0)
	{
		BOOL err = KillTimer(m_timerID);
		m_timerID = 0;
	}

	CDialogEx::OnClose();
}


void HollowDlg::OnBnClickedButtonClear()
{
	m_pCmdOp->ClearSelection();
	UpdateData(false);
}
