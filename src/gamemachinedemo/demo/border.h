#ifndef __DEMO_BORDER_H__
#define __DEMO_BORDER_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_Border)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Border : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_Border, DemoHandler)

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif