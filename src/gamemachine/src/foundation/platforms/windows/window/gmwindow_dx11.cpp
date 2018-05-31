#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include <gmdx11helper.h>
#include "foundation/gamemachine.h"

namespace
{
	const GMwchar* g_classname = L"gamemachine_MainWindow_dx11_class";
	const DXGI_FORMAT g_bufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
}

GM_PRIVATE_OBJECT(GMWindow_Dx11)
{
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<IDXGISwapChain> swapChain;
	GMComPtr<ID3D11DepthStencilView> depthStencilView;
	GMComPtr<ID3D11Texture2D> depthStencilTexture;
	GMComPtr<ID3D11RenderTargetView> renderTargetView;
	D3D_FEATURE_LEVEL d3dFeatureLevel;

	bool vsync = true; //默认开启垂直同步
	DXGI_MODE_DESC* modes = nullptr;
};

class GMWindow_Dx11 : public GMWindow
{
	DECLARE_PRIVATE_AND_BASE(GMWindow_Dx11, GMWindow)

public:
	GMWindow_Dx11() = default;
	~GMWindow_Dx11();

public:
	virtual void msgProc(const GMMessage& message) override;
	virtual bool getInterface(GameMachineInterfaceID id, void** out) override;
	virtual void onWindowCreated(const GMWindowAttributes& attrs, GMWindowHandle handle) override;
};

GMWindow_Dx11::~GMWindow_Dx11()
{
	D(d);
	GM_delete(d->modes);
}

void GMWindow_Dx11::msgProc(const GMMessage& message)
{
	D(d);
	Base::msgProc(message);
	if (message.msgType == GameMachineMessageType::FrameUpdate)
		GM_DX_HR(d->swapChain->Present(d->vsync ? 1 : 0, 0));
}

bool GMWindow_Dx11::getInterface(GameMachineInterfaceID id, void** out)
{
	D(d);
	switch (id)
	{
	case GameMachineInterfaceID::D3D11Device:
		d->device->AddRef();
		(*out) = d->device.get();
		break;
	case GameMachineInterfaceID::D3D11DeviceContext:
		d->deviceContext->AddRef();
		(*out) = d->deviceContext.get();
		break;
	case GameMachineInterfaceID::D3D11SwapChain:
		d->swapChain->AddRef();
		(*out) = d->swapChain.get();
		break;
	case GameMachineInterfaceID::D3D11DepthStencilView:
		d->depthStencilView->AddRef();
		(*out) = d->depthStencilView.get();
		break;
	case GameMachineInterfaceID::D3D11DepthStencilTexture:
		d->depthStencilTexture->AddRef();
		(*out) = d->depthStencilTexture.get();
		break;
	case GameMachineInterfaceID::D3D11RenderTargetView:
		d->renderTargetView->AddRef();
		(*out) = d->renderTargetView.get();
		break;
	default:
		return false;
	}
	return true;
}

void GMWindow_Dx11::onWindowCreated(const GMWindowAttributes& wndAttrs, GMWindowHandle handle)
{
	D(d);
	GM_ASSERT(handle);

	GMGameMachineRunningStates gameMachineRunningState = GM.getGameMachineRunningStates();
	UINT createDeviceFlags = 0;
	DXGI_SWAP_CHAIN_DESC sc = { 0 };
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	D3D11_VIEWPORT vp = { 0 };
	DXGI_ADAPTER_DESC adapterDesc = { 0 };
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	GMMessage msg;

	// COM objs
	GMComPtr<IDXGIFactory> dxgiFactory;
	GMComPtr<IDXGIAdapter> dxgiAdapter;
	GMComPtr<IDXGIOutput> dxgiAdapterOutput;

	GMComPtr<IDXGIDevice> dxgiDevice;
	GMComPtr<ID3D11Texture2D> backBuffer;
	GMComPtr<ID3D11DepthStencilState> depthStencilState;
	GMComPtr<ID3D11RasterizerState> rasterizerState;

	UINT renderWidth = wndAttrs.rc.right - wndAttrs.rc.left;
	UINT renderHeight = wndAttrs.rc.bottom - wndAttrs.rc.top;
	GMuint numerator = 0, denominator = 0;

	// 1.枚举设备属性
	GM_DX_HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory));
	GM_DX_HR(dxgiFactory->EnumAdapters(0, &dxgiAdapter));
	GM_DX_HR(dxgiAdapter->EnumOutputs(0, &dxgiAdapterOutput));
	GMuint numModes;
	GM_DX_HR(dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL));
	d->modes = new DXGI_MODE_DESC[numModes];
	GM_DX_HR(dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, d->modes));
	for (GMuint i = 0; i < numModes; i++)
	{
		if (d->modes[i].Width == (GMuint)renderWidth)
		{
			if (d->modes[i].Height == (GMuint)renderHeight)
			{
				numerator = d->modes[i].RefreshRate.Numerator;
				denominator = d->modes[i].RefreshRate.Denominator;
			}
		}
	}
	GM_DX_HR(dxgiAdapter->GetDesc(&adapterDesc));

	gameMachineRunningState.workingAdapterDesc = adapterDesc.Description;
	gameMachineRunningState.vsyncEnabled = d->vsync;

	// 2.创建交换链、设备和上下文
	sc.BufferDesc.Width = renderWidth;
	sc.BufferDesc.Height = renderHeight;
	sc.BufferDesc.Format = g_bufferFormat;
	sc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sc.BufferCount = 1;
	sc.OutputWindow = getWindowHandle();
	sc.Windowed = true;
	sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sc.Flags = 0;
	if (d->vsync)
	{
		sc.BufferDesc.RefreshRate.Numerator = numerator;
		sc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		sc.BufferDesc.RefreshRate.Numerator = 0;
		sc.BufferDesc.RefreshRate.Denominator = 1;
	}

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
	};

	UINT createFlags = 0;
