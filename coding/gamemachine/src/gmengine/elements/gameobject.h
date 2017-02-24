#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include "common.h"
#include "gameobjectprivate.h"
#include <vector>
#include "btBulletCollisionCommon.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gameworld.h"

class btDynamicsWorld;

BEGIN_NS
struct AnimationMatrices
{
	vmath::mat4 rotation;
	vmath::mat4 tranlation;
	vmath::mat4 scaling;
};

struct Frictions;
class GameWorld;
class GameObject
{
	DEFINE_PRIVATE(GameObject)
public:
	GameObject();
	virtual ~GameObject() {};

public:
	virtual void initPhysics(btDynamicsWorld* world);

	void setId(GMuint id);
	GMuint getId();

	void setObject(AUTORELEASE Object* obj);
	Object* getObject();
	btCollisionObject* getCollisionObject();
	btCollisionShape* getCollisionShape();

	bool isDynamic();

	virtual void setMass(btScalar mass);
	btScalar getMass();

	void setTransform(const btTransform& transform);
	btTransform& getTransform();
	virtual btTransform getRuntimeTransform();

	virtual void setLocalScaling(const btVector3& scale);

	virtual void setWorld(GameWorld* world);
	GameWorld* getWorld();

	void setFrictions(const Frictions& frictions);
	void setFrictions();

	void appendObjectToWorld(btDynamicsWorld* world);

	Keyframes& getKeyframesRotation();
	Keyframes& getKeyframesTranslation();
	Keyframes& getKeyframesScaling();
	void startAnimation(GMuint duration);
	void stopAnimation();

public:
	virtual void getReadyForRender(DrawingList& list);
	virtual btCollisionObject* createCollisionObject() = 0;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) = 0;

protected:
	virtual AnimationMatrices getAnimationMatrix();
	virtual vmath::mat4 getTransformMatrix(btScalar glTrans[16]);
	virtual void initPhysicsAfterCollisionObjectCreated();

private:
	virtual btCollisionShape* createCollisionShape() = 0;
};

END_NS
#endif