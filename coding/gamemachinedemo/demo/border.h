#ifndef __DEMO_BORDER_H__
#define __DEMO_BORDER_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Border)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Border : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Border)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Border();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif