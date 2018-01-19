#include "stdafx.h"
#include "gmparticles.h"
#include "foundation/gamemachine.h"
#include "foundation/gmthread.h"
#include "foundation/utilities/utilities.h"
#include "gmengine/gmgameworld.h"

GMParticleGameObject::GMParticleGameObject(AUTORELEASE GMModel* prototype)
{
	D(d);

	GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, prototype);
	setModel(asset);
}

void GMParticleGameObject::updatePrototype(void* buffer)
{
	D(d);
	GMModel* prototype = getModel();
	GMMesh* mesh = prototype->getMesh();
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
		glm::vec4 basePositionVector(
			*(basePositions + offset * GMModel::PositionDimension + 0),
			*(basePositions + offset * GMModel::PositionDimension + 1),
			*(basePositions + offset * GMModel::PositionDimension + 2),
			1.f
		);
		glm::vec4 transformedPosition = d->transform * basePositionVector;
		GM_ASSERT((GMLargeInteger)(vertexData + offset_position + offset * GMModel::PositionDimension + 2) <= (GMLargeInteger)(vertexData + mesh->get_transferred_positions_byte_size() + mesh->get_transferred_uvs_byte_size()));
		glm::copyToArray(glm::vec3(transformedPosition), (GMfloat*)(vertexData + offset_position) + offset * GMModel::PositionDimension);

		//更新颜色
		GM_ASSERT((GMLargeInteger)(vertexData + color_offset + offset * GMModel::TextureDimension + 3) <= (GMLargeInteger)(vertexData + totalSize));
		glm::copyToArray(d->color, (GMfloat*)(vertexData + color_offset) + offset * GMModel::TextureDimension);
	}
}

//////////////////////////////////////////////////////////////////////////
GMParticles::~GMParticles()
{
	D(d);
	for (auto& positions : d->basePositions)
	{
		if (positions.second)
			delete[] positions.second;
	}

	for (const auto& prototype : d->prototypes)
	{
		delete prototype;
	}

	for (const auto& particle : d->allParticles)
	{
		delete particle;
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
		painter->beginUpdateBuffer(prototype->getMesh());
		void* buffer = painter->getBuffer();
		for (const auto& particle : kv.second)
		{
			particle->updatePrototype(buffer);
		}
		painter->endUpdateBuffer();
		painter->draw(nullptr);
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
		GM_ASSERT(particle);
		particle->setWorld(db->world);
		addParticle(particle->getPrototype(), particle);
	}

	for (auto& kv : d->particles)
	{
		GMModel* prototype = kv.first;
		initPrototype(kv.first, kv.second);
		GameMachine::instance().createModelPainterAndTransfer(prototype);
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

	auto mesh = prototype->getMesh();
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
	d->prototypes.insert(prototype);
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
		d->particleProps[i].direction = glm::fastNormalize(d->particleProps[i].direction);
		d->particleProps[i].startupPosition = d->emitterProps.position;
	}

	GMParticles::onAppendingObjectToWorld();
}

//////////////////////////////////////////////////////////////////////////
void GMLerpParticleEmitter::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	// 位置、大小初始值无关紧要
	static GMfloat size[] = { 1, 1, 1 };
	static GMfloat position[] = { 0, 0, 0 };
	GMPrimitiveCreator::createQuad(size, position, &d->prototype, nullptr, GMModelType::Particles);
	GMParticlesEmitter::onAppendingObjectToWorld();
}

GMParticleGameObject* GMLerpParticleEmitter::createParticle(const GMint index)
{
	D(d);
	D_BASE(db, GMParticlesEmitter);
	GM_ASSERT(db->particleProps);
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
	GMfloat size = glm::lerp(d->particleProps[index].startSize, d->particleProps[index].endSize, percentage);
	
	glm::mat4 transform;
	if (d->particleProps[index].visible)
	{
		if (d->emitterProps.positionType != GMParticlePositionType::FollowEmitter)
		{
			transform = glm::translate(d->particleProps[index].startupPosition + d->particleProps[index].direction * d->emitterProps.speed * diff)
				* glm::mat4_cast(glm::lerp(d->particleProps[index].startAngle, d->particleProps[index].endAngle, percentage))
				* glm::scale(glm::vec3(size));
		}
		else
		{
			transform = glm::translate(d->emitterProps.position + d->particleProps[index].direction * d->emitterProps.speed * diff)
				* glm::mat4_cast(glm::lerp(d->particleProps[index].startAngle, d->particleProps[index].endAngle, percentage))
				* glm::scale(glm::vec3(size));
		}
	}
	else
	{
		transform = glm::mat4(0);
	}
	particle->setTransform(transform);

	glm::vec4 currentColor = glm::lerp(d->particleProps[index].startColor, d->particleProps[index].endColor, percentage);
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
	particle->setCurrentLife(particle->getCurrentLife() - GM.getGameMachineRunningStates().lastFrameElpased);
}

