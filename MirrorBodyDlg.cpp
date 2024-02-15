#include "stdafx.h"
#include "MirrorBodyDlg.h"
#include "afxdialogex.h"

#define COLOR_ACTIVE RGB(255, 128, 128)
#define COLOR_INACTIVE RGB(255, 255, 255)

IMPLEMENT_DYNAMIC(MirrorDlg, CDialogEx)

MirrorDlg::MirrorDlg(CHPSView* in_view, void* pProcess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MIRROR_DIALOG, pParent)
	, view(in_view)
	, m_cTargetBody(_T(""))
	, m_cMirrorPlane(_T(""))
	, m_iActiveCtrl(0)
	, m_dXL(0)
	, m_dYL(0)
	, m_dZL(0)
	, m_dXV(1.0)
	, m_dYV(0)
	, m_dZV(0)
	, m_bCopyBody(FALSE)
	, m_bMergeBodies(FALSE)
{
	Create(IDD_MIRROR_DIALOG, pParent);

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ParasolidTopoBody;
#else
	m_pProcess = (ExProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ExchangeRIBRepModel;
#endif

	m_pCmdOp = new MirrorBodyOp(view, m_pProcess, HPS::MouseButtons::ButtonLeft());

	view->GetCanvas().GetFrontView().GetOperatorControl().Push(m_pCmdOp);

}

MirrorDlg::~MirrorDlg()
{
	m_pCmdOp->DetachView();
}

BOOL MirrorDlg::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();

	UINT timerID = 1;
	UINT interval = 1 * 200;
	m_timerID = SetTimer(timerID, interval, NULL);

	m_activeBrush.CreateSolidBrush(COLOR_ACTIVE);
	m_inactiveBrush.CreateSolidBrush(COLOR_INACTIVE);

	CButton* okBtn = (CButton*)GetDlgItem(IDOK);
	okBtn->EnableWindow(FALSE);

	return ret;
}

void MirrorDlg::OnOK()
{
	auto t0 = std::chrono::system_clock::now();
	
	m_pCmdOp->Unhighlight();

	UpdateData(true);

	HPS::Component targetComp = m_pCmdOp->GetTargetComponent();

	double location[3] = { m_dXL, m_dYL, m_dZL };
	double normal[3] = { m_dXV, m_dYV, m_dZV };

	if (HPS::Type::None != targetComp.Type())
	{
#ifdef USING_EXCHANGE_PARASOLID
		PK_BODY_t targetBody = ((HPS::Parasolid::Component)targetComp).GetParasolidEntity();
		PK_BODY_t mirror_body = PK_ENTITY_null;
		if (m_pProcess->MirrorBody(targetBody, location, normal, m_bCopyBody, m_bMergeBodies, mirror_body))
		{
			if (m_bCopyBody && !m_bMergeBodies)
				view->AddBody(mirror_body);
			else
			{
				// Tessellate
				HPS::Parasolid::FacetTessellationKit ftk = HPS::Parasolid::FacetTessellationKit::GetDefault();
				HPS::Parasolid::LineTessellationKit ltk = HPS::Parasolid::LineTessellationKit::GetDefault();
				HPS::Parasolid::Component(targetComp).Tessellate(ftk, ltk);
			}
		}
#else
		A3DRiBrepModel* pTargetBrep = ((HPS::Exchange::Component)targetComp).GetExchangeEntity();

		if (!m_pProcess->MirrorBody(pTargetBrep, location, normal, m_bCopyBody, m_bMergeBodies))
			return;

		// Reload target component
		HPS::Exchange::ReloadNotifier notifier = ((HPS::Exchange::Component)targetComp).Reload();
		notifier.Wait();

#endif
		view->GetCanvas().Update();

		// Show process time
		auto t1 = std::chrono::system_clock::now();

		auto dur1 = t1 - t0;
		auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

		wchar_t wcsbuf[512];
		swprintf(wcsbuf, sizeof(wcsbuf) / sizeof(wchar_t), L"Process time: %d msec", msec1);
		view->ShowMessage(wcsbuf);

	}

	DestroyWindow();
}

void MirrorDlg::OnCancel()
{
	m_pCmdOp->Unhighlight();

	DestroyWindow();
}

void MirrorDlg::PostNcDestroy()
{
	delete this;

	CDialogEx::PostNcDestroy();
}

void MirrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TARGET, m_cTargetBody);
	DDX_Text(pDX, IDC_EDIT_MIRROR_PLANE, m_cMirrorPlane);
	DDX_Text(pDX, IDC_EDIT_MIRROR_XL, m_dXL);
	DDX_Text(pDX, IDC_EDIT_MIRROR_YL, m_dYL);
	DDX_Text(pDX, IDC_EDIT_MIRROR_ZL, m_dZL);
	DDX_Text(pDX, IDC_EDIT_MIRROR_XV, m_dXV);
	DDX_Text(pDX, IDC_EDIT_MIRROR_YV, m_dYV);
	DDX_Text(pDX, IDC_EDIT_MIRROR_ZV, m_dZV);
	DDX_Check(pDX, IDC_CHECK_MIRROR_COPY, m_bCopyBody);
	DDX_Check(pDX, IDC_CHECK_MIRROR_MERGE, m_bMergeBodies);
}

