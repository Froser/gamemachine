#include "stdafx.h"
#include "bsppatch.h"
#include "bspphysicsstructs.h"
#include "utilities/assert.h"
#include "utilities/vector.h"

#define SUBDIVIDE_DISTANCE 16	//4	// never more than this units away from curve
// Epsilons
#define NORMAL_EPSILON 0.001
#define WRAP_POINT_EPSILON 0.1
#define POINT_EPSILON 0.1
#define	DIST_EPSILON 0.02
#define	PLANE_TRI_EPSILON 0.1

// Bounds
#define MAX_MAP_BOUNDS 65535
#define	MAX_GRID_SIZE 129
#define	MAX_FACETS 1024

typedef enum
{
	EN_TOP,
	EN_RIGHT,
	EN_BOTTOM,
	EN_LEFT
} EdgeName;

enum
{
	SIDE_FRONT = 0,
	SIDE_ON = 2,
	SIDE_BACK = 1,
	SIDE_CROSS = -2,
};

struct PatchCollideContext
{
	AlignedVector<BSPPatchPlane> planes;
	AlignedVector<BSPFacet> facets;
};

struct BSPGrid
{
	GMint width;
	GMint height;
	bool wrapWidth;
	bool wrapHeight;
	linear_math::Vector3 points[MAX_GRID_SIZE][MAX_GRID_SIZE];	// [width][height]
};

//a winding gives the bounding points of a convex polygon
struct BSPWinding
{
	AlignedVector<linear_math::Vector3> p;

	void alloc(GMint pointNum)
	{
		ASSERT(pointNum != 0);
		p.resize(pointNum);
	}
};

//tools
static void clearBounds(linear_math::Vector3& mins, linear_math::Vector3& maxs) {
	mins = linear_math::Vector3(99999);
	maxs = linear_math::Vector3(-99999);
}

static void addPointToBounds(const linear_math::Vector3& v, linear_math::Vector3& mins, linear_math::Vector3& maxs) {
	if (v[0] < mins[0]) {
		mins[0] = v[0];
	}
	if (v[0] > maxs[0]) {
		maxs[0] = v[0];
	}

	if (v[1] < mins[1]) {
		mins[1] = v[1];
	}
	if (v[1] > maxs[1]) {
		maxs[1] = v[1];
	}

	if (v[2] < mins[2]) {
		mins[2] = v[2];
	}
	if (v[2] > maxs[2]) {
		maxs[2] = v[2];
	}
}

static void setGridWrapWidth(BSPGrid* grid)
{
	GMint i, j;
	GMfloat d;

	for (i = 0; i < grid->height; i++) {
		for (j = 0; j < 3; j++) {
			d = grid->points[0][i][j] - grid->points[grid->width - 1][i][j];
			if (d < -WRAP_POINT_EPSILON || d > WRAP_POINT_EPSILON) {
				break;
			}
		}
		if (j != 3) {
			break;
		}
	}
	if (i == grid->height)
		grid->wrapWidth = true;
	else
		grid->wrapWidth = false;
}

static bool needsSubdivision(const linear_math::Vector3& a, const linear_math::Vector3& b, const linear_math::Vector3& c) {
	linear_math::Vector3 cmid;
	linear_math::Vector3 lmid;
	linear_math::Vector3 delta;
	GMfloat dist;

	// calculate the linear midpoint
	lmid = (a + c) * .5f;

	// calculate the exact curve midpoint
	cmid = ((a + b) * .5f + (b + c) * .5f) * .5f;

	// see if the curve is far enough away from the linear mid
	delta = cmid - lmid;
	dist = linear_math::length(delta);

	return dist >= SUBDIVIDE_DISTANCE;
}

static void subdivide(linear_math::Vector3& a, linear_math::Vector3& b, linear_math::Vector3& c, linear_math::Vector3& out1, linear_math::Vector3& out2, linear_math::Vector3& out3) {
	out1 = (a + b) * .5f;
	out3 = (b + c) * .5f;
	out2 = (out1 + out3) * .5f;
}

