#ifndef __GMGLRENDERERS_H__
#define __GMGLRENDERERS_H__
#include <gmcommon.h>
#include <gmenums.h>
#include "gmdata/gmmodel.h"
#include "gmgl/gmglgraphic_engine.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLRenderer)
{
	GMGLGraphicEngine* engine = nullptr;
	const GMShaderVariablesDesc* variablesDesc = nullptr;
	GMDebugConfig debugConfig;
};

class GMGLRenderer : public GMObject, public IRenderer
{
	DECLARE_PRIVATE(GMGLRenderer)

public:
	GMGLRenderer();

	virtual void draw(IQueriable* painter, GMModel* model) override;

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
};

GM_PRIVATE_OBJECT(GMGLRenderer_3D)
{
	GMRenderMode renderMode = GMRenderMode::Forward;
	GMGLDeferredRenderState renderState = GMGLDeferredRenderState::PassingGeometry;
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

protected:
	void activateMaterial(const GMShader& shader);
	void drawDebug();
	void drawTexture(GMModel* model, GMTextureType type, GMint index = 0);
	ITexture* getTexture(GMTextureFrames& frames);
	void activateTextureTransform(GMModel* model, GMTextureType type, GMint index);
	void activateTexture(GMModel* model, GMTextureType type, GMint index);
	void deactivateTexture(GMTextureType type, GMint index);
	void activateShader();
	void getTextureID(GMTextureType type, GMint index, REF GLenum& tex, REF GMint& texId);
};

class GMGLRenderer_2D : public GMGLRenderer_3D
{
public:
	virtual void beforeDraw(GMModel* model) override;
};

GM_PRIVATE_OBJECT(GMGLRenderer_CubeMap)
{
	const GMCubeMapGameObject* cubemap = nullptr;
};

class GMGLRenderer_CubeMap : public GMGLRenderer
{
	DECLARE_PRIVATE_AND_BASE(GMGLRenderer_CubeMap, GMGLRenderer)

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beforeDraw(GMModel* model) override;
	virtual void afterDraw(GMModel* model) override;
};

END_NS
#endif