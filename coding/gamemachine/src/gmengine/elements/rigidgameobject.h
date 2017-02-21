#ifndef __RIGIDGAMEOBJECT_H__
#define __RIGIDGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
BEGIN_NS

class RigidGameObject : public GameObject
{
public:
	RigidGameObject();

public:
	virtual btCollisionObject* createCollisionObject() override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;
};

END_NS
#endif