#ifndef __GMPARTICLE_H__
#define __GMPARTICLE_H__
#include <gmcommon.h>
BEGIN_NS

enum class GMParticleEmitterType
{
	Gravity,
	Radius,
};

enum class GMParticleMotionMode
{
	Free,
	Relative
};

struct GMParticleGravityMode
{
	GMVec3 initialVelocity = Zero<GMVec3>();
	GMVec3 radialAcceleration = Zero<GMVec3>();
} gravityMode;

struct GMParticleRadiusMode
{
	GMfloat angel = 0;
	GMfloat degressPerSecond = 0;
	GMfloat radius = 0;
	GMfloat deltaRadius = 0;
};

GM_PRIVATE_OBJECT(GMParticleDescription)
{
	GMVec3 emitterPosition = Zero<GMVec3>(); //<! 发射器位置
	GMVec3 emitterPositionV = Zero<GMVec3>();
	GMVec3 emitterEmitAngle = Zero<GMVec3>(); //!< 发射器发射角度
	GMVec3 emitterEmitAngleV = Zero<GMVec3>();
	GMfloat emitterEmitSpeed = 0; //!< 发射器发射速率
	GMfloat emitterEmitSpeedV = 0;
	GMint particleCount; //!< 粒子数目
	GMfloat emitRate; //!< 每秒发射速率
	GMfloat duration; //!< 发射时间
	GMParticleEmitterType emitterType = GMParticleEmitterType::Gravity;
	GMParticleMotionMode motionMode = GMParticleMotionMode::Free;
	GMfloat life = 0;
	GMfloat lifeV = 0;
	
	GMVec4 beginColor = Zero<GMVec4>();
	GMVec4 beginColorV = Zero<GMVec4>();
	GMVec4 endColor = Zero<GMVec4>();
	GMVec4 endColorV = Zero<GMVec4>();

	GMfloat beginSize = 0;
	GMfloat beginSizeV = 0;
	GMfloat endSize = 0;
	GMfloat endSizeV = 0;

	GMfloat beginSpin = 0;
	GMfloat beginSpinV = 0;
	GMfloat endSpin = 0;
	GMfloat endSpinV = 0;

	GMParticleGravityMode gravityMode;
	GMParticleRadiusMode radiusMode;
};

class GMParticleDescription : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleDescription)
	GM_ALLOW_COPY_DATA(GMParticleDescription)

	GM_DECLARE_PROPERTY(EmitterPosition, emitterPosition, GMVec3)
	GM_DECLARE_PROPERTY(EmitterPositionV, emitterPositionV, GMVec3)
	GM_DECLARE_PROPERTY(EmitterEmitAngle, emitterEmitAngle, GMVec3)
	GM_DECLARE_PROPERTY(EmitterEmitAngleV, emitterEmitAngleV, GMVec3)
	GM_DECLARE_PROPERTY(EmitterEmitSpeed, emitterEmitSpeed, GMfloat)
	GM_DECLARE_PROPERTY(EmitterEmitSpeedV, emitterEmitSpeedV, GMfloat)
	GM_DECLARE_PROPERTY(ParticleCount, particleCount, GMint)
	GM_DECLARE_PROPERTY(EmitRate, emitRate, GMfloat)
	GM_DECLARE_PROPERTY(Duration, duration, GMfloat)
	GM_DECLARE_PROPERTY(EmitterType, emitterType, GMParticleEmitterType)
	GM_DECLARE_PROPERTY(MotionMode, motionMode, GMParticleMotionMode)
	GM_DECLARE_PROPERTY(Life, life, GMfloat)
	GM_DECLARE_PROPERTY(LifeV, lifeV, GMfloat)
	GM_DECLARE_PROPERTY(BeginColor, beginColor, GMVec4)
	GM_DECLARE_PROPERTY(BeginColorV, beginColorV, GMVec4)
	GM_DECLARE_PROPERTY(EndColor, endColor, GMVec4)
	GM_DECLARE_PROPERTY(EndColorV, endColorV, GMVec4)
	GM_DECLARE_PROPERTY(BeginSize, beginSize, GMfloat)
	GM_DECLARE_PROPERTY(BeginSizeV, beginSizeV, GMfloat)
	GM_DECLARE_PROPERTY(EndSize, endSize, GMfloat)
	GM_DECLARE_PROPERTY(EndSizeV, endSizeV, GMfloat)
	GM_DECLARE_PROPERTY(BeginSpin, beginSpin, GMfloat)
	GM_DECLARE_PROPERTY(BeginSpinV, beginSpinV, GMfloat)
	GM_DECLARE_PROPERTY(EndSpin, endSpin, GMfloat)
	GM_DECLARE_PROPERTY(EndSpinV, endSpinV, GMfloat)
	GM_DECLARE_PROPERTY(GravityMode, gravityMode, GMParticleGravityMode)
	GM_DECLARE_PROPERTY(RadiusMode, radiusMode, GMParticleRadiusMode)
};

