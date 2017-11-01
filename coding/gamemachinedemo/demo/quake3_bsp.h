#ifndef __DEMO_QUAKE3_BSP_H__
#define __DEMO_QUAKE3_BSP_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

class GMBSPGameWorld;
GM_PRIVATE_OBJECT(Demo_Quake3_BSP)
{
	gm::GMBSPGameWorld* world = nullptr;
	gm::GMSpriteGameObject* sprite = nullptr;
	bool mouseEnabled = true;
};

class Demo_Quake3_BSP : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Quake3_BSP)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Quake3_BSP();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
	virtual void onActivate() override;
	virtual void onDeactivate() override;

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;

private:
	void setMouseEnabled(bool enabled);
};

#endif