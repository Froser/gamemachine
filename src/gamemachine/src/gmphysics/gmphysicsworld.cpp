#include "stdafx.h"
#include "gmphysicsworld.h"
#include "gmphysicsstructs.h"

GMPhysicsWorld::GMPhysicsWorld(GMGameWorld* world)
{
	D(d);
	d->world = world;
}

GMPhysicsWorld::~GMPhysicsWorld()
{

}