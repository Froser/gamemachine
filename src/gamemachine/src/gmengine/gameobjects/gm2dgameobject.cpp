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

GMRectF GM2DGameObjectBase::toViewportRect(const GMRect& rc)
{
	// 得到一个原点在中心，x属于[-1,1],y属于[-1,1]范围的参考系的坐标
	const GMRect& client = GM.getGameMachineRunningStates().renderRect;
	GMRectF out = {
		rc.x * 2.f / client.width - 1.f,
		1.f - rc.y * 2.f / client.height,
		rc.width * 2.f / client.width,
		rc.height * 2.f / client.height
	};
	return out;
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
	GMGlyphManager* glyphManager = GM.getGlyphManager();
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

GMTextGameObject::GMTextGameObject()
{
	D(d);
	d->typoEngine = new GMTypoEngine();
}

GMTextGameObject::GMTextGameObject(ITypoEngine* typo)
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
	update();
	GMGameObject::onAppendingObjectToWorld();
}

void GMTextGameObject::draw()
{
	D(d);
	update();
	drawModel(d->model);
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
	GM.createModelDataProxyAndTransfer(model);
	return model;
}

void GMTextGameObject::updateVertices(GMModel* model)
{
	D(d);
	constexpr GMfloat Z = 0;
	const GMRect& rect = GM.getGameMachineRunningStates().renderRect;
	GMRectF coord = toViewportRect(getGeometry());

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
	drawModel(d->model);
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
	GM.createModelDataProxyAndTransfer(model);
	return model;
}

void GMSprite2DGameObject::updateVertices(GMModel* model)
{
	D(d);
	GM_ASSERT(d->texWidth > 0 && d->texHeight > 0);
	GMRectF coord = toViewportRect(getGeometry());
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

//////////////////////////////////////////////////////////////////////////
GMImage2DBorder::GMImage2DBorder(GMAsset texture,
	const GMRect& borderTextureGeometry,
	GMfloat textureWidth,
	GMfloat textureHeight,
	GMfloat cornerWidth,
	GMfloat cornerHeight
)
{
	D(d);
	d->texture = texture;
	d->borderTextureGeometry = borderTextureGeometry;
	d->width = textureWidth;
	d->height = textureHeight;
	d->cornerWidth = cornerWidth;
	d->cornerHeight = cornerHeight;
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

void GMImage2DBorder::clone(const GMImage2DBorder& b)
{
	D(d);
	D_OF(d_b, &b);
	d->texture = d_b->texture;
	d->borderTextureGeometry = d_b->borderTextureGeometry;
	d->width = d_b->width;
	d->height = d_b->height;
	d->cornerWidth = d_b->cornerWidth;
	d->cornerHeight = d_b->cornerHeight;
}

void GMImage2DBorder::createBorder(const GMRect& geometry)
{
	D(d);
	GM_ASSERT(hasBorder());

	// 创建9个区域
	// 0(corner) 3(center) 6(corner)
	// 1(middle)     4     7(middle)
	// 2(corner) 5(center) 8(corner)

	const GMfloat& textureWidth = d->width,
		&textureHeight = d->height;

	const GMRect& textureGeo = textureGeometry();
	GMfloat base[2] = {
		textureGeo.x / (GMfloat)textureWidth,
		textureGeo.y / (GMfloat)textureHeight,
	};

	const GMfloat& center_w_pixel = (textureGeo.width - 2 * d->cornerWidth), &center_h_pixel = d->cornerHeight;
	const GMfloat& middle_w_pixel = d->cornerWidth, &middle_h_pixel = (textureGeo.height - 2 * d->cornerHeight);
	const GMfloat& corner_w = d->cornerWidth / d->width, &corner_h = d->cornerHeight / d->height;
	const GMfloat& corner_w2 = corner_w * 2, &corner_h2 = corner_h * 2;
	const GMfloat& center_w = center_w_pixel / d->width, &center_h = corner_h;
	const GMfloat& middle_w = corner_w, &middle_h = middle_h_pixel / d->height;
	const GMfloat& half_border_width = textureGeo.width / 2,
		&half_border_height = textureGeo.height / 2;

	const GMfloat uv_points[16][2] = {
		{ base[0], base[1] + corner_h2 + middle_h },
		{ base[0], base[1] + corner_h + middle_h },
		{ base[0], base[1] + corner_h },
		{ base[0], base[1] },

		{ base[0] + corner_w, base[1] + corner_h2 + middle_h },
		{ base[0] + corner_w, base[1] + corner_h + middle_h },
		{ base[0] + corner_w, base[1] + corner_h },
		{ base[0] + corner_w, base[1] },

		{ base[0] + corner_w + center_w, base[1] + corner_h2 + middle_h },
		{ base[0] + corner_w + center_w, base[1] + corner_h + middle_h },
		{ base[0] + corner_w + center_w, base[1] + corner_h },
		{ base[0] + corner_w + center_w, base[1] },

		{ base[0] + corner_w2 + center_w, base[1] + corner_h2 + middle_h },
		{ base[0] + corner_w2 + center_w, base[1] + corner_h + middle_h },
		{ base[0] + corner_w2 + center_w, base[1] + corner_h },
		{ base[0] + corner_w2 + center_w, base[1] },
	};

	GMfloat uv[9][12] = {
		UV_INDEX(uv_points[0]), UV_INDEX(uv_points[1]), UV_INDEX(uv_points[5]), UV_INDEX(uv_points[4]),
		UV_INDEX(uv_points[1]), UV_INDEX(uv_points[2]), UV_INDEX(uv_points[6]), UV_INDEX(uv_points[5]),
		UV_INDEX(uv_points[2]), UV_INDEX(uv_points[3]), UV_INDEX(uv_points[7]), UV_INDEX(uv_points[6]),
		UV_INDEX(uv_points[4]), UV_INDEX(uv_points[5]), UV_INDEX(uv_points[9]), UV_INDEX(uv_points[8]),
		UV_INDEX(uv_points[5]), UV_INDEX(uv_points[6]), UV_INDEX(uv_points[10]), UV_INDEX(uv_points[9]),
		UV_INDEX(uv_points[6]), UV_INDEX(uv_points[7]), UV_INDEX(uv_points[11]), UV_INDEX(uv_points[10]),
		UV_INDEX(uv_points[8]), UV_INDEX(uv_points[9]), UV_INDEX(uv_points[13]), UV_INDEX(uv_points[12]),
		UV_INDEX(uv_points[9]), UV_INDEX(uv_points[10]), UV_INDEX(uv_points[14]), UV_INDEX(uv_points[13]),
		UV_INDEX(uv_points[10]), UV_INDEX(uv_points[11]), UV_INDEX(uv_points[15]), UV_INDEX(uv_points[14]),
	};

	const GMRect& w = GM.getGameMachineRunningStates().renderRect;
	GMRectF window = {
		(GMfloat)w.x,
		(GMfloat)w.y,
		(GMfloat)w.width,
		(GMfloat)w.height
	};

	const GMfloat &pos_center_w = geometry.width - 2 * d->cornerWidth,
		&pos_center_h = d->cornerHeight,
		&pos_middle_w = d->cornerWidth,
		&pos_middle_h = geometry.height - 2 * d->cornerHeight;


	GMfloat extentsArray[4][3] = {
		{ d->cornerWidth / window.width, d->cornerHeight / window.height, 1 },
		{ pos_middle_w / window.width, pos_middle_h / window.height, 1 },
		{ pos_center_w / window.width, center_h_pixel / window.height, 1 },
		{ pos_center_w / window.width, pos_middle_h / window.height, 1 },
	};

	GMfloat extents[9][3] = {
		EXTENTS_INDEX(extentsArray[0]),
		EXTENTS_INDEX(extentsArray[1]),
		EXTENTS_INDEX(extentsArray[0]),

		EXTENTS_INDEX(extentsArray[2]),
		EXTENTS_INDEX(extentsArray[3]),
		EXTENTS_INDEX(extentsArray[2]),

		EXTENTS_INDEX(extentsArray[0]),
		EXTENTS_INDEX(extentsArray[1]),
		EXTENTS_INDEX(extentsArray[0]),
	};

	BEGIN_TO_VIEWPORT(window)
		// 9个边框的中心坐标
		GMfloat col[3] = {
			TO_VIEWPORT_X(-(d->cornerWidth + pos_center_w) / 2),
			0,
			TO_VIEWPORT_X((d->cornerWidth + pos_center_w) / 2)
		};

		GMfloat row[3] = {
			TO_VIEWPORT_Y((d->cornerHeight + pos_middle_h) / 2),
			0,
			TO_VIEWPORT_Y(-(d->cornerHeight + pos_middle_h) / 2)
		};

		GMVec4 center[9] = {
			{ col[0], row[0], 0, 1 },
			{ col[0], row[1], 0, 1 },
			{ col[0], row[2], 0, 1 },
			{ col[1], row[0], 0, 1 },
			{ col[1], row[1], 0, 1 },
			{ col[1], row[2], 0, 1 },
			{ col[2], row[0], 0, 1 },
			{ col[2], row[1], 0, 1 },
			{ col[2], row[2], 0, 1 },
		};

		// 把所有边框坐标移到中心（4）
		GMMat4 translation = Translate(
			GMVec3(TO_VIEWPORT_X(-half_border_width), TO_VIEWPORT_Y(half_border_height), 0)
		);
	END_GEOMETRY_TO_VIEWPORT()

	// Shader回调
	struct _Callback: public IPrimitiveCreatorShaderCallback {
		_Callback(GMAsset t)
			: texture(t)
		{
		}

		void onCreateShader(GMShader& shader)
		{
			shader.setNoDepthTest(true);
			auto& tex = shader.getTextureList();
			auto& frames = tex.getTextureSampler(GMTextureType::Ambient);
			frames.addFrame(GMAssets::getTexture(texture));
		}

		GMAsset texture;
	} _cb(d->texture);

	for (GMint i = 0; i < GM_array_size(d->models); ++i)
	{
		// 创建在中心的边框
		GMPrimitiveCreator::createQuad(extents[i], ValuePointer(center[i]),
			d->models + i,
			&_cb,
			GMModelType::Model2D,
			GMPrimitiveCreator::Center,
			&uv[i]);

		GMAsset asset = GMAssets::createIsolatedAsset(GMAssetType::Model, *(d->models + i));
		d->objects[i] = new GMGameObject(asset);

		// 移动到期望的位置
		GMint centerPosition[2] = {
			geometry.x + geometry.width / 2,
			geometry.y + geometry.height / 2
		};

		d->objects[i]->setTranslation(Translate(
			GMVec3(
				centerPosition[0] * 2 / window.width - 1,
				1 - centerPosition[1] * 2 / window.height,
				0))
		);
		d->objects[i]->onAppendingObjectToWorld();
		GM.createModelDataProxyAndTransfer(d->objects[i]->getModels()[0]);
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
	GM_ASSERT(hasBorder());
	drawObjects(d->objects);
}

void GMImage2DBorder::setScaling(const GMMat4& scaling)
{
	D(d);
	size_t size = GM_array_size(d->objects);
	for (GMuint i = 0; i < size; ++i)
	{
		if (d->objects[i])
			d->objects[i]->setScaling(scaling);
	}
}

void GMImage2DBorder::setTranslation(const GMMat4& translation)
{
	D(d);
	size_t size = GM_array_size(d->objects);
	for (GMuint i = 0; i < size; ++i)
	{
		if (d->objects[i])
			d->objects[i]->setTranslation(translation);
	}
}

void GMImage2DBorder::setRotation(const GMQuat& rotation)
{
	D(d);
	size_t size = GM_array_size(d->objects);
	for (GMuint i = 0; i < size; ++i)
	{
		if (d->objects[i])
			d->objects[i]->setRotation(rotation);
	}
}