BEGIN_MESSAGE_MAP(MirrorDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_EN_SETFOCUS(IDC_EDIT_TARGET, &MirrorDlg::OnSetfocusEditTarget)
	ON_EN_SETFOCUS(IDC_EDIT_MIRROR_PLANE, &MirrorDlg::OnSetfocusEditMirrorPlane)
END_MESSAGE_MAP()

void MirrorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_pCmdOp->m_bUpdated)
	{
		HPS::Component targetComp = m_pCmdOp->GetTargetComponent();

		if (HPS::Type::None != targetComp.Type())
		{
			int iBody = 0;

#ifdef USING_EXCHANGE_PARASOLID
			iBody = ((HPS::Parasolid::Component)targetComp).GetParasolidEntity();
#else
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(targetComp).GetExchangeEntity();
			iBody = m_pProcess->GetEntityTag(pRiBrepModel, NULL, false);
#endif
			CString sBody("UNKNOWN");

			if (0 < iBody)
				sBody.Format(_T("%d"), iBody);

			if (m_cTargetBody != sBody)
			{
				m_cTargetBody = sBody;

				m_iActiveCtrl = 1;
				m_pCmdOp->SetSelectionStep(1);
				Invalidate();
			}
		}
		else
		{
			m_cTargetBody = "";
		}

		HPS::Component toolComp = m_pCmdOp->GetToolComponent();
		if (HPS::Type::None != toolComp.Type())
		{
			int iFace = 0;
			double location[3], normal[3];

#ifdef USING_EXCHANGE_PARASOLID
			iFace = ((HPS::Parasolid::Component)toolComp).GetParasolidEntity();
			if (m_pProcess->GetPlaneInfo(iFace, location, normal))
#else
			// Get owner BrepModel
			HPS::Component ownerComp = view->GetOwnerBrepModel(toolComp);
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

			A3DTopoFace* pTopoFace = HPS::Exchange::Component(toolComp).GetExchangeEntity();
			iFace = m_pProcess->GetEntityTag(pRiBrepModel, pTopoFace, false);
			if (m_pProcess->GetPlaneInfo(pRiBrepModel, pTopoFace, location, normal))
#endif
			{
				CString sFace("UNKNOWN");

				if (0 < iFace)
					sFace.Format(_T("%d"), iFace);

				if (m_cMirrorPlane != sFace)
				{
					m_cMirrorPlane = sFace;

					m_dXL = location[0];
					m_dYL = location[1];
					m_dZL = location[2];

					m_dXV = normal[0];
					m_dYV = normal[1];
					m_dZV = normal[2];

					m_iActiveCtrl = 0;
					m_pCmdOp->SetSelectionStep(0);
					Invalidate();
				}
			}
		}
		else
		{
			m_cMirrorPlane = "";
		}

		CButton* okBtn = (CButton*)GetDlgItem(IDOK);
		if (HPS::Type::None != targetComp.Type())
			okBtn->EnableWindow(TRUE);
		else
			okBtn->EnableWindow(FALSE);

		UpdateData(false);

		m_pCmdOp->m_bUpdated = false;
	}
	CDialogEx::OnTimer(nIDEvent);
}



HBRUSH MirrorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	int id = pWnd->GetDlgCtrlID();

	if (nCtlColor == CTLCOLOR_EDIT)
	{
		switch (id)
		{
		case IDC_EDIT_TARGET:
			if (0 == m_iActiveCtrl)
			{
				pDC->SetBkColor(COLOR_ACTIVE);
				return m_activeBrush;
			}
			else
			{
				pDC->SetBkColor(COLOR_INACTIVE);
				return m_inactiveBrush;
			}
			break;
		case IDC_EDIT_MIRROR_PLANE:
			if (1 == m_iActiveCtrl)
			{
				pDC->SetBkColor(COLOR_ACTIVE);
				return m_activeBrush;
			}
			else
			{
				pDC->SetBkColor(COLOR_INACTIVE);
				return m_inactiveBrush;
			}
			break;
		default:
			break;
		}

	}

	return hbr;
}


void MirrorDlg::OnSetfocusEditTarget()
{
	m_iActiveCtrl = 0;
	if (nullptr != m_pCmdOp)
	{
		m_pCmdOp->SetSelectionStep(0);
		Invalidate();
	}
}


void MirrorDlg::OnSetfocusEditMirrorPlane()
{
	m_iActiveCtrl = 1;
	if (nullptr != m_pCmdOp)
	{
		m_pCmdOp->SetSelectionStep(1);
		Invalidate();
	}
}
