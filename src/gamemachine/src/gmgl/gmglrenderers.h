#ifndef __GMGLRENDERERS_H__
#define __GMGLRENDERERS_H__
#include <gmcommon.h>
#include <gmenums.h>
#include "gmdata/gmmodel.h"
#include "gmgl/gmglgraphic_engine.h"
BEGIN_NS

class GMGammaHelper
{
public:
	void setGamma(GMGraphicEngine* engine, IShaderProgram* shaderProgram);

private:
	GMfloat m_gamma = 0.f;
};

GM_PRIVATE_OBJECT(GMGLRenderer)
{
	const IRenderContext* context = nullptr;
	GMGLGraphicEngine* engine = nullptr;
	IShaderProgram* lastShaderProgram_camera = nullptr;
	IShaderProgram* lastShaderProgram_screenInfo = nullptr;
	const GMShaderVariablesDesc* variablesDesc = nullptr;
	GMDebugConfig debugConfig;
	GMGammaHelper gammaHelper;
};

class GMGLRenderer : public GMObject, public IRenderer
{
	GM_DECLARE_PRIVATE(GMGLRenderer)

public:
	GMGLRenderer(const IRenderContext* context);

	virtual void draw(GMModel* model) override;
	virtual IShaderProgram* getShaderProgram() = 0;

protected:
	virtual void beforeDraw(GMModel* model) = 0;
	virtual void afterDraw(GMModel* model) = 0;

protected:
	virtual void activateTextureTransform(GMModel* model, GMTextureType type);
	virtual GMint activateTexture(GMModel* model, GMTextureType type);
	virtual void deactivateTexture(GMTextureType type);
	virtual GMint getTextureID(GMTextureType type);
	virtual bool drawTexture(GMModel* model, GMTextureType type);
	virtual ITexture* getTexture(GMTextureSampler& frames);
	virtual void updateCameraMatrices(IShaderProgram* shaderProgram);
	virtual void prepareScreenInfo(IShaderProgram* shaderProgram);

public:
	static void dirtyShadowMapAttributes();
};

GM_PRIVATE_OBJECT(GMGLRenderer_3D)
{
	GMRenderMode renderMode = GMRenderMode::Forward;
	ITexture* whiteTexture = nullptr;
};

class GMGLRenderer_3D : public GMGLRenderer
{
	DECLARE_PRIVATE_AND_BASE(GMGLRenderer_3D, GMGLRenderer)

public:
	using GMGLRenderer::GMGLRenderer;
	~GMGLRenderer_3D();

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual IShaderProgram* getShaderProgram() override;

protected:
	void activateMaterial(const GMShader& shader);
	void drawDebug();

private:
	ITexture* getWhiteTexture();
};

class GMGLRenderer_2D : public GMGLRenderer_3D
{
public:
	using GMGLRenderer_3D::GMGLRenderer_3D;

public:
	virtual void beforeDraw(GMModel* model) override;
};

class GMGLRenderer_CubeMap : public GMGLRenderer_3D
{
public:
	using GMGLRenderer_3D::GMGLRenderer_3D;

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
};

GM_PRIVATE_OBJECT(GMGLRenderer_Filter)
{
	struct HDRState
	{
		GMToneMapping::Mode toneMapping = GMToneMapping::Reinhard;
		bool HDR = false;
	};
	HDRState state;
};

class GMGLRenderer_Filter : public GMGLRenderer
{
	DECLARE_PRIVATE_AND_BASE(GMGLRenderer_Filter, GMGLRenderer)

public:
	using GMGLRenderer::GMGLRenderer;

private:
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual IShaderProgram* getShaderProgram() override;

protected:
	virtual GMint activateTexture(GMModel* model, GMTextureType type);

private:
	void setHDR(IShaderProgram* shaderProgram);
};

class GMGLRenderer_LightPass : public GMGLRenderer
{
public:
	using GMGLRenderer::GMGLRenderer;

protected:
	virtual IShaderProgram* getShaderProgram() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
};

class GMGLRenderer_3D_Shadow : public GMGLRenderer_3D
{
public:
	using GMGLRenderer_3D::GMGLRenderer_3D;

protected:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
};

END_NS
#endif