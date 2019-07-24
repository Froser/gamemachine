#include "stdafx.h"
#include "gmparticleeffects_cocos2d.h"
#include "foundation/gmasync.h"
#include "foundation/gamemachine.h"
#include <gmengine/gmcomputeshadermanager.h>
#include "gmparticle_cocos2d_p.h"

BEGIN_NS

namespace
{
	const GMVec4 s_rotateStartVector = GMVec4(0, 1, 0, 1);
	static GMString s_gravityCode;
	static GMString s_gravityEntry;
	static GMString s_radialCode;
	static GMString s_radialEntry;

	const GMParticleDescription_Cocos2D* toCocos2DDesc(GMParticleDescription desc)
	{
		return static_cast<const GMParticleDescription_Cocos2D*>(desc);
	}
}

GM_PRIVATE_OBJECT_ALIGNED(GMParticleEffect_Cocos2D)
{
	GMParticleEmitter_Cocos2D* emitter = nullptr;
	GMParticleMotionMode motionMode = GMParticleMotionMode::Free;
	GMParticleGravityMode gravityMode;
	GMParticleRadiusMode radiusMode;
	GMDuration life = 0;
	GMDuration lifeV = 0;
	GMVec4 beginColor = 0;
	GMVec4 beginColorV = 0;
	GMVec4 endColor = 0;
	GMVec4 endColorV = 0;
	GMfloat beginSize = 0;
	GMfloat beginSizeV = 0;
	GMfloat endSize = 0;
	GMfloat endSizeV = 0;
	GMfloat beginSpin = 0;
	GMfloat beginSpinV = 0;
	GMfloat endSpin = 0;
	GMfloat endSpinV = 0;
	Map<const IRenderContext*, IComputeShaderProgram*> shaderPrograms;
	bool GPUValid = true;
	GMComputeBufferHandle particles = 0;
	GMComputeSRVHandle particlesSRV = 0;
	GMComputeBufferHandle particlesResult = 0;
	GMComputeSRVHandle particlesUAV = 0;
	GMComputeBufferHandle particleCpuResult = 0;
	GMComputeBufferHandle constant = 0;
};

GMParticleEffect_Cocos2D::~GMParticleEffect_Cocos2D()
{
	D(d);
	if (const IRenderContext* context = d->emitter->getParticleSystem()->getContext())
	{
		auto handles = {
			d->particles,
			d->particlesSRV,
			d->particlesResult,
			d->particlesUAV,
			d->particleCpuResult,
			d->constant,
		};

		for (auto iter = handles.begin(); iter != handles.end(); ++iter)
		{
			if (IComputeShaderProgram* prog = getComputeShaderProgram(context))
				prog->release(*iter);
		}
	}
}

GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMDuration, Life, life)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMDuration, LifeV, lifeV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMVec4, BeginColor, beginColor)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMVec4, BeginColorV, beginColorV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMVec4, EndColor, endColor)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMVec4, EndColorV, endColorV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, BeginSize, beginSize)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, BeginSizeV, beginSizeV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, EndSize, endSize)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, EndSizeV, endSizeV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, BeginSpin, beginSpin)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, BeginSpinV, beginSpinV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, EndSpin, endSpin)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMfloat, EndSpinV, endSpinV)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMParticleMotionMode, MotionMode, motionMode)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMParticleGravityMode, GravityMode, gravityMode)
GM_DEFINE_PROPERTY(GMParticleEffect_Cocos2D, GMParticleRadiusMode, RadiusMode, radiusMode)

GMParticleEffect_Cocos2D::GMParticleEffect_Cocos2D(GMParticleEmitter_Cocos2D* emitter)
{
	GM_CREATE_DATA(GMParticleEffect_Cocos2D);

	D(d);
	d->emitter = emitter;
}

void GMParticleEffect_Cocos2D::init()
{
	D(d);
	const IRenderContext* context = d->emitter->getParticleSystem()->getContext();
	getComputeShaderProgram(context);
}

