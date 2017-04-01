#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include "common.h"
#include "utilities/vmath.h"
BEGIN_NS

class BSPPhysicsWorld;
struct CollisionObject;
struct BSPMovePrivate
{
	BSPPhysicsWorld* world;
	CollisionObject* object;
};

class BSPMove
{
	DEFINE_PRIVATE(BSPMove)

public:
	BSPMove(BSPPhysicsWorld* world, CollisionObject* obj);
	void slideMove(bool hasGravity);

private:
	void clipVelocity(const vmath::vec3& in, const vmath::vec3& normal, vmath::vec3& out, GMfloat overbounce);
};

END_NS
#endif