class GMParticleManager : public GMObject
{
public:

};


GM_PRIVATE_OBJECT(GMParticle)
{
	GMVec3 position = Zero<GMVec4>();
	GMVec3 startPosition = Zero<GMVec4>();
	GMVec3 velocity = Zero<GMVec4>();
	GMVec4 color = Zero<GMVec4>();
	GMVec4 deltaColor = Zero<GMVec4>();
	GMfloat size = 0;
	GMfloat currentSize = 0;
	GMfloat deltaSize = 0;
	GMQuat rotation = Identity<GMQuat>();
	GMQuat deltaRotation = Identity<GMQuat>();
	GMfloat remainingLife = 0;
	
	struct GravityModeData
	{
		GMVec2  initialVelocity;
		GMfloat radialAcceleration;
		GMfloat tangentialAcceleration;
	} gravityModeData;

	struct RadiusModeData
	{
		GMfloat angle;
		GMfloat degressPerSecond;
		GMfloat radius;
		GMfloat delatRadius;
	} radiusModeData;
};

class GMParticle : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticle)
	GM_ALLOW_COPY_DATA(GMParticle)
	GM_DECLARE_PROPERTY(Position, position, GMVec3)
	GM_DECLARE_PROPERTY(StartPosition, startPosition, GMVec3)
	GM_DECLARE_PROPERTY(Velocity, velocity, GMVec3)
	GM_DECLARE_PROPERTY(Color, color, GMVec4)
	GM_DECLARE_PROPERTY(DeltaColor, deltaColor, GMVec4)
	GM_DECLARE_PROPERTY(Size, size, GMfloat)
	GM_DECLARE_PROPERTY(CurrentSize, currentSize, GMfloat)
	GM_DECLARE_PROPERTY(DeltaSize, deltaSize, GMfloat)
	GM_DECLARE_PROPERTY(Rotation, rotation, GMQuat)
	GM_DECLARE_PROPERTY(DeltaRotation, deltaRotation, GMQuat)
	GM_DECLARE_PROPERTY(RemainingLife, remainingLife, GMfloat)
	GM_DECLARE_PROPERTY(GravityModeData, gravityModeData, GM_PRIVATE_NAME(GMParticle)::GravityModeData)
	GM_DECLARE_PROPERTY(RadiusModeData, radiusModeData, GM_PRIVATE_NAME(GMParticle)::RadiusModeData)

public:
	GMParticle() = default;
};

GM_PRIVATE_OBJECT(GMParticleEmitter)
{
	GMVec3 emitPosition = Zero<GMVec3>();
	GMVec3 emitPositionV = Zero<GMVec3>();
	GMVec3 emitAngle = Zero<GMVec3>();
	GMVec3 emitAngleV = Zero<GMVec3>();
	GMVec3 emitSpeed = Zero<GMVec3>();
	GMVec3 emitSpeedV = Zero<GMVec3>();
	GMint particleCount = 0;
	GMfloat emitRate = 0;
	GMfloat duration = 0;
	GMOwnedPtr<GMParticleEffect> effect;
};

