#ifndef __DEMO_SIMPLE_H__
#define __DEMO_SIMPLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_NormalMap)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMAnimation animation;
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
	virtual void onActivate() override;
	virtual void onDeactivate() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用法线贴图渲染一个对象。";
		return desc;
	}
};
#endif