#include "stdafx.h"
#include "gmparticles.h"
#include "foundation/gamemachine.h"
#include "foundation/gmthreads.h"
#include "foundation/utilities/gmprimitivecreator.h"
#include "gmengine/gmgameworld.h"

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
	if (d->firstSimulate)
	{
		d->startTick = GameMachine::instance().getGameTimeSeconds();
		d->currentTick = d->startTick;
		d->firstSimulate = false;
	}
	else
	{
		d->currentTick = GameMachine::instance().getGameTimeSeconds();
	}

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
	prototype->setUsageHint(GMUsageHint::DynamicDraw);

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

bool GMParticlesEmitter::isEmissionFinished()
{
	D(d);
	if (d->emitterProps.emissionTimes == InfiniteEmissionTimes)
		return false;
	return d->emitterProps.particleCount == d->emitterProps.emissionFinishedParticleCount;
}

void GMParticlesEmitter::onAppendingObjectToWorld()
{
	D(d);
	setParticlesCount(d->emitterProps.particleCount);
	setParticlesHandler(this);
	for (GMint i = 0; i < d->emitterProps.particleCount; i++)
	{
		d->particleProps[i].direction = linear_math::normalize(d->particleProps[i].direction);
		d->particleProps[i].startupPosition = d->emitterProps.position;
	}

	GMParticles::onAppendingObjectToWorld();
}

//////////////////////////////////////////////////////////////////////////
GMLerpParticleEmitter::~GMLerpParticleEmitter()
{
	D(d);
	if (d->prototype)
		delete d->prototype;
}

void GMLerpParticleEmitter::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	// 位置、大小初始值无关紧要
	static GMfloat size[] = { 1, 1, 1 };
	static GMfloat position[] = { 0, 0, 0 };
	GMPrimitiveCreator::createQuad(size, position, &d->prototype, GMMeshType::Particles);
	GMParticlesEmitter::onAppendingObjectToWorld();
}

GMParticleGameObject* GMLerpParticleEmitter::createParticle(const GMint index)
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	ASSERT(db->particleProps);
	auto particle = new GMParticleGameObject(d->prototype);
	auto& props = db->particleProps[index];
	particle->setMaxLife(props.life);
	return particle;
}

void GMLerpParticleEmitter::checkEmit(const GMint index)
{
	D_BASE(d, GMParticlesEmitter);
	if (!d->particleProps[index].emitted)
	{
		// 没有在发射期，不绘制此粒子
		d->particleProps[index].visible = false;

		// 打一个标记，进入发射倒计时，在下一次respawn的时候发射
		d->particleProps[index].emitCountdown = true;
	}
}

void GMLerpParticleEmitter::update(const GMint index, GMParticleGameObject* particle)
{
	D_BASE(d, GMParticlesEmitter);
	checkEmit(index);

	GMfloat percentage = 1 - particle->getCurrentLife() / particle->getMaxLife();
	GMfloat diff = particle->getMaxLife() - particle->getCurrentLife();
	GMfloat size = linear_math::lerp(d->particleProps[index].startSize, d->particleProps[index].endSize, percentage);
	
	linear_math::Matrix4x4 transform;
	if (d->particleProps[index].visible)
	{
		if (d->emitterProps.positionType != GMParticlePositionType::FollowEmitter)
		{
			transform = linear_math::translate(d->particleProps[index].startupPosition + d->particleProps[index].direction * d->emitterProps.speed * diff)
				* linear_math::lerp(d->particleProps[index].startAngle, d->particleProps[index].endAngle, percentage).toMatrix()
				* linear_math::scale(linear_math::Vector3(size));
		}
		else
		{
			transform = linear_math::translate(d->emitterProps.position + d->particleProps[index].direction * d->emitterProps.speed * diff)
				* linear_math::lerp(d->particleProps[index].startAngle, d->particleProps[index].endAngle, percentage).toMatrix()
				* linear_math::scale(linear_math::Vector3(size));
		}
	}
	else
	{
		transform = linear_math::Matrix4x4(0, 0, 0, 0);
	}
	particle->setTransform(transform);

	linear_math::Vector4 currentColor = linear_math::lerp(d->particleProps[index].startColor, d->particleProps[index].endColor, percentage);
	particle->setColor(currentColor);

	reduceLife(particle);
}

void GMLerpParticleEmitter::respawn(const GMint index, GMParticleGameObject* particle)
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
		if (d->emitterProps.emissionTimes > 0)
		{
			if (d->particleProps[index].currentEmissionTimes <= d->emitterProps.emissionTimes)
				d->particleProps[index].currentEmissionTimes++;

			if (d->particleProps[index].currentEmissionTimes == d->emitterProps.emissionTimes)
			{
				d->particleProps[index].visible = false;
				d->emitterProps.emissionFinishedParticleCount++;
			}
		}

		return respawnLife(index, particle);
	}

	if (d->particleProps[index].emitCountdown)
	{
		// 进入了准备发射倒计时，在此时准备发射
		d->particleProps[index].visible = true;
		d->particleProps[index].emitted = true;
		respawnLife(index, particle);
	}
}

void GMLerpParticleEmitter::reduceLife(GMParticleGameObject* particle)
{
	particle->setCurrentLife(particle->getCurrentLife() - GameMachine::instance().getLastFrameElapsed());
}

void GMLerpParticleEmitter::respawnLife(const GMint index, GMParticleGameObject* particle)
{
	D_BASE(d, GMParticlesEmitter);
	GMfloat currentLife = particle->getCurrentLife();
	if (currentLife < 0)
	{
		currentLife = gmFabs(currentLife);
		GMfloat factor = gmFloor((currentLife - .001f) / particle->getMaxLife()) + 1;
		particle->setCurrentLife(particle->getMaxLife() * factor - currentLife);
	}
	else
	{
		particle->setCurrentLife(particle->getMaxLife());
	}

	if (d->emitterProps.positionType == GMParticlePositionType::RespawnAtEmitterPosition)
		d->particleProps[index].startupPosition = d->emitterProps.position;
}

//////////////////////////////////////////////////////////////////////////
GMRadiusParticlesEmitter::~GMRadiusParticlesEmitter()
{
	D(d);
	if (d->currentAngles)
		delete[] d->currentAngles;
}

void GMLerpParticleEmitter::create(
	GMint count,
	GMParticlePositionType positionType,
	GMfloat life,
	GMfloat startSize,
	GMfloat endSize,
	const linear_math::Vector3& emitterPosition,
	const linear_math::Vector3& startDirectionRange,
	const linear_math::Vector3& endDirectionRange,
	const linear_math::Vector4& startColor,
	const linear_math::Vector4& endColor,
	const linear_math::Quaternion& startAngle,
	const linear_math::Quaternion& endAngle,
	GMfloat emissionRate,
	GMfloat speed,
	GMint emissionTimes,
	OUT GMParticlesEmitter** emitter)
{
	GMLerpParticleEmitter* e = new GMLerpParticleEmitter();
	GMParticleEmitterProperties emitterProps;
	emitterProps.positionType = positionType;
	emitterProps.position = emitterPosition;
	emitterProps.particleCount = count;
	emitterProps.emissionRate = emissionRate;
	emitterProps.speed = speed;
	emitterProps.emissionTimes = emissionTimes;

	GMParticleProperties* particleProps = new GMParticleProperties[count];
	for (GMint i = 0; i < count; i++)
	{
		if (count == 1)
		{
			particleProps[i].direction = linear_math::normalize(
				linear_math::normalize(startDirectionRange) + linear_math::normalize(endDirectionRange) / 2
			);
		}
		else
		{
			linear_math::Vector3 normalStart = linear_math::normalize(startDirectionRange),
				normalEnd = linear_math::normalize(endDirectionRange);
			linear_math::Vector3 axis = linear_math::normalize(linear_math::cross(normalStart, normalEnd));
			GMfloat theta = linear_math::dot(normalStart, normalEnd);
			linear_math::Quaternion qStart, qEnd;
			qStart.setRotation(axis, 0);
			qEnd.setRotation(axis, gmAcos(theta));

			linear_math::Quaternion interpolation = linear_math::slerp(qStart, qEnd, (GMfloat)i / (count - 1));
			linear_math::Vector4 transformed = linear_math::toHomogeneous(normalStart) * interpolation.toMatrix();
			particleProps[i].direction = linear_math::toInhomogeneous(transformed);
		}

		particleProps[i].life = life;
		particleProps[i].startAngle = startAngle;
		particleProps[i].endAngle = endAngle;
		particleProps[i].startSize = startSize;
		particleProps[i].endSize = endSize;
		particleProps[i].startColor = startColor;
		particleProps[i].endColor = endColor;
	}
	
	e->setEmitterProperties(emitterProps);
	e->setParticlesProperties(particleProps);
	*emitter = e;
}

void GMRadiusParticlesEmitter::create(
	GMint count,
	GMParticlePositionType positionType,
	GMfloat life,
	GMfloat startSize,
	GMfloat endSize,
	const linear_math::Vector3& rotateAxis,
	GMfloat angularVelocity,
	const linear_math::Vector3& emitterPosition,
	const linear_math::Vector3& direction,
	const linear_math::Vector4& startColor,
	const linear_math::Vector4& endColor,
	const linear_math::Quaternion& startAngle,
	const linear_math::Quaternion& endAngle,
	GMfloat emissionRate,
	GMfloat speed,
	GMint emissionTimes,
	OUT GMParticlesEmitter** emitter
)
{
	GMRadiusParticlesEmitter* e = new GMRadiusParticlesEmitter();
	GMParticleEmitterProperties emitterProps;
	emitterProps.positionType = positionType;
	emitterProps.position = emitterPosition;
	emitterProps.particleCount = count;
	emitterProps.emissionRate = emissionRate;
	emitterProps.speed = speed;
	emitterProps.emissionTimes = emissionTimes;

	GMParticleProperties* particleProps = new GMParticleProperties[count];
	e->data()->currentAngles = new GMfloat[count];
	for (GMint i = 0; i < count; i++)
	{
		particleProps[i].direction = linear_math::normalize(direction);
		particleProps[i].life = life;
		particleProps[i].startAngle = startAngle;
		particleProps[i].endAngle = endAngle;
		particleProps[i].startSize = startSize;
		particleProps[i].endSize = endSize;
		particleProps[i].startColor = startColor;
		particleProps[i].endColor = endColor;
	}

	e->setAngularVelocity(angularVelocity);
	e->setRotateAxis(linear_math::normalize(rotateAxis));
	e->setEmitterProperties(emitterProps);
	e->setParticlesProperties(particleProps);
	*emitter = e;
}

void GMRadiusParticlesEmitter::update(const GMint index, GMParticleGameObject* particle)
{
	D(d);
	D_BASE(db, GMParticlesEmitter);

	if (index == 0)
		d->currentAngle = d->angularVelocity * getElapsedTime();

	checkEmit(index);

	GMfloat percentage = 1 - particle->getCurrentLife() / particle->getMaxLife();
	GMfloat diff = particle->getMaxLife() - particle->getCurrentLife();
	GMfloat size = linear_math::lerp(db->particleProps[index].startSize, db->particleProps[index].endSize, percentage);
	linear_math::Quaternion rotation;
	rotation.setRotation(d->rotateAxis, d->currentAngles[index]);

	linear_math::Matrix4x4 transform;
	if (db->particleProps[index].visible)
	{
		linear_math::Vector4 rotatedDirection = linear_math::toHomogeneous(db->particleProps[index].direction) * rotation.toMatrix();
		
		if (db->emitterProps.positionType != GMParticlePositionType::FollowEmitter)
		{
			transform = linear_math::translate(db->particleProps[index].startupPosition + linear_math::toInhomogeneous(rotatedDirection) * db->emitterProps.speed * diff)
				* linear_math::lerp(db->particleProps[index].startAngle, db->particleProps[index].endAngle, percentage).toMatrix()
				* linear_math::scale(linear_math::Vector3(size));
		}
		else
		{
			transform = linear_math::translate(db->emitterProps.position + linear_math::toInhomogeneous(rotatedDirection) * db->emitterProps.speed * diff)
				* linear_math::lerp(db->particleProps[index].startAngle, db->particleProps[index].endAngle, percentage).toMatrix()
				* linear_math::scale(linear_math::Vector3(size));
		}
	}
	else
	{
		transform = linear_math::Matrix4x4(0, 0, 0, 0);
	}
	particle->setTransform(transform);

	linear_math::Vector4 currentColor = linear_math::lerp(db->particleProps[index].startColor, db->particleProps[index].endColor, percentage);
	particle->setColor(currentColor);

	reduceLife(particle);
}

void GMRadiusParticlesEmitter::respawn(const GMint index, GMParticleGameObject* particle)
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	GMLerpParticleEmitter::respawn(index, particle);

	if (db->particleProps[index].emitted)
	{
		// 更新角速度
		d->currentAngles[index] = d->currentAngle;
	}
}

//////////////////////////////////////////////////////////////////////////
GMCustomParticlesEmitter::GMCustomParticlesEmitter(AUTORELEASE GMModel* model)
{
	D(d);
	d->prototype = model;
}

GMCustomParticlesEmitter::~GMCustomParticlesEmitter()
{
	D(d);
	if (d->prototype)
		delete d->prototype;
}

void GMCustomParticlesEmitter::load(const GMBuffer& script)
{
	D(d);
	if (d->lua.loadBuffer(script) == GMLuaStatus::OK)
		d->loaded = true;
}

void GMCustomParticlesEmitter::onAppendingObjectToWorld()
{
	D(d);
	setParticlesHandler(this);
	if (!d->loaded)
	{
		gm_error("Particle script is not loaded.");
		return;
	}

	// 调用接口 function particlesCount(), 返回粒子数目
	GMLuaVariable ret[1];
	GMLuaStatus result = d->lua.call("particlesCount", {}, ret);
	if (result != GMLuaStatus::OK)
		gm_error("LUA calling 'particlesCount' failed.");
	else if (ret->type != GMLuaVariableType::Number && ret->type != GMLuaVariableType::Int)
		gm_error("LUA calling 'particlesCount' return type error.");
	else if (ret->type == GMLuaVariableType::Int)
		setParticlesCount(ret->valInt);
	else
		setParticlesCount(ret->valFloat);

	GMParticles::onAppendingObjectToWorld();
}

GMParticleGameObject* GMCustomParticlesEmitter::createParticle(const GMint index)
{
	D(d);
	auto particle = new GMParticleGameObject(d->prototype);

	// 调用接口 function particlesMaxLife(), 返回粒子生命
	GMLuaVariable ret[1];
	GMLuaStatus result = d->lua.call("particlesMaxLife", {}, ret);
	if (result != GMLuaStatus::OK)
		gm_error("LUA calling 'particlesMaxLife' failed.");
	else if (ret->type != GMLuaVariableType::Number && ret->type != GMLuaVariableType::Int)
		gm_error("LUA calling 'particlesMaxLife' return type error.");
	else if (ret->type == GMLuaVariableType::Int)
		particle->setMaxLife(ret->valInt);
	else
		particle->setMaxLife(ret->valFloat);
	return particle;
}

void GMCustomParticlesEmitter::update(const GMint index, GMParticleGameObject* particle)
{
	D(d);
	// 调用接口 function particlesUpdate(index, particle), 返回粒子table
	GMLuaStatus result = d->lua.call("particlesUpdate", { index, *particle }, particle, 1);
	if (result != GMLuaStatus::OK)
		gm_error("LUA calling 'particlesUpdate(index)' failed.");
}

void GMCustomParticlesEmitter::respawn(const GMint index, GMParticleGameObject* particle)
{
	D(d);
	// 调用接口 function particlesRespawn(index, particle), 返回粒子table
	GMLuaStatus result = d->lua.call("particlesRespawn", { index, *particle }, particle, 1);
	if (result != GMLuaStatus::OK)
		gm_error("LUA calling 'particlesUpdate(index)' failed.");
}