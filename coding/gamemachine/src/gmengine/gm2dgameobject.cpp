#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"

GM2DGameObject::GM2DGameObject(GMGameWorld* world)
	: GMGameObject(nullptr)
{
	D(d);
	d->world = world;
}

void GM2DGameObject::setRect(const GMRect& rect)
{
	D(d);
	d->rect = rect;
}

//////////////////////////////////////////////////////////////////////////
void GMImage2DGameObject::setImage(const GMTextureAssets::TextureItemType& image)
{
	D_BASE(db, GMGameObject);
	db->world->getAssets();
}