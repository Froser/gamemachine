#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"
#include "foundation/utilities/gmprimitivecreator.h"

void GM2DGameObject::setRect(const GMRect& rect)
{
	D(d);
	d->rect = rect;
}

//////////////////////////////////////////////////////////////////////////
GMImage2DGameObject::~GMImage2DGameObject()
{
	D(d);
	if (d->model)
		delete d->model;
}

void GMImage2DGameObject::setImage(GMAsset& image)
{
	D(d);
	d->image = GMAssets::getTexture(image);
	GM_ASSERT(d->image);
}

void GMImage2DGameObject::onAppendingObjectToWorld()
{
	D(d);
	D_BASE(db, GM2DGameObject);
	auto& rect = GM.getMainWindow()->getClientRect();
	GMfloat extents[3] = {
		(GMfloat)db->rect.width / rect.width,
		(GMfloat)db->rect.height / rect.height,
		1.f,
	};
	GMfloat pos[3] = {
		(GMfloat)db->rect.x / rect.width,
		(GMfloat)db->rect.y / rect.height,
		0.f,
	};
	GMPrimitiveCreator::createQuad(extents, pos, &d->model, this);
	auto asset = GMAssets::createIsolatedAsset(GMAssetType::Model, d->model);
	setModel(&asset);
}

void GMImage2DGameObject::onCreateShader(Shader& shader)
{
	D(d);
	auto& tex = shader.getTexture();
	auto& frames = tex.getTextureFrames(GMTextureType::AMBIENT, 0);
	frames.addFrame(d->image);
}