class GMParticleEmitter : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleEmitter)
	GM_DECLARE_PROPERTY(EmitPosition, emitPosition, GMVec3)
	GM_DECLARE_PROPERTY(EmitPositionV, emitPositionV, GMVec3)
	GM_DECLARE_PROPERTY(EmitAngle, emitAngle, GMVec3)
	GM_DECLARE_PROPERTY(EmitAngleV, emitAngleV, GMVec3)
	GM_DECLARE_PROPERTY(EmitSpeed, emitSpeed, GMVec3)
	GM_DECLARE_PROPERTY(EmitSpeedV, emitSpeedV, GMVec3)
	GM_DECLARE_PROPERTY(ParticleCount, particleCount, GMint)
	GM_DECLARE_PROPERTY(EmitRate, emitRate, GMfloat)
	GM_DECLARE_PROPERTY(Duration, duration, GMfloat)

public:
	void setDescription(const GMParticleDescription& desc);
	void setParticleEffect(GMParticleEffect* effect);
};

GM_PRIVATE_OBJECT(GMParticleEffect)
{
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
};

class GMParticleEffect : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleEffect)
	GM_DECLARE_PROPERTY(Life, life, GMfloat)
	GM_DECLARE_PROPERTY(LifeV, lifeV, GMfloat)
	GM_DECLARE_PROPERTY(BeginColor, beginColor, GMVec4)
	GM_DECLARE_PROPERTY(BeginColorV, beginColorV, GMVec4)
	GM_DECLARE_PROPERTY(EndColor, endColor, GMVec4)
	GM_DECLARE_PROPERTY(EndColorV, endColorV, GMVec4)
	GM_DECLARE_PROPERTY(BeginSize, beginSize, GMfloat)
	GM_DECLARE_PROPERTY(BeginSizeV, beginSizeV, GMfloat)
	GM_DECLARE_PROPERTY(EndSize, endSize, GMfloat)
	GM_DECLARE_PROPERTY(EndSizeV, endSizeV, GMfloat)
	GM_DECLARE_PROPERTY(BeginSpin, beginSpin, GMfloat)
	GM_DECLARE_PROPERTY(BeginSpinV, beginSpinV, GMfloat)
	GM_DECLARE_PROPERTY(EndSpin, endSpin, GMfloat)
	GM_DECLARE_PROPERTY(EndSpinV, endSpinV, GMfloat)
	GM_DECLARE_PROPERTY(MotionMode, motionMode, GMParticleMotionMode)
	GM_DECLARE_PROPERTY(GravityMode, gravityMode, GMParticleGravityMode)
	GM_DECLARE_PROPERTY(RadiusMode, radiusMode, GMParticleRadiusMode)

public:
	void setParticleDescription(const GMParticleDescription& desc);

public:
	virtual void init(GMParticleEmitter* emitter, GMParticle* particle);
	virtual void update(GMParticleEmitter* emitter, GMfloat dt) = 0;
};

GM_PRIVATE_OBJECT(GMParticleSystem)
{
	GMOwnedPtr<GMParticleEmitter> emitter;
	ITexture* texture = nullptr;
};

class GMParticleSystem : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleSystem)

public:
	GMParticleSystem();

public:
	void setDescription(const GMParticleDescription& desc);

public:
	inline void setTexture(ITexture* texture) GM_NOEXCEPT
	{
		D(d);
		d->texture = texture;
	}
};

GM_PRIVATE_OBJECT(GMParticlePool)
{
	Vector<GMOwnedPtr<GMParticle>> particlePool;
	GMsize_t index = 0;
};

class GMParticlePool : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticlePool)

public:
	void init(GMsize_t count);
	void free();
	GMParticle* alloc();
};

END_NS
#endif