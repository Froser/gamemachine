#include "stdafx.h"
#include "gmparticleeffects.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include <gmengine/gmcomputeshadermanager.h>

namespace
{
	const GMVec4 s_rotateStartVector = GMVec4(0, 1, 0, 1);
	static GMString s_gravityCode;
	static GMString s_gravityEntry;
	static GMString s_radialCode;
	static GMString s_radialEntry;
}

GMParticleEffectImplBase::~GMParticleEffectImplBase()
{
	D(d);
	if (d->context)
	{
		auto handles = {
			d->particles,
			d->particlesUAV,
			d->particleCpuResult,
			d->constant,
		};

		for (auto iter = handles.begin(); iter != handles.end(); ++iter)
		{
			if (IComputeShaderProgram* prog = getComputeShaderProgram(d->context))
				prog->release(*iter);
		}
	}
}

bool GMParticleEffectImplBase::GPUUpdate(GMParticleEmitter* emitter, const IRenderContext* context, GMDuration dt)
{
	D(d);
	if (!d->context)
	{
		d->context = context;
	}
	else if (d->context != context)
	{
		gm_error(gm_dbg_wrap("GMParticleEffect is only available for one render context."));
		GM_ASSERT(false);
	}

	// 获取计算着色器
	IComputeShaderProgram* shaderProgram = getComputeShaderProgram(context);
	if (!shaderProgram)
		return false;

	GM_ALIGNED_16(struct) ConstantBuffer
	{
		GMVec3 emitterPosition;
		GMVec3 gravity;
		GMVec3 rotationAxis;
		GMfloat dt;
		GMint32 mode;
	};

	auto& particles = emitter->getParticles();
	auto& progParticles = d->particles;
	auto& progParticlesUAV = d->particlesUAV;
	auto& particleCpuResult = d->particleCpuResult;
	auto& constant = d->constant;

	// 粒子信息
	if (!progParticles)
	{
		shaderProgram->createBuffer(sizeof(*particles[0].data()), gm_sizet_to_uint(particles.size()), nullptr, GMComputeBufferType::UnorderedStructured, &progParticles);
		shaderProgram->createBufferUnorderedAccessView(progParticles, &progParticlesUAV);
	}
	else
	{
		// 如果粒子数量变多了，则重新生成buffer
		GMsize_t sz = shaderProgram->getBufferSize(GMComputeBufferType::Structured, progParticles);
		if (sz < sizeof(*particles[0].data()) * (particles.size()))
		{
			shaderProgram->release(progParticles);
			shaderProgram->release(progParticlesUAV);
			shaderProgram->createBuffer(sizeof(*particles[0].data()), gm_sizet_to_uint(particles.size()), particles.data(), GMComputeBufferType::UnorderedStructured, &progParticles);
			shaderProgram->createBufferUnorderedAccessView(progParticles, &progParticlesUAV);
		}
	}
	shaderProgram->setBuffer(progParticles, GMComputeBufferType::Structured, particles.data(), sizeof(*particles[0].data()) * gm_sizet_to_uint(particles.size()));
	shaderProgram->bindUnorderedAccessView(1, &progParticlesUAV);

	// 传入时间等变量
	ConstantBuffer c = { emitter->getEmitPosition(), getGravityMode().getGravity(), emitter->getRotationAxis(), dt, static_cast<GMint32>(getMotionMode()) };
	if (!constant)
		shaderProgram->createBuffer(sizeof(ConstantBuffer), 1, &c, GMComputeBufferType::Constant, &constant);
	shaderProgram->setBuffer(constant, GMComputeBufferType::Constant, &c, sizeof(ConstantBuffer));
	shaderProgram->bindConstantBuffer(constant);

	// 开始计算
	shaderProgram->dispatch(gm_sizet_to_uint(particles.size()), 1, 1);

	bool canReadFromGPU = shaderProgram->canRead(progParticles);
	if (!canReadFromGPU)
	{
		if (particleCpuResult)
			shaderProgram->release(particleCpuResult);
		shaderProgram->createReadOnlyBufferFrom(progParticles, &particleCpuResult);
	}

	// 处理结果
	{
		// 更新每个粒子的状态
		GMComputeBufferHandle resultHandle = canReadFromGPU ? progParticles : particleCpuResult;
		if (!canReadFromGPU)
			shaderProgram->copyBuffer(resultHandle, progParticles);
		typedef GM_PRIVATE_NAME(GMParticle) ParticleData;
		const ParticleData* resultPtr = static_cast<ParticleData*>(shaderProgram->mapBuffer(resultHandle));
		memcpy_s(particles.data(), sizeof(ParticleData) * particles.size(), resultPtr, sizeof(*particles[0].data()) * particles.size());

		// 将存活的粒子放入临时容器，最后交换
		Vector<GMParticle> tmp;
		tmp.reserve(particles.size());
		for (GMsize_t i = 0; i < particles.size(); ++i)
		{
			auto& particle = particles[i];
			if (particle.getRemainingLife() > 0)
			{
				tmp.push_back(particle);
			}
		}
		particles.swap(tmp);

		shaderProgram->unmapBuffer(resultHandle);
	}
	return true;
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

void GMGravityParticleEffect::CPUUpdate(GMParticleEmitter* emitter, GMDuration dt)
{
	D(d);
	auto& particles = emitter->getParticles();
	for (auto iter = particles.begin(); iter != particles.end();)
	{
		GMParticle& particle = *iter;
		particle.setRemainingLife(particle.getRemainingLife() - dt);
		if (particle.getRemainingLife() > 0)
		{
			GMVec3 offset = Zero<GMVec3>();
			GMVec3 radial = Zero<GMVec3>();
			GMVec3 tangential = Zero<GMVec3>();

			// 径向加速度
			if (!FuzzyCompare(particle.getChangePosition().getX(), 0)
				|| !FuzzyCompare(particle.getChangePosition().getY(), 0)
				|| !FuzzyCompare(particle.getChangePosition().getZ(), 0))
			{
				radial = Normalize(particle.getGravityModeData().initialVelocity);
			}
			tangential = radial;
			radial *= particle.getGravityModeData().radialAcceleration;

			GMfloat y = tangential.getX();
			tangential.setX(-tangential.getY());
			tangential.setY(y);
			tangential *= particle.getGravityModeData().tangentialAcceleration;

			// 计算合力
			offset = (radial + tangential + getGravityMode().getGravity()) * dt;
			
			// 移动粒子
			particle.getGravityModeData().initialVelocity += offset;
			particle.setChangePosition(particle.getChangePosition() + particle.getGravityModeData().initialVelocity * dt);

			particle.setColor(particle.getColor() + particle.getDeltaColor() * dt);
			particle.setSize(Max(0, particle.getSize() + particle.getDeltaSize() * dt));
			particle.setRotation(particle.getRotation() + particle.getDeltaRotation() * dt);

			if (getMotionMode() == GMParticleMotionMode::Relative)
			{
				// 跟随发射器
				particle.setPosition(particle.getChangePosition() + emitter->getEmitPosition() - particle.getStartPosition());
			}
			else
			{
				GM_ASSERT(getMotionMode() == GMParticleMotionMode::Free);
				particle.setPosition(particle.getChangePosition());
			}
			++iter;
		}
		else
		{
			iter = particles.erase(iter);
		}
	}
}

GMString GMGravityParticleEffect::getCode()
{
	return s_gravityCode;
}

GMString GMGravityParticleEffect::getEntry()
{
	return s_gravityEntry;
}

IComputeShaderProgram* GMGravityParticleEffect::getComputeShaderProgram(const IRenderContext* context)
{
	D(d);
	if (getCode().isEmpty())
		return false;

	return GMComputeShaderManager::instance().getComputeShaderProgram(context, GMCS_PARTICLE_GRAVITY, L".", getCode(), getEntry());
}

void GMGravityParticleEffect::setDefaultCodeAndEntry(const GMString& code, const GMString& entry)
{
	s_gravityCode = code;
	s_gravityEntry = entry;
}

void GMRadialParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	GMParticleEffect::initParticle(emitter, particle);

	GMfloat beginRadius = getRadiusMode().getBeginRadius() + getRadiusMode().getBeginRadiusV() * GMRandomMt19937::random_real(-1.f, 1.f);
	GMfloat endRadius = getRadiusMode().getEndRadius() + getRadiusMode().getEndRadiusV() * GMRandomMt19937::random_real(-1.f, 1.f);

	particle->getRadiusModeData().radius = beginRadius;
	particle->getRadiusModeData().deltaRadius = (endRadius - beginRadius) / particle->getRemainingLife();

	particle->getRadiusModeData().angle = emitter->getEmitAngle() + emitter->getEmitAngleV() * GMRandomMt19937::random_real(-1.f, 1.f);
	particle->getRadiusModeData().degressPerSecond = Radian(getRadiusMode().getSpinPerSecond() + getRadiusMode().getSpinPerSecondV() * GMRandomMt19937::random_real(-1.f, 1.f));
}

