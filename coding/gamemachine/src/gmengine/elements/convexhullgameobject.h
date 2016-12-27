#ifndef __CONVEXHULLGAMEOBJECT_H__
#define __CONVEXHULLGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "utilities/autoptr.h"
BEGIN_NS

class Object;
class ConvexHullGameObject : public GameObject
{
public:
	ConvexHullGameObject(AUTORELEASE Object* obj);

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world) override;

private:
	virtual btCollisionShape* createCollisionShape() override;
};

END_NS
#endif