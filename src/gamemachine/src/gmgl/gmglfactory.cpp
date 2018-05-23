#include "stdafx.h"
#include "gmglfactory.h"
#include "gmgltexture.h"
#include "gmglgraphic_engine.h"
#include "gmglmodeldataproxy.h"
#include "gmglglyphmanager.h"
#include "gmdata/gamepackage/gmgamepackagehandler.h"
#include "gmglframebuffer.h"
#include "gmglgbuffer.h"
#include "gmgllight.h"

void GMGLFactory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	GM_ASSERT(engine);
	*engine = new GMGLGraphicEngine();
}

void GMGLFactory::createTexture(GMImage* image, OUT ITexture** texture)
{
	GM_ASSERT(texture);
	GMGLTexture* t = new GMGLTexture(image);
	(*texture) = t;
	t->init();
}

void GMGLFactory::createModelDataProxy(IGraphicEngine* engine, GMModel* model, OUT GMModelDataProxy** modelDataProxy)
{
	GM_ASSERT(modelDataProxy);
	GMGLGraphicEngine* gmglEngine = static_cast<GMGLGraphicEngine*>(engine);
	(*modelDataProxy) = new GMGLModelDataProxy(gmglEngine, model);
}

void GMGLFactory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	*glyphManager = new GMGLGlyphManager();
}

void GMGLFactory::createFramebuffer(OUT IFramebuffer** fb)
{
	*fb = new GMGLFramebuffer();
}

void GMGLFactory::createFramebuffers(OUT IFramebuffers** fbs)
{
	*fbs = new GMGLFramebuffers();
}

void GMGLFactory::createGBuffer(IGraphicEngine* engine, OUT IGBuffer** g)
{
	*g = new GMGLGBuffer(gm_cast<GMGLGraphicEngine*>(engine));
}

void GMGLFactory::createLight(GMLightType type, OUT ILight** out)
{
	if (type == GMLightType::Ambient)
	{
		*out = new GMGLAmbientLight();
	}
	else
	{
		GM_ASSERT(type == GMLightType::Direct);
		*out = new GMGLDirectLight();
	}
}