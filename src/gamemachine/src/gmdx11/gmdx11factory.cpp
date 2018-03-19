#include "stdafx.h"
#include "gmdx11factory.h"
#include "gmdx11graphic_engine.h"
#include "gmdx11modelpainter.h"
#include "gmdx11texture.h"
#include "gmdx11glyphmanager.h"

void GMDx11Factory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	GM_ASSERT(engine);
	*engine = new GMDx11GraphicEngine();
}

void GMDx11Factory::createTexture(GMImage* image, OUT ITexture** texture)
{
	GM_ASSERT(texture);
	*texture = new GMDx11Texture(image);
}

void GMDx11Factory::createPainter(IGraphicEngine* engine, GMModel* model, OUT GMModelPainter** painter)
{
	GM_ASSERT(painter);
	GMDx11GraphicEngine* dx11Engine = static_cast<GMDx11GraphicEngine*>(engine);
	(*painter) = new GMDx11ModelPainter(dx11Engine, model);
}

void GMDx11Factory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	GM_ASSERT(glyphManager);
	*glyphManager = new GMDx11GlyphManager();
}