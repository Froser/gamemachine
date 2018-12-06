#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include <gmcommon.h>
#include <gmlight.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGLLight)
{
	struct LightIndices
	{
		GMint32 Color;
		GMint32 Position;
		GMint32 Type;
		GMint32 AttenuationConstant;
		GMint32 AttenuationLinear;
		GMint32 AttenuationExp;
		GMint32 AmbientIntensity;
		GMint32 DiffuseIntensity;
		GMint32 SpecularIntensity;
	};
	Vector<Vector<LightIndices>> lightIndices;
};

class GMGLLight : public GMLight
{
	GM_DECLARE_PRIVATE_NGO(GMGLLight)
	typedef GMLight Base;

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

GM_PRIVATE_OBJECT_UNALIGNED_FROM(GMGLDirectionalLight, GMDirectionalLight_t)
{
	struct LightIndices
	{
		GMint32 Direction;
	};

	Vector<Vector<LightIndices>> lightIndices;
};

class GMGLDirectionalLight : public GMGLLight
{
	GM_DECLARE_PRIVATE_NGO(GMGLDirectionalLight)
	typedef GMGLLight Base;

	enum { DirectionalLight = 1 };

public:
	virtual int getLightType() override
	{
		return DirectionalLight;
	}

	virtual bool setLightAttribute3(GMLightAttribute attr, GMfloat value[3]) override;
	virtual void activateLight(GMuint32, ITechnique*) override;
};

GM_PRIVATE_OBJECT_UNALIGNED_FROM(GMGLSpotlight, GMSpotlight_t)
{
	struct LightIndices
	{
		GMint32 CutOff;
	};

	Vector<Vector<LightIndices>> lightIndices;
};

class GMGLSpotlight : public GMGLDirectionalLight
{
	GM_DECLARE_PRIVATE_NGO(GMGLSpotlight)
	typedef GMGLDirectionalLight Base;

	enum { Spotlight = 2 };

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