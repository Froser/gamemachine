#ifndef __DEMO_EFFECTS_H__
#define __DEMO_EFFECTS_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"

GM_PRIVATE_OBJECT(Demo_Effects)
{
	gm::GMFilterMode::Mode mode = gm::GMFilterMode::None;
};

class Demo_Effects : public Demo_Texture
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_Effects, Demo_Texture)

public:
	using Base::Base;

	virtual void init() override;
	virtual void onActivate() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用GameMachine内置滤镜。";
		return desc;
	}

private:
	void setEffect(gm::GMFilterMode::Mode mode);
};

#endif