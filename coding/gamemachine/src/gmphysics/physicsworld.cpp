#include "stdafx.h"
#include "physicsworld.h"

PhysicsWorld::PhysicsWorld(GameWorld* world)
{
	D(d);
	d.world = world;
}

PhysicsWorld::~PhysicsWorld()
{

}