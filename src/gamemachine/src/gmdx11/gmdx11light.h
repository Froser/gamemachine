#ifndef __GMDX11LIGHT_H__
#define __GMDX11LIGHT_H__
#include <gmcommon.h>
#include <gmlight.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11Light)
{
	ID3DX11Effect* effect = nullptr;
	ID3DX11EffectVariable* lightAttributes = nullptr;
};

class GMDx11Light : public GMLight
{
	GM_DECLARE_PRIVATE_NGO(GMDx11Light)

public:
	virtual void activateLight(GMuint32, ITechnique*) override;

protected:
	virtual int getLightType() = 0;
};

class GMDx11PointLight : public GMDx11Light
{
	enum { LightType = 0 };

public:
	virtual int getLightType() override
	{
		return LightType;
	}
};

END_NS
#endif