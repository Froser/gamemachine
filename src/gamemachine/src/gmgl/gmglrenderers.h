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
	void setGamma(const GMShaderVariablesDesc* desc, GMGraphicEngine* engine, IShaderProgram* shaderProgram);

private:
	GMfloat m_gamma = 0.f;
};

GM_PRIVATE_OBJECT(GMGLRenderer)
{
	GMGLGraphicEngine* engine = nullptr;
	const GMShaderVariablesDesc* variablesDesc = nullptr;
	GMDebugConfig debugConfig;
	GMGammaHelper gammaHelper;
};

class GMGLRenderer : public GMObject, public IRenderer
{
	DECLARE_PRIVATE(GMGLRenderer)

public:
	GMGLRenderer();

	virtual void draw(GMModel* model) override;
	virtual IShaderProgram* getShaderProgram() = 0;

protected:
	virtual void beforeDraw(GMModel* model) = 0;
	virtual void afterDraw(GMModel* model) = 0;

protected:
	inline const GMShaderVariablesDesc* getVariablesDesc()
	{
		D(d);
		if (!d->variablesDesc)
		{
			d->variablesDesc = &GM.getGraphicEngine()->getShaderProgram()->getDesc();
		}
		return d->variablesDesc;
	}

protected:
	virtual void activateTextureTransform(GMModel* model, GMTextureType type, GMint index);
	virtual GMint activateTexture(GMModel* model, GMTextureType type, GMint index);
	virtual void deactivateTexture(GMTextureType type, GMint index);
	virtual GMint getTextureID(GMTextureType type, GMint index);
	virtual void drawTexture(GMModel* model, GMTextureType type, GMint index = 0);
	virtual ITexture* getTexture(GMTextureFrames& frames);
	virtual void updateCameraMatrices(IShaderProgram* shaderProgram);
	virtual void prepareScreenInfo(IShaderProgram* shaderProgram);

public:
	static void dirtyShadowMapAttributes();
};

GM_PRIVATE_OBJECT(GMGLRenderer_3D)
{
	GMRenderMode renderMode = GMRenderMode::Forward;
};

class GMGLRenderer_3D : public GMGLRenderer
{
	DECLARE_PRIVATE_AND_BASE(GMGLRenderer_3D, GMGLRenderer)

public:
	GMGLRenderer_3D() = default;

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual IShaderProgram* getShaderProgram() override;

protected:
	void activateMaterial(const GMShader& shader);
	void drawDebug();
};

class GMGLRenderer_2D : public GMGLRenderer_3D
{
public:
	virtual void beforeDraw(GMModel* model) override;
};

class GMGLRenderer_CubeMap : public GMGLRenderer_3D
{
public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
};

class GMGLRenderer_Filter : public GMGLRenderer
{
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual IShaderProgram* getShaderProgram() override;

protected:
	virtual GMint activateTexture(GMModel* model, GMTextureType type, GMint index);
};

class GMGLRenderer_LightPass : public GMGLRenderer
{
protected:
	virtual IShaderProgram* getShaderProgram() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
};

class GMGLRenderer_3D_Shadow : public GMGLRenderer_3D
{
protected:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
};

END_NS
#endif