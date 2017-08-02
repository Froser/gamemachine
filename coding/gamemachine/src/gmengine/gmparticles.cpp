#include "stdafx.h"
#include "gmparticles.h"
#include "foundation/gamemachine.h"
#include "foundation/gmthreads.h"
#include "foundation/utilities/gmprimitivecreator.h"

static void destructor(GMGameObject*) {}

GMParticleGameObject::GMParticleGameObject(GMModel* prototype)
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
	GMModel* prototype = getModel();
	GMMesh* mesh = prototype->getAllMeshes()[0];
	GMbyte* vertexData = (GMbyte*)buffer;

	GMint index = getIndexInPrototype();
	GMint particleCount = d->parentParticles->getParticleCount(prototype);

	GMint offset_position = mesh->get_transferred_positions_byte_size() / particleCount * index;
	GMint color_offset = (mesh->get_transferred_positions_byte_size() + mesh->get_transferred_uvs_byte_size())
		+ mesh->get_transferred_colors_byte_size() / particleCount * index;
	GMfloat* basePositions = d->parentParticles->getPositionArray(prototype)
		+ offset_position / sizeof(decltype(basePositions[0] + 0));

#if _DEBUG
	GMint totalSize = mesh->get_transferred_positions_byte_size()
		+ mesh->get_transferred_uvs_byte_size()
		+ mesh->get_transferred_colors_byte_size();
#endif

	// 这一轮需要更新的顶点数量
	GMint vertexCountThisTurn = mesh->get_transferred_positions_byte_size() / sizeof(GMModel::DataType) / particleCount / GMModel::PositionDimension;
	for (GMint offset = 0; offset < vertexCountThisTurn; ++offset)
	{
		linear_math::Vector4 basePositionVector(
			*(basePositions + offset * 4 + 0),
			*(basePositions + offset * 4 + 1),
			*(basePositions + offset * 4 + 2),
			*(basePositions + offset * 4 + 3)
		);
		linear_math::Vector4 transformedPosition = basePositionVector * d->transform;
		ASSERT((GMLargeInteger)(vertexData + offset_position + offset * 4 + 3) <= (GMLargeInteger)(vertexData + mesh->get_transferred_positions_byte_size() + mesh->get_transferred_uvs_byte_size()));
		linear_math::copyVector(transformedPosition, (GMfloat*)(vertexData + offset_position) + offset * 4);

		//更新颜色
		ASSERT((GMLargeInteger)(vertexData + color_offset + offset * 4 + 3) <= (GMLargeInteger)(vertexData + totalSize));
		linear_math::copyVector(d->color, (GMfloat*)(vertexData + color_offset) + offset * 4);
	}

	setCurrentLife(getCurrentLife() - GameMachine::instance().getLastFrameElapsed());
}

//////////////////////////////////////////////////////////////////////////
GMParticles::GMParticles()
	: GMGameObject(nullptr)
{
}

GMParticles::~GMParticles()
{
	D(d);
	return;
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
		GMModel* prototype = kv.first;
		GMModelPainter* painter = prototype->getPainter();
		void* buffer = painter->getBuffer();
		for (const auto& particle : kv.second)
		{
			particle->updatePrototype(buffer);
		}
		painter->endUpdateBuffer();
		kv.first->getPainter()->draw(nullptr);
	}
}

void GMParticles::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMGameObject);

	GMGameObject::onAppendingObjectToWorld();

	for (GMint i = 0; i < d->particlesCount; i++)
	{
		GMParticleGameObject* particle = d->particleHandler->createParticle(i);
		ASSERT(particle);
		particle->setWorld(db->world);
		addParticle(particle->getPrototype(), particle);
	}

	for (auto& kv : d->particles)
	{
		GMModel* prototype = kv.first;
		initPrototype(kv.first, kv.second);
		GameMachine::instance().initObjectPainter(prototype);
	}
}

void GMParticles::simulate()
{
	D(d);
	GMint i = 0;
	for (auto& particle : d->allParticles)
	{
		if (d->particleHandler)
		{
 			if (particle->getCurrentLife() <= 0)
			{
				d->particleHandler->respawn(i++, particle);
			}
			else
			{
				d->particleHandler->update(i++, particle);
			}
		}
	}
}

GMint GMParticles::getParticleCount(GMModel* prototype)
{
	D(d);
	auto iter = d->particles.find(prototype);
	if (iter == d->particles.end())
		return 0;
	return iter->second.size();
}

