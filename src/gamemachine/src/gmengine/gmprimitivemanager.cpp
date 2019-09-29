#include "stdafx.h"
#include "gmprimitivemanager.h"
#include "gameobjects/gm2dgameobject.h"
#include "gameobjects/gmgameobject_p.h"
#include "gameobjects/gm2dgameobject_p.h"
#include "foundation/gamemachine.h"

BEGIN_NS

GM_PRIVATE_OBJECT_ALIGNED(GMPrimitiveLine2D)
{
	GMPrimitiveManager* manager;
	GMPoint p1, p2;
	GMVec4 color;
};

GM_PRIVATE_OBJECT_ALIGNED(GMPrimitiveLine3D)
{
	GMPrimitiveManager* manager;
	GMVec3 p1, p2;
	GMVec4 color;
};

GM_PRIVATE_OBJECT_ALIGNED(GMPrimitive3DObject)
{
	bool dirty = true;
	GMVec3 p1;
	GMVec3 p2;
	GMVec4 color;
	GMModelAsset modelAsset;
	GMModel* model = nullptr;
};

// 基本图元
class GMLine2D : public GMSprite2DGameObject
{
	enum
	{
		VerticesCount = 2,
	};

public:
	using GMSprite2DGameObject::GMSprite2DGameObject;

public:
	virtual GMScene* createScene() override;
	virtual void updateVertices(GMScene* scene) override;
	virtual void initShader(GMShader& shader) override;
};

GMScene* GMLine2D::createScene()
{
	D(d);
	D_BASE(db, GMGameObject);
	GMModel* model = new GMModel();
	db->asset = GMScene::createSceneFromSingleModel(GMAsset(GMAssetType::Model, model));
	model->setType(GMModelType::Model2D);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::Lines);
	model->getShader().setVertexColorOp(GMS_VertexColorOp::Replace);
	initShader(model->getShader());
	GMPart* part = new GMPart(model);
	GMsize_t len = VerticesCount; //每个Line，用2个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		part->vertex(GMVertex());
	}
	getContext()->getEngine()->createModelDataProxy(getContext(), model);
	return db->asset.getScene();
}

void GMLine2D::updateVertices(GMScene* scene)
{
	D(d);
	GMModel* model = scene->getModels()[0].getModel();

	// 绘制对角线
	GMRectF coord = toViewportRect(getGeometry(), getRenderRect());
	GMVertex V[] = {
		{
			{ coord.x, coord.y, d->depth },
			{ 0 },
			{ 0, 0 },
			{ 0 },
			{ 0 },
			{ 0 },
			{ d->color[0], d->color[1], d->color[2], d->color[3] }
		},
		{
			{ coord.x + coord.width, coord.y - coord.height , d->depth },
			{ 0 },
			{ 0, 0 },
			{ 0 },
			{ 0 },
			{ 0 },
			{ d->color[0], d->color[1], d->color[2], d->color[3] }
		},
	};

	GMsize_t sz = sizeof(V);
	GMModelDataProxy* proxy = model->getModelDataProxy();
	proxy->beginUpdateBuffer();
	void* ptr = proxy->getBuffer();
	memcpy_s(ptr, sz, V, sz);
	proxy->endUpdateBuffer();
}

void GMLine2D::initShader(GMShader& shader)
{
	GMSprite2DGameObject::initShader(shader);
	shader.setBlend(false);
}

void GMPrimitive3DObject::setPosition(const GMVec3& p1, const GMVec3& p2)
{
	D(d);
	if (d->p1 != p1 || d->p2 != p2)
	{
		d->p1 = p1;
		d->p2 = p2;
		d->dirty = true;
	}
}

void GMPrimitive3DObject::setColor(const GMVec4& color)
{
	D(d);
	if (d->color != color)
	{
		d->color = color;
		d->dirty = true;
	}
}

void GMPrimitive3DObject::markDirty() GM_NOEXCEPT
{
	D(d);
	d->dirty = true;
}

void GMPrimitive3DObject::cleanDirty() GM_NOEXCEPT
{
	D(d);
	d->dirty = false;
}

bool GMPrimitive3DObject::isDirty() GM_NOEXCEPT
{
	D(d);
	return d->dirty;
}

GMPrimitive3DObject::GMPrimitive3DObject()
{
	GM_CREATE_DATA();
}

GMPrimitive3DObject::GMPrimitive3DObject(GMSceneAsset asset)
	: Base(asset)
{
	GM_CREATE_DATA();
}

GMPrimitive3DObject::~GMPrimitive3DObject()
{

}

void GMPrimitive3DObject::draw()
{
	D(d);
	update();
	drawModel(getContext(), d->model);
	endDraw();
}

void GMPrimitive3DObject::update()
{
	D(d);
	if (!d->model)
	{
		d->model = createModel();
		markDirty();
	}

	if (isDirty())
	{
		updateVertices(d->model);
		cleanDirty();
	}
}

void GMPrimitive3DObject::initShader(GMShader& shader)
{
	GMGlyphManager* glyphManager = getContext()->getEngine()->getGlyphManager();
	auto& frame = shader.getTextureList().getTextureSampler(GMTextureType::Ambient);
	frame.setMinFilter(GMS_TextureFilter::Linear);
	frame.setMagFilter(GMS_TextureFilter::Linear);
	frame.addFrame(glyphManager->glyphTexture());
	shader.setNoDepthTest(true);
	shader.setCull(GMS_Cull::NoCull);
}

class GMLine3D : public GMPrimitive3DObject
{
	enum
	{
		VerticesCount = 2,
	};

protected:
	virtual void updateVertices(GMModel* model) override;
	virtual GMModel* createModel() override;
};

void GMLine3D::updateVertices(GMModel* model)
{
	D(d);
	GMFloat4 p1f4, p2f4, color;
	d->p1.loadFloat4(p1f4);
	d->p2.loadFloat4(p2f4);
	d->color.loadFloat4(color);

	GMVertex V[] = {
		{
			{ p1f4[0], p1f4[1], p1f4[2] },
			{ 0 },
			{ 0, 0 },
			{ 0 },
			{ 0 },
			{ 0 },
			{ color[0], color[1], color[2], color[3] }
		},
		{
			{ p2f4[0], p2f4[1], p2f4[2] },
			{ 0 },
			{ 0, 0 },
			{ 0 },
			{ 0 },
			{ 0 },
			{ color[0], color[1], color[2], color[3] }
		},
	};

	GMsize_t sz = sizeof(V);
	GMModelDataProxy* proxy = model->getModelDataProxy();
	proxy->beginUpdateBuffer();
	void* ptr = proxy->getBuffer();
	memcpy_s(ptr, sz, V, sz);
	proxy->endUpdateBuffer();
}

GMModel* GMLine3D::createModel()
{
	D(d);
	GMModel* model = new GMModel();
	d->modelAsset = GMAsset(GMAssetType::Model, model);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::Lines);
	model->getShader().setVertexColorOp(GMS_VertexColorOp::Replace);
	initShader(model->getShader());
	GMPart* part = new GMPart(model);
	GMsize_t len = VerticesCount; //每个Line，用2个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		part->vertex(GMVertex());
	}
	getContext()->getEngine()->createModelDataProxy(getContext(), model);
	return model;
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_CLASS(GMPrimitiveLine2D);
class GMPrimitiveLine2D : public GMObject, public IPrimitive
{
	GM_DECLARE_PRIVATE(GMPrimitiveLine2D)

public:
	GMPrimitiveLine2D(GMPrimitiveManager& manager, const GMPoint& p1, const GMPoint& p2, const GMVec4& color)
	{
		GM_CREATE_DATA();

		D(d);
		d->manager = &manager;
		d->p1 = p1;
		d->p2 = p2;
		d->color = color;
	}

public:
	virtual void drawPrimitive() override;
};

void GMPrimitiveLine2D::drawPrimitive()
{
	D(d);
	GM_ASSERT(d->manager);
	auto line2D = d->manager->getLine2D();
	GMRect rc = { d->p1.x, d->p1.y, d->p2.x - d->p1.x, d->p2.y - d->p1.y };
	line2D->setDepth(0);
	line2D->setGeometry(rc);
	line2D->setColor(d->color);
	line2D->draw();
}

class GMPrimitiveLine3D : public GMObject, public IPrimitive
{
	GM_DECLARE_PRIVATE(GMPrimitiveLine3D)

public:
	GMPrimitiveLine3D(GMPrimitiveManager& manager, const GMVec3& p1, const GMVec3& p2, const GMVec4& color)
	{
		D(d);
		d->manager = &manager;
		d->p1 = p1;
		d->p2 = p2;
		d->color = color;
	}

public:
	virtual void drawPrimitive() override;
};

void GMPrimitiveLine3D::drawPrimitive()
{
	D(d);
	GM_ASSERT(d->manager);
	auto line3D = d->manager->getLine3D();
	line3D->setPosition(d->p1, d->p2);
	line3D->setColor(d->color);
	line3D->draw();
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_UNALIGNED(GMPrimitiveManager)
{
	const IRenderContext* context = nullptr;
	GMAtomic<GMPrimitiveID> current;
	Map<GMPrimitiveID, GMOwnedPtr<IPrimitive>> primitives;

	// 图元
	GMOwnedPtr<GMSprite2DGameObject> line2D;
	GMOwnedPtr<GMPrimitive3DObject> line3D;

	GMPrimitiveID allocId();
};

GMPrimitiveManager::GMPrimitiveManager(const IRenderContext* context)
{
	GM_CREATE_DATA();

	D(d);
	d->context = context;

	d->line2D.reset(new GMLine2D(context->getWindow()->getRenderRect()));
	d->line2D->setContext(context);

	d->line3D.reset(new GMLine3D());
	d->line3D->setContext(context);
}

GMPrimitiveManager::~GMPrimitiveManager()
{

}

GMPrimitiveID GMPrimitiveManagerPrivate::allocId()
{
	return current++;
}

GMPrimitiveID GMPrimitiveManager::addLine(const GMPoint& p1, const GMPoint& p2, const GMVec4& color)
{
	D(d);
	GMPrimitiveID id = d->allocId();
	d->primitives[id].reset(new GMPrimitiveLine2D(*this, p1, p2, color));
	return id;
}

GMPrimitiveID GMPrimitiveManager::addLine(const GMVec3& p1, const GMVec3& p2, const GMVec4& color)
{
	D(d);
	GMPrimitiveID id = d->allocId();
	d->primitives[id].reset(new GMPrimitiveLine3D(*this, p1, p2, color));
	return id;
}

void GMPrimitiveManager::remove(GMPrimitiveID id)
{
	D(d);
	d->primitives[id].reset(nullptr);
}

void GMPrimitiveManager::clear()
{
	D(d);
	GMClearSTLContainer(d->primitives);
}

void GMPrimitiveManager::render()
{
	D(d);
	for (auto& primitive : d->primitives)
	{
		if (primitive.second)
			primitive.second->drawPrimitive();
	}
}

GMSprite2DGameObject* GMPrimitiveManager::getLine2D() GM_NOEXCEPT
{
	D(d);
	return d->line2D.get();
}

GMPrimitive3DObject* GMPrimitiveManager::getLine3D() GM_NOEXCEPT
{
	D(d);
	return d->line3D.get();
}

END_NS