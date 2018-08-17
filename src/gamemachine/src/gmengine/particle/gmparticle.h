#ifndef __GMPARTICLE_H__
#define __GMPARTICLE_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

class GMParticleSystem;
class GMParticleSystemManager;
class GMParticleEmitter;
class GMParticle;

GM_INTERFACE(IParticleModel)
{
	virtual void render(const IRenderContext* context) = 0;
};

enum class GMParticleModelType
{
	Particle2D,
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

GM_PRIVATE_OBJECT(GMParticleGravityMode)
{
	GMVec3 gravity = Zero<GMVec3>();
	GMfloat tangentialAcceleration = 0;
	GMfloat tangentialAccelerationV = 0;
	GMfloat radialAcceleration = 0;
	GMfloat radialAccelerationV = 0;
};

GM_PRIVATE_OBJECT(GMParticleRadiusMode)
{
	GMfloat beginRadius = 0;
	GMfloat beginRadiusV = 0;
	GMfloat endRadius = 0;
	GMfloat endRadiusV = 0;
	GMfloat spinPerSecond = 0;
	GMfloat spinPerSecondV = 0;
};

class GMParticleGravityMode : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleGravityMode)
	GM_ALLOW_COPY_MOVE(GMParticleGravityMode)
	GM_DECLARE_PROPERTY(Gravity, gravity, GMVec3)
	GM_DECLARE_PROPERTY(TangentialAcceleration, tangentialAcceleration, GMfloat)
	GM_DECLARE_PROPERTY(TangentialAccelerationV, tangentialAccelerationV, GMfloat)
	GM_DECLARE_PROPERTY(RadialAcceleration, radialAcceleration, GMfloat)
	GM_DECLARE_PROPERTY(RadialAccelerationV, radialAccelerationV, GMfloat)

public:
	GMParticleGravityMode() = default;
};

class GMParticleRadiusMode : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleRadiusMode)
	GM_ALLOW_COPY_MOVE(GMParticleRadiusMode)
	GM_DECLARE_PROPERTY(BeginRadius, beginRadius, GMfloat)
	GM_DECLARE_PROPERTY(BeginRadiusV, beginRadiusV, GMfloat)
	GM_DECLARE_PROPERTY(EndRadius, endRadius, GMfloat)
	GM_DECLARE_PROPERTY(EndRadiusV, endRadiusV, GMfloat)
	GM_DECLARE_PROPERTY(SpinPerSecond, spinPerSecond, GMfloat)
	GM_DECLARE_PROPERTY(SpinPerSecondV, spinPerSecondV, GMfloat)

public:
	GMParticleRadiusMode() = default;
};

GM_PRIVATE_OBJECT(GMParticleDescription)
{
	GMVec3 emitterPosition = Zero<GMVec3>(); //<! 发射器位置
	GMVec3 emitterPositionV = Zero<GMVec3>();
	GMfloat emitterEmitAngle = 0; //!< 发射器发射角度
	GMfloat emitterEmitAngleV = 0;
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

	GMBuffer textureImageData;
	GMParticleModelType particleModelType = GMParticleModelType::Particle2D;
};

class GMParticleDescription : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleDescription)
	GM_ALLOW_COPY_MOVE(GMParticleDescription)

	GM_DECLARE_PROPERTY(EmitterPosition, emitterPosition, GMVec3)
	GM_DECLARE_PROPERTY(EmitterPositionV, emitterPositionV, GMVec3)
	GM_DECLARE_PROPERTY(EmitterEmitAngle, emitterEmitAngle, GMfloat)
	GM_DECLARE_PROPERTY(EmitterEmitAngleV, emitterEmitAngleV, GMfloat)
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
	GM_DECLARE_PROPERTY(TextureImageData, textureImageData, GMBuffer)
	GM_DECLARE_PROPERTY(ParticleModelType, particleModelType, GMParticleModelType)

