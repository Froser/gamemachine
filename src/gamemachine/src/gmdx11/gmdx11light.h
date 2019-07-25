#ifndef __GMDX11LIGHT_H__
#define __GMDX11LIGHT_H__
#include <gmcommon.h>
#include <gmlight.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMDx11Light);
class GMDx11Light : public GMLight
{
	GM_DECLARE_PRIVATE(GMDx11Light)

public:
	GMDx11Light();
	~GMDx11Light();

public:
	virtual void activateLight(GMuint32, ITechnique*) override;

protected:
	virtual int getLightType() = 0;
};

GM_PRIVATE_CLASS(GMDx11PointLight);
class GMDx11PointLight : public GMDx11Light
{
	enum { PointLight = 0 };

public:
	virtual int getLightType() override
	{
		return PointLight;
	}
};

GM_PRIVATE_CLASS(GMDx11DirectionalLight);
class GMDx11DirectionalLight : public GMDx11Light
{
	GM_DECLARE_PRIVATE(GMDx11DirectionalLight)
	GM_DECLARE_BASE(GMDx11Light)

	enum { DirectionalLight = 1 };

public:
	GMDx11DirectionalLight();
	~GMDx11DirectionalLight();

	virtual int getLightType() override
	{
		return DirectionalLight;
	}

	virtual bool setLightAttribute3(GMLightAttribute attr, GMfloat value[3]) override;
	virtual void activateLight(GMuint32, ITechnique*) override;
};

GM_PRIVATE_CLASS(GMDx11Spotlight);
class GMDx11Spotlight : public GMDx11DirectionalLight
{
	GM_DECLARE_PRIVATE(GMDx11Spotlight)
	GM_DECLARE_BASE(GMDx11DirectionalLight)

	enum { Spotlight = 2 };

public:
	GMDx11Spotlight();
	~GMDx11Spotlight();

	virtual int getLightType() override
	{
		return Spotlight;
	}

	virtual bool setLightAttribute(GMLightAttribute attr, GMfloat value) override;
	virtual void activateLight(GMuint32, ITechnique*) override;
};
END_NS
#endif