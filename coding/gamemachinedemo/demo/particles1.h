#ifndef __DEMO_PARTICLES1_H__
#define __DEMO_PARTICLES1_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Particles1)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
	gm::GMParticlesEmitter* emitter;
};

class Demo_Particles1 : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_Particles1, DemoHandler)

public:
	using Base::Base;
	~Demo_Particles1();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif