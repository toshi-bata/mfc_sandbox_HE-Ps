#include "ExProcess.h"

A3DRiBrepModel* SearchBrepModelFromTopoBrep(A3DAsmModelFile* pModelFile, A3DTopoBrepData* pTopoBrep)
{
	A3DStatus ex_status;

	A3DAsmModelFileData sModelFileData;
	A3D_INITIALIZE_DATA(A3DAsmModelFileData, sModelFileData);
	if (A3D_SUCCESS != A3DAsmModelFileGet(pModelFile, &sModelFileData))
		return nullptr;

	A3DAsmProductOccurrenceData sPOData;
	A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sPOData);
	if (A3DAsmProductOccurrenceGet(sModelFileData.m_ppPOccurrences[0], &sPOData))
		return nullptr;

	A3DAsmPartDefinitionData sPartData;
	A3D_INITIALIZE_DATA(A3DAsmPartDefinitionData, sPartData);
	if (A3DAsmPartDefinitionGet(sPOData.m_pPart, &sPartData))
		return nullptr;

	A3DRiRepresentationItem* pRi = sPartData.m_ppRepItems[0];
	A3DEEntityType eType;
	ex_status = A3DEntityGetType(pRi, &eType);

	if (A3DEEntityType::kA3DTypeRiBrepModel != eType)
		return nullptr;

	A3DRiBrepModel* pRiBrepModel = (A3DRiBrepModel*)pRi;

	A3DRiBrepModelData sRiBrepModelData;
	A3D_INITIALIZE_DATA(A3DRiBrepModelData, sRiBrepModelData);
	if (A3DRiBrepModelGet(pRiBrepModel, &sRiBrepModelData))
		return nullptr;

	if (sRiBrepModelData.m_pBrepData = pTopoBrep)
		return pRiBrepModel;

	return nullptr;
}

ExProcess::ExProcess()
{
	m_pModelFile = NULL;
	m_bIsPart = false;
	m_pPsProcess = new PsProcess();
}

ExProcess::~ExProcess()
{
	delete m_pPsProcess;
}

void ExProcess::Initialize()
{
	m_pPsProcess->Initialize();

	if (NULL != m_pModelFile)
		m_pModelFile = NULL;

	std::map<A3DRiBrepModel*, PK_BODY_t>().swap(m_bodyMap);
	std::map<A3DRiBrepModel*, A3DMiscPKMapper*>().swap(m_mapperMap);

}

void ExProcess::SetModelFile(A3DAsmModelFile* pModelFile)
{
	A3DStatus status;
	
	m_pModelFile = pModelFile;

	// Get model file data
	A3DAsmModelFileData sData;
	A3D_INITIALIZE_DATA(A3DAsmModelFileData, sData);
	status = A3DAsmModelFileGet(m_pModelFile, &sData);
	A3DAsmProductOccurrence* pPO = sData.m_ppPOccurrences[0];

	// Check whether part or assembly
	A3DAsmProductOccurrenceData sPoData;
	A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sPoData);
	status = A3DAsmProductOccurrenceGet(pPO, &sPoData);

	if (NULL != sPoData.m_pPart)
		m_bIsPart = true;
	else
		m_bIsPart = false;

	A3DAsmProductOccurrenceGet(NULL, &sPoData);
	A3DAsmModelFileGet(NULL, &sData);
}

PK_BODY_t ExProcess::getPkBodyFromRiBrepModel(A3DRiBrepModel* pRiBrepModel, bool translateIfNotThere)
{
	A3DStatus status;

	int body = 0;

	if (0 < m_bodyMap.count(pRiBrepModel))
		body = m_bodyMap[pRiBrepModel];

	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body && translateIfNotThere)
	{
		// Exchange => Parasolid
		A3DRWParamsTranslateToPkPartsData pParamsTranslateToPkPartsData;
		A3D_INITIALIZE_DATA(A3DRWParamsTranslateToPkPartsData, pParamsTranslateToPkPartsData);
		pParamsTranslateToPkPartsData.m_pMapper = &pPkMapper;

		int iNbPkParts;
		PK_PART_t* pPkParts;
		status = A3DRepresentationItemTranslateToPkParts((A3DRiRepresentationItem*)pRiBrepModel, 
			&pParamsTranslateToPkPartsData, 1, &iNbPkParts, &pPkParts);

		if (A3D_SUCCESS != status)
			return 0;


		PK_ASSEMBLY_t pkAssembly = pPkParts[0];

		PK_ASSEMBLY_ask_parts(pkAssembly, &iNbPkParts, &pPkParts);

		body = pPkParts[0];

		m_bodyMap[pRiBrepModel] = body;
		m_mapperMap[pRiBrepModel] = pPkMapper;
	}

	return body;
}

