#include "stdafx.h"
#include "gmgbuffer.h"
#include "foundation/utilities/utilities.h"
#include "gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"
#include "gmgraphicengine.h"
#include "gmgbuffer_p.h"

BEGIN_NS
GMGBuffer::GMGBuffer(const IRenderContext* context)
{
	GM_CREATE_DATA();
	GM_SET_PD();

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

void GMGBufferPrivate::createQuad()
{
	P_D(pd);
	GM_ASSERT(!quad);
	GMSceneAsset scene;
	GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, scene);
	GM_ASSERT(!scene.isEmpty());
	GMModel* model = scene.getScene()->getModels()[0].getModel();
	GM_ASSERT(model);
	model->setType(GMModelType::LightPassQuad);
	pd->getContext()->getEngine()->createModelDataProxy(context, model);
	quad = new GMGameObject(scene);
	quad->setContext(context);
}

void GMGBuffer::init()
{
	D(d);
	if (!d->geometryFramebuffers)
		d->geometryFramebuffers = createGeometryFramebuffers();

	if (!d->quad)
		d->createQuad();
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

END_NS