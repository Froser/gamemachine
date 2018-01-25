#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>

class DemostrationWorld;
GM_PRIVATE_OBJECT(DemoHandler)
{
	DemostrationWorld* demostrationWorld;
	bool inited = false;
	bool activating = false;
};

class DemoHandler : public gm::GMObject
{
	DECLARE_PRIVATE(DemoHandler)

public:
	DemoHandler(DemostrationWorld* demostrationWorld);

public:
	virtual void init();
	virtual bool isInited();
	virtual void onActivate();
	virtual void onDeactivate();
	virtual void event(gm::GameMachineEvent evt);

protected:
	virtual void setLookAt();
	virtual void setDefaultLights();

protected:
	void backToEntrance();
	bool isActivating();
};

typedef Pair<gm::GMString, DemoHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> DemoHandlers;

GM_PRIVATE_OBJECT(DemostrationWorld)
{
	DemoHandlers demos;
	DemoHandler* currentDemo = nullptr;
	DemoHandler* nextDemo = nullptr;
	gm::GMCursorGameObject* cursor = nullptr;
};

class DemostrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE_AND_BASE(DemostrationWorld, gm::GMGameWorld)

public:
	DemostrationWorld() = default;
	~DemostrationWorld();

public:
	inline DemoHandler* getCurrentDemo() { D(d); return d->currentDemo; }
	void setCurrentDemo(DemoHandler* demo) { D(d); d->currentDemo = demo; }
	void setCurrentDemo(gm::GMint index) { D(d); d->currentDemo = d->demos[0].second; }

public:
	void addDemo(const gm::GMString& name, AUTORELEASE DemoHandler* demo);
	void init();
	void renderScene();
	void switchDemo();
	void resetProjectionAndEye();
};

GM_PRIVATE_OBJECT(DemostrationEntrance)
{
	DemostrationWorld* world = nullptr;
};

class DemostrationEntrance : public gm::IGameHandler, public gm::IShaderLoadCallback
{
	DECLARE_PRIVATE_NGO(DemostrationEntrance)

public:
	DemostrationEntrance() = default;
	~DemostrationEntrance();

public:
	inline DemostrationWorld* getWorld() { D(d); return d->world; }

	// IShaderLoadCallback
private:
	void onLoadEffectsShader(gm::GMGLShaderProgram& effectsShaderProgram) override;
	void onLoadShaderProgram(gm::GMGLShaderProgram& forwardShaderProgram, gm::GMGLShaderProgram* deferredShaderProgram[2]) override;

	// IGameHandler
private:
	virtual void init() override;
	virtual void start() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif