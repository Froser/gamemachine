#ifndef __GMGLFRAMEBUFFER_H__
#define __GMGLFRAMEBUFFER_H__
#include <gmcommon.h>
#include <gmassets.h>
#include <gmgraphicengine.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMGLFramebuffer);
class GMGLFramebuffer : public IFramebuffer
{
	GM_DECLARE_PRIVATE(GMGLFramebuffer)
	GM_DISABLE_COPY_ASSIGN(GMGLFramebuffer)

public:
	GMGLFramebuffer(const IRenderContext* context);
	~GMGLFramebuffer();

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void getTexture(REF GMTextureAsset& texture) override;
	virtual void setName(const GMString& name) override {}
	virtual const IRenderContext* getContext();

public:
	GMuint32 getTextureId();
};

class GMGLDefaultFramebuffers;
GM_PRIVATE_CLASS(GMGLFramebuffers);
class GMGLFramebuffers : public IFramebuffers
{
	GM_DECLARE_PRIVATE(GMGLFramebuffers)
	GM_DECLARE_BASE(GMGLFramebuffers)

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
	virtual void setClearColor(const GMfloat rgba[4]) override;

public:
	virtual const IRenderContext* getContext();
	virtual GMuint32 framebufferId();

public:
	const GMFramebufferDesc& getDesc();

protected:
	virtual void createDepthStencilBuffer(const GMFramebufferDesc& desc);
	virtual void setViewport();

public:
	static IFramebuffers* createDefaultFramebuffers(const IRenderContext* context);
};

GM_PRIVATE_CLASS(GMGLShadowMapTexture);
class GMGLShadowMapTexture : public ITexture
{
	GM_DECLARE_PRIVATE(GMGLShadowMapTexture)
	GM_DISABLE_COPY_ASSIGN(GMGLShadowMapTexture)

public:
	GMGLShadowMapTexture(GMuint32 textureId);
	~GMGLShadowMapTexture();

	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override {}
	virtual void useTexture(GMint32) override;
};

GM_PRIVATE_CLASS(GMGLShadowFramebuffers);
class GMGLShadowFramebuffers : public GMGLFramebuffers, public ICSMFramebuffers
{
	GM_DECLARE_PRIVATE(GMGLShadowFramebuffers)
	GM_DECLARE_BASE(GMGLFramebuffers)

public:
	GMGLShadowFramebuffers(const IRenderContext* context);

public:
	virtual bool init(const GMFramebuffersDesc& desc) override;
	virtual void use() override;
	virtual void bind() override;
	virtual GMCascadeLevel cascadedBegin() override;
	virtual GMCascadeLevel cascadedEnd() override;
	virtual void applyCascadedLevel(GMCascadeLevel) override;
	virtual GMCascadeLevel currentLevel() override;
	virtual GMfloat getEndClip(GMCascadeLevel) override;
	virtual void setEachCascadeEndClip(GMCascadeLevel) override;
	virtual void setShadowSource(const GMShadowSourceDesc& shadowSource) override;

public:
	GMint32 getShadowMapWidth();
	GMint32 getShadowMapHeight();
	GMTextureAsset getShadowMapTexture();

private:
	virtual void createDepthStencilBuffer(const GMFramebufferDesc& desc);
};

END_NS
#endif