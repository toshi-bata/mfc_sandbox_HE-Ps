#include "stdafx.h"
#include "ClickEntitiesCmdOp.h"

ClickEntitiesCmdOp::ClickEntitiesCmdOp(HPS::Component::ComponentType entityType, CHPSView* in_view, void* pProcess, 
	bool single, HPS::MouseButtons button, HPS::ModifierKeys modifiers)
	: Operator(button, modifiers)
	, view(in_view)
	, m_compType(entityType)
	, m_bIsSingleSelection(single)
	, m_eClickCompType(ClickCompType::CLICK_BODY)
	, m_bUpdated(false)
{
	if (HPS::Component::ComponentType::ParasolidTopoBody == m_compType ||
		HPS::Component::ComponentType::ExchangeRIBRepModel == m_compType)
	{
		m_selectionLevel = HPS::Selection::Level::Segment;
	}
	else
	{
		m_selectionLevel = HPS::Selection::Level::Subentity;
	}

#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
#else
	m_pProcess = (ExProcess*)pProcess;
#endif

	//! [build_highlight_style]
	{
		const char *styleName = "highlight_style_1";
		HPS::PortfolioKeyArray portArr;
		view->GetCanvas().GetFrontView().GetSegmentKey().GetPortfolioControl().Show(portArr);

		bool bFind = false;
		for (int i = 0; i < portArr.size(); i++)
		{
			HPS::PortfolioKey portforio = portArr[i];
			if (portforio.ShowNamedStyleDefinition(styleName))
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			HPS::PortfolioKey myPortfolio = HPS::Database::CreatePortfolio();
			view->GetCanvas().GetFrontView().GetSegmentKey().GetPortfolioControl().Push(myPortfolio);

			HPS::NamedStyleDefinition myHighlightStyle = myPortfolio.DefineNamedStyle(styleName, HPS::Database::CreateRootSegment());
			myHighlightStyle.GetSource().GetMaterialMappingControl().SetFaceColor(HPS::RGBAColor(0.0f, 1.0f, 1.0f))
				.SetLineColor(HPS::RGBAColor(0.0f, 1.0f, 1.0f));
			myHighlightStyle.GetSource().GetLineAttributeControl().SetWeight(5);
		}

		m_highlight_options_1.SetStyleName(styleName);
		m_highlight_options_1.SetOverlay(HPS::Drawing::Overlay::InPlace);

	}
	{
		const char* styleName = "highlight_style_2";
		HPS::PortfolioKeyArray portArr;
		view->GetCanvas().GetFrontView().GetSegmentKey().GetPortfolioControl().Show(portArr);

		bool bFind = false;
		for (int i = 0; i < portArr.size(); i++)
		{
			HPS::PortfolioKey portforio = portArr[i];
			if (portforio.ShowNamedStyleDefinition(styleName))
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			HPS::PortfolioKey myPortfolio = HPS::Database::CreatePortfolio();
			view->GetCanvas().GetFrontView().GetSegmentKey().GetPortfolioControl().Push(myPortfolio);

			HPS::NamedStyleDefinition myHighlightStyle = myPortfolio.DefineNamedStyle(styleName, HPS::Database::CreateRootSegment());
			myHighlightStyle.GetSource().GetMaterialMappingControl().SetFaceColor(HPS::RGBAColor(1.0f, 0.0f, 1.0f))
				.SetLineColor(HPS::RGBAColor(1.0f, 0.0f, 1.0f));
		}

		m_highlight_options_2.SetStyleName(styleName);
		m_highlight_options_2.SetOverlay(HPS::Drawing::Overlay::InPlace);
	}
	//! [build_highlight_style]
}

ClickEntitiesCmdOp::~ClickEntitiesCmdOp()
{
}

void ClickEntitiesCmdOp::OnViewDetached(HPS::View const& in_view)
{
	Unhighlight();
}

bool ClickEntitiesCmdOp::OnMouseDown(HPS::MouseState const& in_state)
{
	if (!in_state.GetButtons().Left())
		return false;

	m_clickPnt = in_state.GetLocation();
	m_bIsActive = true;

	return false;
}

