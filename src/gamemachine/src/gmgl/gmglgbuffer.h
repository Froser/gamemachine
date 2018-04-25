#ifndef __GMGLGBUFFER_H__
#define __GMGLGBUFFER_H__
#include <gmcommon.h>
#include <gmgbuffer.h>
BEGIN_NS

void copyDepthBuffer(GMuint target);

class GMGLGraphicEngine;
GM_PRIVATE_OBJECT(GMGLGBuffer)
{
	GMGLGraphicEngine* engine = nullptr;
};

class GMGLGBuffer : public GMGBuffer
{
	DECLARE_PRIVATE(GMGLGBuffer)

public:
	GMGLGBuffer(GMGLGraphicEngine* engine);

protected:
	virtual IFramebuffers* createGeometryFramebuffers() override;
	virtual IFramebuffers* createMaterialFramebuffers() override;

public:
	virtual void geometryPass(GMGameObject *objects[], GMuint count) override;
	virtual void lightPass() override;

public:
	static const std::string* GBufferGeometryUniformNames();
	static const std::string* GBufferMaterialUniformNames();
};


END_NS
#endif