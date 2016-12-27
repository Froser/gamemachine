#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaders.h"
BEGIN_NS

class GMGLShaders;
class GMGLShadowMapping;
class GMGLObjectPainter : public ObjectPainter
{
public:
	GMGLObjectPainter(GMGLShaders& shaders, GMGLShadowMapping& shadowMapping, Object* obj);

public:
	virtual void init() override;
	virtual void draw() override;
	virtual void dispose() override;

private:
	GMGLShaders& m_shaders;
	GMGLShadowMapping& m_shadowMapping;
};

END_NS
#endif