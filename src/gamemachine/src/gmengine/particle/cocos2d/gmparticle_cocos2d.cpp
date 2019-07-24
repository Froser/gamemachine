#include "stdafx.h"
#include "gmparticle_cocos2d.h"
#include "gmparticleeffects_cocos2d.h"
#include "gmparticlemodel_cocos2d.h"
#include <gmxml.h>
#include <random>
#include "foundation/gamemachine.h"
#include <zlib.h>
#include "foundation/gmasync.h"

BEGIN_NS

namespace
{
	const GMParticleDescription_Cocos2D* toCocos2DDesc(GMParticleDescription desc)
	{
		return static_cast<const GMParticleDescription_Cocos2D*>(desc);
	}
}

#define Z 0

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
	GM_DECLARE_PROPERTY(Angle, angle)
	GM_DECLARE_PROPERTY(AngleVariance, angleVariance)
	GM_DECLARE_PROPERTY(Speed, speed)
	GM_DECLARE_PROPERTY(SpeedVariance, speedVariance)
	GM_DECLARE_PROPERTY(Duration, duration)
	GM_DECLARE_PROPERTY(EmitterType, emitterType)
	GM_DECLARE_PROPERTY(MaxParticles, maxParticles)
	GM_DECLARE_PROPERTY(SourcePositionx, sourcePositionx)
	GM_DECLARE_PROPERTY(SourcePositiony, sourcePositiony)
	GM_DECLARE_PROPERTY(SourcePositionVariancex, sourcePositionVariancex)
	GM_DECLARE_PROPERTY(SourcePositionVariancey, sourcePositionVariancey)
	GM_DECLARE_PROPERTY(ParticleLifespan, particleLifespan)
	GM_DECLARE_PROPERTY(ParticleLifespanVariance, particleLifespanVariance)
	GM_DECLARE_PROPERTY(StartColorRed, startColorRed)
	GM_DECLARE_PROPERTY(StartColorGreen, startColorGreen)
	GM_DECLARE_PROPERTY(StartColorBlue, startColorBlue)
	GM_DECLARE_PROPERTY(StartColorAlpha, startColorAlpha)
	GM_DECLARE_PROPERTY(StartColorVarianceRed, startColorVarianceRed)
	GM_DECLARE_PROPERTY(StartColorVarianceGreen, startColorVarianceGreen)
	GM_DECLARE_PROPERTY(StartColorVarianceBlue, startColorVarianceBlue)
	GM_DECLARE_PROPERTY(StartColorVarianceAlpha, startColorVarianceAlpha)
	GM_DECLARE_PROPERTY(FinishColorRed, finishColorRed)
	GM_DECLARE_PROPERTY(FinishColorGreen, finishColorGreen)
	GM_DECLARE_PROPERTY(FinishColorBlue, finishColorBlue)
	GM_DECLARE_PROPERTY(FinishColorAlpha, finishColorAlpha)
	GM_DECLARE_PROPERTY(FinishColorVarianceRed, finishColorVarianceRed)
	GM_DECLARE_PROPERTY(FinishColorVarianceGreen, finishColorVarianceGreen)
	GM_DECLARE_PROPERTY(FinishColorVarianceBlue, finishColorVarianceBlue)
	GM_DECLARE_PROPERTY(FinishColorVarianceAlpha, finishColorVarianceAlpha)
	GM_DECLARE_PROPERTY(StartParticleSize, startParticleSize)
	GM_DECLARE_PROPERTY(StartParticleSizeVariance, startParticleSizeVariance)
	GM_DECLARE_PROPERTY(FinishParticleSize, finishParticleSize)
	GM_DECLARE_PROPERTY(FinishParticleSizeVariance, finishParticleSizeVariance)
	GM_DECLARE_PROPERTY(RotationStart, rotationStart)
	GM_DECLARE_PROPERTY(RotationStartVariance, rotationStartVariance)
	GM_DECLARE_PROPERTY(RotationEnd, rotationEnd)
	GM_DECLARE_PROPERTY(RotationEndVariance, rotationEndVariance)
	GM_DECLARE_PROPERTY(PositionType, positionType)
	GM_DECLARE_PROPERTY(Gravityx, gravityx)
	GM_DECLARE_PROPERTY(Gravityy, gravityy)
	GM_DECLARE_PROPERTY(RadialAcceleration, radialAcceleration)
	GM_DECLARE_PROPERTY(RadialAccelVariance, radialAccelVariance)
	GM_DECLARE_PROPERTY(TangentialAcceleration, tangentialAcceleration)
	GM_DECLARE_PROPERTY(TangentialAccelVariance, tangentialAccelVariance)
	GM_DECLARE_PROPERTY(MinRadius, minRadius)
	GM_DECLARE_PROPERTY(MinRadiusVariance, minRadiusVariance)
	GM_DECLARE_PROPERTY(MaxRadius, maxRadius)
	GM_DECLARE_PROPERTY(MaxRadiusVariance, maxRadiusVariance)
	GM_DECLARE_PROPERTY(RotatePerSecond, rotatePerSecond)
	GM_DECLARE_PROPERTY(RotatePerSecondVariance, rotatePerSecondVariance)
	GM_DECLARE_PROPERTY(TextureImageData, textureImageData)
	GM_DECLARE_PROPERTY(TextureFileName, textureFileName)

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

GMParticleSystem_Cocos2D::GMParticleSystem_Cocos2D(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->emitter.reset(new GMParticleEmitter_Cocos2D(this));
}


GMParticleSystem_Cocos2D::~GMParticleSystem_Cocos2D()
{
	D(d);
	if (d->particleModel)
		d->particleModel->destroy();
}

void GMParticleSystem_Cocos2D::setDescription(const GMParticleDescription_Cocos2D& desc)
{
	D(d);
	GM_ASSERT(d->emitter);
	d->emitter->setDescription(&desc);
	d->textureBuffer = desc.getTextureImageData();

	GMParticleModel_Cocos2D* particleModel = createParticleModel(desc);
	GM_ASSERT(particleModel);
	particleModel->init();
	setParticleModel(particleModel);
}

void GMParticleSystem_Cocos2D::update(GMDuration dt)
{
	D(d);
	d->emitter->update(dt);
}

void GMParticleSystem_Cocos2D::render()
{
	D(d);
	GM_ASSERT(d->particleModel);
	d->particleModel->render();
}

void GMParticleSystem_Cocos2D::setParticleSystemManager(IParticleSystemManager* manager)
{
	D(d);
	d->manager = manager;
}


GMParticleEmitter_Cocos2D* GMParticleSystem_Cocos2D::getEmitter()
{
	D(d);
	return d->emitter.get();
}

void GMParticleSystem_Cocos2D::setParticleModel(AUTORELEASE GMParticleModel_Cocos2D* particleModel)
{
	D(d);
	if (d->particleModel && d->particleModel != particleModel)
		d->particleModel->destroy();
	d->particleModel = particleModel;
}

const IRenderContext* gm::GMParticleSystem_Cocos2D::getContext()
{
	D(d);
	return d->context;
}

GMParticleModel_Cocos2D* GMParticleSystem_Cocos2D::createParticleModel(const GMParticleDescription_Cocos2D& desc)
{
	D(d);
	switch (desc.getParticleModelType())
	{
	case GMParticleModelType::Particle2D:
		return new GMParticleModel_2D(this);
	case GMParticleModelType::Particle3D:
		return new GMParticleModel_3D(this);
	default:
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Undefined particle model type."));
		return new GMParticleModel_2D(this);
	}
}

GMParticleDescription_Cocos2D GMParticleSystem_Cocos2D::createParticleDescriptionFromCocos2DPlist(const GMString& content, GMParticleModelType modelType)
{
	GMCocos2DParticleDescriptionProxy proxy;
	GMXML::parsePlist(content, proxy);
	GMParticleDescription_Cocos2D desc;

	// 粒子发射器属性
	// Cocos2D的发射角度，方向向量为(-1, 0, 0)，GameMachine采用(0, 1, 0)，因此要减去90度。
	desc.setEmitterEmitAngle(proxy.getAngle() - 90);
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
	desc.getGravityMode().setGravity(GMVec3(proxy.getGravityx(), -proxy.getGravityy(), Z)); // Cocos2D重力为正，但是是朝GameMachine坐标系负方向
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
		buf = GMBuffer::createBufferView((GMbyte*)(imageData.data()), imageData.length() + 1);

		// Cocos2D 的纹理数据压缩过了，所以要解压
		enum
		{
			TextureSizeHint = 1024 * 64
		};
		auto base64Decoded = GMConvertion::fromBase64(buf);
		GMBuffer inflated;
		if (GMZip::inflate(base64Decoded, inflated, TextureSizeHint) == GMZip::Ok)
			desc.setTextureImageData(std::move(inflated));
	}
	else if (!proxy.getTextureFileName().isEmpty())
	{
		GMBuffer buf;
		if (GM.getGamePackageManager()->readFile(GMPackageIndex::Particle, proxy.getTextureFileName(), &buf))
			desc.setTextureImageData(std::move(buf));
	}

	desc.setParticleModelType(modelType);
	return desc;
}

void GMParticleSystem_Cocos2D::createCocos2DParticleSystem(
	const IRenderContext* context,
	const GMString& filename,
	GMParticleModelType modelType,
	OUT GMParticleSystem_Cocos2D** particleSystem,
	std::function<void(GMParticleDescription_Cocos2D&)> descriptionCallback
)
{
	if (particleSystem)
	{
		GMBuffer buf;
		GM.getGamePackageManager()->readFile(GMPackageIndex::Particle, filename, &buf);
		buf.convertToStringBuffer();

		createCocos2DParticleSystem(context, buf, modelType, particleSystem, descriptionCallback);
	}
}

