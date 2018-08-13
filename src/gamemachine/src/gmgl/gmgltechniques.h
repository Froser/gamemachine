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

GM_PRIVATE_OBJECT(GMGLTechnique)
{
	const IRenderContext* context = nullptr;
	GMGLGraphicEngine* engine = nullptr;
	IShaderProgram* lastShaderProgram_camera = nullptr;
	IShaderProgram* lastShaderProgram_screenInfo = nullptr;
	const GMShaderVariablesDesc* variablesDesc = nullptr;
	GMDebugConfig debugConfig;
	GMGammaHelper gammaHelper;
};

class GMGLTechnique : public GMObject, public ITechnique
{
	GM_DECLARE_PRIVATE(GMGLTechnique)

public:
	GMGLTechnique(const IRenderContext* context);

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
	virtual GMTextureAsset getTexture(GMTextureSampler& frames);
	virtual void updateCameraMatrices(IShaderProgram* shaderProgram);
	virtual void prepareScreenInfo(IShaderProgram* shaderProgram);

protected:
	void applyShader(GMModel* model);
	void prepareBlend(GMModel* model);
	void prepareFrontFace(GMModel* model);
	void prepareCull(GMModel* model);
	void prepareDepth(GMModel* model);
	void prepareLine(GMModel* model);
	void prepareDebug(GMModel* model);

public:
	static void dirtyShadowMapAttributes();
};

GM_PRIVATE_OBJECT(GMGLTechnique_3D)
{
	GMRenderMode renderMode = GMRenderMode::Forward;
	GMTextureAsset whiteTexture;
};

class GMGLTechnique_3D : public GMGLTechnique
{
	GM_DECLARE_PRIVATE_AND_BASE(GMGLTechnique_3D, GMGLTechnique)

public:
	using GMGLTechnique::GMGLTechnique;

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
	GMTextureAsset getWhiteTexture();
};

class GMGLTechnique_2D : public GMGLTechnique_3D
{
public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

public:
	virtual void beforeDraw(GMModel* model) override;
};

class GMGLTechnique_CubeMap : public GMGLTechnique_3D
{
public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
};

GM_PRIVATE_OBJECT(GMGLTechnique_Filter)
{
	struct HDRState
	{
		GMToneMapping::Mode toneMapping = GMToneMapping::Reinhard;
		bool HDR = false;
	};
	HDRState state;
};

class GMGLTechnique_Filter : public GMGLTechnique
{
	GM_DECLARE_PRIVATE_AND_BASE(GMGLTechnique_Filter, GMGLTechnique)

public:
	using GMGLTechnique::GMGLTechnique;

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

class GMGLTechnique_LightPass : public GMGLTechnique
{
public:
	using GMGLTechnique::GMGLTechnique;

protected:
	virtual IShaderProgram* getShaderProgram() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
};

class GMGLTechnique_3D_Shadow : public GMGLTechnique_3D
{
public:
	using GMGLTechnique_3D::GMGLTechnique_3D;

protected:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
};

class GMGLTechnique_Particle : public GMGLTechnique_2D
{
public:
	using GMGLTechnique_2D::GMGLTechnique_2D;
};

END_NS
#endif