bool ExProcess::updatePkBodyToA3DRiBrepModel(PK_BODY_t inBody, A3DRiBrepModel* in_pRiBrepModel)
{
	A3DStatus status;

	A3DRWParamsLoadData sParams;
	A3D_INITIALIZE_DATA(A3DRWParamsLoadData, sParams);
	sParams.m_sGeneral.m_bReadSolids = true;
	sParams.m_sGeneral.m_bReadSurfaces = true;
	sParams.m_sGeneral.m_eReadGeomTessMode = kA3DReadGeomAndTess;
	sParams.m_sMultiEntries.m_bLoadDefault = true;

	A3DAsmModelFile* pModelFile;
	A3DMiscPKMapper* pPkMapper;

	PK_PART_t* pPkParts = &inBody;
	status = A3DPkPartsTranslateToA3DAsmModelFile(1, pPkParts, &sParams, &pModelFile, &pPkMapper);

	// Get Exchange entity
	int iNbA3DEntities;
	A3DEntity** ppEntiries;
	status = A3DMiscPKMapperGetA3DEntitiesFromPKEntity(pPkMapper, inBody, &iNbA3DEntities, &ppEntiries);

	if (A3D_SUCCESS != status || 1 != iNbA3DEntities)
		return false;

	A3DEntity* pResultEntity = ppEntiries[0];

	A3DEEntityType eType = kA3DTypeUnknown;
	status = A3DEntityGetType(pResultEntity, &eType);

	if (kA3DTypeTopoBrepData != eType)
		return false;

	// Update Brep Model Data
	A3DRiBrepModelData sData;
	A3D_INITIALIZE_DATA(A3DRiBrepModelData, sData);
	status = A3DRiBrepModelGet(in_pRiBrepModel, &sData);

	sData.m_pBrepData = pResultEntity;

	// Ask body type
	PK_ERROR_code_t error_code;
	PK_BODY_type_t body_type;
	error_code = PK_BODY_ask_type(inBody, &body_type);
	if (PK_BODY_type_solid_c == body_type)
		sData.m_bSolid = 1;
	else
		sData.m_bSolid = 0;

	status = A3DRiBrepModelEdit(&sData, in_pRiBrepModel);
	if (A3D_SUCCESS != status)
		return false;

	// Update tessellation
	A3DRWParamsTessellationData tessellationParameters;
	A3D_INITIALIZE_DATA(A3DRWParamsTessellationData, tessellationParameters);
	tessellationParameters.m_eTessellationLevelOfDetail = kA3DTessLODMedium;
	status = A3DRiRepresentationItemComputeTessellation((A3DRiRepresentationItem*)in_pRiBrepModel, &tessellationParameters);
	if (A3D_SUCCESS != status)
		return false;

	// Update map
	for (auto it = m_bodyMap.begin(); it != m_bodyMap.end();)
	{
		if (it->second == inBody)
			it = m_bodyMap.erase(it);
		else
			++it;
	}

	m_bodyMap[in_pRiBrepModel] = inBody;
	m_mapperMap[in_pRiBrepModel] = pPkMapper;

	return true;
}

