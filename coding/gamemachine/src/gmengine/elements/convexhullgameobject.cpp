#include "stdafx.h"
#include "convexhullgameobject.h"
#include "gmdatacore/object.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

ConvexHullGameObject::ConvexHullGameObject(AUTORELEASE Object* obj)
{
	setObject(obj);
}

btCollisionShape* ConvexHullGameObject::createCollisionShape()
{
	return new btConvexHullShape(getObject()->vertices().data(), getObject()->vertices().size() / 4, sizeof(Object::DataType) * 4 );
}