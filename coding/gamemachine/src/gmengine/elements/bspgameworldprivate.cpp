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
	/*
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
				ambientLight->setColor(vmath::vec3(1, 1, 1));
				ambientLight->setPosition(vmath::vec3(0, 0, 0));
				ambientLight->setWorld(world);
				ambientLight->setShadowSource(false);
				world->appendLight(ambientLight);
			}
		}
	}
	*/
	GameLight* ambientLight;
	IFactory* factory = world->getGameMachine()->getFactory();
	factory->createLight(Ambient, &ambientLight);
	if (ambientLight)
	{
		ambientLight->setId(0);
		ambientLight->setColor(vmath::vec3(1, 1, 1));
		ambientLight->setPosition(vmath::vec3(0, 0, 0));
		ambientLight->setWorld(world);
		ambientLight->setShadowSource(false);
		world->appendLight(ambientLight);
	}
}

PARSE_FUNC(info_player_deathmatch, entity, world)
{
	gm_info("found playerstart");

	if (world->getMajorCharacter())
		return;

	BSPVector3 origin = BSPVector3(0);
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

	vmath::vec3 playerStart (origin[0], origin[2], -origin[1]);

	Character* character = new Character(6); 
	world->appendObjectAndInit(character);
	world->setMajorCharacter(character);
	character->setMoveSpeed(192);
	character->setJumpSpeed(vmath::vec3(0, 150, 0));

	MotionProperties& prop = world->physicsWorld()->find(character)->motions;
	prop.translation = playerStart;
}

void BSPGameWorldEntityReader::import(const BSPEntity& entity, BSPGameWorld* world)
{
	const char* classname = getClassname(entity);

	BEGIN_PARSE_CLASS(worldspawn);
	PARSE_CLASS(info_player_deathmatch);
	END_PARSE_CLASS;
}