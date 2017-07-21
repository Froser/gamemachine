#ifndef __GMPARTICLEGAMEOBJECT_H__
#define __GMPARTICLEGAMEOBJECT_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "gmgameobject.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMParticleGameObject)
{
	Object* prototype = nullptr;
	linear_math::Vector2 position = linear_math::Vector2(0, 0);
	linear_math::Vector2 velocity = linear_math::Vector2(0, 0);
	linear_math::Vector4 color = linear_math::Vector4(0, 0);
	GMfloat life = 0;
};

class GMParticleGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMParticleGameObject)

public:
	GMParticleGameObject(Object* prototype);

public:
	GMfloat life() { D(d); return d->life; }
};

GM_INTERFACE(IParticleHandler)
{
	virtual void update(GMParticleGameObject* particle) = 0;
};

GM_PRIVATE_OBJECT(GMParticles)
{
	Object* prototype;
	Vector<GMParticleGameObject*> particles;
	GMuint lastUsedParticle = 0;
	IParticleHandler* particleHandler = nullptr;
};

class GMParticles : public GMGameObject
{
	DECLARE_PRIVATE(GMParticles)

public:
	GMParticles(AUTORELEASE Object* prototype, GMint particlesCount, IParticleHandler* handler);
	~GMParticles();

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Particles; }
	virtual void simulate() override;
	virtual void onAppendingObjectToWorld() override;
	virtual void draw() override;

public:
	inline GMParticleGameObject* getParticle(GMint index);
	GMint findFirstUnusedParticle();
};

END_NS
#endif