#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaders.h"
BEGIN_NS

struct IGraphicEngine;
class GMGLGraphicEngine;
class GMGLShaders;
class GMGLShadowMapping;
class GameWorld;
class GMGLObjectPainter : public ObjectPainter
{
public:
	GMGLObjectPainter(IGraphicEngine* engine, GMGLShadowMapping& shadowMapping, Object* objs);

public:
	virtual void transfer() override;
	virtual void draw() override;
	virtual void dispose() override;
	virtual void clone(Object* obj, OUT ObjectPainter** painter) override;

public:
	void setWorld(GameWorld* world);

private:
	void activeTexture(Shader* shader, TextureIndex i, ChildObject::ObjectType type);
	void activeTextureTransform(Shader* shader, TextureIndex i, ChildObject::ObjectType type);
	void deactiveTexture(TextureIndex i, ChildObject::ObjectType type);
	ITexture* getTexture(TextureFrames& frames);
	void activateShader(Shader* shader);
	void deactivateShader(Shader* shader);
	void beginShader(Shader* shader, ChildObject::ObjectType type);
	void endShader(Shader* shader, ChildObject::ObjectType type);
	void activateLight(LightType t, LightInfo& light, ChildObject::ObjectType objectType);
	void drawDebug(ChildObject::ObjectType type);

private:
	GMGLGraphicEngine* m_engine;
	GMGLShadowMapping& m_shadowMapping;
	GameWorld* m_world;

private:
	bool m_inited;
};

END_NS
#endif