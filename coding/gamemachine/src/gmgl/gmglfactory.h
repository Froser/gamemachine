#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include "common.h"
#include "gmglshaders.h"
#include "foundation/utilities/utilities.h"
BEGIN_NS

class Image;
class ObjectPainter;
class GMGLFactory : public IFactory
{
public:
	virtual void createWindow(OUT GMUIWindow**) override;
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(AUTORELEASE Image* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, Object* obj, OUT ObjectPainter** painter) override;
	virtual void createGamePackage(GMGamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler) override;
	virtual void createGlyphManager(OUT GMGlyphManager**) override;
};

END_NS
#endif