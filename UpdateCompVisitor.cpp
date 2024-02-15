#include "UpdateCompVisitor.h"

SearchCompIdVisitor::SearchCompIdVisitor(A3DVisitorContainer* psContainer, std::vector<UpdatedComponent> updatedRiBrepArr)
	: A3DTreeVisitor(psContainer)
	, m_updatedComp(updatedRiBrepArr)
	, m_iRiBrepId(0)
	, m_iPOId(0)
{
}

SearchCompIdVisitor::~SearchCompIdVisitor()
{
}

A3DStatus SearchCompIdVisitor::visitLeave(const A3DRiBrepModelConnector& sConnector)
{
	A3DStatus iRet = A3D_SUCCESS;

	A3DRiBrepModel* pRiBrepModel = (A3DRiBrepModel*)sConnector.GetA3DEntity();

	A3DRiBrepModelData sData = sConnector.m_sRiBrepModelData;

	int iTarget = -1;
	for (A3DUns32 ui = 0; ui < m_updatedComp.size(); ui++)
	{
		if (m_updatedComp[ui].pTargetComp == pRiBrepModel)
		{
			m_updatedComp[ui].iCompId = m_iRiBrepId;
		}
	}

	m_iRiBrepId++;

	iRet = A3DTreeVisitor::visitLeave(sConnector);
	return iRet;
}

A3DStatus SearchCompIdVisitor::visitLeave(const A3DProductOccurrenceConnector& sConnector)
{
	A3DStatus iRet = A3D_SUCCESS;

	A3DAsmProductOccurrence* pPO = (A3DAsmProductOccurrence*)sConnector.GetA3DEntity();

	for (A3DUns32 ui = 0; ui < m_updatedComp.size(); ui++)
	{
		if (m_updatedComp[ui].pTargetComp == pPO)
		{
			m_updatedComp[ui].iCompId = m_iPOId;
		}
	}
	m_iPOId++;

	iRet = A3DTreeVisitor::visitLeave(sConnector);
	return iRet;
}

UpdatedCompVisitor::UpdatedCompVisitor(A3DVisitorContainer* psContainer, std::vector<UpdatedComponent> updatedRiBrepArr)
	: A3DTreeVisitor(psContainer)
	, m_updatedComp(updatedRiBrepArr)
	, m_iRiBrepId(0)
	, m_iPoId(0)
{
}

UpdatedCompVisitor::~UpdatedCompVisitor()
{
}

A3DStatus UpdatedCompVisitor::visitEnter(const A3DProductOccurrenceConnector& sConnector)
{
	A3DStatus iRet = A3DTreeVisitor::visitEnter(sConnector);;

	A3DAsmProductOccurrence* pPO = (A3DAsmProductOccurrence*)sConnector.GetA3DEntity();

	for (A3DUns32 ui = 0; ui < m_updatedComp.size(); ui++)
	{
		if (UpdatedType::DELEATE_PART == m_updatedComp[ui].updateType)
		{
			if (m_iPoId == m_updatedComp[ui].iCompId)
			{
				m_targetPOArr.push_back(pPO);
			}
		}
	}

	return iRet;
}

A3DStatus UpdatedCompVisitor::visitEnter(const A3DPartConnector& sConnector)
{
	A3DStatus iRet = A3DTreeVisitor::visitEnter(sConnector);

	m_targetRiBrep.clear();
	m_newRiBrepArr.clear();
	m_iDeletedCnt = 0;

	return iRet;
}

A3DStatus UpdatedCompVisitor::visitLeave(const A3DRiBrepModelConnector& sConnector)
{
	A3DStatus iRet = A3D_SUCCESS;

	A3DRiBrepModel* pRiBrepModel = (A3DRiBrepModel*)sConnector.GetA3DEntity();

	for (A3DUns32 ui = 0; ui < m_updatedComp.size(); ui++)
	{
		if (m_iRiBrepId == m_updatedComp[ui].iCompId)
		{
			m_targetRiBrep.push_back(pRiBrepModel);

			if (UpdatedType::UPDATE == m_updatedComp[ui].updateType)
			{
				m_newRiBrepArr.push_back(m_updatedComp[ui].pNewRiBrep);
			}
			else
			{
				m_iDeletedCnt++;
				m_newRiBrepArr.push_back(NULL);
			}
		}
	}

	m_iRiBrepId++;

	iRet = A3DTreeVisitor::visitLeave(sConnector);
	return iRet;
}

