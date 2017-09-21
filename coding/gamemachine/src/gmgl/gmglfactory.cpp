#include "stdafx.h"
#include "gmglfactory.h"
#include "gmgltexture.h"
#include "gmglgraphic_engine.h"
#include "gmglmodelpainter.h"
#include "gmglglyphmanager.h"
#include "gmdatacore/gamepackage/gmgamepackagehandler.h"

void GMGLFactory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	GM_ASSERT(engine);
	*engine = new GMGLGraphicEngine();
}

void GMGLFactory::createTexture(GMImage* image, OUT ITexture** texture)
{
	GM_ASSERT(texture);
	(*texture) = new GMGLTexture(image);
}

void GMGLFactory::createPainter(IGraphicEngine* engine, GMModel* obj, OUT GMModelPainter** painter)
{
	GM_ASSERT(painter);
	GMGLGraphicEngine* gmglEngine = static_cast<GMGLGraphicEngine*>(engine);
	(*painter) = new GMGLModelPainter(gmglEngine, obj);
}

void GMGLFactory::createGamePackage(GMGamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler)
{
	switch (t)
	{
	case gm::GPT_DIRECTORY:
		{
			GMGLDefaultGamePackageHandler* h = new GMGLDefaultGamePackageHandler(pk);
			*handler = h;
		}
		break;
	case gm::GPT_ZIP:
		{
			GMGLZipGamePackageHandler* h = new GMGLZipGamePackageHandler(pk);
			*handler = h;
		}
		break;
	default:
		GM_ASSERT(false);
		break;
	}
}

void GMGLFactory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	*glyphManager = new GMGLGlyphManager();
}
