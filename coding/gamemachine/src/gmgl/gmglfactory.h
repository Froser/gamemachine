#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include "common.h"
#include "gmengine/controller/factory.h"
#include "gmglshaders.h"
#include "utilities/autoptr.h"
BEGIN_NS

class IGraphicEngine;
class Image;
class ITexture;
class ObjectPainter;
class GameLight;
class GMGLFactory : public IFactory
{
public:
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(Image* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, Object* obj, OUT ObjectPainter** painter) override;
	virtual void createLight(LightType type, OUT GameLight**) override;
};

END_NS
#endif