#include "stdafx.h"
#include "gmparticleeffects.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"

namespace
{
	const GMVec4 s_rotateStartVector = GMVec4(0, 1, 0, 1);
}

void GMGravityParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	GMParticleEffect::initParticle(emitter, particle);

	GMfloat particleSpeed = emitter->getEmitSpeed() + emitter->getEmitSpeedV() * GMRandomMt19937::random_real(-1.f, 1.f);
	GMfloat angle = emitter->getEmitAngle() + emitter->getEmitAngleV() * GMRandomMt19937::random_real(-1.f, 1.f);

	GMQuat rotationQuat = Rotate(Radian(angle), emitter->getRotationAxis());
	particle->getGravityModeData().initialVelocity = Inhomogeneous(s_rotateStartVector * rotationQuat) * particleSpeed;
	particle->getGravityModeData().tangentialAcceleration = getGravityMode().getTangentialAcceleration() + getGravityMode().getTangentialAccelerationV() * GMRandomMt19937::random_real(-1.f, 1.f);
	particle->getGravityModeData().radialAcceleration = getGravityMode().getRadialAcceleration() + getGravityMode().getRadialAccelerationV() * GMRandomMt19937::random_real(-1.f, 1.f);
}

void GMGravityParticleEffect::update(GMParticleEmitter* emitter, GMDuration dt)
{
	D(d);
	auto& particles = emitter->getParticles();
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
				GM_ASSERT(getMotionMode() == GMParticleMotionMode::Free);
				particle->setPosition(particle->getChangePosition());
			}
			++iter;
		}
		else
		{
			GMParticlePool& pool = emitter->getParticleSystem()->getParticleSystemManager()->getPool();
			pool.free(*iter);
			iter = particles.erase(iter);
		}
	}
}

void GMRadialParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	GMParticleEffect::initParticle(emitter, particle);

	GMfloat beginRadius = getRadiusMode().getBeginRadius() + getRadiusMode().getBeginRadiusV() * GMRandomMt19937::random_real(-1.f, 1.f);
	GMfloat endRadius = getRadiusMode().getEndRadius() + getRadiusMode().getEndRadiusV() * GMRandomMt19937::random_real(-1.f, 1.f);

	particle->getRadiusModeData().radius = beginRadius;
	particle->getRadiusModeData().delatRadius = (endRadius - beginRadius) / particle->getRemainingLife();

	particle->getRadiusModeData().angle = emitter->getEmitAngle() + emitter->getEmitAngleV() * GMRandomMt19937::random_real(-1.f, 1.f);
	particle->getRadiusModeData().degressPerSecond = Radian(getRadiusMode().getSpinPerSecond() + getRadiusMode().getSpinPerSecondV() * GMRandomMt19937::random_real(-1.f, 1.f));
}

void GMRadialParticleEffect::update(GMParticleEmitter* emitter, GMDuration dt)
{
	auto& particles = emitter->getParticles();
	for (auto iter = particles.begin(); iter != particles.end();)
	{
		GMParticle* particle = *iter;
		particle->setRemainingLife(particle->getRemainingLife() - dt);
		if (particle->getRemainingLife() > 0)
		{
			particle->getRadiusModeData().angle += particle->getRadiusModeData().degressPerSecond * dt;
			particle->getRadiusModeData().radius += particle->getRadiusModeData().delatRadius * dt;

			GMQuat rotationQuat = Rotate(particle->getRadiusModeData().angle, emitter->getRotationAxis());
			GMVec3 changePosition = particle->getChangePosition();
			changePosition = s_rotateStartVector * rotationQuat * particle->getRadiusModeData().radius;
			particle->setChangePosition(changePosition);

			if (getMotionMode() == GMParticleMotionMode::Relative)
			{
				// 跟随发射器
				particle->setPosition(particle->getChangePosition() + particle->getStartPosition());
			}
			else
			{
				GM_ASSERT(getMotionMode() == GMParticleMotionMode::Free);
				particle->setPosition(particle->getChangePosition() + emitter->getEmitPosition());
			}

			particle->setColor(particle->getColor() + particle->getDeltaColor() * dt);
			particle->setSize(Max(0, particle->getSize() + particle->getDeltaSize() * dt));
			particle->setRotation(particle->getRotation() + particle->getDeltaRotation() * dt);
			++iter;
		}
		else
		{
			GMParticlePool& pool = emitter->getParticleSystem()->getParticleSystemManager()->getPool();
			pool.free(*iter);
			iter = particles.erase(iter);
		}
	}
}
