#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__
#include "common.h"
#include "gameworldprivate.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

BEGIN_NS
class Object;
struct IGameWorldRenderCallback
{
	virtual void renderObject(btScalar* trans, GameObject* obj) = 0;
	virtual void createObject(GameObject* obj, Object** out) = 0;
};

class GLGameWorldRenderCallback : public IGameWorldRenderCallback
{
public:
	void renderObject(btScalar* trans, GameObject* obj) override;
	void createObject(GameObject* obj, Object** out) override;
};

class Character;
class GameWorld
{
	DEFINE_PRIVATE(GameWorld)
public:
	GameWorld();

public:
	void appendObject(GameObject* obj);
	void setMajorCharacter(Character* character);
	Character* getMajorCharacter();
	void simulateGameWorld(GMint fps);
	void renderGameWorld();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);

private:
	AutoPtr<IGameWorldRenderCallback> m_pCallback;
};

END_NS
#endif