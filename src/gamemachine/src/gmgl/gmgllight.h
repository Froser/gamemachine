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
	DECLARE_PRIVATE(GMGLLight)

public:
	virtual void setLightPosition(GMfloat position[4]) override;
	virtual void setLightColor(GMfloat color[4]) override;
	virtual const GMfloat* getLightPosition() const override;
	virtual const GMfloat* getLightColor() const override;
	virtual void activateLight(GMuint, IRenderer*) override;

protected:
	virtual const char* getImplementName() = 0;
};

class GMGLAmbientLight : public GMGLLight
{
public:
	virtual const char* getImplementName() override
	{
		return "DefaultAmbientLight";
	}
};

class GMGLDirectLight : public GMGLLight
{
public:
	virtual const char* getImplementName() override
	{
		return "DefaultDirectLight";
	}
};


END_NS
#endif