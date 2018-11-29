#ifndef __GMDX11LIGHT_H__
#define __GMDX11LIGHT_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11Light)
{
	GMfloat position[4];
	GMfloat color[4];
	ID3DX11Effect* effect = nullptr;
	ID3DX11EffectVariable* lightAttributes = nullptr;
};

class GMDx11Light : public GMObject, public ILight
{
	GM_DECLARE_PRIVATE(GMDx11Light)

public:
	virtual bool setLightAttribute3(GMLightAttribute, GMfloat[3]) override;
	virtual bool setLightAttribute(GMLightAttribute, GMfloat) override;
	virtual void activateLight(GMuint32, ITechnique*) override;

protected:
	virtual int getLightType() = 0;
};

class GMDx11AmbientLight : public GMDx11Light
{
	enum { LightType = 0 };

public:
	virtual int getLightType() override
	{
		return LightType;
	}
};

class GMDx11DirectLight : public GMDx11Light
{
	enum { LightType = 1 };

public:
	virtual int getLightType() override
	{
		return LightType;
	}
};

END_NS
#endif