#include "stdafx.h"
#include "physicsworld.h"
#include "physicsstructs.h"

PhysicsWorld::PhysicsWorld(GameWorld* world)
{
	D(d);
	d.world = world;
}

PhysicsWorld::~PhysicsWorld()
{

}

CommandParams PhysicsWorld::makeCommand(Command cmd, vmath::vec3* list, GMuint count)
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