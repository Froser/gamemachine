#include "stdafx.h"
#include "script_actions.h"
#include "gmengine/elements/gameobject.h"
#include "utilities/algorithm.h"
#include "../flow/gameloop.h"

Action::Action(GameObject* sourceObj, EventItem* parent)
	: m_sourceObject(sourceObj)
	, m_activateObject(nullptr)
	, m_parentEvent(parent)
{

}

Action::~Action()
{
	m_parentEvent = nullptr;
}

void Action::activate(GameObject* obj)
{
	m_activateObject = obj;
	m_transform = obj->getTransform();
}

void Action::finish()
{
	m_activateObject->deactivateAction();
}

Action_Move::Action_Move(GameObject* sourceObj, EventItem* parent, GMint duration, btVector3& to)
	: Action(sourceObj, parent)
	, m_duration(duration)
	, m_to(to)
{

}

void Action_Move::handleAction()
{
	// 判断是否已经到达了位置
	btVector3 origin = m_transform.getOrigin();
	btVector3 destination (origin[0] + m_to[0], origin[1] + m_to[1], origin[2] + m_to[2]);
	btVector3 current = m_activateObject->getRuntimeTransform().getOrigin();
	if (current == destination)
		return;

	GMfloat startTick = m_activateObject->getActionStartTick();
	GMfloat endTick = startTick + m_duration;
	InterpolationVector2 start(startTick, 0);
	GMfloat t = m_activateObject->getWorld()->getElapsed();

	GMfloat x, y, z;
	{
		InterpolationVector2 end(endTick, m_to[0]);
		x = linearFunctor(start, end, t);
	}

	{
		InterpolationVector2 end(endTick, m_to[1]);
		y = linearFunctor(start, end, t);
	}

	{
		InterpolationVector2 end(endTick, m_to[2]);
		z = linearFunctor(start, end, t);
	}

	btCollisionObject* collisionObj = m_activateObject->getCollisionObject();

	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(origin[0] + x, origin[1] + y, origin[2] + z));
	collisionObj->setWorldTransform(trans);

	if (t > endTick)
	{
		trans.setOrigin(destination);
		collisionObj->setWorldTransform(trans);
		finish();
	}
}
