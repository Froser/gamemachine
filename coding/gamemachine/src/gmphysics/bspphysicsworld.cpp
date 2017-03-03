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
	d.camera.properties.translation += d.camera.properties.velocity;
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