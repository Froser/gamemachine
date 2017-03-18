#ifndef __BSPTRACE_H__
#define __BSPTRACE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "bspphysicsstructs.h"
BEGIN_NS

struct BSPTracePlane : BSPPlane
{
	GMbyte planeType;
	GMint signbits;

	BSPTracePlane& operator =(BSP_Physics_Plane& p)
	{
		this->planeType = p.planeType;
		this->signbits = p.signbits;
		this->normal = p.plane->normal;
		this->intercept = p.plane->intercept;
		return *this;
	}
};

struct BSPTraceResult
{
	bool allsolid; // if true, plane is not valid
	bool startsolid; // if true, the initial point was in a solid area
	float fraction; // time completed, 1.0 = didn't hit anything
	vmath::vec3 endpos; // final position
	BSPTracePlane plane; // surface normal at impact, transformed to world space
	int surfaceFlags; // surface hit
	int contents; // contents on other side of surface hit
	int entityNum; // entity the contacted sirface is a part of
};

struct BSPSphere
{
	bool use;
	GMfloat radius;
	GMfloat halfheight;
	vmath::vec3 offset;
};

struct BSPTraceWork
{
	vmath::vec3 start;
	vmath::vec3 end;
	vmath::vec3 size[2];	// size of the box being swept through the model
	vmath::vec3 offsets[8];	// 表示一个立方体的8个顶点，[signbits][x] = size[0][x] 或 size[1][x]
	GMfloat maxOffset;	// longest corner length from origin
	vmath::vec3 extents;	// greatest of abs(size[0]) and abs(size[1])
	vmath::vec3 bounds[2];	// enclosing box of start and end surrounding by size
	vmath::vec3 modelOrigin;// origin of the model tracing through
	GMint contents; // ored contents of the model tracing through
	bool isPoint; // optimized case
	BSPTraceResult trace; // returned from trace call
	BSPSphere sphere; // sphere for oriendted capsule collision
};

class BSPPhysicsWorld;
struct BSPTracePrivate
{
	BSPData* bsp;
	BSPPhysicsWorld* p_world;
	GMint checkcount;
};

struct BSP_Physics_Brush;
struct BSP_Physics_Patch;
struct BSPPatchCollide;
class BSPTrace
{
	DEFINE_PRIVATE(BSPTrace)

public:
	void initTrace(BSPData* bsp, BSPPhysicsWorld* world);
	void trace(const vmath::vec3& start, const vmath::vec3& end, const vmath::vec3& origin, const vmath::vec3& min, const vmath::vec3& max, REF BSPTraceResult& trace);
	void traceThroughTree(BSPTraceWork& tw, GMint num, GMfloat p1f, GMfloat p2f, vmath::vec3 p1, vmath::vec3 p2);
	void traceThroughLeaf(BSPTraceWork& tw, BSPLeaf* leaf);
	void traceThroughBrush(BSPTraceWork& tw, BSP_Physics_Brush* brush);
	void traceThroughPatch(BSPTraceWork& tw, BSP_Physics_Patch* patch);
	void traceThroughPatchCollide(BSPTraceWork& tw, BSPPatchCollide* pc);
	void tracePointThroughPatchCollide(BSPTraceWork& tw, const BSPPatchCollide *pc);
	GMint checkFacetPlane(const vmath::vec4& plane, const vmath::vec3& start, const vmath::vec3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint *hit);
};

END_NS
#endif