// FeatureRecognitionDlg.cpp
//

#include "stdafx.h"
#include "FeatureRecognitionDlg.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(FeatureRecognitionDlg, CDialogEx)

FeatureRecognitionDlg::FeatureRecognitionDlg(CHPSView* in_view, void* pProcess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FR_DIALOG, pParent)
	, view(in_view)
	, m_iFRType(0)
	, m_iSelFace(0)
	, m_selComp(HPS::Component())
{
	Create(IDD_FR_DIALOG, pParent);

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ParasolidTopoFace;
#else
	m_pProcess = (ExProcess*)pProcess;
	HPS::Component::ComponentType targetComp = HPS::Component::ComponentType::ExchangeTopoFace;
#endif

	m_pCmdOp = new ClickEntitiesCmdOp(targetComp, view, m_pProcess, true, HPS::MouseButtons::ButtonLeft());

	view->GetCanvas().GetFrontView().GetOperatorControl().Push(m_pCmdOp);

	//! [build_highlight_style]
	HPS::PortfolioKey myPortfolio = HPS::Database::CreatePortfolio();
	view->GetCanvas().GetFrontView().GetSegmentKey().GetPortfolioControl().Push(myPortfolio);

	HPS::NamedStyleDefinition myHighlightStyle = myPortfolio.DefineNamedStyle("highlight_style_2", HPS::Database::CreateRootSegment());
	myHighlightStyle.GetSource().GetMaterialMappingControl().SetFaceColor(HPS::RGBAColor(1.0f, 0.0f, 1.0f))
		.SetLineColor(HPS::RGBAColor(1.0f, 0.0f, 1.0f));

	m_highlight_options.SetStyleName("highlight_style_2");
	m_highlight_options.SetOverlay(HPS::Drawing::Overlay::InPlace);
	//! [build_highlight_style]

}

FeatureRecognitionDlg::~FeatureRecognitionDlg()
{
	m_pCmdOp->DetachView();
}

BOOL FeatureRecognitionDlg::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();

	UINT timerID = 1;
	UINT interval = 1 * 200;
	m_timerID = SetTimer(timerID, interval, NULL);

	return ret;
}

void FeatureRecognitionDlg::OnCancel()
{
	clear();

	DestroyWindow();
}

void FeatureRecognitionDlg::PostNcDestroy()
{
	delete this;
}

void FeatureRecognitionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_FR_BOSS, m_iFRType);
	DDX_Text(pDX, IDC_EDIT_FR_SEL, m_iSelFace);
	DDX_Control(pDX, IDC_LIST_FR_DETECTED, m_detectedFaceListBox);
}


BEGIN_MESSAGE_MAP(FeatureRecognitionDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO_FR_BOSS, &FeatureRecognitionDlg::OnBnClickedRadioFrBoss)
	ON_BN_CLICKED(IDC_RADIO_FR_CONCENTRIC, &FeatureRecognitionDlg::OnBnClickedRadioFrConcentric)
	ON_BN_CLICKED(IDC_RADIO_FR_COPLANAR, &FeatureRecognitionDlg::OnBnClickedRadioFrCoplanar)
END_MESSAGE_MAP()


void FeatureRecognitionDlg::OnTimer(UINT_PTR nIDEvent)
{
	HPS::ComponentArray selCompArr = m_pCmdOp->GetSelectedComponents();

	if (0 < selCompArr.size())
	{
		if (!selCompArr[0].operator==(m_selComp))
		{
			// New selection
			auto t0 = std::chrono::system_clock::now();

			m_selComp = selCompArr[0];

			// Clear list box
			int nCount = m_detectedFaceListBox.GetCount();
			for (int i = nCount - 1; i > -1; i--)
				m_detectedFaceListBox.DeleteString(i);

			view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options);

			UpdateData(true);
			
			int selFace;
			HPS::ComponentArray highlightCompArr;

#ifdef USING_EXCHANGE_PARASOLID
			selFace = ((HPS::Parasolid::Component)selCompArr[0]).GetParasolidEntity();

			HPS::Component bodyComp = view->GetOwnerPSBodyCompo(selCompArr[0]);
			PK_BODY_t body = ((HPS::Parasolid::Component)bodyComp).GetParasolidEntity();

			std::vector<PK_ENTITY_t> pkEntityArr;
			if (m_pProcess->FR((PsFRType)m_iFRType, selFace, pkEntityArr))
			{
				for (int i = 0; i < pkEntityArr.size(); i++)
				{
					PK_FACE_t face = pkEntityArr[i];

					if (selFace != face)
					{
						// Highlight face
						HPS::Component comp = view->GetPsComponent(body, face);
						highlightCompArr.push_back(comp);

						// Show list box
						CString sEnt;
						sEnt.Format(_T("%d"), pkEntityArr[i]);
						m_detectedFaceListBox.AddString(sEnt);
					}
				}
			}

#else
			// Get owner BrepModel
			HPS::Component ownerComp = view->GetOwnerBrepModel(selCompArr[0]);
			A3DRiBrepModel* pRiBrepModel = HPS::Exchange::Component(ownerComp).GetExchangeEntity();

			A3DTopoFace* pSelTopoFace = HPS::Exchange::Component(selCompArr[0]).GetExchangeEntity();
			selFace = m_pProcess->GetEntityTag(pRiBrepModel, pSelTopoFace);

			std::vector<A3DEntity*> pEntityArr;
			if (m_pProcess->FR((PsFRType)m_iFRType, pRiBrepModel, pSelTopoFace, pEntityArr))
			{
				// Get component from selected A3DEntity
				HPS::Exchange::CADModel cad_model = view->GetDocument()->GetCADModel();
				for (int i = 0; i < pEntityArr.size(); i++)
				{
					A3DEntity* pEntity = pEntityArr[i];

					if (pSelTopoFace != pEntity)
					{
						HPS::Component comp = cad_model.GetComponentFromEntity(pEntity);
						highlightCompArr.push_back(comp);

						// Show list box
						int iEnt = m_pProcess->GetEntityTag(pRiBrepModel, pEntity);

						CString sEnt;
						sEnt.Format(_T("%d"), iEnt);
						m_detectedFaceListBox.AddString(sEnt);
					}
				}

			}
#endif
			m_iSelFace = selFace;

			// Highlight components for FR
			if (0 < highlightCompArr.size())
			{
				// Find key of the selected ProductOccurence
				HPS::Key selPOKey;
				HPS::CADModel cadModel = view->GetDocument()->GetCADModel();

				HPS::ComponentPath selCompPath = m_pCmdOp->GetSelectedComponentPath(0);
				HPS::ComponentArray selCompArr = selCompPath.GetComponents();
				for (int i = 0; i < selCompArr.size(); i++)
				{
					HPS::Component selComp = selCompArr[i];
					HPS::Component::ComponentType selCompType = selComp.GetComponentType();
					if (HPS::Component::ComponentType::ExchangeProductOccurrence == selCompType)
					{
						HPS::KeyArray selKeyArr = selComp.GetKeys();
						selPOKey = selKeyArr[0];
						break;
					}
				}

				for (int i = 0; i < highlightCompArr.size(); i++)
				{
					HPS::Component highlightComp = highlightCompArr[i];
					HPS::KeyPathArray keyPathArr = HPS::Component::GetKeyPath(highlightComp);

					for (int i = 0; i < keyPathArr.size(); i++)
					{
						HPS::KeyPath keyPath = keyPathArr[i];

						HPS::KeyArray keyArr;
						keyPath.ShowKeys(keyArr);

						for (int j = 0; j < keyArr.size(); j++)
						{
							HPS::Key key = keyArr[j];
							// Highlight if the key path contains selected PO key
							if (selPOKey == key)
							{
								view->GetCanvas().GetWindowKey().GetHighlightControl().Highlight(keyPath, m_highlight_options);
								break;
							}
						}

					}
				}
			}

			UpdateData(false);

			// Show process time
			auto t1 = std::chrono::system_clock::now();

			auto dur1 = t1 - t0;
			auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();

			wchar_t wcsbuf[256];
			swprintf(wcsbuf, sizeof(wcsbuf), L"Process time: %d msec", msec1);
			view->ShowMessage(wcsbuf);
		}
	}
	else
	{
		clear();
	}

	view->GetCanvas().Update();

	CDialogEx::OnTimer(nIDEvent);
}


void FeatureRecognitionDlg::OnClose()
{
	if (m_timerID != 0)
	{
		BOOL err = KillTimer(m_timerID);
		m_timerID = 0;
	}

	CDialogEx::OnClose();
}

void FeatureRecognitionDlg::clear()
{
	UpdateData(true);
	
	m_pCmdOp->ClearSelection();

	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options);

	m_iSelFace = 0;

	// Clear list box
	int nCount = m_detectedFaceListBox.GetCount();
	for (int i = nCount - 1; i > -1; i--)
		m_detectedFaceListBox.DeleteString(i);

	UpdateData(false);

	view->GetCanvas().Update();
}

void FeatureRecognitionDlg::OnBnClickedRadioFrBoss()
{
	clear();
}


void FeatureRecognitionDlg::OnBnClickedRadioFrConcentric()
{
	clear();
}


void FeatureRecognitionDlg::OnBnClickedRadioFrCoplanar()
{
	clear();
}
