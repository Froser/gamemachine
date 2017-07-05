#include "stdafx.h"
#include "gmglfactory.h"
#include "gmgltexture.h"
#include "gmglgraphic_engine.h"
#include "gmglobjectpainter.h"
#include "gmglgamepackagehandler.h"
#include "gmglglyphmanager.h"
#include "gmui/gmui_glwindow.h"

void GMGLFactory::createWindow(OUT GMUIWindow** window)
{
	ASSERT(window);
#if _WINDOWS
	*window = new GMUIGLWindow();
#elif defined __APPLE__
	;
#else
#error need implement
#endif
}

void GMGLFactory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	ASSERT(engine);
	*engine = new GMGLGraphicEngine();
}

void GMGLFactory::createTexture(AUTORELEASE GMImage* image, OUT ITexture** texture)
{
	ASSERT(texture);
	(*texture) = new GMGLTexture(image);
}

void GMGLFactory::createPainter(IGraphicEngine* engine, Object* obj, OUT GMObjectPainter** painter)
{
	ASSERT(painter);
	GMGLGraphicEngine* gmglEngine = static_cast<GMGLGraphicEngine*>(engine);
	(*painter) = new GMGLObjectPainter(gmglEngine, obj);
}

void GMGLFactory::createGamePackage(GMGamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler)
{
	switch (t)
	{
	case gm::GPT_DIRECTORY:
		{
			GMDefaultGLGamePackageHandler* h = new GMDefaultGLGamePackageHandler(pk);
			*handler = h;
		}
		break;
	case gm::GPT_ZIP:
		{
			ZipGMGLGamePackageHandler* h = new ZipGMGLGamePackageHandler(pk);
			*handler = h;
		}
		break;
	default:
		ASSERT(false);
		break;
	}
}

void GMGLFactory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	*glyphManager = new GMGLGlyphManager();
}
