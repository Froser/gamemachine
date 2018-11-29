#ifndef __DEMO_CUSTOMLIGHT_H__
#define __DEMO_CUSTOMLIGHT_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_CustomLight : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用自定义过程实现聚光灯光照。";
		return desc;
	}
};

#endif