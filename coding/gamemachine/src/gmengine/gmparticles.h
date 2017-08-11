#ifndef __GMPARTICLEGAMEOBJECT_H__
#define __GMPARTICLEGAMEOBJECT_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmgameobject.h"
#include "gmlua/gmlua.h"
BEGIN_NS

class GMParticles;
GM_PRIVATE_OBJECT(GMParticleGameObject)
{
	linear_math::Vector4 color = linear_math::Vector4(1);
	linear_math::Matrix4x4 transform = linear_math::Matrix4x4::identity();
	GMfloat currentLife = 0;
	GMfloat maxLife = 0;
	GMParticles* parentParticles = nullptr;
	GMint index = 0;
	GMfloat startTick = 0;
	GMfloat currentTick = 0;
};

class GMParticleGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMParticleGameObject)

public:
	GMParticleGameObject(GMModel* prototype);

public:
	GM_BEGIN_META_MAP
		GM_META(color, GMMetaMemberType::Vector4)
		GM_META(currentLife, GMMetaMemberType::Float)
		GM_META(maxLife, GMMetaMemberType::Float)
	GM_END_META_MAP

	GM_DECLARE_PROPERTY(CurrentLife, currentLife, GMfloat);
	GM_DECLARE_PROPERTY(MaxLife, maxLife, GMfloat);
	GM_DECLARE_PROPERTY(Color, color, linear_math::Vector4);
	GM_DECLARE_PROPERTY(Transform, transform, linear_math::Matrix4x4);

	inline GMModel* getPrototype() { D_BASE(d, GMGameObject); return d->model; }
	inline void setParent(GMParticles* parent) { D(d); d->parentParticles = parent; }
	inline void setIndex(GMint idx) { D(d); d->index = idx; }
	inline GMint getIndexInPrototype() { D(d); return d->index; }
	inline GMfloat getElapsedTime() { D(d); return d->currentTick - d->startTick; }

public:
	void updatePrototype(void* buffer);
};

GM_INTERFACE(IParticleHandler)
{
	virtual GMParticleGameObject* createParticle(const GMint index) = 0;
	virtual void update(const GMint index, GMParticleGameObject* particle) = 0;
	virtual void respawn(const GMint index, GMParticleGameObject* particle) = 0;
};

GM_PRIVATE_OBJECT(GMParticles)
{
	Map<GMModel*, GMfloat*> basePositions;
	Map<GMModel*, Vector<GMParticleGameObject*> > particles;
	Vector<GMParticleGameObject*> allParticles;
	GMuint lastUsedParticle = 0;
	GMint particlesCount = 0;
	IParticleHandler* particleHandler = nullptr;
	bool firstSimulate = true;
	GMfloat startTick = 0;
	GMfloat currentTick = 0;
};

// 粒子低层设施，用它可以来编写自定义粒子
class GMParticles : public GMGameObject
{
	DECLARE_PRIVATE(GMParticles)
public:
	GMParticles();
	~GMParticles();

public:
	inline void setParticlesCount(GMint particlesCount) { D(d); d->particlesCount = particlesCount; }
	inline void setParticlesHandler(IParticleHandler* handler) { D(d); d->particleHandler = handler; }
	inline GMfloat* getPositionArray(GMModel* prototype) { D(d); return d->basePositions[prototype]; }
	GMfloat getElapsedTime() { D(d); return d->currentTick - d->startTick; }

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Particles; }
	virtual void simulate() override;
	virtual void draw() override;
	virtual void onAppendingObjectToWorld() override;

public:
	inline GMParticleGameObject* getParticle(GMint index);
	GMint getParticleCount(GMModel* prototype);

private:
	void initPrototype(GMModel* prototype, const Vector<GMParticleGameObject*>& particles);
	void addParticle(AUTORELEASE GMModel* prototype, AUTORELEASE GMParticleGameObject* particle);
	bool containsPrototype(GMModel* prototype);
};

enum class GMParticlePositionType
{
	// 固定粒子位置
	Static,

	// 粒子位置随发射器移动
	FollowEmitter,

	// 粒子在更新时，在发射器位置重生
	RespawnAtEmitterPosition,
};

GM_ALIGNED_STRUCT(GMParticleEmitterProperties)
{
	GMint particleCount = 2;
	GMParticlePositionType positionType = GMParticlePositionType::Static;
	linear_math::Vector3 position = 0;
	GMfloat emissionRate = .5f;
	GMfloat speed = .5f;
	GMint emissionTimes = 0; // 0(GMParticlesEmitter::InfiniteEmitTimes)表示无限次发射
	GMint emissionFinishedParticleCount = 0;
};

GM_ALIGNED_STRUCT(GMParticleProperties)
{
	GMfloat life = 1;
	linear_math::Vector3 startupPosition = 0; // 在Free模式下表示粒子的起始位置
	linear_math::Vector3 direction = { 1, 0, 0 };
	linear_math::Quaternion startAngle;
	linear_math::Quaternion endAngle;
	linear_math::Vector4 startColor = 1.f;
	linear_math::Vector4 endColor = 1.f;
	GMfloat startSize = .1f;
	GMfloat endSize = .1f;
	GMint currentEmissionTimes = 0;
	bool visible = true;
	bool emitted = false;
	bool emitCountdown = false;
};

