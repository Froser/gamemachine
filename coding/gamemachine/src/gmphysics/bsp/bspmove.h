#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include "common.h"
#include "utilities/vmath.h"
#include "bsptrace.h"
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
};

struct BSPMovePrivate
{
	BSPPhysicsWorld* world;
	CollisionObject* object;
	BSPTrace* trace;
	BSPMovement movement;
};

class BSPMove
{
	DEFINE_PRIVATE(BSPMove)

public:
	BSPMove(BSPPhysicsWorld* world, CollisionObject* obj);
	void move();

private:
	GMfloat now();
	void groundTrace();
	void walkMove();
	void airMove();
	void stepSlideMove(bool hasGravity);
	bool slideMove(bool hasGravity);
	void synchronizeMove();
	void clipVelocity(const vmath::vec3& in, const vmath::vec3& normal, vmath::vec3& out, GMfloat overbounce);
};

END_NS
#endif