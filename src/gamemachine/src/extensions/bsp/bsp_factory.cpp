#include "stdafx.h"
#include "bsp_factory.h"
#include "render/gmbspgameworld.h"

BEGIN_NS

void GMBSPFactory::createBSPGameWorld(const IRenderContext* context, const GMString& map, OUT GMBSPGameWorld** gameWorld)
{
	GM_ASSERT(gameWorld);

	GMBSPGameWorld* world = new GMBSPGameWorld(context);
	*gameWorld = world;
	world->loadBSP(map);
}

void GMBSPFactory::createBSPGameWorld(const IRenderContext* context, const GMBuffer& buffer, OUT GMBSPGameWorld** gameWorld)
{
	GM_ASSERT(gameWorld);

	GMBSPGameWorld* world = new GMBSPGameWorld(context);
	*gameWorld = world;
	world->loadBSP(buffer);
}

END_NS