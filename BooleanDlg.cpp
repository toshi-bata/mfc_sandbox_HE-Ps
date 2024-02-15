//
//

#include "stdafx.h"
#include "BooleanDlg.h"
#include "afxdialogex.h"

#define COLOR_ACTIVE RGB(255, 128, 128)
#define COLOR_INACTIVE RGB(255, 255, 255)

IMPLEMENT_DYNAMIC(BooleanDlg, CDialogEx)

BooleanDlg::BooleanDlg(CHPSView* in_view, void* pProcess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BOOLEAN_DIALOG, pParent)
	, view(in_view)
	, m_iTBoolType(0)
	, m_cTargetBody(_T(""))
	, m_iActiveCtrl(0)
{
	Create(IDD_BOOLEAN_DIALOG, pParent);

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ParasolidTopoBody;
#else
	m_pProcess = (ExProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ExchangeRIBRepModel;
#endif

	m_pCmdOp = new BooleanOp(view, m_pProcess, HPS::MouseButtons::ButtonLeft());

	view->GetCanvas().GetFrontView().GetOperatorControl().Push(m_pCmdOp);

}

BooleanDlg::~BooleanDlg()
{
	m_pCmdOp->DetachView();
}

BOOL BooleanDlg::OnInitDialog()
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

void BooleanDlg::OnOK()
{
	auto t0 = std::chrono::system_clock::now();

	m_pCmdOp->Unhighlight();

	UpdateData(true);

	HPS::Component targetComp = m_pCmdOp->GetTargetComponent();
	HPS::ComponentArray toolCompArr = m_pCmdOp->GetToolComponents();

	if (HPS::Type::None != targetComp.Type() && 0 < toolCompArr.size())
	{
		HPS::Component::DeleteMode deleteMode = HPS::Component::DeleteMode::Standard;

#ifdef USING_EXCHANGE_PARASOLID
		PK_BODY_t targetBody = ((HPS::Parasolid::Component)targetComp).GetParasolidEntity();
		std::vector<PK_BODY_t> toolBodyArr;
		for (int i = 0; i < toolCompArr.size(); i++)
		{
			toolBodyArr.push_back(((HPS::Parasolid::Component)toolCompArr[i]).GetParasolidEntity());
		}

		int resBodyCnt = 0;
		PK_BODY_t* resBodies;
		if (!m_pProcess->Boolean((PsBoolType)m_iTBoolType, targetBody, toolBodyArr.size(), toolBodyArr.data(), resBodyCnt, resBodies))
			return;

		for (int i = 0; i < resBodyCnt; i++)
		{
			PK_BODY_t resBody = resBodies[i];
			if (resBody == targetBody)
			{
				// Tessellate
				((HPS::Parasolid::Component)targetComp).Tessellate(
					HPS::Parasolid::FacetTessellationKit::GetDefault(),
					HPS::Parasolid::LineTessellationKit::GetDefault());

				// Register target body as updated model
				HPS::Exchange::Component ownerComp = view->GetOwnerBrepModel(targetComp);
				A3DRiBrepModel* pRiBrepModel = ownerComp.GetExchangeEntity();
				m_pProcess->RegisterUpdatedBody(pRiBrepModel, targetBody);

				// Update Ps component map
				view->InitPsBodyMap((int)targetBody, targetComp);
			}
			else
			{
				// Add new body
				view->AddBody(resBody);
			}
		}

		for (int i = 0; i < toolBodyArr.size(); i++)
		{
			// Delete toolbody from map
			view->DeletePsBodyMap(toolBodyArr[i]);

			// Register tool body as deleted model
			HPS::Exchange::Component ownerComp = view->GetOwnerBrepModel(toolCompArr[i]);
			A3DRiBrepModel* pRiBrepModel = ownerComp.GetExchangeEntity();
			m_pProcess->RegisterDeleteBody(pRiBrepModel);
		}

#else
		A3DRiBrepModel* pTargetBrep = ((HPS::Exchange::Component)targetComp).GetExchangeEntity();
		std::vector< A3DRiBrepModel*> pToolBrepArr;
		for (int i = 0; i < toolCompArr.size(); i++)
			pToolBrepArr.push_back(((HPS::Exchange::Component)toolCompArr[i]).GetExchangeEntity());

		if (!m_pProcess->Boolean((PsBoolType)m_iTBoolType, pTargetBrep, pToolBrepArr.size(), pToolBrepArr.data()))
			return;

		// Reload target component
		HPS::Exchange::ReloadNotifier notifier = ((HPS::Exchange::Component)targetComp).Reload();
		notifier.Wait();

		deleteMode = HPS::Component::DeleteMode::StandardAndExchange;
#endif
		// Delete tool comonent
		for (int i = 0; i < toolCompArr.size(); i++)
			toolCompArr[i].Delete(deleteMode);

		view->GetCanvas().Update();

		// Show process time
		auto t1 = std::chrono::system_clock::now();

		auto dur1 = t1 - t0;
		auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

		wchar_t wcsbuf[256];
		swprintf(wcsbuf, sizeof(wcsbuf), L"Process time: %d msec", msec1);
		view->ShowMessage(wcsbuf);
	}
	DestroyWindow();
}

void BooleanDlg::OnCancel()
{
	m_pCmdOp->Unhighlight();

	DestroyWindow();
}

void BooleanDlg::PostNcDestroy()
{
	delete this;
}

void BooleanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_UNITE, m_iTBoolType);
	DDX_Control(pDX, IDC_LIST_BOOL_TOOL, m_toolBodyListBox);
	DDX_Text(pDX, IDC_EDIT_BOOL_TARGET, m_cTargetBody);
}