#if _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	GM_DX_HR(D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createFlags,
		featureLevels,
		GM_array_size(featureLevels),
		D3D11_SDK_VERSION,
		&d->device,
		NULL,
		&d->deviceContext)
	);

	UINT msaaQuality = 0;
	GM_DX_HR(d->device->CheckMultisampleQualityLevels(
		g_bufferFormat,
		wndAttrs.samples,
		&msaaQuality
	));

	if (wndAttrs.samples <= 1)
	{
		// 禁用多重采样
		gameMachineRunningState.sampleCount = sc.SampleDesc.Count = 1;
		gameMachineRunningState.sampleQuality = sc.SampleDesc.Quality = 0;
	}
	else
	{
		if (!msaaQuality)
		{
			// 不支持指定MSAA质量
			gameMachineRunningState.sampleCount = sc.SampleDesc.Count = 4;
			gameMachineRunningState.sampleQuality = sc.SampleDesc.Quality = msaaQuality - 1;
		}
		else
		{
			gameMachineRunningState.sampleCount = sc.SampleDesc.Count = wndAttrs.samples;
			gameMachineRunningState.sampleQuality = sc.SampleDesc.Quality = msaaQuality - 1;
		}
	}


	GM_DX_HR(dxgiFactory->CreateSwapChain(
		d->device,
		&sc,
		&d->swapChain
	));

	// 3.创建目标视图
	GM_DX_HR(d->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

	GM_DX_HR(d->device->CreateRenderTargetView(backBuffer, NULL, &d->renderTargetView));
	GM_DX11_SET_OBJECT_NAME_A(d->renderTargetView, "GM_DefaultRenderTargetView");

	// 4.创建深度模板缓存
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = renderWidth;
	depthTextureDesc.Height = renderHeight;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.SampleDesc.Count = sc.SampleDesc.Count;
	depthTextureDesc.SampleDesc.Quality = sc.SampleDesc.Quality;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.CPUAccessFlags = 0;
	depthTextureDesc.MiscFlags = 0;

	GM_DX_HR(d->device->CreateTexture2D(&depthTextureDesc, NULL, &d->depthStencilTexture));
	GM_DX11_SET_OBJECT_NAME_A(d->depthStencilTexture, "GM_DefaultDepthStencilTexture");

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
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

	GM_DX_HR(d->device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));
	d->deviceContext->OMSetDepthStencilState(depthStencilState, 1);

	GM_DX_HR(d->device->CreateDepthStencilView(d->depthStencilTexture, NULL, &d->depthStencilView));

	// 5.绑定渲染目标
	d->deviceContext->OMSetRenderTargets(1, &d->renderTargetView, d->depthStencilView);

	// 6.设置视口
	gameMachineRunningState.viewportTopLeftX = vp.TopLeftX = 0.f;
	gameMachineRunningState.viewportTopLeftY = vp.TopLeftY = 0.f;
	vp.Width = static_cast<GMfloat>(renderWidth);
	vp.Height = static_cast<GMfloat>(renderHeight);
	gameMachineRunningState.minDepth = vp.MinDepth = 0.f;
	gameMachineRunningState.maxDepth = vp.MaxDepth = 1.f;
	d->deviceContext->RSSetViewports(1, &vp);

	// 发送事件，更新状态
	GM.setGameMachineRunningStates(gameMachineRunningState);

	msg.msgType = GameMachineMessageType::Dx11Ready;
	msg.objPtr = static_cast<IQueriable*>(this);
	GM.postMessage(msg);
}

bool GMWindowFactory::createWindowWithDx11(GMInstance instance, OUT IWindow** window)
{
	(*window) = new GMWindow_Dx11();
	if (*window)
		return true;
	return false;
}
