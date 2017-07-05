#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaderprogram.h"
BEGIN_NS

class GMGLGraphicEngine;
class GMGLShaderProgram;
class GMGameWorld;
class GMGLObjectPainter : public GMObjectPainter
{
public:
	GMGLObjectPainter(IGraphicEngine* engine, Object* objs);

public:
	virtual void transfer() override;
	virtual void draw(GMfloat* modelTransform) override;
	virtual void dispose() override;
	virtual void clone(Object* obj, OUT GMObjectPainter** painter) override;

private:
	GMGLGraphicEngine* m_engine;

private:
	bool m_inited;
};

END_NS
#endif