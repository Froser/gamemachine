#include "stdafx.h"
#include "gmdx11factory.h"
#include "gmdx11graphic_engine.h"
#include "gmdx11modelpainter.h"

void GMDX11Factory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	GM_ASSERT(engine);
	*engine = new GMDx11GraphicEngine();
}

void GMDX11Factory::createTexture(GMImage* image, OUT ITexture** texture)
{
	GM_ASSERT(texture);
	*texture = nullptr;
}

void GMDX11Factory::createPainter(IGraphicEngine* engine, GMModel* model, OUT GMModelPainter** painter)
{
	GM_ASSERT(painter);
	GMDx11GraphicEngine* dx11Engine = static_cast<GMDx11GraphicEngine*>(engine);
	(*painter) = new GMDx11ModelPainter(dx11Engine, model);
}

void GMDX11Factory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	*glyphManager = nullptr;
}