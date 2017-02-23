#ifndef __CHARACTER_H__
#define __CHARACTER_H__
#include "common.h"
#include "gameobject.h"
#include "utilities/camera.h"
#include "utilities/autoptr.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "utilities/frustum.h"

class btMotionState;
class btCollisionShape;
class btKinematicCharacterController;

BEGIN_NS

// bits:  r, l, b, f
typedef GMbyte MoveDirection;
enum
{
	MD_FORWARD = 1,
	MD_BACKWARD = 2,
	MD_LEFT = 4,
	MD_RIGHT = 8,
	MD_JUMP = 16,
};

class Character : public GameObject
{
public:
	Character(const btTransform& position, btScalar radius, btScalar height, btScalar stepHeight);

public:
	void setJumpSpeed(const btVector3& jumpSpeed);
	void setFallSpeed(GMfloat speed);

	void setCanFreeMove(bool freeMove);
	void setMoveSpeed(GMfloat moveSpeed);
	void simulation();
	void action(MoveDirection md);
	void lookUp(GMfloat degree);
	void lookRight(GMfloat degree);
	void setPitchLimitDegree(GMfloat deg);
	const PositionState& getPositionState();
	void setEyeOffset(GMfloat* offset);
	void updateCamera();
	CameraLookAt& getLookAt();
	Frustum& getFrustum();

public:
	virtual void getReadyForRender(DrawingList& list) override;

protected:
	virtual btCollisionShape* createCollisionShape() override;
	virtual void appendThisObjectToWorld(btDynamicsWorld* world) override;
	virtual btCollisionObject* createCollisionObject() override;

private:
	GMfloat calcMoveDistance();
	void moveForwardOrBackward(bool forward);
	void moveLeftOrRight(bool left);
	void update();
	void applyWalkDirection();

private:
	btScalar m_height;
	btScalar m_radius;
	btScalar m_stepHeight;
	btVector3 m_jumpSpeed;
	btScalar m_fallSpeed;
	GMfloat m_moveSpeed;
	GMfloat m_eyeOffset[3];
	Frustum m_frustum;
	GMfloat m_walkDirectionFB[3];
	GMfloat m_walkDirectionLR[3];
	MoveDirection m_moveDirection;

	AutoPtr<btKinematicCharacterController> m_controller;
	AutoPtr<btPairCachingGhostObject> m_ghostObject;

	PositionState m_state;
	bool m_freeMove;

	btDynamicsWorld* m_dynamicWorld;

	CameraLookAt m_lookAt;
};

END_NS
#endif