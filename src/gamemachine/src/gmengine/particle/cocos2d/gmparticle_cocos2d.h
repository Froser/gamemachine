#ifndef __GMPARTICLE_COCOS2D_H__
#define __GMPARTICLE_COCOS2D_H__
#include <gmcommon.h>
#include <gmgameobject.h>
#include "../gmparticle.h"
#include "gmparticlemodel_cocos2d.h"
BEGIN_NS

class GMParticleSystem_Cocos2D;
class GMParticleEmitter_Cocos2D;
class GMParticleEffect_Cocos2D;

enum class GMParticleModelType
{
	Particle2D,
	Particle3D,
};

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

GM_PRIVATE_CLASS(GMParticleGravityMode);
class GMParticleGravityMode
{
	GM_DECLARE_PRIVATE(GMParticleGravityMode)
	GM_ALLOW_COPY_MOVE(GMParticleGravityMode)
	GM_DECLARE_PROPERTY(Gravity, gravity)
	GM_DECLARE_PROPERTY(TangentialAcceleration, tangentialAcceleration)
	GM_DECLARE_PROPERTY(TangentialAccelerationV, tangentialAccelerationV)
	GM_DECLARE_PROPERTY(RadialAcceleration, radialAcceleration)
	GM_DECLARE_PROPERTY(RadialAccelerationV, radialAccelerationV)

public:
	GMParticleGravityMode();
};

GM_PRIVATE_CLASS(GMParticleRadiusMode);
class GMParticleRadiusMode
{
	GM_DECLARE_PRIVATE(GMParticleRadiusMode)
	GM_ALLOW_COPY_MOVE(GMParticleRadiusMode)
	GM_DECLARE_PROPERTY(BeginRadius, beginRadius)
	GM_DECLARE_PROPERTY(BeginRadiusV, beginRadiusV)
	GM_DECLARE_PROPERTY(EndRadius, endRadius)
	GM_DECLARE_PROPERTY(EndRadiusV, endRadiusV)
	GM_DECLARE_PROPERTY(SpinPerSecond, spinPerSecond)
	GM_DECLARE_PROPERTY(SpinPerSecondV, spinPerSecondV)

public:
	GMParticleRadiusMode();
};

GM_PRIVATE_OBJECT(GMParticleDescription_Cocos2D)
{
	GMVec3 emitterPosition = Zero<GMVec3>(); //<! 发射器位置
	GMVec3 emitterPositionV = Zero<GMVec3>();
	GMfloat emitterEmitAngle = 0; //!< 发射器发射角度
	GMfloat emitterEmitAngleV = 0;
	GMfloat emitterEmitSpeed = 0; //!< 发射器发射速率
	GMfloat emitterEmitSpeedV = 0;
	GMint32 particleCount; //!< 粒子数目
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

	GMBuffer textureImageData;
	GMParticleModelType particleModelType = GMParticleModelType::Particle2D;

	GMVec3 gravityDirection = GMVec3(1, 1, 1); //!< 重力方向，-1表示粒子坐标系与左手坐标系相反
};

//! 表示一个粒子发射器的描述
/*!
  对于一个粒子发射器而言，它的重力方向为GameMachine坐标系y轴负方向，朝向地面。<BR>
  发射的0度角对应GameMachine坐标系(0, 1, 0)。
  所有Angle的单位为角度，而非弧度。
*/
class GMParticleDescription_Cocos2D : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleDescription_Cocos2D)
	GM_ALLOW_COPY_MOVE(GMParticleDescription_Cocos2D)

	GM_DECLARE_PROPERTY(EmitterPosition, emitterPosition)
	GM_DECLARE_PROPERTY(EmitterPositionV, emitterPositionV)
	GM_DECLARE_PROPERTY(EmitterEmitAngle, emitterEmitAngle)
	GM_DECLARE_PROPERTY(EmitterEmitAngleV, emitterEmitAngleV)
	GM_DECLARE_PROPERTY(EmitterEmitSpeed, emitterEmitSpeed)
	GM_DECLARE_PROPERTY(EmitterEmitSpeedV, emitterEmitSpeedV)
	GM_DECLARE_PROPERTY(ParticleCount, particleCount)
	GM_DECLARE_PROPERTY(EmitRate, emitRate)
	GM_DECLARE_PROPERTY(Duration, duration)
	GM_DECLARE_PROPERTY(EmitterType, emitterType)
	GM_DECLARE_PROPERTY(MotionMode, motionMode)
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
	GM_DECLARE_PROPERTY(GravityMode, gravityMode)
	GM_DECLARE_PROPERTY(RadiusMode, radiusMode)
	GM_DECLARE_PROPERTY(TextureImageData, textureImageData)
	GM_DECLARE_PROPERTY(ParticleModelType, particleModelType)

public:
	GMParticleDescription_Cocos2D() = default;
};

GM_ALIGNED_16(struct) GM_PRIVATE_NAME(GMParticle_Cocos2D)
{
	GMVec4 color = Zero<GMVec4>();
	GMVec4 deltaColor = Zero<GMVec4>();
	GMVec3 position = Zero<GMVec3>(); //xyz
	GMVec3 startPosition = Zero<GMVec3>(); //xyz
	GMVec3 changePosition = Zero<GMVec3>(); //xyz
	GMVec3 velocity = Zero<GMVec3>(); //xyz
	GMfloat size = 0;
	GMfloat currentSize = 0;
	GMfloat deltaSize = 0;
	GMfloat rotation = 0;
	GMfloat deltaRotation = 0;
	GMfloat remainingLife = 0;
	
	struct GravityModeData
	{
		GMVec3 initialVelocity;
		GMfloat radialAcceleration;
		GMfloat tangentialAcceleration;
	} gravityModeData;

	struct RadiusModeData
	{
		GMfloat angle;
		GMfloat degressPerSecond;
		GMfloat radius;
		GMfloat deltaRadius;
	} radiusModeData;
};

class GMParticle_Cocos2D
{
	GM_DECLARE_PRIVATE_NGO(GMParticle_Cocos2D)
	GM_DECLARE_PROPERTY(Position, position)
	GM_DECLARE_PROPERTY(StartPosition, startPosition)
	GM_DECLARE_PROPERTY(ChangePosition, changePosition)
	GM_DECLARE_PROPERTY(Velocity, velocity)
	GM_DECLARE_PROPERTY(Color, color)
	GM_DECLARE_PROPERTY(DeltaColor, deltaColor)
	GM_DECLARE_PROPERTY(Size, size)
	GM_DECLARE_PROPERTY(CurrentSize, currentSize)
	GM_DECLARE_PROPERTY(DeltaSize, deltaSize)
	GM_DECLARE_PROPERTY(Rotation, rotation)
	GM_DECLARE_PROPERTY(DeltaRotation, deltaRotation)
	GM_DECLARE_PROPERTY(RemainingLife, remainingLife)
	GM_DECLARE_PROPERTY(GravityModeData, gravityModeData)
	GM_DECLARE_PROPERTY(RadiusModeData, radiusModeData)

public:
	GMParticle_Cocos2D() = default;
};

GM_PRIVATE_OBJECT(GMParticleEmitter_Cocos2D)
{
	GMVec3 emitPosition = Zero<GMVec3>();
	GMVec3 emitPositionV = Zero<GMVec3>();
	GMfloat emitAngle = 0;
	GMfloat emitAngleV = 0;
	GMfloat emitSpeed = 0;
	GMfloat emitSpeedV = 0;
	GMint32 particleCount = 0;
	GMDuration emitRate = 0;
	GMDuration duration = 0;
	GMVec3 rotationAxis = GMVec3(0, 0, 1);
	AUTORELEASE GMParticleEffect_Cocos2D* effect = nullptr;
	Vector<GMParticle_Cocos2D> particles;
	bool canEmit = true;
	GMParticleSystem_Cocos2D* system = nullptr;
	GMDuration emitCounter = 0;
	GMDuration elapsed = 0;
};

