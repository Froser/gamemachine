#include "stdafx.h"
#include "gmdx11factory.h"
#include "gmdx11graphic_engine.h"
#include "gmdx11modeldataproxy.h"
#include "gmdx11texture.h"
#include "gmdx11glyphmanager.h"
#include "gmdx11framebuffer.h"
#include "gmdx11gbuffer.h"
#include "gmdx11light.h"

void GMDx11Factory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	GM_ASSERT(engine);
	*engine = new GMDx11GraphicEngine();
}

void GMDx11Factory::createTexture(GMImage* image, OUT ITexture** texture)
{
	GM_ASSERT(texture);
	GMDx11Texture* t = new GMDx11Texture(image);
	(*texture) = t;
	t->init();
}

void GMDx11Factory::createModelDataProxy(IGraphicEngine* engine, GMModel* model, OUT GMModelDataProxy** modelDataProxy)
{
	GM_ASSERT(modelDataProxy);
	GMDx11GraphicEngine* dx11Engine = static_cast<GMDx11GraphicEngine*>(engine);
	(*modelDataProxy) = new GMDx11ModelDataProxy(dx11Engine, model);
}

void GMDx11Factory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	GM_ASSERT(glyphManager);
	*glyphManager = new GMDx11GlyphManager();
}

void GMDx11Factory::createFramebuffer(OUT IFramebuffer** fb)
{
	*fb = new GMDx11Framebuffer();
}

void GMDx11Factory::createFramebuffers(OUT IFramebuffers** fbs)
{
	*fbs = new GMDx11Framebuffers();
}

void GMDx11Factory::createGBuffer(IGraphicEngine* engine, OUT IGBuffer** g)
{
	*g = new GMDx11GBuffer(gm_cast<GMDx11GraphicEngine*>(engine));
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