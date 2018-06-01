#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>
#include <gmdemogameworld.h>

namespace gm
{
	class GMWidget;
}

class DemonstrationWorld;
GM_PRIVATE_OBJECT(DemoHandler)
{
	gm::GMRenderConfig renderConfig;
	gm::GMDebugConfig debugConfig;
	DemonstrationWorld* parentDemonstrationWorld = nullptr;
	bool inited = false;
	bool activating = false;
	gm::GMGameWorld* demoWorld = nullptr;
	gm::IGraphicEngine* engine = nullptr;
};

class DemoHandler : public gm::GMObject
{
	DECLARE_PRIVATE(DemoHandler)

public:
	DemoHandler(DemonstrationWorld* parentDemonstrationWorld);
	~DemoHandler();

public:
	virtual void init();
	virtual bool isInited();
	virtual void onActivate();
	virtual void onDeactivate();
	virtual void event(gm::GameMachineHandlerEvent evt);

protected:
	virtual void setLookAt();
	virtual void setDefaultLights();

protected:
	void backToEntrance();
	bool isActivating();
	void switchNormal();

	inline gm::GMGameWorld*& getDemoWorldReference()
	{
		D(d);
		return d->demoWorld;
	}

	inline DemonstrationWorld* getDemonstrationWorld()
	{
		D(d);
		return d->parentDemonstrationWorld;
	}
};

typedef Pair<gm::GMString, DemoHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> DemoHandlers;

GM_PRIVATE_OBJECT(DemonstrationWorld)
{
	DemoHandlers demos;
	DemoHandler* currentDemo = nullptr;
	DemoHandler* nextDemo = nullptr;
	gm::GMWidget* mainWidget = nullptr;
	gm::IWindow* mainWindow = nullptr;
};

class DemonstrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE_AND_BASE(DemonstrationWorld, gm::GMGameWorld)

public:
	DemonstrationWorld(const gm::GMContext*, gm::IWindow*);
	~DemonstrationWorld();

public:
	inline DemoHandler* getCurrentDemo() { D(d); return d->currentDemo; }
	void setCurrentDemo(DemoHandler* demo) { D(d); d->currentDemo = demo; }
	void setCurrentDemo(gm::GMint index) { D(d); d->currentDemo = d->demos[0].second; }
	inline gm::GMWidget* getMainWidget() { D(d); return d->mainWidget; }

public:
	void addDemo(const gm::GMString& name, AUTORELEASE DemoHandler* demo);
	void init();
	void switchDemo();
	void resetProjectionAndEye();

public:
	gm::IWindow* getMainWindow()
	{
		D(d);
		return d->mainWindow;
	}
};

GM_PRIVATE_OBJECT(DemostrationEntrance)
{
	DemonstrationWorld* world = nullptr;
	gm::IWindow* mainWindow = nullptr;
	gm::GMDebugConfig debugConfig;
	gm::GMRenderConfig renderConfig;
};

class DemostrationEntrance : public gm::IGameHandler, public gm::IShaderLoadCallback
{
	DECLARE_PRIVATE_NGO(DemostrationEntrance)

public:
	DemostrationEntrance(gm::IWindow*);
	~DemostrationEntrance();

public:
	inline DemonstrationWorld* getWorld() { D(d); return d->world; }

	// IShaderLoadCallback
private:
	void onLoadShaders(const gm::GMContext* context);

private:
	void initLoadEffectsShader(gm::GMGLShaderProgram* effectsShaderProgram);
	void initLoadShaderProgram(gm::GMGLShaderProgram* forwardShaderProgram, gm::GMGLShaderProgram* deferredShaderProgram[2]);

	// IGameHandler
private:
	virtual void init(const gm::GMContext* context) override;
	virtual void start() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
};

inline gm::GMDemoGameWorld* asDemoGameWorld(gm::GMGameWorld* world)
{
	return gm::gm_cast<gm::GMDemoGameWorld*>(world);
}

#endif