﻿#include "stdafx.h"
#include "gmglfactory.h"
#include "gmgltexture.h"
#include "gmglgraphic_engine.h"
#include "gmglmodelpainter.h"
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

void GMGLFactory::createPainter(IGraphicEngine* engine, GMModel* obj, OUT GMModelPainter** painter)
{
	ASSERT(painter);
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
		ASSERT(false);
		break;
	}
}

void GMGLFactory::createGlyphManager(OUT GMGlyphManager** glyphManager)
{
	*glyphManager = new GMGLGlyphManager();
}
