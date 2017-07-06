#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaderprogram.h"
BEGIN_NS

class GMGLGraphicEngine;
struct IRender;
GM_PRIVATE_OBJECT(GMGLObjectPainter)
{
	GMGLGraphicEngine* engine = nullptr;
	bool inited = false;
};

class GMGLObjectPainter : public GMObjectPainter
{
	DECLARE_PRIVATE(GMGLObjectPainter)

public:
	GMGLObjectPainter(IGraphicEngine* engine, Object* objs);

public:
	virtual void transfer() override;
	virtual void draw(GMfloat* modelTransform) override;
	virtual void dispose() override;

private:
	void draw(IRender* render, Shader& shader, Component* component, GMMesh* mesh, bool fill);
};

END_NS
#endif