#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "gameobjectprivate.h"
#include <vector>
#include "btBulletCollisionCommon.h"

class btDynamicsWorld;

BEGIN_NS

class GameObject
{
	DEFINE_PRIVATE(GameObject)
public:
	virtual ~GameObject() {};

public:
	void setObject(AUTORELEASE Object* obj);
	Object* getObject();
	btCollisionShape* getCollisionShape();

	btCollisionObject* getCollisionObject();
	void setCollisionObject(btCollisionObject* obj);

	bool isDynamic();
	btVector3& getLocalInertia();

	void setMass(btScalar mass);
	btScalar getMass();

	void setTransform(const btTransform& transform);
	btTransform& getTransform();

	void setLocalScaling(const btVector3& scale);

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world) = 0;

private:
	virtual btCollisionShape* createCollisionShape() = 0;
};

END_NS
#endif