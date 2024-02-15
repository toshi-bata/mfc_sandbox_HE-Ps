#pragma once
#include <A3DSDKIncludes.h>
#include "PsProcess.h"
#include <vector>

#include "visitor/VisitorContainer.h"
#include "visitor/VisitorCascadedAttribute.h"
#include "visitor/VisitorTree.h"
#include "visitor/VisitorTransfo.h"
#include "visitor/CascadedAttributeConnector.h"
#include "visitor/TransfoConnector.h"
#include "UpdateCompVisitor.h"

class ExPsProcess
{
public:
	ExPsProcess();
	~ExPsProcess();

private:
	A3DAsmModelFile* m_pModelFile;
	PsProcess* m_pPsProcess;
	std::vector<UpdatedComponent> m_updatedCompArr;

public:
	bool m_bIsPart;
	bool Save() { return m_pPsProcess->Save(); };
	void Initialize();
	void SetModelFile(A3DAsmModelFile* pModelFile);
	bool RegisterUpdatedBody(A3DRiBrepModel* pRiBrepModel, PK_BODY_t body);
	bool RegisterDeleteBody(A3DRiBrepModel* pRiBrepModel);
	bool RegisterDeletePart(A3DAsmProductOccurrence* pPO);
	bool CreateSolid(const SolidShape solidShape, const double* in_size, const double* in_offset, const double *in_dir, PK_BODY_t& body) { return m_pPsProcess->CreateSolid(solidShape, in_size, in_offset, in_dir, body); };
	bool BlendRC(const PsBlendType boolType, const double blendR, const double blendC2, 
		const int edgeCnt, PK_EDGE_t* edges, PK_FACE_t* faces, A3DRiBrepModel* pRiBrepModel);
	bool Hollow(const double thisckness, const PK_BODY_t body, const int faceCnt, const PK_FACE_t* pierceFaces, A3DRiBrepModel* pRiBrepModel);
	bool DeleteFaces(const int faceCnt, PK_FACE_t* faces, A3DRiBrepModel* pRiBrepModel);
	bool Boolean(const PsBoolType boolType, const PK_BODY_t targetBody, const int toolCnt, const PK_BODY_t* toolBodies, int& bodyCnt, PK_BODY_t*& bodies) 
	{ 
		return m_pPsProcess->Boolean(boolType, targetBody, toolCnt, toolBodies, bodyCnt, bodies);
	};
	bool FR(const PsFRType frType, const PK_FACE_t face, std::vector<PK_ENTITY_t>& pkFaceArr) { return m_pPsProcess->FR(frType, face, pkFaceArr); };
	bool CopyAndUpdateModelFile(A3DAsmModelFile*& pCopyModelFile);
	bool MirrorBody(PK_BODY_t targetBody, const double* location, const double* normal, const double isCopy, const double isMerge, PK_BODY_t &mirror_body) {
		return m_pPsProcess->MirrorBody(targetBody, location, normal, isCopy, isMerge, mirror_body);
	};
	bool GetPlaneInfo(PK_FACE_t face, double* location, double* normal) {
		return m_pPsProcess->GetPlaneInfo(face, location, normal);
	};

};

