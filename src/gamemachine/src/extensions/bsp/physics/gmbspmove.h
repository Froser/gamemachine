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
	GMVec3 velocity = Zero<GMVec3>(); //!< 移动速度，相对于整个场景世界
	GMVec3 origin = Zero<GMVec3>();
	GMVec3 targetPosition = Zero<GMVec3>();
};

GM_ALIGNED_STRUCT(GMBSPJumpArgs)
{
	GMVec3 speed;
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
	GMVec3 initialVelocity = Zero<GMVec3>();
};

class GMBSPMove : public GMObject
{
	GM_DECLARE_PRIVATE(GMBSPMove)

public:
	GMBSPMove(GMBSPPhysicsWorld* world, GMPhysicsObject* obj);
	void move(GMDuration dt);
	void applyMove(const GMPhysicsMoveArgs& args);
	void applyJump(const GMVec3& speed);

private:
	GMfloat now();
	void generateMovement();
	void groundTrace();
	void processCommand();
	void processMove();
	void processJump();
	void composeVelocityWithGravity();
	GMVec3 decomposeVelocity(const GMVec3& v);
	void walkMove(GMDuration dt);
	void airMove(GMDuration dt);
	void stepSlideMove(GMDuration dt, bool hasGravity);
	bool slideMove(GMDuration dt, bool hasGravity);
	void synchronizeMotionStates();
	void clipVelocity(const GMVec3& in, const GMVec3& normal, GMVec3& out, GMfloat overbounce);
};

END_NS
#endif