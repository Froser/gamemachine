#ifndef __GMGL_RENDERS_OBJECT_H__
#define __GMGL_RENDERS_OBJECT_H__
#include <gmcommon.h>
#include <gmenums.h>
#include "gmdata/gmmodel.h"
#include "gmgl/gmglgraphic_engine.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLRenders_3D)
{
	GMGLGraphicEngine* engine = nullptr;
	GMMesh* mesh = nullptr;
	GMShader* shader = nullptr;
	GMMeshType type;
	GMRenderMode renderMode = GMStates_RenderOptions::FORWARD;
	GMGLDeferredRenderState renderState = GMGLDeferredRenderState::PassingGeometry;
};

class GMGLRenders_3D : public GMObject, public IRender
{
	DECLARE_PRIVATE(GMGLRenders_3D)

public:
	GMGLRenders_3D();

public:
	virtual void begin(IGraphicEngine* engine, GMMesh* mesh, const GMfloat* modelTransform) override;
	virtual void beginShader(GMShader& shader) override;
	virtual void endShader() override;
	virtual void end() override;

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

END_NS
#endif