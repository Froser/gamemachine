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

struct GMParticle_Cocos2D_GravityModeData
{
	GMVec3 initialVelocity;
	GMfloat radialAcceleration;
	GMfloat tangentialAcceleration;
};

struct GMParticle_Cocos2D_RadiusModeData
{
	GMfloat angle;
	GMfloat degressPerSecond;
	GMfloat radius;
	GMfloat deltaRadius;
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

GM_PRIVATE_CLASS(GMParticleGravityMode);
class GMParticleGravityMode
{
	GM_DECLARE_PRIVATE(GMParticleGravityMode)
	GM_DECLARE_PROPERTY(GMVec3, Gravity)
	GM_DECLARE_PROPERTY(GMfloat, TangentialAcceleration)
	GM_DECLARE_PROPERTY(GMfloat, TangentialAccelerationV)
	GM_DECLARE_PROPERTY(GMfloat, RadialAcceleration)
	GM_DECLARE_PROPERTY(GMfloat, RadialAccelerationV)

public:
	GMParticleGravityMode();
	GMParticleGravityMode(const GMParticleGravityMode&);
	GMParticleGravityMode(GMParticleGravityMode&&) GM_NOEXCEPT;
	GMParticleGravityMode& operator=(const GMParticleGravityMode& rhs);
	GMParticleGravityMode& operator=(GMParticleGravityMode&& rhs) GM_NOEXCEPT;
	~GMParticleGravityMode();
};

GM_PRIVATE_CLASS(GMParticleRadiusMode);
class GMParticleRadiusMode
{
	GM_DECLARE_PRIVATE(GMParticleRadiusMode)
	GM_DECLARE_PROPERTY(GMfloat, BeginRadius)
	GM_DECLARE_PROPERTY(GMfloat, BeginRadiusV)
	GM_DECLARE_PROPERTY(GMfloat, EndRadius)
	GM_DECLARE_PROPERTY(GMfloat, EndRadiusV)
	GM_DECLARE_PROPERTY(GMfloat, SpinPerSecond)
	GM_DECLARE_PROPERTY(GMfloat, SpinPerSecondV)

public:
	GMParticleRadiusMode();
	GMParticleRadiusMode(const GMParticleRadiusMode&);
	GMParticleRadiusMode(GMParticleRadiusMode&&) GM_NOEXCEPT;
	GMParticleRadiusMode& operator=(const GMParticleRadiusMode& rhs);
	GMParticleRadiusMode& operator=(GMParticleRadiusMode&& rhs) GM_NOEXCEPT;
	~GMParticleRadiusMode();
};

GM_PRIVATE_CLASS(GMParticleDescription_Cocos2D);
//! 表示一个粒子发射器的描述
/*!
  对于一个粒子发射器而言，它的重力方向为GameMachine坐标系y轴负方向，朝向地面。<BR>
  发射的0度角对应GameMachine坐标系(0, 1, 0)。
  所有Angle的单位为角度，而非弧度。
*/
class GM_EXPORT GMParticleDescription_Cocos2D
{
	GM_DECLARE_PRIVATE(GMParticleDescription_Cocos2D)

