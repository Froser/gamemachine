#ifndef __DEMO_CSM_H__
#define __DEMO_CSM_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_CSM)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMint32 mouseDownX;
	gm::GMint32 mouseDownY;
	bool dragging = false;
	GMQuat lookAtRotation;
};

class Demo_CSM : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_CSM, DemoHandler)

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

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用CSM渲染场景。鼠标左键旋转场景。";
		return desc;
	}
};

#endif