void GMLerpParticleEmitter::respawnLife(const GMint index, GMParticleGameObject* particle)
{
	D_BASE(d, GMParticlesEmitter);
	GMfloat currentLife = particle->getCurrentLife();
	if (currentLife < 0)
	{
		currentLife = gmFabs(currentLife);
		GMfloat factor = gmFloor((currentLife - .001f) / particle->getMaxLife());
		if (factor < 0)
			factor = 0;
		++factor;
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
	const glm::vec3& emitterPosition,
	const glm::vec3& startDirectionRange,
	const glm::vec3& endDirectionRange,
	const glm::vec4& startColor,
	const glm::vec4& endColor,
	const glm::quat& startAngle,
	const glm::quat& endAngle,
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
			particleProps[i].direction = glm::fastNormalize(
				glm::fastNormalize(startDirectionRange) + glm::fastNormalize(endDirectionRange) / 2.f
			);
		}
		else
		{
			glm::vec3 normalStart = glm::fastNormalize(startDirectionRange),
				normalEnd = glm::fastNormalize(endDirectionRange);
			glm::vec3 axis = glm::fastNormalize(glm::cross(normalStart, normalEnd));
			GMfloat theta = glm::dot(normalStart, normalEnd);
			glm::quat qStart = glm::rotate(glm::identity<glm::quat>(), 0.f, axis),
				qEnd = glm::rotate(glm::identity<glm::quat>(), gmAcos(theta), axis);

			glm::quat interpolation = glm::slerp(qStart, qEnd, (GMfloat)i / (count - 1));
			glm::vec4 transformed = glm::mat4_cast(interpolation) * glm::toHomogeneous(normalStart);
			particleProps[i].direction = glm::toInhomogeneous(transformed);
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
	const glm::vec3& rotateAxis,
	GMfloat angularVelocity,
	const glm::vec3& emitterPosition,
	const glm::vec3& direction,
	const glm::vec4& startColor,
	const glm::vec4& endColor,
	const glm::quat& startAngle,
	const glm::quat& endAngle,
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
		particleProps[i].direction = glm::fastNormalize(direction);
		particleProps[i].life = life;
		particleProps[i].startAngle = startAngle;
		particleProps[i].endAngle = endAngle;
		particleProps[i].startSize = startSize;
		particleProps[i].endSize = endSize;
		particleProps[i].startColor = startColor;
		particleProps[i].endColor = endColor;
	}

	e->setAngularVelocity(angularVelocity);
	e->setRotateAxis(glm::fastNormalize(rotateAxis));
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
	GMfloat size = glm::lerp(db->particleProps[index].startSize, db->particleProps[index].endSize, percentage);
	glm::quat rotation = glm::rotate(glm::identity<glm::quat>(), d->currentAngles[index], d->rotateAxis);

	glm::mat4 transform;
	if (db->particleProps[index].visible)
	{
		glm::vec4 rotatedDirection = glm::mat4_cast(rotation) * glm::toHomogeneous(db->particleProps[index].direction);
		
		if (db->emitterProps.positionType != GMParticlePositionType::FollowEmitter)
		{
			transform = glm::translate(db->particleProps[index].startupPosition + glm::toInhomogeneous(rotatedDirection) * db->emitterProps.speed * diff)
				* glm::mat4_cast(glm::lerp(db->particleProps[index].startAngle, db->particleProps[index].endAngle, percentage))
				* glm::scale(glm::vec3(size));
		}
		else
		{
			transform = glm::translate(db->emitterProps.position + glm::toInhomogeneous(rotatedDirection) * db->emitterProps.speed * diff)
				* glm::mat4_cast(glm::lerp(db->particleProps[index].startAngle, db->particleProps[index].endAngle, percentage))
				* glm::scale(glm::vec3(size));
		}
	}
	else
	{
		transform = glm::mat4(0);
	}
	particle->setTransform(transform);

	glm::vec4 currentColor = glm::lerp(db->particleProps[index].startColor, db->particleProps[index].endColor, percentage);
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