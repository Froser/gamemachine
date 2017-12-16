#ifndef __DEMO_PARTICLES2_H__
#define __DEMO_PARTICLES2_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Particles2)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
	gm::GMParticlesEmitter* emitter;
};

class Demo_Particles2 : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_Particles2, DemoHandler)

public:
	using Base::Base;
	~Demo_Particles2();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif