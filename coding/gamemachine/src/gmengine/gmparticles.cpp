#include "stdafx.h"
#include "gmparticles.h"
#include "foundation/gamemachine.h"
#include "foundation/gmthreads.h"

static void destructor(GMGameObject*) {}

GMParticleGameObject::GMParticleGameObject(Object* prototype)
	: GMGameObject(prototype)
{
	D(d);
	setDestructor(destructor);
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
		particle->setWorld(db->world);
		addParticle(particle->getPrototype(), particle);
	}

	for (auto& particle : d->particles)
	{
		Object* prototype = particle.first;
		GameMachine::instance().initObjectPainter(prototype);
	}
}

GMParticles::~GMParticles()
{
	D(d);
	for (const auto& kv : d->particles)
	{
		delete kv.first;
		
		for (const auto& p : kv.second)
		{
			delete p;
		}
	}
}

inline GMParticleGameObject* GMParticles::getParticle(GMint index)
{
	D(d);
	return d->allParticles[index];
}

void GMParticles::draw()
{
	D(d);
	for (const auto& kv : d->particles)
	{
		for (const auto& particle : kv.second)
		{
			particle->draw();
		}
	}
}

void GMParticles::simulate()
{
	D(d);
	for (const auto& kv : d->particles)
	{
		for (const auto& particle : kv.second)
		{
			if (d->particleHandler)
				d->particleHandler->update(particle);
		}
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

void GMParticles::addParticle(AUTORELEASE Object* prototype, AUTORELEASE GMParticleGameObject* particle)
{
	D(d);
	d->particles[prototype].insert(particle);
	d->allParticles.push_back(particle);
}

bool GMParticles::containsPrototype(Object* prototype)
{
	D(d);
	return d->particles.find(prototype) != d->particles.end();
}