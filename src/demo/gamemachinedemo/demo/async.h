#ifndef __DEMO_ASYNC_H__
#define __DEMO_ASYNC_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Async)
{
	gm::GMFuture<void> future;
	Stack<std::function<void()>> funcStack;
};

class Demo_Async : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Async)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Async(DemonstrationWorld* parentDemonstrationWorld);

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void setDefaultLights() override;

private:
	void mainThreadInvoke(std::function<void()>);
	void invokeThreadFunctions();

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用GMAsync异步加载资源。";
		return desc;
	}
};

#endif