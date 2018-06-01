#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include <gmcommon.h>
#include "gmglshaderprogram.h"
BEGIN_NS

class GMImage;
class GMModelDataProxy;
class GMGLFactory : public IFactory
{
public:
	virtual void createWindow(GMInstance instance, OUT IWindow** window) override;
	virtual void createTexture(const GMContext* context, GMImage* image, OUT ITexture** texture) override;
	virtual void createModelDataProxy(const GMContext* context, GMModel* model, OUT GMModelDataProxy** modelDataProxy) override;
	virtual void createGlyphManager(const GMContext* context, OUT GMGlyphManager**) override;
	virtual void createFramebuffer(const GMContext* context, OUT IFramebuffer**) override;
	virtual void createFramebuffers(const GMContext* context, OUT IFramebuffers**) override;
	virtual void createGBuffer(const GMContext* context, OUT IGBuffer**) override;
	virtual void createLight(GMLightType, OUT ILight**) override;
};

END_NS
#endif