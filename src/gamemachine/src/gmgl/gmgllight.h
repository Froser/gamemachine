#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include <gmcommon.h>
#include <gmlight.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMGLLight);
class GMGLLight : public GMLight
{
	GM_DECLARE_PRIVATE(GMGLLight)
	GM_DECLARE_BASE(GMLight)

public:
	GMGLLight();
	~GMGLLight();

public:
	virtual void activateLight(GMuint32, ITechnique*) override;

protected:
	virtual int getLightType() = 0;
};

class GMGLPointLight : public GMGLLight
{
	enum { PointLight = 0 };

public:
	virtual int getLightType() override
	{
		return PointLight;
	}
};

GM_PRIVATE_CLASS(GMGLDirectionalLight);
class GMGLDirectionalLight : public GMGLLight
{
	GM_DECLARE_PRIVATE(GMGLDirectionalLight)
	GM_DECLARE_BASE(GMGLLight)

	enum { DirectionalLight = 1 };

public:
	GMGLDirectionalLight();
	~GMGLDirectionalLight();

public:
	virtual int getLightType() override
	{
		return DirectionalLight;
	}

	virtual bool setLightAttribute3(GMLightAttribute attr, GMfloat value[3]) override;
	virtual void activateLight(GMuint32, ITechnique*) override;
};

GM_PRIVATE_CLASS(GMGLSpotlight);
class GMGLSpotlight : public GMGLDirectionalLight
{
	GM_DECLARE_PRIVATE(GMGLSpotlight)
	GM_DECLARE_BASE(GMGLDirectionalLight)

	enum { Spotlight = 2 };

public:
	GMGLSpotlight();
	~GMGLSpotlight();

public:
	virtual int getLightType() override
	{
		return Spotlight;
	}

	virtual bool setLightAttribute(GMLightAttribute attr, GMfloat value) override;
	virtual void activateLight(GMuint32, ITechnique*) override;
};

END_NS
#endif