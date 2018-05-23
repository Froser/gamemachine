#ifndef __GMDX11FACTORY_H__
#define __GMDX11FACTORY_H__
#include <gmcommon.h>

BEGIN_NS

class GMDx11Factory : public IFactory
{
public:
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(GMImage* image, OUT ITexture** texture) override;
	virtual void createModelDataProxy(IGraphicEngine* engine, GMModel* model, OUT GMModelDataProxy** painter) override;
	virtual void createGlyphManager(OUT GMGlyphManager**) override;
	virtual void createFramebuffer(OUT IFramebuffer**) override;
	virtual void createFramebuffers(OUT IFramebuffers**) override;
	virtual void createGBuffer(IGraphicEngine*, OUT IGBuffer**) override;
	virtual void createLight(GMLightType, OUT ILight**) override;
};

END_NS
#endif