#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include "common.h"
#include "gmengine/controllers/factory.h"
#include "gmglshaders.h"
#include "utilities/autoptr.h"
BEGIN_NS

struct IGraphicEngine;
class Image;
struct ITexture;
class ObjectPainter;
class GMGLFactory : public IFactory
{
public:
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(Image* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, Object* obj, OUT ObjectPainter** painter) override;
};

END_NS
#endif