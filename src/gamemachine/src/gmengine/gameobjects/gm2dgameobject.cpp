#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "gmtypoengine.h"
#include "gmengine/gameobjects/gmgameobject_p.h"
#include "gm2dgameobject_p.h"

BEGIN_NS

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
	return r.width >= 0;
}

#define BEGIN_TO_VIEWPORT(clientRect) const auto& cw = clientRect.width, &ch = clientRect.height;
#define TO_VIEWPORT_X(i) ((i) * 2.f / cw)
#define TO_VIEWPORT_Y(i) ((i) * 2.f / ch)
#define END_GEOMETRY_TO_VIEWPORT()
#define UV_INDEX(points) points[0], points[1], 0
#define EXTENTS_INDEX(points) points[0], points[1], 1

#define BEGIN_GLYPH_XY(resolutionWidth, resolutionHeight) { GMfloat __resolutionWidth = resolutionWidth, __resolutionHeight = resolutionHeight;
#define X(i) (i) * 2 / __resolutionWidth
#define Y(i) (i) * 2 / __resolutionHeight
#define END_GLYPH_XY() }

#define UV_X(i) ((i) / (GMfloat)GMGlyphManager::CANVAS_WIDTH)
#define UV_Y(i) ((i) / (GMfloat)GMGlyphManager::CANVAS_HEIGHT)

GMRectF GM2DGameObjectBase::toViewportRect(const GMRect& rc, const GMRect& renderRc)
{
	// 得到一个原点在中心，x属于[-1,1],y属于[-1,1]范围的参考系的坐标
	GMRectF out = {
		rc.x * 2.f / renderRc.width - 1.f,
		1.f - rc.y * 2.f / renderRc.height,
		rc.width * 2.f / renderRc.width,
		rc.height * 2.f / renderRc.height
	};
	return out;
}

GM_PRIVATE_OBJECT_UNALIGNED(GM2DGameObjectBase)
{
	GMRect renderRc;
	bool dirty = true;
	GMRect geometry = { 0 };
};

GM2DGameObjectBase::GM2DGameObjectBase(const GMRect& renderRc)
{
	GM_CREATE_DATA();
	D(d);
	d->renderRc = renderRc;
}

GM2DGameObjectBase::~GM2DGameObjectBase()
{

}

bool GM2DGameObjectBase::canDeferredRendering()
{
	return false;
}

void GM2DGameObjectBase::setGeometry(const GMRect& geometry)
{
	D(d);
	if (d->geometry != geometry)
	{
		markDirty();
		d->geometry = geometry;
	}
}


GMShader& GM2DGameObjectBase::getShader()
{
	return getModel()->getShader();
}


const GMRect& GM2DGameObjectBase::getGeometry()
{
	D(d);
	return d->geometry;
}

void GM2DGameObjectBase::initShader(GMShader& shader)
{
	GMGlyphManager* glyphManager = getContext()->getEngine()->getGlyphManager();
	auto& frame = shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	frame.setMinFilter(GMS_TextureFilter::Linear);
	frame.setMagFilter(GMS_TextureFilter::Linear);
	frame.addFrame(glyphManager->glyphTexture());
	shader.setNoDepthTest(true);
	shader.setCull(GMS_Cull::NoCull);
	shader.setBlend(true);
	shader.setBlendFactorSource(GMS_BlendFunc::SourceAlpha);
	shader.setBlendFactorDest(GMS_BlendFunc::OneMinusSourceAlpha);
}

const GMRect& GM2DGameObjectBase::getRenderRect() GM_NOEXCEPT
{
	D(d);
	return d->renderRc;
}

void GM2DGameObjectBase::markDirty() GM_NOEXCEPT
{
	D(d);
	d->dirty = true;
}

void GM2DGameObjectBase::cleanDirty() GM_NOEXCEPT
{
	D(d);
	d->dirty = false;
}

