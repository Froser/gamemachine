#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include <gmcommon.h>
#include "gmdata/gmmodel.h"
#include "gmglshaderprogram.h"
BEGIN_NS

class GMGLGraphicEngine;
struct IRenderer;
GM_PRIVATE_OBJECT(GMGLModelPainter)
{
	GMGLGraphicEngine* engine = nullptr;
	bool inited = false;
};

class GMGLModelPainter : public GMModelPainter
{
	DECLARE_PRIVATE_AND_BASE(GMGLModelPainter, GMModelPainter)

public:
	GMGLModelPainter(IGraphicEngine* engine, GMModel* objs);

public:
	virtual void transfer() override;
	virtual void draw(const GMGameObject* parent) override;
	virtual void dispose(GMModelBuffer* md) override;

	virtual void beginUpdateBuffer(GMModel* model) override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;

private:
	void draw(IRenderer* render, GMModel* model);
};

END_NS
#endif