#include "stdafx.h"
#include "gmparticle.h"
#include "gmparticleeffects.h"
#include <gmxml.h>
#include <random>

#define Z 0

namespace
{
	template <typename Engine>
	class Random
	{
	public:
		template<typename T>
		static inline T random_real(T min, T max)
		{
			std::uniform_real_distribution<T> dist(min, max);
			auto &mt = getEngine();
			return dist(mt);
		}

		template<typename T>
		static inline T random_int(T min, T max)
		{
			std::uniform_int_distribution<T> dist(min, max);
			auto &mt = getEngine();
			return dist(mt);
		}

	private:
		static Engine& getEngine();
	};

	template <typename Engine>
	Engine& Random<Engine>::getEngine()
	{
		static std::random_device seed_gen;
		static Engine engine(seed_gen());
		return engine;
	}

	using RandomMt19937 = Random<std::mt19937>;
}

GM_PRIVATE_OBJECT(GMCocos2DParticleDescriptionProxy)
{
	GMfloat angle = 0;
	GMfloat angleVariance = 0;
	GMfloat speed = 0;
	GMfloat speedVariance = 0;
	GMDuration duration = 0;
	GMParticleEmitterType emitterType = GMParticleEmitterType::Gravity;
	GMfloat maxParticles = 0;
	GMfloat sourcePositionx = 0;
	GMfloat sourcePositiony = 0;
	GMfloat sourcePositionVariancex = 0;
	GMfloat sourcePositionVariancey = 0;
	GMfloat particleLifespan = 0;
	GMfloat particleLifespanVariance = 0;
	GMfloat startColorRed = 0;
	GMfloat startColorGreen = 0;
	GMfloat startColorBlue = 0;
	GMfloat startColorAlpha = 0;
	GMfloat startColorVarianceRed = 0;
	GMfloat startColorVarianceGreen = 0;
	GMfloat startColorVarianceBlue = 0;
	GMfloat startColorVarianceAlpha = 0;
	GMfloat finishColorRed = 0;
	GMfloat finishColorGreen = 0;
	GMfloat finishColorBlue = 0;
	GMfloat finishColorAlpha = 0;
	GMfloat finishColorVarianceRed = 0;
	GMfloat finishColorVarianceGreen = 0;
	GMfloat finishColorVarianceBlue = 0;
	GMfloat finishColorVarianceAlpha = 0;
	GMfloat startParticleSize = 0;
	GMfloat startParticleSizeVariance = 0;
	GMfloat finishParticleSize = 0;
	GMfloat finishParticleSizeVariance = 0;
	GMfloat rotationStart = 0;
	GMfloat rotationStartVariance = 0;
	GMfloat rotationEnd = 0;
	GMfloat rotationEndVariance = 0;
	GMParticleMotionMode positionType = GMParticleMotionMode::Free;
	GMfloat gravityx = 0;
	GMfloat gravityy = 0;
	GMfloat radialAcceleration = 0;
	GMfloat radialAccelVariance = 0;
	GMfloat tangentialAcceleration = 0;
	GMfloat tangentialAccelVariance = 0;
	GMfloat minRadius = 0;
	GMfloat minRadiusVariance = 0;
	GMfloat maxRadius = 0;
	GMfloat maxRadiusVariance = 0;
	GMfloat rotatePerSecond = 0;
	GMfloat rotatePerSecondVariance = 0;
};

class GMCocos2DParticleDescriptionProxy : public GMObject
{
	GM_DECLARE_PRIVATE(GMCocos2DParticleDescriptionProxy)
	GM_DECLARE_PROPERTY(Angle, angle, GMfloat)
	GM_DECLARE_PROPERTY(AngleVariance, angleVariance, GMfloat)
	GM_DECLARE_PROPERTY(Speed, speed, GMfloat)
	GM_DECLARE_PROPERTY(SpeedVariance, speedVariance, GMfloat)
	GM_DECLARE_PROPERTY(Duration, duration, GMfloat)
	GM_DECLARE_PROPERTY(EmitterType, emitterType, GMParticleEmitterType)
	GM_DECLARE_PROPERTY(MaxParticles, maxParticles, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositionx, sourcePositionx, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositiony, sourcePositiony, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositionVariancex, sourcePositionVariancex, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositionVariancey, sourcePositionVariancey, GMfloat)
	GM_DECLARE_PROPERTY(ParticleLifespan, particleLifespan, GMfloat)
	GM_DECLARE_PROPERTY(ParticleLifespanVariance, particleLifespanVariance, GMfloat)
	GM_DECLARE_PROPERTY(StartColorRed, startColorRed, GMfloat)
	GM_DECLARE_PROPERTY(StartColorGreen, startColorGreen, GMfloat)
	GM_DECLARE_PROPERTY(StartColorBlue, startColorBlue, GMfloat)
	GM_DECLARE_PROPERTY(StartColorAlpha, startColorAlpha, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceRed, startColorVarianceRed, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceGreen, startColorVarianceGreen, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceBlue, startColorVarianceBlue, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceAlpha, startColorVarianceAlpha, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorRed, finishColorRed, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorGreen, finishColorGreen, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorBlue, finishColorBlue, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorAlpha, finishColorAlpha, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceRed, finishColorVarianceRed, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceGreen, finishColorVarianceGreen, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceBlue, finishColorVarianceBlue, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceAlpha, finishColorVarianceAlpha, GMfloat)
	GM_DECLARE_PROPERTY(StartParticleSize, startParticleSize, GMfloat)
	GM_DECLARE_PROPERTY(StartParticleSizeVariance, startParticleSizeVariance, GMfloat)
	GM_DECLARE_PROPERTY(FinishParticleSize, finishParticleSize, GMfloat)
	GM_DECLARE_PROPERTY(FinishParticleSizeVariance, finishParticleSizeVariance, GMfloat)
	GM_DECLARE_PROPERTY(RotationStart, rotationStart, GMfloat)
	GM_DECLARE_PROPERTY(RotationStartVariance, rotationStartVariance, GMfloat)
	GM_DECLARE_PROPERTY(RotationEnd, rotationEnd, GMfloat)
	GM_DECLARE_PROPERTY(RotationEndVariance, rotationEndVariance, GMfloat)
	GM_DECLARE_PROPERTY(PositionType, positionType, GMParticleMotionMode)
	GM_DECLARE_PROPERTY(Gravityx, gravityx, GMfloat)
	GM_DECLARE_PROPERTY(Gravityy, gravityy, GMfloat)
	GM_DECLARE_PROPERTY(RadialAcceleration, radialAcceleration, GMfloat)
	GM_DECLARE_PROPERTY(RadialAccelVariance, radialAccelVariance, GMfloat)
	GM_DECLARE_PROPERTY(TangentialAcceleration, tangentialAcceleration, GMfloat)
	GM_DECLARE_PROPERTY(TangentialAccelVariance, tangentialAccelVariance, GMfloat)
	GM_DECLARE_PROPERTY(MinRadius, minRadius, GMfloat)
	GM_DECLARE_PROPERTY(MinRadiusVariance, minRadiusVariance, GMfloat)
	GM_DECLARE_PROPERTY(MaxRadius, maxRadius, GMfloat)
	GM_DECLARE_PROPERTY(MaxRadiusVariance, maxRadiusVariance, GMfloat)
	GM_DECLARE_PROPERTY(RotatePerSecond, rotatePerSecond, GMfloat)
	GM_DECLARE_PROPERTY(RotatePerSecondVariance, rotatePerSecondVariance, GMfloat)

public:
	virtual bool registerMeta() override
	{
		GM_META(angle)
		GM_META(angleVariance)
		GM_META(speed)
		GM_META(speedVariance)
		GM_META(duration)
		GM_META_WITH_TYPE(emitterType, GMMetaMemberType::Int)
		GM_META(maxParticles)
		GM_META(sourcePositionx)
		GM_META(sourcePositiony)
		GM_META(sourcePositionVariancex)
		GM_META(sourcePositionVariancey)
		GM_META(particleLifespan)
		GM_META(particleLifespanVariance)
		GM_META(startColorRed)
		GM_META(startColorGreen)
		GM_META(startColorBlue)
		GM_META(startColorAlpha)
		GM_META(startColorVarianceRed)
		GM_META(startColorVarianceGreen)
		GM_META(startColorVarianceBlue)
		GM_META(startColorVarianceAlpha)
		GM_META(finishColorRed)
		GM_META(finishColorGreen)
		GM_META(finishColorBlue)
		GM_META(finishColorAlpha)
		GM_META(finishColorVarianceRed)
		GM_META(finishColorVarianceGreen)
		GM_META(finishColorVarianceBlue)
		GM_META(finishColorVarianceAlpha)
		GM_META(startParticleSize)
		GM_META(startParticleSizeVariance)
		GM_META(finishParticleSize)
		GM_META(finishParticleSizeVariance)
		GM_META(rotationStart)
		GM_META(rotationStartVariance)
		GM_META(rotationEnd)
		GM_META(rotationEndVariance)
		GM_META_WITH_TYPE(positionType, GMMetaMemberType::Int)
		GM_META(gravityx)
		GM_META(gravityy)
		GM_META(radialAcceleration)
		GM_META(radialAccelVariance)
		GM_META(tangentialAcceleration)
		GM_META(tangentialAccelVariance)
		GM_META(minRadius)
		GM_META(minRadiusVariance)
		GM_META(maxRadius)
		GM_META(maxRadiusVariance)
		GM_META(rotatePerSecond)
		GM_META(rotatePerSecondVariance)
		return true;
	}
};

