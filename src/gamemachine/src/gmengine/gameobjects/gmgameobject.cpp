#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gamemachine.h"
#include "gmassets.h"

GMGameObject::GMGameObject(GMAsset asset)
	: GMGameObject()
{
	addModel(asset);
	updateMatrix();
}

GMGameObject::~GMGameObject()
{
	D(d);
	GM_delete(d->physics);
}

void GMGameObject::addModel(GMAsset asset)
{
	D(d);
	GMModel* model = GMAssets::getModel(asset);
	GM_ASSERT(model);
	d->models.push_back(model);
}

GMModels& GMGameObject::getModels()
{
	D(d);
	return d->models;
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

void GMGameObject::draw()
{
	GMModels models = getModels();
	for (auto& model : models)
	{
		model->getPainter()->draw(this);
	}
}

bool GMGameObject::canDeferredRendering()
{
	D(d);
	if (d->forceDisableDeferredRendering)
		return false;

	return d->canDeferredRendering;
}

GMCubeMapGameObject::GMCubeMapGameObject(ITexture* texture)
{
	createCubeMap(texture);
}

GMCubeMapGameObject::~GMCubeMapGameObject()
{
	deactivate();
	GM_delete(getModels());
}

void GMCubeMapGameObject::deactivate()
{
	GM.getGraphicEngine()->update(GMUpdateDataType::TurnOffCubeMap);
}

void GMCubeMapGameObject::createCubeMap(ITexture* texture)
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
	model->getShader().getTexture().getTextureFrames(GMTextureType::CUBEMAP, 0).addFrame(texture);
	GMMesh* mesh = new GMMesh(model);
	for (GMuint i = 0; i < 12; i++)
	{
		mesh->beginFace();
		mesh->vertex(vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2]);
		mesh->vertex(vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5]);
		mesh->vertex(vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8]);
		mesh->normal(vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2]);
		mesh->normal(vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5]);
		mesh->normal(vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8]);
		mesh->endFace();
	}

	addModel(GMAssets::createIsolatedAsset(GMAssetType::Model, model));
}

bool GMCubeMapGameObject::canDeferredRendering()
{
	return false;
}
