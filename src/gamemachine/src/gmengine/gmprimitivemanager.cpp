#include "stdafx.h"
#include "gmprimitivemanager.h"
#include "gameobjects/gm2dgameobject.h"
#include "foundation/gamemachine.h"

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
	virtual GMModel* createModel() override;
	virtual void updateVertices(GMModel* model) override;
	virtual void initShader(GMShader& shader) override;
};

GMModel* GMLine2D::createModel()
{
	D(d);
	GMModel* model = new GMModel();
	d->modelAsset = GMAsset(GMAssetType::Model, model);
	model->setType(GMModelType::Model2D);
	model->setUsageHint(GMUsageHint::DynamicDraw);
	model->setPrimitiveTopologyMode(GMTopologyMode::Lines);
	model->getShader().setVertexColorOp(GMS_VertexColorOp::Replace);
	initShader(model->getShader());
	GMMesh* mesh = new GMMesh(model);
	GMsize_t len = VerticesCount; //每个Line，用2个顶点来渲染
	for (GMsize_t i = 0; i < len; ++i)
	{
		mesh->vertex(GMVertex());
	}
	GM.createModelDataProxyAndTransfer(getContext(), model);
	return model;
}

void GMLine2D::updateVertices(GMModel* model)
{
	D(d);
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
	shader.setNoDepthTest(true);
}

GM_PRIVATE_OBJECT(GMPrimitiveLine2D)
{
	GMPrimitiveManager* manager;
	GMPoint p1, p2;
	GMVec4 color;
};

class GMPrimitiveLine2D : public IPrimitive
{
	GM_DECLARE_PRIVATE(GMPrimitiveLine2D)

public:
	GMPrimitiveLine2D(GMPrimitiveManager& manager, const GMPoint& p1, const GMPoint& p2, const GMVec4& color)
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

//////////////////////////////////////////////////////////////////////////
GMPrimitiveManager::GMPrimitiveManager(const IRenderContext* context)
{
	D(d);
	d->context = context;

	d->line2D.reset(new GMLine2D(context->getWindow()->getRenderRect()));
	d->line2D->setContext(context);
}

GMPrimitiveID GMPrimitiveManager::allocId()
{
	D(d);
	return d->current++;
}

GMPrimitiveID GMPrimitiveManager::addLine(const GMPoint& p1, const GMPoint& p2, const GMVec4& color)
{
	D(d);
	GMPrimitiveID id = allocId();
	d->primitives[id].reset(new GMPrimitiveLine2D(*this, p1, p2, color));
	return id;
}

void GMPrimitiveManager::render()
{
	D(d);
	for (auto& primitive : d->primitives)
	{
		primitive.second->drawPrimitive();
	}
}