A3DStatus UpdatedCompVisitor::visitLeave(const A3DPartConnector& sConnector)
{
	A3DStatus iRet = A3D_SUCCESS;

	A3DAsmPartDefinition* pPart = (A3DAsmPartDefinition*)sConnector.GetA3DEntity();

	A3DAsmProductOccurrence* pPO = (A3DAsmProductOccurrence*)sConnector.GetProductOccurrenceFather();
	A3DRootBaseData sBaseData;
	A3D_INITIALIZE_DATA(A3DRootBaseData, sBaseData);
	A3DRootBaseGet(pPO, &sBaseData);

	A3DAsmPartDefinitionData sData = sConnector.m_sPartData;

	// kA3DTypeRiBrepModel


	if (m_targetRiBrep.size())
	{
		std::vector<A3DRiRepresentationItem*> pRepItemArr;
		for (A3DUns32 ui = 0; ui < sData.m_uiRepItemsSize; ui++)
		{
			A3DRiRepresentationItem* pRiItem = sData.m_ppRepItems[ui];

			A3DEEntityType eType;
			iRet = A3DEntityGetType(pRiItem, &eType);

			if (kA3DTypeRiBrepModel == eType)
			{
				bool bFlg = false;
				for (A3DUns32 uj = 0; uj < m_targetRiBrep.size(); uj++)
				{
					if (sData.m_ppRepItems[ui] == m_targetRiBrep[uj])
					{
						if (NULL != m_newRiBrepArr[uj])
							pRepItemArr.push_back(m_newRiBrepArr[uj]);

						bFlg = true;
						break;
					}
				}
				if (!bFlg)
					pRepItemArr.push_back(pRiItem);
			}
			else if (kA3DTypeRiSet == eType)
			{
				A3DRiSetData sSetData;
				A3D_INITIALIZE_DATA(A3DRiSetData, sSetData);
				iRet = A3DRiSetGet(pRiItem, &sSetData);

				std::vector<A3DRiRepresentationItem*> pSucRepItemArr;
				bool bSubFlg = false;
				for (A3DUns32 uk = 0; uk < sSetData.m_uiRepItemsSize; uk++)
				{
					for (A3DUns32 uj = 0; uj < m_targetRiBrep.size(); uj++)
					{
						if (sSetData.m_ppRepItems[uk] == m_targetRiBrep[uj])
						{
							if (NULL != m_newRiBrepArr[uj])
								pSucRepItemArr.push_back(m_newRiBrepArr[uj]);

							bSubFlg = true;
							break;
						}
					}
					if (!bSubFlg)
						pSucRepItemArr.push_back(sSetData.m_ppRepItems[uk]);
				}

				if (bSubFlg)
				{
					sSetData.m_uiRepItemsSize = pSucRepItemArr.size();
					sSetData.m_ppRepItems = pSucRepItemArr.data();
					iRet = A3DRiSetEdit(&sSetData, pRiItem);
				}
				pRepItemArr.push_back(pRiItem);
			}
		}

		sData.m_uiRepItemsSize = pRepItemArr.size();
		sData.m_ppRepItems = pRepItemArr.data();

		iRet = A3DAsmPartDefinitionEdit(&sData, pPart);
	}

	iRet = A3DTreeVisitor::visitLeave(sConnector);
	return iRet;
}

A3DStatus UpdatedCompVisitor::visitLeave(const A3DProductOccurrenceConnector& sConnector)
{
	A3DStatus iRet = A3D_SUCCESS;

	A3DAsmProductOccurrence* pPO = (A3DAsmProductOccurrence*)sConnector.GetA3DEntity();
	A3DAsmProductOccurrenceData sData = sConnector.m_sProductOccurrenceData;

	std::vector<A3DAsmProductOccurrence*> pNewPOArr;
	for (A3DUns32 ui = 0; ui < sData.m_uiPOccurrencesSize; ui++)
	{
		bool bFlg = false;
		for (A3DUns32 uj = 0; uj < m_targetPOArr.size(); uj++)
		{
			if (sData.m_ppPOccurrences[ui] == m_targetPOArr[uj])
			{
				bFlg = true;
				break;
			}

		}
		if (!bFlg)
			pNewPOArr.push_back(sData.m_ppPOccurrences[ui]);
	}

	if (pNewPOArr.size() < sData.m_uiPOccurrencesSize)
	{
		sData.m_uiPOccurrencesSize = pNewPOArr.size();
		sData.m_ppPOccurrences = pNewPOArr.data();

		iRet = A3DAsmProductOccurrenceEdit(&sData, pPO);
	}

	m_iPoId++;

	iRet = A3DTreeVisitor::visitLeave(sConnector);
	return iRet;
}