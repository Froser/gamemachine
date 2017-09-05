#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include "common.h"
#include "gmglshaderprogram.h"
#include "foundation/utilities/utilities.h"
BEGIN_NS

class GMImage;
class GMModelPainter;
class GMGLFactory : public IFactory
{
public:
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(AUTORELEASE GMImage* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, GMModel* obj, OUT GMModelPainter** painter) override;
	virtual void createGamePackage(GMGamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler) override;
	virtual void createGlyphManager(OUT GMGlyphManager**) override;
};

END_NS
#endif