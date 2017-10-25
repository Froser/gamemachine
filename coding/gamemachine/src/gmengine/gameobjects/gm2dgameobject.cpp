#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "gmgl/gmglglyphmanager.h"

enum Margins
{
	Left = 0,
	Top,
	Right,
	Bottom,
};

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
	GMModel* model = new GMModel();
	GMMesh* child = new GMMesh();
	model->append(child);
	child->setArrangementMode(GMArrangementMode::Triangle_Strip);
	child->setType(GMMeshType::Model2D);

	GMComponent* component = new GMComponent(child);
	onCreateShader(component->getShader());
	createVertices(component);
	child->appendComponent(component);

	GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, model);
	setModel(asset);
}

void GMGlyphObject::onCreateShader(Shader& shader)
{
	GMGlyphManager* glyphManager = GM.getGlyphManager();
	shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, 0).addFrame(glyphManager->glyphTexture());
	shader.setNoDepthTest(true);
	shader.setCull(GMS_Cull::NONE);
	//shader.setBlend(false);
	//shader.setBlendFactorSource(GMS_BlendFunc::ONE);
	//shader.setBlendFactorDest(GMS_BlendFunc::ONE);
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
GMImage2DBorder::GMImage2DBorder(GMAsset& texture, const GMRect& borderTextureGeometry, GMfloat textureWidth, GMfloat textureHeight)
{
	D(d);
	d->texture = texture;
	d->borderTextureGeometry = borderTextureGeometry;
	d->width = textureWidth;
	d->height = textureHeight;
}

GMImage2DBorder::~GMImage2DBorder()
{
	D(d);
	release(d->models);
	release(d->objects);
}

template <typename T, GMint Size> void GMImage2DBorder::release(T* (&m)[Size])
{
	for (GMint i = 0; i < Size; ++i)
	{
		GM_delete(m[i]);
	}
}

void GMImage2DBorder::clone(GMImage2DBorder& b)
{
	D(d);
	D_OF(d_b, &b);
	d->texture = d_b->texture;
	d->borderTextureGeometry = d_b->borderTextureGeometry;
	d->width = d_b->width;
	d->height = d_b->height;
}

