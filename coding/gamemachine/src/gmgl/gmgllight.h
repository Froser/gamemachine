#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include "common.h"
#include "gmengine/elements/gamelight.h"
BEGIN_NS

struct Material;
class GMGLAmbientLight : public GameLight
{
public:
	virtual void activateLight(Material& material) override;
	virtual bool isAvailable() override;
};

class GMGLSpecularLight : public GameLight
{
public:
	virtual void activateLight(Material& material) override;
	virtual bool isAvailable() override;
};

END_NS
#endif