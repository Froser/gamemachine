#ifndef __GMPARTICLEEFFECT_COCOS2D_H__
#define __GMPARTICLEEFFECT_COCOS2D_H__
#include <gmcommon.h>
#include "../gmparticle.h"
#include "gmparticle_cocos2d.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMParticleEffect_Cocos2D)
{
	GMParticleEmitter_Cocos2D* emitter = nullptr;
	GMParticleMotionMode motionMode = GMParticleMotionMode::Free;
	GMParticleGravityMode gravityMode;
	GMParticleRadiusMode radiusMode;
	GMfloat life = 0;
	GMfloat lifeV = 0;
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

class GM_EXPORT GMParticleEffect_Cocos2D : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleEffect_Cocos2D)
	GM_DECLARE_PROPERTY(Life, life)
	GM_DECLARE_PROPERTY(LifeV, lifeV)
	GM_DECLARE_PROPERTY(BeginColor, beginColor)
	GM_DECLARE_PROPERTY(BeginColorV, beginColorV)
	GM_DECLARE_PROPERTY(EndColor, endColor)
	GM_DECLARE_PROPERTY(EndColorV, endColorV)
	GM_DECLARE_PROPERTY(BeginSize, beginSize)
	GM_DECLARE_PROPERTY(BeginSizeV, beginSizeV)
	GM_DECLARE_PROPERTY(EndSize, endSize)
	GM_DECLARE_PROPERTY(EndSizeV, endSizeV)
	GM_DECLARE_PROPERTY(BeginSpin, beginSpin)
	GM_DECLARE_PROPERTY(BeginSpinV, beginSpinV)
	GM_DECLARE_PROPERTY(EndSpin, endSpin)
	GM_DECLARE_PROPERTY(EndSpinV, endSpinV)
	GM_DECLARE_PROPERTY(MotionMode, motionMode)
	GM_DECLARE_PROPERTY(GravityMode, gravityMode)
	GM_DECLARE_PROPERTY(RadiusMode, radiusMode)

public:
	GMParticleEffect_Cocos2D(GMParticleEmitter_Cocos2D* emitter);
	~GMParticleEffect_Cocos2D();

public:
	virtual void init();
	virtual void initParticle(GMParticle_Cocos2D* particle);

public:
	void setParticleDescription(GMParticleDescription desc);
	void update(GMDuration dt);

protected:
	virtual void CPUUpdate(GMDuration dt) = 0;
	virtual bool GPUUpdate(GMDuration dt);
	virtual GMString getCode() = 0;
	virtual GMString getEntry() = 0;
	virtual IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) { return nullptr; }
};

class GM_EXPORT GMGravityParticleEffect_Cocos2D : public GMParticleEffect_Cocos2D
{
	typedef GMParticleEffect_Cocos2D Base;
	using Base::Base;

public:
	virtual void initParticle(GMParticle_Cocos2D* particle) override;

protected:
	virtual void CPUUpdate(GMDuration dt) override;

protected:
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);
};

class GM_EXPORT GMRadialParticleEffect_Cocos2D : public GMParticleEffect_Cocos2D
{
	typedef GMParticleEffect_Cocos2D Base;
	using Base::Base;

public:
	virtual void initParticle(GMParticle_Cocos2D* particle) override;

protected:
	virtual void CPUUpdate(GMDuration dt) override;
	virtual GMString getCode() override;
	virtual GMString getEntry() override;
	IComputeShaderProgram* getComputeShaderProgram(const IRenderContext* context) override;

public:
	static void setDefaultCodeAndEntry(const GMString& code, const GMString& entry);
};


END_NS
#endif