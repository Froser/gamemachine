#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "gmtypoengine.h"

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

GM2DGameObjectBase::GM2DGameObjectBase(const GMRect& renderRc)
{
	D(d);
	d->renderRc = renderRc;
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

void GM2DGameObjectBase::setShader(GMShader& shader)
{
	GMGlyphManager* glyphManager = getContext()->engine->getGlyphManager();
	auto& frame = shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	frame.setMinFilter(GMS_TextureFilter::LINEAR);
	frame.setMagFilter(GMS_TextureFilter::LINEAR);
	frame.addFrame(glyphManager->glyphTexture());
	shader.setNoDepthTest(true);
	shader.setCull(GMS_Cull::NONE);
	shader.setBlend(true);
	shader.setBlendFactorSource(GMS_BlendFunc::SRC_ALPHA);
	shader.setBlendFactorDest(GMS_BlendFunc::ONE_MINUS_SRC_ALPHA);
}

GMTextGameObject::GMTextGameObject(const GMRect& renderRc)
	: GM2DGameObjectBase(renderRc)
{
}

GMTextGameObject::GMTextGameObject(const GMRect& renderRc, ITypoEngine* typo)
	: GM2DGameObjectBase(renderRc)
{
	D(d);
	d->typoEngine = typo;
	d->insetTypoEngine = false;
}

GMTextGameObject::~GMTextGameObject()
{
	D(d);
	GM_delete(d->model);
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

void GMTextGameObject::setColorType(GMTextColorType type)
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
		gm_error("Please set color type to GMTextColorType::Plain before you set color. Otherwise it won't work.");
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

void GMTextGameObject::setCenter(bool center)
{
	D(d);
	if (d->center != center)
	{
		d->center = center;
		markDirty();
	}
}

void GMTextGameObject::onAppendingObjectToWorld()
{
	D(d);
	update();
	GMGameObject::onAppendingObjectToWorld();
}

void GMTextGameObject::draw()
{
	D(d);
	update();
	drawModel(getContext(), d->model);
}

void GMTextGameObject::update()
{
	D(d);
	// 如果不存在model，创建一个新model
	// 如果需要的空间更大，也重新创建一个model
	if (!d->model || d->length < d->text.length())
	{
		GM_delete(d->model);
		d->model = createModel();
		d->length = d->text.length();
		markDirty();
	}

	// 如果字符被更改，则更新其缓存
	if (isDirty())
	{
		updateVertices(d->model);
		cleanDirty();
	}
}

GMModel* GMTextGameObject::createModel()
{
	D(d);
	GMModel* model = new GMModel();
	model->setType(GMModelType::Text);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::Triangles);
	setShader(model->getShader());

	GMMesh* mesh = new GMMesh(model);
	GMsize_t len = d->text.length() * 6; //每个字符，用6个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		mesh->vertex(GMVertex());
	}
	GM.createModelDataProxyAndTransfer(getContext(), model);
	return model;
}

void GMTextGameObject::updateVertices(GMModel* model)
{
	D(d);
	if (!d->typoEngine)
		d->typoEngine = new GMTypoEngine(getContext());

	constexpr GMfloat Z = 0;
	const GMRect& rect = getRenderRect();
	GMRectF coord = toViewportRect(getGeometry(), getRenderRect());

	Vector<GMVertex>& vertices = d->vericesCache;
	BEGIN_GLYPH_XY(rect.width, rect.height)
		// 使用排版引擎进行排版
		ITypoEngine* typoEngine = d->typoEngine;
		GMTypoOptions options;
		options.center = d->center;
		options.typoArea.width = coord.width * rect.width * .5f;
		options.typoArea.height = coord.height * rect.height * .5f;

		GM_ASSERT(typoEngine);
		const GMfloat *pResultColor = ValuePointer(d->color);

		GMTypoIterator iter = typoEngine->begin(d->text, options);
		for (; iter != typoEngine->end(); ++iter)
		{
			const GMTypoResult& typoResult = *iter;
			if (!typoResult.valid)
				continue;

			const GMGlyphInfo& glyph = *typoResult.glyph;
			if (d->colorType == GMTextColorType::ByScript)
				pResultColor = typoResult.color;

			if (glyph.width > 0 && glyph.height > 0)
			{
				// 如果width和height为0，视为空格，只占用空间而已
				// 否则：按照TriangleList创建顶点：0 2 1, 1 2 3
				// 0 2
				// 1 3
				// 让所有字体origin开始的x轴平齐

				// 采用左上角为原点的Texcoord坐标系

				GMVertex V0 = {
					{ coord.x + X(typoResult.x), coord.y  - Y(typoResult.y + typoResult.lineHeight - glyph.bearingY), Z },
					{ 0 },
					{ UV_X(glyph.x), UV_Y(glyph.y) },
					{ 0 },
					{ 0 },
					{ 0 },
					{ pResultColor[0], pResultColor[1], pResultColor[2] }
				};
				GMVertex V1 = {
					{ coord.x + X(typoResult.x), coord.y  - Y(typoResult.y + typoResult.lineHeight - (glyph.bearingY - glyph.height)), Z },
					{ 0 },
					{ UV_X(glyph.x), UV_Y(glyph.y + glyph.height) },
					{ 0 },
					{ 0 },
					{ 0 },
					{ pResultColor[0], pResultColor[1], pResultColor[2] }
				};
				GMVertex V2 = {
					{ coord.x + X(typoResult.x + typoResult.width), coord.y - Y(typoResult.y + typoResult.lineHeight - glyph.bearingY), Z },
					{ 0 },
					{ UV_X(glyph.x + glyph.width), UV_Y(glyph.y) },
					{ 0 },
					{ 0 },
					{ 0 },
					{ pResultColor[0], pResultColor[1], pResultColor[2] }
				};
				GMVertex V3 = {
					{ coord.x + X(typoResult.x + typoResult.width), coord.y - Y(typoResult.y + typoResult.lineHeight - (glyph.bearingY - glyph.height)), Z },
					{ 0 },
					{ UV_X(glyph.x + glyph.width), UV_Y(glyph.y + glyph.height) },
					{ 0 },
					{ 0 },
					{ 0 },
					{ pResultColor[0], pResultColor[1], pResultColor[2] }
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
	GM_ASSERT(vertices.size() <= d->text.length() * 6);
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

GMSprite2DGameObject::~GMSprite2DGameObject()
{
	D(d);
	GM_delete(d->model);
}

void GMSprite2DGameObject::draw()
{
	D(d);
	update();
	drawModel(getContext(), d->model);
}

void GMSprite2DGameObject::setDepth(GMint depth)
{
	D(d);
	if (d->depth != depth)
	{
		d->depth = depth;
		markDirty();
	}
}

void GMSprite2DGameObject::setTexture(ITexture* tex)
{
	D(d);
	if (d->texture != tex)
	{
		d->texture = tex;
		d->needUpdateTexture = true;
	}
}

void GMSprite2DGameObject::setTextureSize(GMint width, GMint height)
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

void GMSprite2DGameObject::update()
{
	D(d);
	if (!d->model)
	{
		GM_delete(d->model);
		d->model = createModel();
		markDirty();
	}

	if (d->needUpdateTexture)
	{
		updateTexture(d->model);
		d->needUpdateTexture = false;
	}

	if (isDirty())
	{
		updateVertices(d->model);
		cleanDirty();
	}
}

GMModel* GMSprite2DGameObject::createModel()
{
	D(d);
	GMModel* model = new GMModel();
	model->setType(GMModelType::Model2D);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::TriangleStrip);
	setShader(model->getShader());
	GMMesh* mesh = new GMMesh(model);
	GMsize_t len = VerticesCount; //每个Sprite，用4个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		mesh->vertex(GMVertex());
	}
	GM.createModelDataProxyAndTransfer(getContext(), model);
	return model;
}

void GMSprite2DGameObject::updateVertices(GMModel* model)
{
	D(d);
	GM_ASSERT(d->texWidth > 0 && d->texHeight > 0);
	GMRectF coord = toViewportRect(getGeometry(), getRenderRect());
	// 按照以下方式组织节点
	// 1 3
	// 0 2
	GMfloat t0[] = { d->textureRc.x / (GMfloat)d->texWidth, (d->textureRc.y + d->textureRc.height) / (GMfloat)d->texHeight };
	GMfloat t1[] = { d->textureRc.x / (GMfloat)d->texWidth, d->textureRc.y / (GMfloat)d->texHeight };
	GMfloat t2[] = { (d->textureRc.x + d->textureRc.width) / (GMfloat)d->texWidth, (d->textureRc.y + d->textureRc.height) / (GMfloat)d->texHeight };
	GMfloat t3[] = { (d->textureRc.x + d->textureRc.width) / (GMfloat)d->texWidth, d->textureRc.y / (GMfloat)d->texHeight };
	GMVertex V[4] = {
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

void GMSprite2DGameObject::updateTexture(GMModel* model)
{
	D(d);
	GM_ASSERT(d->model);
	GMShader& shader = d->model->getShader();
	GMTextureSampler& texSampler = shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	texSampler.setTexture(0, d->texture);
}
