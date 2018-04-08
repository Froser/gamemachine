#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdata/glyph/gmglyphmanager.h"
#include "foundation/gamemachine.h"
#include "gmassets.h"

GMGameObject::GMGameObject(GMAsset asset)
	: GMGameObject()
{
	setModel(asset);
	updateMatrix();
}

GMGameObject::~GMGameObject()
{
	D(d);
	GM_delete(d->physics);
}

void GMGameObject::setModel(GMAsset asset)
{
	D(d);
	GMModel* model = GMAssets::getModel(asset);
	GM_ASSERT(model);
	d->model = model;
}

GMModel* GMGameObject::getModel()
{
	D(d);
	return d->model;
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
	GMModel* model = getModel();
	if (model)
		model->getPainter()->draw(this);
}

bool GMGameObject::canDeferredRendering()
{
	D(d);
	if (d->forceDisableDeferredRendering)
		return false;

	return d->canDeferredRendering;
}

//GMEntityObject
GMEntityObject::GMEntityObject(GMAsset asset)
	: GMGameObject(asset)
{
	calc();
}

GMPlane* GMEntityObject::getPlanes()
{
	D(d);
	return d->planes;
}

void GMEntityObject::getBounds(REF GMVec3& mins, REF GMVec3& maxs)
{
	D(d);
	mins = d->mins;
	maxs = d->maxs;
}

void GMEntityObject::calc()
{
	D(d);
	d->mins = GMVec3(999999.f);
	d->maxs = -d->mins;

	GMFloat4 f4_mins, f4_maxs;
	d->mins.loadFloat4(f4_mins);
	d->maxs.loadFloat4(f4_maxs);

	GMMesh* mesh = new GMMesh(getModel());
	GMModel::DataType* vertices = mesh->positions().data();
	GMint sz = mesh->positions().size();
	for (GMint i = 0; i < sz; i += 4)
	{
		for (GMint j = 0; j < 3; j++)
		{
			if (vertices[i + j] < f4_mins[j])
				f4_mins[j] = vertices[i + j];
			if (vertices[i + j] > f4_maxs[j])
				f4_maxs[j] = vertices[i + j];
		}
	}
	d->mins.setFloat4(f4_mins);
	d->maxs.setFloat4(f4_maxs);
	makePlanes();
}

void GMEntityObject::makePlanes()
{
	D(d);
	// 前
	d->planes[0] = GMPlane(GMVec3(0, 0, 1), -d->maxs.getZ());
	// 后
	d->planes[1] = GMPlane(GMVec3(0, 0, -1), d->mins.getZ());
	// 左
	d->planes[2] = GMPlane(GMVec3(-1, 0, 0), d->mins.getX());
	// 右
	d->planes[3] = GMPlane(GMVec3(1, 0, 0), -d->maxs.getX());
	// 上
	d->planes[4] = GMPlane(GMVec3(0, 1, 0), -d->maxs.getY());
	// 下
	d->planes[5] = GMPlane(GMVec3(0, -1, 0), d->mins.getY());
}

GMCubeMapGameObject::GMCubeMapGameObject(ITexture* texture)
{
	createCubeMap(texture);
}

GMCubeMapGameObject::~GMCubeMapGameObject()
{
	deactivate();
	GM_delete(getModel());
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

	setModel(GMAssets::createIsolatedAsset(GMAssetType::Model, model));
}

bool GMCubeMapGameObject::canDeferredRendering()
{
	return false;
}
