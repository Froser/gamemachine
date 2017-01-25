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
	void setJumpSpeed(const btVector3& jumpSpeed);
	void setFallSpeed(GMfloat speed);

	void setCanFreeMove(bool freeMove);
	void simulateCamera();

	void setMoveSpeed(GMfloat moveSpeed);

	void moveForward();
	void moveBackward();
	void moveRight();
	void moveLeft();
	void moveCollisionObject();
	void lookUp(GMfloat degree);
	void lookRight(GMfloat degree);
	void setLookUpLimitDegree(GMfloat deg);
	void jump();
	bool isJumping();

	const PositionState& getPositionState();

	void setEyeOffset(GMfloat* offset);
	void applyEyeOffset(CameraLookAt& lookAt);

	void updateLookAt();
	CameraLookAt& getLookAt();

public:
	virtual void getReadyForRender(DrawingList& list) override;

protected:
	virtual btCollisionShape* createCollisionShape() override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;
	virtual btCollisionObject* createCollisionObject() override;

private:
	GMfloat calcMoveDistance();
	void moveForwardOrBackward(bool forward);
	void moveLeftOrRight(bool right);

private:
	btScalar m_height;
	btScalar m_radius;
	btScalar m_stepHeight;
	btVector3 m_jumpSpeed;
	btScalar m_fallSpeed;
	GMfloat m_moveSpeed;
	GMfloat m_eyeOffset[3];

	AutoPtr<btKinematicCharacterController> m_controller;
	AutoPtr<btPairCachingGhostObject> m_ghostObject;

	PositionState m_state;
	bool m_freeMove;

	btDynamicsWorld* m_dynamicWorld;

	CameraLookAt m_lookAt;
};

END_NS
#endif