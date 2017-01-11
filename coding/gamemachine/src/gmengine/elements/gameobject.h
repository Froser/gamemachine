#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "gameobjectprivate.h"
#include <vector>
#include "btBulletCollisionCommon.h"
#include "gmengine/controller/graphic_engine.h"

class btDynamicsWorld;

BEGIN_NS

struct Frictions;
class GameWorld;
class GameObject
{
	DEFINE_PRIVATE(GameObject)
public:
	GameObject();
	virtual ~GameObject() {};

public:
	void setObject(AUTORELEASE Object* obj);
	Object* getObject();
	btCollisionShape* getCollisionShape();

	btCollisionObject* getCollisionObject();
	void setCollisionObject(btCollisionObject* obj);

	bool isDynamic();
	btVector3& getLocalInertia();

	virtual void setMass(btScalar mass);
	btScalar getMass();

	void setTransform(const btTransform& transform);
	btTransform& getTransform();

	virtual void setLocalScaling(const btVector3& scale);

	virtual void setWorld(GameWorld* world);
	GameWorld* getWorld();

	void setFrictions(const Frictions& frictions);
	void setFrictions();

	Keyframes& getKeyframes();
	void startAnimation(GMuint duration);
	void stopAnimation();

public:
	virtual void getReadyForRender(DrawingList& list);
	virtual void appendObjectToWorld(btDynamicsWorld* world) = 0;

protected:
	virtual vmath::mat4 getAnimationMatrix();
	virtual vmath::mat4 getScalingAndTransformMatrix(btScalar glTrans[16], const btVector3& scaling);

private:
	virtual btCollisionShape* createCollisionShape() = 0;
};

END_NS
#endif