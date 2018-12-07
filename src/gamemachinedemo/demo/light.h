#ifndef __DEMO_LIGHT_H__
#define __DEMO_LIGHT_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"

class Demo_Light : public DemoHandler
{
	typedef DemoHandler Base;

public:
	using Base::Base;

public:
	virtual void init() override;
	virtual void setLookAt() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	virtual void onAssetAdded(gm::GMModelAsset modelAsset);
	virtual void onGameObjectAdded(gm::GMGameObject* obj);
};

class Demo_Light_Point : public Demo_Light
{
	typedef Demo_Light Base;

public:
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的点光源场景。";
		return desc;
	}

	virtual void setDefaultLights() override;
};

class Demo_Light_Point_NormalMap : public Demo_Light
{
	typedef Demo_Light Base;
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的点光源法线贴图场景。";
		return desc;
	}

protected:
	virtual void setDefaultLights() override;
	virtual void onAssetAdded(gm::GMModelAsset modelAsset) override;
	virtual void onGameObjectAdded(gm::GMGameObject* obj) override;
};

class Demo_Light_Directional : public Demo_Light
{
	typedef Demo_Light Base;

public:
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的方向光场景。";
		return desc;
	}

	virtual void setDefaultLights() override;
};

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Light_Directional_Normalmap)
{
	gm::GMLightIndex light = 0;
	gm::GMLightIndex ambientLight = 0;
};

class Demo_Light_Directional_Normalmap : public Demo_Light_Point_NormalMap
{
	GM_DECLARE_PRIVATE_NGO(Demo_Light_Directional_Normalmap)
	typedef Demo_Light_Point_NormalMap Base;
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的方向光法线贴图场景。";
		return desc;
	}

protected:
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void setDefaultLights() override;
	virtual void onGameObjectAdded(gm::GMGameObject* obj) override;
};

class Demo_Light_Spotlight : public Demo_Light_Directional
{
	typedef Demo_Light_Directional Base;

public:
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的聚光灯场景。";
		return desc;
	}

	virtual void setDefaultLights() override;
};

class Demo_Light_Spotlight_Normalmap : public Demo_Light_Directional_Normalmap
{
	typedef Demo_Light_Directional_Normalmap Base;
	using Base::Base;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染一个带衰减的聚光灯法线贴图场景。";
		return desc;
	}

protected:
	virtual void setDefaultLights() override;
};
#endif