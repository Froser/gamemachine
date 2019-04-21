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
	GM_DECLARE_PROPERTY(Gravity, gravity)
	GM_DECLARE_PROPERTY(TangentialAcceleration, tangentialAcceleration)
	GM_DECLARE_PROPERTY(TangentialAccelerationV, tangentialAccelerationV)
	GM_DECLARE_PROPERTY(RadialAcceleration, radialAcceleration)
	GM_DECLARE_PROPERTY(RadialAccelerationV, radialAccelerationV)

public:
	GMParticleGravityMode() = default;
};

class GMParticleRadiusMode : public GMObject
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
class GMParticleDescription : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleDescription)
	GM_ALLOW_COPY_MOVE(GMParticleDescription)

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
	GMint32 particleCount = 0;
	GMDuration emitRate = 0;
	GMDuration duration = 0;
	GMVec3 rotationAxis = GMVec3(0, 0, 1);
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
};

class GMParticleEffect : public GMObject
{
	GM_DECLARE_PRIVATE(GMParticleEffect)
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
	GM_DECLARE_PROPERTY(Texture, texture)

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
	void updateData(const IRenderContext* context, void* dataPtr);

private:
	inline void setParticleSystemManager(GMParticleSystemManager* manager) GM_NOEXCEPT
	{
		D(d);
		d->manager = manager;
	}

public:
	static GMParticleDescription createParticleDescriptionFromCocos2DPlist(const GMString& content, GMParticleModelType modelType);

	static void createCocos2DParticleSystem(
		const GMString& filename,
		GMParticleModelType modelType,
		OUT GMParticleSystem** particleSystem,
		std::function<void(GMParticleDescription&)> descriptionCallback = std::function<void(GMParticleDescription&)>()
	);
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

class GM_EXPORT GMParticleSystemManager : public GMObject
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