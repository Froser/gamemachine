#include "stdafx.h"
#include "gmparticle.h"
#include "gmparticleeffects.h"
#include <gmxml.h>
#include <random>
#include "foundation/gamemachine.h"
#include <zlib.h>

#define Z 0

namespace
{
	GMMat4 getTransformMatrix(const IRenderContext* context)
	{
		GMMat4 mat = Ortho(
			0,
			context->getWindow()->getWindowRect().width,
			0,
			context->getWindow()->getWindowRect().height,
			-1,
			1
		);
		return mat;
	}
}

GM_PRIVATE_OBJECT(GMParticleModel_2D)
{
	GMOwnedPtr<GMGameObject> particleObject;
	GMOwnedPtr<GMModel> particleModel;
	GMParticleSystem* system = nullptr;
};

//! 表示一个2D粒子，是一个四边形
class GMParticleModel_2D : public GMObject, public IParticleModel
{
	GM_DECLARE_PRIVATE(GMParticleModel_2D)

public:
	GMParticleModel_2D(GMParticleSystem* system);

public:
	virtual void render(const IRenderContext* context) override;

private:
	GMGameObject* createGameObject(
		const IRenderContext* context
	);

	void updateData(
		const IRenderContext* context,
		void* dataPtr
	);
	
	void update6Vertices(
		GMVertex* vertex,
		const GMVec3& centerPt,
		const GMVec2& halfExtents,
		const GMVec4& color,
		GMfloat rotationRad,
		const GMVec3& rotationAxis,
		const GMMat4& transformMatrix,
		GMfloat z = 0
	);
};

GMParticleModel_2D::GMParticleModel_2D(GMParticleSystem* system)
{
	D(d);
	d->system = system;
}

void GMParticleModel_2D::render(const IRenderContext* context)
{
	D(d);
	if (!d->particleObject)
	{
		d->particleObject.reset(createGameObject(context));

		if (!d->system->getTexture())
		{
			// 获取并设置纹理
			GMImage* image = nullptr;
			auto& buffer = d->system->getTextureBuffer();
			if (buffer.buffer)
			{
				GMImageReader::load(buffer.buffer, buffer.size, &image);
				if (image)
				{
					ITexture* texture = nullptr;
					GM.getFactory()->createTexture(context, image, &texture);
					GM_delete(image);
					GM_ASSERT(!d->particleObject->getModels().empty());
					GMModel* model = d->particleObject->getModels()[0];
					model->getShader().getTextureList().getTextureSampler(GMTextureType::Ambient).addFrame(texture);
					d->system->setTexture(texture);
				}
			}
		}
	}

	if (d->particleObject)
	{
		// 开始更新粒子数据
		auto dataProxy = d->particleModel->getModelDataProxy();
		dataProxy->beginUpdateBuffer();
		void* dataPtr = dataProxy->getBuffer();
		updateData(context, dataPtr);
		dataProxy->endUpdateBuffer();
	}

	GM_ASSERT(d->particleObject);
	d->particleObject->draw();
}

