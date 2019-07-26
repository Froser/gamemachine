#ifndef __DEMO_LUA_H__
#define __DEMO_LUA_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include <gmlua.h>
#include "demonstration_world.h"
#include <gmcontroltextedit.h>

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Lua)
{
	gm::GMControlTextArea* textCode = nullptr;
	gm::GMControlLabel* lbState = nullptr;
	gm::GMLua lua;
};

class Demo_Lua : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Lua);
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Lua(DemonstrationWorld* parentDemonstrationWorld);

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual gm::GMWidget* createDefaultWidget() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"执行一段Lua。";
		return desc;
	}

private:
	void runScript();
};

#endif