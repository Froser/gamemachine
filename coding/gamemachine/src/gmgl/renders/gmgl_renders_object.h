#ifndef __GMGL_RENDERS_OBJECT_H__
#define __GMGL_RENDERS_OBJECT_H__
#include "common.h"
#include "gmgl_render.h"
#include "gmdatacore/shader.h"
#include "gmdatacore/object.h"
BEGIN_NS

class GMGLGraphicEngine;
class GameWorld;
struct ITexture;
struct GMGLRenders_ObjectPrivate
{
	GMGLGraphicEngine* engine;
	ChildObject* childObj;
	ChildObject::ObjectType type;
	Shader* shader;
	GMGLShaders* gmglShaders;
	GameWorld* world;
};

class GMGLRenders_Object : public IRender
{
	DEFINE_PRIVATE(GMGLRenders_Object)

public:
	virtual void begin(IGraphicEngine* engine, ChildObject* childObj, GMfloat* modelTransform) override;
	virtual void beginShader(Shader& shader) override;
	virtual void updateVPMatrices(const linear_math::Matrix4x4& projection, const linear_math::Matrix4x4& view, const CameraLookAt& lookAt) override;
	virtual void endShader() override;
	virtual void end() override;

protected:
	void clearData();
	void activateLight(LightType t, LightInfo& light);
	void drawDebug();
	ITexture* getTexture(TextureFrames& frames);
	void activeTextureTransform(Shader* shader, TextureIndex i);
	void activeTexture(Shader* shader, TextureIndex i);
	void deactiveTexture(TextureIndex i);
	void activateShader(Shader* shader);
	void deactivateShader(Shader* shader);
};

END_NS
#endif