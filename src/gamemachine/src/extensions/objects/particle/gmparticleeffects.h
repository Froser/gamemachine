#ifndef __GMPARTICLEEFFECT_H__
#define __GMPARTICLEEFFECT_H__
#include <gmcommon.h>
#include "gmparticle.h"
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMParticleEffectImplBase)
{
	bool loaded = false;
	const IRenderContext* context = nullptr;
	GMComputeBufferHandle particles = 0;
	GMComputeUAVHandle particlesUAV = 0;
	GMComputeBufferHandle particleCpuResult = 0;
	GMComputeBufferHandle constant = 0;
};

class GMParticleEffectImplBase : public GMParticleEffect
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
	virtual void onContextAttached(const IRenderContext* context) = 0;
};

class GMGravityParticleEffect : public GMParticleEffectImplBase
{
	typedef GMParticleEffectImplBase Base;

public:
	GMGravityParticleEffect();
	~GMGravityParticleEffect();

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter* emitter, GMDuration dt);

protected:
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;
	virtual void onContextAttached(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);

protected:
	static Map<const IRenderContext*, IComputeShaderProgram*> s_program;
	static Map<const IRenderContext*, GMAtomic<GMint32>> s_ref;
};

class GMRadialParticleEffect : public GMParticleEffectImplBase
{
	typedef GMParticleEffectImplBase Base;

public:
	GMRadialParticleEffect();
	~GMRadialParticleEffect();

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter* emitter, GMDuration dt);
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;
	virtual void onContextAttached(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);

protected:
	static Map<const IRenderContext*, IComputeShaderProgram*> s_program;
	static Map<const IRenderContext*, GMAtomic<GMint32>> s_ref;
};


END_NS
#endif