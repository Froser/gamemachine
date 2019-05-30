#ifndef __DEMO_LITERATURE_H__
#define __DEMO_LITERATURE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_Literature : public DemoHandler
{
	typedef DemoHandler Base;
public:
	using DemoHandler::DemoHandler;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用GameMachine渲染一段富文本。";
		return desc;
	}
};

#endif