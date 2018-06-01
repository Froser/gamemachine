#include "stdafx.h"
#include "bsp_factory.h"
#include "render/gmbspgameworld.h"

void GMBSPFactory::createBSPGameWorld(const GMContext* context, const GMString& map, OUT GMBSPGameWorld** gameWorld)
{
	GM_ASSERT(gameWorld);

	GMBSPGameWorld* world = new GMBSPGameWorld(context);
	*gameWorld = world;
	world->loadBSP(map);
}