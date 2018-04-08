#include "stdafx.h"
#include "gmbsptrace.h"
#include <linearmath.h>
#include "gmbspphysicsworld.h"
#include "gmengine/gameobjects/gmgameobject.h"

// keep 1/8 unit away to keep the position valid before network snapping
// and to avoid various numeric issues
#define	SURFACE_CLIP_EPSILON	(0.125)

BEGIN_NS
GM_ALIGNED_STRUCT(GMBSPTraceWork)
{
	GMVec3 start = Zero<GMVec3>();
	GMVec3 end = Zero<GMVec3>();
	GMVec3 size[2] = { Zero<GMVec3>(), Zero<GMVec3>() };	// size of the box being swept through the model
	GMVec3 offsets[8] = { Zero<GMVec3>(), Zero<GMVec3>(), Zero<GMVec3>(), Zero<GMVec3>(), Zero<GMVec3>(), Zero<GMVec3>(), Zero<GMVec3>(), Zero<GMVec3>() };	// 表示一个立方体的8个顶点，[signbits][x] = size[0][x] 或 size[1][x]
	GMfloat maxOffset = 0;	// longest corner length from origin
	GMVec3 extents{ 0 };	// greatest of abs(size[0]) and abs(size[1])
	GMVec3 bounds[2] = { Zero<GMVec3>(), Zero<GMVec3>() };	// enclosing box of start and end surrounding by size
	GMVec3 modelOrigin = Zero<GMVec3>();// origin of the model tracing through
	GMint contents = 0; // ored contents of the model tracing through
	bool isPoint = false; // optimized case
	BSPTraceResult trace; // returned from trace call
	BSPSphere sphere; // sphere for oriendted capsule collision
};
END_NS

