#ifndef __BSPTRACE_H__
#define __BSPTRACE_H__
#include <gmcommon.h>
#include <extensions/bsp/gmbsp.h>
#include "gmbspphysicsstructs.h"
BEGIN_NS

#define MAX_TOUCHED_ENTITY_NUM 1024

GM_ALIGNED_STRUCT_FROM(BSPTracePlane, BSPPlane)
{
	GMbyte planeType = 0;
	GMint32 signbits = 0;

	BSPTracePlane& operator =(const BSPTracePlane& p)
	{
		BSPPlane::operator=(p);

		this->planeType = p.planeType;
		this->signbits = p.signbits;
		return *this;
	}

	BSPTracePlane& operator =(const BSPPlane& p)
	{
		BSPPlane::operator=(p);
		return *this;
	}
};

GM_ALIGNED_STRUCT(BSPTraceResult)
{
	bool allsolid = false; // if true, plane is not valid
	bool startsolid = false; // if true, the initial point was in a solid area
	GMfloat fraction = 0; // time completed, 1.0 = didn't hit anything
	GMVec3 endpos = Zero<GMVec3>(); // final position
	BSPTracePlane plane; // surface normal at impact, transformed to world space
	GMint32 surfaceFlags = 0; // surface hit
	GMint32 contents = 0; // contents on other side of surface hit

	GMint32 entityNum = 0;
	GMint32 entities[MAX_TOUCHED_ENTITY_NUM]{ 0 };
};

GM_ALIGNED_STRUCT(BSPSphere)
{
	GMVec3 offset = Zero<GMVec3>();
	GMfloat radius = false;
	GMfloat halfheight = false;
	bool use = false;
};

class GMBSPPhysicsWorld;
class GMEntityObject;
GM_PRIVATE_OBJECT(GMBSPTrace)
{
	BSPData* bsp;
	Map<GMint32, Set<GMBSPEntity*> >* entities;
	Map<GMBSPEntity*, GMEntityObject*>* entityObjects;
	GMBSPPhysicsWorld* world;
	GMint32 checkcount = 0;
};

struct GMBSP_Physics_Brush;
struct GMBSP_Physics_Patch;
struct GMBSPPatchCollide;
struct GMBSPTraceWork;
class GMBSPTrace : public GMObject
{
	GM_DECLARE_PRIVATE(GMBSPTrace)

public:
	void initTrace(BSPData* bsp, Map<GMint32, Set<GMBSPEntity*> >* entities, Map<GMBSPEntity*, GMEntityObject*>* entityObjects, GMBSPPhysicsWorld* world);
	void trace(const GMVec3& start, const GMVec3& end, const GMVec3& origin, const GMVec3& min, const GMVec3& max, REF BSPTraceResult& trace);
	void traceThroughTree(GMBSPTraceWork& tw, GMint32 num, GMfloat p1f, GMfloat p2f, const GMVec3& p1, const GMVec3& p2);
	void traceThroughLeaf(GMBSPTraceWork& tw, GMBSPLeaf* leaf);
	void traceThroughBrush(GMBSPTraceWork& tw, GMBSP_Physics_Brush* brush);
	void traceThroughPatch(GMBSPTraceWork& tw, GMBSP_Physics_Patch* patch);
	void traceThroughPatchCollide(GMBSPTraceWork& tw, GMBSPPatchCollide* pc);
	void tracePointThroughPatchCollide(GMBSPTraceWork& tw, const GMBSPPatchCollide *pc);
	GMint32 checkFacetPlane(const GMVec4& plane, const GMVec3& start, const GMVec3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint32 *hit);
	bool boundsIntersect(const GMVec3& mins, const GMVec3& maxs, const GMVec3& mins2, const GMVec3& maxs2);
};

END_NS
#endif