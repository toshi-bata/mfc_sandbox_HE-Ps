#include "stdafx.h"
#include "MirrorBodyOp.h"

MirrorBodyOp::MirrorBodyOp(CHPSView* in_view, void* pProcess, HPS::MouseButtons button, HPS::ModifierKeys modifiers)
	:Operator(button, modifiers)
	, view(in_view)
	, m_bUpdated(false)
	, m_iStep(0)
{
#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	m_compType1 = HPS::Component::ComponentType::ParasolidTopoBody;
	m_compType2 = HPS::Component::ComponentType::ParasolidTopoFace;
#else
	m_pProcess = (ExProcess*)pProcess;
	m_compType1 = HPS::Component::ComponentType::ExchangeRIBRepModel;
	m_compType2 = HPS::Component::ComponentType::ExchangeTopoFace;
#endif

	//! [build_highlight_style 2]
	{
		m_highlight_options_1 = HPS::HighlightOptionsKit::GetDefault();
		m_highlight_options_1.SetStyleName("highlight_style");
		m_highlight_options_1.SetSubentityHighlighting(false);
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
	//! [build_highlight_style 2]
}

MirrorBodyOp::~MirrorBodyOp()
{
}

void MirrorBodyOp::OnViewDetached(HPS::View const& in_view)
{
	Unhighlight();
}

bool MirrorBodyOp::OnMouseDown(HPS::MouseState const& in_state)
{
	if (!in_state.GetButtons().Left())
		return false;

	m_clickPnt = in_state.GetLocation();
	m_bIsActive = true;

	return false;
}

bool MirrorBodyOp::OnMouseUp(HPS::MouseState const& in_state)
{
	if (!m_bIsActive)
		return false;

	HPS::Vector sub = m_clickPnt.operator-(in_state.GetLocation());
	double len = sub.Length();
	if (1.0e-6 < len)
		return false;


	HPS::SelectionOptionsKit selection_options;
	selection_options.SetAlgorithm(HPS::Selection::Algorithm::Analytic);
	selection_options.SetSorting(true);
	if (0 == m_iStep)
		selection_options.SetLevel(HPS::Selection::Level::Segment);
	else
		selection_options.SetLevel(HPS::Selection::Level::Subentity);

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

			if (m_compType1 == comptype || m_compType2 == comptype)
			{
				if (0 == m_iStep)
				{
					m_targetComp = selComp;
					m_targetCompPath = compPath;

					view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_1);
					GetAttachedView().Update(HPS::Window::UpdateType::Complete);
				
					m_highlight_options_1.SetNotification(true);
					m_targetCompPath.Highlight(view->GetCanvas(), m_highlight_options_1);
				}
				else
				{
					m_toolComp = selComp;
					m_toolCompPath = compPath;

					view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_2);
					GetAttachedView().Update(HPS::Window::UpdateType::Complete);
					
					m_highlight_options_2.SetNotification(true);
					compPath.Highlight(view->GetCanvas(), m_highlight_options_2);
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

void MirrorBodyOp::Unhighlight()
{
	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_1);
	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_2);
	view->GetCanvas().Update(HPS::Window::UpdateType::Complete);
}