void GMParticleSystem_Cocos2D::createCocos2DParticleSystem(
	const IRenderContext* context,
	const GMBuffer& buffer,
	GMParticleModelType modelType,
	OUT GMParticleSystem_Cocos2D** particleSystem,
	std::function<void(GMParticleDescription_Cocos2D&)> descriptionCallback /*= std::function<void(GMParticleDescription&)>() */)
{
	if (particleSystem)
	{
		*particleSystem = new GMParticleSystem_Cocos2D(context);
		GMParticleDescription_Cocos2D description = GMParticleSystem_Cocos2D::createParticleDescriptionFromCocos2DPlist(gm::GMString((const char*)buffer.getData()), modelType);
		if (descriptionCallback)
			descriptionCallback(description);

		(*particleSystem)->setDescription(description);
	}
}

GMParticleEmitter_Cocos2D::GMParticleEmitter_Cocos2D(GMParticleSystem_Cocos2D* system)
{
	D(d);
	d->system = system;
}


GMParticleEmitter_Cocos2D::~GMParticleEmitter_Cocos2D()
{
	D(d);
	if (d->effect)
		d->effect->destroy();
}

void GMParticleEmitter_Cocos2D::setDescription(GMParticleDescription desc)
{
	const GMParticleDescription_Cocos2D* cocos2DDesc = toCocos2DDesc(desc);

	setEmitPosition(cocos2DDesc->getEmitterPosition());
	setEmitPositionV(cocos2DDesc->getEmitterPositionV());

	setEmitAngle(cocos2DDesc->getEmitterEmitAngle());
	setEmitAngleV(cocos2DDesc->getEmitterEmitAngleV());

	setEmitSpeed(cocos2DDesc->getEmitterEmitSpeed());
	setEmitSpeedV(cocos2DDesc->getEmitterEmitSpeedV());

	setEmitRate(cocos2DDesc->getEmitRate());
	setDuration(cocos2DDesc->getDuration());
	setParticleCount(cocos2DDesc->getParticleCount());

	GMParticleEffect_Cocos2D* eff = nullptr;
	if (cocos2DDesc->getEmitterType() == GMParticleEmitterType::Gravity)
	{
		eff = new GMGravityParticleEffect_Cocos2D(this);
	}
	else
	{
		GM_ASSERT(cocos2DDesc->getEmitterType() == GMParticleEmitterType::Radius);
		eff = new GMRadialParticleEffect_Cocos2D(this);
	}
	eff->init();
	eff->setParticleDescription(cocos2DDesc);
	setParticleEffect(eff);
}

void GMParticleEmitter_Cocos2D::addParticle()
{
	D(d);
	if (d->particles.size() < static_cast<GMsize_t>(getParticleCount()))
	{
		GMParticle_Cocos2D particle;
		d->effect->initParticle(&particle);
		d->particles.push_back(particle);
	}
}

void GMParticleEmitter_Cocos2D::emitParticles(GMDuration dt)
{
	D(d);
	// 如果emitRate为0，则要调用emitOnce手动触发
	if (!isinf(d->emitRate))
	{
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
}

void GMParticleEmitter_Cocos2D::emitOnce()
{
	D(d);
	while (d->particles.size() < static_cast<GMsize_t>(d->particleCount))
	{
		// 发射待发射的粒子
		addParticle();
	}
}

void GMParticleEmitter_Cocos2D::setParticleEffect(GMParticleEffect_Cocos2D* effect)
{
	D(d);
	if (d->effect && d->effect != effect)
		d->effect->destroy();
	d->effect = effect;
}

void GMParticleEmitter_Cocos2D::update(GMDuration dt)
{
	D(d);
	if (d->canEmit)
	{
		emitParticles(dt);
		d->effect->update(dt);
	}
}

void GMParticleEmitter_Cocos2D::startEmit()
{
	D(d);
	d->canEmit = true;
}

void GMParticleEmitter_Cocos2D::stopEmit()
{
	D(d);
	d->canEmit = false;
	d->particles.clear();
}


GM_PRIVATE_OBJECT_ALIGNED(GMParticleGravityMode)
{
	GMVec3 gravity = Zero<GMVec3>();
	GMfloat tangentialAcceleration = 0;
	GMfloat tangentialAccelerationV = 0;
	GMfloat radialAcceleration = 0;
	GMfloat radialAccelerationV = 0;
};

GMParticleGravityMode::GMParticleGravityMode()
{
	GM_CREATE_DATA(GMParticleGravityMode);
}

GM_PRIVATE_OBJECT_ALIGNED(GMParticleRadiusMode)
{
	GMfloat beginRadius = 0;
	GMfloat beginRadiusV = 0;
	GMfloat endRadius = 0;
	GMfloat endRadiusV = 0;
	GMfloat spinPerSecond = 0;
	GMfloat spinPerSecondV = 0;
};

GMParticleRadiusMode::GMParticleRadiusMode()
{
	GM_CREATE_DATA(GMParticleRadiusMode);
}

END_NS