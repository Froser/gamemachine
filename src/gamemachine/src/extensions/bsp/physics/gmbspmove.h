#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include <gmcommon.h>
#include <linearmath.h>
#include "gmbsptrace.h"
#include "gmphysics/gmphysicsworld.h"
BEGIN_NS

class GMBSPPhysicsWorld;
class GMPhysicsObject;
class GMBSPTrace;

GM_ALIGNED_STRUCT(GMBSPMovement)
{
	GMfloat startTime = 0;
	BSPTraceResult groundTrace;
	bool freefall = false;
	bool walking = false;
	glm::vec3 velocity = glm::zero<glm::vec3>(); //!< 移动速度，相对于整个场景世界
	glm::vec3 origin = glm::zero<glm::vec3>();
	glm::vec3 targetPosition = glm::zero<glm::vec3>();
};

GM_ALIGNED_STRUCT(GMBSPJumpArgs)
{
	glm::vec3 speed;
	bool jumped = false;
};

GM_ALIGNED_STRUCT(GMBSPMoveArgs)
{
	bool moved = false;
	GMPhysicsMoveArgs args;
};

GM_ALIGNED_STRUCT(GMBSPAction)
{
	GMBSPMoveArgs move;
	GMBSPJumpArgs jump;
};

GM_PRIVATE_OBJECT(GMBSPMove)
{
	bool inited = false;
	GMBSPPhysicsWorld* world = nullptr;
	GMPhysicsObject* object = nullptr;
	GMBSPTrace* trace = nullptr;
	GMBSPMovement movementState;
	GMBSPAction action;
	glm::vec3 initialVelocity = glm::zero<glm::vec3>();
};

class GMBSPMove : public GMObject
{
	DECLARE_PRIVATE(GMBSPMove)

public:
	GMBSPMove(GMBSPPhysicsWorld* world, GMPhysicsObject* obj);
	void move();
	void applyMove(const GMPhysicsMoveArgs& args);
	void applyJump(const glm::vec3& speed);

private:
	GMfloat now();
	void generateMovement();
	void groundTrace();
	void processCommand();
	void processMove();
	void processJump();
	void composeVelocityWithGravity();
	glm::vec3 decomposeVelocity(const glm::vec3& v);
	void walkMove();
	void airMove();
	void stepSlideMove(bool hasGravity);
	bool slideMove(bool hasGravity);
	void synchronizeMotionStates();
	void clipVelocity(const glm::vec3& in, const glm::vec3& normal, glm::vec3& out, GMfloat overbounce);
};

END_NS
#endif