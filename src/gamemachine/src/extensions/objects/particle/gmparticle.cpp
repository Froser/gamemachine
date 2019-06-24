#include "stdafx.h"
#include "gmparticle.h"

GMParticleSystemManager::GMParticleSystemManager(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

void GMParticleSystemManager::addParticleSystem(AUTORELEASE IParticleSystem* ps)
{
	D(d);
	ps->setParticleSystemManager(this);
	d->particleSystems.push_back(GMOwnedPtr<IParticleSystem>(ps));
}

void GMParticleSystemManager::render()
{
	D(d);
	for (decltype(auto) ps : d->particleSystems)
	{
		ps->render();
	}
}

void GMParticleSystemManager::update(GMDuration dt)
{
	D(d);
	// 涉及到粒子池的分配，不能并行
	for (decltype(auto) system : d->particleSystems)
	{
		system->update(dt);
	}
}