#ifndef __GMPARTICLEGAMEOBJECT_H__
#define __GMPARTICLEGAMEOBJECT_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmgameobject.h"
BEGIN_NS

class GMParticles;
GM_PRIVATE_OBJECT(GMParticleGameObject)
{
	linear_math::Vector2 uv = linear_math::Vector2(0, 0);
	linear_math::Vector4 position = linear_math::Vector4(0, 0, 0, 1);
	linear_math::Vector4 color = linear_math::Vector4(0, 0);
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
	inline void setIndex(GMint idx) { D(d); d->index = idx; }
	inline GMint getIndexInPrototype() { D(d); return d->index; }
	inline linear_math::Vector4& position() { D(d); return d->position; }
	inline const linear_math::Vector4& position() const { D(d); return d->position; }
	inline linear_math::Vector4& color() { D(d); return d->color; }
	inline const linear_math::Vector4& color() const { D(d); return d->color; }
	inline linear_math::Vector2& uv() { D(d); return d->uv; }
	inline const linear_math::Vector2& uv() const { D(d); return d->uv; }

public:
	void updatePrototype(void* buffer);
};

GM_INTERFACE(IParticleHandler)
{
	virtual GMParticleGameObject* createParticle(const GMint index) = 0;
	virtual void update(GMParticleGameObject* particle) = 0;
};

GM_PRIVATE_OBJECT(GMParticles)
{
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
	virtual GMGameObjectType getType() { return GMGameObjectType::Particles; }
	virtual void simulate() override;
	virtual void draw() override;

public:
	inline GMParticleGameObject* getParticle(GMint index);
	GMint findFirstUnusedParticle();

private:
	void initPrototype(Object* prototype, const Vector<GMParticleGameObject*>& particles);
	void addParticle(AUTORELEASE Object* prototype, AUTORELEASE GMParticleGameObject* particle);
	bool containsPrototype(Object* prototype);
	void drawInstances(Object* prototype, GMuint count);
};

END_NS
#endif