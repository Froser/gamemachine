#ifndef __DEMO_PARTICLE_H__
#define __DEMO_PARTICLE_H__

#include <gamemachine.h>
#include <gmdemogameworld.h>
#include "demonstration_world.h"
#include "texture.h"
#include <gmparticle.h>

GM_PRIVATE_OBJECT(Demo_Particle)
{
	gm::GMFilterMode::Mode mode = gm::GMFilterMode::None;
	gm::GMOwnedPtr<gm::GMParticleSystemManager> particleSystemManager;
};

class Demo_Particle : public DemoHandler
{
	GM_DECLARE_PRIVATE_AND_BASE(Demo_Particle, DemoHandler)

public:
	using Base::Base;
	virtual void init() override;

protected:
	const gm::GMString& getDescription() const
	{
		static gm::GMString desc = L"渲染粒子。";
		return desc;
	}
};

#endif