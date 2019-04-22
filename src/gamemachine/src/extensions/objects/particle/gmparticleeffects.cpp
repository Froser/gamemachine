#include "stdafx.h"
#include "gmparticleeffects.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"

#define __SHADER_CODE( _x ) L ## # _x
#define SHADER_CODE( _x ) __SHADER_CODE( _x )

namespace
{
	const GMVec4 s_rotateStartVector = GMVec4(0, 1, 0, 1);

	auto GL_GRAVITY_COMPUTE = SHADER_CODE(
		#version 430 core\n

		const int GM_MOTION_MODE_FREE = 0;
		const int GM_MOTION_MODE_RELATIVE = 1;

		struct gravity_t
		{
			vec3 initialVelocity;
			float radialAcceleration;
			float tangentialAcceleration;
		};

		struct radius_t
		{
			float angle;
			float degressPerSecond;
			float radius;
			float delatRadius;
		};

		struct particle_t
		{
			vec4 color;
			vec4 deltaColor;
			vec3 position;
			vec3 startPosition;
			vec3 changePosition;
			vec3 velocity;
			float _padding_; // velocity's placeholder
			float size;
			float currentSize;
			float deltaSize;
			float rotation;
			float deltaRotation;
			float remainingLife;

			gravity_t gravityModeData;
			radius_t radiusModeData;
		};\n

		layout(std430, binding = 0) buffer Particle
		{
			particle_t particles[];
		};\n

		layout(std140, binding = 1) uniform Constant
		{
			vec3 emitterPosition;
			vec3 gravity;
			float _padding_; // gravity's placeholder
			float dt;
			int motionMode;
		};

		layout(local_size_x = 1, local_size_y = 1) in;\n

		void main(void)
		{
			uint gid = gl_GlobalInvocationID.x;
			/*
			particles[gid].color = vec4(1, 2, 3, 4);
			particles[gid].deltaColor = vec4(5, 6, 7, 8);
			particles[gid].position = vec3(9, 10, 11);
			particles[gid].startPosition = vec3(12, 13, 14);
			particles[gid].changePosition = vec3(15, 16, 17);
			particles[gid].velocity = vec3(18, 19, 20);
			particles[gid].size = 21;
			particles[gid].currentSize = 22;
			particles[gid].deltaSize = 23;
			particles[gid].rotation = 24;
			particles[gid].deltaRotation = 25;
			particles[gid].remainingLife = 26;
			*/
			particles[gid].remainingLife -= dt;
			if (particles[gid].remainingLife > 0)
			{
				vec3 offset = vec3(0);
				vec3 radial = vec3(0);
				vec3 tangential = vec3(0);
				if (abs(particles[gid].changePosition.x) > .01f &&
					abs(particles[gid].changePosition.y) > .01f &&
					abs(particles[gid].changePosition.z) > .01f)
				{
					radial = normalize(particles[gid].gravityModeData.initialVelocity);
				}
				tangential = radial;
				radial *= particles[gid].gravityModeData.initialVelocity;

				tangential.xyz = vec3(-tangential.y, tangential.x, tangential.z);
				tangential *= particles[gid].gravityModeData.tangentialAcceleration;

				offset = (radial + tangential + gravity) * dt;

				particles[gid].gravityModeData.initialVelocity += offset;
				particles[gid].changePosition += particles[gid].gravityModeData.initialVelocity * dt;
				particles[gid].color += particles[gid].deltaColor * dt;
				particles[gid].size = max(0, particles[gid].size + particles[gid].deltaSize * dt);
				particles[gid].rotation += particles[gid].deltaRotation * dt;

				if (motionMode == GM_MOTION_MODE_RELATIVE)
					particles[gid].position = particles[gid].changePosition + emitterPosition - particles[gid].startPosition;
				else
					particles[gid].position = particles[gid].changePosition;
			}
		}
	);
}

