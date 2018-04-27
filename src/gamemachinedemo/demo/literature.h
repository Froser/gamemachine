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
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif