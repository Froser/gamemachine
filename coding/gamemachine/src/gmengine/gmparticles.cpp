#include "stdafx.h"
#include "gmparticles.h"
#include "foundation/gamemachine.h"
#include "foundation/gmthreads.h"

GMParticleGameObject::GMParticleGameObject(Object* prototype)
	: GMGameObject(prototype)
{
	D(d);
	setDestructor(this);
}

//////////////////////////////////////////////////////////////////////////
GMParticles::GMParticles(GMint particlesCount, IParticleHandler* handler)
	: GMGameObject(nullptr)
{
	D(d);
	D_BASE(db, GMGameObject);
	d->particleHandler = handler;
	d->particlesCount = particlesCount;

	for (GMint i = 0; i < d->particlesCount; i++)
	{
		GMParticleGameObject* particle = d->particleHandler->createParticle(i);
		ASSERT(particle);
		addPrototype(particle->getPrototype());
		particle->setWorld(db->world);
		d->particles.push_back(particle);
	}

	for (const auto& prototype : d->prototypes)
	{
		//GameMachine::instance().initObjectPainter(prototype);
#error 这里存在一个transfer的问题，我们默认transfer之后删除顶点数据了，但是对于一个共享的prototype这样做不行
	}
}

GMParticles::~GMParticles()
{
	D(d);
	for (const auto& prototype : d->prototypes)
	{
		delete prototype;
	}

	for (const auto& particle : d->particles)
	{
		delete particle;
	}
}

inline GMParticleGameObject* GMParticles::getParticle(GMint index)
{
	D(d);
	return d->particles[index];
}

void GMParticles::draw()
{
	D(d);
	for (const auto& particle : d->particles)
	{
		particle->draw();
	}
}

void GMParticles::simulate()
{
	D(d);
	for (const auto& particle : d->particles)
	{
		if (d->particleHandler)
			d->particleHandler->update(particle);
	}
}

GMint GMParticles::findFirstUnusedParticle()
{
	D(d);
	GMuint size = d->particles.size();

	for (GMuint i = d->lastUsedParticle; i < size; ++i)
	{
		if (getParticle(i)->life() <= 0.0f)
		{
			d->lastUsedParticle = i;
			return i;
		}
	}

	for (GMuint i = 0; i < d->lastUsedParticle; ++i)
	{
		if (getParticle(i)->life() <= 0.0f)
		{
			d->lastUsedParticle = i;
			return i;
		}
	}

	d->lastUsedParticle = 0;
	return 0;
}

void GMParticles::addPrototype(AUTORELEASE Object* prototype)
{
	D(d);
	d->prototypes.insert(prototype);
}

bool GMParticles::containsPrototype(Object* prototype)
{
	D(d);
	return d->prototypes.find(prototype) != d->prototypes.end();
}