void ExProcess::addBody(PK_BODY_t body, A3DAsmModelFile*& pNewModelFile)
{
	A3DStatus status;

	// Parasolid to Exchange
	A3DRWParamsLoadData sParams;
	A3D_INITIALIZE_DATA(A3DRWParamsLoadData, sParams);
	sParams.m_sGeneral.m_bReadSolids = true;
	sParams.m_sGeneral.m_eReadGeomTessMode = kA3DReadGeomAndTess;
	sParams.m_sTessellation.m_eTessellationLevelOfDetail = A3DETessellationLevelOfDetail::kA3DTessLODMedium;

	A3DAsmModelFile* pBlockModelFile;
	A3DMiscPKMapper* pPkMapper;
	status = A3DPkPartsTranslateToA3DAsmModelFile(1, &body, &sParams, &pBlockModelFile, &pPkMapper);

	// Get A3DEntity of created PK_BODY
	int iCnt;
	A3DEntity** ppEntities;
	status = A3DMiscPKMapperGetA3DEntitiesFromPKEntity(pPkMapper, body, &iCnt, &ppEntities);

	// Get RiBrepModel from TopoBrepData and add to the mapping table
	A3DTopoBrepData* pTopoBrepData = ppEntities[0];
	A3DRiBrepModel* pRiBrepModel = SearchBrepModelFromTopoBrep(pBlockModelFile, pTopoBrepData);

	// Add mapping table
	m_bodyMap[pRiBrepModel] = body;
	m_mapperMap[pRiBrepModel] = pPkMapper;

	if (NULL == m_pModelFile)
	{
		// Create new part model
		pNewModelFile = pBlockModelFile;
		SetModelFile(pBlockModelFile);

	}
	else
	{
		// Get model file data
		A3DAsmModelFileData sData;
		A3D_INITIALIZE_DATA(A3DAsmModelFileData, sData);
		status = A3DAsmModelFileGet(m_pModelFile, &sData);
		A3DAsmProductOccurrence* pPO = sData.m_ppPOccurrences[0];

		A3DAsmProductOccurrenceData sPoData;
		A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sPoData);
		status = A3DAsmProductOccurrenceGet(pPO, &sPoData);

		if (m_bIsPart)
		{
			// Add body to the existing part
			A3DAsmPartDefinition* pPart = sPoData.m_pPart;
			A3DAsmPartDefinitionData sPartData;
			A3D_INITIALIZE_DATA(A3DAsmPartDefinitionData, sPartData);
			status = A3DAsmPartDefinitionGet(pPart, &sPartData);

			A3DRiRepresentationItem** ppRepItem = new A3DRiRepresentationItem * [sPartData.m_uiRepItemsSize];

			for (A3DUns32 ui = 0; ui < sPartData.m_uiRepItemsSize; ui++)
				ppRepItem[ui] = sPartData.m_ppRepItems[ui];

			ppRepItem[sPartData.m_uiRepItemsSize] = (A3DRiRepresentationItem*)pRiBrepModel;

			sPartData.m_uiRepItemsSize++;
			sPartData.m_ppRepItems = ppRepItem;

			status = A3DAsmPartDefinitionEdit(&sPartData, pPart);

			A3DAsmProductOccurrenceGet(NULL, &sPoData);
		}
		else
		{
			// Add PO to the existing assembly model 
			A3DAsmProductOccurrence** ppPO = new A3DAsmProductOccurrence * [sPoData.m_uiPOccurrencesSize + 1];
			for (A3DUns32 ui = 0; ui < sPoData.m_uiPOccurrencesSize; ui++)
				ppPO[ui] = sPoData.m_ppPOccurrences[ui];

			// Make copy PO of block
			A3DAsmModelFileData sBlockData;
			A3D_INITIALIZE_DATA(A3DAsmModelFileData, sBlockData);
			status = A3DAsmModelFileGet(pBlockModelFile, &sBlockData);

			A3DAsmProductOccurrence* pCopyPO;
			status = A3DAsmProductOccurrenceDeepCopy(sBlockData.m_ppPOccurrences[0], &pCopyPO);

			ppPO[sPoData.m_uiPOccurrencesSize] = pCopyPO;
			sPoData.m_uiPOccurrencesSize++;
			sPoData.m_ppPOccurrences = ppPO;

			status = A3DAsmProductOccurrenceEdit(&sPoData, pPO);
		}

		A3DAsmModelFileGet(NULL, &sData);
	}
}

bool ExProcess::CreateSolid(const SolidShape solidShape, const double* in_size, const double* in_offset, const double* in_dir, A3DAsmModelFile*& pNewModelFile)
{
	// Create a block model using Parasolid
	PK_BODY_t body;
	
	if (!m_pPsProcess->CreateSolid(solidShape, in_size, in_offset, in_dir, body))
		return false;

	addBody(body, pNewModelFile);

	return true;
}

