#include "stdafx.h"
#include "compoundconvexhullgameobject.h"

CompoundConvexHullGameObject::CompoundConvexHullGameObject(AUTORELEASE Object* object)
	: ConvexHullGameObject(object)
{
}

CompoundConvexHullGameObject::~CompoundConvexHullGameObject()
{
	for (auto iter = m_childs.begin(); iter != m_childs.end(); iter++)
	{
		if (*iter)
			delete *iter;
	}
}

GameObjectFindResults CompoundConvexHullGameObject::findChildObjectByName(const char* name)
{
	D_BASE(GameObject, d);
	GameObjectFindResults results;
	auto iter = m_nameSet.find(name);
	if (iter != m_nameSet.end())
	{
		GameObjectFindResult found = { this, (*iter).second.first, (*iter).second.second, d.collisionObject };
		results.push_back(found);
	}
	return results;
}

btCollisionShape* CompoundConvexHullGameObject::createCollisionShape()
{
	btCompoundShape* compound = new btCompoundShape();

	D_BASE(GameObject, d);
	BEGIN_FOREACH_OBJ(d.object, childObj)
	{
		btConvexHullShape* convex = new btConvexHullShape(childObj->vertices().data(), childObj->vertices().size() / 4, sizeof(Object::DataType) * 4);
		convex->optimizeConvexHull();
		compound->addChildShape(btTransform::getIdentity(), convex);
		m_childs.push_back(convex);
		m_nameSet.insert(std::make_pair(childObj->getName(), std::make_pair(childObj, convex)));
	}
	END_FOREACH_OBJ

	return compound;
}

btTransform CompoundConvexHullGameObject::getCollisionShapeTransform(btCollisionShape* shape)
{
	btConvexHullShape* convex = static_cast<btConvexHullShape*>(shape);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(convex->getUnscaledPoints()[0] * convex->getLocalScaling());
	return transform;
}

void CompoundConvexHullGameObject::removeShapeByName(const char* name)
{
	D(d);

	GameObjectFindResults found = findChildObjectByName(name);
	for (auto iter = found.begin(); iter != found.end(); iter++)
	{
		btCompoundShape* thisShape = static_cast<btCompoundShape*>(d.collisionShape.get());
		thisShape->removeChildShape((*iter).collisionShape);

		ChildObject* obj = (*iter).childObject;
		obj->setVisibility(false);
	}
}