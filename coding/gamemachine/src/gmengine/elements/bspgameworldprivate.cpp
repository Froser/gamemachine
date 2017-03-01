#include "stdafx.h"
#include "bspgameworld.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmengine/controllers/factory.h"
#include "gmengine/elements/gamelight.h"
#include "character.h"
#include "utilities/scanner.h"

#define PARSE_FUNC(name, entity, world) void import_##name(const BSPEntity& entity, BSPGameWorld* world)
#define BEGIN_PARSE_CLASS(value) if (strEqual(classname, #value)) import_##value(entity, world)
#define PARSE_CLASS(value) else if (strEqual(classname, #value)) import_##value(entity, world)
#define END_PARSE_CLASS
#define EACH_PAIR_OF_ENTITY(entity, pair) BSPKeyValuePair* pair = entity.epairs; for (; pair; pair = pair->next)
#define SAME_KEY(pair, k) strEqual(pair->key, k)
#define SAME_VALUE(pair, v) strEqual(pair->value, v)

static const char* getClassname(const BSPEntity& entity)
{
	BSPKeyValuePair* e = entity.epairs;
	while (e)
	{
		if (strEqual(e->key, "classname"))
			return e->value;
		e = e->next;
	}
	return nullptr;
}

PARSE_FUNC(worldspawn, entity, world)
{
	EACH_PAIR_OF_ENTITY(entity, e)
	{
		if (SAME_KEY(e, "ambient")); //nothing
		else if (SAME_KEY(e, "_color"))
		{
			GameLight* ambientLight;
			IFactory* factory = world->getGameMachine()->getFactory();
			factory->createLight(Ambient, &ambientLight);
			if (ambientLight)
			{
				ambientLight->setId(0);
				ambientLight->setColor(btVector3(1, 1, 1));
				ambientLight->setPosition(btVector3(0, 0, 0));
				ambientLight->setWorld(world);
				ambientLight->setShadowSource(false);
				world->appendLight(ambientLight);
			}
		}
	}

	world->setGravity(0, 0, 0);
}

PARSE_FUNC(info_player_deathmatch, entity, world)
{
	gm_info("found playerstart");

	if (world->getMajorCharacter())
		return;

	BSPVector3 origin = { 0, 0, 0 };
	GMfloat yaw = 0;

	EACH_PAIR_OF_ENTITY(entity, e)
	{
		Scanner s(e->value);
		if (SAME_KEY(e, "origin"))
		{
			s.nextFloat(&origin[0]);
			s.nextFloat(&origin[1]);
			s.nextFloat(&origin[2]);
		}
		else if (SAME_KEY(e, "angle"))
		{
			s.nextFloat(&yaw);
		}
	}

	btTransform playerStart;
	playerStart.setIdentity();
	vmath::vec3 center = (world->bspData().boundMax + world->bspData().boundMin) / 2;
	playerStart.setOrigin(btVector3(center[0], 1.8, center[2]));
	//playerStart.setOrigin(btVector3(0, 0.875,0));
	//btQuaternion rotation(btVector3(0, 1, 0), RAD(yaw));
	//playerStart.setRotation(rotation);

	Character* character = new Character(playerStart, .6, .1, .1);
	character->setMoveSpeed(3);
	character->setFallSpeed(250);
	character->setJumpSpeed(vmath::vec3(0, 50, 0));
	character->setCanFreeMove(true);
	world->appendObjectAndInit(character);
	world->setMajorCharacter(character);
}

void BSPGameWorldEntityReader::import(const BSPEntity& entity, BSPGameWorld* world)
{
	const char* classname = getClassname(entity);

	BEGIN_PARSE_CLASS(worldspawn);
	PARSE_CLASS(info_player_deathmatch);
	END_PARSE_CLASS;
}