#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include "common.h"
#include "gmdatacore/gmmodel.h"
#include "gmglshaderprogram.h"
BEGIN_NS

class GMGLGraphicEngine;
struct IRender;
GM_PRIVATE_OBJECT(GMGLModelPainter)
{
	GMGLGraphicEngine* engine = nullptr;
	bool inited = false;
};

class GMGLModelPainter : public GMModelPainter
{
	DECLARE_PRIVATE(GMGLModelPainter)

public:
	GMGLModelPainter(IGraphicEngine* engine, GMModel* objs);

public:
	virtual void transfer() override;
	virtual void draw(GMfloat* modelTransform) override;
	virtual void dispose() override;

	virtual void beginUpdateBuffer(GMMesh* mesh) override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;

private:
	void draw(IRender* render, Shader& shader, GMComponent* component, GMMesh* mesh, bool fill);
};

END_NS
#endif