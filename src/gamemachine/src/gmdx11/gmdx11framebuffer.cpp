#include "stdafx.h"
#include "gmdx11framebuffer.h"
#include "foundation/gamemachine.h"
#include <gmimagebuffer.h>
#include "gmengine/gmgraphicengine.h"
#include "gmdx11helper.h"

BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11DefaultFramebuffers)
{
	GMComPtr<ID3D11RenderTargetView> defaultRenderTargetView;
};

class GMDx11DefaultFramebuffers : public GMDx11Framebuffers
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11DefaultFramebuffers, GMDx11Framebuffers);

public:
	GMDx11DefaultFramebuffers(const IRenderContext* context)
		: Base(context)
	{
		D(d);
		D_BASE(db, Base);
		db->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11DepthStencilView, (void**)&db->depthStencilView);
		GM_ASSERT(db->depthStencilView);
		db->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11DepthStencilTexture, (void**)&db->depthStencilTexture);
		GM_ASSERT(db->depthStencilTexture);
		db->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11RenderTargetView, (void**)&d->defaultRenderTargetView);
		GM_ASSERT(d->defaultRenderTargetView);
		db->renderTargetViews.push_back(d->defaultRenderTargetView);

		const GMWindowStates& windowStates = db->context->getWindow()->getWindowStates();
		db->viewport.TopLeftX = windowStates.viewportTopLeftX;
		db->viewport.TopLeftY = windowStates.viewportTopLeftY;
		db->viewport.Width = windowStates.renderRect.width;
		db->viewport.Height = windowStates.renderRect.height;
		db->viewport.MinDepth = windowStates.minDepth;
		db->viewport.MaxDepth = windowStates.maxDepth;
	}

public:
	virtual bool init(const GMFramebuffersDesc& desc) override
	{
		return false;
	}

	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override
	{
	}

	virtual GMsize_t count() override
	{
		return 1;
	}

	virtual IFramebuffer* getFramebuffer(GMsize_t) override
	{
		return nullptr;
	}
};

GM_PRIVATE_OBJECT(GMDx11FramebufferTexture)
{
	GMFramebufferDesc desc;
};

class GMDx11FramebufferTexture : public GMDx11Texture
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11FramebufferTexture, GMDx11Texture);

public:
	GMDx11FramebufferTexture(const IRenderContext* context, const GMFramebufferDesc& desc);

public:
	virtual void init() override;

public:
	ID3D11Resource* getTexture();
};

GMDx11FramebufferTexture::GMDx11FramebufferTexture(const IRenderContext* context, const GMFramebufferDesc& desc)
	: GMDx11Texture(context, nullptr)
{
	D(d);
	d->desc = desc;
}

void GMDx11FramebufferTexture::init()
{
	D(d);
	D_BASE(db, Base);
	DXGI_FORMAT format;
	if (d->desc.framebufferFormat == GMFramebufferFormat::R8G8B8A8_UNORM)
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else if (d->desc.framebufferFormat == GMFramebufferFormat::R32G32B32A32_FLOAT)
	{
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	else
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		GM_ASSERT(!"Unsupported format.");
		gm_error(gm_dbg_wrap("Unsupported format."));
	}

	const GMWindowStates& windowStates = db->context->getWindow()->getWindowStates();
	GMComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	texDesc.Width = d->desc.rect.width;
	texDesc.Height = d->desc.rect.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = windowStates.sampleCount;
	texDesc.SampleDesc.Quality = windowStates.sampleQuality;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	GM_DX_HR(db->device->CreateTexture2D(&texDesc, NULL, &texture));
	db->resource = texture;

	GM_ASSERT(db->resource);

	GM_DX_HR(db->device->CreateShaderResourceView(
		db->resource,
		NULL,
		&db->shaderResourceView
	));

	if (!db->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::GMGetDx11DefaultSamplerDesc();
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		GM_DX_HR(db->device->CreateSamplerState(&desc, &db->samplerState));
	}
}

ID3D11Resource* GMDx11FramebufferTexture::getTexture()
{
	D_BASE(db, Base);
	return db->resource;
}
END_NS

GMDx11Framebuffer::GMDx11Framebuffer(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

GMDx11Framebuffer::~GMDx11Framebuffer()
{
	D(d);
	GM_delete(d->renderTexture);
}

bool GMDx11Framebuffer::init(const GMFramebufferDesc& desc)
{
	D(d);
	GMComPtr<ID3D11Device> device;
	d->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	GM_ASSERT(device);
	GMComPtr<ID3D11Texture2D> depthStencilBuffer;

	GM_ASSERT(!d->renderTexture);
	GMDx11FramebufferTexture* renderTexture = new GMDx11FramebufferTexture(d->context, desc);
	d->renderTexture = renderTexture;
	d->renderTexture->init();

	if (!d->name.isEmpty())
		GM_DX11_SET_OBJECT_NAME_A(renderTexture->getTexture(), d->name.toStdString().c_str());
	GM_DX_HR_RET(device->CreateRenderTargetView(renderTexture->getTexture(), NULL, &d->renderTargetView));

	return true;
}

void GMDx11Framebuffer::setName(const GMString& name)
{
	D(d);
	d->name = name;
	GM_ASSERT(!d->renderTexture && "Please call setName before init.");
}

ITexture* GMDx11Framebuffer::getTexture()
{
	D(d);
	GM_ASSERT(d->renderTexture);
	return d->renderTexture;
}

const IRenderContext* GMDx11Framebuffer::getContext()
{
	D(d);
	return d->context;
}

GMDx11Framebuffers::GMDx11Framebuffers(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
	GM_ASSERT(d->deviceContext);
	d->engine = gm_cast<GMGraphicEngine*>(d->context->getEngine());
}

bool GMDx11Framebuffers::init(const GMFramebuffersDesc& desc)
{
	D(d);
	d->clearColor[0] = desc.clearColor[0];
	d->clearColor[1] = desc.clearColor[1];
	d->clearColor[2] = desc.clearColor[2];
	d->clearColor[3] = desc.clearColor[3];

	GMComPtr<ID3D11Device> device;
	d->engine->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	GM_ASSERT(device);

	const GMWindowStates& windowStates = d->context->getWindow()->getWindowStates();
	d->viewport.TopLeftX = windowStates.viewportTopLeftX;
	d->viewport.TopLeftY = windowStates.viewportTopLeftY;
	d->viewport.Width = desc.rect.width;
	d->viewport.Height = desc.rect.height;
	d->viewport.MinDepth = windowStates.minDepth;
	d->viewport.MaxDepth = windowStates.maxDepth;

	// 创建深度缓存模板
	D3D11_TEXTURE2D_DESC depthStencilTextureDesc = getDepthTextureDesc();
	depthStencilTextureDesc.Width = desc.rect.width;
	depthStencilTextureDesc.Height = desc.rect.height;
	depthStencilTextureDesc.SampleDesc.Count = windowStates.sampleCount;
	depthStencilTextureDesc.SampleDesc.Quality = windowStates.sampleQuality;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};
	if (windowStates.sampleCount > 1)
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	else
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	GMComPtr<ID3D11DepthStencilState> depthStencilState;
	GM_DX_HR_RET(device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));
	GM_DX_HR_RET(device->CreateTexture2D(&depthStencilTextureDesc, NULL, &d->depthStencilTexture));
	GM_DX_HR_RET(device->CreateDepthStencilView(d->depthStencilTexture, &dsvd, &d->depthStencilView));
	return true;
}

void GMDx11Framebuffers::addFramebuffer(AUTORELEASE IFramebuffer* framebuffer)
{
	D(d);
	GM_ASSERT(dynamic_cast<GMDx11Framebuffer*>(framebuffer));
	GMDx11Framebuffer* dxFramebuffer = static_cast<GMDx11Framebuffer*>(framebuffer);
	d->framebuffers.push_back(GMOwnedPtr<GMDx11Framebuffer>(dxFramebuffer));
	d->renderTargetViews.push_back(dxFramebuffer->getRenderTargetView());
}

void GMDx11Framebuffers::use()
{
	D(d);
	d->deviceContext->RSSetViewports(1, &d->viewport);
	GM_ASSERT(d->renderTargetViews.size() < std::numeric_limits<UINT>::max());
	d->deviceContext->OMSetRenderTargets((UINT)d->renderTargetViews.size(), d->renderTargetViews.data(), d->depthStencilView);
}

