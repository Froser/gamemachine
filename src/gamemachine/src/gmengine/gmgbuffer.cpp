#include "stdafx.h"
#include "gmgbuffer.h"
#include "foundation/utilities/utilities.h"
#include "gameobjects/gmgameobject.h"
#include "foundation/gamemachine.h"

GMGBuffer::~GMGBuffer()
{
	D(d);
	GM_delete(d->quad);
	GM_delete(d->quadModel);
	GM_delete(d->geometryFramebuffers);
	GM_delete(d->materialFramebuffers);
}

void GMGBuffer::createQuad()
{
	D(d);
	GM_ASSERT(!d->quad);
	GM_ASSERT(!d->quadModel);
	GMPrimitiveCreator::createQuadrangle(GMPrimitiveCreator::one2(), 0, &d->quadModel);
	GM_ASSERT(d->quadModel);
	d->quadModel->setType(GMModelType::LightPassQuad);
	GM.createModelPainterAndTransfer(d->quadModel);
	d->quad = new GMGameObject(GMAssets::createIsolatedAsset(GMAssetType::Model, d->quadModel));
}

void GMGBuffer::init()
{
	D(d);
	if (!d->geometryFramebuffers)
		d->geometryFramebuffers = createGeometryFramebuffers();

	if (!d->materialFramebuffers)
		d->materialFramebuffers = createMaterialFramebuffers();

	if (!d->quad)
		createQuad();
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

IFramebuffers* GMGBuffer::getMaterialFramebuffers()
{
	D(d);
	GM_ASSERT(d->materialFramebuffers);
	return d->materialFramebuffers;
}