#ifndef __Demo_Phong_PBR_H__
#define __Demo_Phong_PBR_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_ALIGNED(Demo_Phong_PBR)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMGameObject* gameObject_Phong = nullptr;
	gm::GMint32 mouseDownX;
	gm::GMint32 mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
	bool stashedGammaCorrection;
	bool stashedToneMapping;
};

class Demo_Phong_PBR : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Phong_PBR)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Phong_PBR(DemonstrationWorld* parentDemonstrationWorld);

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
		static gm::GMString desc = L"使用PBR和Phong来渲染。按住鼠标旋转球体。";
		return desc;
	}
};

#endif