class GMParticleEmitter_Cocos2D : public GMObject, public IParticleEmitter
{
	GM_DECLARE_PRIVATE(GMParticleEmitter_Cocos2D)
	GM_DECLARE_PROPERTY(EmitPosition, emitPosition)
	GM_DECLARE_PROPERTY(EmitPositionV, emitPositionV)
	GM_DECLARE_PROPERTY(EmitAngle, emitAngle)
	GM_DECLARE_PROPERTY(EmitAngleV, emitAngleV)
	GM_DECLARE_PROPERTY(EmitSpeed, emitSpeed)
	GM_DECLARE_PROPERTY(EmitSpeedV, emitSpeedV)
	GM_DECLARE_PROPERTY(ParticleCount, particleCount)
	GM_DECLARE_PROPERTY(EmitRate, emitRate)
	GM_DECLARE_PROPERTY(Duration, duration)
	GM_DECLARE_PROPERTY(RotationAxis, rotationAxis)

public:
	GMParticleEmitter_Cocos2D(GMParticleSystem_Cocos2D* system);
	~GMParticleEmitter_Cocos2D();

public:
	virtual void emitParticles(GMDuration dt) override;
	virtual void emitOnce() override;
	virtual void update(GMDuration dt) override;
	virtual void startEmit() override;
	virtual void stopEmit() override;

public:
	void addParticle();
	void setDescription(GMParticleDescription desc);
	void setParticleEffect(GMParticleEffect_Cocos2D* effect);

	inline GMParticleSystem_Cocos2D* getParticleSystem() GM_NOEXCEPT
	{
		D(d);
		return d->system;
	}

	inline GMParticleEffect_Cocos2D* getEffect() GM_NOEXCEPT
	{
		D(d);
		return d->effect;
	}

	inline Vector<GMParticle_Cocos2D>& getParticles() GM_NOEXCEPT
	{
		D(d);
		return d->particles;
	}
};

GM_PRIVATE_OBJECT(GMParticleSystem_Cocos2D)
{
	const IRenderContext* context = nullptr;
	GMOwnedPtr<GMParticleEmitter_Cocos2D> emitter;
	IParticleSystemManager* manager = nullptr;
	GMTextureAsset texture;
	GMBuffer textureBuffer;
	GMParticleModel_Cocos2D* particleModel = nullptr;
};

class GM_EXPORT GMParticleSystem_Cocos2D : public GMObject, public IParticleSystem
{
	GM_DECLARE_PRIVATE(GMParticleSystem_Cocos2D)
	GM_DECLARE_PROPERTY(Texture, texture)

public:
	GMParticleSystem_Cocos2D(const IRenderContext* context);
	~GMParticleSystem_Cocos2D();

public:
	void setDescription(const GMParticleDescription_Cocos2D& desc);
	GMParticleModel_Cocos2D* createParticleModel(const GMParticleDescription_Cocos2D& desc);

public:
	virtual void update(GMDuration dt) override;
	virtual void render() override;
	virtual const IRenderContext* getContext() override;
	virtual void setParticleSystemManager(IParticleSystemManager* manager);
	virtual GMParticleEmitter_Cocos2D* getEmitter() override;

public:
	inline IParticleSystemManager* getParticleSystemManager() GM_NOEXCEPT
	{
		D(d);
		return d->manager;
	}

	inline const GMBuffer& getTextureBuffer() GM_NOEXCEPT
	{
		D(d);
		return d->textureBuffer;
	}

	void setParticleModel(AUTORELEASE GMParticleModel_Cocos2D* particleModel);

private:
	void updateData(void* dataPtr);

public:
	static GMParticleDescription_Cocos2D createParticleDescriptionFromCocos2DPlist(const GMString& content, GMParticleModelType modelType);

	static void createCocos2DParticleSystem(
		const IRenderContext* context,
		const GMString& filename,
		GMParticleModelType modelType,
		OUT GMParticleSystem_Cocos2D** particleSystem,
		std::function<void(GMParticleDescription_Cocos2D&)> descriptionCallback = std::function<void(GMParticleDescription_Cocos2D&)>()
	);

	static void createCocos2DParticleSystem(
		const IRenderContext* context,
		const GMBuffer& buffer,
		GMParticleModelType modelType,
		OUT GMParticleSystem_Cocos2D** particleSystem,
		std::function<void(GMParticleDescription_Cocos2D&)> descriptionCallback = std::function<void(GMParticleDescription_Cocos2D&)>()
	);
};

END_NS
#endif