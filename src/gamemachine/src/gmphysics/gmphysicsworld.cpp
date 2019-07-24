#include "stdafx.h"
#include "gmphysicsworld.h"
#include "gmphysicsobject.h"
#include <gmgameworld.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMPhysicsWorld)
{
	GMGameWorld* world;
	GMfloat gravity;
	bool enabled = true;
};

GM_DEFINE_PROPERTY(GMPhysicsWorld, bool, Enabled, enabled)
GMPhysicsWorld::GMPhysicsWorld(GMGameWorld* world)
{
	GM_CREATE_DATA(GMPhysicsWorld);

	D(d);
	d->world = world;
	d->world->setPhysicsWorld(this);
}

GMPhysicsWorld::~GMPhysicsWorld()
{

}

END_NS