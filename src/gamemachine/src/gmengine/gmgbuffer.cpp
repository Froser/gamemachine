#include "stdafx.h"
#include "gmgbuffer.h"
#include "foundation/utilities/utilities.h"
#include "gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"
#include "gmgraphicengine.h"

GMGBuffer::GMGBuffer(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->engine = gm_cast<GMGraphicEngine*>(d->context->getEngine());

}

GMGBuffer::~GMGBuffer()
{
	D(d);
	GM_delete(d->quad);
	GM_delete(d->geometryFramebuffers);
}

void GMGBuffer::createQuad()
{
	D(d);
	GM_ASSERT(!d->quad);
	GMModel* model = nullptr;
	GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, &model);
	GM_ASSERT(model);
	model->setType(GMModelType::LightPassQuad);
	GM.createModelDataProxyAndTransfer(d->context, model);
	d->quad = new GMGameObject(GMAsset(GMAssetType::Model, model));
	d->quad->setContext(d->context);
}

void GMGBuffer::init()
{
	D(d);
	if (!d->geometryFramebuffers)
		d->geometryFramebuffers = createGeometryFramebuffers();

	if (!d->quad)
		createQuad();
}

void GMGBuffer::setGeometryPassingState(GMGeometryPassingState state)
{
	D(d);
	d->state = state;
}

GMGeometryPassingState GMGBuffer::getGeometryPassingState()
{
	D(d);
	return d->state;
}

GMGameObject* GMGBuffer::getQuad()
{
	D(d);
	return d->quad;
}

IFramebuffers* GMGBuffer::getGeometryFramebuffers()
{
	D(d);
	GM_ASSERT(d->geometryFramebuffers);
	return d->geometryFramebuffers;
}

const IRenderContext* GMGBuffer::getContext()
{
	D(d);
	return d->context;
}