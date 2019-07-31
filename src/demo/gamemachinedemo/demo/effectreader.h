#ifndef __DEMO_EFFECTREADER_H__
#define __DEMO_EFFECTREADER_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_ALIGNED(Demo_EffectReader)
{
	gm::GMSceneAsset cubeScene;
	gm::GMGameObject* obj = nullptr;
};

class Demo_EffectReader : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_EffectReader);
	GM_DECLARE_BASE(DemoHandler);

public:
	Demo_EffectReader(DemonstrationWorld* parentDemonstrationWorld);

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用GameMachine Effect Loader读取一个效果。";
		return desc;
	}
};

#endif