#ifndef __GMGL_RENDERS_OBJECT_H__
#define __GMGL_RENDERS_OBJECT_H__
#include <gmcommon.h>
#include "gmdatacore/shader.h"
#include "gmdatacore/gmmodel.h"
#include "gmgl/gmglgraphic_engine.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMGLRenders_Object)
{
	GMGLGraphicEngine* engine = nullptr;
	GMMesh* mesh = nullptr;
	Shader* shader = nullptr;
	GMGLShaderProgram* gmglShaderProgram = nullptr;
	GMMeshType type;
	GMDrawMode mode;
	GMRenderMode renderMode = GMStates_RenderOptions::FORWARD;
	GMGLDeferredRenderState renderState = GMGLDeferredRenderState::PassingGeometry;
};

class GMGLRenders_Object : public GMObject, public IRender
{
	DECLARE_PRIVATE(GMGLRenders_Object)

public:
	GMGLRenders_Object();

public:
	virtual void begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform) override;
	virtual void beginShader(Shader& shader, GMDrawMode mode) override;
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) override;
	virtual void endShader() override;
	virtual void end() override;
	virtual void activateLights(const GMLight* lights, GMint count);

protected:
	GMGLShaderProgram* getShaderProgram();
	void activateMaterial(const Shader& shader);
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