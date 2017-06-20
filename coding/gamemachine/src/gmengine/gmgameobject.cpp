#include "stdafx.h"
#include "gmgameobject.h"
#include "gmengine/gmgameworld.h"
#include "gmdatacore/glyph/gmglyphmanager.h"
#include "gmgl/gmglglyphmanager.h" //TODO 不应该有GMGL
#include "foundation/gamemachine.h"

GMGameObject::GMGameObject(AUTORELEASE Object* obj)
{
	setObject(obj);
}

void GMGameObject::setObject(AUTORELEASE Object* obj)
{
	D(d);
	d->object.reset(obj);
}

Object* GMGameObject::getObject()
{
	D(d);
	if (!d->object)
		d->object.reset(new Object());
	return d->object;
}

void GMGameObject::setWorld(GMGameWorld* world)
{
	D(d);
	ASSERT(!d->world);
	d->world = world;
}

GMGameWorld* GMGameObject::getWorld()
{
	D(d);
	return d->world;
}

void GMGameObject::onAppendingObjectToWorld()
{

}

void GMGameObject::onBeforeDraw()
{

}

//GlyphObject
#define X(i) (i) / resolutionWidth
#define Y(i) (i) / resolutionHeight
#define UV_X(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_HEIGHT)

GMGlyphObject::GMGlyphObject()
	: GMGameObject(nullptr)
{
	D(d);
}

void GMGlyphObject::setText(const GMWchar* text)
{
	D(d);
	d->text = text;
}

// 窗口中央坐标为(0,0)，左下角坐标为(-1, -1)
void GMGlyphObject::setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height)
{
	D(d);
	d->left = left;
	d->bottom = bottom;
	d->width = width;
	d->height = height;
}

void GMGlyphObject::constructObject()
{
	D(d);
	D_BASE(db, GMGameObject);

	GMGlyphManager* glyphManager = GameMachine::instance().getGlyphManager();
	GMUIWindow* window = GameMachine::instance().getMainWindow();
	GMRect rect = window->getWindowRect();
	GMfloat resolutionWidth = rect.width, resolutionHeight = rect.height;

	Object* obj = new Object();
	Mesh* child = new Mesh();
	obj->append(child);
	child->setArrangementMode(Mesh::Triangle_Strip);
	child->setType(Mesh::Glyph);

	Component* component = new Component(child);
	Shader& shader = component->getShader();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frames[0] = glyphManager->glyphTexture();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frameCount = 1;
	shader.noDepthTest = true;
	shader.cull = GMS_NONE;
	shader.blend = true;
	shader.blendFactors[0] = GMS_ONE;
	shader.blendFactors[1] = GMS_ONE;

	const GMWchar* p = d->text.c_str();
	const GMfloat Z = 0;
	GMfloat x = d->left, y = d->bottom;

	while (*p)
	{
		component->beginFace();
		const GlyphInfo& glyph = glyphManager->getChar(*p);

		if (glyph.width > 0 && glyph.height > 0)
		{
			// 如果width和height为0，视为空格，只占用空间而已
			// 否则：按照条带顺序，创建顶点
			// 0 2
			// 1 3
			// 让所有字体origin开始的x轴平齐

			component->vertex(x + X(glyph.bearingX), y + Y(glyph.bearingY), Z);
			component->vertex(x + X(glyph.bearingX), y + Y(glyph.bearingY - glyph.height), Z);
			component->vertex(x + X(glyph.bearingX + glyph.width), y + Y(glyph.bearingY), Z);
			component->vertex(x + X(glyph.bearingX + glyph.width), y + Y(glyph.bearingY - glyph.height), Z);

			component->uv(UV_X(glyph.x), UV_Y(glyph.y));
			component->uv(UV_X(glyph.x), UV_Y(glyph.y + glyph.height));
			component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y));
			component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height));
		}
		x += X(glyph.advance);

		component->endFace();
		p++;
	}

	child->appendComponent(component);
	setObject(obj);
}


void GMGlyphObject::onAppendingObjectToWorld()
{
	D(d);
	d->lastRenderText = d->text;
	constructObject();
}

void GMGlyphObject::onBeforeDraw()
{
	D(d);
	if (d->lastRenderText != d->text)
	{
		updateObject();
		d->lastRenderText = d->text;
	}
	GMGameObject::onBeforeDraw();
}

void GMGlyphObject::updateObject()
{
	D_BASE(d, GMGameObject);
	constructObject();
	GameMachine::instance().initObjectPainter(this);
}

//GMEntityObject
GMEntityObject::GMEntityObject(AUTORELEASE Object* obj)
	: GMGameObject(obj)
{
	calc();
}

Plane* GMEntityObject::getPlanes()
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

	Object* obj = getObject();
	for (auto mesh : obj->getAllMeshes())
	{
		Object::DataType* vertices = mesh->vertices().data();
		GMint sz = mesh->vertices().size();
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
	d->planes[0] = Plane(linear_math::Vector3(0, 0, 1), -d->maxs[2]);
	// 后
	d->planes[1] = Plane(linear_math::Vector3(0, 0, -1), d->mins[2]);
	// 左
	d->planes[2] = Plane(linear_math::Vector3(-1, 0, 0), d->mins[0]);
	// 右
	d->planes[3] = Plane(linear_math::Vector3(1, 0, 0), -d->maxs[0]);
	// 上
	d->planes[4] = Plane(linear_math::Vector3(0, 1, 0), -d->maxs[0]);
	// 下
	d->planes[5] = Plane(linear_math::Vector3(0, -1, 0), d->mins[0]);
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

SkyGameObject::SkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max)
	: GMGameObject(nullptr)
{
	D(d);
	d->shader = shader;
	d->min = min;
	d->max = max;

	Object* obj = nullptr;
	createSkyBox(&obj);
	setObject(obj);
}

void SkyGameObject::createSkyBox(OUT Object** obj)
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

	Object* object = new Object();
	*obj = object;

	Mesh* child = new Mesh();
	child->setType(Mesh::Sky);

	Component* component = new Component(child);
	component->getShader() = d->shader;

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