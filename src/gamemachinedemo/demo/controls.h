#ifndef __DEMO_CONTROLS_H__
#define __DEMO_CONTROLS_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

namespace gm
{
	class GMWidget;
}

class Demo_Controls : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual gm::GMWidget* createDefaultWidget();

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"";
		return desc;
	}
};

#endif