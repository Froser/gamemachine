#ifndef __DEMO_PBR_H__
#define __DEMO_PBR_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_PBR)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMint mouseDownX;
	gm::GMint mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
	bool stashedGammaCorrection;
	bool stashedToneMapping;
};

class Demo_PBR : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_PBR, DemoHandler)

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void onDeactivate() override;

private:
	void handleMouseEvent();
	void handleDragging();

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;
};

#endif