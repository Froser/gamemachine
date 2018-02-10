#include "stdafx.h"
#include "gmbsppatch.h"
#include "gmbspphysicsstructs.h"
#include "foundation/vector.h"

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

enum EdgeName
{
	EN_TOP,
	EN_RIGHT,
	EN_BOTTOM,
	EN_LEFT
};

enum
{
	SIDE_FRONT = 0,
	SIDE_ON = 2,
	SIDE_BACK = 1,
	SIDE_CROSS = -2,
};

GM_ALIGNED_STRUCT(PatchCollideContext)
{
	AlignedVector<GMBSPPatchPlane> planes;
	AlignedVector<GMBSPFacet> facets;
};

GM_ALIGNED_STRUCT(BSPGrid)
{
	GMint width;
	GMint height;
	bool wrapWidth;
	bool wrapHeight;
	GMVec3 points[MAX_GRID_SIZE][MAX_GRID_SIZE];	// [width][height]
};

//a winding gives the bounding points of a convex polygon
GM_ALIGNED_STRUCT(BSPWinding)
{
	AlignedVector<GMVec3> p;

	void alloc(GMint pointNum)
	{
		GM_ASSERT(pointNum != 0);
		p.resize(pointNum);
	}
};

//tools
namespace
{
	void clearBounds(GMVec3& mins, GMVec3& maxs)
	{
		mins = GMVec3(99999);
		maxs = GMVec3(-99999);
	}

	void addPointToBounds(const GMVec3& v, GMVec3& mins, GMVec3& maxs)
	{
		if (v.getX() < mins.getX()) {
			mins.setX(v.getX());
		}
		if (v.getX() > maxs.getX()) {
			maxs.setX(v.getX());
		}

		if (v.getY() < mins.getY()) {
			mins.setY(v.getY());
		}
		if (v.getY() > maxs.getY()) {
			maxs.setY(v.getY());
		}

		if (v.getZ() < mins.getZ()) {
			mins.setZ(v.getZ());
		}
		if (v.getZ() > maxs.getZ()) {
			maxs.setZ(v.getZ());
		}
	}

	void setGridWrapWidth(BSPGrid* grid)
	{
		GMint i, j;
		GMfloat d;

		GMFloat4 f4_points_0, f4_points_1;
		for (i = 0; i < grid->height; i++)
		{
			grid->points[0][i].loadFloat4(f4_points_0);
			grid->points[grid->width - 1][i].loadFloat4(f4_points_1);
			for (j = 0; j < 3; j++)
			{
				d = f4_points_0[j] - f4_points_1[j];
				if (d < -WRAP_POINT_EPSILON || d > WRAP_POINT_EPSILON)
					break;
			}
			if (j != 3)
				break;
		}
		if (i == grid->height)
			grid->wrapWidth = true;
		else
			grid->wrapWidth = false;
	}

	bool needsSubdivision(const GMVec3& a, const GMVec3& b, const GMVec3& c)
	{
		GMVec3 cmid;
		GMVec3 lmid;
		GMVec3 delta;
		GMfloat dist;

		// calculate the linear midpoint
		lmid = (a + c) * .5f;

		// calculate the exact curve midpoint
		cmid = ((a + b) * .5f + (b + c) * .5f) * .5f;

		// see if the curve is far enough away from the linear mid
		delta = cmid - lmid;
		dist = Length(delta);

		return dist >= SUBDIVIDE_DISTANCE;
	}

	void subdivide(GMVec3& a, GMVec3& b, GMVec3& c, GMVec3& out1, GMVec3& out2, GMVec3& out3)
	{
		out1 = (a + b) * .5f;
		out3 = (b + c) * .5f;
		out2 = (out1 + out3) * .5f;
	}

	void subdivideGridColumns(BSPGrid* grid)
	{
		GMint i, j, k;

		for (i = 0; i < grid->width - 2; ) {
			// grid->points[i][x] is an interpolating control point
			// grid->points[i+1][x] is an aproximating control point
			// grid->points[i+2][x] is an interpolating control point

			//
			// first see if we can collapse the aproximating collumn away
			//
			for (j = 0; j < grid->height; j++)
			{
				if (needsSubdivision(grid->points[i][j], grid->points[i + 1][j], grid->points[i + 2][j]))
				{
					break;
				}
			}
			if (j == grid->height) {
				// all of the points were close enough to the linear midpoints
				// that we can collapse the entire column away
				for (j = 0; j < grid->height; j++)
				{
					// remove the column
					for (k = i + 2; k < grid->width; k++)
					{
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
				GMVec3 prev, mid, next;

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

	bool comparePoints(const GMVec3& a, const GMVec3& b)
	{
		GMfloat d;

		d = a.getX() - b.getX();
		if (d < -POINT_EPSILON || d > POINT_EPSILON)
		{
			return false;
		}
		d = a.getY() - b.getY();
		if (d < -POINT_EPSILON || d > POINT_EPSILON)
		{
			return false;
		}
		d = a.getZ() - b.getZ();
		if (d < -POINT_EPSILON || d > POINT_EPSILON)
		{
			return false;
		}
		return true;
	}

	void removeDegenerateColumns(BSPGrid* grid)
	{
		GMint i, j, k;

		for (i = 0; i < grid->width - 1; i++)
		{
			for (j = 0; j < grid->height; j++)
			{
				if (!comparePoints(grid->points[i][j], grid->points[i + 1][j])) {
					break;
				}
			}

			if (j != grid->height)
			{
				continue;	// not degenerate
			}

			for (j = 0; j < grid->height; j++)
			{
				// remove the column
				for (k = i + 2; k < grid->width; k++)
				{
					grid->points[k - 1][j] = grid->points[k][j];
				}
			}
			grid->width--;

			// check against the next column
			i--;
		}
	}

	void transposeGrid(BSPGrid* grid)
	{
		GMint i, j, l;
		GMVec3 temp;
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

	bool planeFromPoints(GMVec4& plane, const GMVec3& a, const GMVec3& b, const GMVec3& c)
	{
		GMVec3 d1, d2;
		d1 = b - a;
		d2 = c - a;
		GMVec3 t = Cross(d2, d1);
		if (Length(t) == 0)
			return false;

		t = Normalize(t);
		plane = CombineVector4(t, plane);
		plane.setW(-Dot(a, MakeVector3(plane)));

#if _DEBUG
		GMFloat4 f4_a, f4_plane;
		a.loadFloat4(f4_a);
		plane.loadFloat4(f4_plane);
		GM_ASSERT(f4_a[0] * f4_plane[0] + f4_a[1] * f4_plane[1] + f4_a[2] * f4_plane[2] + f4_plane[3] < NORMAL_EPSILON);
#endif
		return true;
	}

	int signbitsForNormal(const GMVec4& normal)
	{
		GMint bits, j;
		GMFloat4 f4_normal;
		normal.loadFloat4(f4_normal);

		bits = 0;
		for (j = 0; j < 3; j++)
		{
			if (f4_normal[j] < 0)
			{
				bits |= 1 << j;
			}
		}
		return bits;
	}

	GMint planeEqual(GMBSPPatchPlane* p, const GMVec4& plane, GMint *flipped)
	{
		GMVec4 invplane;
		GMFloat4 f4_plane, f4_param_plane, f4_invplane;
		p->plane.loadFloat4(f4_plane);
		plane.loadFloat4(f4_param_plane);

		if (
			fabs(f4_plane[0] - f4_param_plane[0]) < NORMAL_EPSILON
			&& fabs(f4_plane[1] - f4_param_plane[1]) < NORMAL_EPSILON
			&& fabs(f4_plane[2] - f4_param_plane[2]) < NORMAL_EPSILON
			&& fabs(f4_plane[3] - f4_param_plane[3]) < DIST_EPSILON)
		{
			*flipped = false;
			return true;
		}

		invplane = -plane;
		invplane.loadFloat4(f4_invplane);

		if (
			fabs(f4_plane[0] - f4_invplane[0]) < NORMAL_EPSILON
			&& fabs(f4_plane[1] - f4_invplane[1]) < NORMAL_EPSILON
			&& fabs(f4_plane[2] - f4_invplane[2]) < NORMAL_EPSILON
			&& fabs(f4_plane[3] - f4_invplane[3]) < DIST_EPSILON)
		{
			*flipped = true;
			return true;
		}

		return false;
	}

	int findPlane(PatchCollideContext& context, const GMVec3& p1, const GMVec3& p2, const GMVec3& p3)
	{
		GMVec4 plane;
		GMfloat d;

		if (!planeFromPoints(plane, p1, p2, p3))
			return -1;

		// see if the points are close enough to an existing plane
		for (GMuint i = 0; i < context.planes.size(); i++)
		{
			if (Dot(MakeVector3(plane), MakeVector3(context.planes[i].plane)) < 0)
				continue;	// allow backwards planes?

			d = Dot(p1, MakeVector3(context.planes[i].plane)) + context.planes[i].plane.getW();
			if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
				continue;

			d = Dot(p2, MakeVector3(context.planes[i].plane)) + context.planes[i].plane.getW();
			if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
				continue;

			d = Dot(p3, MakeVector3(context.planes[i].plane)) + context.planes[i].plane.getW();
			if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON) {
				continue;
			}

			// found it
			return i;
		}

		context.planes.push_back(GMBSPPatchPlane());
		GMBSPPatchPlane& p = context.planes.back();
		p.plane = plane;
		p.signbits = signbitsForNormal(plane);
		return context.planes.size() - 1;
	}

	GMint findPlane(PatchCollideContext& context, const GMVec4& plane, GMint *flipped)
	{
		// see if the points are close enough to an existing plane
		for (GMuint i = 0; i < context.planes.size(); i++)
		{
			if (planeEqual(&context.planes[i], plane, flipped))
				return i;
		}

		context.planes.push_back(GMBSPPatchPlane());
		GMBSPPatchPlane& p = context.planes.back();
		p.plane = plane;
		p.signbits = signbitsForNormal(plane);

		*flipped = false;

		return context.planes.size() - 1;
	}

	GMint gridPlane(int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], GMint i, GMint j, GMint tri)
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
		gm_warning(L"WARNING: gridPlane unresolvable\n");
		return -1;
	}

	int edgePlaneNum(PatchCollideContext& context, BSPGrid* grid, GMint gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], GMint i, GMint j, GMint k) {
		GMVec3 p1, p2;
		GMVec3 up;
		GMint p;
		GMVec4 t;

		switch (k) {
		case 0:	// top border
			p1 = grid->points[i][j];
			p2 = grid->points[i + 1][j];
			p = gridPlane(gridPlanes, i, j, 0);
			t = context.planes[p].plane * 4.f;
			up = p1 + MakeVector3(t);
			return findPlane(context, p1, p2, up);

		case 2:	// bottom border
			p1 = grid->points[i][j + 1];
			p2 = grid->points[i + 1][j + 1];
			p = gridPlane(gridPlanes, i, j, 1);
			t = context.planes[p].plane * 4.f;
			up = p1 + MakeVector3(t);

			return findPlane(context, p2, p1, up);

		case 3: // left border
			p1 = grid->points[i][j];
			p2 = grid->points[i][j + 1];
			p = gridPlane(gridPlanes, i, j, 1);
			t = context.planes[p].plane * 4.f;
			up = p1 + MakeVector3(t);
			return findPlane(context, p2, p1, up);

		case 1:	// right border
			p1 = grid->points[i + 1][j];
			p2 = grid->points[i + 1][j + 1];
			p = gridPlane(gridPlanes, i, j, 0);
			t = context.planes[p].plane * 4.f;
			up = p1 + MakeVector3(t);
			return findPlane(context, p1, p2, up);

		case 4:	// diagonal out of triangle 0
			p1 = grid->points[i + 1][j + 1];
			p2 = grid->points[i][j];
			p = gridPlane(gridPlanes, i, j, 0);
			t = context.planes[p].plane * 4.f;
			up = p1 + MakeVector3(t);
			return findPlane(context, p1, p2, up);

		case 5:	// diagonal out of triangle 1
			p1 = grid->points[i][j];
			p2 = grid->points[i + 1][j + 1];
			p = gridPlane(gridPlanes, i, j, 1);
			t = context.planes[p].plane * 4.f;
			up = p1 + MakeVector3(t);
			return findPlane(context, p1, p2, up);

		}

		gm_error(L"edgePlaneNum: bad k");
		return -1;
	}

	int pointOnPlaneSide(PatchCollideContext& context, const GMVec3& p, GMint planeNum)
	{
		GMfloat d;

		if (planeNum == -1)
			return SIDE_ON;

		const GMVec4& plane = context.planes[planeNum].plane;

		d = Dot(p, MakeVector3(plane)) + plane.getW();

		if (d > PLANE_TRI_EPSILON)
			return SIDE_FRONT;

		if (d < -PLANE_TRI_EPSILON)
			return SIDE_BACK;

		return SIDE_ON;
	}

	void setBorderInward(PatchCollideContext& context, GMBSPFacet* facet, BSPGrid* grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], GMint i, GMint j, GMint which)
	{
		static bool debugBlock = false;
		GMint k, l;
		GMVec3 points[4];
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
			gm_error(L"setBorderInward: bad parameter");
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
				if (side == SIDE_FRONT)
					front++;
				if (side == SIDE_BACK)
					back++;
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
				gm_error(L"WARNING: setBorderInward: mixed plane sides");
				facet->borderInward[k] = false;
			}
		}
	}

	void baseWindingForPlane(const GMVec4& plane, REF BSPWinding& w)
	{
		GMVec3 normal = MakeVector3(plane);
		GMfloat dist = plane.getW();
		GMint i, x;
		GMfloat max, v;
		GMVec3 org, vright, vup;
		GMFloat4 f4_normal;
		normal.loadFloat4(f4_normal);
		// find the major axis

		max = -MAX_MAP_BOUNDS;
		x = -1;
		for (i = 0; i < 3; i++)
		{
			v = Fabs(f4_normal[i]);
			if (v > max)
			{
				x = i;
				max = v;
			}
		}
		if (x == -1)
			gm_error(L"baseWindingForPlane: no axis found");

		// 找到向上轴，openGL、Dx中y坐标轴表示向上
		vup = Zero<GMVec3>();
		switch (x)
		{
		case 0:
		case 2:
			vup.setY(1);
			break;
		case 1:
			vup.setZ(1);
			break;
		}

		v = Dot(vup, normal);
		vup = vup - normal * v;
		vup = Normalize(vup);
		org = normal * -dist;
		vright = Cross(vup, normal);
		vup = vup * (GMfloat)MAX_MAP_BOUNDS;
		vright = vright * (GMfloat)MAX_MAP_BOUNDS;

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

	bool chopWindingInPlace(REF BSPWinding& inout, const GMVec4& plane, GMfloat epsilon)
	{
		AlignedVector<GMfloat> dists;
		AlignedVector<GMint> sides;
		GMint counts[3];
		GMfloat dot;
		GMint j;
		GMVec3 mid;
		GMFloat4 f4_mid, fs_normal, fs_p1, fs_p2;

		GMVec3 normal = MakeVector3(plane);
		GMfloat dist = plane.getW();

		BSPWinding& in = inout;
		BSPWinding f;

		counts[0] = counts[1] = counts[2] = 0;

		// determine sides for each point
		//for (auto iter = in.p.begin(); iter != in.p.end(); iter++)
		for (auto& p : in.p)
		{
			dot = Dot(p, normal);
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
			GMVec3 p1 = in.p[i];

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
			GMVec3 p2 = in.p[(i + 1) % in.p.size()];

			dot = dists[i] / (dists[i] - dists[i + 1]);
			normal.loadFloat4(fs_normal);
			p1.loadFloat4(fs_p1);
			p2.loadFloat4(fs_p2);
			for (j = 0; j < 3; j++)
			{	// avoid round off error when possible
				if (fs_normal[j] == 1)
					f4_mid[j] = -dist;
				else if (fs_normal[j] == -1)
					f4_mid[j] = dist;
				else
					f4_mid[j] = fs_p1[j] + dot*(fs_p2[j] - fs_p1[j]);
			}

			mid.setFloat4(f4_mid);
			f.p.push_back(mid);
		}

		inout = f;
		return true;
	}

	void windingBounds(const BSPWinding& w, GMVec3& mins, GMVec3& maxs)
	{
		GMfloat v;
		GMFloat4 f4_p, f4_mins, f4_maxs;

		mins = GMVec3(MAX_MAP_BOUNDS);
		maxs = GMVec3(-MAX_MAP_BOUNDS);
		mins.loadFloat4(f4_mins);
		maxs.loadFloat4(f4_maxs);

		for (GMuint i = 0; i < w.p.size(); i++)
		{
			w.p[i].loadFloat4(f4_p);
			for (GMint j = 0; j < 3; j++)
			{
				v = f4_p[j];
				if (v < f4_mins[j])
					f4_mins[j] = v;
				if (v > f4_maxs[j])
					f4_maxs[j] = v;
			}
		}
		mins.setFloat4(f4_mins);
		maxs.setFloat4(f4_maxs);
	}

	void snapVector(GMVec3& normal)
	{
		GMint i;
		GMFloat4 f4_normal;
		normal.loadFloat4(f4_normal);

		for (i = 0; i < 3; i++)
		{
			if (fabs(f4_normal[i] - 1) < NORMAL_EPSILON)
			{
				f4_normal[0] = f4_normal[1] = f4_normal[2] = 0;
				f4_normal[i] = 1;
				break;
			}
			if (fabs(f4_normal[i] - -1) < NORMAL_EPSILON)
			{
				f4_normal[0] = f4_normal[1] = f4_normal[2] = 0;
				f4_normal[i] = -1;
				break;
			}
		}

		normal.setFloat4(f4_normal);
	}

	void addFacetBevels(PatchCollideContext& context, GMBSPFacet *facet)
	{
		GMint axis, dir, flipped;
		GMVec4 plane;
		GMfloat d;
		GMVec4 newplane;
		GMVec3 mins, maxs, vec, Vector2;

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
		GMFloat4 f4_plane, f4_mins, f4_maxs;
		mins.loadFloat4(f4_mins);
		maxs.loadFloat4(f4_maxs);
		for (axis = 0; axis < 3; axis++)
		{
			for (dir = -1; dir <= 1; dir += 2)
			{
				plane = GMVec4(0);
				plane.loadFloat4(f4_plane);

				f4_plane[axis] = dir;
				if (dir == 1)
					f4_plane[3] = -f4_maxs[axis];
				else
					f4_plane[3] = f4_mins[axis];
				plane.setFloat4(f4_plane);

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
					if (facet->numBorders > 4 + 6 + 16) gm_error(L"ERROR: too many bevels\n");
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
		GMFloat4 f4_Vector2, f4_vec;
		GMint k;
		for (j = 0; j < (GMint)w.p.size(); j++)
		{
			k = (j + 1) % w.p.size();
			vec = w.p[j] - w.p[k];
			//if it's a degenerate edge
			vec = Normalize(vec);
			if (Length(vec) < 0.5)
				continue;
			snapVector(vec);
			vec.setFloat4(f4_vec);
			for (k = 0; k < 3; k++)
			{
				if (FuzzyCompare(f4_vec[k], -1, .01f) || FuzzyCompare(f4_vec[k], 1, .01f))
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
					Vector2 = GMVec3(0);
					Vector2.loadFloat4(f4_Vector2);
					f4_Vector2[axis] = dir;
					Vector2.setFloat4(f4_Vector2);

					GMVec3 t = Cross(vec, Vector2);
					t = Normalize(t);
					if (Length(t) < 0.5)
						continue;
					plane = CombineVector4(t, plane);
					plane.setW(-Dot(w.p[j], MakeVector3(plane)));

					// if all the points of the facet winding are
					// behind this plane, it is a proper edge bevel
					GMuint l;
					for (l = 0; l < w.p.size(); l++)
					{
						d = Dot(w.p[l], MakeVector3(plane)) + plane.getW();
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
							gm_error(L"ERROR: too many bevels\n");

						facet->borderPlanes[facet->numBorders] = findPlane(context, plane, &flipped);

						for (k = 0; k < facet->numBorders; k++)
						{
							if (facet->borderPlanes[facet->numBorders] ==
								facet->borderPlanes[k]) gm_warning(L"WARNING: bevel plane already used\n");
						}

						facet->borderNoAdjust[facet->numBorders] = 0;
						facet->borderInward[facet->numBorders] = flipped;
						w2 = w;
						newplane = context.planes[facet->borderPlanes[facet->numBorders]].plane;
						if (!facet->borderInward[facet->numBorders])
							newplane = -newplane;

						if (!chopWindingInPlace(w2, newplane, 0.1f))
						{
							gm_warning(L"WARNING: addFacetBevels... invalid bevel");
							continue;
						}
						//
						facet->numBorders++;
					}
				}
			}
		}
	}

	bool validateFacet(PatchCollideContext& context, GMBSPFacet* facet)
	{
		GMVec4 plane;
		GMVec3 bounds[2];
		GMVec3 origin(0, 0, 0);

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

		GMFloat4 f4_bounds[2];
		bounds[0].loadFloat4(f4_bounds[0]);
		bounds[1].loadFloat4(f4_bounds[1]);
		for (i = 0; i < 3; i++)
		{
			if (f4_bounds[1][i] - f4_bounds[0][i] > MAX_MAP_BOUNDS)
				return false;		// we must be missing a plane
			if (f4_bounds[0][i] >= MAX_MAP_BOUNDS)
				return false;
			if (f4_bounds[1][i] <= -MAX_MAP_BOUNDS)
				return false;
		}
		return true;		// winding is fine
	}

	void patchCollideFromGrid(BSPGrid *grid, GMBSPPatchCollide *pf)
	{
		GMint i, j;
		GMVec3 p1, p2, p3;
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

				GMBSPFacet facet;
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
}

GM_PRIVATE_NAME(GMBSPPatch)::GM_PRIVATE_DESTRUCT(GMBSPPatch)
{
	for (auto patch : patches)
	{
		delete patch;
	}
}

void GMBSPPatch::alloc(GMint num)
{
	D(d);
	d->patches.resize(num);
}

GMBSP_Physics_Patch* GMBSPPatch::patches(GMint at)
{
	D(d);
	return d->patches[at];
}

void GMBSPPatch::generatePatchCollide(GMint index, GMint width, GMint height, const GMVec3* points, AUTORELEASE GMBSP_Physics_Patch* patch)
{
	D(d);

	BSPGrid grid;
	GMint i, j;

	if (width <= 2 || height <= 2 || !points)
	{
		gm_error(L"generatePatchFacets: bad parameters: (%i, %i, %p)",
			width, height, points);
	}

	if (!(width & 1) || !(height & 1))
		gm_error(L"generatePatchFacets: even sizes are invalid for quadratic meshes");

	if (width > MAX_GRID_SIZE || height > MAX_GRID_SIZE)
		gm_error(L"generatePatchFacets: source is > MAX_GRID_SIZE");

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
	GMBSPPatchCollide* pf = new GMBSPPatchCollide();
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
	d->patches[index] = patch;
}
