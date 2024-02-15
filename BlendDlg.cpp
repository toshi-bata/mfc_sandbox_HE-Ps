//

#include "stdafx.h"
#include "BlendDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(BlendDlg, CDialogEx)

BlendDlg::BlendDlg(CHPSView* in_view, void* pProcess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BLEND_DIALOG, pParent)
	,view(in_view)
	, m_dBlendR(10)
	, m_dBlendC2(10)
	, m_iBlendType(0)
	, m_bFlip(FALSE)
{
	Create(IDD_BLEND_DIALOG, pParent);

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ParasolidTopoEdge;
#else
	m_pProcess = (ExProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ExchangeTopoEdge;
#endif

	m_pCmdOp = new ClickEntitiesCmdOp(targetComp, view, m_pProcess, false, HPS::MouseButtons::ButtonLeft());

	view->GetCanvas().GetFrontView().GetOperatorControl().Push(m_pCmdOp);

}

BlendDlg::~BlendDlg()
{

	m_pCmdOp->DetachView();
}

BOOL BlendDlg::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();

	UINT timerID = 1;
	UINT interval = 1 * 200;
	m_timerID = SetTimer(timerID, interval, NULL);

	((CStatic*)GetDlgItem(IDC_STATIC_BLEND_R))->SetWindowTextW(_T("Bland R"));
	((CStatic*)GetDlgItem(IDC_STATIC_BLEND_C2))->ShowWindow(false);
	((CEdit*)GetDlgItem(IDC_EDIT_BLEND_C2))->ShowWindow(false);
	((CStatic*)GetDlgItem(IDC_CHECK_CHAMFER_FLIP))->ShowWindow(false);

	return ret;
}

