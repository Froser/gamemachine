#include "stdafx.h"
#include "gmdx11factory.h"
#include "gmdx11graphic_engine.h"
#include "gmdx11modeldataproxy.h"
#include "gmdx11texture.h"
#include "gmdx11glyphmanager.h"
#include "gmdx11framebuffer.h"
#include "gmdx11gbuffer.h"
#include "gmdx11light.h"
#include "gmengine/ui/gmwindow.h"

void GMDx11Factory::createWindow(GMInstance instance, OUT IWindow** window)
{
	bool b = GMWindowFactory::createWindowWithDx11(instance, window);
	GM_ASSERT(b);
}

void GMDx11Factory::createTexture(const IRenderContext* context, GMImage* image, REF GMTextureAsset& texture)
{
	GMDx11Texture* t = new GMDx11Texture(context, image);
	t->init();
	texture = GMAsset(GMAssetType::Texture, t);
}

void GMDx11Factory::createModelDataProxy(const IRenderContext* context, GMModel* model, OUT GMModelDataProxy** modelDataProxy)
{
	GM_ASSERT(modelDataProxy);
	(*modelDataProxy) = new GMDx11ModelDataProxy(context, model);
}

void GMDx11Factory::createGlyphManager(const IRenderContext* context, OUT GMGlyphManager** glyphManager)
{
	GM_ASSERT(glyphManager);
	*glyphManager = new GMDx11GlyphManager(context);
}

void GMDx11Factory::createFramebuffer(const IRenderContext* context, OUT IFramebuffer** fb)
{
	*fb = new GMDx11Framebuffer(context);
}

void GMDx11Factory::createFramebuffers(const IRenderContext* context, OUT IFramebuffers** fbs)
{
	*fbs = new GMDx11Framebuffers(context);
}

void GMDx11Factory::createGBuffer(const IRenderContext* context, OUT IGBuffer** g)
{
	*g = new GMDx11GBuffer(context);
}

void GMDx11Factory::createLight(GMLightType type, OUT ILight** out)
{
	if (type == GMLightType::Ambient)
	{
		*out = new GMDx11AmbientLight();
	}
	else
	{
		GM_ASSERT(type == GMLightType::Direct);
		*out = new GMDx11DirectLight();
	}
}

void GMDx11Factory::createWhiteTexture(const IRenderContext* context, REF GMTextureAsset& texture)
{
	GMDx11WhiteTexture* t = new GMDx11WhiteTexture(context);
	t->init();
	texture = GMAsset(GMAssetType::Texture, t);
}