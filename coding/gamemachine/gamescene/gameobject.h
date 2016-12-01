#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "gameobjectprivate.h"
#include <vector>
#include "btBulletCollisionCommon.h"
BEGIN_NS

class GameObject
{
	DEFINE_PRIVATE(GameObject)
public:
	virtual ~GameObject() {};

public:
	void setObject(Object* obj);
	Object* getObject();
	btCollisionShape* getCollisionShape();
	void setMass(btScalar mass);
	btScalar getMass();
	bool isDynamic();
	btVector3& getLocalInertia();

public:
	virtual btMotionState* createMotionState() = 0;
	virtual void drawObject() = 0;

private:
	virtual btCollisionShape* createCollisionShape() = 0;
};

class GLCubeGameObject : public GameObject
{
public:
	GLCubeGameObject(GMfloat width, GMfloat height, GMfloat depth, const btTransform& position);

private:
	btVector3 m_extents;
	btTransform m_transform;
};

END_NS
#endif