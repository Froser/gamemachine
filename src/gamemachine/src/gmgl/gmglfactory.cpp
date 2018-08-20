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
#include "gmengine/ui/gmwindow.h"

void GMGLFactory::createWindow(GMInstance instance, OUT IWindow** window)
{
	bool b = GMWindowFactory::createWindowWithOpenGL(instance, window);
	GM_ASSERT(b);
}

void GMGLFactory::createTexture(const IRenderContext* context, GMImage* image, REF GMTextureAsset& texture)
{
	GMGLTexture* t = new GMGLTexture(image);
	t->init();
	texture = GMAsset(GMAssetType::Texture, t);
}

void GMGLFactory::createModelDataProxy(const IRenderContext* context, GMModel* model, OUT GMModelDataProxy** modelDataProxy)
{
	GM_ASSERT(modelDataProxy);
	(*modelDataProxy) = new GMGLModelDataProxy(context, model);
}

void GMGLFactory::createGlyphManager(const IRenderContext* context, OUT GMGlyphManager** glyphManager)
{
	*glyphManager = new GMGLGlyphManager(context);
}

void GMGLFactory::createFramebuffer(const IRenderContext* context, OUT IFramebuffer** fb)
{
	*fb = new GMGLFramebuffer(context);
}

void GMGLFactory::createFramebuffers(const IRenderContext* context, OUT IFramebuffers** fbs)
{
	*fbs = new GMGLFramebuffers(context);
}

void GMGLFactory::createGBuffer(const IRenderContext* context, OUT IGBuffer** g)
{
	*g = new GMGLGBuffer(context);
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

void GMGLFactory::createWhiteTexture(const IRenderContext* context, REF GMTextureAsset& out)
{
	GMGLWhiteTexture* t = new GMGLWhiteTexture(context);
	t->init();
	out = GMAsset(GMAssetType::Texture, t);
}

void GMGLFactory::createShaderProgram(const IRenderContext* context, const GMRenderTechniques& renderTechniques, OUT IShaderProgram** out)
{
	const auto& techniques = renderTechniques.getTechniques();
	GMGLShaderProgram* shaderProgram = new GMGLShaderProgram(context);
	for (auto& technique : techniques)
	{
		GMGLShaderInfo shaderInfo = { GMGLShaderInfo::toGLShaderType(technique.getShaderType()), technique.getCode(GMRenderTechniqueEngineType::OpenGL) };
		shaderProgram->attachShader(shaderInfo);
	}

	shaderProgram->load();
	if (out)
		*out = shaderProgram;
}