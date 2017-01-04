#include "stdafx.h"
#include "gmmap.h"
#include "utilities/assert.h"

static bool strEqual(const char* str1, const char* str2)
{
	return !strcmp(str1, str2);
}

TextureType GMMapTexture::getType(const char* name)
{
	if (!name)
		return TextureTypeAmbient;

	if (strEqual("ambient", name))
		return TextureType::TextureTypeAmbient;
	if (strEqual("cubemap", name))
		return TextureType::TextureTypeCubeMap;

	ASSERT(false);
	return TextureTypeUnknown;
}

GMMapObject::GMMapObjectType GMMapObject::getType(const char* name)
{
	if (!name)
		return GMMapObject::Default;

	if (strEqual("cube", name))
		return GMMapObject::Cube;
	if (strEqual("sphere", name))
		return GMMapObject::Sphere;
	if (strEqual("sky", name))
		return GMMapObject::Sky;
	if (strEqual("convexhull", name))
		return GMMapObject::ConvexHull;

	ASSERT(false);
	return GMMapObject::Error;
}