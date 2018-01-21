#include "stdafx.h"
#include "gmbspgameworld.h"
#include "foundation/utilities/tools.h"
#include "gmengine/gameobjects/gmspritegameobject.h"

#define EACH_PAIR_OF_ENTITY(entity, pair) GMBSPEPair* pair = entity.epairs; for (; pair; pair = pair->next)

namespace
{
	GMString getClassname(const GMBSPEntity& entity)
	{
		GMBSPEPair* e = entity.epairs;
		while (e)
		{
			if (e->key == L"classname")
				return e->value;
			e = e->next;
		}
		return "";
	}

	void import_worldspawn(const GMBSPEntity& entity, GMBSPGameWorld* world)
	{
		world->setDefaultLights();
	}

	void import_info_player_deathmatch(const GMBSPEntity& entity, GMBSPGameWorld* world)
	{
		static bool created = false; //TODO
		gm_info(L"found playerstart");

		if (created)
			return;

		BSPVector3 origin = BSPVector3(0);
		GMfloat yaw = 0;

		EACH_PAIR_OF_ENTITY(entity, e)
		{
			std::string value = e->value.toStdString();
			GMScanner s(value.c_str());
			if (e->key == L"origin")
			{
				s.nextFloat(&origin[0]);
				s.nextFloat(&origin[1]);
				s.nextFloat(&origin[2]);
			}
			else if (e->key == L"angle")
			{
				s.nextFloat(&yaw);
			}
		}

		GMSpriteGameObject* sprite = new GMSpriteGameObject(6, glm::vec3(0, 10, 0));
		sprite->setMoveSpeed(glm::vec3(193));
		sprite->setJumpSpeed(glm::vec3(0, 150, 0));
		sprite->setPhysicsObject(new GMBSPPhysicsObject());
		world->addObjectAndInit(sprite);

		GMBSPPhysicsObject* physics = gmBSPPhysicsObjectCast(sprite->getPhysicsObject());

		GMMotionStates prop = physics->getMotionStates();
		prop.transform = glm::translate(glm::vec3(origin[0], origin[2], -origin[1]));
		physics->setMotionStates(prop);
		physics->shapeProperties().stepHeight = 18.f;
		physics->shapeProperties().bounding[0] = glm::vec3(-15, -35, -15);
		physics->shapeProperties().bounding[1] = glm::vec3(15, 35, 15);
		created = true;
	}
}

void BSPGameWorldEntityReader::import(const GMBSPEntity& entity, GMBSPGameWorld* world)
{
	const GMString& classname = getClassname(entity);

	if (classname == "worldspawn")
		import_worldspawn(entity, world);
	else if (classname == "info_player_deathmatch")
		import_info_player_deathmatch(entity, world);
}