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
	GMfloat life = 0;
	GMParticles* parentParticles = nullptr;
	GMint index;
};

class GMParticleGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMParticleGameObject)

public:
	GMParticleGameObject(Object* prototype);

public:
	inline GMfloat life() { D(d); return d->life; }
	inline Object* getPrototype() { D_BASE(d, GMGameObject); return d->object; }
	inline void setParent(GMParticles* parent) { D(d); d->parentParticles = parent; }
	inline void setIndex(GMint idx) { D(d); d->index = idx; }
	inline GMint getIndexInPrototype() { D(d); return d->index; }
	inline linear_math::Vector4& color() { D(d); return d->color; }
	inline const linear_math::Vector4& color() const { D(d); return d->color; }
	inline linear_math::Matrix4x4& transform() { D(d); return d->transform; }
	inline const linear_math::Matrix4x4& transform() const { D(d); return d->transform; }

public:
	void updatePrototype(void* buffer);
};

GM_INTERFACE(IParticleHandler)
{
	virtual GMParticleGameObject* createParticle(const GMint index) = 0;
	virtual void update(const GMint index, GMParticleGameObject* particle) = 0;
};

GM_PRIVATE_OBJECT(GMParticles)
{
	Map<Object*, GMfloat*> basePositions;
	Map<Object*, Vector<GMParticleGameObject*> > particles;
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
	inline GMfloat* getPositionArray(Object* prototype) { D(d); return d->basePositions[prototype]; }

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Particles; }
	virtual void simulate() override;
	virtual void draw() override;

public:
	inline GMParticleGameObject* getParticle(GMint index);
	GMint findFirstUnusedParticle();
	GMint getParticleCount(Object* prototype);

private:
	void initPrototype(Object* prototype, const Vector<GMParticleGameObject*>& particles);
	void addParticle(AUTORELEASE Object* prototype, AUTORELEASE GMParticleGameObject* particle);
	bool containsPrototype(Object* prototype);
};

END_NS
#endif