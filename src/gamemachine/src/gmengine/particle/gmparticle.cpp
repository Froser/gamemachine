#include "stdafx.h"
#include "gmparticle.h"

void GMParticlePool::init(GMsize_t count)
{
	D(d);
	d->particlePool.reserve(count);
	for (GMsize_t i = 0; i < count; ++i)
	{
		d->particlePool.emplace_back(GMOwnedPtr<GMParticle>(new GMParticle()));
	}
}

void GMParticlePool::free()
{
	D(d);
	GMClearSTLContainer(d->particlePool);
}

GMParticle* GMParticlePool::alloc()
{
	D(d);
	return d->particlePool[d->index++].get();
}