#ifndef __DEMO_MODEL_H__
#define __DEMO_MODEL_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Model)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
	gm::GMGameObject* gameObject = nullptr;
	gm::GMCubeMapGameObject* skyObject = nullptr;
	gm::GMint mouseDownX;
	gm::GMint mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	glm::quat lookAtRotation;
};

class Demo_Model : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Model)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Model();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;

private:
	gm::GMCubeMapGameObject* createCubeMap();
	void handleMouseEvent();
	void handleDragging();

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;
};

#endif