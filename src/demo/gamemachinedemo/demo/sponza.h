#ifndef __DEMO_SPONZA_H__
#define __DEMO_SPONZA_H__
#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Sponza)
{
	gm::GMGameObject* sponza = nullptr;
	gm::GMGameObject* skyObject = nullptr;
	gm::GMSpriteGameObject* sprite = nullptr;
	gm::GMControlLabel* cpu = nullptr;
	bool activate = false;
	bool mouseTrace = false;
};

class Demo_Sponza : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Sponza)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Sponza(DemonstrationWorld* parentDemonstrationWorld);

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;

	virtual void onActivate() override;
	virtual void onDeactivate() override;

	virtual void createMenu();
	virtual void createObject();

protected:
	void setMouseTrace(bool enabled);

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"绘制一个Sponza。晃动鼠标以调整视角。WSAD移动视角，R显示鼠标。";
		return desc;
	}
};

#endif