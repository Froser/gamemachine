#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmbsptrace.h"
#include "gmphysics/gmphysicsworld.h"
BEGIN_NS

class GMBSPPhysicsWorld;
struct GMCollisionObject;
class GMBSPTrace;

struct GMBSPMovement
{
	GMfloat startTime;
	BSPTraceResult groundTrace;
	bool freefall;
	bool walking;
	linear_math::Vector3 velocity;
	linear_math::Vector3 origin;
	linear_math::Vector3 targetPosition;
};

struct GMBSPMoveCommand
{
	GMCommand command;
	CommandParams params;
};

GM_PRIVATE_OBJECT(GMBSPMove)
{
	bool inited;
	GMBSPPhysicsWorld* world;
	GMCollisionObject* object;
	GMBSPTrace* trace;
	GMBSPMovement movement;
	GMBSPMoveCommand moveCommand;
};

class GMBSPMove : public GMObject
{
	DECLARE_PRIVATE(GMBSPMove)

public:
	GMBSPMove(GMBSPPhysicsWorld* world, GMCollisionObject* obj);
	void move();
	void sendCommand(GMCommand cmd, const CommandParams& dataParam);

private:
	GMfloat now();
	void generateMovement();
	void groundTrace();
	void processCommand();
	void processMove();
	void processJump();
	void composeVelocityWithGravity();
	linear_math::Vector3 decomposeVelocity(const linear_math::Vector3& v);
	void walkMove();
	void airMove();
	void stepSlideMove(bool hasGravity);
	bool slideMove(bool hasGravity);
	void synchronizePosition();
	void clipVelocity(const linear_math::Vector3& in, const linear_math::Vector3& normal, linear_math::Vector3& out, GMfloat overbounce);
};

END_NS
#endif