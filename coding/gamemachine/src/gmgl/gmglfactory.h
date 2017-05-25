#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include "common.h"
#include "gmengine/controllers/factory.h"
#include "gmglshaders.h"
#include "foundation/utilities/utilities.h"
BEGIN_NS

struct IGraphicEngine;
class Image;
struct ITexture;
class ObjectPainter;
class GMGLFactory : public IFactory
{
public:
	virtual void createWindow(OUT IWindow**) override;
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(AUTORELEASE Image* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, Object* obj, OUT ObjectPainter** painter) override;
	virtual void createGamePackage(GamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler) override;
	virtual void createGlyphManager(OUT GlyphManager**) override;
};

END_NS
#endif