bool GM2DGameObjectBase::isDirty() GM_NOEXCEPT
{
	D(d);
	return d->dirty;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMTextGameObject)
{
	GM_DECLARE_PUBLIC(GMTextGameObject)

	GMString text;
	GMint32 lineSpacing = 0;
	bool center = false;
	bool newline = true;
	GMsize_t length = 0;
	GMAsset texture;
	ITypoEngine* typoEngine = nullptr;
	bool insetTypoEngine = true;
	GMFontHandle font = 0;
	GMScene* scene = nullptr;
	GMTextColorType colorType = GMTextColorType::ByScript;
	GMFloat4 color = GMFloat4(1, 1, 1, 1);
	GMFontSizePt fontSize = 12;
	Vector<GMVertex> vericesCache;
	GMTypoTextBuffer* textBuffer = nullptr;
	GMTextDrawMode drawMode = GMTextDrawMode::Immediate;

	void update();
	GMScene* createScene();
	GMModel* createModel();
	void updateVertices(GMScene* scene);
};

void GMTextGameObjectPrivate::update()
{
	P_D(pd);
	// 如果不存在model，创建一个新model
	// 如果需要的空间更大，也重新创建一个model
	if (!scene || length < text.length())
	{
		scene = createScene();
		length = text.length();
		pd->markDirty();
	}

	// 如果字符被更改，则更新其缓存
	if (pd->isDirty())
	{
		updateVertices(scene);
		pd->cleanDirty();
	}
}

GMScene* GMTextGameObjectPrivate::createScene()
{
	P_D(pd);
	GMModel* model = createModel();
	pd->initShader(model->getShader());

	GMPart* part = new GMPart(model);
	GMsize_t len = text.length() * 6; //每个字符，用6个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		part->vertex(GMVertex());
	}

	pd->getContext()->getEngine()->createModelDataProxy(pd->getContext(), model);
	return pd->getScene();
}

void GMTextGameObjectPrivate::updateVertices(GMScene* scene)
{
	P_D(pd);
	if (!typoEngine)
		typoEngine = new GMTypoEngine(pd->getContext());

	GMModel* model = scene->getModels()[0].getModel();
	constexpr GMfloat Z = 0;
	const GMRect& rect = pd->getRenderRect();
	GMRectF coord = pd->toViewportRect(pd->getGeometry(), rect);

	Vector<GMVertex>& vertices = vericesCache;
	BEGIN_GLYPH_XY(rect.width, rect.height)
	GMTypoIterator iter;

	GMTypoOptions options;
	if (drawMode == GMTextDrawMode::Immediate)
	{
		// 使用排版引擎进行排版
		options.useCache = false;
		options.newline = newline;
		options.center = center;
		options.lineSpacing = lineSpacing;
		options.typoArea.width = coord.width * rect.width * .5f;
		options.typoArea.height = coord.height * rect.height * .5f;
		options.plainText = colorType == GMTextColorType::Plain;

		typoEngine->setFont(font);
		typoEngine->setLineHeight(0);
		typoEngine->setFontSize(fontSize);
		iter = typoEngine->begin(text, options);
	}
	else
	{
		// 使用已有的typoEngine来排版，这样就不用重头开始排版了
		// 我们不会更改typoEngine的值，但是还是要const_cast一下，不然编译器会不高兴
		typoEngine = const_cast<ITypoEngine*>(textBuffer->getTypoEngine());
		options.plainText = textBuffer->isPlainText();
		options.useCache = true;
		options.renderStart = textBuffer->getRenderStart();
		options.renderEnd = textBuffer->getRenderEnd();
		typoEngine->setFontSize(fontSize);
		iter = typoEngine->begin(text, options);
	}

	GM_ASSERT(typoEngine);
	const GMfloat *pResultColor = ValuePointer(color);

	auto lineHeight = typoEngine->getResults().lineHeight;
	auto end = typoEngine->end();
	for (; iter != end; ++iter)
	{
		const GMTypoResult& typoResult = *iter;
		if (!typoResult.valid || typoResult.newLineOrEOFSeparator)
			continue;

		const GMGlyphInfo& glyph = *typoResult.glyph;
		if (!options.plainText)
		{
			//富文本的情况，使用推导出来的颜色
			pResultColor = typoResult.color;
		}

		if (glyph.width > 0 && glyph.height > 0)
		{
			// 如果width和height为0，视为空格，只占用空间而已
			// 否则：按照TriangleList创建顶点：0 2 1, 1 2 3
			// 0 2
			// 1 3
			// 让所有字体origin开始的x轴平齐

			// 采用左上角为原点的Texcoord坐标系

			GMVertex V0 = {
				{ coord.x + X(typoResult.x), coord.y - Y(typoResult.y + lineHeight - glyph.bearingY), Z },
				{ 0 },
				{ UV_X(glyph.x), UV_Y(glyph.y) },
				{ 0 },
				{ 0 },
				{ 0 },
				{ pResultColor[0], pResultColor[1], pResultColor[2], pResultColor[3] }
			};
			GMVertex V1 = {
				{ coord.x + X(typoResult.x), coord.y - Y(typoResult.y + lineHeight - (glyph.bearingY - glyph.height)), Z },
				{ 0 },
				{ UV_X(glyph.x), UV_Y(glyph.y + glyph.height) },
				{ 0 },
				{ 0 },
				{ 0 },
				{ pResultColor[0], pResultColor[1], pResultColor[2], pResultColor[3] }
			};
			GMVertex V2 = {
				{ coord.x + X(typoResult.x + typoResult.width), coord.y - Y(typoResult.y + lineHeight - glyph.bearingY), Z },
				{ 0 },
				{ UV_X(glyph.x + glyph.width), UV_Y(glyph.y) },
				{ 0 },
				{ 0 },
				{ 0 },
				{ pResultColor[0], pResultColor[1], pResultColor[2], pResultColor[3] }
			};
			GMVertex V3 = {
				{ coord.x + X(typoResult.x + typoResult.width), coord.y - Y(typoResult.y + lineHeight - (glyph.bearingY - glyph.height)), Z },
				{ 0 },
				{ UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height) },
				{ 0 },
				{ 0 },
				{ 0 },
				{ pResultColor[0], pResultColor[1], pResultColor[2], pResultColor[3] }
			};

			vertices.push_back(V0);
			vertices.push_back(V2);
			vertices.push_back(V1);
			vertices.push_back(V1);
			vertices.push_back(V2);
			vertices.push_back(V3);
		}
	}
	END_GLYPH_XY()

	// 已经得到所有顶点，更新到GPU
	GM_ASSERT(vertices.size() <= text.length() * 6);
	GMsize_t vertexCount = vertices.size();
	model->setVerticesCount(vertexCount);

	GMsize_t sz = vertexCount * sizeof(GMVertex);
	GMModelDataProxy* proxy = model->getModelDataProxy();
	proxy->beginUpdateBuffer();
	void* ptr = proxy->getBuffer();
	memcpy_s(ptr, sz, vertices.data(), sz);
	proxy->endUpdateBuffer();
	vertices.clear();
}


GMModel* GMTextGameObjectPrivate::createModel()
{
	P_D(pd);
	GMModel* model = new GMModel();
	pd->setAsset(GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, model)));
	model->setType(GMModelType::Text);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	return model;
}

GMTextGameObject::GMTextGameObject(const GMRect& renderRc)
	: GM2DGameObjectBase(renderRc)
{
	GM_CREATE_DATA();
	GM_SET_PD();
}

GMTextGameObject::GMTextGameObject(const GMRect& renderRc, ITypoEngine* typo)
	: GM2DGameObjectBase(renderRc)
{
	GM_CREATE_DATA();
	GM_SET_PD();
	D(d);
	d->typoEngine = typo;
	d->insetTypoEngine = false;
}

GMTextGameObject::~GMTextGameObject()
{
	D(d);
	if (d->insetTypoEngine)
		GM_delete(d->typoEngine);
}

void GMTextGameObject::setText(const GMString& text)
{
	D(d);
	if (d->text != text)
	{
		markDirty();
		d->text = text;
	}
}

void GMTextGameObject::setColorType(GMTextColorType type) GM_NOEXCEPT
{
	D(d);
	if (d->colorType != type)
	{
		markDirty();
		d->colorType = type;
	}
}

