#ifndef __GMLIGHT_H__
#define __GMLIGHT_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMLight)
{
	GMfloat position[4] = { 1, 1, 1, 1 };
	GMfloat color[4] = { 1, 1, 1, 1 };
	GMfloat ambientIntensity[4] = { 1, 1, 1, 1 };
	GMfloat diffuseIntensity[4] = { 1, 1, 1, 1 };
	GMfloat specularIntensity = 1;

	struct
	{
		gm::GMfloat constant = 1.f;
		gm::GMfloat linear = 0.0f;
		gm::GMfloat exp = 0.0f;
	} attenuation;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMDirectionalLight_t)
{
	GMfloat direction[4] = { 0, -1, 0, 0 };
};

class GMLight : public ILight
{
	GM_DECLARE_PRIVATE_NGO(GMLight)

public:
	enum
	{
		Position, // GMfloat[3]
		Color, // GMfloat[3]
		AmbientIntensity, // GMfloat[3]
		DiffuseIntensity, // GMfloat[3]
		SpecularIntensity, // GMfloat
		AttenuationConstant, // GMfloat
		AttenuationLinear, // GMfloat
		AttenuationExp, // GMfloat
		Direction, // GMfloat[3]
		CustomStart = 0x1000,
	};

public:
	virtual bool setLightAttribute3(GMLightAttribute attr, GMfloat value[3]) override;
	virtual bool setLightAttribute(GMLightAttribute, GMfloat) override;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMDirectionalLight)
{
	struct LightIndices
	{
		GMint32 Direction;
	};

	Vector<Vector<LightIndices>> lightIndices;
};

END_NS
#endif