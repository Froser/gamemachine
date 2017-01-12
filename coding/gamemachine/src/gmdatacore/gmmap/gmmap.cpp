#include "stdafx.h"
#include "gmmap.h"
#include "utilities/assert.h"

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
	if (strEqual("hallucination", name))
		return GMMapObject::Hallucination;
	if (strEqual("capsule", name))
		return GMMapObject::Capsule;
	if (strEqual("cylinder", name))
		return GMMapObject::Cylinder;
	if (strEqual("cone", name))
		return GMMapObject::Cone;

	ASSERT(false);
	return GMMapObject::Error;
}

LightType GMMapLight::getType(const char* name)
{
	if (!name)
		return Specular;

	if (strEqual("ambient", name))
		return Ambient;
	if (strEqual("specular", name))
		return Specular;

	LOG_ASSERT_MSG(false, "Wrong light type");
	return Specular;
}

Keyframes::Interpolation GMMapKeyframes::getFunctorType(const char* name)
{
	if (!name)
		return Keyframes::Linear;

	if (strEqual("linear", name))
		return Keyframes::Linear;

	LOG_ASSERT_MSG(false, "Wrong keyframes functor type");
	return Keyframes::Linear;
}

GMMapKeyframes::Type GMMapKeyframes::getType(const char* name)
{
	if (!name)
		return Type::Rotation;

	if (strEqual("rotation", name))
		return Type::Rotation;
	if (strEqual("translation", name))
		return Type::Translation;
	if (strEqual("scaling", name))
		return Type::Scaling;

	LOG_ASSERT_MSG(false, "Wrong keyframes type");
	return Type::Scaling;
}