void GMParticleEffect_Cocos2D::initParticle(GMParticle_Cocos2D* particle)
{
	D(d);
	GMVec3 randomPos(GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f));
	particle->setPosition(d->emitter->getEmitPosition() + d->emitter->getEmitPositionV() * randomPos);

	particle->setStartPosition(d->emitter->getEmitPosition());
	particle->setChangePosition(particle->getPosition());
	particle->setRemainingLife(Max(.1f, getLife() + getLifeV() * GMRandomMt19937::random_real(-1.f, 1.f)));

	GMVec4 randomBeginColor(GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f));
	GMVec4 randomEndColor(GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f));

	GMVec4 beginColor, endColor;
	beginColor = Clamp(getBeginColor() + getBeginColorV() * randomBeginColor, 0, 1);
	endColor = Clamp(getEndColor() + getEndColorV() * randomEndColor, 0, 1);

	GMfloat remainingLifeRev = 1.f / (particle->getRemainingLife());
	particle->setColor(beginColor);
	particle->setDeltaColor((endColor - beginColor) * remainingLifeRev);

	GMfloat beginSize = Max(0, getBeginSize() + getBeginSizeV() * GMRandomMt19937::random_real(-1.f, 1.f));
	GMfloat endSize = Max(0, getEndSize() + getEndSize() * GMRandomMt19937::random_real(-1.f, 1.f));
	particle->setSize(beginSize);
	particle->setDeltaSize((endSize - beginSize) / particle->getRemainingLife());

	GMfloat beginSpin = Radians(Max(0, getBeginSpin() + getBeginSpinV() * GMRandomMt19937::random_real(-1.f, 1.f)));
	GMfloat endSpin = Radians(Max(0, getEndSpin() + getEndSpin() * GMRandomMt19937::random_real(-1.f, 1.f)));
	particle->setRotation(beginSpin);
	particle->setDeltaRotation((endSpin - beginSpin) * remainingLifeRev);
}


void GMParticleEffect_Cocos2D::update(GMDuration dt)
{
	D(d);
	IComputeShaderProgram* computeShader = nullptr;
	if (d->GPUValid)
	{
		if (!GPUUpdate(dt))
			d->GPUValid = false;
	}
	else
	{
		CPUUpdate(dt);
	}
}