void BlendDlg::OnOK()
{
	auto t0 = std::chrono::system_clock::now();
	
	m_pCmdOp->Unhighlight();

	HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();
	HPS::ComponentArray firstFaceCompArr = m_pCmdOp->GetFirstFaceComponents();

	UpdateData(true);
	int iType = m_iBlendType;

	if (PsBlendType::C == iType)
	{
		if (selCompArr.size() != firstFaceCompArr.size())
			return;
	}

	// Get owner BrepModel
	HPS::Component ownerComp = view->GetOwnerBrepModel(selCompArr[0]);
	A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

#ifdef USING_EXCHANGE_PARASOLID
	// Get selected edges and first faces
	std::vector<PK_EDGE_t>edges;
	std::vector<PK_FACE_t>faces;
	for (int i = 0; i < selCompArr.size(); i++)
	{
		HPS::Component comp = HPS::Component(selCompArr[i]);
		if (ownerComp == view->GetOwnerBrepModel(selCompArr[i]))
			edges.push_back(((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity());

		if (PsBlendType::C == iType)
		{
			if (ownerComp == view->GetOwnerBrepModel(selCompArr[i]))
				faces.push_back(((HPS::Parasolid::Component)firstFaceCompArr[i]).GetParasolidEntity());

		}
	}

	if (m_pProcess->BlendRC((PsBlendType)m_iBlendType, m_dBlendR, m_dBlendC2, edges.size(), edges.data(), faces.data(), pRiBrepModel))
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
	// Get selected edges and first faces
	std::vector<A3DTopoEdge*> pTopoEdges;
	for (int i = 0; i < selCompArr.size(); i++)
	{
		if (ownerComp == view->GetOwnerBrepModel(selCompArr[i]))
			pTopoEdges.push_back(HPS::Exchange::Component(selCompArr[i]).GetExchangeEntity());
	}

	std::vector<A3DTopoFace*> pTopoFaces;
	if (PsBlendType::C == iType)
	{
		for (int i = 0; i < firstFaceCompArr.size(); i++)
		{
			if (ownerComp == view->GetOwnerBrepModel(firstFaceCompArr[i]))
				pTopoFaces.push_back(HPS::Exchange::Component(firstFaceCompArr[i]).GetExchangeEntity());
		}
	}

	if (0 < pTopoEdges.size())
	{
		if (m_pProcess->BlendRC((PsBlendType)iType, m_dBlendR, m_dBlendC2, pRiBrepModel, 
			pTopoEdges.size(), pTopoEdges.data(), pTopoFaces.data()))
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
	swprintf(wcsbuf, sizeof(wcsbuf), L"Process time: %d msec", msec1);
	view->ShowMessage(wcsbuf);

	DestroyWindow();
}

void BlendDlg::OnCancel()
{
	m_pCmdOp->Unhighlight();

	DestroyWindow();
}

void BlendDlg::PostNcDestroy()
{
	delete this;
}

void BlendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_BLEND_R, m_dBlendR);
	DDX_Control(pDX, IDC_LIST_PS_TAG, m_psTagListBox);
	DDX_Radio(pDX, IDC_RADIO_BLEND_TYPE_R, m_iBlendType);
	DDX_Text(pDX, IDC_EDIT_BLEND_C2, m_dBlendC2);
	DDX_Check(pDX, IDC_CHECK_CHAMFER_FLIP, m_bFlip);
}



BEGIN_MESSAGE_MAP(BlendDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &BlendDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_RADIO_BLEND_TYPE_R, &BlendDlg::OnClickedRadioBlendTypeR)
	ON_BN_CLICKED(IDC_RADIO_BLEND_TYPE_C, &BlendDlg::OnBnClickedRadioBlendTypeC)
	ON_BN_CLICKED(IDC_CHECK_CHAMFER_FLIP, &BlendDlg::OnBnClickedCheckChamferFlip)
END_MESSAGE_MAP()




void BlendDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_pCmdOp->m_bUpdated)
	{
		UpdateData(true);

		int nCount = m_psTagListBox.GetCount();
		for (int i = nCount - 1; i > -1; i--)
			m_psTagListBox.DeleteString(i);

		HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

		for (int i = 0; i < selCompArr.size(); i++)
		{
			int iEnt;

#ifdef USING_EXCHANGE_PARASOLID
			iEnt = ((HPS::Parasolid::Component)selCompArr[i]).GetParasolidEntity();
#else
			// Get owner BrepModel
			HPS::Component ownerComp = view->GetOwnerBrepModel(selCompArr[0]);
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

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

		// Highlight first face
		if (PsBlendType::C == m_iBlendType)
		{
			m_pCmdOp->HighlightFaceOfEdge(0);
		}

		CButton* okBtn = (CButton*)GetDlgItem(IDOK);
		if (0 < selCompArr.size())
			okBtn->EnableWindow(TRUE);
		else
			okBtn->EnableWindow(FALSE);

		UpdateData(false);

		m_pCmdOp->m_bUpdated = false;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void BlendDlg::OnClose()
{
	if (m_timerID != 0)
	{
		BOOL err = KillTimer(m_timerID);
		m_timerID = 0;
	}

	CDialogEx::OnClose();
}


void BlendDlg::OnBnClickedButtonClear()
{
	m_pCmdOp->ClearSelection();
	UpdateData(false);
}



void BlendDlg::OnClickedRadioBlendTypeR()
{
	((CStatic*)GetDlgItem(IDC_STATIC_BLEND_R))->SetWindowTextW(_T("Blend R"));
	((CStatic*)GetDlgItem(IDC_STATIC_BLEND_C2))->ShowWindow(false);
	((CEdit*)GetDlgItem(IDC_EDIT_BLEND_C2))->ShowWindow(false);
	((CStatic*)GetDlgItem(IDC_CHECK_CHAMFER_FLIP))->ShowWindow(false);
}


void BlendDlg::OnBnClickedRadioBlendTypeC()
{
	((CStatic*)GetDlgItem(IDC_STATIC_BLEND_R))->SetWindowTextW(_T("C1"));
	((CStatic*)GetDlgItem(IDC_STATIC_BLEND_C2))->ShowWindow(true);
	((CEdit*)GetDlgItem(IDC_EDIT_BLEND_C2))->ShowWindow(true);
	((CStatic*)GetDlgItem(IDC_CHECK_CHAMFER_FLIP))->ShowWindow(true);
}


void BlendDlg::OnBnClickedCheckChamferFlip()
{
	UpdateData(true);

	if (PsBlendType::C == m_iBlendType)
	{
		if (m_bFlip)
			m_pCmdOp->HighlightFaceOfEdge(1);
		else
			m_pCmdOp->HighlightFaceOfEdge(0);
	}
}
