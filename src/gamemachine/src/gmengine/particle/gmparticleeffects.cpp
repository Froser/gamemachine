#include "stdafx.h"
#include "gmparticleeffects.h"

void GMGravityParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	GMParticleEffect::initParticle(emitter, particle);

	GMfloat particleSpeed = emitter->getEmitSpeed() + emitter->getEmitSpeedV() * GMRandomMt19937::random_real(-1.f, 1.f);
	GMfloat angle = emitter->getEmitAngle() + emitter->getEmitAngleV() * GMRandomMt19937::random_real(-1.f, 1.f);

	// TODO 目前只考虑二维的，之后会要指定一个方向向量
	GMVec3 particleDirection = GMVec3(Cos(Radians(angle)), Sin(Radians(angle)), 0);
	particle->getGravityModeData().initialVelocity = particleDirection * particleSpeed;
	particle->getGravityModeData().tangentialAcceleration = getGravityMode().getTangentialAcceleration() + getGravityMode().getTangentialAccelerationV() * GMRandomMt19937::random_real(-1.f, 1.f);
	particle->getGravityModeData().radialAcceleration = getGravityMode().getRadialAcceleration() + getGravityMode().getRadialAccelerationV() * GMRandomMt19937::random_real(-1.f, 1.f);
}

void GMGravityParticleEffect::update(GMParticleEmitter* emitter, GMDuration dt)
{
	D(d);
	auto& particles = emitter->getParticles();
	// TODO 可以变成异步
	for (auto iter = particles.begin(); iter != particles.end();)
	{
		GMParticle* particle = *iter;
		particle->setRemainingLife(particle->getRemainingLife() - dt);
		if (particle->getRemainingLife() > 0)
		{
			GMVec3 offset = Zero<GMVec3>();
			GMVec3 radial = Zero<GMVec3>();
			GMVec3 tangential = Zero<GMVec3>();

			// 径向加速度
			if (!FuzzyCompare(particle->getChangePosition().getX(), 0)
				|| !FuzzyCompare(particle->getChangePosition().getY(), 0)
				|| !FuzzyCompare(particle->getChangePosition().getZ(), 0))
			{
				radial = Normalize(particle->getGravityModeData().initialVelocity);
			}
			tangential = radial;
			radial *= particle->getGravityModeData().radialAcceleration;

			// 切向加速度，这里只考虑了2D情况
			GMfloat y = tangential.getX();
			tangential.setX(-tangential.getY());
			tangential.setY(y);
			tangential *= particle->getGravityModeData().tangentialAcceleration;

			// 计算合力
			offset = (radial + tangential + getGravityMode().getGravity()) * dt;
			
			// 移动粒子
			particle->getGravityModeData().initialVelocity += offset;
			particle->setChangePosition(particle->getChangePosition() + particle->getGravityModeData().initialVelocity * dt);

			particle->setColor(particle->getColor() + particle->getDeltaColor() * dt);
			particle->setSize(Max(0, particle->getSize() + particle->getDeltaSize() * dt));
			particle->setRotation(particle->getRotation() + particle->getDeltaRotation() * dt);

			if (getMotionMode() == GMParticleMotionMode::Relative)
			{
				// 跟随发射器
				particle->setPosition(particle->getChangePosition() + emitter->getEmitPosition() - particle->getStartPosition());
			}
			else
			{
				particle->setPosition(particle->getChangePosition());
			}
			++iter;
		}
		else
		{
			// TODO 移除粒子
			iter = particles.erase(iter);
		}
	}
}

void GMRadialParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	GMParticleEffect::initParticle(emitter, particle);
}

void GMRadialParticleEffect::update(GMParticleEmitter* emitter, GMDuration dt)
{
}
