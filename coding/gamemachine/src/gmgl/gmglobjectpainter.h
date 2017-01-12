#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaders.h"
BEGIN_NS

class GMGLShaders;
class GMGLShadowMapping;
class GameWorld;
class GMGLObjectPainter : public ObjectPainter
{
public:
	GMGLObjectPainter(GMGLShaders& shaders, GMGLShadowMapping& shadowMapping, Object* obj);

public:
	virtual void transfer() override;
	virtual void draw() override;
	virtual void dispose() override;
	void setWorld(GameWorld* world);

private:
	void setLights(Material& material);
	void beginTextures(TextureInfo* startTexture);
	void endTextures(TextureInfo* startTexture);
	void resetTextures();

private:
	GMGLShaders& m_shaders;
	GMGLShadowMapping& m_shadowMapping;
	GameWorld* m_world;

private:
	bool m_inited;
};

END_NS
#endif