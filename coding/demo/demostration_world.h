#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>

typedef Pair<gm::GMString, gm::IGameHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> GameHandlers;

GM_PRIVATE_OBJECT(DemostrationWorld)
{
	GameHandlers handlers;
};

namespace gm
{
	class GMPhysicsWorld;
}

class DemostrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE(DemostrationWorld)

public:
	DemostrationWorld() = default;

public:
	void init(GameHandlers& handlers);

private:
	void addMenu(GameHandlerItem& item);

public:
	virtual gm::GMPhysicsWorld* physicsWorld() override { return nullptr; }

private:
	static void mouseDownCallback(GMObject*, GMObject*);
};

GM_PRIVATE_OBJECT(DemostrationEntrance)
{
	DemostrationWorld world;
};

class DemostrationEntrance : public gm::IGameHandler, public gm::IShaderLoadCallback
{
	DECLARE_PRIVATE(DemostrationEntrance)

public:
	DemostrationEntrance() = default;

public:
	inline DemostrationWorld& getWorld() { D(d); return d->world; }

	// IShaderLoadCallback
private:
	void onLoadForwardShader(const gm::GMMeshType type, gm::GMGLShaderProgram& shader) override;
	void onLoadDeferredPassShader(gm::GMGLDeferredRenderState state, gm::GMGLShaderProgram& shaderProgram) override;
	void onLoadDeferredLightPassShader(gm::GMGLShaderProgram& lightPassProgram) override;
	void onLoadEffectsShader(gm::GMGLShaderProgram& lightPassProgram) override;

	// IGameHandler
private:
	virtual void init() override;
	virtual void start() override;
	virtual void event(gm::GameMachineEvent evt) override;

};

#endif