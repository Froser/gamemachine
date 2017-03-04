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

	d.camera.properties.translation += d.camera.properties.velocity;
	//GMint node = d.world->calculateLeafNode(d.camera.properties.translation);
}

CollisionObject* BSPPhysicsWorld::find(GameObject* obj)
{
	D(d);
	// 优先查找视角位置
	if (d.camera.object == obj)
		return &d.camera;

	return nullptr;
}

void BSPPhysicsWorld::setCamera(GameObject* obj)
{
	D(d);
	CollisionObject c = { obj };
	d.camera = c;
}

// 和BSPGameWorld的calculateLeafNode原理一样
void BSPPhysicsWorld::getTouchedLeafs(GMint nodeNum)
{
	/*
	D(d);
	BSPData& bsp = d.world->bspData();
	BSPNode* node;
	BSPPlane* plane;
	while (true) {
		if (nodeNum < 0) {
			//ll->storeLeafs(ll, nodenum);
			return;
		}

		node = &bsp.nodes[nodeNum];
		/*
		plane = node->plane;
		
		s = BoxOnPlaneSide(ll->bounds[0], ll->bounds[1], plane);
		if (s == 1) {
			nodenum = node->children[0];
		}
		else if (s == 2) {
			nodenum = node->children[1];
		}
		else {
			// go down both
			CM_BoxLeafnums_r(ll, node->children[0]);
			nodenum = node->children[1];
		}
	}
	*/
}