#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include <gmcommon.h>
#include "gmglshaderprogram.h"
BEGIN_NS

class GMImage;
class GMModelPainter;
class GMGLFactory : public IFactory
{
public:
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(GMImage* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, GMModel* model, OUT GMModelPainter** painter) override;
	virtual void createGlyphManager(OUT GMGlyphManager**) override;
	virtual void createFramebuffer(OUT IFramebuffer**) override;
	virtual void createFramebuffers(OUT IFramebuffers**) override;
	virtual void createGBuffer(IGraphicEngine*, OUT IGBuffer**) override;
	virtual void createLight(GMLightType, OUT ILight**) override;
};

END_NS
#endif