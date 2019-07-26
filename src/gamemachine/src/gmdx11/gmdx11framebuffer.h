#ifndef __GMDX11FRAMEBUFFER_H__
#define __GMDX11FRAMEBUFFER_H__
#include <gmcommon.h>
#include <gmdxincludes.h>
#include <gmcom.h>
#include "gmdx11texture.h"
#include <gmtools.h>
#include <gmassets.h>
#include <gmgraphicengine.h>

BEGIN_NS

GM_PRIVATE_CLASS(GMDx11Framebuffer);
class GMDx11Framebuffer : public IFramebuffer
{
	GM_DECLARE_PRIVATE(GMDx11Framebuffer)
	GM_DISABLE_COPY_ASSIGN(GMDx11Framebuffer)

public:
	GMDx11Framebuffer(const IRenderContext* context);

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void setName(const GMString& name) override;
	virtual void getTexture(REF GMTextureAsset& texture) override;

public:
	virtual const IRenderContext* getContext();

public:
	ID3D11RenderTargetView* getRenderTargetView();
};

class GMGraphicEngine;
GM_PRIVATE_CLASS(GMDx11Framebuffers);
class GMDx11Framebuffers : public IFramebuffers
{
	GM_DECLARE_PRIVATE(GMDx11Framebuffers)
	GM_DISABLE_COPY_ASSIGN(GMDx11Framebuffers)

public:
	GMDx11Framebuffers(const IRenderContext* context);
	~GMDx11Framebuffers();

public:
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

protected:
	D3D11_TEXTURE2D_DESC getDepthTextureDesc();

public:
	static IFramebuffers* createDefaultFramebuffers(const IRenderContext* context);
};

GM_PRIVATE_CLASS(GMDx11ShadowFramebuffers);
class GMDx11ShadowFramebuffers : public GMDx11Framebuffers, public ICSMFramebuffers
{
	GM_DECLARE_PRIVATE(GMDx11ShadowFramebuffers)
	GM_DECLARE_BASE(GMDx11Framebuffers)

public:
	GMDx11ShadowFramebuffers(const IRenderContext* context);
	~GMDx11ShadowFramebuffers();

public:
	virtual bool init(const GMFramebuffersDesc& desc) override;
	virtual void use() override;

public:
	virtual void setShadowSource(const GMShadowSourceDesc& shadowSource);
	virtual GMCascadeLevel cascadedBegin() override;
	virtual GMCascadeLevel cascadedEnd() override;
	virtual void applyCascadedLevel(GMCascadeLevel) override;
	virtual GMCascadeLevel currentLevel() override;
	virtual GMfloat getEndClip(GMCascadeLevel) override;
	virtual void setEachCascadeEndClip(GMCascadeLevel) override;

public:
	ID3D11ShaderResourceView* getShadowMapShaderResourceView();
	GMint32 getShadowMapWidth() GM_NOEXCEPT;
	GMint32 getShadowMapHeight() GM_NOEXCEPT;
};

END_NS
#endif