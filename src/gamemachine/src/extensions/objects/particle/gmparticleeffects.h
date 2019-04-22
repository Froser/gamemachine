#ifndef __GMPARTICLEEFFECT_H__
#define __GMPARTICLEEFFECT_H__
#include <gmcommon.h>
#include "gmparticle.h"
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGravityParticleEffect)
{
	Map<IComputeShaderProgram*, bool> loaded;
	Map<IComputeShaderProgram*, GMComputeBufferHandle> particles;
	Map<IComputeShaderProgram*, GMComputeUAVHandle> particlesUAV;
	Map<IComputeShaderProgram*, GMComputeBufferHandle> particleCpuResult;
	Map<IComputeShaderProgram*, GMComputeBufferHandle> constant;
};

class GMParticleEffectImplBase : public GMParticleEffect
{
	GM_DECLARE_PRIVATE_NGO(GMGravityParticleEffect)

public:
	~GMParticleEffectImplBase();

protected:
	virtual bool GPUUpdate(GMParticleEmitter* emitter, IComputeShaderProgram* shaderProgram, GMDuration dt);

protected:
	virtual GMString getCode() = 0;
	virtual GMString getEntry() = 0;
};

class GMGravityParticleEffect : public GMParticleEffectImplBase
{
	typedef GMParticleEffectImplBase Base;

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter* emitter, GMDuration dt);

protected:
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
};

class GMRadialParticleEffect : public GMParticleEffectImplBase
{
	typedef GMParticleEffectImplBase Base;

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter* emitter, GMDuration dt);
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
};


END_NS
#endif