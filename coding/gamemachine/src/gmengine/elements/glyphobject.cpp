#include "stdafx.h"
#include "glyphobject.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmengine/controllers/factory.h"
#include "gmdatacore/glyph/glyphmanager.h"
#include "gmgl/gmglglyphmanager.h"

#define X(i) (i) / resolutionWidth
#define Y(i) (i) / resolutionHeight
#define UV_X(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_HEIGHT)

GlyphObject::GlyphObject()
	: GameObject(nullptr)
{
	D(d);
}

void GlyphObject::setText(const GMWChar* text)
{
	D(d);
	d.text = text;
}

// 窗口中央坐标为(0,0)，左下角坐标为(-1, -1)
void GlyphObject::setGeometry(GMfloat left, GMfloat bottom, GMfloat width, GMfloat height)
{
	D(d);
	d.left = left;
	d.bottom = bottom;
	d.width = width;
	d.height = height;
}

void GlyphObject::constructObject()
{
	D(d);
	D_BASE(GameObject, db);

	GlyphManager* glyphManager = db.world->getGameMachine()->getGlyphManager();
	IWindow* window = db.world->getGameMachine()->getWindow();
	GMRect rect = window->getWindowRect();
	GMfloat resolutionWidth = rect.width, resolutionHeight = rect.height;

	Object* obj = new Object();
	ChildObject* child = new ChildObject();
	obj->append(child);
	child->setArrangementMode(ChildObject::Triangle_Strip);
	child->setType(ChildObject::Glyph);

	Component* component = new Component(child);
	Shader& shader = component->getShader();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frames[0] = glyphManager->glyphTexture();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frameCount = 1;
	shader.noDepthTest = true;
	shader.cull = GMS_NONE;
	shader.blend = true;
	shader.blendFactors[0] = GMS_ONE;
	shader.blendFactors[1] = GMS_ONE;

	const GMWChar* p = d.text.c_str();
	const GMfloat Z = 0;
	GMfloat x = d.left, y = d.bottom;

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


void GlyphObject::onAppendingObjectToWorld()
{
	D(d);
	d.lastRenderText = d.text;
	constructObject();
}

void GlyphObject::getReadyForRender(DrawingList& list)
{
	D(d);
	if (d.lastRenderText != d.text)
	{
		updateObject();
		d.lastRenderText = d.text;
	}
	GameObject::getReadyForRender(list);
}

void GlyphObject::updateObject()
{
	D_BASE(GameObject, d);
	constructObject();
	d.world->initObject(this);
}