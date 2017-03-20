#include "stdafx.h"
#include "bspphysicsworldprivate.h"

BSPPhysicsWorldPrivate::~BSPPhysicsWorldPrivate()
{
	for (auto iter = patches.begin(); iter != patches.end(); iter++)
	{
		delete *iter;
	}
}