#include "stdafx.h"
#include "gmglfactory.h"
#include "utilities/assert.h"
#include "gmgltexture.h"
#include "gmglgraphic_engine.h"
#include "gmglobjectpainter.h"
#include "gmglgamepackagehandler.h"

void GMGLFactory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	ASSERT(engine);
	*engine = new GMGLGraphicEngine();
}

void GMGLFactory::createTexture(AUTORELEASE Image* image, OUT ITexture** texture)
{
	ASSERT(texture);
	(*texture) = new GMGLTexture(image);
}

void GMGLFactory::createPainter(IGraphicEngine* engine, Object* obj, OUT ObjectPainter** painter)
{
	ASSERT(painter);
	GMGLGraphicEngine* gmglEngine = static_cast<GMGLGraphicEngine*>(engine);
	(*painter) = new GMGLObjectPainter(gmglEngine, *gmglEngine->getShadowMapping(), obj);
}

void GMGLFactory::createGamePackage(GamePackage* pk, GamePackageType t, OUT IGamePackageHandler** handler)
{
	switch (t)
	{
	case gm::GPT_DIRECTORY:
		{
			DefaultGMGLGamePackageHandler* h = new DefaultGMGLGamePackageHandler(pk);
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