GMParticleSystem::GMParticleSystem()
{
	D(d);
	d->emitter.reset(new GMParticleEmitter(this));
}

void GMParticleSystem::setDescription(const GMParticleDescription& desc)
{
	D(d);
	GM_ASSERT(d->emitter);
	d->emitter->setDescription(desc);
}

void GMParticleSystem::update(GMDuration dt)
{
	D(d);
	d->emitter->update(dt);
}

void GMParticleSystem::render(const IRenderContext* context)
{
	D(d);
}

GMParticleDescription GMParticleSystem::createParticleDescriptionFromCocos2DPlist(const GMString& content)
{
	GMCocos2DParticleDescriptionProxy proxy;
	GMXML::parsePlist(content, proxy);
	GMParticleDescription desc;

	// 粒子发射器属性
	desc.setEmitterEmitAngle(proxy.getAngle());
	desc.setEmitterEmitAngleV(proxy.getAngleVariance());

	desc.setEmitterEmitSpeed(proxy.getSpeed());
	desc.setEmitterEmitSpeedV(proxy.getSpeedVariance());

	desc.setDuration(proxy.getDuration());

	desc.setEmitterType(proxy.getEmitterType());
	desc.setParticleCount(proxy.getMaxParticles());
	desc.setEmitterPosition(GMVec3(proxy.getSourcePositionx(), proxy.getSourcePositiony(), Z));
	desc.setEmitterPositionV(GMVec3(proxy.getSourcePositionVariancex(), proxy.getSourcePositionVariancey(), Z));
	desc.setEmitRate(proxy.getMaxParticles() / proxy.getParticleLifespan());

	// 粒子属性
	desc.setLife(proxy.getParticleLifespan());
	desc.setLifeV(proxy.getParticleLifespanVariance());

	desc.setBeginColor(GMVec4(
		proxy.getStartColorRed(),
		proxy.getStartColorGreen(),
		proxy.getStartColorBlue(),
		proxy.getStartColorAlpha())
	);
	desc.setBeginColorV(GMVec4(
		proxy.getStartColorVarianceRed(),
		proxy.getStartColorVarianceGreen(),
		proxy.getStartColorVarianceBlue(),
		proxy.getStartColorVarianceAlpha())
	);
	desc.setEndColor(GMVec4(
		proxy.getFinishColorRed(),
		proxy.getFinishColorGreen(),
		proxy.getFinishColorBlue(),
		proxy.getFinishColorAlpha())
	);
	desc.setEndColorV(GMVec4(
		proxy.getFinishColorVarianceRed(),
		proxy.getFinishColorVarianceGreen(),
		proxy.getFinishColorVarianceBlue(),
		proxy.getFinishColorVarianceAlpha())
	);

	desc.setBeginSize(proxy.getStartParticleSize());
	desc.setBeginSizeV(proxy.getStartParticleSizeVariance());
	desc.setEndSize(proxy.getFinishParticleSize());
	desc.setEndSizeV(proxy.getFinishParticleSizeVariance());

	desc.setMotionMode(proxy.getPositionType());
	desc.getGravityMode().setGravity(GMVec3(proxy.getGravityx(), proxy.getGravityy(), Z));
	desc.getGravityMode().setRadialAcceleration(proxy.getRadialAcceleration());
	desc.getGravityMode().setRadialAccelerationV(proxy.getRadialAccelVariance());
	desc.getGravityMode().setTangentialAcceleration(proxy.getTangentialAcceleration());
	desc.getGravityMode().setTangentialAccelerationV(proxy.getTangentialAccelVariance());

	desc.getRadiusMode().setBeginRadius(proxy.getMaxRadius());
	desc.getRadiusMode().setBeginRadiusV(proxy.getMaxRadiusVariance());
	desc.getRadiusMode().setEndRadius(proxy.getMinRadius());
	desc.getRadiusMode().setEndRadiusV(proxy.getMinRadiusVariance());
	desc.getRadiusMode().setSpinPerSecond(proxy.getRotatePerSecond());
	desc.getRadiusMode().setSpinPerSecondV(proxy.getRotatePerSecondVariance());
	return desc;
}

