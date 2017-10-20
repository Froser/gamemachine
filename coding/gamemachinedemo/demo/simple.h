#ifndef __DEMO_SIMPLE_H__
#define __DEMO_SIMPLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Simple)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Simple : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Simple)

	typedef DemoHandler Base;

public:
	~Demo_Simple();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif