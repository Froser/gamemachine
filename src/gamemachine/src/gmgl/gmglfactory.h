#ifndef __GMGLFACTORY_H__
#define __GMGLFACTORY_H__
#include <gmcommon.h>
#include "gmglshaderprogram.h"
BEGIN_NS

class GMImage;
class GMModelDataProxy;
class GMRenderTechniques;
class GM_EXPORT GMGLFactory : public IFactory
{
public:
	virtual void createWindow(GMInstance instance, IWindow* parent, OUT IWindow** window) override;
	virtual void createTexture(const IRenderContext* context, GMImage* image, REF GMTextureAsset& texture) override;
	virtual void createModelDataProxy(const IRenderContext* context, GMModel* model, OUT GMModelDataProxy** modelDataProxy) override;
	virtual void createGlyphManager(const IRenderContext* context, OUT GMGlyphManager**) override;
	virtual void createFramebuffer(const IRenderContext* context, OUT IFramebuffer**) override;
	virtual void createFramebuffers(const IRenderContext* context, OUT IFramebuffers**) override;
	virtual void createGBuffer(const IRenderContext* context, OUT IGBuffer**) override;
	virtual void createLight(GMLightType, OUT ILight**) override;
	virtual void createWhiteTexture(const IRenderContext* context, REF GMTextureAsset&) override;
	virtual void createEmptyTexture(const IRenderContext* context, REF GMTextureAsset&) override;
	virtual void createShaderPrograms(const IRenderContext* context, const GMRenderTechniqueManager& manager, REF Vector<IShaderProgram*>* out);
};

END_NS
#endif