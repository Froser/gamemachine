#ifndef __GMGL_RENDERS_OBJECT_H__
#define __GMGL_RENDERS_OBJECT_H__
#include "common.h"
#include "gmdatacore/shader.h"
#include "gmdatacore/object.h"
BEGIN_NS

class GMGLGraphicEngine;
GM_PRIVATE_OBJECT(GMGLRenders_Object)
{
	GMGLGraphicEngine* engine;
	GMMesh* mesh;
	GMMeshType type;
	Shader* shader;
	GMDrawMode mode;
	GMGLShaderProgram* gmglShaderProgram;
};

class GMGLRenders_Object : public GMObject, public IRender
{
	DECLARE_PRIVATE(GMGLRenders_Object)

public:
	virtual void begin(IGraphicEngine* engine, GMMesh* mesh, GMfloat* modelTransform) override;
	virtual void beginShader(Shader& shader, GMDrawMode mode) override;
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) override;
	virtual void endShader() override;
	virtual void end() override;

protected:
	void clearData();
	void activateLight(LightType t, GMLight& light);
	void drawDebug();
	void drawTexture(GMTextureType type, GMint index = 0);
	ITexture* getTexture(GMTextureFrames& frames);
	void activateTextureTransform(GMTextureType i, GMint index);
	void activateTexture(GMTextureType type, GMint index);
	void deactivateTexture(GMTextureType type, GMint index);
	void activateShader();
	void deactivateShader();
};

END_NS
#endif