#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "common.h"
BEGIN_NS

struct ILightController
{
	virtual ~ILightController();
	virtual void setAmbient(GMfloat rgb[3]) = 0;
	virtual void setAmbientCoefficient(GMfloat ka[3]) = 0;
	virtual void setLightColor(GMfloat rgb[3]) = 0;
	virtual void setDiffuseCoefficient(GMfloat kd[3]) = 0;
	virtual void setShininess(GMfloat n) = 0;
	virtual void setSpecularCoefficient(GMfloat ks[3]) = 0;
	virtual void setLightPosition(GMfloat xyz[3]) = 0;
	virtual GMfloat* getLightPosition() = 0;
	virtual void setViewPosition(GMfloat xyz[3]) = 0;
};

END_NS
#endif