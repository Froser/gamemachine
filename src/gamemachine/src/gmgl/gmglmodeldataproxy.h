#ifndef __OBJECT_PAINTER_H__
#define __OBJECT_PAINTER_H__
#include <gmcommon.h>
#include "gmdata/gmmodel.h"
#include "gmglshaderprogram.h"
BEGIN_NS

class GMGLGraphicEngine;
struct ITechnique;
GM_PRIVATE_CLASS(GMGLModelDataProxy);
class GMGLModelDataProxy : public GMModelDataProxy
{
	GM_DECLARE_PRIVATE(GMGLModelDataProxy)
	GM_DECLARE_BASE(GMModelDataProxy)

public:
	GMGLModelDataProxy(const IRenderContext* context, GMModel* objs);

public:
	virtual void transfer() override;
	virtual void dispose(GMModelBuffer* md) override;

	virtual void beginUpdateBuffer(GMModelBufferType type) override;
	virtual void endUpdateBuffer() override;
	virtual void* getBuffer() override;
};

END_NS
#endif