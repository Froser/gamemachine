#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include <gmcommon.h>
#include <gmgbuffer.h>
BEGIN_NS

class GMGLGBuffer : public GMGBuffer
{
public:
	GMGLGBuffer(const IRenderContext* context);

protected:
	virtual IFramebuffers* createGeometryFramebuffers() override;

public:
	virtual void geometryPass(const GMGameObjectContainer& objects) override;
	virtual void lightPass() override;

public:
	void drawGeometryBuffer(GMuint32 index, const GMRect& rect);

public:
	static const GMString* GBufferGeometryUniformNames();
};


END_NS
#endif