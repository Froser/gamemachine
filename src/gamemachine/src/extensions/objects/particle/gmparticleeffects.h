#ifndef __GMPARTICLEEFFECT_H__
#define __GMPARTICLEEFFECT_H__
#include <gmcommon.h>
#include "gmparticle.h"
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMParticleEffectImplBase)
{
	const IRenderContext* context = nullptr;
	GMComputeBufferHandle particles = 0;
	GMComputeSRVHandle particlesSRV = 0;
	GMComputeBufferHandle particlesResult = 0;
	GMComputeSRVHandle particlesUAV = 0;
	GMComputeBufferHandle particleCpuResult = 0;
	GMComputeBufferHandle constant = 0;
};

class GM_EXPORT GMParticleEffectImplBase : public GMParticleEffect
{
	GM_DECLARE_PRIVATE_NGO(GMParticleEffectImplBase)

public:
	~GMParticleEffectImplBase();

protected:
	virtual bool GPUUpdate(GMParticleEmitter* emitter, const IRenderContext* context, GMDuration dt);

protected:
	virtual GMString getCode() = 0;
	virtual GMString getEntry() = 0;
	virtual IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) { return nullptr; }
};

class GM_EXPORT GMGravityParticleEffect : public GMParticleEffectImplBase
{
	typedef GMParticleEffectImplBase Base;

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter* emitter, GMDuration dt);

protected:
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);
};

class GM_EXPORT GMRadialParticleEffect : public GMParticleEffectImplBase
{
	typedef GMParticleEffectImplBase Base;

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter* emitter, GMDuration dt);
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);
};


END_NS
#endif