#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include <gmcommon.h>
#include <gmgbuffer.h>
BEGIN_NS

class GMGLGBuffer : public GMGBuffer
{
public:
	GMGLGBuffer(GMGraphicEngine* engine);

protected:
	virtual IFramebuffers* createGeometryFramebuffers() override;

public:
	virtual void geometryPass(const List<GMGameObject*>& objects) override;
	virtual void lightPass() override;

public:
	void drawGeometryBuffer(GMuint index, const GMRect& rect);

public:
	static const std::string* GBufferGeometryUniformNames();
};


END_NS
#endif