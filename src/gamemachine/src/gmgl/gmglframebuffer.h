#ifndef __GMGLFRAMEBUFFER_H__
#define __GMGLFRAMEBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

class GMGraphicEngine;
GM_PRIVATE_OBJECT(GMGLFramebuffer)
{
	ITexture* texture = nullptr;
};

class GMGLFramebuffer : public GMObject, public IFramebuffer
{
	DECLARE_PRIVATE(GMGLFramebuffer);

public:
	~GMGLFramebuffer();

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual ITexture* getTexture() override;
	virtual void setName(const GMString& name) override {}

public:
	GMuint getTextureId();
};

GM_PRIVATE_OBJECT(GMGLFramebuffers)
{
	GMuint fbo = 0;
	GMuint depthStencilBuffer = 0;
	Vector<GMGLFramebuffer*> framebuffers;
	bool framebuffersCreated = false;
	GMGraphicEngine* engine = nullptr;
};

class GMGLFramebuffers : public GMObject, public IFramebuffers
{
	DECLARE_PRIVATE(GMGLFramebuffers)

public:
	GMGLFramebuffers();
	~GMGLFramebuffers();

	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override;
	virtual void bind() override;
	virtual void unbind() override;
	virtual void clear() override;
	virtual IFramebuffer* getFramebuffer(GMuint) override;
	virtual GMuint count() override;

public:
	inline GMuint framebufferId()
	{
		D(d);
		return d->fbo;
	}

private:
	void createDepthStencilBuffer(const GMFramebufferDesc& desc);
	bool createFramebuffers();
};

END_NS
#endif