static void subdivideGridColumns(BSPGrid* grid) {
	GMint i, j, k;

	for (i = 0; i < grid->width - 2; ) {
		// grid->points[i][x] is an interpolating control point
		// grid->points[i+1][x] is an aproximating control point
		// grid->points[i+2][x] is an interpolating control point

		//
		// first see if we can collapse the aproximating collumn away
		//
		for (j = 0; j < grid->height; j++) {
			if (needsSubdivision(grid->points[i][j], grid->points[i + 1][j], grid->points[i + 2][j])) {
				break;
			}
		}
		if (j == grid->height) {
			// all of the points were close enough to the linear midpoints
			// that we can collapse the entire column away
			for (j = 0; j < grid->height; j++) {
				// remove the column
				for (k = i + 2; k < grid->width; k++) {
					grid->points[k - 1][j] = grid->points[k][j];
				}
			}

			grid->width--;

			// go to the next curve segment
			i++;
			continue;
		}

		//
		// we need to subdivide the curve
		//
		for (j = 0; j < grid->height; j++)
		{
			linear_math::Vector3 prev, mid, next;

			// save the control points now
			prev = grid->points[i][j];
			mid = grid->points[i + 1][j];
			next = grid->points[i + 2][j];

			// make room for two additional columns in the grid
			// columns i+1 will be replaced, column i+2 will become i+4
			// i+1, i+2, and i+3 will be generated
			for (k = grid->width - 1; k > i + 1; k--)
			{
				grid->points[k + 2][j] = grid->points[k][j];
			}

			// generate the subdivided points
			subdivide(prev, mid, next, grid->points[i + 1][j], grid->points[i + 2][j], grid->points[i + 3][j]);
		}

		grid->width += 2;

		// the new aproximating point at i+1 may need to be removed
		// or subdivided farther, so don't advance i
	}
}

static bool comparePoints(const linear_math::Vector3& a, const linear_math::Vector3& b) {
	GMfloat d;

	d = a[0] - b[0];
	if (d < -POINT_EPSILON || d > POINT_EPSILON) {
		return false;
	}
	d = a[1] - b[1];
	if (d < -POINT_EPSILON || d > POINT_EPSILON) {
		return false;
	}
	d = a[2] - b[2];
	if (d < -POINT_EPSILON || d > POINT_EPSILON) {
		return false;
	}
	return true;
}

static void removeDegenerateColumns(BSPGrid* grid) {
	GMint i, j, k;

	for (i = 0; i < grid->width - 1; i++) {
		for (j = 0; j < grid->height; j++) {
			if (!comparePoints(grid->points[i][j], grid->points[i + 1][j])) {
				break;
			}
		}

		if (j != grid->height) {
			continue;	// not degenerate
		}

		for (j = 0; j < grid->height; j++) {
			// remove the column
			for (k = i + 2; k < grid->width; k++) {
				grid->points[k - 1][j] = grid->points[k][j];
			}
		}
		grid->width--;

		// check against the next column
		i--;
	}
}

static void transposeGrid(BSPGrid* grid)
{
	GMint i, j, l;
	linear_math::Vector3	temp;
	bool tempWrap;

	if (grid->width > grid->height)
	{
		for (i = 0; i < grid->height; i++)
		{
			for (j = i + 1; j < grid->width; j++)
			{
				if (j < grid->height)
				{
					// swap the value
					temp = grid->points[i][j];
					grid->points[i][j] = grid->points[j][i];
					grid->points[j][i] = temp;
				}
				else
				{
					// just copy
					grid->points[i][j] = grid->points[j][i];
				}
			}
		}
	}
	else
	{
		for (i = 0; i < grid->width; i++)
		{
			for (j = i + 1; j < grid->height; j++)
			{
				if (j < grid->width)
				{
					// swap the value
					temp = grid->points[j][i];
					grid->points[j][i] = grid->points[i][j];
					grid->points[i][j] = temp;
				}
				else
				{
					// just copy
					grid->points[j][i] = grid->points[i][j];
				}
			}
		}
	}

	l = grid->width;
	grid->width = grid->height;
	grid->height = l;

	tempWrap = grid->wrapWidth;
	grid->wrapWidth = grid->wrapHeight;
	grid->wrapHeight = tempWrap;
}

static bool planeFromPoints(linear_math::Vector4& plane, const linear_math::Vector3& a, const linear_math::Vector3& b, const linear_math::Vector3& c) {
	linear_math::Vector3 d1, d2;
	d1 = b - a;
	d2 = c - a;
	linear_math::Vector3 t = linear_math::cross(d2, d1);
	if (linear_math::length(t) == 0)
		return false;

	t = linear_math::precise_normalize(t);
	plane = VEC4(t, plane);
	plane[3] = -linear_math::dot(a, VEC3(plane));

	ASSERT(a[0] * plane[0] + a[1] * plane[1] + a[2] * plane[2] + plane[3] < NORMAL_EPSILON);
	return true;
}

static int signbitsForNormal(const linear_math::Vector4& normal)
{
	GMint bits, j;

	bits = 0;
	for (j = 0; j < 3; j++) {
		if (normal[j] < 0) {
			bits |= 1 << j;
		}
	}
	return bits;
}

