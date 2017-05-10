#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaders.h"
BEGIN_NS

struct IGraphicEngine;
class GMGLGraphicEngine;
class GMGLShaders;
class GameWorld;
class GMGLObjectPainter : public ObjectPainter
{
public:
	GMGLObjectPainter(IGraphicEngine* engine, Object* objs);

public:
	virtual void transfer() override;
	virtual void draw(GMfloat* modelTransform) override;
	virtual void dispose() override;
	virtual void clone(Object* obj, OUT ObjectPainter** painter) override;

private:
	GMGLGraphicEngine* m_engine;

private:
	bool m_inited;
};

END_NS
#endif