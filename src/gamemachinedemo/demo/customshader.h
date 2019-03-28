#ifndef __DEMO_CUSTOMSHADER_H__
#define __DEMO_CUSTOMSHADER_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_CustomGeometryShader : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	virtual gm::GMRenderTechinqueID techId();
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用自定义几何着色器。";
		return desc;
	}

public:
	static void initCustomShader(const gm::IRenderContext* context);
};

class Demo_CustomAndDefaultShader : public Demo_CustomGeometryShader
{
	typedef Demo_CustomGeometryShader Base;

public:
	using Base::Base;

protected:
	virtual gm::GMRenderTechinqueID techId();
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用自定义着色器。只改顶点阶段。";
		return desc;
	}

public:
	static void initCustomShader(const gm::IRenderContext* context);
};

#endif