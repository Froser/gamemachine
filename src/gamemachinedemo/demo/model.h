#ifndef __DEMO_MODEL_H__
#define __DEMO_MODEL_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_Model)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMGameObject* gameObject2 = nullptr;
	gm::GMGameObject* gameObject3 = nullptr;
	gm::GMCubeMapGameObject* skyObject = nullptr;
	gm::GMint mouseDownX;
	gm::GMint mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
};

class Demo_Model : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_Model, DemoHandler)

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void onDeactivate() override;

private:
	gm::GMCubeMapGameObject* createCubeMap();
	void handleMouseEvent();
	void handleDragging();

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"加载一个模型。按住旋转，滚轮调整大小。按住右键调整视野。";
		return desc;
	}
};

#endif