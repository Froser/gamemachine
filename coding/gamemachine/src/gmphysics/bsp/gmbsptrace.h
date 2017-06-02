#ifndef __BSPTRACE_H__
#define __BSPTRACE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "gmbspphysicsstructs.h"
BEGIN_NS

#define MAX_TOUCHED_ENTITY_NUM 1024

GM_ALIGNED_STRUCT_FROM(BSPTracePlane, BSPPlane)
{
	GMbyte planeType;
	GMint signbits;

	BSPTracePlane& operator =(BSPPlane& p)
	{
		this->normal = p.normal;
		this->intercept = p.intercept;
		return *this;
	}
};

GM_ALIGNED_STRUCT(BSPTraceResult)
{
	bool allsolid; // if true, plane is not valid
	bool startsolid; // if true, the initial point was in a solid area
	GMfloat fraction; // time completed, 1.0 = didn't hit anything
	linear_math::Vector3 endpos; // final position
	BSPTracePlane plane; // surface normal at impact, transformed to world space
	GMint surfaceFlags; // surface hit
	GMint contents; // contents on other side of surface hit

	GMint entityNum;
	GMint entities[MAX_TOUCHED_ENTITY_NUM];
};

GM_ALIGNED_STRUCT(BSPSphere)
{
	linear_math::Vector3 offset;
	GMfloat radius;
	GMfloat halfheight;
	bool use;
};

class GMBSPPhysicsWorld;
class GMEntityObject;
GM_PRIVATE_OBJECT(GMBSPTrace)
{
	GM_PRIVATE_CONSTRUCT(GMBSPTrace) : checkcount(0) {}
	BSPData* bsp;
	std::map<GMint, std::set<BSPEntity*> >* entities;
	std::map<BSPEntity*, GMEntityObject*>* entityObjects;
	GMBSPPhysicsWorld* world;
	GMint checkcount;
};

struct GMBSP_Physics_Brush;
struct GMBSP_Physics_Patch;
struct GMBSPPatchCollide;
struct GMBSPTraceWork;
class GMBSPTrace
{
	DECLARE_PRIVATE(GMBSPTrace)

public:
	void initTrace(BSPData* bsp, std::map<GMint, std::set<BSPEntity*> >* entities, std::map<BSPEntity*, GMEntityObject*>* entityObjects, GMBSPPhysicsWorld* world);
	void trace(const linear_math::Vector3& start, const linear_math::Vector3& end, const linear_math::Vector3& origin, const linear_math::Vector3& min, const linear_math::Vector3& max, REF BSPTraceResult& trace);
	void traceThroughTree(GMBSPTraceWork& tw, GMint num, GMfloat p1f, GMfloat p2f, const linear_math::Vector3& p1, const linear_math::Vector3& p2);
	void traceThroughLeaf(GMBSPTraceWork& tw, BSPLeaf* leaf);
	void traceThroughBrush(GMBSPTraceWork& tw, GMBSP_Physics_Brush* brush);
	void traceThroughPatch(GMBSPTraceWork& tw, GMBSP_Physics_Patch* patch);
	void traceEntityThroughLeaf(GMBSPTraceWork& tw, std::set<BSPEntity*>& entity);
	void traceThroughPatchCollide(GMBSPTraceWork& tw, GMBSPPatchCollide* pc);
	void tracePointThroughPatchCollide(GMBSPTraceWork& tw, const GMBSPPatchCollide *pc);
	GMint checkFacetPlane(const linear_math::Vector4& plane, const linear_math::Vector3& start, const linear_math::Vector3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint *hit);
	bool boundsIntersect(const linear_math::Vector3& mins, const linear_math::Vector3& maxs, const linear_math::Vector3& mins2, const linear_math::Vector3& maxs2);
};

END_NS
#endif