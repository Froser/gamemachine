#ifndef __DEMO_SIMPLE_H__
#define __DEMO_SIMPLE_H__

#include <gamemachine.h>

class Demo_Simple : public gm::IGameHandler
{
public:
	virtual void init() override;
	virtual void start() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

#endif