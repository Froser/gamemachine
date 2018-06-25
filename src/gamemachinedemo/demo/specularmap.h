#ifndef __DEMO_SPECULARMAP_H__
#define __DEMO_SPECULARMAP_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_SpecularMap)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMGameObject* gameObject2 = nullptr;
	gm::GMint mouseDownX;
	gm::GMint mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
};

class Demo_SpecularMap : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_SpecularMap, DemoHandler)

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

private:
	void handleMouseEvent();
	void handleDragging();

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染带有和不带有高光贴图的对象。按住鼠标旋转对象。";
		return desc;
	}
};

#endif