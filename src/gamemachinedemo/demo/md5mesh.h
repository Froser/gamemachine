#ifndef __DEMO_MD5MESH_H__
#define __DEMO_MD5MESH_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"

GM_PRIVATE_OBJECT(Demo_MD5Mesh)
{
	gm::GMGameObject* boblampclean = nullptr;
};

class Demo_MD5Mesh : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_MD5Mesh, DemoHandler)

public:
	using Base::Base;

	virtual void init() override;
	virtual void Demo_MD5Mesh::event(gm::GameMachineHandlerEvent evt) override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用MD5骨骼动画。";
		return desc;
	}
};

#endif