bool GMParticleEffect_Cocos2D::GPUUpdate(GMDuration dt)
{
	D(d);
	// 获取计算着色器
	const IRenderContext* context = d->emitter->getParticleSystem()->getContext();
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

	auto& particles = d->emitter->getParticles();
	if (particles.empty())
		return true;

	auto& progParticles = d->particles;
	auto& progParticlesSRV = d->particlesSRV;
	auto& progParticlesResult = d->particlesResult;
	auto& progParticlesUAV = d->particlesUAV;
	auto& particleCpuResult = d->particleCpuResult;
	auto& constant = d->constant;

	// 粒子信息
	if (!progParticles)
	{
		shaderProgram->createBuffer(sizeof(particles[0].dataRef()), gm_sizet_to_uint(particles.size()), nullptr, GMComputeBufferType::Structured, &progParticles);
		shaderProgram->createBufferShaderResourceView(progParticles, &progParticlesSRV);

		shaderProgram->createBuffer(sizeof(particles[0].dataRef()), gm_sizet_to_uint(particles.size()), nullptr, GMComputeBufferType::UnorderedStructured, &progParticlesResult);
		shaderProgram->createBufferUnorderedAccessView(progParticlesResult, &progParticlesUAV);
	}
	else
	{
		// 如果粒子数量变多了，则重新生成buffer
		GMsize_t sz = shaderProgram->getBufferSize(GMComputeBufferType::Structured, progParticles);
		if (sz < sizeof(particles[0].dataRef()) * (particles.size()))
		{
			shaderProgram->release(progParticles);
			shaderProgram->release(progParticlesSRV);
			shaderProgram->release(progParticlesResult);
			shaderProgram->release(progParticlesUAV);
			shaderProgram->createBuffer(sizeof(particles[0].dataRef()), gm_sizet_to_uint(particles.size()), particles.data(), GMComputeBufferType::Structured, &progParticles);
			shaderProgram->createBufferShaderResourceView(progParticles, &progParticlesSRV);
			shaderProgram->createBuffer(sizeof(particles[0].dataRef()), gm_sizet_to_uint(particles.size()), nullptr, GMComputeBufferType::UnorderedStructured, &progParticlesResult);
			shaderProgram->createBufferUnorderedAccessView(progParticlesResult, &progParticlesUAV);
		}
	}
	shaderProgram->setBuffer(progParticles, GMComputeBufferType::Structured, particles.data(), sizeof(particles[0].dataRef()) * gm_sizet_to_uint(particles.size()));
	shaderProgram->bindShaderResourceView(1, &progParticlesSRV);

	// 传入时间等变量
	ConstantBuffer c = { d->emitter->getEmitPosition(), getGravityMode().getGravity(), d->emitter->getRotationAxis(), dt, static_cast<GMint32>(getMotionMode()) };
	if (!constant)
		shaderProgram->createBuffer(sizeof(ConstantBuffer), 1, &c, GMComputeBufferType::Constant, &constant);
	shaderProgram->setBuffer(constant, GMComputeBufferType::Constant, &c, sizeof(ConstantBuffer));
	shaderProgram->bindConstantBuffer(constant);

	// 绑定结果
	shaderProgram->bindUnorderedAccessView(1, &progParticlesUAV);

	// 开始计算
	shaderProgram->dispatch(gm_sizet_to_uint(particles.size()), 1, 1);

	bool canReadFromGPU = shaderProgram->canRead(progParticlesResult);
	if (!canReadFromGPU)
	{
		if (particleCpuResult)
			shaderProgram->release(particleCpuResult);
		shaderProgram->createReadOnlyBufferFrom(progParticlesResult, &particleCpuResult);
	}

	// 处理结果
	{
		// 更新每个粒子的状态
		GMComputeBufferHandle resultHandle = canReadFromGPU ? progParticlesResult : particleCpuResult;
		if (!canReadFromGPU)
			shaderProgram->copyBuffer(resultHandle, progParticlesResult);
		typedef GM_PRIVATE_NAME(GMParticle_Cocos2D) ParticleData;
		const ParticleData* resultPtr = static_cast<ParticleData*>(shaderProgram->mapBuffer(resultHandle));
		memcpy_s(particles.data(), sizeof(ParticleData) * particles.size(), resultPtr, sizeof(particles[0].dataRef()) * particles.size());

		// 将存活的粒子放入临时容器，最后交换
		Vector<GMParticle_Cocos2D> tmp;
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

void GMGravityParticleEffect_Cocos2D::initParticle(GMParticle_Cocos2D* particle)
{
	D(d);
	GMParticleEffect_Cocos2D::initParticle(particle);

	GMfloat particleSpeed = d->emitter->getEmitSpeed() + d->emitter->getEmitSpeedV() * GMRandomMt19937::random_real(-1.f, 1.f);
	GMfloat angle = d->emitter->getEmitAngle() + d->emitter->getEmitAngleV() * GMRandomMt19937::random_real(-1.f, 1.f);

	GMQuat rotationQuat = Rotate(Radians(angle), d->emitter->getRotationAxis());
	particle->getGravityModeData().initialVelocity = Inhomogeneous(s_rotateStartVector * rotationQuat) * particleSpeed;
	particle->getGravityModeData().tangentialAcceleration = getGravityMode().getTangentialAcceleration() + getGravityMode().getTangentialAccelerationV() * GMRandomMt19937::random_real(-1.f, 1.f);
	particle->getGravityModeData().radialAcceleration = getGravityMode().getRadialAcceleration() + getGravityMode().getRadialAccelerationV() * GMRandomMt19937::random_real(-1.f, 1.f);
}

void GMGravityParticleEffect_Cocos2D::CPUUpdate(GMDuration dt)
{
	D_BASE(d, GMParticleEffect_Cocos2D);
	auto& particles = d->emitter->getParticles();
	for (auto iter = particles.begin(); iter != particles.end();)
	{
		GMParticle_Cocos2D& particle = *iter;
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
				particle.setPosition(particle.getChangePosition() + d->emitter->getEmitPosition() - particle.getStartPosition());
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

GMString GMGravityParticleEffect_Cocos2D::getCode()
{
	return s_gravityCode;
}

GMString GMGravityParticleEffect_Cocos2D::getEntry()
{
	return s_gravityEntry;
}

void GMParticleEffect_Cocos2D::setParticleDescription(GMParticleDescription desc)
{
	const GMParticleDescription_Cocos2D* cocos2dDescription = toCocos2DDesc(desc);

	setLife(cocos2dDescription->getLife());
	setLifeV(cocos2dDescription->getLifeV());

	setBeginColor(cocos2dDescription->getBeginColor());
	setBeginColorV(cocos2dDescription->getBeginColorV());
	setEndColor(cocos2dDescription->getEndColor());
	setEndColorV(cocos2dDescription->getEndColorV());

	setBeginSize(cocos2dDescription->getBeginSize());
	setBeginSizeV(cocos2dDescription->getBeginSizeV());
	setEndSize(cocos2dDescription->getEndSize());
	setEndSizeV(cocos2dDescription->getEndSizeV());

	setBeginSpin(cocos2dDescription->getBeginSpin());
	setBeginSpinV(cocos2dDescription->getBeginSpinV());
	setEndSpin(cocos2dDescription->getEndSpin());
	setEndSpinV(cocos2dDescription->getEndSpinV());

	setMotionMode(cocos2dDescription->getMotionMode());
	setGravityMode(cocos2dDescription->getGravityMode());
	setRadiusMode(cocos2dDescription->getRadiusMode());
}

IComputeShaderProgram* GMGravityParticleEffect_Cocos2D::getComputeShaderProgram(const IRenderContext* context)
{
	D(d);
	if (getCode().isEmpty())
		return nullptr;

	return GMComputeShaderManager::instance().getComputeShaderProgram(context, GMCS_PARTICLE_GRAVITY, L".", getCode(), getEntry());
}

void GMGravityParticleEffect_Cocos2D::setDefaultCodeAndEntry(const GMString& code, const GMString& entry)
{
	s_gravityCode = code;
	s_gravityEntry = entry;
}

void GMRadialParticleEffect_Cocos2D::initParticle(GMParticle_Cocos2D* particle)
{
	D_BASE(d, GMParticleEffect_Cocos2D);
	GMParticleEffect_Cocos2D::initParticle(particle);

	GMfloat beginRadius = getRadiusMode().getBeginRadius() + getRadiusMode().getBeginRadiusV() * GMRandomMt19937::random_real(-1.f, 1.f);
	GMfloat endRadius = getRadiusMode().getEndRadius() + getRadiusMode().getEndRadiusV() * GMRandomMt19937::random_real(-1.f, 1.f);

	particle->getRadiusModeData().radius = beginRadius;
	particle->getRadiusModeData().deltaRadius = (endRadius - beginRadius) / particle->getRemainingLife();

	particle->getRadiusModeData().angle = d->emitter->getEmitAngle() + d->emitter->getEmitAngleV() * GMRandomMt19937::random_real(-1.f, 1.f);
	particle->getRadiusModeData().degressPerSecond = Radians(getRadiusMode().getSpinPerSecond() + getRadiusMode().getSpinPerSecondV() * GMRandomMt19937::random_real(-1.f, 1.f));
}

void GMRadialParticleEffect_Cocos2D::CPUUpdate(GMDuration dt)
{
	D_BASE(d, GMParticleEffect_Cocos2D);
	auto& particles = d->emitter->getParticles();
	for (auto iter = particles.begin(); iter != particles.end();)
	{
		GMParticle_Cocos2D& particle = *iter;
		particle.setRemainingLife(particle.getRemainingLife() - dt);
		if (particle.getRemainingLife() > 0)
		{
			particle.getRadiusModeData().angle += particle.getRadiusModeData().degressPerSecond * dt;
			particle.getRadiusModeData().radius += particle.getRadiusModeData().deltaRadius * dt;

			GMQuat rotationQuat = Rotate(particle.getRadiusModeData().angle, d->emitter->getRotationAxis());
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
				particle.setPosition(particle.getChangePosition() + d->emitter->getEmitPosition());
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

GMString GMRadialParticleEffect_Cocos2D::getCode()
{
	return s_radialCode;
}

GMString GMRadialParticleEffect_Cocos2D::getEntry()
{
	return s_radialEntry;
}

IComputeShaderProgram* GMRadialParticleEffect_Cocos2D::getComputeShaderProgram(const IRenderContext* context)
{
	D(d);
	if (getCode().isEmpty())
		return nullptr;

	return GMComputeShaderManager::instance().getComputeShaderProgram(context, GMCS_PARTICLE_RADIAL, L".", getCode(), getEntry());
}

void GMRadialParticleEffect_Cocos2D::setDefaultCodeAndEntry(const GMString& code, const GMString& entry)
{
	s_radialCode = code;
	s_radialEntry = entry;
}

END_NS