GM_PRIVATE_OBJECT(GMParticlesEmitter)
{
	GMParticleEmitterProperties emitterProps;
	GMParticleProperties* particleProps = nullptr;
};

// 抽象的粒子发射器
class GMParticlesEmitter : public GMParticles, public IParticleHandler
{
	DECLARE_PRIVATE(GMParticlesEmitter)

public:
	enum
	{
		InfiniteEmissionTimes = 0,
	};

public:
	~GMParticlesEmitter();

public:
	inline void setEmitterProperties(const GMParticleEmitterProperties& props) { D(d); d->emitterProps = props; }
	inline void setParticlesProperties(AUTORELEASE GMParticleProperties* props) { D(d); d->particleProps = props; }
	GMParticleEmitterProperties& getEmitterPropertiesReference() { D(d); return d->emitterProps; }

public:
	bool isEmissionFinished();

private:
	using GMParticles::setParticlesCount;
	using GMParticles::setParticlesHandler;

public:
	virtual void onAppendingObjectToWorld() override;
};

GM_PRIVATE_OBJECT(GMLerpParticleEmitter)
{
	GMModel* prototype = nullptr;
};

// 具体化的粒子发射器，提供粒子的线性变换
class GMLerpParticleEmitter : public GMParticlesEmitter
{
	DECLARE_PRIVATE(GMLerpParticleEmitter)

protected:
	GMLerpParticleEmitter() = default;

public:
	~GMLerpParticleEmitter();

public:
	virtual void onAppendingObjectToWorld() override;

protected:
	// IParticleHandler
	virtual GMParticleGameObject* createParticle(const GMint index) override;
	virtual void update(const GMint index, GMParticleGameObject* particle) override;
	virtual void respawn(const GMint index, GMParticleGameObject* particle) override;

protected:
	void checkEmit(const GMint index);
	void reduceLife(GMParticleGameObject* particle);
	void respawnLife(const GMint index, GMParticleGameObject* particle);

public:
	static void create(
		GMint count,
		GMParticlePositionType positionType,
		GMfloat life,
		GMfloat startSize,
		GMfloat endSize,
		const linear_math::Vector3& emitterPosition,
		const linear_math::Vector3& startDirectionRange,
		const linear_math::Vector3& endDirectionRange,
		const linear_math::Vector4& startColor,
		const linear_math::Vector4& endColor,
		const linear_math::Quaternion& startAngle,
		const linear_math::Quaternion& endAngle,
		GMfloat emissionRate,
		GMfloat speed,
		GMint emissionTimes,
		OUT GMParticlesEmitter** emitter
	);
};

// 有恒定角速度的半径发射器
GM_PRIVATE_OBJECT(GMRadiusParticlesEmitter)
{
	linear_math::Vector3 rotateAxis;
	GMfloat angularVelocity = 0;
	GMfloat currentAngle = 0;
	GMfloat* currentAngles = nullptr;
};

class GMRadiusParticlesEmitter : public GMLerpParticleEmitter
{
	DECLARE_PRIVATE(GMRadiusParticlesEmitter)

public:
	~GMRadiusParticlesEmitter();

public:
	virtual void update(const GMint index, GMParticleGameObject* particle) override;
	virtual void respawn(const GMint index, GMParticleGameObject* particle) override;

private:
	inline void setAngularVelocity(GMfloat angularVelocity) { D(d); d->angularVelocity = angularVelocity; }
	inline void setRotateAxis(const linear_math::Vector3& axis) { D(d); d->rotateAxis = axis; }

protected:
	GMRadiusParticlesEmitter() = default;

public:
	static void create(
		GMint count,
		GMParticlePositionType positionType,
		GMfloat life,
		GMfloat startSize,
		GMfloat endSize,
		const linear_math::Vector3& rotateAxis,
		GMfloat angularVelocity,
		const linear_math::Vector3& emitterPosition,
		const linear_math::Vector3& direction,
		const linear_math::Vector4& startColor,
		const linear_math::Vector4& endColor,
		const linear_math::Quaternion& startAngle,
		const linear_math::Quaternion& endAngle,
		GMfloat emissionRate,
		GMfloat speed,
		GMint emissionTimes,
		OUT GMParticlesEmitter** emitter
	);
};

GM_PRIVATE_OBJECT(GMCustomParticlesEmitter)
{
	bool loaded = false;
	GMModel* prototype = nullptr;
	GMLua lua;
};

// 一个读取LUA脚本的粒子发射器
class GMCustomParticlesEmitter : public GMParticles, public IParticleHandler
{
	DECLARE_PRIVATE(GMCustomParticlesEmitter)

public:
	GMCustomParticlesEmitter(AUTORELEASE GMModel* model);
	~GMCustomParticlesEmitter();

public:
	inline GMLua& getScript() { D(d); return d->lua; }

public:
	void load(const GMBuffer& script);
	void onAppendingObjectToWorld() override;

private:
	virtual GMParticleGameObject* createParticle(const GMint index) override;
	virtual void update(const GMint index, GMParticleGameObject* particle) override;
	virtual void respawn(const GMint index, GMParticleGameObject* particle) override;
};

END_NS
#endif