void GMDx11Framebuffers::bind()
{
	D(d);
	use();
	d->engine->getFramebuffersStack().push(this);
}

void GMDx11Framebuffers::unbind()
{
	D(d);
	GMFramebuffersStack& stack = d->engine->getFramebuffersStack();
	IFramebuffers* currentFramebuffers = stack.pop();
	if (currentFramebuffers != this)
	{
		GM_ASSERT(false);
		gm_error(gm_dbg_wrap("Cannot unbind framebuffer because current framebuffer isn't this framebuffer."));
	}
	else
	{
		IFramebuffers* lastFramebuffers = stack.peek();
		if (lastFramebuffers)
			lastFramebuffers->use();
		else
			d->context->getEngine()->getDefaultFramebuffers()->use();
	}
}

void GMDx11Framebuffers::clear(GMFramebuffersClearType type)
{
	D(d);
	UINT clearFlag = 0;
	GMuint iType = (GMuint)type;
	if (iType & (GMuint)GMFramebuffersClearType::Depth)
		clearFlag |= D3D11_CLEAR_DEPTH;
	if (iType & (GMuint)GMFramebuffersClearType::Stencil)
		clearFlag |= D3D11_CLEAR_STENCIL;

	if (iType & (GMuint)GMFramebuffersClearType::Color)
	{
		for (auto renderTargetView : d->renderTargetViews)
		{
			d->deviceContext->ClearRenderTargetView(renderTargetView, d->clearColor);
		}
	}
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);
}

IFramebuffer* GMDx11Framebuffers::getFramebuffer(GMsize_t index)
{
	D(d);
	GM_ASSERT(index < d->framebuffers.size());
	return d->framebuffers[index].get();
}

GMsize_t GMDx11Framebuffers::count()
{
	D(d);
	return d->framebuffers.size();
}

void GMDx11Framebuffers::copyDepthStencilFramebuffer(IFramebuffers* dest)
{
	GMDx11Framebuffers* destFramebuffers = gm_cast<GMDx11Framebuffers*>(dest);
	D(d);
	D_OF(d_dest, destFramebuffers);
	d->deviceContext->CopyResource(d_dest->depthStencilTexture, d->depthStencilTexture);
}

const IRenderContext* GMDx11Framebuffers::getContext()
{
	D(d);
	return d->context;
}

D3D11_TEXTURE2D_DESC GMDx11Framebuffers::getDepthTextureDesc()
{
	D3D11_TEXTURE2D_DESC depthStencilTextureDesc = { 0 };
	depthStencilTextureDesc.MipLevels = 1;
	depthStencilTextureDesc.ArraySize = 1;
	depthStencilTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilTextureDesc.CPUAccessFlags = 0;
	depthStencilTextureDesc.MiscFlags = 0;
	return depthStencilTextureDesc;
}

IFramebuffers* GMDx11Framebuffers::createDefaultFramebuffers(const IRenderContext* context)
{
	return new GMDx11DefaultFramebuffers(context);
}

bool GMDx11ShadowFramebuffers::init(const GMFramebuffersDesc& desc)
{
	bool b = Base::init(desc);
	if (!b)
		return false;

	D(d);
	D_BASE(db, Base);
	const GMWindowStates& windowStates = db->context->getWindow()->getWindowStates();
	d->width = desc.rect.width;
	d->height = desc.rect.height;

	GMComPtr<ID3D11Device> device;
	db->engine->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	GM_ASSERT(device);

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =
	{
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D_SRV_DIMENSION_UNKNOWN,
		0,
		0
	};
	dsrvd.Texture2D.MipLevels = 1;
	dsrvd.ViewDimension = windowStates.sampleCount > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	GM_DX_HR_RET(device->CreateShaderResourceView(db->depthStencilTexture, &dsrvd, &d->depthShaderResourceView));
	GM_DX11_SET_OBJECT_NAME_A(db->depthStencilTexture, "GM_ShadowMap");
	GM_DX11_SET_OBJECT_NAME_A(d->depthShaderResourceView, "GM_ShadowMap_SRV");
	return true;
}

ID3D11ShaderResourceView* GMDx11ShadowFramebuffers::getShadowMapShaderResourceView()
{
	D(d);
	return d->depthShaderResourceView;
}