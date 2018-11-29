#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include <gmcommon.h>
#include <gmlight.h>
BEGIN_NS

class GMGLLight : public GMLight
{
public:
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