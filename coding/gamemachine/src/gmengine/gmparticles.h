#ifndef __GMPARTICLEGAMEOBJECT_H__
#define __GMPARTICLEGAMEOBJECT_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmgameobject.h"
BEGIN_NS

class GMParticles;
GM_PRIVATE_OBJECT(GMParticleGameObject)
{
	linear_math::Vector2 position = linear_math::Vector2(0, 0);
	linear_math::Vector2 velocity = linear_math::Vector2(0, 0);
	linear_math::Vector4 color = linear_math::Vector4(0, 0);
	GMfloat life = 0;
	GMParticles* parentParticles = nullptr;
};

class GMParticleGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMParticleGameObject)

public:
	GMParticleGameObject(Object* prototype);

public:
	inline GMfloat life() { D(d); return d->life; }
	inline Object* getPrototype() { D_BASE(d, GMGameObject); return d->object; }
};

GM_INTERFACE(IParticleHandler)
{
	virtual GMParticleGameObject* createParticle(const GMint index) = 0;
	virtual void update(GMParticleGameObject* particle) = 0;
};

GM_PRIVATE_OBJECT(GMParticles)
{
	Map<Object*, Set<GMParticleGameObject*> > particles;
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
	void addParticle(AUTORELEASE Object* prototype, AUTORELEASE GMParticleGameObject* particle);
	bool containsPrototype(Object* prototype);
};

END_NS
#endif