void GMTextGameObject::setColor(const GMVec4& color)
{
	D(d);
	if (d->colorType != GMTextColorType::Plain)
	{
		gm_error(gm_dbg_wrap("Please set color type to GMTextColorType::Plain before you set color. Otherwise it won't work."));
		GM_ASSERT(false);
	}

	GMFloat4 colorCache;
	color.loadFloat4(colorCache);
	if (colorCache[0] != d->color[0] ||
		colorCache[1] != d->color[1] ||
		colorCache[2] != d->color[2] || 
		colorCache[3] != d->color[3])
	{
		markDirty();
		d->color = colorCache;
	}
}


GMVec4 GMTextGameObject::getColor()
{
	D(d);
	GMVec4 color;
	color.setFloat4(d->color);
	return color;
}

void GMTextGameObject::setCenter(bool center) GM_NOEXCEPT
{
	D(d);
	if (d->center != center)
	{
		d->center = center;
		markDirty();
	}
}

void GMTextGameObject::setFont(GMFontHandle font) GM_NOEXCEPT
{
	D(d);
	if (d->font != font)
	{
		d->font = font;
		markDirty();
	}
}

void GMTextGameObject::setNewline(bool newline) GM_NOEXCEPT
{
	D(d);
	if (d->newline != newline)
	{
		d->newline = newline;
		markDirty();
	}
}

void GMTextGameObject::setLineSpacing(GMint32 lineSpacing) GM_NOEXCEPT
{
	D(d);
	if (d->lineSpacing != lineSpacing)
	{
		d->lineSpacing = lineSpacing;
		markDirty();
	}
}


void GMTextGameObject::setFontSize(GMFontSizePt fontSize) GM_NOEXCEPT
{
	D(d);
	if (d->fontSize != fontSize)
	{
		d->fontSize = fontSize;
		markDirty();
	}
}

void GMTextGameObject::setTextBuffer(GMTypoTextBuffer* textBuffer) GM_NOEXCEPT
{
	D(d);
	if (d->textBuffer != textBuffer || d->drawMode != GMTextDrawMode::UseBuffer)
	{
		d->textBuffer = textBuffer;
		markDirty();
		setDrawMode(GMTextDrawMode::UseBuffer);

		// 一定要调用setText，因为在更新顶点的时候会计算Buffer是否够空间，否则会造成缓存区溢出
		setText(textBuffer->getBuffer());
	}
}

void GMTextGameObject::setDrawMode(GMTextDrawMode mode) GM_NOEXCEPT
{
	D(d);
	if (d->drawMode != mode)
	{
		d->drawMode = mode;
		markDirty();
	}
}

ITypoEngine* GMTextGameObject::getTypoEngine() GM_NOEXCEPT
{
	D(d);
	return d->typoEngine;
}

GMModel* GMTextGameObject::getModel()
{
	D(d);
	if (getAsset().isEmpty())
		d->createModel();

	return GMGameObject::getModel();
}

void GMTextGameObject::onAppendingObjectToWorld()
{
	D(d);
	d->update();
	GMGameObject::onAppendingObjectToWorld();
}

void GMTextGameObject::draw()
{
	D(d);
	d->update();

	GMModel* model = d->scene->getModels()[0].getModel();
	drawModel(getContext(), model);
	endDraw();
}

void GMSprite2DGameObjectPrivate::update()
{
	P_D(pd);
	if (!scene)
	{
		scene = pd->createScene();
		pd->markDirty();
	}

	if (needUpdateTexture)
	{
		pd->updateTexture(scene);
		needUpdateTexture = false;
	}

	if (pd->isDirty())
	{
		pd->updateVertices(scene);
		pd->cleanDirty();
	}
}

GMSprite2DGameObject::GMSprite2DGameObject(const GMRect& renderRc)
	: Base(renderRc)
{
	GM_CREATE_DATA();
	GM_SET_PD();
}

GMSprite2DGameObject::~GMSprite2DGameObject()
{

}

