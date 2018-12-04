#ifndef __DEMO_LIGHT_H__
#define __DEMO_LIGHT_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_Light : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void setLookAt() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
};

class Demo_Light_Point : public Demo_Light
{
	typedef Demo_Light Base;

public:
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的点光源场景。";
		return desc;
	}

	virtual void setDefaultLights() override;
};

#endif