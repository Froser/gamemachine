#include "stdafx.h"
#include "gmglfactory.h"
#include "utilities/assert.h"
#include "gmgltexture.h"
#include "gmglgraphic_engine.h"
#include "gmglobjectpainter.h"
#include "gmgllight.h"

void GMGLFactory::createGraphicEngine(OUT IGraphicEngine** engine)
{
	ASSERT(engine);
	*engine = new GMGLGraphicEngine();
}

void GMGLFactory::createTexture(Image* image, OUT ITexture** texture)
{
	ASSERT(texture);
	(*texture) = new GMGLTexture(image);
}

void GMGLFactory::createPainter(IGraphicEngine* engine, Object* obj, OUT ObjectPainter** painter)
{
	ASSERT(painter);
	GMGLGraphicEngine* gmglEngine = static_cast<GMGLGraphicEngine*>(engine);
	(*painter) = new GMGLObjectPainter(gmglEngine, gmglEngine->getShadowMapping(), obj);
}

void GMGLFactory::createLight(LightType type, OUT GameLight** light)
{
	ASSERT(light);

	switch (type)
	{
	case Ambient:
		*light = new GMGLAmbientLight();
		break;
	case Specular:
		*light = new GMGLSpecularLight();
		break;
	default:
		ASSERT(false);
		break;
	}
}