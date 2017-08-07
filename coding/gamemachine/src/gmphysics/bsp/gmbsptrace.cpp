#include "stdafx.h"
#include "gmbsptrace.h"
#include "foundation/linearmath.h"
#include "gmbspphysicsworld.h"
#include "gmengine/gmgameobject.h"

// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
#define	SURFACE_CLIP_EPSILON	(0.125)

BEGIN_NS
GM_ALIGNED_STRUCT(GMBSPTraceWork)
{
	linear_math::Vector3 start;
	linear_math::Vector3 end;
	linear_math::Vector3 size[2];	// size of the box being swept through the model
	linear_math::Vector3 offsets[8];	// 表示一个立方体的8个顶点，[signbits][x] = size[0][x] 或 size[1][x]
	GMfloat maxOffset;	// longest corner length from origin
	linear_math::Vector3 extents;	// greatest of abs(size[0]) and abs(size[1])
	linear_math::Vector3 bounds[2];	// enclosing box of start and end surrounding by size
	linear_math::Vector3 modelOrigin;// origin of the model tracing through
	GMint contents; // ored contents of the model tracing through
	bool isPoint; // optimized case
	BSPTraceResult trace; // returned from trace call
	BSPSphere sphere; // sphere for oriendted capsule collision
};
END_NS

void GMBSPTrace::initTrace(BSPData* bsp, std::map<GMint, std::set<GMBSPEntity*> >* entities, std::map<GMBSPEntity*, GMEntityObject*>* entityObjects, GMBSPPhysicsWorld* world)
{
	D(d);
	d->bsp = bsp;
	d->world = world;
	d->entities = entities;
	d->entityObjects = entityObjects;
}

