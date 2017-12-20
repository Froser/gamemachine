#ifndef __GMGLRENDERERS_H__
#define __GMGLRENDERERS_H__
#include <gmcommon.h>
#include <gmenums.h>
#include "gmdata/gmmodel.h"
#include "gmgl/gmglgraphic_engine.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLRenderer_3D)
{
	GMGLGraphicEngine* engine = nullptr;
	GMShader* shader = nullptr;
	GMRenderMode renderMode = GMStates_RenderOptions::FORWARD;
	GMGLDeferredRenderState renderState = GMGLDeferredRenderState::PassingGeometry;
};

class GMGLRenderer_3D : public GMObject, public IRenderer
{
	DECLARE_PRIVATE(GMGLRenderer_3D)

public:
	GMGLRenderer_3D() = default;

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beginComponent(GMComponent* component) override;
	virtual void endComponent() override;

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
	virtual void beginComponent(GMComponent* component) override;
};

class GMGLRenderer_CubeMap : public IRenderer
{
public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void beginComponent(GMComponent* component) override;
	virtual void endComponent() override;
};

END_NS
#endif