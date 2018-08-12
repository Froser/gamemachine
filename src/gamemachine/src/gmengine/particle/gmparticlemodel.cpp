#include "stdafx.h"
#include "foundation/gamemachine.h"
#include "gmparticlemodel.h"
#include "foundation/gmasync.h"

GMParticleModel_2D::GMParticleModel_2D(GMParticleSystem* system)
{
	D(d);
	d->system = system;
}

GMGameObject* GMParticleModel_2D::createGameObject(
	const IRenderContext* context
)
{
	D(d);
	GMGameObject* object = new GMGameObject();
	d->particleModel.reset(new GMModel());
	d->particleModel->getShader().setCull(GMS_Cull::None);
	d->particleModel->getShader().setBlend(true);
	d->particleModel->getShader().setNoDepthTest(true);
	d->particleModel->getShader().setBlendFactorSource(GMS_BlendFunc::SourceAlpha);
	d->particleModel->getShader().setBlendFactorDest(GMS_BlendFunc::One);
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
		raw[0] * q,
		raw[1] * q,
		raw[2] * q,
		raw[3] * q,
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
					GM_ASSERT(!d->particleObject->getModels().isEmpty());
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
	auto& particles = d->system->getEmitter()->getParticles();

	// 一个粒子有6个顶点，2个三角形，放入并行计算
	enum { VerticesPerParticle = 6 };
	GMAsync::blockedAsync(
		GMAsync::Async,
		GM.getRunningStates().systemInfo.numberOfProcessors,
		particles.begin(),
		particles.end(),
		[&particles, dataPtr, this](auto begin, auto end) {
		// 计算一下数据偏移
		GMVertex* dataOffset = reinterpret_cast<GMVertex*>(dataPtr) + (begin - particles.begin()) * VerticesPerParticle;
		for (auto iter = begin; iter != end; ++iter)
		{
			GMParticle* particle = *iter;
			GMfloat he = particle->getSize() / 2.f;
			update6Vertices(
				dataOffset,
				particle->getPosition(),
				he,
				particle->getColor(),
				particle->getRotation(),
				GMVec3(0, 0, 1)
			);
			dataOffset += VerticesPerParticle;
		}
	}
	);
}
