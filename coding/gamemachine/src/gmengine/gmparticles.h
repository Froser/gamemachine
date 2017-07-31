#ifndef __GMPARTICLEGAMEOBJECT_H__
#define __GMPARTICLEGAMEOBJECT_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmgameobject.h"
BEGIN_NS

class GMParticles;
GM_PRIVATE_OBJECT(GMParticleGameObject)
{
	linear_math::Vector4 color = linear_math::Vector4(0, 0, 0, 0);
	linear_math::Matrix4x4 transform = linear_math::Matrix4x4::identity();
	GMfloat currentLife = 0;
	GMfloat maxLife = 0;
	GMParticles* parentParticles = nullptr;
	GMint index;
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
	inline linear_math::Matrix4x4& getTransform() { D(d); return d->transform; }
	inline const linear_math::Matrix4x4& getTransform() const { D(d); return d->transform; }

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

class GMParticles : public GMGameObject
{
	DECLARE_PRIVATE(GMParticles)
public:
	GMParticles(GMint particlesCount, IParticleHandler* handler);
	~GMParticles();

public:
	inline GMfloat* getPositionArray(GMModel* prototype) { D(d); return d->basePositions[prototype]; }

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Particles; }
	virtual void simulate() override;
	virtual void draw() override;

public:
	inline GMParticleGameObject* getParticle(GMint index);
	GMint getParticleCount(GMModel* prototype);

private:
	void initPrototype(GMModel* prototype, const Vector<GMParticleGameObject*>& particles);
	void addParticle(AUTORELEASE GMModel* prototype, AUTORELEASE GMParticleGameObject* particle);
	bool containsPrototype(GMModel* prototype);
};

END_NS
#endif