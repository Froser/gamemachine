#include "stdafx.h"
#include "gmparticles.h"
#include "foundation/gamemachine.h"

GMParticleGameObject::GMParticleGameObject(Object* prototype)
	: GMGameObject(nullptr)
{
	D(d);
	d->prototype = prototype;
	setManualDelete();
}

//////////////////////////////////////////////////////////////////////////
GMParticles::GMParticles(AUTORELEASE Object* prototype, GMint particlesCount, IParticleHandler* handler)
	: GMGameObject(nullptr)
{
	D(d);
	d->prototype = prototype;
	d->particleHandler = handler;
	for (GMint i = 0; i < particlesCount; i++)
	{
		d->particles.push_back(new GMParticleGameObject(prototype));
	}
}

GMParticles::~GMParticles()
{
	D(d);
	if (d->prototype)
		delete d->prototype;

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

void GMParticles::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMGameObject);
	for (auto& particle : d->particles)
	{
		particle->setWorld(db->world);
		GameMachine::instance().initObjectPainter(particle);
	}
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