/* 获得碰撞状态
start: 物体开始位置
end: 物体结束位置
origin: 坐标系原点，一般为(0,0,0)
min: 物体包围盒最小向量
max: 物体包围盒最大向量
trace: 返回的碰撞跟踪结果
*/
void GMBSPTrace::trace(const linear_math::Vector3& start, const linear_math::Vector3& end, const linear_math::Vector3& origin, const linear_math::Vector3& min, const linear_math::Vector3& max, REF BSPTraceResult& trace)
{
	D(d);
	BSPData& bsp = *d->bsp;
	d->checkcount++;

	GMBSPTraceWork tw;
	memset(&tw, 0, sizeof(tw));
	tw.trace.fraction = 1;
	tw.modelOrigin = origin;

	if (!bsp.numnodes)
	{
		trace = tw.trace;
		return;	// map not loaded, shouldn't happen
	}

	tw.contents = 1; //TODO brushmask

	linear_math::Vector3 offset = (min + max) * 0.5;
	tw.size[0] = min - offset;
	tw.size[1] = max - offset;
	tw.start = start + offset;
	tw.end = end + offset;

	tw.maxOffset = tw.size[1][0] + tw.size[1][1] + tw.size[1][2];

	// tw.offsets[signbits] = vector to appropriate corner from origin
	// 以原点为中心，offsets[8]表示立方体的8个顶点
	// 使用offsets[signbits]可以找到3个平面相交的那个角
	tw.offsets[0][0] = tw.size[0][0];
	tw.offsets[0][1] = tw.size[0][1];
	tw.offsets[0][2] = tw.size[0][2];

	tw.offsets[1][0] = tw.size[1][0];
	tw.offsets[1][1] = tw.size[0][1];
	tw.offsets[1][2] = tw.size[0][2];

	tw.offsets[2][0] = tw.size[0][0];
	tw.offsets[2][1] = tw.size[1][1];
	tw.offsets[2][2] = tw.size[0][2];

	tw.offsets[3][0] = tw.size[1][0];
	tw.offsets[3][1] = tw.size[1][1];
	tw.offsets[3][2] = tw.size[0][2];

	tw.offsets[4][0] = tw.size[0][0];
	tw.offsets[4][1] = tw.size[0][1];
	tw.offsets[4][2] = tw.size[1][2];

	tw.offsets[5][0] = tw.size[1][0];
	tw.offsets[5][1] = tw.size[0][1];
	tw.offsets[5][2] = tw.size[1][2];

	tw.offsets[6][0] = tw.size[0][0];
	tw.offsets[6][1] = tw.size[1][1];
	tw.offsets[6][2] = tw.size[1][2];

	tw.offsets[7][0] = tw.size[1][0];
	tw.offsets[7][1] = tw.size[1][1];
	tw.offsets[7][2] = tw.size[1][2];

	if (tw.sphere.use) {
		for (GMint i = 0; i < 3; i++) {
			if (tw.start[i] < tw.end[i]) {
				tw.bounds[0][i] = tw.start[i] - fabs(tw.sphere.offset[i]) - tw.sphere.radius;
				tw.bounds[1][i] = tw.end[i] + fabs(tw.sphere.offset[i]) + tw.sphere.radius;
			}
			else {
				tw.bounds[0][i] = tw.end[i] - fabs(tw.sphere.offset[i]) - tw.sphere.radius;
				tw.bounds[1][i] = tw.start[i] + fabs(tw.sphere.offset[i]) + tw.sphere.radius;
			}
		}
	}
	else
	{
		for (GMint i = 0; i < 3; i++) {
			if (tw.start[i] < tw.end[i]) {
				tw.bounds[0][i] = tw.start[i] + tw.size[0][i];
				tw.bounds[1][i] = tw.end[i] + tw.size[1][i];
			}
			else {
				tw.bounds[0][i] = tw.end[i] + tw.size[0][i];
				tw.bounds[1][i] = tw.start[i] + tw.size[1][i];
			}
		}
	}

	if (linear_math::equals(start, end))
	{
		/*
		if (model) {
			if (model == CAPSULE_MODEL_HANDLE) {
				if (tw.sphere.use) {
					CM_TestCapsuleInCapsule(&tw, model);
				}
				else {
					CM_TestBoundingBoxInCapsule(&tw, model);
				}
			}
			else {
				CM_TestInLeaf(&tw, &cmod->leaf);
			}
		}
		else
			positionTest(tw);
		*/
	}
	else
	{
		if (tw.size[0][0] == 0 && tw.size[0][1] == 0 && tw.size[0][2] == 0)
		{
			tw.isPoint = true;
			tw.extents = linear_math::Vector3(0);
		}
		else {
			tw.isPoint = false;
			tw.extents = tw.size[1];
		}

		traceThroughTree(tw, 0, 0, 1, tw.start, tw.end);
	}

	// generate endpos from the original, unmodified start/end
	if (tw.trace.fraction == 1)
	{
		tw.trace.endpos = end;
	}
	else
	{
		tw.trace.endpos = start + tw.trace.fraction * (end - start);
	}

	// If allsolid is set (was entirely inside something solid), the plane is not valid.
	// If fraction == 1.0, we never hit anything, and thus the plane is not valid.
	// Otherwise, the normal on the plane should have unit length
	ASSERT(tw.trace.allsolid ||
		tw.trace.fraction == 1.0 ||
		linear_math::lengthSquare(tw.trace.plane.normal) > 0.9999f);
	trace = tw.trace;
}

