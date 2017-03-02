#include "stdafx.h"
#include "physicsworld.h"
#include "gmengine/elements/gameobject.h"
#include "gmengine/elements/character.h"

void PhysicsWorld::addToPhysics(GameObject* obj)
{
	D(d);

	//TEST
	if (dynamic_cast<Character*>(obj))
	{
		CollisionObject co = { obj, vmath::vec3(0, 0, 0), vmath::vec3(0, 0, 0) };
		d.camera = co;
		return;
	}

	CollisionObject co = { obj, vmath::vec3(0, 0, 0), vmath::vec3(0, 0, 0) };

	ASSERT(d.tree.find(obj) == d.tree.end());
	d.tree.insert(std::make_pair(obj, co));
}

void PhysicsWorld::simulate()
{
	// test
	D(d);
	d.camera.translate += d.camera.velocity;
}

CollisionObject* PhysicsWorld::find(GameObject* o)
{
	D(d);
	if (o == d.camera.object)
		return &d.camera;

	auto c = d.tree.find(o);
	if (c == d.tree.end())
		return nullptr;
	
	return &(c->second);
}