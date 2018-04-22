#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

namespace gm
{
	class GMControlGameObjectAnimation;
}

struct Button
{
	gm::GMImage2DGameObject* button = nullptr;
	gm::GMControlGameObjectAnimation* animation = nullptr;
};

GM_PRIVATE_OBJECT(Demo_Button)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
	gm::GMAsset border;
	gm::GMRect textureGeo;
	Button buttons[4];
};

typedef std::function<void(gm::GMControlGameObjectAnimation*)> AnimationAction;

class Demo_Button : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_Button, DemoHandler)

public:
	using Base::Base;
	~Demo_Button();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;

private:
	void initBorder();
	template <gm::GMint idx> Button& newButton(gm::GMImage* border, const AnimationAction& onHoverAction);
};

#endif