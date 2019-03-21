#ifndef __DEMO_WAVE_H__
#define __DEMO_WAVE_H__
#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_Wave)
{
	gm::GMGameObject* wave = nullptr;
	gm::GMint32 mouseDownX;
	gm::GMint32 mouseDownY;
	bool draggingL = false;
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

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"绘制一个水波。左键拖拽，滚轮缩放。";
		return desc;
	}

private:
	void handleMouseEvent();
	void handleDragging();
};

#endif