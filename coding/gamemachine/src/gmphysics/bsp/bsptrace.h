#ifndef __BSPTRACE_H__
#define __BSPTRACE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "bspphysicsstructs.h"
BEGIN_NS

#define MAX_TOUCHED_ENTITY_NUM 1024

struct BSPTracePlane : BSPPlane
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

struct BSPTraceResult
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

struct BSPSphere
{
	bool use;
	GMfloat radius;
	GMfloat halfheight;
	linear_math::Vector3 offset;
};

class BSPPhysicsWorld;
class EntityObject;
struct BSPTracePrivate
{
	BSPTracePrivate() : checkcount(0) {}
	BSPData* bsp;
	std::map<GMint, std::set<BSPEntity*> >* entities;
	std::map<BSPEntity*, EntityObject*>* entityObjects;
	BSPPhysicsWorld* p_world;
	GMint checkcount;
};

struct BSP_Physics_Brush;
struct BSP_Physics_Patch;
struct BSPPatchCollide;
struct BSPTraceWork;
class BSPTrace
{
	DECLARE_PRIVATE(BSPTrace)

public:
	void initTrace(BSPData* bsp, std::map<GMint, std::set<BSPEntity*> >* entities, std::map<BSPEntity*, EntityObject*>* entityObjects, BSPPhysicsWorld* world);
	void trace(const linear_math::Vector3& start, const linear_math::Vector3& end, const linear_math::Vector3& origin, const linear_math::Vector3& min, const linear_math::Vector3& max, REF BSPTraceResult& trace);
	void traceThroughTree(BSPTraceWork& tw, GMint num, GMfloat p1f, GMfloat p2f, const linear_math::Vector3& p1, const linear_math::Vector3& p2);
	void traceThroughLeaf(BSPTraceWork& tw, BSPLeaf* leaf);
	void traceThroughBrush(BSPTraceWork& tw, BSP_Physics_Brush* brush);
	void traceThroughPatch(BSPTraceWork& tw, BSP_Physics_Patch* patch);
	void traceEntityThroughLeaf(BSPTraceWork& tw, std::set<BSPEntity*>& entity);
	void traceThroughPatchCollide(BSPTraceWork& tw, BSPPatchCollide* pc);
	void tracePointThroughPatchCollide(BSPTraceWork& tw, const BSPPatchCollide *pc);
	GMint checkFacetPlane(const linear_math::Vector4& plane, const linear_math::Vector3& start, const linear_math::Vector3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint *hit);
	bool boundsIntersect(const linear_math::Vector3& mins, const linear_math::Vector3& maxs, const linear_math::Vector3& mins2, const linear_math::Vector3& maxs2);
};

END_NS
#endif