BEGIN_MESSAGE_MAP(BooleanDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_LBN_SETFOCUS(IDC_LIST_BOOL_TOOL, &BooleanDlg::OnLbnSetfocusListBoolTool)
	ON_EN_SETFOCUS(IDC_EDIT_BOOL_TARGET, &BooleanDlg::OnEnSetfocusEditBoolTarget)
END_MESSAGE_MAP()




void BooleanDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_pCmdOp->m_bUpdated)
	{
		UpdateData(true);

		HPS::Component targetComp = m_pCmdOp->GetTargetComponent();
		HPS::ComponentArray toolCompArr = m_pCmdOp->GetToolComponents();

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

		int nCount = m_toolBodyListBox.GetCount();
		for (int i = nCount - 1; i > -1; i--)
			m_toolBodyListBox.DeleteString(i);

		for (int i = 0; i < toolCompArr.size(); i++)
		{
			int iBody = 0;
#ifdef USING_EXCHANGE_PARASOLID
			iBody = ((HPS::Parasolid::Component)toolCompArr[i]).GetParasolidEntity();
#else
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(toolCompArr[i]).GetExchangeEntity();
			iBody = m_pProcess->GetEntityTag(pRiBrepModel, NULL, false);
#endif
			CString sBody("UNKNOWN");

			if (0 < iBody)
				sBody.Format(_T("%d"), iBody);

			m_toolBodyListBox.AddString(sBody);
		}

		CButton* okBtn = (CButton*)GetDlgItem(IDOK);
		if (HPS::Type::None != targetComp.Type() && 0 < toolCompArr.size())
			okBtn->EnableWindow(TRUE);
		else
			okBtn->EnableWindow(FALSE);

		UpdateData(false);
		
		m_pCmdOp->m_bUpdated = false;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void BooleanDlg::OnClose()
{
	if (m_timerID != 0)
	{
		BOOL err = KillTimer(m_timerID);
		m_timerID = 0;
	}

	CDialogEx::OnClose();
}


HBRUSH BooleanDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	int id = pWnd->GetDlgCtrlID();

	if (nCtlColor == CTLCOLOR_EDIT)
	{
		switch(id)
		{
		case IDC_EDIT_BOOL_TARGET:
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
		default:
			break;
		}
	}
	else if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		switch (id)
		{
		case IDC_LIST_BOOL_TOOL:
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


void BooleanDlg::OnLbnSetfocusListBoolTool()
{
	m_iActiveCtrl = 1;
	m_pCmdOp->SetSelectionStep(1);
	Invalidate();
}


void BooleanDlg::OnEnSetfocusEditBoolTarget()
{
	m_iActiveCtrl = 0;
	m_pCmdOp->SetSelectionStep(0);
	Invalidate();
}
