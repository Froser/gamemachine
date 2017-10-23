#include "stdafx.h"
#include "gmbspgameworld.h"
#include "foundation/utilities/utilities.h"
#include "gmengine/gameobjects/gmspritegameobject.h"

#define EACH_PAIR_OF_ENTITY(entity, pair) GMBSPEPair* pair = entity.epairs; for (; pair; pair = pair->next)

static GMString getClassname(const GMBSPEntity& entity)
{
	GMBSPEPair* e = entity.epairs;
	while (e)
	{
		if (e->key == "classname")
			return e->value;
		e = e->next;
	}
	return "";
}

void import_worldspawn(const GMBSPEntity& entity, GMBSPGameWorld* world)
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
	{
		GMLight ambientLight(GMLightType::AMBIENT);
		GMfloat lightColor[] = { .9f, .9f, .9f };
		ambientLight.setLightColor(lightColor);
		world->addLight(ambientLight);
	}

	{
		GMLight specularLight(GMLightType::SPECULAR);
		GMfloat lightColor[] = { 1, 1, 1 };
		specularLight.setLightColor(lightColor);
		world->addLight(specularLight);
	}
}

void import_info_player_deathmatch(const GMBSPEntity& entity, GMBSPGameWorld* world)
{
	static bool created = false; //TODO
	gm_info(_L("found playerstart"));

	if (created)
		return;

	BSPVector3 origin = BSPVector3(0);
	GMfloat yaw = 0;

	EACH_PAIR_OF_ENTITY(entity, e)
	{
		std::string value = e->value.toStdString();
		Scanner s(value.c_str());
		if (e->key == "origin")
		{
			s.nextFloat(&origin[0]);
			s.nextFloat(&origin[1]);
			s.nextFloat(&origin[2]);
		}
		else if (e->key == "angle")
		{
			s.nextFloat(&yaw);
		}
	}

	linear_math::Vector3 playerStart (origin[0], origin[2], -origin[1]);

	GMSpriteGameObject* sprite = new GMSpriteGameObject(6);
	world->appendObjectAndInit(sprite);
	sprite->setMoveSpeed(192);
	sprite->setJumpSpeed(linear_math::Vector3(0, 150, 0));

	GMMotionProperties& prop = world->physicsWorld()->find(sprite)->motions;
	prop.translation = playerStart;
	created = true;
}

void BSPGameWorldEntityReader::import(const GMBSPEntity& entity, GMBSPGameWorld* world)
{
	const GMString& classname = getClassname(entity);

	if (classname == "worldspawn")
		import_worldspawn(entity, world);
	else if (classname == "info_player_deathmatch")
		import_info_player_deathmatch(entity, world);
}