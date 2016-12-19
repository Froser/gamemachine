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
	void setObject(Object* obj);
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

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world) = 0;
	virtual void drawObject() = 0;

private:
	virtual btCollisionShape* createCollisionShape() = 0;
};

class GLCubeGameObject : public GameObject
{
public:
	GLCubeGameObject(GMfloat size, const btTransform& position, GMfloat* color);

public:
	void drawObject();
	void setExtents(const btVector3& extents);
	btVector3& getExtents();

public:
	virtual void appendObjectToWorld(btDynamicsWorld* world);

private:
	btCollisionShape* createCollisionShape();

private:
	GMfloat m_color[3];
	GMfloat m_size;
	btVector3 m_extents;
};

END_NS
#endif