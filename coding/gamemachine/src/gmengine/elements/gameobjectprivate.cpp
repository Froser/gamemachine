#include "stdafx.h"
#include "gameobjectprivate.h"
#include "gmgl/gmglgraphic_engine.h"

struct _RegionPredicator : public IEventPredicator
{
	virtual bool eventPredicate(GameObject* source, EventItem& item) override
	{
		GameObject* target = item.targetObject;

		// TODO:

		return true;
	}
};

GameObjectPrivate::GameObjectPrivate()
	: mass(0)
	, isDynamic(true)
	, localInertia(0, 0, 0)
	, world(nullptr)
	, collisionObject(nullptr)
	, collisionShape(nullptr)
	, animationStartTick(0)
	, animationDuration(0)
	, animationState(Stopped)
	, localScaling(btVector3(1, 1, 1))
	, id(0)
{
	transform.setIdentity();
	memset(&frictions, 0, sizeof(Frictions));
	setupPredicator();
}

void GameObjectPrivate::setupPredicator()
{
	predicators[EventItem::Region] = new _RegionPredicator();
}

GameObjectPrivate::~GameObjectPrivate()
{
	for (GMuint i = 0; i < EventItem::EventItemEnd; i++)
	{
		delete predicators[i];
	}
}