static GMint planeEqual(BSPPatchPlane* p, const linear_math::Vector4& plane, GMint *flipped)
{
	linear_math::Vector4 invplane;

	if (
		fabs(p->plane[0] - plane[0]) < NORMAL_EPSILON
		&& fabs(p->plane[1] - plane[1]) < NORMAL_EPSILON
		&& fabs(p->plane[2] - plane[2]) < NORMAL_EPSILON
		&& fabs(p->plane[3] - plane[3]) < DIST_EPSILON)
	{
		*flipped = false;
		return true;
	}

	invplane = -plane;

	if (
		fabs(p->plane[0] - invplane[0]) < NORMAL_EPSILON
		&& fabs(p->plane[1] - invplane[1]) < NORMAL_EPSILON
		&& fabs(p->plane[2] - invplane[2]) < NORMAL_EPSILON
		&& fabs(p->plane[3] - invplane[3]) < DIST_EPSILON)
	{
		*flipped = true;
		return true;
	}

	return false;
}

static int findPlane(PatchCollideContext& context, const linear_math::Vector3& p1, const linear_math::Vector3& p2, const linear_math::Vector3& p3)
{
	linear_math::Vector4 plane;
	GMfloat d;

	if (!planeFromPoints(plane, p1, p2, p3))
		return -1;

	// see if the points are close enough to an existing plane
	for (GMuint i = 0; i < context.planes.size(); i++)
	{
		if (linear_math::dot(VEC3(plane), VEC3(context.planes[i].plane)) < 0)
			continue;	// allow backwards planes?

		d = linear_math::dot(p1, VEC3(context.planes[i].plane)) + context.planes[i].plane[3];
		if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
			continue;

		d = linear_math::dot(p2, VEC3(context.planes[i].plane)) + context.planes[i].plane[3];
		if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
			continue;

		d = linear_math::dot(p3, VEC3(context.planes[i].plane)) + context.planes[i].plane[3];
		if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON) {
			continue;
		}

		// found it
		return i;
	}

	context.planes.push_back(BSPPatchPlane());
	BSPPatchPlane& p = context.planes.back();
	p.plane = plane;
	p.signbits = signbitsForNormal(plane);
	return context.planes.size() - 1;
}

static GMint findPlane(PatchCollideContext& context, const linear_math::Vector4& plane, GMint *flipped)
{
	// see if the points are close enough to an existing plane
	for (GMuint i = 0; i < context.planes.size(); i++)
	{
		if (planeEqual(&context.planes[i], plane, flipped))
			return i;
	}

	context.planes.push_back(BSPPatchPlane());
	BSPPatchPlane& p = context.planes.back();
	p.plane = plane;
	p.signbits = signbitsForNormal(plane);

	*flipped = false;

	return context.planes.size() - 1;
}

static GMint gridPlane(int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], GMint i, GMint j, GMint tri)
{
	GMint p;

	p = gridPlanes[i][j][tri];
	if (p != -1) {
		return p;
	}
	p = gridPlanes[i][j][!tri];
	if (p != -1) {
		return p;
	}

	// should never happen
	gm_warning("WARNING: gridPlane unresolvable\n");
	return -1;
}

static int edgePlaneNum(PatchCollideContext& context, BSPGrid* grid, GMint gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], GMint i, GMint j, GMint k) {
	linear_math::Vector3 p1, p2;
	linear_math::Vector3 up;
	GMint p;
	linear_math::Vector4 t;

	switch (k) {
	case 0:	// top border
		p1 = grid->points[i][j];
		p2 = grid->points[i + 1][j];
		p = gridPlane(gridPlanes, i, j, 0);
		t = context.planes[p].plane * 4;
		up = p1 + VEC3(t);
		return findPlane(context, p1, p2, up);

	case 2:	// bottom border
		p1 = grid->points[i][j + 1];
		p2 = grid->points[i + 1][j + 1];
		p = gridPlane(gridPlanes, i, j, 1);
		t = context.planes[p].plane * 4;
		up = p1 + VEC3(t);

		return findPlane(context, p2, p1, up);

	case 3: // left border
		p1 = grid->points[i][j];
		p2 = grid->points[i][j + 1];
		p = gridPlane(gridPlanes, i, j, 1);
		t = context.planes[p].plane * 4;
		up = p1 + VEC3(t);
		return findPlane(context, p2, p1, up);

	case 1:	// right border
		p1 = grid->points[i + 1][j];
		p2 = grid->points[i + 1][j + 1];
		p = gridPlane(gridPlanes, i, j, 0);
		t = context.planes[p].plane * 4;
		up = p1 + VEC3(t);
		return findPlane(context, p1, p2, up);

	case 4:	// diagonal out of triangle 0
		p1 = grid->points[i + 1][j + 1];
		p2 = grid->points[i][j];
		p = gridPlane(gridPlanes, i, j, 0);
		t = context.planes[p].plane * 4;
		up = p1 + VEC3(t);
		return findPlane(context, p1, p2, up);

	case 5:	// diagonal out of triangle 1
		p1 = grid->points[i][j];
		p2 = grid->points[i + 1][j + 1];
		p = gridPlane(gridPlanes, i, j, 1);
		t = context.planes[p].plane * 4;
		up = p1 + VEC3(t);
		return findPlane(context, p1, p2, up);

	}

	gm_error("edgePlaneNum: bad k");
	return -1;
}

