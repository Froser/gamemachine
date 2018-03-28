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
};

class GMGLRenderer : public GMObject, public IRenderer
{
	DECLARE_PRIVATE(GMGLRenderer)

public:
	GMGLRenderer();

	virtual void draw(IQueriable* painter, GMComponent* component, GMMesh* mesh) override;

protected:
	virtual void beforeDraw(GMComponent* component) = 0;
	virtual void afterDraw() = 0;

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
	GMShader* shader = nullptr;
	GMRenderMode renderMode = GMStates_RenderOptions::FORWARD;
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
	virtual void beforeDraw(GMComponent* component) override;
	virtual void afterDraw() override;

protected:
	void activateMaterial(const GMShader& shader);
	void drawDebug();
	void drawTexture(GMTextureType type, GMint index = 0);
	ITexture* getTexture(GMTextureFrames& frames);
	void activateTextureTransform(GMTextureType type, GMint index);
	void activateTexture(GMTextureType type, GMint index);
	void deactivateTexture(GMTextureType type, GMint index);
	void activateShader();
	void getTextureID(GMTextureType type, GMint index, REF GLenum& tex, REF GMint& texId);
};

class GMGLRenderer_2D : public GMGLRenderer_3D
{
public:
	virtual void beforeDraw(GMComponent* component) override;
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
	virtual void beforeDraw(GMComponent* component) override;
	virtual void afterDraw() override;
};

END_NS
#endif