#ifndef __GMDX11FACTORY_H__
#define __GMDX11FACTORY_H__
#include <gmcommon.h>

BEGIN_NS

class GMDX11Factory : public IFactory
{
public:
	virtual void createGraphicEngine(OUT IGraphicEngine** engine) override;
	virtual void createTexture(GMImage* image, OUT ITexture** texture) override;
	virtual void createPainter(IGraphicEngine* engine, GMModel* model, OUT GMModelPainter** painter) override;
	virtual void createGlyphManager(OUT GMGlyphManager**) override;
};

END_NS
#endif