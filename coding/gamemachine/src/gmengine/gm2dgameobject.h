#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include "common.h"
#include "gmgameobject.h"
#include "gmassets.h"
BEGIN_NS

class GMGameWorld;
GM_PRIVATE_OBJECT(GM2DGameObject)
{
	GMRect rect;
	GMGameWorld* world = nullptr;
};

class GM2DGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GM2DGameObject)

public:
	GM2DGameObject(GMGameWorld* world);

public:
	void setRect(const GMRect& rect);
};

GM_PRIVATE_OBJECT(GMImage2DGameObject)
{
	ITexture* image;
};

class GMImage2DGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMImage2DGameObject)

public:
	void setImage(const GMTextureAssets::TextureItemType& image);
	void setImage(GMImage& image);

private:
	virtual void onAppendingObjectToWorld();
};

END_NS
#endif