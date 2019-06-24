#ifndef __GMPARTICLEEFFECT_COCOS2D_H__
#define __GMPARTICLEEFFECT_COCOS2D_H__
#include <gmcommon.h>
#include "../gmparticle.h"
#include "gmparticle_cocos2d.h"
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMParticleEffect_Cocos2DImplBase)
{
	const IRenderContext* context = nullptr;
	GMComputeBufferHandle particles = 0;
	GMComputeSRVHandle particlesSRV = 0;
	GMComputeBufferHandle particlesResult = 0;
	GMComputeSRVHandle particlesUAV = 0;
	GMComputeBufferHandle particleCpuResult = 0;
	GMComputeBufferHandle constant = 0;
};

class GM_EXPORT GMParticleEffect_Cocos2DImplBase : public GMParticleEffect_Cocos2D
{
	GM_DECLARE_PRIVATE_NGO(GMParticleEffect_Cocos2DImplBase)

public:
	~GMParticleEffect_Cocos2DImplBase();

protected:
	virtual bool GPUUpdate(IParticleEmitter* emitter, GMDuration dt);

protected:
	virtual GMString getCode() = 0;
	virtual GMString getEntry() = 0;
	virtual IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) { return nullptr; }
};

class GM_EXPORT GMGravityParticleEffect : public GMParticleEffect_Cocos2DImplBase
{
	typedef GMParticleEffect_Cocos2DImplBase Base;

public:
	virtual void initParticle(IParticleEmitter* emitter, GMParticle particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter_Cocos2D* emitter, GMDuration dt);

protected:
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);
};

class GM_EXPORT GMRadialParticleEffect : public GMParticleEffect_Cocos2DImplBase
{
	typedef GMParticleEffect_Cocos2DImplBase Base;

public:
	virtual void initParticle(IParticleEmitter* emitter, GMParticle particle) override;

protected:
	virtual void CPUUpdate(GMParticleEmitter_Cocos2D* emitter, GMDuration dt);
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);
};


END_NS
#endif