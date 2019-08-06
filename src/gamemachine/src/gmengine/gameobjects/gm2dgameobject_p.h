#ifndef __GM2DGAMEOBJECT_P_H__
#define __GM2DGAMEOBJECT_P_H__
#include <gmcommon.h>

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMSprite2DGameObject)
{
	GM_DECLARE_PUBLIC(GMSprite2DGameObject)

	GMScene* scene = nullptr;
	GMAsset texture;
	GMRect textureRc;
	GMint32 texHeight = 0;
	GMint32 texWidth = 0;
	GMfloat depth = 0;
	bool needUpdateTexture = false;
	GMFloat4 color = GMFloat4(1, 1, 1, 1);

	void update();
};

END_NS
#endif