#ifndef __DEMO_ASYNC_H__
#define __DEMO_ASYNC_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Async)
{
	gm::GMFuture<void> future;
};

class Demo_Async : public DemoHandler
{
	GM_DECLARE_PRIVATE_NGO(Demo_Async)
	typedef DemoHandler Base;

public:
	using DemoHandler::DemoHandler;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用GMAsync异步加载资源。";
		return desc;
	}
};

#endif