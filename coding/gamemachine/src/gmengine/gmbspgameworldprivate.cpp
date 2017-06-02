#include "stdafx.h"
#include "gmbspgameworld.h"
#include "gmcharacter.h"
#include "foundation/utilities/utilities.h"

#define PARSE_FUNC(name, entity, world) void import_##name(const GMBSPEntity& entity, GMBSPGameWorld* world)
#define BEGIN_PARSE_CLASS(value) if (strEqual(classname, #value)) import_##value(entity, world)
#define PARSE_CLASS(value) else if (strEqual(classname, #value)) import_##value(entity, world)
#define END_PARSE_CLASS
#define EACH_PAIR_OF_ENTITY(entity, pair) GMBSPKeyValuePair* pair = entity.epairs; for (; pair; pair = pair->next)
#define SAME_KEY(pair, k) strEqual(pair->key, k)
#define SAME_VALUE(pair, v) strEqual(pair->value, v)

static const char* getClassname(const GMBSPEntity& entity)
{
	GMBSPKeyValuePair* e = entity.epairs;
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
				ambientLight->setColor(linear_math::Vector3(1, 1, 1));
				ambientLight->setPosition(linear_math::Vector3(0, 0, 0));
				ambientLight->setWorld(world);
				ambientLight->setShadowSource(false);
				world->appendLight(ambientLight);
			}
		}
	}
	*/
	LightInfo ambientLight;
	ambientLight.on = true;
	ambientLight.lightColor = linear_math::Vector3(.5f, .5f, .5f);
	for (GMint i = 0; i < 3; i++)
		ambientLight.args[LA_KA + i] = 1.f;
	world->setDefaultAmbientLight(ambientLight);
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

	linear_math::Vector3 playerStart (origin[0], origin[2], -origin[1]);

	GMCharacter* character = new GMCharacter(6); 
	world->appendObjectAndInit(character);
	world->setMajorCharacter(character);
	character->setMoveSpeed(192);
	character->setJumpSpeed(linear_math::Vector3(0, 150, 0));

	GMMotionProperties& prop = world->physicsWorld()->find(character)->motions;
	prop.translation = playerStart;
}

void BSPGameWorldEntityReader::import(const GMBSPEntity& entity, GMBSPGameWorld* world)
{
	const char* classname = getClassname(entity);

	BEGIN_PARSE_CLASS(worldspawn);
	PARSE_CLASS(info_player_deathmatch);
	END_PARSE_CLASS;
}