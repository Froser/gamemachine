#include "stdafx.h"
#include "gmphysicsworld.h"
#include "gmphysicsobject.h"
#include <gmgameworld.h>
#include "gmphysicsworld_p.h"

BEGIN_NS

GM_DEFINE_PROPERTY(GMPhysicsWorld, bool, Enabled, enabled)
GMPhysicsWorld::GMPhysicsWorld(GMGameWorld* world)
{
	GM_CREATE_DATA();

	D(d);
	d->world = world;
	d->world->setPhysicsWorld(this);
}

GMPhysicsWorld::~GMPhysicsWorld()
{

}

END_NS