void GMSprite2DGameObject::draw()
{
	D(d);
	d->update();
	GMModel* model = d->scene->getModels()[0].getModel();
	drawModel(getContext(), model);
	endDraw();
}

void GMSprite2DGameObject::setDepth(GMfloat depth)
{
	D(d);
	if (d->depth != depth)
	{
		d->depth = depth;
		markDirty();
	}
}

void GMSprite2DGameObject::setTexture(GMAsset tex)
{
	D(d);
	if (!tex.isEmpty() && d->texture != tex)
	{
		d->texture = tex;
		d->needUpdateTexture = true;
	}
}

void GMSprite2DGameObject::setTextureSize(GMint32 width, GMint32 height)
{
	D(d);
	if (d->texWidth != width || d->texHeight != height)
	{
		d->texWidth = width;
		d->texHeight = height;
		markDirty();
	}
}

void GMSprite2DGameObject::setTextureRect(const GMRect& rect)
{
	D(d);
	if (d->textureRc != rect)
	{
		d->textureRc = rect;
		markDirty();
	}
}

void GMSprite2DGameObject::setColor(const GMVec4& color)
{
	D(d);
	GMFloat4 colorCache;
	color.loadFloat4(colorCache);
	if (colorCache[0] != d->color[0] ||
		colorCache[1] != d->color[1] ||
		colorCache[2] != d->color[2] ||
		colorCache[3] != d->color[3])
	{
		markDirty();
		d->color = colorCache;
	}
}

GMScene* GMSprite2DGameObject::createScene()
{
	D(d);
	D_BASE(db, GMGameObject);
	GMModel* model = new GMModel();
	db->asset = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, model));
	model->setType(GMModelType::Model2D);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	model->getShader().setVertexColorOp(GMS_VertexColorOp::Multiply);
	initShader(model->getShader());
	GMPart* part = new GMPart(model);
	GMsize_t len = VerticesCount; //每个Sprite，用4个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		part->vertex(GMVertex());
	}
	getContext()->getEngine()->createModelDataProxy(getContext(), model);

	return db->asset.getScene();
}

void GMSprite2DGameObject::updateVertices(GMScene* scene)
{
	D(d);
	GMModel* model = scene->getModels()[0].getModel();

	GM_ASSERT(d->texWidth > 0 && d->texHeight > 0);
	GMRectF coord = toViewportRect(getGeometry(), getRenderRect());
	// 按照以下方式组织节点
	// 1 3
	// 0 2
	GMfloat t0[] = { d->textureRc.x / (GMfloat)d->texWidth, (d->textureRc.y + d->textureRc.height) / (GMfloat)d->texHeight };
	GMfloat t1[] = { d->textureRc.x / (GMfloat)d->texWidth, d->textureRc.y / (GMfloat)d->texHeight };
	GMfloat t2[] = { (d->textureRc.x + d->textureRc.width) / (GMfloat)d->texWidth, (d->textureRc.y + d->textureRc.height) / (GMfloat)d->texHeight };
	GMfloat t3[] = { (d->textureRc.x + d->textureRc.width) / (GMfloat)d->texWidth, d->textureRc.y / (GMfloat)d->texHeight };
	GMVertex V[] = {
			{
			{ coord.x, coord.y - coord.height , d->depth },
			{ 0 },
			{ t0[0], t0[1] },
			{ 0 },
			{ 0 },
			{ 0 },
			{ d->color[0], d->color[1], d->color[2], d->color[3] }
		},
		{
			{ coord.x, coord.y , d->depth },
			{ 0 },
			{ t1[0], t1[1] },
			{ 0 },
			{ 0 },
			{ 0 },
			{ d->color[0], d->color[1], d->color[2], d->color[3] }
		},
		{
			{ coord.x + coord.width, coord.y - coord.height , d->depth },
			{ 0 },
			{ t2[0], t2[1] },
			{ 0 },
			{ 0 },
			{ 0 },
			{ d->color[0], d->color[1], d->color[2], d->color[3] }
		},
		{
			{ coord.x + coord.width, coord.y, d->depth },
			{ 0 },
			{ t3[0], t3[1] },
			{ 0 },
			{ 0 },
			{ 0 },
			{ d->color[0], d->color[1], d->color[2], d->color[3] }
		}
	};

	GMsize_t sz = sizeof(V);
	GMModelDataProxy* proxy = model->getModelDataProxy();
	proxy->beginUpdateBuffer();
	void* ptr = proxy->getBuffer();
	memcpy_s(ptr, sz, V, sz);
	proxy->endUpdateBuffer();
}