public:
	GMParticleDescription() = default;
};

GM_PRIVATE_OBJECT(GMParticle)
{
	GMVec3 position = Zero<GMVec4>();
	GMVec3 startPosition = Zero<GMVec4>();
	GMVec3 changePosition = Zero<GMVec4>();
	GMVec3 velocity = Zero<GMVec4>();
	GMVec4 color = Zero<GMVec4>();
	GMVec4 deltaColor = Zero<GMVec4>();
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
		GMfloat delatRadius;
	} radiusModeData;
};

class GMParticle : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticle)
	GM_ALLOW_COPY_MOVE(GMParticle)
	GM_DECLARE_PROPERTY(Position, position, GMVec3)
	GM_DECLARE_PROPERTY(StartPosition, startPosition, GMVec3)
	GM_DECLARE_PROPERTY(ChangePosition, changePosition, GMVec3)
	GM_DECLARE_PROPERTY(Velocity, velocity, GMVec3)
	GM_DECLARE_PROPERTY(Color, color, GMVec4)
	GM_DECLARE_PROPERTY(DeltaColor, deltaColor, GMVec4)
	GM_DECLARE_PROPERTY(Size, size, GMfloat)
	GM_DECLARE_PROPERTY(CurrentSize, currentSize, GMfloat)
	GM_DECLARE_PROPERTY(DeltaSize, deltaSize, GMfloat)
	GM_DECLARE_PROPERTY(Rotation, rotation, GMfloat)
	GM_DECLARE_PROPERTY(DeltaRotation, deltaRotation, GMfloat)
	GM_DECLARE_PROPERTY(RemainingLife, remainingLife, GMfloat)
	GM_DECLARE_PROPERTY(GravityModeData, gravityModeData, GM_PRIVATE_NAME(GMParticle)::GravityModeData)
	GM_DECLARE_PROPERTY(RadiusModeData, radiusModeData, GM_PRIVATE_NAME(GMParticle)::RadiusModeData)

public:
	GMParticle() = default;
};

class GMParticleEffect;
GM_PRIVATE_OBJECT(GMParticleEmitter)
{
	GMVec3 emitPosition = Zero<GMVec3>();
	GMVec3 emitPositionV = Zero<GMVec3>();
	GMfloat emitAngle = 0;
	GMfloat emitAngleV = 0;
	GMfloat emitSpeed = 0;
	GMfloat emitSpeedV = 0;
	GMint particleCount = 0;
	GMDuration emitRate = 0;
	GMDuration duration = 0;
	GMOwnedPtr<GMParticleEffect> effect;
	Vector<GMParticle*> particles;
	bool canEmit = true;
	GMParticleSystem* system = nullptr;
	GMDuration emitCounter = 0;
	GMDuration elapsed = 0;
};

class GMParticleEmitter : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleEmitter)
	GM_DECLARE_PROPERTY(EmitPosition, emitPosition, GMVec3)
	GM_DECLARE_PROPERTY(EmitPositionV, emitPositionV, GMVec3)
	GM_DECLARE_PROPERTY(EmitAngle, emitAngle, GMfloat)
	GM_DECLARE_PROPERTY(EmitAngleV, emitAngleV, GMfloat)
	GM_DECLARE_PROPERTY(EmitSpeed, emitSpeed, GMfloat)
	GM_DECLARE_PROPERTY(EmitSpeedV, emitSpeedV, GMfloat)
	GM_DECLARE_PROPERTY(ParticleCount, particleCount, GMint)
	GM_DECLARE_PROPERTY(EmitRate, emitRate, GMfloat)
	GM_DECLARE_PROPERTY(Duration, duration, GMfloat)

public:
	GMParticleEmitter(GMParticleSystem* system);

public:
	void setDescription(const GMParticleDescription& desc);
	void setParticleEffect(GMParticleEffect* effect);
	void addParticle();
	void emitParticles(GMDuration dt);
	void update(GMDuration dt);
	void startEmit();
	void stopEmit();

