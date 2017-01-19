#ifndef __CONVEXHULLGAMEOBJECT_H__
#define __CONVEXHULLGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "utilities/autoptr.h"
#include "rigidgameobject.h"
BEGIN_NS

class ChildObject;
class ConvexHullGameObject : public RigidGameObject
{
public:
	ConvexHullGameObject(AUTORELEASE Object* obj);

public:
	virtual void getReadyForRender(DrawingList& list);

private:
	virtual btCollisionShape* createCollisionShape() override;
};

END_NS
#endif