void GMSprite2DGameObject::updateTexture(GMScene* scene)
{
	D(d);
	GM_ASSERT(d->scene);
	GMModel* model = d->scene->getModels()[0].getModel();
	GMShader& shader = model->getShader();
	GMTextureSampler& texSampler = shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	texSampler.setTexture(0, d->texture);
}

//////////////////////////////////////////////////////////////////////////
// i1 i2
// i3 i4
#define INDICES(part, i1, i2, i3, i4) \
	{ part->index(i3); part->index(i1); part->index(i4); } \
	{ part->index(i4); part->index(i1); part->index(i2); }


GM_PRIVATE_OBJECT_UNALIGNED(GMBorder2DGameObject)
{
	GMModel* model = nullptr;
	GMRect corner;
};

GMBorder2DGameObject::GMBorder2DGameObject(const GMRect& renderRc)
	: Base(renderRc)
{
	GM_CREATE_DATA();
}

GMBorder2DGameObject::~GMBorder2DGameObject()
{

}

void GMBorder2DGameObject::setCornerRect(const GMRect& rc)
{
	D(d);
	if (d->corner != rc)
	{
		d->corner = rc;
		markDirty();
	}
}

GMScene* GMBorder2DGameObject::createScene()
{
	D(d);
	D_BASE(db, GMGameObject);
	GMModel* model = new GMModel();
	db->asset = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, model));
	model->setType(GMModelType::Model2D);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	model->setDrawMode(GMModelDrawMode::Index);
	model->getShader().setVertexColorOp(GMS_VertexColorOp::Multiply);
	initShader(model->getShader());
	GMPart* part = new GMPart(model);
	GMsize_t len = VerticesCount; //9个四边形，使用索引绘制，一共16个顶点
	for (GMsize_t i = 0; i < len; ++i)
	{
		part->vertex(GMVertex());
	}

	// 逆时针方向绘制
	// 第1行
	INDICES(part, 0, 1, 4, 5);
	INDICES(part, 1, 2, 5, 6);
	INDICES(part, 2, 3, 6, 7);

	// 第2行
	INDICES(part, 4, 5, 8, 9);
	INDICES(part, 5, 6, 9, 10);
	INDICES(part, 6, 7, 10, 11);

	// 第3行
	INDICES(part, 8, 9, 12, 13);
	INDICES(part, 9, 10, 13, 14);
	INDICES(part, 10, 11, 14, 15);

	getContext()->getEngine()->createModelDataProxy(getContext(), model);
	return db->asset.getScene();
}

void GMBorder2DGameObject::updateVertices(GMScene* scene)
{
	D(d);
	D_BASE(db, Base);

	GMModel* model = scene->getModels()[0].getModel();

	GM_ASSERT(db->texWidth > 0 && db->texHeight > 0);
	const GMRect& rc = getGeometry();
	GMRectF coord = toViewportRect(rc, getRenderRect());
	GMRectF cornerCoord = toViewportRect(d->corner, getRenderRect());

	/*
	以下是个纹理，以像素为单位。
		center
	 _ ________ _
	|_|________|_| corner
	
	*/

	const GMfloat& cornerWidth = d->corner.width;
	const GMfloat& cornerWidthX2 = d->corner.width * 2;
	const GMfloat& cornerHeight = d->corner.height;
	const GMfloat& cornerHeightX2 = d->corner.height * 2;
	const GMfloat& centerWidth = db->textureRc.width - cornerWidthX2;
	const GMfloat& centerHeight = db->textureRc.height - cornerHeightX2;

	const GMfloat& cornerCoordWidth = cornerCoord.width;
	const GMfloat& cornerCoordHeight = cornerCoord.height;
	const GMfloat& cornerCoordWidthX2 = cornerCoord.width * 2;
	const GMfloat& cornerCoordHeightX2 = cornerCoord.height * 2;
	const GMfloat& centerCoordWidth = coord.width - cornerCoordWidthX2;
	const GMfloat& centerCoordHeight = coord.height - cornerCoordHeightX2;

	// Arrangement:
	// 0 1 2 3
	// 4 5 6 7
	// 8 9 A B
	// C D E F

	GMVertex V[] = {
		// 0
		{
			{ coord.x, coord.y, db->depth },
			{ 0 },
			{ (db->textureRc.x) / (GMfloat)db->texWidth, (db->textureRc.y) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 1
		{
			{ coord.x + cornerCoordWidth, coord.y, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth) / (GMfloat)db->texWidth, (db->textureRc.y) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 2
		{
			{ coord.x + cornerCoordWidth + centerCoordWidth, coord.y, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 3
		{
			{ coord.x + cornerCoordWidthX2 + centerCoordWidth, coord.y, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidthX2 + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 4
		{
			{ coord.x, coord.y - cornerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 5
		{
			{ coord.x + cornerCoordWidth, coord.y - cornerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 6
		{
			{ coord.x + cornerCoordWidth + centerCoordWidth, coord.y - cornerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 7
		{
			{ coord.x + cornerCoordWidthX2 + centerCoordWidth, coord.y - cornerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidthX2 + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 8
		{
			{ coord.x, coord.y - cornerCoordHeight - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// 9
		{
			{ coord.x + cornerCoordWidth, coord.y - cornerCoordHeight - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// A
		{
			{ coord.x + cornerCoordWidth + centerCoordWidth, coord.y - cornerCoordHeight - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// B
		{
			{ coord.x + cornerCoordWidthX2 + centerCoordWidth, coord.y - cornerCoordHeight - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidthX2 + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeight + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// C
		{
			{ coord.x, coord.y - cornerCoordHeightX2 - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeightX2 + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// D
		{
			{ coord.x + cornerCoordWidth, coord.y - cornerCoordHeightX2 - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeightX2 + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// E
		{
			{ coord.x + cornerCoordWidth + centerCoordWidth, coord.y - cornerCoordHeightX2 - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidth + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeightX2 + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		},

		// F
		{
			{ coord.x + cornerCoordWidthX2 + centerCoordWidth, coord.y - cornerCoordHeightX2 - centerCoordHeight, db->depth },
			{ 0 },
			{ (db->textureRc.x + cornerWidthX2 + centerWidth) / (GMfloat)db->texWidth, (db->textureRc.y + cornerHeightX2 + centerHeight) / (GMfloat)db->texHeight },
			{ 0 },
			{ 0 },
			{ 0 },
			{ db->color[0], db->color[1], db->color[2], db->color[3] }
		}
	};

	// 处理重叠的情况
	// 宽度重叠
	if (V[2].positions[0] < V[1].positions[0])
	{
		GMfloat mid = (V[3].positions[0] - V[0].positions[0]) / 2;
		GMfloat adjusted = V[0].positions[0] + mid;
		V[1].positions[0] = V[2].positions[0] = V[5].positions[0] = V[6].positions[0] =
		V[9].positions[0] = V[10].positions[0] = V[13].positions[0] = V[14].positions[0] = adjusted;
	}

	// 高度重叠
	if (V[4].positions[1] < V[8].positions[1])
	{
		GMfloat mid = (V[12].positions[1] - V[0].positions[1]) / 2;
		GMfloat adjusted = V[0].positions[1] + mid;
		for (auto i = 4; i <= 11; ++i)
		{
			V[i].positions[1] = adjusted;
		}
	}

	GMsize_t sz = sizeof(V);
	GMModelDataProxy* proxy = model->getModelDataProxy();
	proxy->beginUpdateBuffer();
	void* ptr = proxy->getBuffer();
	memcpy_s(ptr, sz, V, sz);
	proxy->endUpdateBuffer();
}

END_NS