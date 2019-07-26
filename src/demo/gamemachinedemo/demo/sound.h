#ifndef __DEMO_SOUND_H__
#define __DEMO_SOUND_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Sound)
{
	gm::IAudioFile* wavFile = nullptr;
	gm::IAudioFile* mp3File = nullptr;
	gm::IAudioSource* wavSource = nullptr;
	gm::IAudioSource* mp3Source = nullptr;
};

class Demo_Sound : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Sound)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Sound(DemonstrationWorld* parentDemonstrationWorld);
	~Demo_Sound();

	virtual void onActivate() override;
	virtual void onDeactivate() override;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"播放一段Wave音频或者MP3音频。";
		return desc;
	}
};

#endif