static int pointOnPlaneSide(PatchCollideContext& context, const linear_math::Vector3& p, GMint planeNum) {
	float	d;

	if (planeNum == -1) {
		return SIDE_ON;
	}

	const linear_math::Vector4& plane = context.planes[planeNum].plane;

	d = linear_math::dot(p, VEC3(plane)) + plane[3];

	if (d > PLANE_TRI_EPSILON) {
		return SIDE_FRONT;
	}

	if (d < -PLANE_TRI_EPSILON) {
		return SIDE_BACK;
	}

	return SIDE_ON;
}


static void setBorderInward(PatchCollideContext& context, BSPFacet* facet, BSPGrid* grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], GMint i, GMint j, GMint which)
{
	static bool debugBlock = false;
	GMint k, l;
	linear_math::Vector3 points[4];
	GMint numPoints;

	switch (which) {
	case -1:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i + 1][j];
		points[2] = grid->points[i + 1][j + 1];
		points[3] = grid->points[i][j + 1];
		numPoints = 4;
		break;
	case 0:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i + 1][j];
		points[2] = grid->points[i + 1][j + 1];
		numPoints = 3;
		break;
	case 1:
		points[0] = grid->points[i + 1][j + 1];
		points[1] = grid->points[i][j + 1];
		points[2] = grid->points[i][j];
		numPoints = 3;
		break;
	default:
		gm_error("setBorderInward: bad parameter");
		numPoints = 0;
		break;
	}

	for (k = 0; k < facet->numBorders; k++)
	{
		GMint front, back;

		front = 0;
		back = 0;

		for (l = 0; l < numPoints; l++)
		{
			GMint side;

			side = pointOnPlaneSide(context, points[l], facet->borderPlanes[k]);
			if (side == SIDE_FRONT) {
				front++;
			} if (side == SIDE_BACK) {
				back++;
			}
		}

		if (front && !back)
		{
			facet->borderInward[k] = true;
		}
		else if (back && !front)
		{
			facet->borderInward[k] = false;
		}
		else if (!front && !back)
		{
			// flat side border
			facet->borderPlanes[k] = -1;
		}
		else
		{
			// bisecting side border
			gm_error("WARNING: setBorderInward: mixed plane sides");
			facet->borderInward[k] = false;
		}
	}
}

static void baseWindingForPlane(const linear_math::Vector4& plane, REF BSPWinding& w)
{
	linear_math::Vector3 normal = VEC3(plane);
	GMfloat dist = plane[3];
	GMint i, x;
	GMfloat max, v;
	linear_math::Vector3 org, vright, vup;

	// find the major axis

	max = -MAX_MAP_BOUNDS;
	x = -1;
	for (i = 0; i < 3; i++)
	{
		v = fabs(normal[i]);
		if (v > max)
		{
			x = i;
			max = v;
		}
	}
	if (x == -1)
		gm_error("baseWindingForPlane: no axis found");

	// 找到向上轴，openGL中y坐标轴表示向上
	vup = linear_math::Vector3(0, 0, 0);
	switch (x)
	{
	case 0:
	case 2:
		vup[1] = 1;
		break;
	case 1:
		vup[2] = 1;
		break;
	}

	v = linear_math::dot(vup, normal);
	vup = vup - normal * v;
	vup = linear_math::precise_normalize(vup);
	org = normal * -dist;
	vright = linear_math::cross(vup, normal);
	vup = vup * MAX_MAP_BOUNDS;
	vright = vright * MAX_MAP_BOUNDS;

	// project a really big	axis aligned box onto the plane
	w.alloc(4);

	w.p[0] = org - vright;
	w.p[0] = w.p[0] + vup;

	w.p[1] = org + vright;
	w.p[1] = w.p[1] + vup;

	w.p[2] = org + vright;
	w.p[2] = w.p[2] - vup;

	w.p[3] = org - vright;
	w.p[3] = w.p[3] - vup;
}

