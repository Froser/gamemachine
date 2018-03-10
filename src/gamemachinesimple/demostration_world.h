#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>

class DemostrationWorld;

GM_PRIVATE_OBJECT(DemostrationWorld)
{
	gm::GMGameObject* gameObj = nullptr;
};

class DemostrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE_AND_BASE(DemostrationWorld, gm::GMGameWorld)

public:
	DemostrationWorld() = default;
	~DemostrationWorld();

public:
	void init();
	void renderScene();
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
	void onLoadShaders(gm::IGraphicEngine* engine);

	// IGameHandler
private:
	virtual void init() override;
	virtual void start() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif