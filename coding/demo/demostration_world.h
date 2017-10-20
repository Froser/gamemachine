#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>

typedef Pair<gm::GMString, gm::IGameHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> GameHandlers;

GM_PRIVATE_OBJECT(DemostrationWorld)
{
	GameHandlers demos;
	gm::IGameHandler* currentDemo = nullptr;
};

namespace gm
{
	class GMPhysicsWorld;
}

class DemostrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE(DemostrationWorld)

	typedef gm::GMGameWorld Base;

public:
	DemostrationWorld() = default;
	~DemostrationWorld();

public:
	void addDemo(const gm::GMString& name, AUTORELEASE gm::IGameHandler* demo);
	void init();
	void renderScene();

private:
	void setCurrentDemo(gm::IGameHandler* demo) { D(d); d->currentDemo = demo; }
	void setCurrentDemo(gm::GMint index) { D(d); d->currentDemo = d->demos[0].second; }
};

GM_PRIVATE_OBJECT(DemostrationEntrance)
{
	DemostrationWorld* world = nullptr;
};

class DemostrationEntrance : public gm::IGameHandler, public gm::IShaderLoadCallback
{
	DECLARE_PRIVATE(DemostrationEntrance)

public:
	DemostrationEntrance() = default;
	~DemostrationEntrance();

public:
	inline DemostrationWorld* getWorld() { D(d); return d->world; }

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