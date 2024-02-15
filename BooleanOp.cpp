#include "stdafx.h"
#include "BooleanOp.h"

BooleanOp::BooleanOp(CHPSView* in_view, void* pProcess, HPS::MouseButtons button, HPS::ModifierKeys modifiers)
	:Operator(button, modifiers)
	, view(in_view)
	, m_bUpdated(false)
	, m_iStep(0)
{
#ifdef USING_EXCHANGE_PARASOLID
	m_pProcess = (ExPsProcess*)pProcess;
	m_compType = HPS::Component::ComponentType::ParasolidTopoBody;
#else
	m_pProcess = (ExProcess*)pProcess;
	m_compType = HPS::Component::ComponentType::ExchangeRIBRepModel;
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

BooleanOp::~BooleanOp()
{
}

void BooleanOp::OnViewDetached(HPS::View const& in_view)
{
	Unhighlight();
}

bool BooleanOp::OnMouseDown(HPS::MouseState const& in_state)
{
	if (!in_state.GetButtons().Left())
		return false;

	m_clickPnt = in_state.GetLocation();
	m_bIsActive = true;

	return false;
}

bool BooleanOp::OnMouseUp(HPS::MouseState const& in_state)
{
	if (!m_bIsActive)
		return false;

	HPS::Vector sub = m_clickPnt.operator-(in_state.GetLocation());
	double len = sub.Length();
	if (1.0e-6 < len)
		return false;


	HPS::SelectionOptionsKit selection_options;
	selection_options.SetAlgorithm(HPS::Selection::Algorithm::Analytic);
	selection_options.SetLevel(HPS::Selection::Level::Segment).SetSorting(true);

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

			if (m_compType == comptype)
			{
				if (0 == m_iStep)
				{
					for (int i = 0; i < m_toolCompArr.size(); i++)
					{
						if (m_toolCompArr[i] == selComp)
						{
							m_toolCompArr.erase(std::cbegin(m_toolCompArr) + i);
							m_toolCompPathArr.erase(std::cbegin(m_toolCompPathArr) + i);
							break;
						}
					}

					m_targetComp = selComp;
					m_targetCompPath = compPath;

					m_highlight_options_1.SetNotification(true);
					m_targetCompPath.Highlight(view->GetCanvas(), m_highlight_options_1);
				}
				else
				{
					if (m_targetComp == selComp)
					{
						m_targetComp = HPS::Component();
						m_targetCompPath = HPS::ComponentPath();
					}

					bool bFlg = false;
					for (int i = 0; i < m_toolCompArr.size(); i++)
					{
						if (m_toolCompArr[i] == selComp)
						{
							m_toolCompArr.erase(std::cbegin(m_toolCompArr) + i);
							m_toolCompPathArr.erase(std::cbegin(m_toolCompPathArr) + i);
							bFlg = true;
							break;
						}
					}

					if (!bFlg)
					{
						m_toolCompArr.push_back(selComp);
						m_toolCompPathArr.push_back(compPath);

						// Make the selected component get highlighted in the model browser
						m_highlight_options_2.SetNotification(true);
						compPath.Highlight(view->GetCanvas(), m_highlight_options_2);
					}
					else
					{
						view->Unhighlight();
						m_highlight_options_1.SetNotification(true);
						m_targetCompPath.Highlight(view->GetCanvas(), m_highlight_options_1);

						for (int i = 0; i < m_toolCompPathArr.size(); i++)
						{
							m_highlight_options_2.SetNotification(true);
							m_toolCompPathArr[i].Highlight(view->GetCanvas(), m_highlight_options_2);
						}
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

void BooleanOp::Unhighlight()
{
	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_1);
	view->GetCanvas().GetWindowKey().GetHighlightControl().Unhighlight(m_highlight_options_2);
	view->GetCanvas().Update(HPS::Window::UpdateType::Complete);
}