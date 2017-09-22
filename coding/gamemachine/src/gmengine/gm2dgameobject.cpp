#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "foundation/utilities/gmprimitivecreator.h"
#include "gmgl/gmglglyphmanager.h"

static GMRectF toNormalCoord(const GMRect& in)
{
	GMRect client = GM.getMainWindow()->getClientRect();
	GMRectF out = {
		in.x * 2.f / client.width - 1.f,
		1.f - in.y * 2.f / client.height,
		(GMfloat)in.width / client.width,
		(GMfloat)in.height / client.height
	};
	return out;
}

//////////////////////////////////////////////////////////////////////////
GM2DEvent::GM2DEvent(GM2DEventType type)
	: m_type(type)
{
}

bool GM2DMouseDownEvent::buttonDown(Button button)
{
	if (button == GM2DMouseDownEvent::Left)
		return !!(m_state.button & GMMouseButton_Left);
	if (button == GM2DMouseDownEvent::Right)
		return !!(m_state.button & GMMouseButton_Right);
	if (button == GM2DMouseDownEvent::Middle)
		return !!(m_state.button & GMMouseButton_Middle);
	return false;
}
//////////////////////////////////////////////////////////////////////////
GM2DGameObject::GM2DGameObject()
{
	D(d);
	GMRect client = GM.getMainWindow()->getClientRect();
	d->clientSize = client;
}

void GM2DGameObject::setGeometry(const GMRect& rect)
{
	D(d);
	d->geometry = rect;
}

void GM2DGameObject::notifyControl()
{
	D(d);
	updateUI();

	IInput* input = GM.getMainWindow()->getInputMananger();
	IMouseState& mouseState = input->getMouseState();
	GMMouseState ms = mouseState.mouseState();
	
	if (insideGeometry(ms.posX, ms.posY))
	{
		GM2DMouseMoveEvent e(ms);
		event(&e);

		if (ms.button != GMMouseButton_None)
		{
			GM2DMouseDownEvent e(ms);
			event(&e);
		}
	}
}

bool GM2DGameObject::insideGeometry(GMint x, GMint y)
{
	D(d);
	return GM_in_rect(d->geometry, x, y);
}

void GM2DGameObject::updateUI()
{
	D(d);
	switch(GM.peekMessage().msgType)
	{
	case GameMachineMessageType::WindowSizeChanged:
		GMRect nowClient = GM.getMainWindow()->getClientRect();
		GMfloat scaleX = (GMfloat)nowClient.width / d->clientSize.width,
			scaleY = (GMfloat)nowClient.height / d->clientSize.height;

		if (getStretch())
		{
			d->geometry.x *= scaleX;
			d->geometry.y *= scaleY;
			d->geometry.width *= scaleX;
			d->geometry.height *= scaleY;
		}
		else
		{
			// 调整大小，防止拉伸
			GMfloat scaling[] = { 1.f / scaleX, 1.f / scaleY, 1 };
			GMPrimitiveUtil::scaleModel(*getModel(), scaling);

			// 相对于左上角位置也不能变
			GMRectF rect = toNormalCoord(d->geometry);
			GMfloat trans[] = { rect.x, rect.y, 0 };
			GMPrimitiveUtil::translateModelTo(*getModel(), trans);
		}

		d->clientSize = nowClient;
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//GlyphObject
template <typename T>
inline bool isValidRect(const T& r)
{
	return r.x >= 0;
}

#define X(i) (i) / resolutionWidth
#define Y(i) (i) / resolutionHeight
#define UV_X(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((i) / (GMfloat)GMGLGlyphManager::CANVAS_HEIGHT)

GMGlyphObject::~GMGlyphObject()
{
	D(d);
	GMModel* m = getModel();
	if (m)
		delete m;
}

void GMGlyphObject::setText(const GMWchar* text)
{
	D(d);
	d->text = text;
}

void GMGlyphObject::constructModel()
{
	D(d);
	D_BASE(db, GMGameObject);

	GMGlyphManager* glyphManager = GM.getGlyphManager();
	GMModel* model = new GMModel();
	GMMesh* child = new GMMesh();
	model->append(child);
	child->setArrangementMode(GMArrangementMode::Triangle_Strip);
	child->setType(GMMeshType::Model2D);

	GMComponent* component = new GMComponent(child);
	Shader& shader = component->getShader();
	shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, 0).addFrame(glyphManager->glyphTexture());
	shader.setNoDepthTest(true);
	shader.setCull(GMS_Cull::NONE);
	shader.setBlend(true);
	shader.setBlendFactorSource(GMS_BlendFunc::ONE);
	shader.setBlendFactorDest(GMS_BlendFunc::ONE);

	createVertices(component);
	child->appendComponent(component);

	model->setUsageHint(GMUsageHint::DynamicDraw);
	GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, model);
	setModel(&asset);
}

void GMGlyphObject::updateModel()
{
	D(d);
	GMComponent* component = getModel()->getAllMeshes()[0]->getComponents()[0];
	component->clear();
	createVertices(component);
	GMModelPainter* painter = getModel()->getPainter();
	painter->dispose();
	painter->transfer();
}

void GMGlyphObject::createVertices(GMComponent* component)
{
	D(d);
	D_BASE(db, GM2DGameObject);
	GMGlyphManager* glyphManager = GM.getGlyphManager();
	IWindow* window = GM.getMainWindow();

	std::wstring str = d->text.toStdWString();
	const GMWchar* p = str.c_str();
	const GMfloat Z = 0;

	GMRect rect = d->autoResize && isValidRect(d->lastClientRect) ? d->lastClientRect : GM.getMainWindow()->getClientRect();
	GMRectF coord = d->autoResize && isValidRect(d->lastGeometry) ? d->lastGeometry : toNormalCoord(db->geometry);

	if (!isValidRect(d->lastClientRect) || d->autoResize)
	{
		GM_ASSERT(!isValidRect(d->lastGeometry));
		d->lastClientRect = rect;
		d->lastGeometry = coord;
	}

	GMfloat resolutionWidth = rect.width, resolutionHeight = rect.height;
	GMfloat &x = coord.x, &y = coord.y;

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
}

void GMGlyphObject::onAppendingObjectToWorld()
{
	D(d);
	d->lastRenderText = d->text;
	constructModel();
}

void GMGlyphObject::draw()
{
	D(d);
	if (d->lastRenderText != d->text)
	{
		update();
		d->lastRenderText = d->text;
	}
	GMGameObject::draw();
}

void GMGlyphObject::update()
{
	D_BASE(d, GMGameObject);
	updateModel();
}


//////////////////////////////////////////////////////////////////////////
GMImage2DGameObject::~GMImage2DGameObject()
{
	D(d);
	if (d->model)
		delete d->model;
}

void GMImage2DGameObject::setImage(GMAsset& image)
{
	D(d);
	d->image = GMAssets::getTexture(image);
	GM_ASSERT(d->image);
}

void GMImage2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GM2DGameObject);
	GMRectF coord = toNormalCoord(db->geometry);
	GMfloat extents[3] = {
		coord.width,
		coord.height,
		1.f,
	};
	GMfloat pos[3] = { coord.x, coord.y, 0 };
	GMPrimitiveCreator::createQuad(extents, pos, &d->model, this, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft);
	d->model->setUsageHint(GMUsageHint::DynamicDraw);

	auto asset = GMAssets::createIsolatedAsset(GMAssetType::Model, d->model);
	setModel(&asset);
}

void GMImage2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setNoDepthTest(true);
	auto& tex = shader.getTexture();
	auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
	frames.addFrame(d->image);
}