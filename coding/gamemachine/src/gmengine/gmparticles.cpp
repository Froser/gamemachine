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
	Object* prototype = getObject();
	GMfloat* basePositions = d->parentParticles->getPositionArray(prototype);
	GMMesh* mesh = prototype->getAllMeshes()[0];
	GMbyte* vertexData = (GMbyte*)buffer;
	GMint index = getIndexInPrototype();
	GMint color_offset = mesh->get_transferred_positions_byte_size() + mesh->get_transferred_uvs_byte_size() + 4 * index;
	GMfloat *ptrPosition = (GMfloat*)(vertexData),
		*ptrColor = (GMfloat*)(vertexData + color_offset);

	GMint offset = 0;
	for (auto& component : mesh->getComponents())
	{
		for (GMuint i = 0; i < component->getPrimitiveCount(); i++)
		{
			for (GMint j = 0; j < component->getPrimitiveVerticesCountPtr()[i]; j++)
			{
				//linear_math::copyVector(d->color, offset * 4);
				linear_math::Vector4 basePositionVector(
					*(basePositions + offset * 4 + 0),
					*(basePositions + offset * 4 + 1),
					*(basePositions + offset * 4 + 2),
					*(basePositions + offset * 4 + 3)
				);
				linear_math::Vector4 transformedPosition = basePositionVector * d->transform;
				linear_math::copyVector(transformedPosition, ptrPosition + offset * 4);

				//更新颜色
				linear_math::copyVector(d->color, ptrColor + offset * 4);
				offset++;
			}
		}
	}
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
	for (auto& positions : d->basePositions)
	{
		if (positions.second)
			delete[] positions.second;
	}

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
		Object* prototype = kv.first;
		GMObjectPainter* painter = prototype->getPainter();
		void* buffer = painter->getBuffer();
		for (const auto& particle : kv.second)
		{
			particle->updatePrototype(buffer);
		}
		painter->endUpdateBuffer();
		kv.first->getPainter()->draw(nullptr);
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

			break; //////////////////////////////////////////////////////////////////////////
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

void GMParticles::initPrototype(Object* prototype, const Vector<GMParticleGameObject*>& particles)
{
	D(d);
	prototype->setHint(GMUsageHint::DynamicDraw);

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

	// 需要把所有positions记录下来，用于做顶点变换
	auto targetOriginalPositions = d->basePositions.find(prototype);
	if (targetOriginalPositions == d->basePositions.end())
	{
		auto size = mesh->positions().size();
		auto sizeInBytes = size * sizeof(mesh->positions()[0]);
		GMfloat* positions = new GMfloat[size];
		memcpy_s(positions, sizeInBytes, mesh->positions().data(), sizeInBytes);
		d->basePositions.insert({ prototype, positions });
	}
}

void GMParticles::addParticle(AUTORELEASE Object* prototype, AUTORELEASE GMParticleGameObject* particle)
{
	D(d);
	auto& vec = d->particles[prototype];
	vec.push_back(particle);
	particle->setParent(this);
	particle->setIndex(vec.size() - 1);
	d->allParticles.push_back(particle);
}

bool GMParticles::containsPrototype(Object* prototype)
{
	D(d);
	return d->particles.find(prototype) != d->particles.end();
}