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
typedef GMbyte MoveAction;
enum 
{
	MD_FORWARD = 1,
	MD_BACKWARD = 2,
	MD_LEFT = 4,
	MD_RIGHT = 8,
	MD_JUMP = 16,

	// 以上MoveAction数目
	MD_ACTION_COUNT = 5,
};

class MoveRate
{
public:
	MoveRate()
	{
		for (GMuint i = 0; i < MD_ACTION_COUNT; i++)
		{
			m_moveRate[i] = 1;
		}
	}

	void setMoveRate(MoveAction action, GMfloat rate)
	{
		m_moveRate[(GMint)std::log((GMfloat)action)] = rate;
	}

	GMfloat getMoveRate(MoveAction action)
	{
		return m_moveRate[(GMint)std::log((GMfloat)action)];
	}

private:
	GMfloat m_moveRate[MD_ACTION_COUNT];
};

class Character : public GameObject
{
public:
	Character(const btTransform& position, btScalar radius, btScalar height, btScalar stepHeight);

public:
	void setJumpSpeed(const vmath::vec3& jumpSpeed);
	void setFallSpeed(GMfloat speed);

	void setCanFreeMove(bool freeMove);
	void setMoveSpeed(GMfloat moveSpeed);
	void simulation();
	void action(MoveAction md, MoveRate rate);
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
	GMfloat calcMoveDistance(GMfloat rate);
	GMfloat calcFallSpeed();
	void moveForwardOrBackward(bool forward);
	void moveLeftOrRight(bool left);
	void update();
	void applyWalkDirection();

private:
	GMfloat m_height;
	GMfloat m_radius;
	GMfloat m_stepHeight;
	vmath::vec3 m_jumpSpeed;
	btScalar m_fallSpeed;
	GMfloat m_moveSpeed;
	GMfloat m_eyeOffset[3];
	Frustum m_frustum;
	GMfloat m_walkDirectionFB[3];
	GMfloat m_walkDirectionLR[3];
	MoveAction m_moveDirection;
	MoveRate m_moveRate;

	AutoPtr<btKinematicCharacterController> m_controller;
	AutoPtr<btPairCachingGhostObject> m_ghostObject;

	PositionState m_state;
	bool m_freeMove;

	btDynamicsWorld* m_dynamicWorld;

	CameraLookAt m_lookAt;
};

END_NS
#endif