void GMRadialParticleEffect::CPUUpdate(GMParticleEmitter* emitter, GMDuration dt)
{
	auto& particles = emitter->getParticles();
	for (auto iter = particles.begin(); iter != particles.end();)
	{
		GMParticle& particle = *iter;
		particle.setRemainingLife(particle.getRemainingLife() - dt);
		if (particle.getRemainingLife() > 0)
		{
			particle.getRadiusModeData().angle += particle.getRadiusModeData().degressPerSecond * dt;
			particle.getRadiusModeData().radius += particle.getRadiusModeData().deltaRadius * dt;

			GMQuat rotationQuat = Rotate(particle.getRadiusModeData().angle, emitter->getRotationAxis());
			GMVec3 changePosition = particle.getChangePosition();
			changePosition = s_rotateStartVector * rotationQuat * particle.getRadiusModeData().radius;
			particle.setChangePosition(changePosition);

			if (getMotionMode() == GMParticleMotionMode::Relative)
			{
				// 跟随发射器
				particle.setPosition(particle.getChangePosition() + particle.getStartPosition());
			}
			else
			{
				GM_ASSERT(getMotionMode() == GMParticleMotionMode::Free);
				particle.setPosition(particle.getChangePosition() + emitter->getEmitPosition());
			}

			particle.setColor(particle.getColor() + particle.getDeltaColor() * dt);
			particle.setSize(Max(0, particle.getSize() + particle.getDeltaSize() * dt));
			particle.setRotation(particle.getRotation() + particle.getDeltaRotation() * dt);
			++iter;
		}
		else
		{
			iter = particles.erase(iter);
		}
	}
}

GMString GMRadialParticleEffect::getCode()
{
	return s_radialCode;
}

GMString GMRadialParticleEffect::getEntry()
{
	return s_radialEntry;
}

IComputeShaderProgram* GMRadialParticleEffect::getComputeShaderProgram(const IRenderContext* context)
{
	D(d);
	if (getCode().isEmpty())
		return false;

	return GMComputeShaderManager::instance().getComputeShaderProgram(context, GMCS_PARTICLE_RADIAL, L".", getCode(), getEntry());
}

void GMRadialParticleEffect::setDefaultCodeAndEntry(const GMString& code, const GMString& entry)
{
	s_radialCode = code;
	s_radialEntry = entry;
}