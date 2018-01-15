#include "stdafx.h"
#include "gmphysicsworld.h"
#include "gmphysicsobject.h"
#include <gmgameworld.h>

GMPhysicsWorld::GMPhysicsWorld(GMGameWorld* world)
{
	D(d);
	d->world = world;
	d->world->setPhysicsWorld(this);
}

GMPhysicsWorld::~GMPhysicsWorld()
{

}