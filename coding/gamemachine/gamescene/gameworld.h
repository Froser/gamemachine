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
	virtual void getBufferedObject(GameObject* obj, Object** out) = 0;
};

class GLGameWorldRenderCallback : public IGameWorldRenderCallback
{
public:
	void renderObject(btScalar* trans, GameObject* obj) override;
	void getBufferedObject(GameObject* obj, Object** out) override;
};

class GameWorld
{
	DEFINE_PRIVATE(GameWorld)
public:
	GameWorld();

public:
	void appendObject(GameObject* obj);
	void renderGameWorld(GMint fps);

private:
	AutoPtr<IGameWorldRenderCallback> m_pCallback;
};

END_NS
#endif