void GMBSPTrace::initTrace(BSPData* bsp, Map<GMint, Set<GMBSPEntity*> >* entities, Map<GMBSPEntity*, GMEntityObject*>* entityObjects, GMBSPPhysicsWorld* world)
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
void GMBSPTrace::trace(const GMVec3& start, const GMVec3& end, const GMVec3& origin, const GMVec3& min, const GMVec3& max, REF BSPTraceResult& trace)
{
	D(d);
	BSPData& bsp = *d->bsp;
	d->checkcount++;

	GMBSPTraceWork tw;
	tw.trace.fraction = 1;
	tw.modelOrigin = origin;

	if (!bsp.numnodes)
	{
		trace = tw.trace;
		return;	// map not loaded, shouldn't happen
	}

	tw.contents = 1; //TODO brushmask

	GMVec3 offset = (min + max) * 0.5f;
	tw.size[0] = min - offset;
	tw.size[1] = max - offset;
	tw.start = start + offset;
	tw.end = end + offset;

	GMFloat4 f4_size[2];
	tw.size[0].loadFloat4(f4_size[0]);
	tw.size[1].loadFloat4(f4_size[1]);
	tw.maxOffset = f4_size[1][0] + f4_size[1][1] + f4_size[1][2];

	// tw.offsets[signbits] = vector to appropriate corner from origin
	// 以原点为中心，offsets[8]表示立方体的8个顶点
	// 使用offsets[signbits]可以找到3个平面相交的那个角
	tw.offsets[0].setX(tw.size[0].getX());
	tw.offsets[0].setY(tw.size[0].getY());
	tw.offsets[0].setZ(tw.size[0].getZ());

	tw.offsets[1].setX(tw.size[1].getX());
	tw.offsets[1].setY(tw.size[0].getY());
	tw.offsets[1].setZ(tw.size[0].getZ());

	tw.offsets[2].setX(tw.size[0].getX());
	tw.offsets[2].setY(tw.size[1].getY());
	tw.offsets[2].setZ(tw.size[0].getZ());

	tw.offsets[3].setX(tw.size[1].getX());
	tw.offsets[3].setY(tw.size[1].getY());
	tw.offsets[3].setZ(tw.size[0].getZ());

	tw.offsets[4].setX(tw.size[0].getX());
	tw.offsets[4].setY(tw.size[0].getY());
	tw.offsets[4].setZ(tw.size[1].getZ());

	tw.offsets[5].setX(tw.size[1].getX());
	tw.offsets[5].setY(tw.size[0].getY());
	tw.offsets[5].setZ(tw.size[1].getZ());

	tw.offsets[6].setX(tw.size[0].getX());
	tw.offsets[6].setY(tw.size[1].getY());
	tw.offsets[6].setZ(tw.size[1].getZ());

	tw.offsets[7].setX(tw.size[1].getX());
	tw.offsets[7].setY(tw.size[1].getY());
	tw.offsets[7].setZ(tw.size[1].getZ());

	GMFloat4 f4_bounds[2], f4_start, f4_end, f4_sphere_offset;
	tw.start.loadFloat4(f4_start);
	tw.end.loadFloat4(f4_end);

	if (tw.sphere.use)
	{
		tw.sphere.offset.loadFloat4(f4_sphere_offset);
		for (GMint i = 0; i < 3; i++) {
			if (f4_start[i] < f4_end[i]) {
				f4_bounds[0][i] = f4_start[i] - fabs(f4_sphere_offset[i]) - tw.sphere.radius;
				f4_bounds[1][i] = f4_end[i] + fabs(f4_sphere_offset[i]) + tw.sphere.radius;
			}
			else {
				f4_bounds[0][i] = f4_end[i] - fabs(f4_sphere_offset[i]) - tw.sphere.radius;
				f4_bounds[1][i] = f4_start[i] + fabs(f4_sphere_offset[i]) + tw.sphere.radius;
			}
		}
	}
	else
	{
		for (GMint i = 0; i < 3; i++)
		{
			if (f4_start[i] < f4_end[i])
			{
				f4_bounds[0][i] = f4_start[i] + f4_size[0][i];
				f4_bounds[1][i] = f4_end[i] + f4_size[1][i];
			}
			else
			{
				f4_bounds[0][i] = f4_end[i] + f4_size[0][i];
				f4_bounds[1][i] = f4_start[i] + f4_size[1][i];
			}
		}
	}
	tw.bounds[0].setFloat4(f4_bounds[0]);
	tw.bounds[1].setFloat4(f4_bounds[1]);

	if (start == end)
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
		if (f4_size[0][0] == 0 && f4_size[0][1] == 0 && f4_size[0][2] == 0)
		{
			tw.isPoint = true;
			tw.extents = Zero<GMVec3>();
		}
		else
		{
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
	GM_ASSERT(tw.trace.allsolid ||
		tw.trace.fraction == 1.0 ||
		LengthSq(tw.trace.plane.normal) > 0.9999f);
	trace = tw.trace;
}

void GMBSPTrace::traceThroughTree(GMBSPTraceWork& tw, GMint num, GMfloat p1f, GMfloat p2f, const GMVec3& p1, const GMVec3& p2)
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
	GMFloat4 f4_p1, f4_p2, f4_extents;
	p1.loadFloat4(f4_p1);
	p2.loadFloat4(f4_p2);
	tw.extents.loadFloat4(f4_extents);
	if (plane->planeType < PLANE_NON_AXIAL)
	{
		t1 = f4_p1[plane->planeType] + dist;
		t2 = f4_p2[plane->planeType] + dist;
		offset = f4_extents[plane->planeType];
	}
	else
	{
		t1 = Dot(plane->normal, p1) + dist;
		t2 = Dot(plane->normal, p2) + dist;
		if (tw.isPoint) {
			offset = 0;
		}
		else {
			// this is silly
			offset = 2048;
		}
	}

	// see which sides we need to consider
	if (t1 >= offset + 1 && t2 >= offset + 1)
	{
		traceThroughTree(tw, node->children[0], p1f, p2f, p1, p2); // 在平面前
		return;
	}

	if (t1 < -offset - 1 && t2 < -offset - 1)
	{
		traceThroughTree(tw, node->children[1], p1f, p2f, p1, p2); // 在平面后
		return;
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	GMfloat idist;
	GMint side;
	GMfloat frac, frac2;
	if (t1 < t2)
	{
		idist = 1.0 / (t1 - t2);
		side = 1;
		frac2 = (t1 + offset + SURFACE_CLIP_EPSILON)*idist;
		frac = (t1 - offset + SURFACE_CLIP_EPSILON)*idist;
	}
	else if (t1 > t2)
	{
		idist = 1.0 / (t1 - t2);
		side = 0;
		frac2 = (t1 - offset - SURFACE_CLIP_EPSILON)*idist;
		frac = (t1 + offset + SURFACE_CLIP_EPSILON)*idist;
	}
	else
	{
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if (frac < 0)
	{
		frac = 0;
	}
	if (frac > 1)
	{
		frac = 1;
	}

	GMfloat midf;
	GMVec3 mid;
	midf = p1f + (p2f - p1f)*frac;

	mid = p1 + frac * (p2 - p1);

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
	GMVec4 plane, bestplane;
	GMVec3 startp, endp;

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
		if (tw.sphere.use)
		{
			// adjust the plane distance apropriately for radius
			plane.setW(plane.getW() + tw.sphere.radius);

			// find the closest point on the capsule to the plane
			t = Dot(MakeVector3(plane), tw.sphere.offset);
			if (t > 0.0f)
			{
				startp = tw.start - tw.sphere.offset;
				endp = tw.end - tw.sphere.offset;
			}
			else
			{
				startp = tw.start + tw.sphere.offset;
				endp = tw.end + tw.sphere.offset;
			}
		}
		else
		{
			offset = Dot(tw.offsets[planes->signbits], MakeVector3(plane));
			plane.setW(plane.getW() + offset);
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
				plane.setW(plane.getW() + tw.sphere.radius);

				// find the closest point on the capsule to the plane
				t = Dot(MakeVector3(plane), tw.sphere.offset);
				if (t > 0.0f)
				{
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
				offset = Dot(tw.offsets[planes->signbits], MakeVector3(plane));
				plane.setW(plane.getW() - Fabs(offset));
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
				tw.trace.plane.normal = MakeVector3(bestplane);
				tw.trace.plane.intercept = bestplane.getW();
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
		offset = Dot(tw.offsets[plane.signbits], MakeVector3(plane.plane));
		GMfloat intercept = plane.plane.getW();
		d1 = Dot(tw.start, MakeVector3(plane.plane)) - intercept + offset;
		d2 = Dot(tw.end, MakeVector3(plane.plane)) - intercept + offset;
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
			GMfloat intercept = planes->plane.getW();
			// calculate intersection with a slight pushoff
			offset = Dot(tw.offsets[planes->signbits], MakeVector3(planes->plane));
			d1 = Dot(tw.start, MakeVector3(planes->plane)) - intercept + offset;
			d2 = Dot(tw.end, MakeVector3(planes->plane)) - intercept + offset;
			tw.trace.fraction = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);

			if (tw.trace.fraction < 0)
			{
				tw.trace.fraction = 0;
			}

			tw.trace.plane.normal = MakeVector3(planes->plane);
			tw.trace.plane.intercept = intercept;
		}
	}
}

GMint GMBSPTrace::checkFacetPlane(const GMVec4& plane, const GMVec3& start, const GMVec3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint *hit)
{
	float d1, d2, f;

	*hit = false;

	GMfloat intercept = plane.getW();
	d1 = Dot(start, MakeVector3(plane)) + intercept;
	d2 = Dot(end, MakeVector3(plane)) + intercept;

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
	GMVec3 startp, endp;

	if (tw.sphere.use)
	{
		for (GMint i = 0; i < brush->brush->numSides; i++) {
			side = brush->sides + i;
			plane = &pw.planes[side->side->planeNum];

			// adjust the plane distance apropriately for radius
			GMfloat dist = plane->intercept - tw.sphere.radius;

			// find the closest point on the capsule to the plane
			GMfloat t = Dot(plane->normal, tw.sphere.offset);
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

			GMfloat d1 = Dot(startp, plane->normal) + dist;
			GMfloat d2 = Dot(endp, plane->normal) + dist;

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
			GMfloat bound = Dot(tw.offsets[plane->signbits], plane->normal);
			GMfloat dist = plane->intercept + bound;

			GMfloat d1 = Dot(tw.start, plane->normal) + dist;
			GMfloat d2 = Dot(tw.end, plane->normal) + dist;

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

bool GMBSPTrace::boundsIntersect(const GMVec3& mins, const GMVec3& maxs, const GMVec3& mins2, const GMVec3& maxs2)
{
	GMFloat4 f4_mins, f4_maxs, f4_mins2, f4_maxs2;
	mins.loadFloat4(f4_mins);
	maxs.loadFloat4(f4_maxs);
	mins2.loadFloat4(f4_mins2);
	maxs2.loadFloat4(f4_maxs2);

	if (f4_maxs[0] < f4_mins2[0] - SURFACE_CLIP_EPSILON ||
		f4_maxs[1] < f4_mins2[1] - SURFACE_CLIP_EPSILON ||
		f4_maxs[2] < f4_mins2[2] - SURFACE_CLIP_EPSILON ||
		f4_mins[0] > f4_maxs2[0] + SURFACE_CLIP_EPSILON ||
		f4_mins[1] > f4_maxs2[1] + SURFACE_CLIP_EPSILON ||
		f4_mins[2] > f4_maxs2[2] + SURFACE_CLIP_EPSILON)
	{
		return false;
	}

	return true;
}