bool ExProcess::BlendRC(const PsBlendType blendType, const double blendR, const double blendC2, A3DRiBrepModel* pRiBrepModel, 
	const int edgeCnt, A3DTopoEdge** ppTopoEdges, A3DTopoFace** ppTopoFaces)
{
	A3DStatus status;
	
	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return false;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	// Get Parasolid edge Tag
	PK_EDGE_t* edges = new PK_EDGE_t[edgeCnt];
	PK_FACE_t* faces = new PK_EDGE_t[edgeCnt];
	for (int i = 0; i < edgeCnt; i++)
	{
		PK_ENTITY_t* entities;
		int iNbA3DEntities;
		status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, ppTopoEdges[i], &iNbA3DEntities, &entities);
	
		if (A3D_SUCCESS != status || 1 != iNbA3DEntities)
			return false;

		edges[i] = entities[0];

		if (PsBlendType::C == blendType)
		{
			status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, ppTopoFaces[i], &iNbA3DEntities, &entities);
			
			if (A3D_SUCCESS != status || 1 != iNbA3DEntities)
				return false;

			faces[i] = entities[0];
		}
	}

	if (!m_pPsProcess->BlendRC(blendType, blendR, blendC2, body, edgeCnt, edges, faces))
		return false;

	// Update Exchange Brep
	if (updatePkBodyToA3DRiBrepModel(body, pRiBrepModel))
		return true;

	return false;
}

bool ExProcess::Hollow(const double thisckness, A3DRiBrepModel* pRiBrepModel, const int faceCnt, A3DTopoFace** ppTopoFaces)
{
	A3DStatus status;

	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return false;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	// Get Parasolid edge Tag
	PK_FACE_t* faces = new PK_FACE_t[faceCnt];

	for (int i = 0; i < faceCnt; i++)
	{
		int iNbA3DEntities;
		PK_ENTITY_t* entities;
		status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, ppTopoFaces[i], &iNbA3DEntities, &entities);

		if (A3D_SUCCESS != status || 1 != iNbA3DEntities)
			return false;

		faces[i] = entities[0];
	}

	if (!m_pPsProcess->Hollow(thisckness, body, faceCnt, faces))
		return false;

	// Update Exchange Brep
	if (!updatePkBodyToA3DRiBrepModel(body, pRiBrepModel))
		return false;

	return true;
}

bool ExProcess::DeleteBody(A3DRiBrepModel* pRiBrepModel)
{
	A3DStatus status;
	
	// Delete bofy of Parasolid
	PK_BODY_t body = GetEntityTag(pRiBrepModel, NULL, false);

	if (0 < body)
		m_pPsProcess->DeleteBody(body);

	return true;
}

bool ExProcess::DeletePart(A3DAsmProductOccurrence* pTargetPO)
{
	A3DStatus status;

	return true;
}

bool ExProcess::DeleteFaces(A3DRiBrepModel* pRiBrepModel, const int faceCnt, A3DTopoEdge** ppTopoFaces)
{
	A3DStatus status;

	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return false;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	// Get Parasolid edge Tag
	PK_FACE_t* faces = new PK_FACE_t[faceCnt];

	for (int i = 0; i < faceCnt; i++)
	{
		int iNbA3DEntities;
		PK_ENTITY_t* entities;
		status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, ppTopoFaces[i], &iNbA3DEntities, &entities);

		if (A3D_SUCCESS != status || 1 != iNbA3DEntities)
			return false;

		faces[i] = entities[0];
	}

	if (!m_pPsProcess->DeleteFace(faceCnt, faces))
		return false;

	// Update Exchange Brep
	if (!updatePkBodyToA3DRiBrepModel(body, pRiBrepModel))
		return false;

	return true;
}

