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
	GMFramebufferDesc desc;
};

class GMGLFramebuffers : public GMObject, public IFramebuffers
{
	DECLARE_PRIVATE(GMGLFramebuffers)

public:
	GMGLFramebuffers();
	~GMGLFramebuffers();

	virtual bool init(const GMFramebuffersDesc& desc) override;
	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override;
	virtual void bind() override;
	virtual void unbind() override;
	virtual void clear() override;
	virtual IFramebuffer* getFramebuffer(GMuint) override;
	virtual GMuint count() override;
	virtual void copyDepthStencilFramebuffer(IFramebuffers* dest) override;

public:
	virtual GMuint framebufferId();

	const GMFramebufferDesc& getDesc()
	{
		D(d);
		return d->desc;
	}

private:
	void createDepthStencilBuffer(const GMFramebufferDesc& desc);
	bool createFramebuffers();

public:
	static IFramebuffers* getDefaultFramebuffers();
};

END_NS
#endif