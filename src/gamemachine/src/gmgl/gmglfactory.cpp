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
#include "gmglhelper.h"
#include <GL/glew.h>

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
	if (fb)
		*fb = new GMGLFramebuffer(context);
}

void GMGLFactory::createFramebuffers(const IRenderContext* context, OUT IFramebuffers** fbs)
{
	if (fbs)
		*fbs = new GMGLFramebuffers(context);
}

void GMGLFactory::createShadowFramebuffers(const IRenderContext* context, OUT IFramebuffers** fbs)
{
	if (fbs)
		*fbs = new GMGLShadowFramebuffers(context);
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
	GMGLShaderInfo s_vs, s_ps;
	s_vs = GMGLHelper::getDefaultShaderCode(GMShaderType::Vertex);
	s_ps = GMGLHelper::getDefaultShaderCode(GMShaderType::Pixel);
	for (auto& renderTechniques : manager.getRenderTechniques())
	{
		const auto& techniques = renderTechniques.getTechniques();
		GMGLShaderProgram* shaderProgram = new GMGLShaderProgram(context);
		bool hasVS = false, hasPS = false;
		for (auto& technique : techniques)
		{
			GMShaderType t = technique.getShaderType();
			if (t == GMShaderType::Vertex)
				hasVS = true;
			else if (t == GMShaderType::Pixel)
				hasPS = true;

			GMGLShaderInfo shaderInfo = { GMGLShaderInfo::toGLShaderType(t), technique.getCode(GMRenderEnvironment::OpenGL), t == GMShaderType::Vertex ? s_vs.filename : s_ps.filename };
			shaderProgram->attachShader(shaderInfo);
		}

		// 如果着色器程序没有VS或PS(FS)，则使用默认的
		if (!hasVS)
			shaderProgram->attachShader(s_vs);

		if (!hasPS)
			shaderProgram->attachShader(s_ps);

		shaderProgram->load();
		if (out)
			out->push_back(shaderProgram);
	}
}

bool GMGLFactory::createComputeShaderProgram(const IRenderContext* context, OUT IComputeShaderProgram** out)
{
	static GMint32 s_glVersion[2];
	static std::once_flag s_flag;
	std::call_once(s_flag, [](GMint32* version) {
		glGetIntegerv(GL_MAJOR_VERSION, &version[0]);
		glGetIntegerv(GL_MINOR_VERSION, &version[1]);
	}, s_glVersion);

	if (s_glVersion[0] < 4 || (s_glVersion[0] == 4 && s_glVersion[1] < 3))
		return false;

	if (out)
	{
		*out = new GMGLComputeShaderProgram(context);
	}
	return true;
}
