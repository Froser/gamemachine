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

void GMGLFactory::createWindow(GMInstance instance, IWindow* parent, OUT IWindow** window)
{
	bool b = GMWindowFactory::createWindowWithOpenGL(instance, parent, window);
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
	if (out)
	{
		if (type == GMLightType::PointLight)
			*out = new GMGLPointLight();
		else if (type == GMLightType::DirectionalLight)
			*out = new GMGLDirectionalLight();
		else if (type == GMLightType::Spotlight)
			*out = new GMGLSpotlight();
	}
}

void GMGLFactory::createWhiteTexture(const IRenderContext* context, REF GMTextureAsset& out)
{
	GMGLWhiteTexture* t = new GMGLWhiteTexture(context);
	t->init();
	out = GMAsset(GMAssetType::Texture, t);
}

void GMGLFactory::createEmptyTexture(const IRenderContext* context, REF GMTextureAsset& out)
{
	GMGLEmptyTexture* t = new GMGLEmptyTexture(context);
	t->init();
	out = GMAsset(GMAssetType::Texture, t);
}

void GMGLFactory::createShaderPrograms(const IRenderContext* context, const GMRenderTechniqueManager& manager, REF Vector<IShaderProgram*>* out)
{
	for (auto& renderTechniques : manager.getRenderTechniques())
	{
		const auto& techniques = renderTechniques.getTechniques();
		GMGLShaderProgram* shaderProgram = new GMGLShaderProgram(context);
		for (auto& technique : techniques)
		{
			GMGLShaderInfo shaderInfo = { GMGLShaderInfo::toGLShaderType(technique.getShaderType()), technique.getCode(GMRenderEnvironment::OpenGL) };
			shaderProgram->attachShader(shaderInfo);
		}

		shaderProgram->load();
		if (out)
			out->push_back(shaderProgram);
	}
}