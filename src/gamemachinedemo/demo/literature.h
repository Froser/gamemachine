#ifndef __DEMO_LITERATURE_H__
#define __DEMO_LITERATURE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_Literature)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Literature : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_Literature, DemoHandler)

public:
	using Base::Base;
	~Demo_Literature();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif