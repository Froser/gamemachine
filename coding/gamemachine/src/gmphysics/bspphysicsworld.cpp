#include "stdafx.h"
#include "bspphysicsworld.h"
#include "gmengine/elements/bspgameworld.h"

BSPPhysicsWorld::BSPPhysicsWorld(GameWorld* world)
	: PhysicsWorld(world)
{
	D(d);
	d.world = static_cast<BSPGameWorld*>(world);
	memset(&d.camera, 0, sizeof(d.camera));
}

void BSPPhysicsWorld::simulate()
{
	D(d);
	BSPData& bsp = d.world->bspData();

	d.camera.motions.translation += d.camera.motions.velocity;

	// TODO
	// camera position test
	//TODO:
	trace(d.camera.motions.translation, 
		d.camera.motions.translation + d.camera.motions.velocity,
		d.camera.motions.translation
		);
}

CollisionObject* BSPPhysicsWorld::find(GameObject* obj)
{
	D(d);
	// 优先查找视角位置
	if (d.camera.object == obj)
		return &d.camera;

	return nullptr;
}

void BSPPhysicsWorld::initBSPPhysicsWorld()
{
	generatePhysicsPlaneData();
}

void BSPPhysicsWorld::setCamera(GameObject* obj)
{
	D(d);
	CollisionObject c;
	// Setup physical properties
	c.object = obj;
	d.camera = c;
}

void BSPPhysicsWorld::generatePhysicsPlaneData()
{
	D(d);
	BSPData& bsp = d.world->bspData();
	d.planes.resize(bsp.numplanes);
	for (GMint i = 0; i < bsp.numplanes; i++)
	{
		d.planes[i].plane = &bsp.planes[i];
		d.planes[i].planeType = PlaneTypeForNormal(bsp.planes[i].normal);
		d.planes[i].signbits = 0;
		for (GMint j = 0; j < 3; j++)
		{
			if (bsp.planes[i].normal[j] < 0)
				d.planes[i].signbits |= 1 << j;
		}
	}
}

void BSPPhysicsWorld::trace(const vmath::vec3& start, const vmath::vec3& end, const vmath::vec3& origin)
{
	// camera
	BSPTraceWork tw = { 0 };
	tw.trace.fraction = 1;
	tw.modelOrigin = origin;
	tw.contents = 0; //TODO brushmask
	tw.maxOffset = tw.size[1][0] + tw.size[1][1] + tw.size[1][2];

	/*
	offset[i] = (mins[i] + maxs[i]) * 0.5;
	tw.size[0][i] = mins[i] - offset[i];
	tw.size[1][i] = maxs[i] - offset[i];
	tw.start[i] = start[i] + offset[i];
	tw.end[i] = end[i] + offset[i];
	*/

	// tw.offsets[signbits] = vector to appropriate corner from origin
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

	if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
#if 0
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
#endif
			positionTest(tw);
	}
}

void BSPPhysicsWorld::positionTest(BSPTraceWork& tw)
{
	BSPLeafList ll;
	ll.bounds[0] = tw.start + tw.size[0] - vmath::vec3(1);
	ll.bounds[1] = tw.start + tw.size[1] + vmath::vec3(1);
	ll.lastLeaf = 0;

	getTouchedLeafs(ll, 0);

	// TODO testInLeaf
}

void BSPPhysicsWorld::getTouchedLeafs(REF BSPLeafList& leafList, int nodeNum)
{
	D(d);
	BSPData& bsp = d.world->bspData();
	while (true)
	{
		if (nodeNum < 0) {
			storeLeafs(leafList, nodeNum);
			return;
		}

		BSPNode* node = &bsp.nodes[nodeNum];
		BSPPlane* plane = &bsp.planes[node->planeNum];
		BSP_Physics_Plane* p_plane = &d.planes[node->planeNum];
		GMint p = p_plane->classifyBox(leafList.bounds[0], leafList.bounds[1]);
		if (p == 1)
		{
			nodeNum = node->children[0]; //front
		}
		else if (p == 2)
		{
			nodeNum = node->children[1]; //back
		}
		else
		{
			// go down both
			getTouchedLeafs(leafList, node->children[0]);
			nodeNum = node->children[1];
		}
	}
}

void BSPPhysicsWorld::storeLeafs(REF BSPLeafList& lst, GMint nodeNum)
{
	D(d);
	BSPData& bsp = d.world->bspData();
	GMint leafNum = -1 - nodeNum;
	if (bsp.leafs[leafNum].cluster != -1)
		lst.lastLeaf = leafNum;

	lst.list.push_back(leafNum);
}