bool ExProcess::Boolean(const PsBoolType boolType, A3DRiBrepModel* pTargetBrep, const int toolCnt, A3DRiBrepModel** ppToolBreps)
{
	// Get Parasolid body tag
	PK_BODY_t targetBody = getPkBodyFromRiBrepModel(pTargetBrep);
	std::vector<PK_BODY_t> toolBodyArr;
	for (int i = 0; i < toolCnt; i++)
		toolBodyArr.push_back(getPkBodyFromRiBrepModel(ppToolBreps[i]));

	if (0 == targetBody || 0 == toolCnt)
		return false;

	int resBodyCnt = 0;
	PK_BODY_t* resBodies;
	if (m_pPsProcess->Boolean(boolType, targetBody, toolCnt, toolBodyArr.data(), resBodyCnt, resBodies))
	{
		for (int i = 0; i < resBodyCnt; i++)
		{
			PK_BODY_t resBody = resBodies[i];
			if (targetBody == resBody)
			{
				updatePkBodyToA3DRiBrepModel(resBody, pTargetBrep);
			}
			else
			{
				A3DAsmModelFile* pModelFile;
				addBody(resBody, pModelFile);
			}
		}

		for (int i = 0; i < toolCnt; i++)
			DeleteBody(ppToolBreps[i]);
		return true;
	}

	return false;
}

bool ExProcess::FR(PsFRType frType, A3DRiBrepModel* pRiBrepModel, A3DTopoFace* pTopoFace, std::vector<A3DEntity*>& entityArr)
{
	A3DStatus status;
	
	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return false;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	// Get Parasolid face Tag
	PK_FACE_t* faces;
	int iNbA3DEntities = 0;
	status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, pTopoFace, &iNbA3DEntities, &faces);

	if (A3D_SUCCESS != status || 0 == iNbA3DEntities)
		return false;

	std::vector<PK_FACE_t> pkFaceArr;
	bool bRet = m_pPsProcess->FR(frType, faces[0], pkFaceArr);

	if (!bRet)
		return false;

	for (size_t i = 0; i < pkFaceArr.size(); i++)
	{
		PK_FACE_t face = pkFaceArr[i];
		A3DEntity** pEntities;
		status = A3DMiscPKMapperGetA3DEntitiesFromPKEntity(pPkMapper, face, &iNbA3DEntities, &pEntities);

		if (A3D_SUCCESS == status || 1 == iNbA3DEntities)
			entityArr.push_back(pEntities[0]);
	}

	return true;
}

int ExProcess::GetEntityTag(A3DRiBrepModel* pRiBrepModel, A3DEntity* pEntity, bool translateIfNotThere)
{
	A3DStatus status;
	
	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel, translateIfNotThere);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return 0;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	if (NULL != pEntity)
	{
		// Get Parasolid face / edge Tag
		int iNbA3DEntities;
		PK_ENTITY_t* entities;
		status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, pEntity, &iNbA3DEntities, &entities);

		if (A3D_SUCCESS == status && 0 < iNbA3DEntities)
			return entities[0];
	}
	else
		return body;

	return 0;
}

bool ExProcess::MirrorBody(A3DRiBrepModel* pRiBrepModel, const double* location, const double* normal, const double isCopy, const double isMerge)
{
	A3DStatus status;

	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return false;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	PK_BODY_t mirror_body = PK_ENTITY_null;
	if (!m_pPsProcess->MirrorBody(body, location, normal, isCopy, isMerge, mirror_body))
		return false;

	if (isCopy && !isMerge)
	{
		if (PK_ENTITY_null != mirror_body)
		{
			A3DAsmModelFile* pNewModelFile;
			addBody(mirror_body, pNewModelFile);
		}
	}

	// Update Exchange Brep
	if (!updatePkBodyToA3DRiBrepModel(body, pRiBrepModel))
		return false;

	return true;
}

bool ExProcess::GetPlaneInfo(A3DRiBrepModel* pRiBrepModel, A3DTopoFace* pTopoFace, double* position, double* normal)
{
	A3DStatus status;
	
	// Get Parasolid body tag
	PK_BODY_t body = getPkBodyFromRiBrepModel(pRiBrepModel);
	A3DMiscPKMapper* pPkMapper = NULL;

	if (0 == body)
		return false;
	else
		pPkMapper = m_mapperMap[pRiBrepModel];

	// Get Parasolid face Tag
	PK_FACE_t* faces;
	int iNbA3DEntities = 0;
	status = A3DMiscPKMapperGetPKEntitiesFromA3DEntity(pPkMapper, pTopoFace, &iNbA3DEntities, &faces);

	if (A3D_SUCCESS != status || 0 == iNbA3DEntities)
		return false;

	bool bRet = m_pPsProcess->GetPlaneInfo(faces[0], position, normal);

}