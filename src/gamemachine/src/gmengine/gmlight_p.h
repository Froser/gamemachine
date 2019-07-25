#ifndef __GMLIGHT_P_H__
#define __GMLIGHT_P_H__
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

GM_PRIVATE_OBJECT_UNALIGNED(GMSpotlight_t)
{
	GMfloat cutOff; //!< 切光角，单位是角度。
};


END_NS
#endif