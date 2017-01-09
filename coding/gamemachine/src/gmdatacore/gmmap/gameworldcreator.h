#ifndef __GAMEWORLD_CREATOR_H__
#define __GAMEWORLD_CREATOR_H__
#include "common.h"
BEGIN_NS

class GameWorld;
struct GMMap;
struct GameMachine;
class GameWorldCreator
{
public:
	static void createGameWorld(GameMachine* gm, GMMap* map, OUT GameWorld** gameWorld);
};

END_NS
#endif