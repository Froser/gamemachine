#ifndef __DEMO_SPECULARMAP_H__
#define __DEMO_SPECULARMAP_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_ALIGNED(Demo_SpecularMap)
{
	gm::GMGameObject* gameObject = nullptr;
	gm::GMGameObject* gameObject2 = nullptr;
	gm::GMint32 mouseDownX;
	gm::GMint32 mouseDownY;
	bool draggingL = false;
	bool draggingR = false;
	GMQuat lookAtRotation;
};

class Demo_SpecularMap : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_SpecularMap)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_SpecularMap(DemonstrationWorld* parentDemonstrationWorld);

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