#ifndef __DEMO_SIMPLE_H__
#define __DEMO_SIMPLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_NormalMap)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_NormalMap : public DemoHandler
{
	DECLARE_PRIVATE(Demo_NormalMap)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_NormalMap();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif