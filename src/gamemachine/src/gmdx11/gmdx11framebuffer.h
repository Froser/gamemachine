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

GM_PRIVATE_OBJECT(GMDx11Framebuffer)
{
	const IRenderContext* context = nullptr;
	GMTextureAsset renderTexture;
	GMComPtr<ID3D11RenderTargetView> renderTargetView;
	GMString name;
};

class GMDx11Framebuffer : public GMObject, public IFramebuffer
{
	GM_DECLARE_PRIVATE(GMDx11Framebuffer)

public:
	GMDx11Framebuffer(const IRenderContext* context);

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void setName(const GMString& name) override;
	virtual void getTexture(REF GMTextureAsset& texture) override;

public:
	virtual const IRenderContext* getContext();

public:
	ID3D11RenderTargetView* getRenderTargetView()
	{
		D(d);
		GM_ASSERT(d->renderTargetView);
		return d->renderTargetView;
	}
};

class GMGraphicEngine;
GM_PRIVATE_OBJECT(GMDx11Framebuffers)
{
	const IRenderContext* context = nullptr;
	GMComPtr<ID3D11DeviceContext> deviceContext;
	Vector<GMOwnedPtr<GMDx11Framebuffer>> framebuffers;
	Vector<ID3D11RenderTargetView*> renderTargetViews;
	GMComPtr<ID3D11DepthStencilView> depthStencilView;
	GMComPtr<ID3D11Texture2D> depthStencilTexture;
	GMGraphicEngine* engine = nullptr;
	GMfloat clearColor[4];
	D3D11_VIEWPORT viewport;
};

class GMDx11Framebuffers : public GMObject, public IFramebuffers
{
	GM_DECLARE_PRIVATE(GMDx11Framebuffers)

public:
	GMDx11Framebuffers(const IRenderContext* context);
	~GMDx11Framebuffers() = default;

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

GM_PRIVATE_OBJECT(GMDx11ShadowFramebuffers)
{
	GMComPtr<ID3D11ShaderResourceView> depthShaderResourceView;
	GMint32 width = 0;
	GMint32 height = 0;
	GMShadowSourceDesc shadowSource;
	D3D11_VIEWPORT viewports[GMGraphicEngine::getMaxCascades()];
	GMfloat cascadeEndClip[GMGraphicEngine::getMaxCascades()] = { 0 };
	GMCascadeLevel currentViewport;
};

class GMDx11ShadowFramebuffers : public GMDx11Framebuffers, public ICSMFramebuffers
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11ShadowFramebuffers, GMDx11Framebuffers)

public:
	using GMDx11Framebuffers::GMDx11Framebuffers;

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

public:
	ID3D11ShaderResourceView* getShadowMapShaderResourceView();

public:
	inline GMint32 getShadowMapWidth() GM_NOEXCEPT
	{
		D(d);
		return d->width;
	}

	inline GMint32 getShadowMapHeight() GM_NOEXCEPT
	{
		D(d);
		return d->height;
	}
};

END_NS
#endif