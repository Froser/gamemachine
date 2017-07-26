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

	if (prototype->getAllMeshes().size() > 1)
	{
		ASSERT(false);
		gm_warning("Only one mesh in particle's prototype is allowed.");
	}
}

void GMParticleGameObject::updatePrototype(void* buffer)
{
	D(d);
	GMfloat* vertexData = (GMfloat*)buffer;
	GMint index = getIndexInPrototype();
	GMint position_offset = 10 * index, // 10 = 4(pos) + 2(uv) + 4(color)
		uv_offset = 10 * index + 2,
		color_offset = 10 * index + 4;
	GMfloat* ptrPos = vertexData + position_offset,
		*ptrUv = vertexData + uv_offset,
		*ptrColor = vertexData + color_offset;
	copyVector(d->position, ptrPos);
	copyVector(d->uv, ptrUv);
	copyVector(d->color, ptrColor);
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

	for (auto& kv : d->particles)
	{
		Object* prototype = kv.first;
		initPrototype(kv.first, kv.second);
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
		drawInstances(kv.first, kv.second.size());
	}
}

void GMParticles::simulate()
{
	D(d);
	for (const auto& kv : d->particles)
	{
		Object* prototype = kv.first;
		GMObjectPainter* painter = prototype->getPainter();
		painter->beginUpdateBuffer(prototype->getAllMeshes()[0]);
		void* buffer = painter->getBuffer();
		ASSERT(buffer);
		for (const auto& particle : kv.second)
		{
			if (d->particleHandler)
				d->particleHandler->update(particle);

			particle->updatePrototype(buffer);
		}
		painter->endUpdateBuffer();
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

void GMParticles::initPrototype(Object* prototype, const Vector<GMParticleGameObject*>& particles)
{
	prototype->setHint(GMUsageHint::DynamicDraw);
	prototype->setVertexDataDivisor(GMVertexDataType::Position, 1);
	prototype->setVertexDataDivisor(GMVertexDataType::UV, 1);
	prototype->setVertexDataDivisor(GMVertexDataType::Color, 1);

	auto mesh = prototype->getAllMeshes()[0];
	mesh->disableData(GMVertexDataType::Normal);
	mesh->disableData(GMVertexDataType::Tangent);
	mesh->disableData(GMVertexDataType::Bitangent);
	mesh->disableData(GMVertexDataType::Lightmap);

	// 拷贝若干倍数据
	decltype(particles.size()) size = particles.size();
	mesh->positions().resize(mesh->positions().size() * size);
	mesh->uvs().resize(mesh->uvs().size() * size);
	mesh->colors().resize(mesh->colors().size() * size);
}

void GMParticles::addParticle(AUTORELEASE Object* prototype, AUTORELEASE GMParticleGameObject* particle)
{
	D(d);
	auto& vec = d->particles[prototype];
	vec.push_back(particle);
	particle->setIndex(vec.size() - 1);
	d->allParticles.push_back(particle);
}

bool GMParticles::containsPrototype(Object* prototype)
{
	D(d);
	return d->particles.find(prototype) != d->particles.end();
}

void GMParticles::drawInstances(Object* prototype, GMuint count)
{
	GMObjectPainter* painter = prototype->getPainter();
	painter->drawInstances(count);
}