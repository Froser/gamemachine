#include "stdafx.h"
#include "gameobjectprivate.h"
#include "gmgl/gmglgraphic_engine.h"

struct _ReachPredicator : public IEventPredicator
{
	_ReachPredicator(bool _reached)
		: reached(_reached)
	{
	}

	virtual bool eventPredicate(GameObject* source, EventItem& item) override
	{
		GameObject* target = item.targetObject;

		btTransform destTrans = target->getTransform();
		btVector3 origin = destTrans.getOrigin();

		btCollisionShape* sourceShape = source->getCollisionShape();
		btTransform myTrans = source->getCollisionObject()->getWorldTransform();
		btVector3 aabbMin, aabbMax;
		sourceShape->getAabb(myTrans, aabbMin, aabbMax);

		return (!reached) ^ ( aabbMin.x() <= origin.x() && origin.x() <= aabbMax.x()
			&& aabbMin.y() <= origin.y() && origin.y() <= aabbMax.y()
			&& aabbMin.z() <= origin.z() && origin.z() <= aabbMax.z()
			);
	}

	bool reached;
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
	predicators[EventItem::Reached] = new _ReachPredicator(true);
	predicators[EventItem::Unreached] = new _ReachPredicator(false);
}

GameObjectPrivate::~GameObjectPrivate()
{
	for (GMuint i = 0; i < EventItem::EventItemEnd; i++)
	{
		delete predicators[i];
	}
}