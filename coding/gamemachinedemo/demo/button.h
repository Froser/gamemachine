#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

namespace gm
{
	class GMControlGameObjectAnimation;
}

GM_PRIVATE_OBJECT(Demo_Button)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
	gm::GMControlGameObjectAnimation* buttonAnimation = nullptr;
};

class Demo_Button : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Button)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Button();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif