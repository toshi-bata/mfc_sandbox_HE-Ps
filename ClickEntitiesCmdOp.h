#pragma once
#include "sprk.h"
#include "CHPSDoc.h"
#include "CHPSView.h"

#include "ExProcess.h"

enum ClickCompType
{
	CLICK_PART,
	CLICK_BODY
};

class ClickEntitiesCmdOp :
	public HPS::Operator
{
public:
	ClickEntitiesCmdOp(HPS::Component::ComponentType entityType, CHPSView* in_view, void* pProcess, bool single,
		HPS::MouseButtons button = HPS::MouseButtons::ButtonLeft(), 
		HPS::ModifierKeys modifiers = HPS::ModifierKeys());
	~ClickEntitiesCmdOp();

	virtual HPS::UTF8 GetName() const { return "ClickEntitiesCmdOperator"; }

	virtual bool OnMouseDown(HPS::MouseState const& in_state) override;
	virtual bool OnMouseUp(HPS::MouseState const& in_state) override;
	void OnViewDetached(HPS::View const& in_view) override;

private:
	CHPSView* view;
	HPS::Selection::Level m_selectionLevel;
	HPS::Component::ComponentType m_compType;
	ClickCompType m_eClickCompType;
	bool m_bIsSingleSelection;
	bool m_bIsActive;
	HPS::WindowPoint m_clickPnt;
#ifdef USING_EXCHANGE_PARASOLID
	ExPsProcess* m_pProcess;
#else
	ExProcess* m_pProcess;
#endif
	HPS::ComponentArray m_selCompArr;
	HPS::ComponentArray m_firstFaceCompArr;
	HPS::ComponentPathArray m_selCompPathArr;
	HPS::HighlightOptionsKit m_highlight_options_1;
	HPS::HighlightOptionsKit m_highlight_options_2;

public:
	bool m_bUpdated;
	HPS::ComponentArray GetSelectedComponents() { return m_selCompArr; };
	HPS::ComponentArray GetFirstFaceComponents() { return m_firstFaceCompArr; };
	HPS::ComponentPath GetSelectedComponentPath(int id) { return m_selCompPathArr[id]; }
	void ClearSelection();
	void SetClickCompType(ClickCompType clickType) { m_eClickCompType = clickType; }
	void Unhighlight();
	void HighlightFaceOfEdge(const int id);
};