public:
	inline GMParticleEffect* getEffect() GM_NOEXCEPT
	{
		D(d);
		return d->effect.get();
	}

	inline Vector<GMParticle*>& getParticles() GM_NOEXCEPT
	{
		D(d);
		return d->particles;
	}

	inline GMParticleSystem* getParticleSystem() GM_NOEXCEPT
	{
		D(d);
		return d->system;
	}
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
	GMVec3 rotationAxis = GMVec3(0, 0, 1);
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
	GM_DECLARE_PROPERTY(RotationAxis, rotationAxis, GMVec3)

public:
	void setParticleDescription(const GMParticleDescription& desc);

public:
	virtual void initParticle(GMParticleEmitter* emitter, GMParticle* particle);
	virtual void update(GMParticleEmitter* emitter, GMDuration dt) = 0;
};

GM_PRIVATE_OBJECT(GMParticleSystem)
{
	GMOwnedPtr<GMParticleEmitter> emitter;
	GMParticleSystemManager* manager = nullptr;
	GMTextureAsset texture;
	GMBuffer textureBuffer;
	GMOwnedPtr<IParticleModel> particleModel;
};

class GMParticleSystem : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleSystem)
	GM_FRIEND_CLASS(GMParticleSystemManager)
	GM_DECLARE_PROPERTY(Texture, texture, GMTextureAsset)

public:
	GMParticleSystem();

public:
	void setDescription(const GMParticleDescription& desc);
	void update(GMDuration dt);
	void render(const IRenderContext* context);

public:
	virtual IParticleModel* createParticleModel(const GMParticleDescription& desc);

public:
	inline GMParticleEmitter* getEmitter() GM_NOEXCEPT
	{
		D(d);
		return d->emitter.get();
	}

	inline GMParticleSystemManager* getParticleSystemManager() GM_NOEXCEPT
	{
		D(d);
		return d->manager;
	}

	inline const GMBuffer& getTextureBuffer() GM_NOEXCEPT
	{
		D(d);
		return d->textureBuffer;
	}

	inline void setParticleModel(AUTORELEASE IParticleModel* particleModel) GM_NOEXCEPT
	{
		D(d);
		d->particleModel.reset(particleModel);
	}

private:
	GMGameObject* createGameObject(const IRenderContext* context);
	void updateData(const IRenderContext* context, void* dataPtr);

private:
	inline void setParticleSystemManager(GMParticleSystemManager* manager) GM_NOEXCEPT
	{
		D(d);
		d->manager = manager;
	}

public:
	static GMParticleDescription createParticleDescriptionFromCocos2DPlist(const GMString& content);
};

GM_PRIVATE_OBJECT(GMParticlePool)
{
	Vector<GMOwnedPtr<GMParticle>> particlePool;
	Vector<GMParticle*> unused;
	GMsize_t index = 0;
	GMsize_t capacity = 0;
};

class GMParticlePool : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticlePool)

public:
	void init(GMsize_t count);
	void freeAll();
	GMParticle* alloc();
	void free(GMParticle* particle);
	GMsize_t getCapacity() GM_NOEXCEPT;

private:
	void expand(GMsize_t size);
};

GM_PRIVATE_OBJECT(GMParticleSystemManager)
{
	const IRenderContext* context;
	Vector<GMOwnedPtr<GMParticleSystem>> particleSystems;
	GMParticlePool pool;
};

class GMParticleSystemManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleSystemManager)

public:
	GMParticleSystemManager(const IRenderContext* context, GMsize_t particleCountHint = 128);

public:
	void addParticleSystem(AUTORELEASE GMParticleSystem* ps);
	void render();
	void update(GMDuration dt);

public:
	inline GMParticlePool& getPool() GM_NOEXCEPT
	{
		D(d);
		return d->pool;
	}
};

END_NS
#endif