#include "stdafx.h"
#include "glyphobject.h"
#include "gmdatacore\glyph\glyphpainter.h"
#include "gmengine/controllers/gamemachine.h"
#include "gmengine/controllers/factory.h"

GlyphObject::GlyphObject()
	: GameObject(nullptr)
{
}

void GlyphObject::setText(const wchar_t* text)
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
	Object* obj = new Object();
	ChildObject* child = new ChildObject();
	obj->append(child);
	child->setType(ChildObject::Glyph);
	const GMfloat Z = 0;

	Component* component = new Component(child);
	component->beginFace();
	component->vertex(d.left, d.bottom, Z);
	component->vertex(d.left, d.bottom + d.height, Z);
	component->vertex(d.left + d.width, d.bottom + d.height, Z);
	component->uv(0, 0);
	component->uv(0, 1);
	component->uv(1, 1);
	component->endFace();

	component->beginFace();
	component->vertex(d.left + d.width, d.bottom + d.height, Z);
	component->vertex(d.left + d.width, d.bottom, Z);
	component->vertex(d.left, d.bottom, Z);
	component->uv(1, 1);
	component->uv(1, 0);
	component->uv(0, 0);
	component->endFace();

	component->getShader().noDepthTest = true;
	drawText(component);
	child->appendComponent(component);

	setObject(obj);
}

void GlyphObject::drawText(Component* component)
{
	D(d);
	D_BASE(GameObject, db);
	GlyphPainter p;
	FontAttributes a = { "", 24, 300, 300 };
	p.drawString(a, L"a");
	Image* img;
	p.getImage(&img);
	
	GameMachine* gm = db.world->getGameMachine();
	IFactory* factory = gm->getFactory();
	factory->createTexture(img, &d.texture);

	Shader& shader = component->getShader();
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frames[0] = d.texture;
	shader.texture.textures[TEXTURE_INDEX_AMBIENT].frameCount = 1;
}

void GlyphObject::onAppendingObjectToWorld()
{
	constructObject();
}
