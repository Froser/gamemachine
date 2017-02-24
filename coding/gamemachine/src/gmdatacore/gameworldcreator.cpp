#include "stdafx.h"
#include "gameworldcreator.h"
#include "utilities/assert.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmengine/controllers/factory.h"
#include "gmengine/controllers/gamemachine.h"

// BSP Gameworld
void GameWorldCreator::createBSPGameWorld(GameMachine* gm, const char* bspPath, OUT BSPGameWorld** gameWorld)
{
	ASSERT(gameWorld);

	BSPGameWorld* world = new BSPGameWorld();
	*gameWorld = world;

	IFactory* factory = gm->getFactory();
	IGraphicEngine* engine = gm->getGraphicEngine();
	world->setGameMachine(gm);
	world->loadBSP(bspPath);
}