void GMParticleModel_2D::updateData(const IRenderContext* context, void* dataPtr)
{
	D(d);
	GMVertex* vPtr = reinterpret_cast<GMVertex*>(dataPtr);
	constexpr GMsize_t szVertex = sizeof(GMVertex);
	auto& particles = d->system->getEmitter()->getParticles();
	GMMat4 transformMatrix = getTransformMatrix(context);
	// 一个粒子有6个顶点，2个三角形
	for (auto particle : particles)
	{
		GMfloat he = particle->getSize() / 2.f;
		update6Vertices(
			vPtr,
			particle->getPosition(),
			he,
			particle->getColor(),
			particle->getRotation(),
			GMVec3(0, 0, 1),
			transformMatrix
		);
		vPtr += 6;
	}
}
GMGameObject* GMParticleModel_2D::createGameObject(
	const IRenderContext* context
)
{
	D(d);
	GMGameObject* object = new GMGameObject();
	d->particleModel.reset(new GMModel());
	d->particleModel->getShader().setBlend(true);
	d->particleModel->getShader().setBlendFactorSource(GMS_BlendFunc::SRC_ALPHA);
	d->particleModel->getShader().setBlendFactorDest(GMS_BlendFunc::ONE);
	d->particleModel->setUsageHint(GMUsageHint::DynamicDraw);
	d->particleModel->setType(GMModelType::Particle);

	d->particleModel->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	GMMesh* mesh = new GMMesh(d->particleModel.get());

	// 使用triangles拓扑，一次性填充所有的矩形
	GMsize_t total = d->system->getEmitter()->getParticleCount();
	for (GMsize_t i = 0; i < total; ++i)
	{
		// 一个particle由6个定点组成
		mesh->vertex(GMVertex());
		mesh->vertex(GMVertex());
		mesh->vertex(GMVertex());
		mesh->vertex(GMVertex());
		mesh->vertex(GMVertex());
		mesh->vertex(GMVertex());
	}

	GM.createModelDataProxyAndTransfer(context, d->particleModel.get());
	object->setContext(context);
	object->addModel(GMAssets::createIsolatedAsset(GMAssetType::Model, d->particleModel.get()));
	return object;
}

