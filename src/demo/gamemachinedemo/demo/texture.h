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
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用顶点缓存来渲染一个纹理。";
		return desc;
	}
};

class Demo_Texture_Index : public Demo_Texture
{
	typedef Demo_Texture Base;

public:
	using Base::Base;

public:
	virtual void init() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用索引缓存来渲染一个纹理。";
		return desc;
	}
};

#endif