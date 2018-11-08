#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gamemachine.h"
#include "gmassets.h"

GMGameObject::GMGameObject(GMAsset asset)
	: GMGameObject()
{
	setAsset(asset);
	updateTransformMatrix();
}

void GMGameObject::setAsset(GMAsset asset)
{
	D(d);
	if (!asset.getScene())
	{
		if (!asset.getModel())
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

void GMGameObject::foreach(std::function<void(GMModel*)> cb)
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

void GMGameObject::draw()
{
	foreach([this](GMModel* m) {
		drawModel(getContext(), m);
	});
	endDraw();
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