void GMBSPTrace::traceThroughTree(GMBSPTraceWork& tw, GMint num, GMfloat p1f, GMfloat p2f, const linear_math::Vector3& p1, const linear_math::Vector3& p2)
{
	D(d);
	BSPData& bsp = *d->bsp;
	GMBSPPhysicsWorld::Data& pw = d->world->physicsData();
	GMBSPNode* node;
	BSPTracePlane* plane;

	if (tw.trace.fraction <= p1f)
	{
		return; // already hit something nearer
	}

	// if < 0, we are in a leaf node
	if (num < 0)
	{
		traceThroughLeaf(tw, &bsp.leafs[~num]);
		traceEntityThroughLeaf(tw, (*d->entities)[~num]);
		return;
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = &bsp.nodes[num];
	plane = &pw.planes[node->planeNum];

	// t1, t2表示p1和p2与plane的垂直距离
	// 如果平面是与坐标系垂直，可以直接用p[plane->planeType]来拿距离
	GMfloat t1, t2, offset;

	GMfloat dist = plane->intercept;

	if (plane->planeType < PLANE_NON_AXIAL) {
		t1 = p1[plane->planeType] + dist;
		t2 = p2[plane->planeType] + dist;
		offset = tw.extents[plane->planeType];
	}
	else {
		t1 = linear_math::dot(plane->normal, p1) + dist;
		t2 = linear_math::dot(plane->normal, p2) + dist;
		if (tw.isPoint) {
			offset = 0;
		}
		else {
			// this is silly
			offset = 2048;
		}
	}

	// see which sides we need to consider
	if (t1 >= offset + 1 && t2 >= offset + 1) {
		traceThroughTree(tw, node->children[0], p1f, p2f, p1, p2); // 在平面前
		return;
	}
	if (t1 < -offset - 1 && t2 < -offset - 1) {
		traceThroughTree(tw, node->children[1], p1f, p2f, p1, p2); // 在平面后
		return;
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	GMfloat idist;
	GMint side;
	GMfloat frac, frac2;
	if (t1 < t2) {
		idist = 1.0 / (t1 - t2);
		side = 1;
		frac2 = (t1 + offset + SURFACE_CLIP_EPSILON)*idist;
		frac = (t1 - offset + SURFACE_CLIP_EPSILON)*idist;
	}
	else if (t1 > t2) {
		idist = 1.0 / (t1 - t2);
		side = 0;
		frac2 = (t1 - offset - SURFACE_CLIP_EPSILON)*idist;
		frac = (t1 + offset + SURFACE_CLIP_EPSILON)*idist;
	}
	else {
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if (frac < 0) {
		frac = 0;
	}
	if (frac > 1) {
		frac = 1;
	}

	GMfloat midf;
	linear_math::Vector3 mid;
	midf = p1f + (p2f - p1f)*frac;

	mid = p1 + frac*(p2 - p1);

	traceThroughTree(tw, node->children[side], p1f, midf, p1, mid);


	// go past the node
	if (frac2 < 0) {
		frac2 = 0;
	}
	if (frac2 > 1) {
		frac2 = 1;
	}

	midf = p1f + (p2f - p1f)*frac2;

	mid = p1 + frac2*(p2 - p1);

	traceThroughTree(tw, node->children[side ^ 1], midf, p2f, mid, p2);
}

void GMBSPTrace::traceThroughLeaf(GMBSPTraceWork& tw, GMBSPLeaf* leaf)
{
	D(d);
	BSPData& bsp = *d->bsp;
	GMBSPPhysicsWorld::Data& pw = d->world->physicsData();
	// trace line against all brushes in the leaf
	for (GMint k = 0; k < leaf->numLeafBrushes; k++)
	{
		GMint brushnum = bsp.leafbrushes[leaf->firstLeafBrush + k];

		GMBSP_Physics_Brush* b = &pw.brushes[brushnum];

		if (b->checkcount == d->checkcount) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = d->checkcount;

		if (!(b->contents & tw.contents)) {
			continue;
		}

		traceThroughBrush(tw, b);
		if (!tw.trace.fraction) {
			return;
		}
	}

	for (GMint k = 0; k < leaf->numLeafSurfaces; k++)
	{
		GMBSP_Physics_Patch* patch = pw.patch.patches(bsp.leafsurfaces[leaf->firstLeafSurface + k]);
		if (!patch) {
			continue;
		}
		if (patch->checkcount == d->checkcount) {
			continue;	// already checked this patch in another leaf
		}
		patch->checkcount = d->checkcount;

		if (!(patch->shader->contentFlags & tw.contents)) {
			continue;
		}

		traceThroughPatch(tw, patch);
		if (!tw.trace.fraction)
			return;
	}
}

void GMBSPTrace::traceThroughPatch(GMBSPTraceWork& tw, GMBSP_Physics_Patch* patch)
{
	GMfloat oldFrac;

	oldFrac = tw.trace.fraction;

	traceThroughPatchCollide(tw, patch->pc);

	if (tw.trace.fraction < oldFrac)
	{
		tw.trace.surfaceFlags = patch->shader->surfaceFlags;
		tw.trace.contents = patch->shader->contentFlags;
	}
}

void GMBSPTrace::traceThroughPatchCollide(GMBSPTraceWork& tw, GMBSPPatchCollide* pc)
{
	GMint j, hit, hitnum;
	GMfloat offset, enterFrac, leaveFrac, t;
	GMBSPPatchPlane* planes;
	linear_math::Vector4 plane, bestplane;
	linear_math::Vector3 startp, endp;

	if (!boundsIntersect(tw.bounds[0], tw.bounds[1],
		pc->bounds[0], pc->bounds[1])) {
		return;
	}

	if (tw.isPoint)
	{
		tracePointThroughPatchCollide(tw, pc);
		return;
	}

	for (const auto& facet_ref : pc->facets)
	{
		const GMBSPFacet* facet = &facet_ref;

		enterFrac = -1.0;
		leaveFrac = 1.0;
		hitnum = -1;
		//
		planes = &pc->planes[facet->surfacePlane];
		plane = planes->plane;
		if (tw.sphere.use) {
			// adjust the plane distance apropriately for radius
			plane[3] += tw.sphere.radius;

			// find the closest point on the capsule to the plane
			t = linear_math::dot(VEC3(plane), tw.sphere.offset);
			if (t > 0.0f) {
				startp = tw.start - tw.sphere.offset;
				endp = tw.end - tw.sphere.offset;
			}
			else {
				startp = tw.start + tw.sphere.offset;
				endp = tw.end + tw.sphere.offset;
			}
		}
		else
		{
			offset = linear_math::dot(tw.offsets[planes->signbits], VEC3(plane));
			plane[3] += offset;
			startp = tw.start;
			endp = tw.end;
		}

		if (!checkFacetPlane(plane, startp, endp, &enterFrac, &leaveFrac, &hit))
			continue;

		if (hit)
			bestplane = plane;

		for (j = 0; j < facet->numBorders; j++)
		{
			planes = &pc->planes[facet->borderPlanes[j]];
			if (facet->borderInward[j])
				plane = -planes->plane;
			else
				plane = planes->plane;

			if (tw.sphere.use)
			{
				// adjust the plane distance apropriately for radius
				plane[3] += tw.sphere.radius;

				// find the closest point on the capsule to the plane
				t = linear_math::dot(VEC3(plane), tw.sphere.offset);
				if (t > 0.0f) {
					startp = tw.start - tw.sphere.offset;
					endp = tw.end - tw.sphere.offset;
				}
				else {
					startp = tw.start + tw.sphere.offset;
					endp = tw.end + tw.sphere.offset;
				}
			}
			else
			{
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = linear_math::dot(tw.offsets[planes->signbits], VEC3(plane));
				plane[3] -= fabs(offset);
				startp = tw.start;
			}

			if (!checkFacetPlane(plane, startp, endp, &enterFrac, &leaveFrac, &hit))
				break;

			if (hit)
			{
				hitnum = j;
				bestplane = plane;
			}
		}

		if (j < facet->numBorders)
			continue;

		//never clip against the back side
		if (hitnum == facet->numBorders - 1)
			continue;

		if (enterFrac < leaveFrac && enterFrac >= 0)
		{
			if (enterFrac < tw.trace.fraction)
			{
				if (enterFrac < 0) {
					enterFrac = 0;
				}

				tw.trace.fraction = enterFrac;
				tw.trace.plane.normal = VEC3(bestplane);
				tw.trace.plane.intercept = bestplane[3];
			}
		}
	}
}

void GMBSPTrace::traceEntityThroughLeaf(GMBSPTraceWork& tw, std::set<GMBSPEntity*>& entities)
{
	D(d);
	tw.trace.entityNum = 0;
	for (auto entity : entities)
	{
		auto objIter = (d->entityObjects->find(entity));
		if (objIter != d->entityObjects->end())
		{
			GMEntityObject* obj = objIter->second;
			if (!obj)
				continue;

			for (GMint i = 0; i < EntityPlaneNum; i++)
			{
				// 首先判断起点和终点是否在AABB中，如果没有，肯定没有接触到entity
				linear_math::Vector3 mins, maxs;
				obj->getBounds(mins, maxs);
				if (!boundsIntersect(tw.bounds[0], tw.bounds[1], mins, maxs))
					continue;
			}
		}
	}
}

void GMBSPTrace::tracePointThroughPatchCollide(GMBSPTraceWork& tw, const GMBSPPatchCollide *pc)
{
	GMfloat intersect;
	GMint j, k;
	GMfloat offset;
	GMfloat d1, d2;

	AlignedVector<GMint> frontFacing;
	frontFacing.resize(pc->planes.size());
	AlignedVector<GMfloat> intersection;
	intersection.resize(pc->planes.size());

	// determine the trace's relationship to all planes
	const AlignedVector<GMBSPPatchPlane>& planes = pc->planes;
	GMint i = 0;
	for (const auto& plane : planes)
	{
		offset = linear_math::dot(tw.offsets[plane.signbits], VEC3(plane.plane));
		d1 = linear_math::dot(tw.start, VEC3(plane.plane)) - plane.plane[3] + offset;
		d2 = linear_math::dot(tw.end, VEC3(plane.plane)) - plane.plane[3] + offset;
		if (d1 <= 0)
			frontFacing[i] = 0;
		else
			frontFacing[i] = 1;

		if (d1 == d2)
		{
			intersection[i] = 99999;
		}
		else
		{
			intersection[i] = d1 / (d1 - d2);
			if (intersection[i] <= 0) {
				intersection[i] = 99999;
			}
		}
	}


	// see if any of the surface planes are intersected
	const AlignedVector<GMBSPFacet>& facets = pc->facets;
	for (const auto& facet : facets)
	{
		if (!frontFacing[facet.surfacePlane]) {
			continue;
		}
		intersect = intersection[facet.surfacePlane];
		if (intersect < 0) {
			continue;		// surface is behind the starting point
		}
		if (intersect > tw.trace.fraction) {
			continue;		// already hit something closer
		}
		for (j = 0; j < facet.numBorders; j++) {
			k = facet.borderPlanes[j];
			if (frontFacing[k] ^ facet.borderInward[j]) {
				if (intersection[k] > intersect) {
					break;
				}
			}
			else {
				if (intersection[k] < intersect) {
					break;
				}
			}
		}
		if (j == facet.numBorders)
		{
			const GMBSPPatchPlane* planes = &pc->planes[facet.surfacePlane];

			// calculate intersection with a slight pushoff
			offset = linear_math::dot(tw.offsets[planes->signbits], VEC3(planes->plane));
			d1 = linear_math::dot(tw.start, VEC3(planes->plane)) - planes->plane[3] + offset;
			d2 = linear_math::dot(tw.end, VEC3(planes->plane)) - planes->plane[3] + offset;
			tw.trace.fraction = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);

			if (tw.trace.fraction < 0)
			{
				tw.trace.fraction = 0;
			}

			tw.trace.plane.normal = VEC3(planes->plane);
			tw.trace.plane.intercept = planes->plane[3];
		}
	}
}

GMint GMBSPTrace::checkFacetPlane(const linear_math::Vector4& plane, const linear_math::Vector3& start, const linear_math::Vector3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint *hit)
{
	float d1, d2, f;

	*hit = false;

	d1 = linear_math::dot(start, linear_math::Vector3(plane[0], plane[1], plane[2])) + plane[3];
	d2 = linear_math::dot(end, linear_math::Vector3(plane[0], plane[1], plane[2])) + plane[3];

	// if completely in front of face, no intersection with the entire facet
	if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
		return false;
	}

	// if it doesn't cross the plane, the plane isn't relevent
	if (d1 <= 0 && d2 <= 0) {
		return true;
	}

	// crosses face
	if (d1 > d2) {	// enter
		f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
		if (f < 0) {
			f = 0;
		}
		//always favor previous plane hits and thus also the surface plane hit
		if (f > *enterFrac) {
			*enterFrac = f;
			*hit = true;
		}
	}
	else {	// leave
		f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
		if (f > 1) {
			f = 1;
		}
		if (f < *leaveFrac) {
			*leaveFrac = f;
		}
	}
	return true;
}

void GMBSPTrace::traceThroughBrush(GMBSPTraceWork& tw, GMBSP_Physics_Brush *brush)
{
	D(d);
	BSPData& bsp = *d->bsp;
	GMBSPPhysicsWorld::Data& pw = d->world->physicsData();

	if (!brush->brush->numSides) {
		return;
	}

	bool getout = false, startout = false;
	GMBSP_Physics_BrushSide* side = nullptr, *leadside = nullptr;
	BSPTracePlane* plane = nullptr, *clipplane = nullptr;
	GMfloat f = 0;
	GMfloat enterFrac = -1.0;
	GMfloat leaveFrac = 1.0;
	linear_math::Vector3 startp, endp;

	if (tw.sphere.use)
	{
		for (GMint i = 0; i < brush->brush->numSides; i++) {
			side = brush->sides + i;
			plane = &pw.planes[side->side->planeNum];

			// adjust the plane distance apropriately for radius
			GMfloat dist = plane->intercept - tw.sphere.radius;

			// find the closest point on the capsule to the plane
			GMfloat t = linear_math::dot(plane->normal, tw.sphere.offset);
			if (t > 0)
			{
				startp = tw.start - tw.sphere.offset;
				endp = tw.end - tw.sphere.offset;
			}
			else
			{
				startp = tw.start + tw.sphere.offset;
				endp = tw.end + tw.sphere.offset;
			}

			GMfloat d1 = linear_math::dot(startp, plane->normal) + dist;
			GMfloat d2 = linear_math::dot(endp, plane->normal) + dist;

			if (d2 > 0) {
				getout = true;	// endpoint is not in solid
			}
			if (d1 > 0) {
				startout = false;
			}

			// if completely in front of face, no intersection with the entire brush
			if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
				return;
			}

			// if it doesn't cross the plane, the plane isn't relevent
			if (d1 <= 0 && d2 <= 0) {
				continue;
			}

			// crosses face
			if (d1 > d2) {	// enter
				f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
				if (f < 0) {
					f = 0;
				}
				if (f > enterFrac) {
					enterFrac = f;
					clipplane = plane;
					leadside = side;
				}
			}
			else {	// leave
				f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
				if (f > 1) {
					f = 1;
				}
				if (f < leaveFrac) {
					leaveFrac = f;
				}
			}
		}
	}
	else
	{
		//
		// compare the trace against all planes of the brush
		// find the latest time the trace crosses a plane towards the interior
		// and the earliest time the trace crosses a plane towards the exterior
		//
		for (GMint i = 0; i < brush->brush->numSides; i++) {
			side = brush->sides + i;
			plane = &pw.planes[side->side->planeNum];

			// adjust the plane distance apropriately for mins/maxs
			GMfloat bound = linear_math::dot(tw.offsets[plane->signbits], plane->normal);
			GMfloat dist = plane->intercept + bound;

			GMfloat d1 = linear_math::dot(tw.start, plane->normal) + dist;
			GMfloat d2 = linear_math::dot(tw.end, plane->normal) + dist;

			if (d2 > 0) {
				getout = true;	// endpoint is not in solid
			}
			if (d1 > 0) {
				startout = true;
			}

			// if completely in front of face, no intersection with the entire brush
			if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
				return;
			}

			// if it doesn't cross the plane, the plane isn't relevent
			if (d1 <= 0 && d2 <= 0) {
				continue;
			}

			// crosses face
			if (d1 > d2) {	// enter
				f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
				if (f < 0) {
					f = 0;
				}
				if (f > enterFrac) {
					enterFrac = f;
					clipplane = plane;
					leadside = side;
				}
			}
			else {	// leave
				f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
				if (f > 1) {
					f = 1;
				}
				if (f < leaveFrac) {
					leaveFrac = f;
				}
			}
		}
	}

	//
	// all planes have been checked, and the trace was not
	// completely outside the brush
	//
	if (!startout) {	// original point was inside brush
		tw.trace.startsolid = true;
		if (!getout) {
			tw.trace.allsolid = true;
			tw.trace.fraction = 0;
			tw.trace.contents = brush->contents;
		}
		return;
	}

	if (enterFrac < leaveFrac) {
		if (enterFrac > -1 && enterFrac < tw.trace.fraction) {
			if (enterFrac < 0) {
				enterFrac = 0;
			}
			tw.trace.fraction = enterFrac;
			tw.trace.plane = *clipplane;
			tw.trace.surfaceFlags = leadside->surfaceFlags;
			tw.trace.contents = brush->contents;
		}
	}
}

bool GMBSPTrace::boundsIntersect(const linear_math::Vector3& mins, const linear_math::Vector3& maxs, const linear_math::Vector3& mins2, const linear_math::Vector3& maxs2)
{
	if (maxs[0] < mins2[0] - SURFACE_CLIP_EPSILON ||
		maxs[1] < mins2[1] - SURFACE_CLIP_EPSILON ||
		maxs[2] < mins2[2] - SURFACE_CLIP_EPSILON ||
		mins[0] > maxs2[0] + SURFACE_CLIP_EPSILON ||
		mins[1] > maxs2[1] + SURFACE_CLIP_EPSILON ||
		mins[2] > maxs2[2] + SURFACE_CLIP_EPSILON)
	{
		return false;
	}

	return true;
}
