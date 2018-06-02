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
	DECLARE_PRIVATE(GMDx11Light)

public:
	virtual void setLightPosition(GMfloat position[4]) override;
	virtual void setLightColor(GMfloat color[4]) override;
	virtual const GMfloat* getLightPosition() const override;
	virtual const GMfloat* getLightColor() const override;
	virtual void activateLight(GMuint, IRenderer*) override;

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