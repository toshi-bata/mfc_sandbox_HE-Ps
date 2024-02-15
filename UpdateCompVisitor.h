#pragma once
#include "visitor/VisitorTree.h"

enum UpdatedType
{
	UPDATE,
	DELEATE_BODY,
	DELEATE_PART
};
struct UpdatedComponent
{
	A3DEntity* pTargetComp;
	int body;
	UpdatedType updateType;
	int iCompId;
	A3DRiBrepModel* pNewRiBrep;
};

class SearchCompIdVisitor :
	public A3DTreeVisitor
{
public:
	SearchCompIdVisitor(A3DVisitorContainer* psContainer, std::vector<UpdatedComponent> updatedRiBrepArr);
	~SearchCompIdVisitor();

private:
	int m_iRiBrepId;
	int m_iPOId;

public:
	std::vector<UpdatedComponent> m_updatedComp;
	virtual A3DStatus visitLeave(const A3DRiBrepModelConnector& sConnector) override;
	virtual A3DStatus visitLeave(const A3DProductOccurrenceConnector& sConnector) override;

};

class UpdatedCompVisitor :
	public A3DTreeVisitor
{
public:
	UpdatedCompVisitor(A3DVisitorContainer* psContainer, std::vector<UpdatedComponent> updatedRiBrepArr);
	~UpdatedCompVisitor();

private:
	std::vector<UpdatedComponent> m_updatedComp;
	std::vector<A3DRiBrepModel*> m_targetRiBrep;
	std::vector<A3DRiBrepModel*> m_newRiBrepArr;
	std::vector<A3DRiBrepModel*> m_emptyPartArr;
	int m_iRiBrepId;
	int m_iDeletedCnt;
	int m_iPoId;
	std::vector<A3DAsmProductOccurrence*> m_targetPOArr;

public:
	virtual A3DStatus visitEnter(const A3DProductOccurrenceConnector& sConnector) override;
	virtual A3DStatus visitEnter(const A3DPartConnector& sConnector) override;

	virtual A3DStatus visitLeave(const A3DRiBrepModelConnector& sConnector) override;
	virtual A3DStatus visitLeave(const A3DPartConnector& sConnector) override;
	virtual A3DStatus visitLeave(const A3DProductOccurrenceConnector& sConnector) override;

};

