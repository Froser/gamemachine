#ifndef __GMLIGHT_H__
#define __GMLIGHT_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMLight);
class GM_EXPORT GMLight : public ILight
{
	GM_DECLARE_PRIVATE(GMLight)
	GM_DISABLE_COPY_ASSIGN(GMLight)

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
		CutOff, // GMfloat
		CustomStart = 0x1000,
	};

public:
	GMLight();
	~GMLight();

	virtual bool setLightAttribute3(GMLightAttribute attr, GMfloat value[3]) override;
	virtual bool setLightAttribute(GMLightAttribute, GMfloat) override;
	virtual bool getLightAttribute3(GMLightAttribute, OUT GMfloat[3]) override;
	virtual bool getLightAttribute(GMLightAttribute, REF GMfloat&) override;
};

END_NS
#endif