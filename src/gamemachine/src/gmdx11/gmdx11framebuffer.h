#ifndef __GMDX11FRAMEBUFFER_H__
#define __GMDX11FRAMEBUFFER_H__
#include <gmcommon.h>
#include <gmdxincludes.h>
#include <gmcom.h>
#include "gmdx11texture.h"
#include <gmtools.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11Framebuffer)
{
	ITexture* renderTexture = nullptr;
	GMComPtr<ID3D11RenderTargetView> renderTargetView;
};

class GMDx11Framebuffer : public GMObject, public IFramebuffer
{
	DECLARE_PRIVATE(GMDx11Framebuffer)

public:
	GMDx11Framebuffer() = default;
	~GMDx11Framebuffer();

public:
	bool init(const GMFramebufferDesc& desc);

public:
	ID3D11RenderTargetView* getRenderTargetView()
	{
		D(d);
		GM_ASSERT(d->renderTargetView);
		return d->renderTargetView;
	}

	ITexture* getTexture()
	{
		D(d);
		GM_ASSERT(d->renderTexture);
		return d->renderTexture;
	}
};

GM_PRIVATE_OBJECT(GMDx11Framebuffers)
{
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<ID3D11RenderTargetView> defaultRenderTargetView;
	GMComPtr<ID3D11DepthStencilView> defaultDepthStencilView;

	Vector<GMDx11Framebuffer*> framebuffers;
	Vector<ID3D11RenderTargetView*> renderTargetViews;
	GMComPtr<ID3D11DepthStencilView> depthStencilView;
};

class GMDx11Framebuffers : public GMObject, public IFramebuffers
{
	DECLARE_PRIVATE(GMDx11Framebuffers)

public:
	GMDx11Framebuffers();
	~GMDx11Framebuffers();

public:
	virtual bool init(const GMFramebufferDesc& desc) override;
	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override;
	virtual void bind() override;
	virtual void unbind() override;
	virtual void clear() override;
	virtual IFramebuffer* getFramebuffer(GMuint) override;
};

END_NS
#endif