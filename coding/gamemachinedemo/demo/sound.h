#ifndef __DEMO_SOUND_H__
#define __DEMO_SOUND_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demostration_world.h"

GM_PRIVATE_OBJECT(Demo_Sound)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
	gm::IAudioFile* file = nullptr;
	gm::IAudioSource* audioSource = nullptr;
};

class Demo_Sound : public DemoHandler
{
	DECLARE_PRIVATE(Demo_Sound)

	typedef DemoHandler Base;

public:
	using Base::Base;
	~Demo_Sound();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;

private:
	void setupItem(gm::GMImage2DGameObject* item, gm::GMAsset border, const gm::GMRect& textureGeo, gm::GMint imgWidth, gm::GMint imgHeight);
};

#endif