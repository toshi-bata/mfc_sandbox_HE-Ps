#pragma once
#include "sprk.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ExProcess.h"

class BooleanOp :
	public HPS::Operator
{
public:
	BooleanOp(CHPSView* in_view, void* pProcess, HPS::MouseButtons button = HPS::MouseButtons::ButtonLeft(), HPS::ModifierKeys modifiers = HPS::ModifierKeys());
	~BooleanOp();
	bool OnMouseDown(HPS::MouseState const& in_state) override;
	bool OnMouseUp(HPS::MouseState const& in_state) override;
	void OnViewDetached(HPS::View const& in_view) override;

private:
	PsBoolType m_eBoolType;
	CHPSView* view;
#ifdef USING_EXCHANGE_PARASOLID
	ExPsProcess* m_pProcess;
#else
	ExProcess* m_pProcess;
#endif	
	HPS::Component::ComponentType m_compType;
	bool m_bIsActive;
	HPS::WindowPoint m_clickPnt;
	
	HPS::HighlightOptionsKit m_highlight_options_1;
	HPS::HighlightOptionsKit m_highlight_options_2;

	int m_iStep;
	HPS::Component m_targetComp;
	HPS::ComponentPath m_targetCompPath;
	HPS::ComponentArray m_toolCompArr;
	HPS::ComponentPathArray m_toolCompPathArr;

public:
	bool m_bUpdated;
	void SetSelectionStep(const int step) { m_iStep = step; }
	HPS::Component GetTargetComponent() { return m_targetComp; }
	HPS::ComponentArray GetToolComponents() { return m_toolCompArr; }
	void Unhighlight();
};

