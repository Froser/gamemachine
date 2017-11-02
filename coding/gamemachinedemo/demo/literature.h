#ifndef __DEMO_LITERATURE_H__
#define __DEMO_LITERATURE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Literature)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Literature : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Literature)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Literature();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;

private:
	void setupItem(gm::GMImage2DGameObject* item, gm::GMAsset border, const gm::GMRect& textureGeo, gm::GMint imgWidth, gm::GMint imgHeight);
};

#endif