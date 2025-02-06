#pragma once
#include "A3DSDKIncludes.h"
#include "parasolid_kernel.h"
#include <map>

#ifdef USING_EXCHANGE
#include "sprk_exchange.h"
#endif

enum PsBoolType
{
	UNITE,
	SUBTRACT,
	INTERSECT
};

enum PsFRType
{
	BOSS,
	CONCENTRIC,
	COPLANAR
};

enum SolidShape
{
	BLOCK,
	CYLINDER,
	CONE,
	SPHERE,
	SPRING
};

enum PsBlendType
{
	R,
	C
};

class PsProcess
{
public:
	PsProcess();
	~PsProcess();

private:
	const double m_dUnit = 1000.0;
	const double m_dTol = 1.0e-8;
	PK_PARTITION_t m_partition;

	PK_ASSEMBLY_t findTopAssy();
	void setBasisSet(const double* in_offset, const double* in_dir, PK_AXIS2_sf_s& basis_set);
	bool createBlock(const double* in_size, const double* in_offset, const double* in_dir, PK_BODY_t& body);
	bool createCylinder(const double rad, const double height, const double* in_offset, const double* in_dir, PK_BODY_t& body);
	bool createCone(const double rad, const double height, const double in_angle, const double* in_offset, const double* in_dir, PK_BODY_t& body);
	bool createSphere(const double rad, const double* in_offset, const double* in_dir, PK_BODY_t& body);
	bool createSpring(const double in_outDia, const double in_L, const double in_wireDia, const double in_hook_ang, PK_BODY_t& body);
	void setPartName(const PK_PART_t in_part, const char* in_value);
	void setPartColor(const PK_PART_t in_part, const double* in_color);
	bool FR_BOSS(const PK_BODY_t, const PK_FACE_t face, std::vector<PK_ENTITY_t>& entityArr);
	bool FR_CONCENTRIC(const PK_BODY_t, const PK_FACE_t face, std::vector<PK_ENTITY_t>& entityArr);
	bool FR_COPLANAR(const PK_BODY_t, const PK_FACE_t face, std::vector<PK_ENTITY_t>& entityArr);

public:
	void Initialize();
	bool Save();
	bool BlendRC(const PsBlendType blendType, const double inBlendR, const double blendC2, const PK_BODY_t body, 
		const int edgeCnt, const PK_EDGE_t *edges, const PK_FACE_t* faces);
	bool Hollow(const double thisckness, const PK_BODY_t body, const int faceCnt, const PK_FACE_t* pierceFaces);
	bool CreateSolid(const SolidShape solidShape, const double* in_size, const double* in_offset, const double* in_dir, PK_BODY_t& body);
	bool DeleteBody(const PK_BODY_t body);
	bool DeleteFace(const int faceCnt, const PK_FACE_t* faces);
	bool Boolean(const PsBoolType boolType, const PK_BODY_t targetBody, const int toolCnt, const PK_BODY_t* toolBodies, int& bodyCnt, PK_BODY_t*& bodies);
	bool FR(const PsFRType frType, const PK_FACE_t face, std::vector<PK_ENTITY_t>& pkFaceArr);
	bool MirrorBody(const PK_BODY_t body, const double* location, const double* normal, const double isCopy, const double isMerge, PK_BODY_t& mirror_body);
	bool GetPlaneInfo(const PK_FACE_t face, double* position, double* normal);
};

