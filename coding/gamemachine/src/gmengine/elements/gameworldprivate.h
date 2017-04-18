#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include <vector>
#include "utilities/autoptr.h"
#include "gmphysics/physicsworld.h"
#include "gmdatacore/shader.h"

BEGIN_NS

class GameObject;
class Character;
class GameMachine;
struct GameWorldPrivate
{
	GameWorldPrivate();
	GameMachine* gameMachine;
	std::vector<GameObject*> shapes;
	Character* character;
	GMfloat ellapsed;
	LightInfo ambientLight;
};

END_NS
#endif