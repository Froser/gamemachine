﻿#ifndef __GMDX11FACTORY_H__
#define __GMDX11FACTORY_H__
#include <gmcommon.h>

BEGIN_NS

class GM_EXPORT GMDx11Factory : public IFactory
{
public:
	virtual void createWindow(GMInstance instance, IWindow* parent, OUT IWindow** window) override;
	virtual void createTexture(const IRenderContext* context, GMImage* image, REF GMTextureAsset& texture) override;
	virtual void createModelDataProxy(const IRenderContext* context, GMModel* model, OUT GMModelDataProxy** painter) override;
	virtual void createGlyphManager(const IRenderContext* context, OUT GMGlyphManager**) override;
	virtual void createFramebuffer(const IRenderContext* context, OUT IFramebuffer**) override;
	virtual void createFramebuffers(const IRenderContext* context, OUT IFramebuffers**) override;
	virtual void createGBuffer(const IRenderContext* context, OUT IGBuffer**) override;
	virtual void createLight(GMLightType, OUT ILight**) override;
	virtual void createWhiteTexture(const IRenderContext* context, REF GMTextureAsset&) override;
	virtual void createShaderPrograms(const IRenderContext* context, const GMRenderTechniqueManager& manager, REF Vector<IShaderProgram*>* out) override;
};

END_NS
#endif