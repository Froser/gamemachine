#ifndef __RIGIDGAMEOBJECT_H__
#define __RIGIDGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
BEGIN_NS

class RigidGameObject : public GameObject
{
public:
	RigidGameObject();

	void setIsSensor(bool s) { m_isSensor = s; }

public:
	virtual btCollisionObject* createCollisionObject() override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;

protected:
	bool m_isSensor;
};

END_NS
#endif