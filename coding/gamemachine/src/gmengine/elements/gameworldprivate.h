#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include <set>
#include "utilities/autoptr.h"
#include "gmphysics/physicsworld.h"
#include "gmdatacore/shader.h"

BEGIN_NS

class GameObject;
class Character;
class GameMachine;
class GamePackage;
struct GameWorldPrivate
{
	GameWorldPrivate();
	GamePackage* gamePackage;
	GameMachine* gameMachine;
	std::set<GameObject*> shapes;
	Character* character;
	GMfloat ellapsed;
	LightInfo ambientLight;
};

END_NS
#endif