void GMImage2DBorder::createBorder(const GMRect& geometry)
{
	D(d);
	GM_ASSERT(d->models);

	// 创建9个区域
	// 0 3 6
	// 1 4 7
	// 2 5 8
	std::array<std::array<GMfloat, 12>, 9> uv;

	const GMfloat& textureWidth = d->width,
		&textureHeight = d->height;

	const GMRect& textureGeo = textureGeometry();
	GMfloat basedUV[2] = {
		textureGeo.x / (GMfloat)textureWidth,
		textureGeo.y / (GMfloat)textureHeight,
	};

	// 三等分点
	GMfloat w_3 = textureGeo.width / 3.f, h_3 = textureGeo.height / 3.f;
	GMfloat d1[2] = { w_3 / textureWidth, h_3 / textureHeight };
	GMfloat d2[2] = { d1[0] * 2.f, d1[1] * 2.f };
	GMfloat d3[3] = { d1[0] * 3.f, d1[1] * 3.f };
	uv[0] = {
		basedUV[0], basedUV[1], 0,
		basedUV[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d1[0], basedUV[1], 0,
	};
	uv[1] = {
		basedUV[0], basedUV[1] - d1[1], 0,
		basedUV[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d1[1], 0,
	};
	uv[2] = {
		basedUV[0], basedUV[1] - d2[1], 0,
		basedUV[0], basedUV[1] - d3[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d3[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d2[1], 0,
	};

	uv[3] = {
		basedUV[0] + d1[0], basedUV[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d2[0], basedUV[1], 0,
	};
	uv[4] = {
		basedUV[0] + d1[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d1[1], 0,
	};
	uv[5] = {
		basedUV[0] + d1[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d1[0], basedUV[1] - d3[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d3[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d2[1], 0,
	};

	uv[6] = {
		basedUV[0] + d2[0], basedUV[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d3[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d3[0], basedUV[1], 0,
	};
	uv[7] = {
		basedUV[0] + d2[0], basedUV[1] - d1[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d3[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d3[0], basedUV[1] - d1[1], 0,
	};
	uv[8] = {
		basedUV[0] + d2[0], basedUV[1] - d2[1], 0,
		basedUV[0] + d2[0], basedUV[1] - d3[1], 0,
		basedUV[0] + d3[0], basedUV[1] - d3[1], 0,
		basedUV[0] + d3[0], basedUV[1] - d2[1], 0,
	};

	GMRect w = GM.getMainWindow()->getClientRect();
	GMRectF window = {
		(GMfloat)w.x,
		(GMfloat)w.y,
		(GMfloat)w.width,
		(GMfloat)w.height
	};

	GMfloat gw_3 = geometry.width / 3.f, gh_3 = geometry.height / 3.f;
	GMfloat extents[3] = {
		gw_3 / window.width,
		gh_3 / window.height,
		1,
	};

	GMfloat pos[9][3] = {
		(geometry.x * 2) / window.width - 1, 1 - (geometry.y) * 2 / window.height, 0,
		(geometry.x * 2) / window.width - 1, 1 - (geometry.y + gh_3) * 2 / window.height, 0,
		(geometry.x * 2) / window.width - 1, 1 - (geometry.y + gh_3 * 2) * 2 / window.height, 0,
		(geometry.x + gw_3) * 2 / window.width - 1, 1 - (geometry.y) * 2 / window.height, 0,
		(geometry.x + gw_3) * 2 / window.width - 1, 1 - (geometry.y + gh_3) * 2 / window.height, 0,
		(geometry.x + gw_3) * 2 / window.width - 1, 1 - (geometry.y + gh_3 * 2) * 2 / window.height, 0,
		(geometry.x + gw_3 * 2) * 2 / window.width - 1, 1 - (geometry.y) * 2 / window.height, 0,
		(geometry.x + gw_3 * 2) * 2 / window.width - 1, 1 - (geometry.y + gh_3) * 2 / window.height, 0,
		(geometry.x + gw_3 * 2) * 2 / window.width - 1, 1 - (geometry.y + gh_3 * 2) * 2 / window.height, 0,
	};

	// Shader回调
	struct _Callback: public IPrimitiveCreatorShaderCallback {
		_Callback(GMAsset t)
			: texture(t)
		{
		}

		void onCreateShader(Shader& shader)
		{
			shader.setNoDepthTest(true);
			auto& tex = shader.getTexture();
			auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
			frames.addFrame(GMAssets::getTexture(texture));
		}

		GMAsset texture;
	} _cb(d->texture);

	// 制作9个矩形进行拉伸（可以以后还会有非拉伸的模式）
	for (GMint i = 0; i < 9; ++i)
	{
		GMfloat (*ptr)[12] = (GMfloat(*)[12])uv[i].data();
		GMPrimitiveCreator::createQuad(extents, pos[i], d->models + i, &_cb, GMMeshType::Model2D, GMPrimitiveCreator::TopLeft, ptr);

		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, *(d->models + i));
		d->objects[i] = new GMGameObject(asset);
		d->objects[i]->onAppendingObjectToWorld();
		GM.initObjectPainter(d->objects[i]->getModel());
	}
}

template <GMint Size> void GMImage2DBorder::drawObjects(GMGameObject* (&objects)[Size])
{
	IGraphicEngine* engine = GM.getGraphicEngine();
	for (GMint i = 0; i < Size; ++i)
	{
		objects[i]->draw();
	}
}

void GMImage2DBorder::draw()
{
	D(d);
	drawObjects(d->objects);
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

void GMImage2DGameObject::setPaddings(GMint left, GMint top, GMint right, GMint bottom)
{
	D(d);
	d->paddings[Left] = left;
	d->paddings[Top] = top;
	d->paddings[Right] = right;
	d->paddings[Bottom] = bottom;
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

void GMImage2DGameObject::setBorder(GMImage2DBorder& border)
{
	D(d);
	d->border.clone(border);
}

void GMImage2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	Base::onAppendingObjectToWorld();
	createBackgroundImage();
	createBorder();
	createGlyphs();
}

void GMImage2DGameObject::draw()
{
	D(d);

	// 首先创建出一个裁剪框
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->beginCreateStencil();
	Base::draw();
	engine->endCreateStencil();

	engine->beginUseStencil(false);

	// 背景
	Base::draw();

	// 边框
	if (d->border.hasBorder())
		d->border.draw();

	// 文字
	if (d->textModel)
		d->textModel->draw();

	engine->endUseStencil();
}

void GMImage2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setBlend(false);
	shader.setNoDepthTest(true);

	if (d->image)
	{
		auto& tex = shader.getTexture();
		auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
		frames.addFrame(d->image);
	}
}

void GMImage2DGameObject::createBackgroundImage()
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

	setModel(GMAssets::createIsolatedAsset(GMAssetType::Model, model));
}

void GMImage2DGameObject::createBorder()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	d->border.createBorder(db->geometry);
}

void GMImage2DGameObject::createGlyphs()
{
	D(d);
	D_BASE(db, GMControlGameObject);
	if (d->textModel)
	{
		GMRect geometry = {
			db->geometry.x + d->paddings[Left],
			db->geometry.y + d->paddings[Top],
			db->geometry.width - d->paddings[Left] - d->paddings[Right],
			db->geometry.height - d->paddings[Top] - d->paddings[Bottom],
		};

		d->textModel->setGeometry(geometry);
		d->textModel->setWorld(getWorld());
		d->textModel->setText(d->text.c_str());
		d->textModel->onAppendingObjectToWorld();
		GM.initObjectPainter(d->textModel->getModel());
		GMAssets::createIsolatedAsset(GMAssetType::Model, d->textModel);
	}
}

//////////////////////////////////////////////////////////////////////////
GMImage2DGameObject* GMListbox2DGameObject::addItem(const GMString& text)
{
	D(d);
	GMImage2DGameObject* item = new GMImage2DGameObject(this);
	item->setText(text);

	GMRect geo = getGeometry();
	geo.x = geo.y = 0;
	item->setGeometry(geo);
	return item;
}

void GMListbox2DGameObject::setItemMargins(GMint left, GMint top, GMint right, GMint bottom)
{
	D(d);
	d->itemMargins[Left] = left;
	d->itemMargins[Top] = top;
	d->itemMargins[Right] = right;
	d->itemMargins[Bottom] = bottom;
}

void GMListbox2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	shader.setNoDepthTest(true);
}

void GMListbox2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GMControlGameObject);

	Base::onAppendingObjectToWorld();
	auto x = db->geometry.x + d->itemMargins[Left],
		y = db->geometry.y + d->itemMargins[Top];

	for (auto item : getItems())
	{
		static GMRect rect;
		rect.x = x;
		rect.y = y;
		rect.width = item->getGeometry().width - d->itemMargins[Left] - d->itemMargins[Right];
		rect.height = item->getGeometry().height;
		item->setGeometry(rect);
		item->setWorld(getWorld());
		item->onAppendingObjectToWorld();
		GM.initObjectPainter(item->getModel());

		y += rect.height + d->itemMargins[Top] + d->itemMargins[Bottom];
	}
}

void GMListbox2DGameObject::draw()
{
	D(d);
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->beginCreateStencil();
	Base::draw();
	engine->endCreateStencil();

	engine->beginUseStencil(false);
	for (auto item : getItems())
	{
		item->draw();
	}
	engine->endUseStencil();
}

void GMListbox2DGameObject::notifyControl()
{
	D(d);
	Base::notifyControl();
	for (auto item : getItems())
	{
		item->notifyControl();
	}
}