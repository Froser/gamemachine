#ifndef __DEMO_SIMPLE_H__
#define __DEMO_SIMPLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_NormalMap)
{
	GMQuat rotation;
	gm::GMfloat angle = 0.f;
	bool rotate = true;
	gm::GMGameObject* gameObject = nullptr;
};

class Demo_NormalMap : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_NormalMap, DemoHandler)

	GM_DECLARE_SIGNAL(rotateStateChanged);

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
};
#endif