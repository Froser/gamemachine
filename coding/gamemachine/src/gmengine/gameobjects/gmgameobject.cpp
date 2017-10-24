#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmgl/gmglglyphmanager.h" //TODO 不应该有GMGL
#include "foundation/gamemachine.h"
#include "gmassets.h"

GMGameObject::GMGameObject()
{
	D(d);
	d->scaling = linear_math::Matrix4x4::identity();
	d->translation = linear_math::Matrix4x4::identity();
}

GMGameObject::GMGameObject(GMAsset asset)
	: GMGameObject()
{
	setModel(asset);
	updateMatrix();
}

void GMGameObject::setModel(GMAsset asset)
{
	D(d);
	GMModel* model = GMAssets::getModel(asset);
	GM_ASSERT(model);
	d->model = model;

	if (d->model)
	{
		for (auto& mesh : d->model->getAllMeshes())
		{
			for (auto& component : mesh->getComponents())
			{
				Shader& shader = component->getShader();
				attachEvent(shader, GM_SET_PROPERTY_EVENT_ENUM(Blend), onShaderSetBlend);
				shader.emitEvent(GM_SET_PROPERTY_EVENT_ENUM(Blend));
			}
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
	GMModel* coreObj = getModel();
	if (coreObj)
	{
		GMfloat transform[16];
		getTransform().toArray(transform);
		coreObj->getPainter()->draw(transform);
	}
}

bool GMGameObject::canDeferredRendering()
{
	D(d);
	return d->canDeferredRendering;
}

void GMGameObject::onShaderSetBlend(GMObject* sender, GMObject* receiver)
{
	GMGameObject* gameObject = gmobject_cast<GMGameObject*>(receiver);
	Shader* shader = gmobject_cast<Shader*>(sender);
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

void GMEntityObject::getBounds(REF linear_math::Vector3& mins, REF linear_math::Vector3& maxs)
{
	D(d);
	mins = d->mins;
	maxs = d->maxs;
}

void GMEntityObject::calc()
{
	D(d);
	d->mins[0] = d->mins[1] = d->mins[2] = 999999.f;
	d->maxs[0] = d->maxs[1] = d->maxs[2] = -d->mins[0];

	GMModel* obj = getModel();
	for (auto mesh : obj->getAllMeshes())
	{
		GMModel::DataType* vertices = mesh->positions().data();
		GMint sz = mesh->positions().size();
		for (GMint i = 0; i < sz; i += 4)
		{
			for (GMint j = 0; j < 3; j++)
			{
				if (vertices[i + j] < d->mins[j])
					d->mins[j] = vertices[i + j];
				if (vertices[i + j] > d->maxs[j])
					d->maxs[j] = vertices[i + j];
			}
		}
	}

	makePlanes();
}

void GMEntityObject::makePlanes()
{
	D(d);
	// 前
	d->planes[0] = GMPlane(linear_math::Vector3(0, 0, 1), -d->maxs[2]);
	// 后
	d->planes[1] = GMPlane(linear_math::Vector3(0, 0, -1), d->mins[2]);
	// 左
	d->planes[2] = GMPlane(linear_math::Vector3(-1, 0, 0), d->mins[0]);
	// 右
	d->planes[3] = GMPlane(linear_math::Vector3(1, 0, 0), -d->maxs[0]);
	// 上
	d->planes[4] = GMPlane(linear_math::Vector3(0, 1, 0), -d->maxs[0]);
	// 下
	d->planes[5] = GMPlane(linear_math::Vector3(0, -1, 0), d->mins[0]);
}

// 天空

static linear_math::Vector2 uvs[24] = {
	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),

	/*
	linear_math::Vector2(0, 0),
	linear_math::Vector2(0, 1),
	linear_math::Vector2(1, 1),
	linear_math::Vector2(1, 0),
	*/
};

GMSkyGameObject::GMSkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max)
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
	linear_math::Vector3 vertices[20] = {
		//Front
		linear_math::Vector3(d->min[0], d->max[1], d->max[2]),
		linear_math::Vector3(d->min[0], d->min[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->min[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->max[1], d->max[2]),

		//Back
		linear_math::Vector3(d->min[0], d->max[1], d->min[2]),
		linear_math::Vector3(d->min[0], d->min[1], d->min[2]),
		linear_math::Vector3(d->max[0], d->min[1], d->min[2]),
		linear_math::Vector3(d->max[0], d->max[1], d->min[2]),

		//Left
		linear_math::Vector3(d->min[0], d->max[1], d->min[2]),
		linear_math::Vector3(d->min[0], d->max[1], d->max[2]),
		linear_math::Vector3(d->min[0], d->min[1], d->max[2]),
		linear_math::Vector3(d->min[0], d->min[1], d->min[2]),

		//Right
		linear_math::Vector3(d->max[0], d->max[1], d->min[2]),
		linear_math::Vector3(d->max[0], d->max[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->min[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->min[1], d->min[2]),

		//Up
		linear_math::Vector3(d->min[0], d->max[1], d->min[2]),
		linear_math::Vector3(d->min[0], d->max[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->max[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->max[1], d->min[2]),

		//Down
		/*
		linear_math::Vector3(d->min[0], d->min[1], d->min[2]),
		linear_math::Vector3(d->min[0], d->min[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->min[1], d->max[2]),
		linear_math::Vector3(d->max[0], d->min[1], d->min[2]),
		*/
	};

	// Scaling surface
	const GMint SCALING = 2;
	linear_math::Vector3 center = (d->min + d->max) / 2;
	linear_math::Matrix4x4 transScale = linear_math::scale(linear_math::Vector3(SCALING, 1, SCALING));
	for (GMuint i = 0; i < 20; i++)
	{
		linear_math::Matrix4x4 transRestore = linear_math::translate(center);
		linear_math::Matrix4x4 transMoveToAxisOrigin = linear_math::translate(-center);
		linear_math::Matrix4x4 transFinal = transRestore * transScale * transMoveToAxisOrigin;

		linear_math::Vector4 pt = linear_math::Vector4(vertices[i], 1) * transFinal;
		vertices[i] = linear_math::Vector3(pt[0], pt[1], pt[2]);
	}

	GMModel* object = new GMModel();
	*obj = object;

	GMMesh* child = new GMMesh();
	GMComponent* component = new GMComponent(child);
	component->setShader(d->shader);

	// We don't draw surface beneath us
	for (GMuint i = 0; i < 5; i++)
	{
		component->beginFace();
		component->vertex(vertices[i * 4][0], vertices[i * 4][1], vertices[i * 4][2]);
		component->vertex(vertices[i * 4 + 1][0], vertices[i * 4 + 1][1], vertices[i * 4 + 1][2]);
		component->vertex(vertices[i * 4 + 2][0], vertices[i * 4 + 2][1], vertices[i * 4 + 2][2]);
		component->vertex(vertices[i * 4 + 3][0], vertices[i * 4 + 3][1], vertices[i * 4 + 3][2]);
		component->uv(uvs[i * 4][0], uvs[i * 4][1]);
		component->uv(uvs[i * 4 + 1][0], uvs[i * 4 + 1][1]);
		component->uv(uvs[i * 4 + 2][0], uvs[i * 4 + 2][1]);
		component->uv(uvs[i * 4 + 3][0], uvs[i * 4 + 3][1]);
		component->endFace();
	}
	child->appendComponent(component);
	object->append(child);
}
