#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include "common.h"
#include "utilities/vmath.h"
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
	vmath::vec3 velocity;
	vmath::vec3 origin;
	vmath::vec3 targetPosition;
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
	vmath::vec3 decomposeVelocity(const vmath::vec3& v);
	void walkMove();
	void airMove();
	void stepSlideMove(bool hasGravity);
	bool slideMove(bool hasGravity);
	void synchronizePosition();
	void clipVelocity(const vmath::vec3& in, const vmath::vec3& normal, vmath::vec3& out, GMfloat overbounce);
};

END_NS
#endif