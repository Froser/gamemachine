#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include "common.h"
#include "gmengine/elements/gamelight.h"
BEGIN_NS

struct Material;
class GMGLShaders;
class GMGLLight : public GameLight
{
public:
	GMGLLight();

public:
	void setShaders(GMGLShaders* shaders);

protected:
	GMGLShaders* m_shaders;
};

class GMGLAmbientLight : public GMGLLight
{
public:
	virtual void activateLight(Material& material) override;
	virtual bool isAvailable() override;
};

class GMGLSpecularLight : public GMGLLight
{
public:
	virtual void activateLight(Material& material) override;
	virtual bool isAvailable() override;
};

END_NS
#endif