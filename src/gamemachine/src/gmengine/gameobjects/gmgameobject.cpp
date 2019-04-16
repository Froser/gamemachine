#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gamemachine.h"
#include "gmassets.h"

namespace
{
	void calculateAABB(bool indexMode, GMPart* part, REF GMVec3& min, REF GMVec3& max)
	{
		if (!indexMode)
		{
			for (const auto& v : part->vertices())
			{
				GMVec3 cmp(v.positions[0], v.positions[1], v.positions[2]);
				min = MinComponent(cmp, min);
				max = MaxComponent(cmp, max);
			}
		}
		else
		{
			const GMVertices& v = part->vertices();
			for (const auto& i : part->indices())
			{
				GMVec3 cmp(v[i].positions[0], v[i].positions[1], v[i].positions[2]);
				min = MinComponent(cmp, min);
				max = MaxComponent(cmp, max);
			}
		}
	}

	bool isInsideCameraFrustum(GMCamera* camera, GMVec3 (&points)[8])
	{
		GM_ASSERT(camera);
		const GMFrustum& frustum = camera->getFrustum();
		// 如果min和max出现在了某个平面的两侧，
		GMFrustumPlanes planes;
		frustum.getPlanes(planes);
		return GMCamera::isBoundingBoxInside(planes, points);
	}
}

GMGameObject::GMGameObject(GMAsset asset)
	: GMGameObject()
{
	setAsset(asset);
	updateTransformMatrix();
}

void GMGameObject::setAsset(GMAsset asset)
{
	D(d);
	if (!asset.isScene())
	{
		if (!asset.isModel())
		{
			GM_ASSERT(false);
			gm_error(gm_dbg_wrap("Asset must be a 'scene' or a 'model' type."));
			return;
		}
		else
		{
			asset = GMScene::createSceneFromSingleModel(asset);
		}
	}
	d->asset = asset;
}

GMScene* GMGameObject::getScene()
{
	D(d);
	return d->asset.getScene();
}

GMModel* GMGameObject::getModel()
{
	GMScene* scene = getScene();
	if (!scene)
		return nullptr;

	if (scene->getModels().size() > 1)
	{
		gm_warning(gm_dbg_wrap("Models are more than one. So it will return nothing."));
		return nullptr;
	}

	return scene->getModels()[0].getModel();
}

void GMGameObject::setWorld(GMGameWorld* world)
{
	D(d);
	GM_ASSERT(!d->world);
	d->world = world;
}

GMGameWorld* GMGameObject::getWorld()
{
	D(d);
	return d->world;
}

void GMGameObject::setPhysicsObject(AUTORELEASE GMPhysicsObject* phyObj)
{
	D(d);
	d->physics.reset(phyObj);
	d->physics->setGameObject(this);
}

void GMGameObject::foreachModel(std::function<void(GMModel*)> cb)
{
	GMScene* scene = getScene();
	if (scene)
	{
		for (auto& model : scene->getModels())
		{
			cb(model.getModel());
		}
	}
}

void GMGameObject::onAppendingObjectToWorld()
{
	D(d);
	if (d->cullOption == GMGameObjectCullOption::AABB)
		makeAABB();
}

void GMGameObject::draw()
{
	foreachModel([this](GMModel* m) {
		drawModel(getContext(), m);
	});
	endDraw();
}

void GMGameObject::update(GMDuration dt)
{
	D(d);
	if (d->cullOption == GMGameObjectCullOption::AABB)
	{
		Vector<GMAsset>& models = getScene()->getModels();
		// 计算每个Model的AABB是否与相机Frustum有交集，如果没有，则不进行绘制
		GM_ASSERT(d->cullAABB.size() == models.size());

		static GMVec3 vertices[8];
		for (GMsize_t i = 0; i < d->cullAABB.size(); ++i)
		{
			auto& shader = models[i].getModel()->getShader();

			for (auto j = 0; j < 8; ++j)
			{
				vertices[j] = d->cullAABB[i].points[j] * d->transforms.transformMatrix;
			}

			if (isInsideCameraFrustum(d->cullCamera ? d->cullCamera : &getContext()->getEngine()->getCamera(), vertices))
				shader.setDiscard(false);
			else
				shader.setDiscard(true);
		}
	}
}

bool GMGameObject::canDeferredRendering()
{
	D(d);
	GMScene* scene = getScene();
	if (scene)
	{
		for (decltype(auto) model : scene->getModels())
		{
			if (model.getModel()->getShader().getBlend() == true)
				return false;

			if (model.getModel()->getShader().getVertexColorOp() == GMS_VertexColorOp::Replace)
				return false;

			if (model.getModel()->getType() == GMModelType::Custom)
				return false;
		}
	}
	return true;
}

const IRenderContext* GMGameObject::getContext()
{
	D(d);
	return d->context;
}


bool GMGameObject::isSkeletalObject() const
{
	return false;
}

