#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLLight)
{
	GMfloat position[4];
	GMfloat color[4];
};

class GMGLLight : public GMObject, public ILight
{
	GM_DECLARE_PRIVATE(GMGLLight)

public:
	virtual bool setLightAttribute3(GMLightAttribute attr, GMfloat value[3]) override;
	virtual bool setLightAttribute(GMLightAttribute, GMfloat) override;
	virtual void activateLight(GMuint32, ITechnique*) override;

protected:
	virtual int getType() = 0;
};

class GMGLAmbientLight : public GMGLLight
{
	enum { LightType = 0 };

public:
	virtual int getType() override
	{
		return LightType;
	}
};

class GMGLDirectLight : public GMGLLight
{
	enum { LightType = 1 };

public:
	virtual int getType() override
	{
		return LightType;
	}
};


END_NS
#endif