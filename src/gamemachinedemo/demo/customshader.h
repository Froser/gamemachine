#ifndef __DEMO_CUSTOMSHADER_H__
#define __DEMO_CUSTOMSHADER_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_CustomShader : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用自定义着色器。";
		return desc;
	}

public:
	static void initCustomShader(const gm::IRenderContext* context);
};

#endif