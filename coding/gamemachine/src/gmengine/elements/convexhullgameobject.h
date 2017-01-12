#ifndef __CONVEXHULLGAMEOBJECT_H__
#define __CONVEXHULLGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "utilities/autoptr.h"
#include "rigidgameobject.h"
BEGIN_NS

class Object;
class ConvexHullGameObject : public RigidGameObject
{
public:
	ConvexHullGameObject(AUTORELEASE Object* obj);

private:
	virtual btCollisionShape* createCollisionShape() override;
};

END_NS
#endif