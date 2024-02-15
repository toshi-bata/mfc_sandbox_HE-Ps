#pragma once
#include "sprk.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ExProcess.h"

class MirrorBodyOp :
	public HPS::Operator
{
public:
	MirrorBodyOp(CHPSView* in_view, void* pProcess, HPS::MouseButtons button = HPS::MouseButtons::ButtonLeft(), HPS::ModifierKeys modifiers = HPS::ModifierKeys());
	~MirrorBodyOp();
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
	HPS::Component::ComponentType m_compType1;
	HPS::Component::ComponentType m_compType2;
	bool m_bIsActive;
	HPS::WindowPoint m_clickPnt;
	
	HPS::HighlightOptionsKit m_highlight_options_1;
	HPS::HighlightOptionsKit m_highlight_options_2;

	int m_iStep;
	HPS::Component m_targetComp;
	HPS::ComponentPath m_targetCompPath;
	HPS::Component m_toolComp;
	HPS::ComponentPath m_toolCompPath;

public:
	bool m_bUpdated;
	void SetSelectionStep(const int step) { m_iStep = step; }
	HPS::Component GetTargetComponent() { return m_targetComp; }
	HPS::Component GetToolComponent() { return m_toolComp; }
	void Unhighlight();
};

