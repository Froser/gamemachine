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
	d->quad->destroy();
	d->geometryFramebuffers->destroy();
}

void GMGBuffer::createQuad()
{
	D(d);
	GM_ASSERT(!d->quad);
	GMSceneAsset scene;
	GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, scene);
	GM_ASSERT(!scene.isEmpty());
	GMModel* model = scene.getScene()->getModels()[0].getModel();
	GM_ASSERT(model);
	model->setType(GMModelType::LightPassQuad);
	getContext()->getEngine()->createModelDataProxy(d->context, model);
	d->quad = new GMGameObject(scene);
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