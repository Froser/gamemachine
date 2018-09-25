#ifndef __GMGLFRAMEBUFFER_H__
#define __GMGLFRAMEBUFFER_H__
#include <gmcommon.h>
#include <gmassets.h>
BEGIN_NS

class GMGraphicEngine;
GM_PRIVATE_OBJECT(GMGLFramebuffer)
{
	const IRenderContext* context = nullptr;
	GMTextureAsset texture;
};

class GMGLFramebuffer : public GMObject, public IFramebuffer
{
	GM_DECLARE_PRIVATE(GMGLFramebuffer);

public:
	GMGLFramebuffer(const IRenderContext* context);

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void getTexture(REF GMTextureAsset& texture) override;
	virtual void setName(const GMString& name) override {}
	virtual const IRenderContext* getContext();

public:
	GMuint32 getTextureId();
};

class GMGLDefaultFramebuffers;
GM_PRIVATE_OBJECT(GMGLFramebuffers)
{
	GMuint32 fbo = 0;
	GMuint32 depthStencilBuffer = 0;
	Vector<GMGLFramebuffer*> framebuffers;
	bool framebuffersCreated = false;
	GMGraphicEngine* engine = nullptr;
	GMFramebufferDesc desc;
	GMfloat clearColor[4];
	const IRenderContext* context = nullptr;
};

class GMGLFramebuffers : public GMObject, public IFramebuffers
{
	GM_DECLARE_PRIVATE(GMGLFramebuffers)

public:
	GMGLFramebuffers(const IRenderContext* context);
	~GMGLFramebuffers();

	virtual bool init(const GMFramebuffersDesc& desc) override;
	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override;
	virtual void use() override;
	virtual void bind() override;
	virtual void unbind() override;
	virtual void clear(GMFramebuffersClearType type) override;
	virtual IFramebuffer* getFramebuffer(GMsize_t) override;
	virtual GMsize_t count() override;
	virtual void copyDepthStencilFramebuffer(IFramebuffers* dest) override;

public:
	virtual const IRenderContext* getContext();

public:
	virtual GMuint32 framebufferId();

	const GMFramebufferDesc& getDesc()
	{
		D(d);
		return d->desc;
	}

private:
	void createFramebuffers();

protected:
	virtual void createDepthStencilBuffer(const GMFramebufferDesc& desc);
	virtual void setViewport();

public:
	static IFramebuffers* createDefaultFramebuffers(const IRenderContext* context);
};

GM_PRIVATE_OBJECT(GMGLShadowMapTexture)
{
	GMuint32 textureId;
};

class GMGLShadowMapTexture : public ITexture
{
	GM_DECLARE_PRIVATE(GMGLShadowMapTexture)

public:
	GMGLShadowMapTexture(GMuint32 textureId);
	~GMGLShadowMapTexture();

	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override {}
	virtual void useTexture(GMint32) override;
};

GM_PRIVATE_OBJECT(GMGLShadowFramebuffers)
{
	GMint32 width = 0;
	GMint32 height = 0;
	GMTextureAsset shadowMapTexture;
};

class GMGLShadowFramebuffers : public GMGLFramebuffers
{
	GM_DECLARE_PRIVATE_AND_BASE(GMGLShadowFramebuffers, GMGLFramebuffers)

public:
	GMGLShadowFramebuffers(const IRenderContext* context);

public:
	virtual bool init(const GMFramebuffersDesc& desc) override;
	virtual void use() override;
	virtual void bind() override;

public:
	inline GMint32 getShadowMapWidth()
	{
		D(d);
		return d->width;
	}

	inline GMint32 getShadowMapHeight()
	{
		D(d);
		return d->height;
	}

	inline GMTextureAsset getShadowMapTexture()
	{
		D(d);
		return d->shadowMapTexture;
	}

private:
	virtual void createDepthStencilBuffer(const GMFramebufferDesc& desc);
};

END_NS
#endif