#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include <vector>
#include "utilities/autoptr.h"
#include "gmphysics/physicsworld.h"

BEGIN_NS

class GameObject;
class Character;
class GameMachine;
class GameLight;
struct GameWorldPrivate
{
	GameWorldPrivate();
	GameMachine* gameMachine;
	std::vector<GameObject*> shapes;
	std::vector<GameLight*> lights;
	Character* character;
	GMfloat ellapsed;
};

END_NS
#endif