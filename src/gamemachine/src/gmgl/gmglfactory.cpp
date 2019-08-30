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

BEGIN_NS

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
	static Vector<GMGLShaderInfo> s_vertexShaders, s_pixelShaders, s_vertexIncludes, s_pixelIncludes;
	if (s_vertexShaders.empty())
		s_vertexShaders = GMGLHelper::getDefaultShaderCodes(GMShaderType::Vertex);
	if (s_pixelShaders.empty())
		s_pixelShaders = GMGLHelper::getDefaultShaderCodes(GMShaderType::Pixel);
	if (s_vertexIncludes.empty())
		s_vertexIncludes = GMGLHelper::getDefaultShaderIncludes(GMShaderType::Vertex);
	if (s_pixelIncludes.empty())
		s_pixelIncludes = GMGLHelper::getDefaultShaderIncludes(GMShaderType::Pixel);

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
		}

		// 如果着色器程序没有VS或PS(FS)，则使用默认的，并且不使用includes
		if (!hasVS)
		{
			for (const auto& s : s_vertexShaders)
			{
				shaderProgram->attachShader(s);
			}
		}

		if (!hasPS)
		{
			for (const auto& s : s_pixelShaders)
			{
				shaderProgram->attachShader(s);
			}
		}

		for (auto& technique : techniques)
		{
			GMShaderType t = technique.getShaderType();
			GMGLShaderInfo shaderInfo = { GMGLShaderInfo::toGLShaderType(t), technique.getCode(GMRenderEnvironment::OpenGL), t == GMShaderType::Vertex ? "VS Memory" : "PS Memory" };
			// 如果一个自定义着色器程序指定了不忽略include路径（这个也是默认行为），那么在这个着色器代码前将会指定插入manifest文件中shaders/includes所指定的文件。
			// 但是，如果一个自定义着色器程序的某个着色器过程缺失，它将采用默认的着色器过程（来源于shaders/*[default=true]的结点），此时它便不会在这些默认着色器代码前插入shaders/includes所指定的文件。
			bool ignoreIncludes = technique.isNoIncludes();
			if (t == GMShaderType::Vertex && hasVS && !ignoreIncludes)
			{
				for (const auto& s : s_vertexIncludes)
				{
					shaderProgram->attachShader(s);
				}
			}
			else if (t == GMShaderType::Pixel && hasPS && !ignoreIncludes)
			{
				for (const auto& s : s_pixelIncludes)
				{
					shaderProgram->attachShader(s);
				}
			}
			shaderProgram->attachShader(shaderInfo);
		}

		shaderProgram->load();
		if (out)
			out->push_back(shaderProgram);
	}
}

bool GMGLFactory::createComputeShaderProgram(const IRenderContext* context, OUT IComputeShaderProgram** out)
{
	static bool s_canCreateComputeShader;
	static std::once_flag s_flag;
	std::call_once(s_flag, [](bool& result) {
		// 事实上，OpenGL ES 3.1开始支持了计算着色器。不过我们先屏蔽它
		if (GMGLHelper::isOpenGLShaderLanguageES())
		{
			result = false;
		}
		else
		{
			GMint32 version[2];
			glGetIntegerv(GL_MAJOR_VERSION, &version[0]);
			glGetIntegerv(GL_MINOR_VERSION, &version[1]);

			if (version[0] < 4 || (version[0] == 4 && version[1] < 3))
				result = false;
			else
				result = true;
		}
	}, s_canCreateComputeShader);

	if (!s_canCreateComputeShader)
		return false;

	if (out)
	{
		*out = new GMGLComputeShaderProgram(context);
	}
	return true;
}

void GMGLFactory::createComputeContext(OUT const IRenderContext** out)
{
	class ComputeEngine : public GMGLGraphicEngine
	{
		using GMGLGraphicEngine::GMGLGraphicEngine;

	public:
		ComputeEngine(const IRenderContext* context)
			: GMGLGraphicEngine(context)
		{
			// 需要先创建一个临时窗口，才能正常glewInit
			GMWindowFactory::createTempWindow(32, 8, 24, 8, hwnd, dc, rc);
			GLenum err = glewInit();
			GM_ASSERT(err == GLEW_OK);
			glGetError(); // glewInit bug, clear error
		}

		~ComputeEngine()
		{
			GMWindowFactory::destroyTempWindow(hwnd, dc, rc);
		}

	private:
		GMWindowHandle hwnd;
		GMDeviceContextHandle dc;
		GMOpenGLRenderContextHandle rc;
	};

	class RenderContext : public GMRenderContext
	{
	public:
		~RenderContext()
		{
			// Engine生命周期由Context管理
			if (getEngine())
				getEngine()->destroy();
		}

	public:
		virtual void switchToContext() const override
		{
		}
	};

	if (out)
	{
		RenderContext* ctx = new RenderContext();
		ctx->setEngine(new ComputeEngine(ctx));
		*out = ctx;
	}
}

class EngineCapabilityImpl : public IEngineCapability
{
public:
	virtual bool isSupportGeometryShader()
	{
		return GMGLHelper::isSupportGeometryShader();
	}

	virtual bool isSupportDeferredRendering()
	{
		return !GMGLHelper::isOpenGLShaderLanguageES();
	}
};

IEngineCapability& GMGLFactory::getEngineCapability()
{
	static EngineCapabilityImpl s_impl;
	return s_impl;
}

END_NS
