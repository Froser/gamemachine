#include "stdafx.h"
#include "glyphobject.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmengine/controllers/factory.h"
#include "gmdatacore/glyph/glyphmanager.h"
#include "gmgl/gmglglyphmanager.h"

#define VERTEX_X(i) i / resolutionWidth
#define VERTEX_Y(i) i / resolutionHeight
#define UV_X(i) ((GMfloat)i / (GMfloat)GMGLGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((GMfloat)i / (GMfloat)GMGLGlyphManager::CANVAS_HEIGHT)

GlyphObject::GlyphObject()
	: GameObject(nullptr)
{
	D(d);
	d.properties.fontSize = GMGLGlyphManager::FONT_SIZE * 2;
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
	/*
	shader.blend = true;
	shader.blendFactors[0] = GMS_SRC_ALPHA;
	shader.blendFactors[1] = GMS_ONE_MINUS_SRC_ALPHA;
	*/

	const GMWChar* p = d.text.c_str();
	const GMfloat Z = 0;
	GMfloat offsetX = d.left, offsetY = d.bottom;
	while (*p)
	{
		component->beginFace();
		const GlyphInfo& glyph = glyphManager->getChar(*p);

		// 按照条带顺序，创建顶点
		// 0 2
		//  1 3
		component->vertex(offsetX, offsetY + VERTEX_Y(d.properties.fontSize), Z);
		component->vertex(offsetX, offsetY, Z);
		component->vertex(offsetX + VERTEX_X(d.properties.fontSize), offsetY + VERTEX_Y(d.properties.fontSize), Z);
		component->vertex(offsetX + VERTEX_X(d.properties.fontSize), offsetY, Z);
		
		component->uv(UV_X(glyph.x), UV_Y(glyph.y));
		component->uv(UV_X(glyph.x), UV_Y(glyph.y + glyph.height));
		component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y));
		component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height));
		
		offsetX += VERTEX_X(d.properties.fontSize); //TODO 先不考虑换行
		component->endFace();
		p++;
	}

	child->appendComponent(component);
	setObject(obj);
}

void GlyphObject::onAppendingObjectToWorld()
{
	constructObject();
}
