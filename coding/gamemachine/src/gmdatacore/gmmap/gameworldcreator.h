#ifndef __GAMEWORLD_CREATOR_H__
#define __GAMEWORLD_CREATOR_H__
#include "common.h"
BEGIN_NS

class GameWorld;
class GMMap;
class IFactory;
class GameWorldCreator
{
public:
	static void createGameWorld(IFactory* factory, GMMap* map, OUT GameWorld** gameWorld);
};

END_NS
#endif