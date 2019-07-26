#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>
#include <gmdemogameworld.h>
#include <gmanimation.h>
#include <gmuiconfiguration.h>
#include <gmprimitivemanager.h>
#include <gmlight.h>

namespace gm
{
	class GMWidget;
	class GMControlLabel;
}

class DemonstrationWorld;


GM_PRIVATE_OBJECT_UNALIGNED(DemoHandler)
{
	gm::GMRenderConfig renderConfig;
	gm::GMDebugConfig debugConfig;
	DemonstrationWorld* parentDemonstrationWorld = nullptr;
	bool inited = false;
	bool activating = false;
	gm::GMOwnedPtr<gm::GMGameWorld> demoWorld;
	gm::IGraphicEngine* engine = nullptr;
	gm::GMOwnedPtr<gm::GMWidget> mainWidget;
	gm::GMControlLabel* lbFPS = nullptr;
	gm::GMControlLabel* lbRendering = nullptr;
	gm::GMControlLabel* lbGammaCorrection = nullptr;
	gm::GMControlLabel* lbDebugNormal = nullptr;
	gm::GMint32 nextControlTop = 0;
};

class DemoHandler : public gm::GMObject
{
	GM_DECLARE_PRIVATE(DemoHandler)

	GM_DECLARE_SIGNAL(renderingChanged);
	GM_DECLARE_SIGNAL(gammaCorrectionChanged);

public:
	DemoHandler(DemonstrationWorld* parentDemonstrationWorld);
	~DemoHandler();

public:
	virtual void init();
	virtual bool isInited();
	virtual void onActivate();
	virtual void onDeactivate();
	virtual void event(gm::GameMachineHandlerEvent evt);
	virtual gm::GMWidget* getWidget();

protected:
	virtual void setLookAt();
	virtual void setDefaultLights();
	virtual const gm::GMString& getDescription() const;
	virtual gm::GMWidget* createDefaultWidget();

protected:
	void backToEntrance();
	bool isActivating();
	gm::GMint32 getClientAreaTop();
	gm::GMOwnedPtr<gm::GMGameWorld>& getDemoWorldReference();
	DemonstrationWorld* getDemonstrationWorld();
	const GMVec4& getLabelFontColor();

private:
	void switchNormal();
};

typedef Pair<gm::GMString, DemoHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> DemoHandlers;


GM_PRIVATE_OBJECT_UNALIGNED(DemonstrationWorld)
{
	DemoHandlers demos;
	DemoHandler* currentDemo = nullptr;
	DemoHandler* nextDemo = nullptr;
	gm::GMWidget* mainWidget = nullptr;
	gm::GMWidget* billboard = nullptr;
	gm::IWindow* mainWindow = nullptr;
	gm::GMWidgetResourceManager* manager = nullptr;
	gm::GMOwnedPtr<gm::GMUIConfiguration> configuration;
	gm::GMGameObject* logoObj = nullptr;
	gm::GMAnimation logoAnimation;
	gm::GMFuture<void> logoLoadedFuture;
	Stack<std::function<void()>> funcQueue;
};

class DemonstrationWorld : public gm::GMGameWorld
{
	GM_DECLARE_PRIVATE(DemonstrationWorld)
	GM_DECLARE_BASE(gm::GMGameWorld)

public:
	DemonstrationWorld(const gm::IRenderContext*, gm::IWindow*);
	~DemonstrationWorld();

public:
	DemoHandler* getCurrentDemo();
	void setCurrentDemo(DemoHandler* demo);
	void setCurrentDemo(gm::GMint32 index);
	gm::GMWidget* getMainWidget();
	gm::GMWidget* getBillboardWidget();

public:
	void addDemo(const gm::GMString& name, AUTORELEASE DemoHandler* demo);
	void init();
	void switchDemo();
	void resetCameraAndLights();
	void waitForExit();
	void mainThreadInvoke(std::function<void()>);
	void invokeThreadFunctions();

public:
	gm::IWindow* getMainWindow();
	gm::GMWidgetResourceManager* getManager();
	gm::GMUIConfiguration* getUIConfiguration();
	gm::GMPrimitiveManager* getPrimitiveManager();
	gm::GMAnimation& getLogoAnimation();

private:
	void initObjects();
};

GM_PRIVATE_OBJECT_UNALIGNED(DemostrationEntrance)
{
	DemonstrationWorld* world = nullptr;
	gm::IWindow* mainWindow = nullptr;
	gm::GMDebugConfig debugConfig;
	gm::GMRenderConfig renderConfig;
};

class DemostrationEntrance : public gm::IGameHandler, public gm::IShaderLoadCallback
{
	GM_DECLARE_PRIVATE(DemostrationEntrance)

public:
	DemostrationEntrance(gm::IWindow*);
	~DemostrationEntrance();

public:
	DemonstrationWorld* getWorld();

	// IShaderLoadCallback
private:
	void onLoadShaders(const gm::IRenderContext* context);

	// IGameHandler
private:
	virtual void init(const gm::IRenderContext* context) override;
	virtual void start() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
};

inline gm::GMDemoGameWorld* asDemoGameWorld(const gm::GMOwnedPtr<gm::GMGameWorld>& world)
{
	return gm::gm_cast<gm::GMDemoGameWorld*>(world.get());
}

#endif