bool ClickEntitiesCmdOp::OnMouseUp(HPS::MouseState const& in_state)
{
	if (!m_bIsActive)
		return false;

	HPS::Vector sub = m_clickPnt.operator-(in_state.GetLocation());
	double len = sub.Length();
	if (1.0e-6 < len)
		return false;

	if (m_bIsSingleSelection)
	{
		Unhighlight();
		HPS::ComponentArray().swap(m_selCompArr);
		HPS::ComponentArray().swap(m_firstFaceCompArr);
		HPS::ComponentPathArray().swap(m_selCompPathArr);

	}

	HPS::SelectionOptionsKit selection_options;
	selection_options.SetAlgorithm(HPS::Selection::Algorithm::Analytic);
	selection_options.SetLevel(m_selectionLevel).SetSorting(true);

	HPS::SelectionResults selection_results;
	size_t number_of_selected_items = in_state.GetEventSource().GetSelectionControl().SelectByPoint(in_state.GetLocation(), selection_options, selection_results);

	if (0 < number_of_selected_items)
	{

		HPS::SelectionResultsIterator it = selection_results.GetIterator();
		while (it.IsValid())
		{
			HPS::Key selected_key;
			it.GetItem().ShowSelectedItem(selected_key);

			HPS::CADModel cad_model = view->GetDocument()->GetCADModel();
			HPS::Component selComp = cad_model.GetComponentFromKey(selected_key);
			HPS::Component::ComponentType comptype = selComp.GetComponentType();

			HPS::ComponentPath compPath = cad_model.GetComponentPath(it.GetItem());

			if (ClickCompType::CLICK_PART == m_eClickCompType)
			{
				// Get PO of the RiBrepModel from component path 
				if (!compPath.Empty())
				{
					HPS::ComponentArray compArr = compPath.GetComponents();

					do
					{
						HPS::Component poComp = compArr[0];
						HPS::Component::ComponentType compType = poComp.GetComponentType();
						if (HPS::Component::ComponentType::ExchangeProductOccurrence == compType)
						{
							selComp = poComp;
							compPath = HPS::ComponentPath(compArr);
							break;
						}

						compArr.erase(compArr.begin());
					} while (compArr.size());
				}
			}

			if (m_compType == comptype)
			{
				bool bFlg = false;
				for (int i = 0; i < m_selCompArr.size(); i++)
				{
					if (m_selCompArr[i] == selComp)
					{
						m_selCompArr.erase(std::cbegin(m_selCompArr) + i);
						m_firstFaceCompArr.erase(std::cbegin(m_firstFaceCompArr) + i);
						m_selCompPathArr.erase(std::cbegin(m_selCompPathArr) + i);
						bFlg = true;
						break;
					}
				}

				if (!bFlg)
				{
					m_selCompArr.push_back(selComp);
					m_selCompPathArr.push_back(compPath);

					// Make the selected component get highlighted in the model browser
					m_highlight_options_1.SetNotification(true);
					compPath.Highlight(view->GetCanvas(), m_highlight_options_1);
				}
				else
				{
					Unhighlight();
					for (int i = 0; i < m_selCompPathArr.size(); i++)
					{
						m_highlight_options_1.SetNotification(true);
						m_selCompPathArr[i].Highlight(view->GetCanvas(), m_highlight_options_1);
					}
				}
				GetAttachedView().Update();

				m_bUpdated = true;
				return true;
			}
			it.Next();
		}
	}
	return false;
}

void ClickEntitiesCmdOp::ClearSelection()
{
	HPS::ComponentArray().swap(m_selCompArr);
	HPS::ComponentArray().swap(m_firstFaceCompArr);
	HPS::ComponentPathArray().swap(m_selCompPathArr);

	Unhighlight();
	GetAttachedView().Update();
}

void ClickEntitiesCmdOp::Unhighlight()
{
	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_1);
	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_2);
	view->GetCanvas().Update(HPS::Window::UpdateType::Complete);
}

void ClickEntitiesCmdOp::HighlightFaceOfEdge(const int id)
{
	if (0 == m_selCompPathArr.size())
		return;

	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_2);
	view->GetCanvas().Update(HPS::Window::UpdateType::Complete);

	// Get the last component and path
	HPS::ComponentPath compPath = m_selCompPathArr[m_selCompPathArr.size() - 1];

	HPS::ComponentArray pathCompArr = compPath.GetComponents();
	
	HPS::Component selComp = pathCompArr[0];

	HPS::Component::ComponentType compType = selComp.GetComponentType();
	HPS::Component::ComponentType targetFaceCompType;

#ifdef USING_EXCHANGE_PARASOLID
	if (HPS::Component::ComponentType::ParasolidTopoEdge != compType)
		return;
	targetFaceCompType = HPS::Component::ComponentType::ParasolidTopoFace;
#else
	if (HPS::Component::ComponentType::ExchangeTopoEdge != compType)
		return;
	targetFaceCompType = HPS::Component::ComponentType::ExchangeTopoFace;
#endif

	// Get comp path array from topoShell to ModelFile
	pathCompArr.erase(pathCompArr.begin());

	HPS::ComponentArray compArr;
	compArr = selComp.GetOwners();

	compArr = compArr[id].GetOwners();
	compType = compArr[0].GetComponentType();

	while (targetFaceCompType != compType)
	{
		compArr = compArr[0].GetOwners();
		compType = compArr[0].GetComponentType();
	}

	HPS::Component faceComp = compArr[0];

	if (m_firstFaceCompArr.size() >= m_selCompArr.size())
		m_firstFaceCompArr.pop_back();

	m_firstFaceCompArr.push_back(faceComp);

	pathCompArr.insert(pathCompArr.begin(), faceComp);

	HPS::ComponentPath faceCompPath = HPS::ComponentPath(pathCompArr);

	// Make the selected component get highlighted in the model browser
	m_highlight_options_2.SetNotification(true);
	faceCompPath.Highlight(view->GetCanvas(), m_highlight_options_2);
	GetAttachedView().Update();
}