void GMParticles::initPrototype(GMModel* prototype, const Vector<GMParticleGameObject*>& particles)
{
	D(d);
	prototype->setHint(GMUsageHint::DynamicDraw);

	auto mesh = prototype->getAllMeshes()[0];
	mesh->disableData(GMVertexDataType::Normal);
	mesh->disableData(GMVertexDataType::Tangent);
	mesh->disableData(GMVertexDataType::Bitangent);
	mesh->disableData(GMVertexDataType::Lightmap);

	auto size = particles.size();
	// 创建指针列表
	GMint offset = 0;
	for (auto& component : mesh->getComponents())
	{
		component->expand(size);

		// 开启alpha通道
		auto& shader = component->getShader();
		shader.setBlend(true);
		shader.setBlendFactorSource(GMS_BlendFunc::SRC_ALPHA);
		shader.setBlendFactorDest(GMS_BlendFunc::DST_ALPHA);
	}
	
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

void GMParticles::addParticle(AUTORELEASE GMModel* prototype, AUTORELEASE GMParticleGameObject* particle)
{
	D(d);
	auto& vec = d->particles[prototype];
	vec.push_back(particle);
	particle->setParent(this);
	particle->setIndex(vec.size() - 1);
	d->allParticles.push_back(particle);
}

bool GMParticles::containsPrototype(GMModel* prototype)
{
	D(d);
	return d->particles.find(prototype) != d->particles.end();
}

GMParticlesEmitter::~GMParticlesEmitter()
{
	D(d);
	if (d->particleProps)
		delete[] d->particleProps;
}

void GMParticlesEmitter::setEmitterProperties(const GMParticleEmitterProperties& props)
{
	D(d);
	d->emitterProps = props;
}

void GMParticlesEmitter::setParticlesProperties(AUTORELEASE GMParticleProperties* props)
{
	D(d);
	d->particleProps = props;
}

void GMParticlesEmitter::onAppendingObjectToWorld()
{
	D(d);
	setParticlesCount(d->emitterProps.particleCount);
	setParticlesHandler(this);
	GMParticles::onAppendingObjectToWorld();
}

//////////////////////////////////////////////////////////////////////////
GMDefaultParticleEmitter::~GMDefaultParticleEmitter()
{
	D(d);
	if (d->prototype)
		delete d->prototype;
}

void GMDefaultParticleEmitter::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	// 位置、大小初始值无关紧要
	static GMfloat size[] = { 1, 1, 1 };
	static GMfloat position[] = { 0, 0, 0 };
	GMPrimitiveCreator::createQuad(size, position, &d->prototype, GMMeshType::Particles);
	GMParticlesEmitter::onAppendingObjectToWorld();
}

GMParticleGameObject* GMDefaultParticleEmitter::createParticle(const GMint index)
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	ASSERT(db->particleProps);
	auto particle = new GMParticleGameObject(d->prototype);
	auto& props = db->particleProps[index];
	particle->setMaxLife(props.life);
	return particle;
}

void GMDefaultParticleEmitter::update(const GMint index, GMParticleGameObject* particle)
{
	D_BASE(d, GMParticlesEmitter);
	if (!d->particleProps[index].emitted)
	{
		// 没有在发射期，不绘制此粒子
		d->particleProps[index].visible = false;

		// 打一个标记，进入发射倒计时，在下一次respawn的时候发射
		d->particleProps[index].emitCountdown = true;
	}

	GMfloat percentage = 1 - particle->getCurrentLife() / particle->getMaxLife();
	GMfloat diff = particle->getMaxLife() - particle->getCurrentLife();
	GMfloat size = linear_math::lerp(d->particleProps[index].startSize, d->particleProps[index].endSize, percentage);
	
	// TODO 区分positionType
	linear_math::Matrix4x4 transform;
	if (d->particleProps[index].visible)
	{
		transform = d->particleProps[index].angle.toMatrix()
			* linear_math::translate(d->emitterProps.position + linear_math::Vector3(d->emitterProps.speed * diff))
			* linear_math::scale(linear_math::Vector3(size));
	}
	else
	{
		transform = linear_math::Matrix4x4(0, 0, 0, 0);
	}
	particle->setTransform(transform);

	linear_math::Vector4 currentColor = linear_math::lerp(d->particleProps[index].startColor, d->particleProps[index].endColor, percentage);
	particle->setColor(currentColor);
}

void GMDefaultParticleEmitter::respawn(const GMint index, GMParticleGameObject* particle)
{
	D_BASE(d, GMParticlesEmitter);
	if (!d->particleProps[index].emitted)
	{
		// 对于还没有发射的情况，计算其发射速率决定是否要发射
		GMfloat duration = d->emitterProps.emissionRate * index;
		particle->setCurrentLife(duration);
		// 如果为0，表示马上发射
		if (duration == 0)
			d->particleProps[index].emitCountdown = true;
	}
	else
	{
		return particle->setCurrentLife(particle->getMaxLife());
	}

	if (d->particleProps[index].emitCountdown)
	{
		// 进入了准备发射倒计时，在此时准备发射
		d->particleProps[index].visible = true;
		d->particleProps[index].emitted = true;
		particle->setCurrentLife(particle->getMaxLife());
	}
}

//////////////////////////////////////////////////////////////////////////
void GMEjectionParticleEmitter::create(
	GMint count,
	GMfloat startSize,
	GMfloat endSize,
	const linear_math::Vector4& startColor,
	const linear_math::Vector4& endColor,
	const linear_math::Quaternion& startAngleRange,
	const linear_math::Quaternion& endAngleRange,
	GMfloat emissionRate,
	GMfloat speed,
	OUT GMParticles** emitter)
{
	GMDefaultParticleEmitter* e = new GMDefaultParticleEmitter();

	GMParticleEmitterProperties emitterProps;
	emitterProps.particleCount = count;
	emitterProps.emissionRate = emissionRate;
	emitterProps.speed = speed;

	GMParticleProperties* particleProps = new GMParticleProperties[count];
	for (GMint i = 0; i < count; i++)
	{
		particleProps[i].angle = linear_math::lerp(startAngleRange, endAngleRange, (GMfloat)i / (count - 1));
		particleProps[i].startSize = startSize;
		particleProps[i].endSize = endSize;
		particleProps[i].startColor = startColor;
		particleProps[i].endColor = endColor;
	}
	
	e->setEmitterProperties(emitterProps);
	e->setParticlesProperties(particleProps);
	*emitter = e;
}