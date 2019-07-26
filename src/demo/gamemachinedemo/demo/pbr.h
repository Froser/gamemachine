#ifndef __DEMO_PBR_H__
#define __DEMO_PBR_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_ALIGNED(Demo_PBR)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMint32 mouseDownX;
	gm::GMint32 mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
	bool stashedGammaCorrection;
	bool stashedToneMapping;
};

class Demo_PBR : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_PBR)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_PBR(DemonstrationWorld* parentDemonstrationWorld);

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
		static gm::GMString desc = L"使用PBR技术渲染球体。按住鼠标左键来旋转球体。";
		return desc;
	}
};

#endif