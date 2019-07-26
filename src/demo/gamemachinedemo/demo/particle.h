#ifndef __DEMO_PARTICLE_H__
#define __DEMO_PARTICLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"
#include <gmparticle.h>

GM_PRIVATE_OBJECT_UNALIGNED(Demo_Particle2D)
{
	gm::GMFilterMode::Mode mode = gm::GMFilterMode::None;
	gm::GMOwnedPtr<gm::IParticleSystemManager> particleSystemManager;
};

class Demo_Particle2D : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_Particle2D)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_Particle2D(DemonstrationWorld* parentDemonstrationWorld);
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

GM_PRIVATE_OBJECT_UNALIGNED(Demo_ParticleBillboard)
{
	gm::GMFilterMode::Mode mode = gm::GMFilterMode::None;
	gm::GMOwnedPtr<gm::IParticleSystemManager> particleSystemManager;
	gm::GMCameraUtility cameraUtility;
	bool activated = false;
};

class Demo_ParticleBillboard : public DemoHandler
{
	GM_DECLARE_PRIVATE(Demo_ParticleBillboard)
	GM_DECLARE_BASE(DemoHandler)

public:
	Demo_ParticleBillboard(DemonstrationWorld* parentDemonstrationWorld);

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