GMParticleEmitter::GMParticleEmitter(GMParticleSystem* system)
{
	D(d);
	d->system = system;
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

	eff->setParticleDescription(desc);
	setParticleEffect(eff);
}

void GMParticleEmitter::addParticle()
{
	D(d);
	if (d->particles.size() < static_cast<GMsize_t>(getParticleCount()))
	{
		GMParticlePool& pool = d->system->getParticleSystemManager()->getPool();
		GMParticle* particle = pool.alloc();
		if (particle)
		{
			d->particles.push_back(particle);
			d->effect->initParticle(this, particle);
		}
	}
}

void GMParticleEmitter::emitParticles(GMDuration dt)
{
	D(d);
	GMfloat dtEmit = 1.f / d->emitRate;
	if (d->particles.size() < static_cast<GMsize_t>(d->particleCount))
	{
		d->emitCounter += dt;
	}

	while (d->particles.size() < static_cast<GMsize_t>(d->particleCount) && d->emitCounter > 0)
	{
		// 发射待发射的粒子
		addParticle();
		d->emitCounter -= dtEmit;
	}

	d->elapsed += dt;
	// 如果duration是个负数，表示永远发射
	if (d->duration >= 0 && d->duration < d->elapsed)
	{
		d->elapsed = 0;
		stopEmit();
	}
}

void GMParticleEmitter::setParticleEffect(GMParticleEffect* effect)
{
	D(d);
	d->effect.reset(effect);
}

void GMParticleEmitter::update(GMDuration dt)
{
	D(d);
	if (d->canEmit)
	{
		emitParticles(dt);
		d->effect->update(this, dt);
	}
}

void GMParticleEmitter::startEmit()
{
	D(d);
	d->canEmit = true;
}

void GMParticleEmitter::stopEmit()
{
	D(d);
	d->canEmit = false;

	// TODO
	// 释放未发射粒子
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

void GMParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	D(d);
	GMVec3 randomPos(RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1));
	particle->setPosition(emitter->getEmitPosition() + emitter->getEmitPositionV() * randomPos);

	particle->setStartPosition(emitter->getEmitPosition());
	particle->setChangePosition(particle->getPosition());
	particle->setRemainingLife(Max(.1f, getLife() + getLifeV() * RandomMt19937::random_real(-1, 1)));

	GMVec4 randomBeginColor(RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1));
	GMVec4 randomEndColor(RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1), RandomMt19937::random_real(-1, 1));

	GMVec4 beginColor, endColor;
	beginColor = Clamp(getBeginColor() + getBeginColorV() * randomBeginColor, 0, 1);
	endColor = Clamp(getEndColor() + getEndColorV() * randomEndColor, 0, 1);
}

GMParticleSystemManager::GMParticleSystemManager(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->pool.init(1024); //事先分配若干个粒子
}

void GMParticleSystemManager::addParticleSystem(AUTORELEASE GMParticleSystem* ps)
{
	D(d);
	ps->setParticleSystemManager(this);
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

void GMParticleSystemManager::update(GMDuration dt)
{
	// TODO 考虑成异步
	D(d);
	for (decltype(auto) system : d->particleSystems)
	{
		system->update(dt);
	}
}