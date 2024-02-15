#include "ExPsProcess.h"
#include "ExUtilities.h"

ExPsProcess::ExPsProcess()
{
	m_bIsPart = false;
	m_pPsProcess = new PsProcess();
}

ExPsProcess::~ExPsProcess()
{
}

void ExPsProcess::Initialize()
{
	m_pPsProcess->Initialize();

	if (NULL != m_pModelFile)
		m_pModelFile = NULL;

	std::vector<UpdatedComponent>().swap(m_updatedCompArr);

}

void ExPsProcess::SetModelFile(A3DAsmModelFile* pModelFile)
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

bool ExPsProcess::RegisterUpdatedBody(A3DRiBrepModel* pRiBrepModel, PK_BODY_t body)
{
	UpdatedComponent updatedComp;
	updatedComp.pTargetComp = pRiBrepModel;
	updatedComp.body = body;
	updatedComp.updateType = UpdatedType::UPDATE;
	m_updatedCompArr.push_back(updatedComp);

	return true;
}

bool ExPsProcess::RegisterDeleteBody(A3DRiBrepModel* pRiBrepModel)
{
	for (A3DUns32 ui = 0; ui < m_updatedCompArr.size(); ui++)
	{
		if (m_updatedCompArr[ui].pTargetComp == pRiBrepModel)
		{
			m_updatedCompArr[ui].updateType = UpdatedType::DELEATE_BODY;
			return true;
		}
	}

	UpdatedComponent updatedBrep;
	updatedBrep.pTargetComp = pRiBrepModel;
	updatedBrep.updateType = UpdatedType::DELEATE_BODY;
	m_updatedCompArr.push_back(updatedBrep);

	return true;
}

bool ExPsProcess::RegisterDeletePart(A3DAsmProductOccurrence* pPO)
{
	UpdatedComponent updatedComp;
	updatedComp.pTargetComp = pPO;
	updatedComp.updateType = UpdatedType::DELEATE_PART;
	m_updatedCompArr.push_back(updatedComp);

	return true;
}

bool ExPsProcess::BlendRC(const PsBlendType boolType, const double blendR, const double blendC2, 
	const int edgeCnt, PK_EDGE_t* edges, PK_FACE_t* first_faces, A3DRiBrepModel* pRiBrepModel)
{ 
	PK_ERROR_code_t error_code;

	// Get body tag
	int n_faces = 0;
	PK_FACE_t* faces = NULL;

	error_code = PK_EDGE_ask_faces(edges[0], &n_faces, &faces);

	PK_BODY_t body;
	error_code = PK_FACE_ask_body(faces[0], &body);

	if (!m_pPsProcess->BlendRC(boolType, blendR, blendC2, body, edgeCnt, edges, first_faces))
		return false;

	RegisterUpdatedBody(pRiBrepModel, body);

	return true; 
}

bool ExPsProcess::Hollow(const double thisckness, const PK_BODY_t body, const int faceCnt, const PK_FACE_t* pierceFaces, A3DRiBrepModel* pRiBrepModel)
{
	if (!m_pPsProcess->Hollow(thisckness, body, faceCnt, pierceFaces))
		return false;

	RegisterUpdatedBody(pRiBrepModel, body);

	return true;
}

bool ExPsProcess::DeleteFaces(const int faceCnt, PK_FACE_t* faces, A3DRiBrepModel* pRiBrepModel)
{
	PK_ERROR_code_t error_code;
	
	// Get body tag
	PK_BODY_t body;
	error_code = PK_FACE_ask_body(faces[0], &body);

	if (!m_pPsProcess->DeleteFace(faceCnt, faces))
		return false;
	
	RegisterUpdatedBody(pRiBrepModel, body);

	return true;
};

bool ExPsProcess::CopyAndUpdateModelFile(A3DAsmModelFile*& pCopyModelFile)
{
	A3DStatus status;
	
	// Search updated Brep ID
	{
		A3DVisitorContainer sA3DVisitorContainer(CONNECT_TRANSFO);
		sA3DVisitorContainer.SetTraverseInstance(false);

		SearchCompIdVisitor* pMyTreeVisitor = new SearchCompIdVisitor(&sA3DVisitorContainer, m_updatedCompArr);
		sA3DVisitorContainer.push(pMyTreeVisitor);

		A3DModelFileConnector sModelFileConnector(m_pModelFile);
		status = sModelFileConnector.Traverse(&sA3DVisitorContainer);

		m_updatedCompArr = pMyTreeVisitor->m_updatedComp;
	}

	// PK_BODY to RiBrepModel
	for (A3DUns32 ui = 0; ui < m_updatedCompArr.size(); ui++)
	{
		if (UpdatedType::UPDATE == m_updatedCompArr[ui].updateType)
		{
			PK_BODY_t body = m_updatedCompArr[ui].body;
			A3DRiBrepModel* pRiBrepModel;
			A3DMiscPKMapper* pPkMapper;
			if (!PkBodyToA3DRiBrepModel(body, pRiBrepModel, pPkMapper))
				return false;

			m_updatedCompArr[ui].pNewRiBrep = pRiBrepModel;
		}
		else
		{
			m_updatedCompArr[ui].pNewRiBrep = NULL;
		}
	}

	// Make a copy ModelFile
	A3DAsmModelFileData sData;
	A3D_INITIALIZE_DATA(A3DAsmModelFileData, sData);
	A3DAsmModelFileGet(m_pModelFile, &sData);

	A3DAsmProductOccurrence* pCopyPO;
	status = A3DAsmProductOccurrenceDeepCopy(sData.m_ppPOccurrences[0], &pCopyPO);

	A3DAsmModelFileData sCopyData;
	A3D_INITIALIZE_DATA(A3DAsmModelFileData, sCopyData);
	sCopyData.m_uiPOccurrencesSize = 1;
	sCopyData.m_ppPOccurrences = &pCopyPO;
	sCopyData.m_bUnitFromCAD = sData.m_bUnitFromCAD;
	sCopyData.m_dUnit = sData.m_dUnit;
	sCopyData.m_eModellerType = sData.m_eModellerType;
	sCopyData.m_pBIMData = sData.m_pBIMData;

	status = A3DAsmModelFileCreate(&sCopyData, &pCopyModelFile);

	// Update Breps of copy ModelFile
	{
		A3DVisitorContainer sA3DVisitorContainer(CONNECT_TRANSFO);
		sA3DVisitorContainer.SetTraverseInstance(false);

		UpdatedCompVisitor* pMyTreeVisitor = new UpdatedCompVisitor(&sA3DVisitorContainer, m_updatedCompArr);
		sA3DVisitorContainer.push(pMyTreeVisitor);

		A3DModelFileConnector sModelFileConnector(pCopyModelFile);
		status = sModelFileConnector.Traverse(&sA3DVisitorContainer);
	}

	return true;
}
