#ifndef __GMPARTICLEGAMEOBJECT_H__
#define __GMPARTICLEGAMEOBJECT_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmgameobject.h"
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
};

class GMParticleGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMParticleGameObject)

public:
	GMParticleGameObject(GMModel* prototype);

public:
	inline GMfloat getCurrentLife() { D(d); return d->currentLife; }
	inline void setCurrentLife(GMfloat life) { D(d); d->currentLife = life; }
	inline GMfloat getMaxLife() { D(d); return d->maxLife; }
	inline void setMaxLife(GMfloat life) { D(d); d->maxLife = life; }
	inline GMModel* getPrototype() { D_BASE(d, GMGameObject); return d->model; }
	inline void setParent(GMParticles* parent) { D(d); d->parentParticles = parent; }
	inline void setIndex(GMint idx) { D(d); d->index = idx; }
	inline GMint getIndexInPrototype() { D(d); return d->index; }
	inline linear_math::Vector4& getColor() { D(d); return d->color; }
	inline const linear_math::Vector4& getColor() const { D(d); return d->color; }
	inline void setColor(linear_math::Vector4& color) { D(d); d->color = color; }
	inline linear_math::Matrix4x4& getTransform() { D(d); return d->transform; }
	inline const linear_math::Matrix4x4& getTransform() const { D(d); return d->transform; }
	inline void setTransform(const linear_math::Matrix4x4& transform) { D(d); d->transform = transform; }

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
};

// 粒子低层设施，用它可以来编写自定义粒子
class GMParticles : public GMGameObject
{
	DECLARE_PRIVATE(GMParticles)
public:
	GMParticles();
	~GMParticles();

public:
	inline GMfloat* getPositionArray(GMModel* prototype) { D(d); return d->basePositions[prototype]; }
	inline void setParticlesCount(GMint particlesCount) { D(d); d->particlesCount = particlesCount; }
	inline void setParticlesHandler(IParticleHandler* handler) { D(d); d->particleHandler = handler; }

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
	Free,
};

GM_ALIGNED_STRUCT(GMParticleEmitterProperties)
{
	GMint particleCount = 2;
	GMParticlePositionType positionType = GMParticlePositionType::Free;
	linear_math::Vector3 position = 0;
	GMfloat emissionRate = .5f;
	GMfloat speed = .5f;
};

GM_ALIGNED_STRUCT(GMParticleProperties)
{
	GMfloat life = 1;
	linear_math::Quaternion angle;
	linear_math::Vector4 startColor = 1.f;
	linear_math::Vector4 endColor = 1.f;
	GMfloat startSize = .1f;
	GMfloat endSize = .1f;
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
	~GMParticlesEmitter();

public:
	void setEmitterProperties(const GMParticleEmitterProperties& props);
	void setParticlesProperties(AUTORELEASE GMParticleProperties* props);

private:
	using GMParticles::setParticlesCount;
	using GMParticles::setParticlesHandler;

public:
	virtual void onAppendingObjectToWorld() override;
};

GM_PRIVATE_OBJECT(GMDefaultParticleEmitter)
{
	GMModel* prototype = nullptr;
};

// 具体化的粒子发射器，提供基本的粒子变换
class GMDefaultParticleEmitter : public GMParticlesEmitter
{
	DECLARE_PRIVATE(GMDefaultParticleEmitter)

public:
	~GMDefaultParticleEmitter();

public:
	virtual void onAppendingObjectToWorld() override;

public:
	// IParticleHandler
	virtual GMParticleGameObject* createParticle(const GMint index) override;
	virtual void update(const GMint index, GMParticleGameObject* particle) override;
	virtual void respawn(const GMint index, GMParticleGameObject* particle) override;
};

// 内置一些现成的粒子发射器
class GMEjectionParticleEmitter : public GMParticlesEmitter
{
public:
	static void create(
		GMint count,
		GMfloat startSize,
		GMfloat endSize,
		const linear_math::Vector4& startColor,
		const linear_math::Vector4& endColor,
		const linear_math::Quaternion& startAngleRange,
		const linear_math::Quaternion& endAngleRange,
		GMfloat emissionRate,
		GMfloat speed,
		OUT GMParticles** emitter
	);
};

END_NS
#endif