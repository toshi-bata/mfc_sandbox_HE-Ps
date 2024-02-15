#include "ExUtilities.h"

bool SearchBrepModelFromTopoBrep(A3DAsmModelFile* pModelFile, A3DTopoBrepData* pTopoBrep, A3DRiBrepModel*& pRiBrepModel)
{
	A3DStatus ex_status;

	A3DAsmModelFileData sModelFileData;
	A3D_INITIALIZE_DATA(A3DAsmModelFileData, sModelFileData);
	if (A3D_SUCCESS != A3DAsmModelFileGet(pModelFile, &sModelFileData))
		return false;

	A3DAsmProductOccurrenceData sPOData;
	A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sPOData);
	if (A3DAsmProductOccurrenceGet(sModelFileData.m_ppPOccurrences[0], &sPOData))
		return false;

	A3DAsmPartDefinitionData sPartData;
	A3D_INITIALIZE_DATA(A3DAsmPartDefinitionData, sPartData);
	if (A3DAsmPartDefinitionGet(sPOData.m_pPart, &sPartData))
		return false;

	A3DRiRepresentationItem* pRi = sPartData.m_ppRepItems[0];
	A3DEEntityType eType;
	ex_status = A3DEntityGetType(pRi, &eType);

	if (A3DEEntityType::kA3DTypeRiBrepModel != eType)
		return false;

	pRiBrepModel = (A3DRiBrepModel*)pRi;

	A3DRiBrepModelData sRiBrepModelData;
	A3D_INITIALIZE_DATA(A3DRiBrepModelData, sRiBrepModelData);
	if (A3DRiBrepModelGet(pRiBrepModel, &sRiBrepModelData))
		return false;

	if (sRiBrepModelData.m_pBrepData = pTopoBrep)
		return true;

	return false;
}

bool PkBodyToA3DRiBrepModel(int body, A3DRiBrepModel*& pRiBrepModel, A3DMiscPKMapper*& pPkMapper)
{
	A3DStatus status;

	A3DRWParamsLoadData sParams;
	A3D_INITIALIZE_DATA(A3DRWParamsLoadData, sParams);
	sParams.m_sGeneral.m_bReadSolids = true;
	sParams.m_sGeneral.m_eReadGeomTessMode = kA3DReadGeomAndTess;

	A3DAsmModelFile* pBlockModelFile;
	status = A3DPkPartsTranslateToA3DAsmModelFile(1, &body, &sParams, &pBlockModelFile, &pPkMapper);

	// Get A3DEntity of created PK_BODY
	int iCnt;
	A3DEntity** ppEntities;
	status = A3DMiscPKMapperGetA3DEntitiesFromPKEntity(pPkMapper, body, &iCnt, &ppEntities);

	// Get RiBrepModel from TopoBrepData and add to the mapping table
	A3DTopoBrepData* pTopoBrepData = ppEntities[0];
	
	if (!SearchBrepModelFromTopoBrep(pBlockModelFile, pTopoBrepData, pRiBrepModel))
		return false;

	return true;
}