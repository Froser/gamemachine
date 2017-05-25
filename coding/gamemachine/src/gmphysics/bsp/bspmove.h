#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "bsptrace.h"
#include "gmphysics/physicsworld.h"
BEGIN_NS

class BSPPhysicsWorld;
struct CollisionObject;
class BSPTrace;

struct BSPMovement
{
	GMfloat startTime;
	BSPTraceResult groundTrace;
	bool freefall;
	bool walking;
	linear_math::Vector3 velocity;
	linear_math::Vector3 origin;
	linear_math::Vector3 targetPosition;
};

struct BSPMoveCommand
{
	Command command;
	CommandParams params;
};

struct BSPMovePrivate
{
	bool inited;
	BSPPhysicsWorld* world;
	CollisionObject* object;
	BSPTrace* trace;
	BSPMovement movement;
	BSPMoveCommand moveCommand;
};

class BSPMove
{
	GM_DECLARE_ALIGNED_ALLOCATOR();
	DEFINE_PRIVATE(BSPMove)

public:
	BSPMove(BSPPhysicsWorld* world, CollisionObject* obj);
	void move();
	void sendCommand(Command cmd, const CommandParams& dataParam);

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