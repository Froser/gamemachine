#ifndef __DEMO_TEXTURE_H__
#define __DEMO_TEXTURE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

GM_PRIVATE_OBJECT(Demo_Texture)
{
	gm::GMDemoGameWorld* demoWorld = nullptr;
};

class Demo_Texture : public DemoHandler
{
	DECLARE_PRIVATE_AND_BASE(Demo_Texture, DemoHandler)

public:
	using Base::Base;
	~Demo_Texture();

public:
	virtual void init() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

class Demo_Texture_Index : public Demo_Texture
{
public:
	using Demo_Texture::Demo_Texture;

public:
	virtual void init() override;
};

#endif