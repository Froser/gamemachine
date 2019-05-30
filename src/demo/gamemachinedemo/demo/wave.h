#ifndef __DEMO_WAVE_H__
#define __DEMO_WAVE_H__
#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include <extensions/objects/gmwavegameobject.h>

GM_PRIVATE_OBJECT(Demo_Wave)
{
	gm::GMWaveGameObject* wave = nullptr;
	gm::GMGameObject* skyObject = nullptr;
	gm::GMCameraUtility cameraUtility;
	bool activate = false;
	bool mouseTrace = false;
	gm::GMControlLabel* handwareAccelerationLabel = nullptr;
};

class Demo_Wave : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_Wave, DemoHandler)

public:
	using Base::Base;

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
	void setWaterFlow(bool flow);

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"绘制一个水波。晃动鼠标以调整视角。R键显示鼠标。";
		return desc;
	}
};

GM_PRIVATE_OBJECT(Demo_CleanWater)
{
	gm::GMGameObject* ground = nullptr;
};

#endif