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

void GMEntityObject::getBounds(REF glm::vec3& mins, REF glm::vec3& maxs)
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

	GMMesh* mesh = getModel()->getMesh();
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

	makePlanes();
}

void GMEntityObject::makePlanes()
{
	D(d);
	// 前
	d->planes[0] = GMPlane(glm::vec3(0, 0, 1), -d->maxs[2]);
	// 后
	d->planes[1] = GMPlane(glm::vec3(0, 0, -1), d->mins[2]);
	// 左
	d->planes[2] = GMPlane(glm::vec3(-1, 0, 0), d->mins[0]);
	// 右
	d->planes[3] = GMPlane(glm::vec3(1, 0, 0), -d->maxs[0]);
	// 上
	d->planes[4] = GMPlane(glm::vec3(0, 1, 0), -d->maxs[0]);
	// 下
	d->planes[5] = GMPlane(glm::vec3(0, -1, 0), d->mins[0]);
}

// 天空
static glm::vec2 uvs[24] = {
	glm::vec2(0, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(1, 0),

	glm::vec2(0, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(1, 0),

	glm::vec2(0, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(1, 0),

	glm::vec2(0, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(1, 0),

	glm::vec2(0, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(1, 0),

	glm::vec2(0, 0),
	glm::vec2(0, 1),
	glm::vec2(1, 1),
	glm::vec2(1, 0),
};

GMSkyGameObject::GMSkyGameObject(const GMShader& shader, const glm::vec3& min, const glm::vec3& max)
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
	glm::vec3 vertices[] = {
		//Front
		glm::vec3(d->min[0], d->max[1], d->max[2]),
		glm::vec3(d->min[0], d->min[1], d->max[2]),
		glm::vec3(d->max[0], d->min[1], d->max[2]),
		glm::vec3(d->max[0], d->max[1], d->max[2]),

		//Back
		glm::vec3(d->min[0], d->max[1], d->min[2]),
		glm::vec3(d->min[0], d->min[1], d->min[2]),
		glm::vec3(d->max[0], d->min[1], d->min[2]),
		glm::vec3(d->max[0], d->max[1], d->min[2]),

		//Left
		glm::vec3(d->min[0], d->max[1], d->min[2]),
		glm::vec3(d->min[0], d->max[1], d->max[2]),
		glm::vec3(d->min[0], d->min[1], d->max[2]),
		glm::vec3(d->min[0], d->min[1], d->min[2]),

		//Right
		glm::vec3(d->max[0], d->max[1], d->min[2]),
		glm::vec3(d->max[0], d->max[1], d->max[2]),
		glm::vec3(d->max[0], d->min[1], d->max[2]),
		glm::vec3(d->max[0], d->min[1], d->min[2]),

		//Up
		glm::vec3(d->min[0], d->max[1], d->min[2]),
		glm::vec3(d->min[0], d->max[1], d->max[2]),
		glm::vec3(d->max[0], d->max[1], d->max[2]),
		glm::vec3(d->max[0], d->max[1], d->min[2]),

		//Down
		glm::vec3(d->min[0], d->min[1], d->min[2]),
		glm::vec3(d->min[0], d->min[1], d->max[2]),
		glm::vec3(d->max[0], d->min[1], d->max[2]),
		glm::vec3(d->max[0], d->min[1], d->min[2]),
	};

	// Scaling surface
	const GMint SCALING = 2;
	glm::vec3 center = (d->min + d->max) / 2.f;
	glm::mat4 transScale = glm::scale(glm::vec3(SCALING, 1, SCALING));
	for (GMuint i = 0; i < 20; i++)
	{
		glm::mat4 transRestore = glm::translate(center);
		glm::mat4 transMoveToAxisOrigin = glm::translate(-center);
		glm::mat4 transFinal = transRestore * transScale * transMoveToAxisOrigin;

		glm::vec4 pt = transFinal * glm::vec4(vertices[i], 1);
		vertices[i] = glm::vec3(pt[0], pt[1], pt[2]);
	}

	GMModel* model = new GMModel();
	*obj = model;

	GMComponent* component = new GMComponent(model->getMesh());
	component->setShader(d->shader);

	for (GMuint i = 0; i < 6; i++)
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