static bool chopWindingInPlace(REF BSPWinding& inout, const linear_math::Vector4& plane, GMfloat epsilon)
{
	Vector<GMfloat> dists;
	Vector<GMint> sides;
	GMint counts[3];
	GMfloat dot;
	GMint j;
	linear_math::Vector3 mid;

	linear_math::Vector3 normal = VEC3(plane);
	GMfloat dist = plane[3];

	BSPWinding& in = inout;
	BSPWinding f;

	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for (auto iter = in.p.begin(); iter != in.p.end(); iter++)
	{
		linear_math::Vector3& p = *iter;
		dot = linear_math::dot(p, normal);
		dot += dist;
		dists.push_back(dot);
		if (dot > epsilon)
			sides.push_back(SIDE_FRONT);
		else if (dot < -epsilon)
			sides.push_back(SIDE_BACK);
		else
			sides.push_back(SIDE_ON);
		counts[sides.back()]++;
	}
	sides.push_back(sides.front());
	dists.push_back(dists.front());

	if (!counts[0])
		return false;

	if (!counts[1])
		return true; // inout stays the same

	for (GMuint i = 0; i < in.p.size(); i++)
	{
		linear_math::Vector3 p1 = in.p[i];

		if (sides[i] == SIDE_ON)
		{
			f.p.push_back(p1);
			continue;
		}

		if (sides[i] == SIDE_FRONT)
		{
			f.p.push_back(p1);
		}

		if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i])
			continue;

		// generate a split point
		linear_math::Vector3 p2 = in.p[(i + 1) % in.p.size()];

		dot = dists[i] / (dists[i] - dists[i + 1]);
		for (j = 0; j < 3; j++)
		{	// avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = -dist;
			else if (normal[j] == -1)
				mid[j] = dist;
			else
				mid[j] = p1[j] + dot*(p2[j] - p1[j]);
		}

		f.p.push_back(mid);
	}

	inout = f;
	return true;
}

static void windingBounds(const BSPWinding& w, linear_math::Vector3& mins, linear_math::Vector3& maxs)
{
	GMfloat v;

	mins = linear_math::Vector3(MAX_MAP_BOUNDS);
	maxs = linear_math::Vector3(-MAX_MAP_BOUNDS);

	for (GMuint i = 0; i < w.p.size(); i++)
	{
		for (GMint j = 0; j < 3; j++)
		{
			v = w.p[i][j];
			if (v < mins[j])
				mins[j] = v;
			if (v > maxs[j])
				maxs[j] = v;
		}
	}
}

void snapVector(linear_math::Vector3& normal) {
	GMint i;

	for (i = 0; i < 3; i++)
	{
		if (fabs(normal[i] - 1) < NORMAL_EPSILON)
		{
			normal = linear_math::Vector3(0);
			normal[i] = 1;
			break;
		}
		if (fabs(normal[i] - -1) < NORMAL_EPSILON)
		{
			normal = linear_math::Vector3(0);
			normal[i] = -1;
			break;
		}
	}
}

static void addFacetBevels(PatchCollideContext& context, BSPFacet *facet)
{
	GMint axis, dir, flipped;
	linear_math::Vector4 plane;
	GMfloat d;
	linear_math::Vector4 newplane;
	linear_math::Vector3 mins, maxs, vec, Vector2;

	plane = context.planes[facet->surfacePlane].plane;

	BSPWinding w, w2;
	baseWindingForPlane(plane, w);

	GMint j;
	for (j = 0; j < facet->numBorders; j++)
	{
		if (facet->borderPlanes[j] == facet->surfacePlane)
			continue;

		plane = context.planes[facet->borderPlanes[j]].plane;
		if (!facet->borderInward[j])
			plane = -plane;

		if (!chopWindingInPlace(w, plane, 0.1f))
			break;
	}

	if (j < facet->numBorders)
		return;

	windingBounds(w, mins, maxs);

	// add the axial planes
	for (axis = 0; axis < 3; axis++)
	{
		for (dir = -1; dir <= 1; dir += 2)
		{
			plane = linear_math::Vector4(0);
			plane[axis] = dir;
			if (dir == 1) {
				plane[3] = -maxs[axis];
			}
			else {
				plane[3] = mins[axis];
			}
			//if it's the surface plane
			if (planeEqual(&context.planes[facet->surfacePlane], plane, &flipped)) {
				continue;
			}
			// see if the plane is already present
			GMint i;
			for (i = 0; i < facet->numBorders; i++) {
				if (planeEqual(&context.planes[facet->borderPlanes[i]], plane, &flipped))
					break;
			}

			if (i == facet->numBorders)
			{
				if (facet->numBorders > 4 + 6 + 16) gm_error("ERROR: too many bevels\n");
				facet->borderPlanes[facet->numBorders] = findPlane(context, plane, &flipped);
				facet->borderNoAdjust[facet->numBorders] = 0;
				facet->borderInward[facet->numBorders] = flipped;
				facet->numBorders++;
			}
		}
	}
	//
	// add the edge bevels
	//
	// test the non-axial plane edges
	GMint k;
	for (j = 0; j < (GMint) w.p.size(); j++)
	{
		k = (j + 1) % w.p.size();
		vec = w.p[j] - w.p[k];
		//if it's a degenerate edge
		vec = linear_math::precise_normalize(vec);
		if (linear_math::length(vec) < 0.5)
			continue;
		snapVector(vec);
		for (k = 0; k < 3; k++)
		{
			if (linear_math::fuzzyCompare(vec[k], -1) || linear_math::fuzzyCompare(vec[k], 1))
				break;	// axial
		}
		if (k < 3)
			continue;	// only test non-axial edges

						// try the six possible slanted axials from this edge
		for (axis = 0; axis < 3; axis++)
		{
			for (dir = -1; dir <= 1; dir += 2)
			{
				// construct a plane
				Vector2 = linear_math::Vector3(0);
				Vector2[axis] = dir;
				linear_math::Vector3 t = linear_math::cross(vec, Vector2);
				t = linear_math::precise_normalize(t);
				if (linear_math::length(t) < 0.5)
					continue;
				plane = VEC4(t, plane);
				plane[3] = -linear_math::dot(w.p[j], VEC3(plane));

				// if all the points of the facet winding are
				// behind this plane, it is a proper edge bevel
				GMuint l;
				for (l = 0; l < w.p.size(); l++)
				{
					d = linear_math::dot(w.p[l], VEC3(plane)) + plane[3];
					if (d > 0.1)
						break;	// point in front
				}

				if (l < w.p.size())
					continue;

				//if it's the surface plane
				if (planeEqual(&context.planes[facet->surfacePlane], plane, &flipped)) {
					continue;
				}
				// see if the plane is allready present
				GMint i;
				for (i = 0; i < facet->numBorders; i++) {
					if (planeEqual(&context.planes[facet->borderPlanes[i]], plane, &flipped)) {
						break;
					}
				}

				if (i == facet->numBorders)
				{
					if (facet->numBorders > 4 + 6 + 16)
						gm_error("ERROR: too many bevels\n");

					facet->borderPlanes[facet->numBorders] = findPlane(context, plane, &flipped);

					for (k = 0; k < facet->numBorders; k++)
					{
						if (facet->borderPlanes[facet->numBorders] ==
							facet->borderPlanes[k]) gm_warning("WARNING: bevel plane already used\n");
					}

					facet->borderNoAdjust[facet->numBorders] = 0;
					facet->borderInward[facet->numBorders] = flipped;
					w2 = w;
					newplane = context.planes[facet->borderPlanes[facet->numBorders]].plane;
					if (!facet->borderInward[facet->numBorders])
						newplane = -newplane;

					if (!chopWindingInPlace(w2, newplane, 0.1f))
					{
						gm_warning("WARNING: addFacetBevels... invalid bevel");
						continue;
					}
					//
					facet->numBorders++;
				}
			}
		}
	}
}

static bool validateFacet(PatchCollideContext& context, BSPFacet* facet)
{
	linear_math::Vector4 plane;
	linear_math::Vector3 bounds[2];
	linear_math::Vector3 origin(0, 0, 0);

	if (facet->surfacePlane == -1)
		return false;

	plane = context.planes[facet->surfacePlane].plane;

	BSPWinding w;
	baseWindingForPlane(plane, w);

	GMint i;
	for (i = 0; i < facet->numBorders; i++)
	{
		if (facet->borderPlanes[i] == -1)
			return false;

		plane = context.planes[facet->borderPlanes[i]].plane;
		if (!facet->borderInward[i])
			plane = -plane;

		if (!chopWindingInPlace(w, plane, 0.1f))
			break;
	}

	if (i < facet->numBorders)
		return false;		// winding was completely chopped away

	// see if the facet is unreasonably large
	windingBounds(w, bounds[0], bounds[1]);

	for (i = 0; i < 3; i++)
	{
		if (bounds[1][i] - bounds[0][i] > MAX_MAP_BOUNDS) {
			return false;		// we must be missing a plane
		}
		if (bounds[0][i] >= MAX_MAP_BOUNDS) {
			return false;
		}
		if (bounds[1][i] <= -MAX_MAP_BOUNDS) {
			return false;
		}
	}
	return true;		// winding is fine
}

