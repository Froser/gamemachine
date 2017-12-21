#ifndef __BSPMOVE_H__
#define __BSPMOVE_H__
#include <gmcommon.h>
#include <linearmath.h>
#include "gmbsptrace.h"
#include "gmphysics/gmphysicsworld.h"
BEGIN_NS

class GMBSPPhysicsWorld;
struct GMCollisionObject;
class GMBSPTrace;

struct GMBSPMovement
{
	GMfloat startTime = 0;
	BSPTraceResult groundTrace;
	bool freefall = false;
	bool walking = false;
	glm::vec3 velocity{ 0 };
	glm::vec3 origin{ 0 };
	glm::vec3 targetPosition{ 0 };
};

struct GMBSPMoveCommand
{
	GMCommand command;
	CommandParams params;
};

GM_PRIVATE_OBJECT(GMBSPMove)
{
	bool inited;
	GMBSPPhysicsWorld* world = nullptr;
	GMCollisionObject* object = nullptr;
	GMBSPTrace* trace = nullptr;
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