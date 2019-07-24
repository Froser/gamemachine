#ifndef __GMPARTICLEEFFECT_COCOS2D_H__
#define __GMPARTICLEEFFECT_COCOS2D_H__
#include <gmcommon.h>
#include "../gmparticle.h"
#include "gmparticle_cocos2d.h"
BEGIN_NS

GM_PRIVATE_CLASS(GMParticleEffect_Cocos2D);
class GM_EXPORT GMParticleEffect_Cocos2D : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMParticleEffect_Cocos2D)
	GM_DISABLE_COPY_ASSIGN(GMParticleEffect_Cocos2D)
	GM_DECLARE_PROPERTY(GMDuration, Life)
	GM_DECLARE_PROPERTY(GMDuration, LifeV)
	GM_DECLARE_PROPERTY(GMVec4, BeginColor)
	GM_DECLARE_PROPERTY(GMVec4, BeginColorV)
	GM_DECLARE_PROPERTY(GMVec4, EndColor)
	GM_DECLARE_PROPERTY(GMVec4, EndColorV)
	GM_DECLARE_PROPERTY(GMfloat, BeginSize)
	GM_DECLARE_PROPERTY(GMfloat, BeginSizeV)
	GM_DECLARE_PROPERTY(GMfloat, EndSize)
	GM_DECLARE_PROPERTY(GMfloat, EndSizeV)
	GM_DECLARE_PROPERTY(GMfloat, BeginSpin)
	GM_DECLARE_PROPERTY(GMfloat, BeginSpinV)
	GM_DECLARE_PROPERTY(GMfloat, EndSpin)
	GM_DECLARE_PROPERTY(GMfloat, EndSpinV)
	GM_DECLARE_PROPERTY(GMParticleMotionMode, MotionMode)
	GM_DECLARE_PROPERTY(GMParticleGravityMode, GravityMode)
	GM_DECLARE_PROPERTY(GMParticleRadiusMode, RadiusMode)

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