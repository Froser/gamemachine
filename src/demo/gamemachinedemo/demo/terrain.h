#ifndef __DEMO_TERRIAN_H__
#define __DEMO_TERRIAN_H__
#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Terrain)
{
	gm::GMAsset terrainScene;
	gm::GMGameObject* terrain = nullptr;
	gm::GMint32 mouseDownX;
	gm::GMint32 mouseDownY;
	bool draggingL = false;
};

class Demo_Terrain : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Terrain)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Terrain(DemonstrationWorld* parentDemonstrationWorld);

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	virtual void setLookAt() override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"绘制一个地形图。左键拖拽，滚轮缩放。";
		return desc;
	}

private:
	void handleMouseEvent();
	void handleDragging();
};

#endif