void GMParticleModel_2D::update6Vertices(
	GMVertex* vertex,
	const GMVec3& centerPt,
	const GMVec2& halfExtents,
	const GMVec4& color,
	GMfloat rotationRad,
	const GMVec3& rotationAxis,
	const GMMat4& transformMatrix,
	GMfloat z
)
{
	constexpr GMfloat texcoord[4][2] =
	{
		{ 0, 1 },
		{ 0, 0 },
		{ 1, 1 },
		{ 1, 0 },
	};

	const GMfloat x = halfExtents.getX(), y = halfExtents.getY();
	GMQuat q = Rotate(rotationRad, rotationAxis);
	GMVec4 raw[4] = {
		GMVec4(centerPt.getX() - x, centerPt.getY() - y, z, 1),
		GMVec4(centerPt.getX() - x, centerPt.getY() + y, z, 1),
		GMVec4(centerPt.getX() + x, centerPt.getY() - y, z, 1),
		GMVec4(centerPt.getX() + x, centerPt.getY() + y, z, 1),
	};

	GMVec4 transformed[4] = {
		raw[0] * q * transformMatrix,
		raw[1] * q * transformMatrix,
		raw[2] * q * transformMatrix,
		raw[3] * q * transformMatrix,
	};

	// 排列方式：
	// 1   | 1 3
	// 0 2 |   2
	// (0, 1, 2), (2, 1, 3)
	const GMfloat vertices[4][3] = {
		{ transformed[0].getX(), transformed[0].getY(), transformed[0].getZ() },
		{ transformed[1].getX(), transformed[1].getY(), transformed[1].getZ() },
		{ transformed[2].getX(), transformed[2].getY(), transformed[2].getZ() },
		{ transformed[3].getX(), transformed[3].getY(), transformed[3].getZ() },
	};

	vertex[0] = {
		{ vertices[0][0], vertices[0][1], vertices[0][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[0][0], texcoord[0][1] }, //texcoord
		{ 0 },
		{ 0 },
		{ 0 },
		{ color.getX(), color.getY(), color.getZ(), color.getW() }
	};
	vertex[1] = {
		{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[1][0], texcoord[1][1] }, //texcoord
		{ 0 },
		{ 0 },
		{ 0 },
		{ color.getX(), color.getY(), color.getZ(), color.getW() }
	};
	vertex[2] = {
		{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[2][0], texcoord[2][1] }, //texcoord
		{ 0 },
		{ 0 },
		{ 0 },
		{ color.getX(), color.getY(), color.getZ(), color.getW() }
	};
	vertex[3] = {
		{ vertices[2][0], vertices[2][1], vertices[2][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[2][0], texcoord[2][1] }, //texcoord
		{ 0 },
		{ 0 },
		{ 0 },
		{ color.getX(), color.getY(), color.getZ(), color.getW() }
	};
	vertex[4] = {
		{ vertices[1][0], vertices[1][1], vertices[1][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[1][0], texcoord[1][1] }, //texcoord
		{ 0 },
		{ 0 },
		{ 0 },
		{ color.getX(), color.getY(), color.getZ(), color.getW() }
	};
	vertex[5] = {
		{ vertices[3][0], vertices[3][1], vertices[3][2] }, //position
		{ 0, -1.f, 0 }, //normal
		{ texcoord[3][0], texcoord[3][1] }, //texcoord
		{ 0 },
		{ 0 },
		{ 0 },
		{ color.getX(), color.getY(), color.getZ(), color.getW() }
	};
}

GM_PRIVATE_OBJECT(GMCocos2DParticleDescriptionProxy)
{
	GMfloat angle = 0;
	GMfloat angleVariance = 0;
	GMfloat speed = 0;
	GMfloat speedVariance = 0;
	GMDuration duration = 0;
	GMParticleEmitterType emitterType = GMParticleEmitterType::Gravity;
	GMfloat maxParticles = 0;
	GMfloat sourcePositionx = 0;
	GMfloat sourcePositiony = 0;
	GMfloat sourcePositionVariancex = 0;
	GMfloat sourcePositionVariancey = 0;
	GMfloat particleLifespan = 0;
	GMfloat particleLifespanVariance = 0;
	GMfloat startColorRed = 0;
	GMfloat startColorGreen = 0;
	GMfloat startColorBlue = 0;
	GMfloat startColorAlpha = 0;
	GMfloat startColorVarianceRed = 0;
	GMfloat startColorVarianceGreen = 0;
	GMfloat startColorVarianceBlue = 0;
	GMfloat startColorVarianceAlpha = 0;
	GMfloat finishColorRed = 0;
	GMfloat finishColorGreen = 0;
	GMfloat finishColorBlue = 0;
	GMfloat finishColorAlpha = 0;
	GMfloat finishColorVarianceRed = 0;
	GMfloat finishColorVarianceGreen = 0;
	GMfloat finishColorVarianceBlue = 0;
	GMfloat finishColorVarianceAlpha = 0;
	GMfloat startParticleSize = 0;
	GMfloat startParticleSizeVariance = 0;
	GMfloat finishParticleSize = 0;
	GMfloat finishParticleSizeVariance = 0;
	GMfloat rotationStart = 0;
	GMfloat rotationStartVariance = 0;
	GMfloat rotationEnd = 0;
	GMfloat rotationEndVariance = 0;
	GMParticleMotionMode positionType = GMParticleMotionMode::Free;
	GMfloat gravityx = 0;
	GMfloat gravityy = 0;
	GMfloat radialAcceleration = 0;
	GMfloat radialAccelVariance = 0;
	GMfloat tangentialAcceleration = 0;
	GMfloat tangentialAccelVariance = 0;
	GMfloat minRadius = 0;
	GMfloat minRadiusVariance = 0;
	GMfloat maxRadius = 0;
	GMfloat maxRadiusVariance = 0;
	GMfloat rotatePerSecond = 0;
	GMfloat rotatePerSecondVariance = 0;
	GMString textureImageData;
	GMString textureFileName;
};

class GMCocos2DParticleDescriptionProxy : public GMObject
{
	GM_DECLARE_PRIVATE(GMCocos2DParticleDescriptionProxy)
	GM_DECLARE_PROPERTY(Angle, angle, GMfloat)
	GM_DECLARE_PROPERTY(AngleVariance, angleVariance, GMfloat)
	GM_DECLARE_PROPERTY(Speed, speed, GMfloat)
	GM_DECLARE_PROPERTY(SpeedVariance, speedVariance, GMfloat)
	GM_DECLARE_PROPERTY(Duration, duration, GMfloat)
	GM_DECLARE_PROPERTY(EmitterType, emitterType, GMParticleEmitterType)
	GM_DECLARE_PROPERTY(MaxParticles, maxParticles, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositionx, sourcePositionx, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositiony, sourcePositiony, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositionVariancex, sourcePositionVariancex, GMfloat)
	GM_DECLARE_PROPERTY(SourcePositionVariancey, sourcePositionVariancey, GMfloat)
	GM_DECLARE_PROPERTY(ParticleLifespan, particleLifespan, GMfloat)
	GM_DECLARE_PROPERTY(ParticleLifespanVariance, particleLifespanVariance, GMfloat)
	GM_DECLARE_PROPERTY(StartColorRed, startColorRed, GMfloat)
	GM_DECLARE_PROPERTY(StartColorGreen, startColorGreen, GMfloat)
	GM_DECLARE_PROPERTY(StartColorBlue, startColorBlue, GMfloat)
	GM_DECLARE_PROPERTY(StartColorAlpha, startColorAlpha, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceRed, startColorVarianceRed, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceGreen, startColorVarianceGreen, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceBlue, startColorVarianceBlue, GMfloat)
	GM_DECLARE_PROPERTY(StartColorVarianceAlpha, startColorVarianceAlpha, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorRed, finishColorRed, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorGreen, finishColorGreen, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorBlue, finishColorBlue, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorAlpha, finishColorAlpha, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceRed, finishColorVarianceRed, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceGreen, finishColorVarianceGreen, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceBlue, finishColorVarianceBlue, GMfloat)
	GM_DECLARE_PROPERTY(FinishColorVarianceAlpha, finishColorVarianceAlpha, GMfloat)
	GM_DECLARE_PROPERTY(StartParticleSize, startParticleSize, GMfloat)
	GM_DECLARE_PROPERTY(StartParticleSizeVariance, startParticleSizeVariance, GMfloat)
	GM_DECLARE_PROPERTY(FinishParticleSize, finishParticleSize, GMfloat)
	GM_DECLARE_PROPERTY(FinishParticleSizeVariance, finishParticleSizeVariance, GMfloat)
	GM_DECLARE_PROPERTY(RotationStart, rotationStart, GMfloat)
	GM_DECLARE_PROPERTY(RotationStartVariance, rotationStartVariance, GMfloat)
	GM_DECLARE_PROPERTY(RotationEnd, rotationEnd, GMfloat)
	GM_DECLARE_PROPERTY(RotationEndVariance, rotationEndVariance, GMfloat)
	GM_DECLARE_PROPERTY(PositionType, positionType, GMParticleMotionMode)
	GM_DECLARE_PROPERTY(Gravityx, gravityx, GMfloat)
	GM_DECLARE_PROPERTY(Gravityy, gravityy, GMfloat)
	GM_DECLARE_PROPERTY(RadialAcceleration, radialAcceleration, GMfloat)
	GM_DECLARE_PROPERTY(RadialAccelVariance, radialAccelVariance, GMfloat)
	GM_DECLARE_PROPERTY(TangentialAcceleration, tangentialAcceleration, GMfloat)
	GM_DECLARE_PROPERTY(TangentialAccelVariance, tangentialAccelVariance, GMfloat)
	GM_DECLARE_PROPERTY(MinRadius, minRadius, GMfloat)
	GM_DECLARE_PROPERTY(MinRadiusVariance, minRadiusVariance, GMfloat)
	GM_DECLARE_PROPERTY(MaxRadius, maxRadius, GMfloat)
	GM_DECLARE_PROPERTY(MaxRadiusVariance, maxRadiusVariance, GMfloat)
	GM_DECLARE_PROPERTY(RotatePerSecond, rotatePerSecond, GMfloat)
	GM_DECLARE_PROPERTY(RotatePerSecondVariance, rotatePerSecondVariance, GMfloat)
	GM_DECLARE_PROPERTY(TextureImageData, textureImageData, GMString)
	GM_DECLARE_PROPERTY(TextureFileName, textureFileName, GMString)

public:
	virtual bool registerMeta() override
	{
		GM_META(angle)
		GM_META(angleVariance)
		GM_META(speed)
		GM_META(speedVariance)
		GM_META(duration)
		GM_META_WITH_TYPE(emitterType, GMMetaMemberType::Int)
		GM_META(maxParticles)
		GM_META(sourcePositionx)
		GM_META(sourcePositiony)
		GM_META(sourcePositionVariancex)
		GM_META(sourcePositionVariancey)
		GM_META(particleLifespan)
		GM_META(particleLifespanVariance)
		GM_META(startColorRed)
		GM_META(startColorGreen)
		GM_META(startColorBlue)
		GM_META(startColorAlpha)
		GM_META(startColorVarianceRed)
		GM_META(startColorVarianceGreen)
		GM_META(startColorVarianceBlue)
		GM_META(startColorVarianceAlpha)
		GM_META(finishColorRed)
		GM_META(finishColorGreen)
		GM_META(finishColorBlue)
		GM_META(finishColorAlpha)
		GM_META(finishColorVarianceRed)
		GM_META(finishColorVarianceGreen)
		GM_META(finishColorVarianceBlue)
		GM_META(finishColorVarianceAlpha)
		GM_META(startParticleSize)
		GM_META(startParticleSizeVariance)
		GM_META(finishParticleSize)
		GM_META(finishParticleSizeVariance)
		GM_META(rotationStart)
		GM_META(rotationStartVariance)
		GM_META(rotationEnd)
		GM_META(rotationEndVariance)
		GM_META_WITH_TYPE(positionType, GMMetaMemberType::Int)
		GM_META(gravityx)
		GM_META(gravityy)
		GM_META(radialAcceleration)
		GM_META(radialAccelVariance)
		GM_META(tangentialAcceleration)
		GM_META(tangentialAccelVariance)
		GM_META(minRadius)
		GM_META(minRadiusVariance)
		GM_META(maxRadius)
		GM_META(maxRadiusVariance)
		GM_META(rotatePerSecond)
		GM_META(rotatePerSecondVariance)
		GM_META(textureImageData)
		GM_META(textureFileName)
		return true;
	}
};

GMParticleSystem::GMParticleSystem()
{
	D(d);
	d->emitter.reset(new GMParticleEmitter(this));
}

void GMParticleSystem::setDescription(const GMParticleDescription& desc)
{
	D(d);
	GM_ASSERT(d->emitter);
	d->emitter->setDescription(desc);
	d->textureBuffer = desc.getTextureImageData();
	setParticleModel(createParticleModel(desc));
}

void GMParticleSystem::update(GMDuration dt)
{
	D(d);
	d->emitter->update(dt);
}

void GMParticleSystem::render(const IRenderContext* context)
{
	D(d);
	GM_ASSERT(d->particleModel);
	d->particleModel->render(context);
}

IParticleModel* GMParticleSystem::createParticleModel(const GMParticleDescription& desc)
{
	D(d);
	switch (desc.getParticleModelType())
	{
	case GMParticleModelType::Particle2D:
		return new GMParticleModel_2D(this);
	default:
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Undefined particle model type."));
		return new GMParticleModel_2D(this);
	}
}

GMParticleDescription GMParticleSystem::createParticleDescriptionFromCocos2DPlist(const GMString& content)
{
	GMCocos2DParticleDescriptionProxy proxy;
	GMXML::parsePlist(content, proxy);
	GMParticleDescription desc;

	// 粒子发射器属性
	desc.setEmitterEmitAngle(proxy.getAngle());
	desc.setEmitterEmitAngleV(proxy.getAngleVariance());

	desc.setEmitterEmitSpeed(proxy.getSpeed());
	desc.setEmitterEmitSpeedV(proxy.getSpeedVariance());

	desc.setDuration(proxy.getDuration());

	desc.setEmitterType(proxy.getEmitterType());
	desc.setParticleCount(proxy.getMaxParticles());
	desc.setEmitterPosition(GMVec3(proxy.getSourcePositionx(), proxy.getSourcePositiony(), Z));
	desc.setEmitterPositionV(GMVec3(proxy.getSourcePositionVariancex(), proxy.getSourcePositionVariancey(), Z));
	desc.setEmitRate(proxy.getMaxParticles() / proxy.getParticleLifespan());

	// 粒子属性
	desc.setLife(proxy.getParticleLifespan());
	desc.setLifeV(proxy.getParticleLifespanVariance());

	desc.setBeginColor(GMVec4(
		proxy.getStartColorRed(),
		proxy.getStartColorGreen(),
		proxy.getStartColorBlue(),
		proxy.getStartColorAlpha())
	);
	desc.setBeginColorV(GMVec4(
		proxy.getStartColorVarianceRed(),
		proxy.getStartColorVarianceGreen(),
		proxy.getStartColorVarianceBlue(),
		proxy.getStartColorVarianceAlpha())
	);
	desc.setEndColor(GMVec4(
		proxy.getFinishColorRed(),
		proxy.getFinishColorGreen(),
		proxy.getFinishColorBlue(),
		proxy.getFinishColorAlpha())
	);
	desc.setEndColorV(GMVec4(
		proxy.getFinishColorVarianceRed(),
		proxy.getFinishColorVarianceGreen(),
		proxy.getFinishColorVarianceBlue(),
		proxy.getFinishColorVarianceAlpha())
	);

	desc.setBeginSize(proxy.getStartParticleSize());
	desc.setBeginSizeV(proxy.getStartParticleSizeVariance());
	desc.setEndSize(proxy.getFinishParticleSize());
	desc.setEndSizeV(proxy.getFinishParticleSizeVariance());

	desc.setMotionMode(proxy.getPositionType());
	desc.getGravityMode().setGravity(GMVec3(proxy.getGravityx(), proxy.getGravityy(), Z));
	desc.getGravityMode().setRadialAcceleration(proxy.getRadialAcceleration());
	desc.getGravityMode().setRadialAccelerationV(proxy.getRadialAccelVariance());
	desc.getGravityMode().setTangentialAcceleration(proxy.getTangentialAcceleration());
	desc.getGravityMode().setTangentialAccelerationV(proxy.getTangentialAccelVariance());

	desc.getRadiusMode().setBeginRadius(proxy.getMaxRadius());
	desc.getRadiusMode().setBeginRadiusV(proxy.getMaxRadiusVariance());
	desc.getRadiusMode().setEndRadius(proxy.getMinRadius());
	desc.getRadiusMode().setEndRadiusV(proxy.getMinRadiusVariance());
	desc.getRadiusMode().setSpinPerSecond(proxy.getRotatePerSecond());
	desc.getRadiusMode().setSpinPerSecondV(proxy.getRotatePerSecondVariance());

	GMBuffer buf;
	std::string imageData = proxy.getTextureImageData().toStdString();

	// 如果存在Base64形式的粒子纹理，优先处理
	if (!imageData.empty())
	{
		buf.buffer = (GMbyte*)(imageData.data());
		buf.size = imageData.length() + 1; // \0

		// Cocos2D 的纹理数据压缩过了，所以要解压
		enum
		{
			TextureSizeHint = 1024 * 64
		};
		auto base64Decoded = GMConvertion::fromBase64(buf);
		GMBuffer inflated;
		GMsize_t imgSize;
		if (GMZip::inflateMemory(base64Decoded, inflated, imgSize, TextureSizeHint) == GMZip::Ok)
			desc.setTextureImageData(std::move(inflated));
	}
	else if (!proxy.getTextureFileName().isEmpty())
	{
		GMBuffer buf;
		if (GM.getGamePackageManager()->readFile(GMPackageIndex::Particle, proxy.getTextureFileName(), &buf))
			desc.setTextureImageData(std::move(buf));
	}

	// Cocos2D 使用二维游戏对象渲染粒子
	desc.setParticleModelType(GMParticleModelType::Particle2D);
	return desc;
}

GMParticleEmitter::GMParticleEmitter(GMParticleSystem* system)
{
	D(d);
	d->system = system;
}

void GMParticleEmitter::setDescription(const GMParticleDescription& desc)
{
	setEmitPosition(desc.getEmitterPosition());
	setEmitPositionV(desc.getEmitterPositionV());

	setEmitAngle(desc.getEmitterEmitAngle());
	setEmitAngleV(desc.getEmitterEmitAngleV());

	setEmitSpeed(desc.getEmitterEmitSpeed());
	setEmitSpeedV(desc.getEmitterEmitSpeedV());

	setEmitRate(desc.getEmitRate());
	setDuration(desc.getDuration());
	setParticleCount(desc.getParticleCount());

	GMParticleEffect* eff = nullptr;
	if (desc.getEmitterType() == GMParticleEmitterType::Gravity)
	{
		eff = new GMGravityParticleEffect();
	}
	else
	{
		GM_ASSERT(desc.getEmitterType() == GMParticleEmitterType::Radius);
		eff = new GMRadialParticleEffect();
	}

	eff->setParticleDescription(desc);
	setParticleEffect(eff);
}

void GMParticleEmitter::addParticle()
{
	D(d);
	if (d->particles.size() < static_cast<GMsize_t>(getParticleCount()))
	{
		GMParticlePool& pool = d->system->getParticleSystemManager()->getPool();
		GMParticle* particle = pool.alloc();
		if (particle)
		{
			d->particles.push_back(particle);
			d->effect->initParticle(this, particle);
		}
	}
}

void GMParticleEmitter::emitParticles(GMDuration dt)
{
	D(d);
	GMfloat dtEmit = 1.f / d->emitRate;
	if (d->particles.size() < static_cast<GMsize_t>(d->particleCount))
	{
		d->emitCounter += dt;
	}

	while (d->particles.size() < static_cast<GMsize_t>(d->particleCount) && d->emitCounter > 0)
	{
		// 发射待发射的粒子
		addParticle();
		d->emitCounter -= dtEmit;
	}

	d->elapsed += dt;
	// 如果duration是个负数，表示永远发射
	if (d->duration >= 0 && d->duration < d->elapsed)
	{
		d->elapsed = 0;
		stopEmit();
	}
}

void GMParticleEmitter::setParticleEffect(GMParticleEffect* effect)
{
	D(d);
	d->effect.reset(effect);
}

void GMParticleEmitter::update(GMDuration dt)
{
	D(d);
	if (d->canEmit)
	{
		emitParticles(dt);
		d->effect->update(this, dt);
	}
}

void GMParticleEmitter::startEmit()
{
	D(d);
	d->canEmit = true;
}

void GMParticleEmitter::stopEmit()
{
	D(d);
	d->canEmit = false;

	auto& pool = getParticleSystem()->getParticleSystemManager()->getPool();
	for (auto particle : d->particles)
	{
		pool.free(particle);
	}
	d->particles.clear();
}

void GMParticlePool::init(GMsize_t count)
{
	D(d);
	d->capacity = count;
	d->particlePool.reserve(count);
	d->unused.reserve(count);
	for (GMsize_t i = 0; i < count; ++i)
	{
		GMParticle* p = new GMParticle();
		d->particlePool.emplace_back(GMOwnedPtr<GMParticle>(p));
		d->unused.push_back(p);
	}
}

void GMParticlePool::freeAll()
{
	D(d);
	GMClearSTLContainer(d->particlePool);
}

GMParticle* GMParticlePool::alloc()
{
	D(d);
	if (d->index < d->unused.size())
		return d->unused[d->index++];

	expand(d->capacity);
	return d->unused[d->index++];
}

void GMParticlePool::free(GMParticle* particle)
{
	D(d);
	d->unused[--d->index] = particle;
}

GMsize_t GMParticlePool::getCapacity() GM_NOEXCEPT
{
	D(d);
	return d->particlePool.size();
}

void GMParticlePool::expand(GMsize_t size)
{
	D(d);
	GMsize_t currentSize = d->particlePool.size();
	GMsize_t targetSize = currentSize + d->capacity;
	d->particlePool.resize(targetSize);
	d->unused.resize(targetSize);
	for (GMsize_t i = currentSize; i < targetSize; ++i)
	{
		GMParticle* p = new GMParticle();
		d->particlePool[i] = GMOwnedPtr<GMParticle>(p);
		d->unused[i] = p;
	}
	d->capacity = targetSize;
}

void GMParticleEffect::setParticleDescription(const GMParticleDescription& desc)
{
	setLife(desc.getLife());
	setLifeV(desc.getLifeV());

	setBeginColor(desc.getBeginColor());
	setBeginColorV(desc.getBeginColorV());
	setEndColor(desc.getEndColor());
	setEndColorV(desc.getEndColorV());

	setBeginSize(desc.getBeginSize());
	setBeginSizeV(desc.getBeginSizeV());
	setEndSize(desc.getEndSize());
	setEndSizeV(desc.getEndSizeV());

	setBeginSpin(desc.getBeginSpin());
	setBeginSpinV(desc.getBeginSpinV());
	setEndSpin(desc.getEndSpin());
	setEndSpinV(desc.getEndSpinV());

	setMotionMode(desc.getMotionMode());
	setGravityMode(desc.getGravityMode());
	setRadiusMode(desc.getRadiusMode());
}

void GMParticleEffect::initParticle(GMParticleEmitter* emitter, GMParticle* particle)
{
	D(d);
	GMVec3 randomPos(GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f));
	particle->setPosition(emitter->getEmitPosition() + emitter->getEmitPositionV() * randomPos);

	particle->setStartPosition(emitter->getEmitPosition());
	particle->setChangePosition(particle->getPosition());
	particle->setRemainingLife(Max(.1f, getLife() + getLifeV() * GMRandomMt19937::random_real(-1.f, 1.f)));

	GMVec4 randomBeginColor(GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f));
	GMVec4 randomEndColor(GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f), GMRandomMt19937::random_real(-1.f, 1.f));

	GMVec4 beginColor, endColor;
	beginColor = Clamp(getBeginColor() + getBeginColorV() * randomBeginColor, 0, 1);
	endColor = Clamp(getEndColor() + getEndColorV() * randomEndColor, 0, 1);

	GMfloat remainingLifeRev = 1.f / (particle->getRemainingLife());
	particle->setColor(beginColor);
	particle->setDeltaColor((endColor - beginColor) * remainingLifeRev);

	GMfloat beginSize = Max(0, getBeginSize() + getBeginSizeV() * GMRandomMt19937::random_real(-1.f, 1.f));
	GMfloat endSize = Max(0, getEndSize() + getEndSize() * GMRandomMt19937::random_real(-1.f, 1.f));
	particle->setSize(beginSize);
	particle->setDeltaSize((endSize - beginSize) / particle->getRemainingLife());

	GMfloat beginSpin = Radians(Max(0, getBeginSpin() + getBeginSpinV() * GMRandomMt19937::random_real(-1.f, 1.f)));
	GMfloat endSpin = Radians(Max(0, getEndSpin() + getEndSpin() * GMRandomMt19937::random_real(-1.f, 1.f)));
	particle->setRotation(beginSpin);
	particle->setDeltaRotation((endSpin - beginSpin) * remainingLifeRev);
}

GMParticleSystemManager::GMParticleSystemManager(const IRenderContext* context, GMsize_t particleCountHint)
{
	D(d);
	d->context = context;
	//事先分配若干个粒子
	d->pool.init(particleCountHint); 
}

void GMParticleSystemManager::addParticleSystem(AUTORELEASE GMParticleSystem* ps)
{
	D(d);
	ps->setParticleSystemManager(this);
	d->particleSystems.push_back(GMOwnedPtr<GMParticleSystem>(ps));
}

void GMParticleSystemManager::render()
{
	D(d);
	for (decltype(auto) ps : d->particleSystems)
	{
		ps->render(d->context);
	}
}

void GMParticleSystemManager::update(GMDuration dt)
{
	// TODO 考虑成异步
	D(d);
	for (decltype(auto) system : d->particleSystems)
	{
		system->update(dt);
	}
}