static void patchCollideFromGrid(BSPGrid *grid, BSPPatchCollide *pf)
{
	GMint i, j;
	linear_math::Vector3 p1, p2, p3;
	GMint gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
	GMint borders[4];
	bool noAdjust[4];
	PatchCollideContext context;

	// find the planes for each triangle of the grid
	for (i = 0; i < grid->width - 1; i++)
	{
		for (j = 0; j < grid->height - 1; j++)
		{
			p1 = grid->points[i][j];
			p2 = grid->points[i + 1][j];
			p3 = grid->points[i + 1][j + 1];
			gridPlanes[i][j][0] = findPlane(context, p1, p2, p3);

			p1 = grid->points[i + 1][j + 1];
			p2 = grid->points[i][j + 1];
			p3 = grid->points[i][j];
			gridPlanes[i][j][1] = findPlane(context, p1, p2, p3);
		}
	}

	// create the borders for each facet
	for (i = 0; i < grid->width - 1; i++) {
		for (j = 0; j < grid->height - 1; j++) {

			borders[EN_TOP] = -1;
			if (j > 0) {
				borders[EN_TOP] = gridPlanes[i][j - 1][1];
			}
			else if (grid->wrapHeight) {
				borders[EN_TOP] = gridPlanes[i][grid->height - 2][1];
			}
			noAdjust[EN_TOP] = (borders[EN_TOP] == gridPlanes[i][j][0]);
			if (borders[EN_TOP] == -1 || noAdjust[EN_TOP]) {
				borders[EN_TOP] = edgePlaneNum(context, grid, gridPlanes, i, j, 0);
			}

			borders[EN_BOTTOM] = -1;
			if (j < grid->height - 2) {
				borders[EN_BOTTOM] = gridPlanes[i][j + 1][0];
			}
			else if (grid->wrapHeight) {
				borders[EN_BOTTOM] = gridPlanes[i][0][0];
			}
			noAdjust[EN_BOTTOM] = (borders[EN_BOTTOM] == gridPlanes[i][j][1]);
			if (borders[EN_BOTTOM] == -1 || noAdjust[EN_BOTTOM]) {
				borders[EN_BOTTOM] = edgePlaneNum(context, grid, gridPlanes, i, j, 2);
			}

			borders[EN_LEFT] = -1;
			if (i > 0) {
				borders[EN_LEFT] = gridPlanes[i - 1][j][0];
			}
			else if (grid->wrapWidth) {
				borders[EN_LEFT] = gridPlanes[grid->width - 2][j][0];
			}
			noAdjust[EN_LEFT] = (borders[EN_LEFT] == gridPlanes[i][j][1]);
			if (borders[EN_LEFT] == -1 || noAdjust[EN_LEFT]) {
				borders[EN_LEFT] = edgePlaneNum(context, grid, gridPlanes, i, j, 3);
			}

			borders[EN_RIGHT] = -1;
			if (i < grid->width - 2) {
				borders[EN_RIGHT] = gridPlanes[i + 1][j][1];
			}
			else if (grid->wrapWidth) {
				borders[EN_RIGHT] = gridPlanes[0][j][1];
			}
			noAdjust[EN_RIGHT] = (borders[EN_RIGHT] == gridPlanes[i][j][0]);
			if (borders[EN_RIGHT] == -1 || noAdjust[EN_RIGHT]) {
				borders[EN_RIGHT] = edgePlaneNum(context, grid, gridPlanes, i, j, 1);
			}

			BSPFacet facet;
			memset(&facet, 0, sizeof(facet));

			if (gridPlanes[i][j][0] == gridPlanes[i][j][1]) {
				if (gridPlanes[i][j][0] == -1) {
					continue;		// degenrate
				}
				facet.surfacePlane = gridPlanes[i][j][0];
				facet.numBorders = 4;
				facet.borderPlanes[0] = borders[EN_TOP];
				facet.borderNoAdjust[0] = noAdjust[EN_TOP];
				facet.borderPlanes[1] = borders[EN_RIGHT];
				facet.borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet.borderPlanes[2] = borders[EN_BOTTOM];
				facet.borderNoAdjust[2] = noAdjust[EN_BOTTOM];
				facet.borderPlanes[3] = borders[EN_LEFT];
				facet.borderNoAdjust[3] = noAdjust[EN_LEFT];
				setBorderInward(context, &facet, grid, gridPlanes, i, j, -1);
				if (validateFacet(context, &facet))
				{
					addFacetBevels(context, &facet);
					context.facets.push_back(facet);
				}
			}
			else
			{
				// two seperate triangles
				facet.surfacePlane = gridPlanes[i][j][0];
				facet.numBorders = 3;
				facet.borderPlanes[0] = borders[EN_TOP];
				facet.borderNoAdjust[0] = noAdjust[EN_TOP];
				facet.borderPlanes[1] = borders[EN_RIGHT];
				facet.borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet.borderPlanes[2] = gridPlanes[i][j][1];
				if (facet.borderPlanes[2] == -1)
				{
					facet.borderPlanes[2] = borders[EN_BOTTOM];
					if (facet.borderPlanes[2] == -1)
						facet.borderPlanes[2] = edgePlaneNum(context, grid, gridPlanes, i, j, 4);
				}
				setBorderInward(context, &facet, grid, gridPlanes, i, j, 0);
				if (validateFacet(context, &facet))
				{
					addFacetBevels(context, &facet);
					context.facets.push_back(facet);
				}

				memset(&facet, 0, sizeof(facet));
				facet.surfacePlane = gridPlanes[i][j][1];
				facet.numBorders = 3;
				facet.borderPlanes[0] = borders[EN_BOTTOM];
				facet.borderNoAdjust[0] = noAdjust[EN_BOTTOM];
				facet.borderPlanes[1] = borders[EN_LEFT];
				facet.borderNoAdjust[1] = noAdjust[EN_LEFT];
				facet.borderPlanes[2] = gridPlanes[i][j][0];
				if (facet.borderPlanes[2] == -1) {
					facet.borderPlanes[2] = borders[EN_TOP];
					if (facet.borderPlanes[2] == -1) {
						facet.borderPlanes[2] = edgePlaneNum(context, grid, gridPlanes, i, j, 5);
					}
				}
				setBorderInward(context, &facet, grid, gridPlanes, i, j, 1);
				if (validateFacet(context, &facet)) {
					addFacetBevels(context, &facet);
					context.facets.push_back(facet);
				}
			}
		}
	}

	// copy the results out
	pf->facets = context.facets;
	pf->planes = context.planes;
}