GMParticleEffectImplBase::~GMParticleEffectImplBase()
{
	D(d);
	auto handles = {
		d->particles,
		d->particlesUAV,
		d->particleCpuResult,
		d->constant,
	};

	for (auto iter = handles.begin(); iter != handles.end(); ++iter)
	{
		for (auto handleIter = iter->begin(); handleIter != iter->end(); ++handleIter)
		{
			handleIter->first->release(handleIter->second);
		}
	}
}

bool GMParticleEffectImplBase::GPUUpdate(GMParticleEmitter* emitter, IComputeShaderProgram* shaderProgram, GMDuration dt)
{
	GM_ALIGNED_16(struct) ConstantBuffer
	{
		GMVec3 emitterPosition;
		GMVec3 gravity;
		GMfloat dt;
		GMint32 mode;
	};

	D(d);
	if (!d->loaded[shaderProgram])
	{
		if (getCode().isEmpty())
			return false;

		shaderProgram->load(".", getCode(), getEntry());
		d->loaded[shaderProgram] = true;
	}

	auto& particles = emitter->getParticles();
	auto& progParticles = d->particles[shaderProgram];
	auto& progParticlesUAV = d->particlesUAV[shaderProgram];
	auto& particleCpuResult = d->particleCpuResult[shaderProgram];
	auto& constant = d->constant[shaderProgram];

	// 粒子信息
	if (!progParticles)
	{
		shaderProgram->createBuffer(sizeof(*particles[0].data()), gm_sizet_to_uint(particles.size()), nullptr, GMComputeBufferType::Structured, &progParticles);
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
			shaderProgram->createBuffer(sizeof(*particles[0].data()), gm_sizet_to_uint(particles.size()), particles.data(), GMComputeBufferType::Structured, &progParticles);
			shaderProgram->createBufferUnorderedAccessView(progParticles, &progParticlesUAV);
		}
	}
	shaderProgram->setBuffer(progParticles, GMComputeBufferType::Structured, particles.data(), sizeof(*particles[0].data()) * gm_sizet_to_uint(particles.size()));
	shaderProgram->setUnorderedAccessView(1, &progParticlesUAV);

	// 传入时间等变量
	ConstantBuffer c = { emitter->getEmitPosition(), getGravityMode().getGravity(), dt, static_cast<GMint32>(getMotionMode()) };
	if (!constant)
		shaderProgram->createBuffer(sizeof(ConstantBuffer), 1, &c, GMComputeBufferType::Constant, &constant);
	shaderProgram->setBuffer(constant, GMComputeBufferType::Constant, &c, sizeof(ConstantBuffer));

	// 开始计算
	shaderProgram->dispatch(gm_sizet_to_uint(particles.size()), 1, 1);

	bool canReadFromGPU = shaderProgram->canRead(progParticles);
	if (!canReadFromGPU)
	{
		if (!particleCpuResult)
			shaderProgram->createReadOnlyBufferFrom(progParticles, &particleCpuResult);
	}

	// 处理结果
	{
		// 再来更新每个粒子的状态
		GMComputeBufferHandle resultHandle = canReadFromGPU ? progParticles : particleCpuResult;
		if (!canReadFromGPU)
			shaderProgram->copyBuffer(resultHandle, progParticles);
		typedef GM_PRIVATE_NAME(GMParticle) ParticleData;
		ParticleData* resultPtr = static_cast<ParticleData*>(shaderProgram->mapBuffer(resultHandle));
		memcpy_s(particles.data(), sizeof(ParticleData) * particles.size(), resultPtr, sizeof(*particles[0].data()) * particles.size());

		for (auto iter = particles.begin(); iter != particles.end();)
		{
			auto offset = iter - particles.begin();
			if (resultPtr[offset].remainingLife <= 0)
			{
				iter = particles.erase(iter);
			}
			else
			{
				++iter;
			}
		}

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
	return GL_GRAVITY_COMPUTE;
}

GMString GMGravityParticleEffect::getEntry()
{
	return L"main";
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
			particle.getRadiusModeData().radius += particle.getRadiusModeData().delatRadius * dt;

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
	return L"";
}

GMString GMRadialParticleEffect::getEntry()
{
	return L"";
}