#ifndef __CHARACTER_H__
#define __CHARACTER_H__
#include "common.h"
#include "gameobject.h"
#include "utilities/camera.h"
#include "utilities/autoptr.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class btMotionState;
class btCollisionShape;
class btKinematicCharacterController;

BEGIN_NS
class Character : public GameObject
{
public:
	Character(const btTransform& position, btScalar radius, btScalar height, btScalar stepHeight);

public:
	void setJumpSpeed(const btVector3& jumpSpeed) { m_jumpSpeed = jumpSpeed; }

	void setCanFreeMove(bool freeMove);
	void simulateCamera();
	Camera& getCamera();

	void moveFront(GMfloat distance);
	void moveRight(GMfloat distance);
	void jump();

private:
	void move();

private:
	virtual btCollisionShape* createCollisionShape() override;
	virtual void appendObjectToWorld(btDynamicsWorld* world) override;

private:
	Camera m_camera;

	btScalar m_height;
	btScalar m_radius;
	btScalar m_stepHeight;
	btVector3 m_jumpSpeed;

	btKinematicCharacterController* m_controller;
	AutoPtr<btPairCachingGhostObject> m_ghostObject;
};

END_NS
#endif