#include "stdafx.h"
#include "gmparticle.h"
#include "gmparticleeffects.h"

GMParticleSystem::GMParticleSystem()
{
	D(d);
	d->emitter.reset(new GMParticleEmitter());
}

void GMParticleSystem::setDescription(const GMParticleDescription& desc)
{
	D(d);
	GM_ASSERT(d->emitter);
	d->emitter->setDescription(desc);
}

void GMParticleSystem::render(IRenderContext* context)
{
	D(d);

}

void GMParticleEmitter::setDescription(const GMParticleDescription& desc)
{
	setEmitPosition(desc.getEmitterPosition());
	setEmitPositionV(desc.getEmitterPositionV());

	setEmitAngle(desc.getEmitterEmitAngle());
	setEmitAngleV(desc.getEmitterEmitAngleV());

	setEmitSpeed(desc.getEmitterEmitSpeed());
	setEmitSpeedV(desc.getEmitterEmitSpeedV());

	setEmitRate(desc.getEmitRate());
	setDuration(desc.getDuration());
	setParticleCount(desc.getParticleCount());

	GMParticleEffect* eff = nullptr;
	if (desc.getEmitterType() == GMParticleEmitterType::Gravity)
	{
		eff = new GMGravityParticleEffect();
	}
	else
	{
		GM_ASSERT(desc.getEmitterType() == GMParticleEmitterType::Radius);
		eff = new GMRadialParticleEffect();
	}
	setParticleEffect(eff);
}

void GMParticleEmitter::setParticleEffect(GMParticleEffect* effect)
{
	D(d);
	d->effect.reset(effect);
}

void GMParticlePool::init(GMsize_t count)
{
	D(d);
	d->particlePool.reserve(count);
	for (GMsize_t i = 0; i < count; ++i)
	{
		d->particlePool.emplace_back(GMOwnedPtr<GMParticle>(new GMParticle()));
	}
}

void GMParticlePool::free()
{
	D(d);
	GMClearSTLContainer(d->particlePool);
}

GMParticle* GMParticlePool::alloc()
{
	D(d);
	return d->particlePool[d->index++].get();
}

void GMParticleEffect::setParticleDescription(const GMParticleDescription& desc)
{
	setLife(desc.getLife());
	setLifeV(desc.getLifeV());

	setBeginColor(desc.getBeginColor());
	setBeginColorV(desc.getBeginColorV());
	setEndColor(desc.getEndColor());
	setEndColorV(desc.getEndColorV());

	setBeginSize(desc.getBeginSize());
	setBeginSizeV(desc.getBeginSizeV());
	setEndSize(desc.getEndSize());
	setEndSizeV(desc.getEndSizeV());

	setBeginSpin(desc.getBeginSpin());
	setBeginSpinV(desc.getBeginSpinV());
	setEndSpin(desc.getEndSpin());
	setEndSpinV(desc.getEndSpinV());

	setMotionMode(desc.getMotionMode());
	setGravityMode(desc.getGravityMode());
	setRadiusMode(desc.getRadiusMode());
}

GMParticleSystemManager::GMParticleSystemManager(IRenderContext* context)
{
	D(d);
	d->context = context;
}

void GMParticleSystemManager::addParticleSystem(AUTORELEASE GMParticleSystem* ps)
{
	D(d);
	d->particleSystems.push_back(GMOwnedPtr<GMParticleSystem>(ps));
}

void GMParticleSystemManager::render()
{
	D(d);
	for (decltype(auto) ps : d->particleSystems)
	{
		ps->render(d->context);
	}
}

void GMParticleSystemManager::update(GMfloat dt)
{
	// TODO 考虑成异步
}