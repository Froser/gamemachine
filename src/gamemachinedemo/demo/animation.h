#ifndef __DEMO_MD5MESH_H__
#define __DEMO_MD5MESH_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"

namespace gm
{
	class GMSkeletalGameObject;
}

GM_PRIVATE_OBJECT(Demo_MD5Mesh)
{
	gm::GMSkeletalGameObject* object = nullptr;
};

class Demo_MD5Mesh : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_MD5Mesh, DemoHandler)

public:
	using Base::Base;

	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"使用MD5骨骼动画。";
		return desc;
	}
};

class Demo_MultiAnimation : public Demo_MD5Mesh
{
public:
	using Demo_MD5Mesh::Demo_MD5Mesh;

	virtual void init() override;
	virtual void setDefaultLights() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"一个对象中使用多个动画。";
		return desc;
	}
};

#endif