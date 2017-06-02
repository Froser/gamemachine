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

CommandParams GMPhysicsWorld::makeCommand(GMCommand cmd, GMCommandVector3* list, GMuint count)
{
	CommandParams p;
	if (!list)
	{
		p[cmd];
	}
	else
	{
		for (GMuint i = 0; i < count; i++)
		{
			p[cmd].push_back(list[i]);
		}
	}
	return p;
}