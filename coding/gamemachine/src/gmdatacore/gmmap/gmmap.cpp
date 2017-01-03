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

	ASSERT(false);
	return TextureTypeUnknown;
}

GMMapObject::GMMapObjectType GMMapObject::getType(const char* name)
{
	if (!name)
		return GMMapObject::Default;

	if (strEqual("fromfile", name))
		return GMMapObject::FromFile;
	if (strEqual("cube", name))
		return GMMapObject::Cube;
	if (strEqual("sphere", name))
		return GMMapObject::Sphere;
	if (strEqual("sky", name))
		return GMMapObject::Sky;

	ASSERT(false);
	return GMMapObject::Error;
}