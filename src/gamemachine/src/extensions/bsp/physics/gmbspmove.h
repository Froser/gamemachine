#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include <gmcommon.h>
#include <linearmath.h>
#include "gmbsptrace.h"
#include "gmphysics/gmphysicsworld.h"
BEGIN_NS

class GMBSPPhysicsWorld;
struct GMPhysicsObject;
class GMBSPTrace;

GM_ALIGNED_STRUCT(GMBSPMovement)
{
	GMfloat startTime = 0;
	BSPTraceResult groundTrace;
	bool freefall = false;
	bool walking = false;
	glm::vec3 velocity = glm::zero<glm::vec3>();
	glm::vec3 origin = glm::zero<glm::vec3>();
	glm::vec3 targetPosition = glm::zero<glm::vec3>();
};

GM_ALIGNED_STRUCT(GMBSPMoveCommand)
{

};

GM_PRIVATE_OBJECT(GMBSPMove)
{
	bool inited;
	GMBSPPhysicsWorld* world = nullptr;
	GMPhysicsObject* object = nullptr;
	GMBSPTrace* trace = nullptr;
	GMBSPMovement movement;
	GMBSPMoveCommand moveCommand;
};

class GMBSPMove : public GMObject
{
	DECLARE_PRIVATE(GMBSPMove)

public:
	GMBSPMove(GMBSPPhysicsWorld* world, GMPhysicsObject* obj);
	void move();
	void applyMove(const GMPhysicsObject& phy, const GMPhysicsMoveArgs& args);
	void applyJump(const GMPhysicsObject& phy);

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
	void synchronizePosition();
	void clipVelocity(const glm::vec3& in, const glm::vec3& normal, glm::vec3& out, GMfloat overbounce);
};

END_NS
#endif