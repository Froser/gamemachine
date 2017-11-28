#ifndef __DEMO_MODEL_H__
#define __DEMO_MODEL_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Model)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Model : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Model)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Model();

public:
	virtual void setLookAt() override;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif