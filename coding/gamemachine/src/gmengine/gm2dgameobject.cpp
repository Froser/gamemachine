#include "stdafx.h"
#include "gm2dgameobject.h"
#include "gmgameworld.h"

GM2DGameObject::GM2DGameObject(GMGameWorld* world)
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
void GMImage2DGameObject::setImage(GMAsset& image)
{
	D(d);
	d->image = GMAssets::getTexture(image);
}