	GM_DECLARE_PROPERTY(GMVec3, EmitterPosition)
	GM_DECLARE_PROPERTY(GMVec3, EmitterPositionV)
	GM_DECLARE_PROPERTY(GMfloat, EmitterEmitAngle)
	GM_DECLARE_PROPERTY(GMfloat, EmitterEmitAngleV)
	GM_DECLARE_PROPERTY(GMfloat, EmitterEmitSpeed)
	GM_DECLARE_PROPERTY(GMfloat, EmitterEmitSpeedV)
	GM_DECLARE_PROPERTY(GMint32, ParticleCount)
	GM_DECLARE_PROPERTY(GMDuration, EmitRate)
	GM_DECLARE_PROPERTY(GMDuration, Duration)
	GM_DECLARE_PROPERTY(GMParticleEmitterType, EmitterType)
	GM_DECLARE_PROPERTY(GMParticleMotionMode, MotionMode)
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
	GM_DECLARE_PROPERTY(GMParticleGravityMode, GravityMode)
	GM_DECLARE_PROPERTY(GMParticleRadiusMode, RadiusMode)
	GM_DECLARE_PROPERTY(GMBuffer, TextureImageData)
	GM_DECLARE_PROPERTY(GMParticleModelType, ParticleModelType)

public:
	GMParticleDescription_Cocos2D();
	GMParticleDescription_Cocos2D(const GMParticleDescription_Cocos2D&);
	GMParticleDescription_Cocos2D(GMParticleDescription_Cocos2D&&) GM_NOEXCEPT;
	GMParticleDescription_Cocos2D& operator=(const GMParticleDescription_Cocos2D& rhs);
	GMParticleDescription_Cocos2D& operator=(GMParticleDescription_Cocos2D&& rhs) GM_NOEXCEPT;
	~GMParticleDescription_Cocos2D();
};

GM_PRIVATE_CLASS(GMParticle_Cocos2D);
class GMParticle_Cocos2D
{
	GM_DECLARE_PRIVATE(GMParticle_Cocos2D)
	GM_DECLARE_PROPERTY(GMVec3, Position)
	GM_DECLARE_PROPERTY(GMVec3, StartPosition)
	GM_DECLARE_PROPERTY(GMVec3, ChangePosition)
	GM_DECLARE_PROPERTY(GMVec3, Velocity)
	GM_DECLARE_PROPERTY(GMVec4, Color)
	GM_DECLARE_PROPERTY(GMVec4, DeltaColor)
	GM_DECLARE_PROPERTY(GMfloat, Size)
	GM_DECLARE_PROPERTY(GMfloat, CurrentSize)
	GM_DECLARE_PROPERTY(GMfloat, DeltaSize)
	GM_DECLARE_PROPERTY(GMfloat, Rotation)
	GM_DECLARE_PROPERTY(GMfloat, DeltaRotation)
	GM_DECLARE_PROPERTY(GMDuration, RemainingLife)
	GM_DECLARE_PROPERTY(GMParticle_Cocos2D_GravityModeData, GravityModeData)
	GM_DECLARE_PROPERTY(GMParticle_Cocos2D_RadiusModeData, RadiusModeData)

public:
	GMParticle_Cocos2D();
	GMParticle_Cocos2D(const GMParticle_Cocos2D&);
	GMParticle_Cocos2D(GMParticle_Cocos2D&&) GM_NOEXCEPT;
	GMParticle_Cocos2D& operator=(const GMParticle_Cocos2D& rhs);
	GMParticle_Cocos2D& operator=(GMParticle_Cocos2D&& rhs) GM_NOEXCEPT;
	~GMParticle_Cocos2D();
	const Data& dataRef();
};

GM_PRIVATE_CLASS(GMParticleEmitter_Cocos2D);
class GM_EXPORT GMParticleEmitter_Cocos2D : public IParticleEmitter
{
	GM_DECLARE_PRIVATE(GMParticleEmitter_Cocos2D)
	GM_DECLARE_PROPERTY(GMVec3, EmitPosition)
	GM_DECLARE_PROPERTY(GMVec3, EmitPositionV)
	GM_DECLARE_PROPERTY(GMfloat, EmitAngle)
	GM_DECLARE_PROPERTY(GMfloat, EmitAngleV)
	GM_DECLARE_PROPERTY(GMfloat, EmitSpeed)
	GM_DECLARE_PROPERTY(GMfloat, EmitSpeedV)
	GM_DECLARE_PROPERTY(GMint32, ParticleCount)
	GM_DECLARE_PROPERTY(GMDuration, EmitRate)
	GM_DECLARE_PROPERTY(GMDuration, Duration)
	GM_DECLARE_PROPERTY(GMVec3, RotationAxis)

public:
	GMParticleEmitter_Cocos2D(GMParticleSystem_Cocos2D* system);
	~GMParticleEmitter_Cocos2D();
	GMParticleEmitter_Cocos2D(const GMParticleEmitter_Cocos2D&);
	GMParticleEmitter_Cocos2D(GMParticleEmitter_Cocos2D&&) GM_NOEXCEPT;
	GMParticleEmitter_Cocos2D& operator=(const GMParticleEmitter_Cocos2D& rhs);
	GMParticleEmitter_Cocos2D& operator=(GMParticleEmitter_Cocos2D&& rhs) GM_NOEXCEPT;

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
	GMParticleSystem_Cocos2D* getParticleSystem() GM_NOEXCEPT;
	GMParticleEffect_Cocos2D* getEffect() GM_NOEXCEPT;
	Vector<GMParticle_Cocos2D>& getParticles() GM_NOEXCEPT;
};

GM_PRIVATE_CLASS(GMParticleSystem_Cocos2D);
class GM_EXPORT GMParticleSystem_Cocos2D : public IParticleSystem
{
	GM_DECLARE_PRIVATE(GMParticleSystem_Cocos2D)
	GM_DISABLE_COPY_ASSIGN(GMParticleSystem_Cocos2D);
	GM_DECLARE_PROPERTY(GMTextureAsset, Texture)

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
	IParticleSystemManager* getParticleSystemManager() GM_NOEXCEPT;
	const GMBuffer& getTextureBuffer() GM_NOEXCEPT;
	void setParticleModel(AUTORELEASE GMParticleModel_Cocos2D* particleModel);

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