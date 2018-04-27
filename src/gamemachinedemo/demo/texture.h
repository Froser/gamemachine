#ifndef __DEMO_TEXTURE_H__
#define __DEMO_TEXTURE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_Texture : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

class Demo_Texture_Index : public Demo_Texture
{
	typedef Demo_Texture Base;

public:
	using Base::Base;

public:
	virtual void init() override;
};

#endif