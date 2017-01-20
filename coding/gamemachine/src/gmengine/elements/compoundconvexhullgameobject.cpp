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

btCollisionShape* CompoundConvexHullGameObject::createCollisionShape()
{
	btCompoundShape* compound = new btCompoundShape();

	D_BASE(GameObject, d);
	BEGIN_FOREACH_OBJ(d.object, childObj)
	{
		btConvexHullShape* convex = new btConvexHullShape(childObj->vertices().data(), childObj->vertices().size() / 4, sizeof(Object::DataType) * 4);
		compound->addChildShape(btTransform::getIdentity(), convex);
		m_childs.push_back(convex);
		m_nameSet.insert(std::make_pair(childObj->getName(), convex));
	}
	END_FOREACH_OBJ

	return compound;
}