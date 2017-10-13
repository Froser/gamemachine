#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "gmgl/gmglglyphmanager.h"

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
	D_BASE(db, GMControlGameObject);
	GMGlyphManager* glyphManager = GM.getGlyphManager();
	IWindow* window = GM.getMainWindow();

	std::wstring str = d->text.toStdWString();
	const GMWchar* p = str.c_str();
	const GMfloat Z = 0;

	GMRect rect = d->autoResize && isValidRect(d->lastClientRect) ? d->lastClientRect : GM.getMainWindow()->getClientRect();
	GMRectF coord = d->autoResize && isValidRect(d->lastGeometry) ? d->lastGeometry : toViewportCoord(db->geometry);

	if (!isValidRect(d->lastClientRect) || d->autoResize)
	{
		GM_ASSERT(!isValidRect(d->lastGeometry));
		d->lastClientRect = rect;
		d->lastGeometry = coord;
	}

	GMfloat resolutionWidth = rect.width, resolutionHeight = rect.height;
	GMfloat &x = coord.x, &y = coord.y;

	// 获取字符串高度
	GMfloat maxHeight = 0;
	while (*p)
	{
		const GlyphInfo& glyph = glyphManager->getChar(*p);
		if (maxHeight < glyph.height)
			maxHeight = glyph.height;
		++p;
	}
	p = str.c_str();

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

			component->vertex(x + X(glyph.bearingX), y - Y(maxHeight - glyph.bearingY), Z);
			component->vertex(x + X(glyph.bearingX), y - Y(maxHeight - (glyph.bearingY - glyph.height)), Z);
			component->vertex(x + X(glyph.bearingX + glyph.width), y - Y(maxHeight - glyph.bearingY), Z);
			component->vertex(x + X(glyph.bearingX + glyph.width), y - Y(maxHeight - (glyph.bearingY - glyph.height)), Z);

			component->uv(UV_X(glyph.x), UV_Y(glyph.y));
			component->uv(UV_X(glyph.x), UV_Y(glyph.y + glyph.height));
			component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y));
			component->uv(UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height));
		}
		x += X(glyph.advance);

		component->endFace();
		++p;
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
	GMModel* model = getModel();
	if (model)
		delete model;
	if (d->textModel)
		delete d->textModel;
}

void GMImage2DGameObject::setImage(GMAsset& image)
{
	D(d);
	d->image = GMAssets::getTexture(image);
	GM_ASSERT(d->image);
}

void GMImage2DGameObject::setText(const GMString& text)
{
	D(d);
	d->text = text.toStdWString();
	if (!d->text.empty())
		d->textModel = new GMGlyphObject();
}

void GMImage2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	GMRectF coord = toViewportCoord(db->geometry);
	GMfloat extents[3] = {
		coord.width,
		coord.height,
		1.f,
	};
	GMModel* model = nullptr;
	GMfloat pos[3] = { coord.x, coord.y, 0 };
	GMPrimitiveCreator::createQuad(extents, pos, &model, this, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft);

	auto asset = GMAssets::createIsolatedAsset(GMAssetType::Model, model);
	setModel(&asset);

	if (d->textModel)
	{
		d->textModel->setWorld(getWorld());
		d->textModel->setGeometry(db->geometry);
		d->textModel->setText(d->text.c_str());
		d->textModel->onAppendingObjectToWorld();
		GM.initObjectPainter(d->textModel->getModel());
		GMAssets::createIsolatedAsset(GMAssetType::Model, d->textModel);
	}
}

void GMImage2DGameObject::draw()
{
	D(d);
	Base::draw();
	if (d->textModel)
		d->textModel->draw();
}

void GMImage2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setNoDepthTest(true);

	if (d->image)
	{
		auto& tex = shader.getTexture();
		auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
		frames.addFrame(d->image);
	}
}

//////////////////////////////////////////////////////////////////////////
void GMListbox2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setNoDepthTest(true);
}

void GMListbox2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	GMRectF coord = toViewportCoord(db->geometry);
	GMfloat extents[3] = {
		coord.width,
		coord.height,
		1.f,
	};
	GMModel* model = nullptr;
	GMfloat pos[3] = { coord.x, coord.y, 0 };
	GMPrimitiveCreator::createQuad(extents, pos, &model, this, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft);
	model->setUsageHint(GMUsageHint::DynamicDraw);

	auto asset = GMAssets::createIsolatedAsset(GMAssetType::Model, model);
	setModel(&asset);
}