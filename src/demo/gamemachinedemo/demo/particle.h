#ifndef __DEMO_PARTICLE_H__
#define __DEMO_PARTICLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"
#include <gmparticle.h>

GM_PRIVATE_OBJECT(Demo_Particle2D)
{
	gm::GMFilterMode::Mode mode = gm::GMFilterMode::None;
	gm::GMOwnedPtr<gm::GMParticleSystemManager> particleSystemManager;
};

class Demo_Particle2D : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_Particle2D, DemoHandler)

public:
	using Base::Base;
	virtual void setLookAt() override;
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染2D粒子。";
		return desc;
	}
};


GM_PRIVATE_OBJECT(Demo_ParticleBillboard)
{
	gm::GMFilterMode::Mode mode = gm::GMFilterMode::None;
	gm::GMOwnedPtr<gm::GMParticleSystemManager> particleSystemManager;
	gm::GMCameraUtility cameraUtility;
	bool activated = false;
};

class Demo_ParticleBillboard : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_ParticleBillboard, DemoHandler)

public:
	using Base::Base;
	virtual void setLookAt() override;
	virtual void init() override;
	virtual void event(gm::GameMachineHandlerEvent evt) override;
	virtual void onActivate() override;
	virtual void onDeactivate() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染Billboard粒子。移动鼠标调整视觉。";
		return desc;
	}

private:
	void handleMouseEvent();
};

#endif