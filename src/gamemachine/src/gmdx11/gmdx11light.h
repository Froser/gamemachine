#ifndef __GMDX11LIGHT_H__
#define __GMDX11LIGHT_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11Light)
{
	GMfloat position[4];
	GMfloat color[4];
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
	virtual const char* getImplementName() = 0;
};

class GMDx11AmbientLight : public GMDx11Light
{
public:
	virtual const char* getImplementName() override
	{
		return "DefaultAmbientLight";
	}
};

class GMDx11DirectLight : public GMDx11Light
{
public:
	virtual const char* getImplementName() override
	{
		return "DefaultDirectLight";
	}
};

END_NS
#endif