void GMGameObject::updateTransformMatrix()
{
	D(d);
	d->transforms.transformMatrix = d->transforms.scaling * QuatToMatrix(d->transforms.rotation) * d->transforms.translation;
}

void GMGameObject::setScaling(const GMMat4& scaling)
{
	D(d);
	d->transforms.scaling = scaling;
	if (d->autoUpdateTransformMatrix)
		updateTransformMatrix();
}

void GMGameObject::setTranslation(const GMMat4& translation)
{
	D(d);
	d->transforms.translation = translation;
	if (d->autoUpdateTransformMatrix)
		updateTransformMatrix();
}

void GMGameObject::setRotation(const GMQuat& rotation)
{
	D(d);
	d->transforms.rotation = rotation;
	if (d->autoUpdateTransformMatrix)
		updateTransformMatrix();
}

void GMGameObject::beginUpdateTransform()
{
	setAutoUpdateTransformMatrix(false);
}

void GMGameObject::endUpdateTransform()
{
	setAutoUpdateTransformMatrix(true);
	updateTransformMatrix();
}


void GMGameObject::setCullOption(GMGameObjectCullOption option, GMCamera* camera /*= nullptr*/)
{
	D(d);
	d->cullOption = option;
	// setCullOption一定要在将顶点数据传输到显存之前设置，否则顶点信息将不会在内处中保留，从而无法计算AABB

	d->cullCamera = camera;

	// 如果不需要事先裁剪，重置Shader状态
	if (option == GMGameObjectCullOption::None)
	{
		Vector<GMAsset>& models = getScene()->getModels();
		for (GMsize_t i = 0; i < d->cullAABB.size(); ++i)
		{
			auto& shader = models[i].getModel()->getShader();
			shader.setDiscard(false);
		}
	}
}

void GMGameObject::drawModel(const IRenderContext* context, GMModel* model)
{
	D(d);
	IGraphicEngine* engine = context->getEngine();
	if (model->getShader().getDiscard())
		return;

	ITechnique* technique = engine->getTechnique(model->getType());
	if (technique != d->drawContext.currentTechnique)
	{
		if (d->drawContext.currentTechnique)
			d->drawContext.currentTechnique->endScene();

		technique->beginScene(getScene());
		d->drawContext.currentTechnique = technique;
	}

	technique->beginModel(model, this);
	technique->draw(model);
	technique->endModel();
}

void GMGameObject::endDraw()
{
	D(d);
	if (d->drawContext.currentTechnique)
		d->drawContext.currentTechnique->endScene();

	d->drawContext.currentTechnique = nullptr;
}


void GMGameObject::makeAABB()
{
	D(d);
	typedef std::remove_reference_t<decltype(d->cullAABB[0])> AABB;
	const Vector<GMAsset>& models = getScene()->getModels();
	for (auto modelAsset : models)
	{
		GMModel* model = modelAsset.getModel();
		GM_ASSERT(model->isNeedTransfer());
		const GMParts& parts = model->getParts();
		GMVec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
		GMVec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (auto part : parts)
		{
			calculateAABB(model->getDrawMode() == GMModelDrawMode::Index, part, min, max);
		}

		// 将AABB和gameobject添加到成员数据中
		AABB aabb = {
			GMVec4(min.getX(), min.getY(), min.getZ(), 1),
			GMVec4(min.getX(), min.getY(), max.getZ(), 1),
			GMVec4(min.getX(), max.getY(), max.getZ(), 1),
			GMVec4(max.getX(), max.getY(), max.getZ(), 1),
			GMVec4(min.getX(), max.getY(), min.getZ(), 1),
			GMVec4(max.getX(), min.getY(), max.getZ(), 1),
			GMVec4(max.getX(), max.getY(), min.getZ(), 1),
			GMVec4(max.getX(), min.getY(), min.getZ(), 1),
		};
		d->cullAABB.push_back(aabb);
	}
}

GMCubeMapGameObject::GMCubeMapGameObject(GMTextureAsset texture)
{
	createCubeMap(texture);
}

void GMCubeMapGameObject::deactivate()
{
	D(d);
	GMGameWorld* world = getWorld();
	if (world)
		world->getContext()->getEngine()->update(GMUpdateDataType::TurnOffCubeMap);
}

void GMCubeMapGameObject::createCubeMap(GMTextureAsset texture)
{
	GMfloat vertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	GMfloat v[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
	};

	GMModel* model = new GMModel();
	model->setType(GMModelType::CubeMap);
	model->getShader().getTextureList().getTextureSampler(GMTextureType::CubeMap).addFrame(texture);
	GMPart* part = new GMPart(model);
	for (GMuint32 i = 0; i < 12; i++)
	{
		GMVertex V0 = { { vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2] }, { vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2] } };
		GMVertex V1 = { { vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5] },{ vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5] } };
		GMVertex V2 = { { vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8] },{ vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8] } };
		part->vertex(V0);
		part->vertex(V1);
		part->vertex(V2);
	}

	setAsset(GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, model)));
}

bool GMCubeMapGameObject::canDeferredRendering()
{
	return false;
}
