#pragma once
#include <A3DSDKIncludes.h>
#include "PsProcess.h"
#include <map>

class ExProcess
{
public:
	ExProcess();
	~ExProcess();

private:
	A3DAsmModelFile* m_pModelFile;
	PsProcess* m_pPsProcess;
	std::map<A3DRiBrepModel*, PK_BODY_t> m_bodyMap;
	std::map<A3DRiBrepModel*, A3DMiscPKMapper*> m_mapperMap;

	PK_BODY_t getPkBodyFromRiBrepModel(A3DRiBrepModel* pRiBrepModel, bool translateIfNotThere = true);
	bool updatePkBodyToA3DRiBrepModel(PK_BODY_t inBody, A3DRiBrepModel* in_pRiBrepModel);
	void addBody(PK_BODY_t body, A3DAsmModelFile*& pNewModelFile);

public:
	bool m_bIsPart;
	
	void Initialize();
	void SetModelFile(A3DAsmModelFile* pModelFile);
	bool CreateSolid(const SolidShape solidShape, const double *in_size, const double *in_offset, const double* in_dir, A3DAsmModelFile*& pNewModelFile);
	bool BlendRC(const PsBlendType blendType, const double blendR, const double blendC2, A3DRiBrepModel* pRiBrepModel, 
		const int edgeCnt, A3DTopoEdge** ppTopoEdges, A3DTopoFace** ppTopoFaces);
	bool Hollow(const double thisckness, A3DRiBrepModel* pRiBrepModel, const int faceCnt, A3DTopoFace** ppTopoFaces);
	bool DeleteBody(A3DRiBrepModel* pRiBrepModel);
	bool DeletePart(A3DAsmProductOccurrence* pTargetPO);
	bool DeleteFaces(A3DRiBrepModel* pRiBrepModel, const int faccCnt, A3DTopoEdge** ppTopoFaces);
	bool Boolean(const PsBoolType, A3DRiBrepModel* pTargetBrep, const int toolCnt, A3DRiBrepModel** ppToolBreps);
	bool FR(const PsFRType frType, A3DRiBrepModel* pRiBrepModel, A3DTopoFace* pTopoFace, std::vector<A3DEntity*> &entityArr);
	int GetEntityTag(A3DRiBrepModel* pRiBrepModel, A3DEntity* pEntity, bool translateIfNotThere = true);
	bool MirrorBody(A3DRiBrepModel* pRiBrepModel, const double* location, const double* normal, const double isCopy, const double isMerge);
	bool GetPlaneInfo(A3DRiBrepModel* pRiBrepModel, A3DTopoFace* pTopoFace, double* position, double* normal);

};

