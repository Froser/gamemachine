#ifndef __GMDX11FRAMEBUFFER_H__
#define __GMDX11FRAMEBUFFER_H__
#include <gmcommon.h>
#include <gmdxincludes.h>
#include <gmcom.h>
#include "gmdx11texture.h"
#include <gmtools.h>
BEGIN_NS

struct GMDx11FramebufferDesc
{
	GMRect rect;
	UINT sampleCount;
	UINT sampleQuality;
};

GM_PRIVATE_OBJECT(GMDx11Framebuffer)
{
	ITexture* renderTexture = nullptr;
	GMComPtr<ID3D11RenderTargetView> renderTargetView;
};

class GMDx11Framebuffer : public GMObject
{
	DECLARE_PRIVATE(GMDx11Framebuffer)

public:
	GMDx11Framebuffer() = default;
	~GMDx11Framebuffer();

public:
	bool init(const GMDx11FramebufferDesc& desc);

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

class GMDx11Framebuffers : public GMObject
{
	DECLARE_PRIVATE(GMDx11Framebuffers)

public:
	GMDx11Framebuffers();
	~GMDx11Framebuffers();

public:
	void init(const GMDx11FramebufferDesc& desc);
	void addFramebuffer(AUTORELEASE GMDx11Framebuffer* framebuffer);
	void bind();
	void unbind();
	void clear();
	ITexture* getTexture(GMuint index);
};

END_NS
#endif