BSPPatchPrivate::~BSPPatchPrivate()
{
	for (auto iter = patches.begin(); iter != patches.end(); iter++)
	{
		delete *iter;
	}
}

void BSPPatch::alloc(GMint num)
{
	D(d);
	d.patches.resize(num);
}

BSP_Physics_Patch* BSPPatch::patches(GMint at)
{
	D(d);
	return d.patches[at];
}

void BSPPatch::generatePatchCollide(GMint index, GMint width, GMint height, const linear_math::Vector3* points, AUTORELEASE BSP_Physics_Patch* patch)
{
	D(d);

	BSPGrid grid;
	GMint i, j;

	if (width <= 2 || height <= 2 || !points) {
		gm_error("generatePatchFacets: bad parameters: (%i, %i, %p)",
			width, height, points);
	}

	if (!(width & 1) || !(height & 1)) {
		gm_error("generatePatchFacets: even sizes are invalid for quadratic meshes");
	}

	if (width > MAX_GRID_SIZE || height > MAX_GRID_SIZE) {
		gm_error("generatePatchFacets: source is > MAX_GRID_SIZE");
	}

	// build a grid
	grid.width = width;
	grid.height = height;
	grid.wrapWidth = false;
	grid.wrapHeight = false;
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			grid.points[i][j] = points[j*width + i];
		}
	}

	// subdivide the grid
	setGridWrapWidth(&grid);
	subdivideGridColumns(&grid);
	removeDegenerateColumns(&grid);

	transposeGrid(&grid);

	setGridWrapWidth(&grid);
	subdivideGridColumns(&grid);
	removeDegenerateColumns(&grid);

	// we now have a grid of points exactly on the curve
	// the aproximate surface defined by these points will be
	// collided against
	BSPPatchCollide* pf = new BSPPatchCollide();
	clearBounds(pf->bounds[0], pf->bounds[1]);
	for (i = 0; i < grid.width; i++)
	{
		for (j = 0; j < grid.height; j++)
		{
			addPointToBounds(grid.points[i][j], pf->bounds[0], pf->bounds[1]);
		}
	}

	// generate a bsp tree for the surface
	patchCollideFromGrid(&grid, pf);

	// expand by one unit for epsilon purposes
	pf->bounds[0] -= 1;
	pf->bounds[1] += 1;

	patch->pc = pf;
	d.patches[index] = patch;
}
