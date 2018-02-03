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

	if (d->model)
	{
		for (auto& component : d->model->getMesh()->getComponents())
		{
			GMShader& shader = component->getShader();
			attachEvent(shader, GM_SET_PROPERTY_EVENT_ENUM(Blend), onShaderSetBlend);
			shader.emitEvent(GM_SET_PROPERTY_EVENT_ENUM(Blend));
		}
	}
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
	return d->canDeferredRendering;
}

void GMGameObject::onShaderSetBlend(GMObject* sender, GMObject* receiver)
{
	GMGameObject* gameObject = gmobject_cast<GMGameObject*>(receiver);
	GMShader* shader = gmobject_cast<GMShader*>(sender);
	gameObject->data()->canDeferredRendering = !shader->getBlend();
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

	GMMesh* mesh = getModel()->getMesh();
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

// 天空
static GMVec2 uvs[24] = {
	GMVec2(0, 0),
	GMVec2(0, 1),
	GMVec2(1, 1),
	GMVec2(1, 0),

	GMVec2(0, 0),
	GMVec2(0, 1),
	GMVec2(1, 1),
	GMVec2(1, 0),

	GMVec2(0, 0),
	GMVec2(0, 1),
	GMVec2(1, 1),
	GMVec2(1, 0),

	GMVec2(0, 0),
	GMVec2(0, 1),
	GMVec2(1, 1),
	GMVec2(1, 0),

	GMVec2(0, 0),
	GMVec2(0, 1),
	GMVec2(1, 1),
	GMVec2(1, 0),

	GMVec2(0, 0),
	GMVec2(0, 1),
	GMVec2(1, 1),
	GMVec2(1, 0),
};

GMSkyGameObject::GMSkyGameObject(const GMShader& shader, const GMVec3& min, const GMVec3& max)
{
	D(d);
	d->shader = shader;
	d->min = min;
	d->max = max;

	GMModel* obj = nullptr;
	createSkyBox(&obj);
	GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, obj);
	setModel(asset);
}

GMSkyGameObject::~GMSkyGameObject()
{
	D(d);
	GMModel* m = getModel();
	if (m)
		delete m;
}

void GMSkyGameObject::createSkyBox(OUT GMModel** obj)
{
	D(d);
	GMFloat4 f4_min, f4_max;
	d->min.loadFloat4(f4_min);
	d->max.loadFloat4(f4_max);
	GMVec3 vertices[] = {
		//Front
		GMVec3(f4_min[0], f4_max[1], f4_max[2]),
		GMVec3(f4_min[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_max[1], f4_max[2]),

		//Back
		GMVec3(f4_min[0], f4_max[1], f4_min[2]),
		GMVec3(f4_min[0], f4_min[1], f4_min[2]),
		GMVec3(f4_max[0], f4_min[1], f4_min[2]),
		GMVec3(f4_max[0], f4_max[1], f4_min[2]),

		//Left
		GMVec3(f4_min[0], f4_max[1], f4_min[2]),
		GMVec3(f4_min[0], f4_max[1], f4_max[2]),
		GMVec3(f4_min[0], f4_min[1], f4_max[2]),
		GMVec3(f4_min[0], f4_min[1], f4_min[2]),

		//Right
		GMVec3(f4_max[0], f4_max[1], f4_min[2]),
		GMVec3(f4_max[0], f4_max[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_min[2]),

		//Up
		GMVec3(f4_min[0], f4_max[1], f4_min[2]),
		GMVec3(f4_min[0], f4_max[1], f4_max[2]),
		GMVec3(f4_max[0], f4_max[1], f4_max[2]),
		GMVec3(f4_max[0], f4_max[1], f4_min[2]),

		//Down
		GMVec3(f4_min[0], f4_min[1], f4_min[2]),
		GMVec3(f4_min[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_min[2]),
	};

	// Scaling surface
	const GMint SCALING = 2;
	GMVec3 center = (d->min + d->max) / 2.f;
	GMMat4 transScale = Scale(GMVec3(SCALING, 1, SCALING));
	for (GMuint i = 0; i < 20; i++)
	{
		GMMat4 transRestore = Translate(center);
		GMMat4 transMoveToAxisOrigin = Translate(-center);
		GMMat4 transFinal = transRestore * transScale * transMoveToAxisOrigin;

		GMVec4 pt = GMVec4(vertices[i], 1) * transFinal;
		vertices[i] = GMVec3(pt);
	}

	GMModel* model = new GMModel();
	*obj = model;

	GMComponent* component = new GMComponent(model->getMesh());
	component->setShader(d->shader);

	for (GMuint i = 0; i < 6; i++)
	{
		component->beginFace();
		component->vertex(vertices[i * 4].getX(), vertices[i * 4].getY(), vertices[i * 4].getZ());
		component->vertex(vertices[i * 4 + 1].getX(), vertices[i * 4 + 1].getY(), vertices[i * 4 + 1].getZ());
		component->vertex(vertices[i * 4 + 2].getX(), vertices[i * 4 + 2].getY(), vertices[i * 4 + 2].getZ());
		component->vertex(vertices[i * 4 + 3].getX(), vertices[i * 4 + 3].getY(), vertices[i * 4 + 3].getZ());
		component->uv(uvs[i * 4].getX(), uvs[i * 4].getY());
		component->uv(uvs[i * 4 + 1].getX(), uvs[i * 4 + 1].getY());
		component->uv(uvs[i * 4 + 2].getX(), uvs[i * 4 + 2].getY());
		component->uv(uvs[i * 4 + 3].getX(), uvs[i * 4 + 3].getY());
		component->endFace();
	}
}

//////////////////////////////////////////////////////////////////////////
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
	GMMesh* mesh = model->getMesh();
	GMComponent* component = new GMComponent(mesh);
	component->getShader().getTexture().getTextureFrames(GMTextureType::CUBEMAP, 0).addFrame(texture);
	for (GMuint i = 0; i < 12; i++)
	{
		component->beginFace();
		component->vertex(vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2]);
		component->vertex(vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5]);
		component->vertex(vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8]);
		component->normal(vertices[i * 9 + 0], vertices[i * 9 + 1], vertices[i * 9 + 2]);
		component->normal(vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5]);
		component->normal(vertices[i * 9 + 6], vertices[i * 9 + 7], vertices[i * 9 + 8]);
		component->endFace();
	}

	setModel(GMAssets::createIsolatedAsset(GMAssetType::Model, model));
}

bool GMCubeMapGameObject::canDeferredRendering()
{
	return false;
}
