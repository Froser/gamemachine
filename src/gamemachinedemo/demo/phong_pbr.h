#ifndef __Demo_Phong_PBR_H__
#define __Demo_Phong_PBR_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_Phong_PBR)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMGameObject* gameObject_Phong = nullptr;
	gm::GMint mouseDownX;
	gm::GMint mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
	bool stashedGammaCorrection;
	bool stashedToneMapping;
};

class Demo_Phong_PBR : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_Phong_PBR, DemoHandler)

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