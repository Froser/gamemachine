#include "stdafx.h"
#include "gmuidx11window.h"

#define EXIT __exit
#define CHECK_HR(hr) if(FAILED(hr)) { GM_ASSERT(false); goto EXIT; }

namespace
{
	const gm::GMwchar* g_classname = L"gamemachine_MainWindow_dx11_class";
	const DXGI_FORMAT g_bufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
}

GMUIDx11Window::GMUIDx11Window()
{

}

GMUIDx11Window::~GMUIDx11Window()
{
	D(d);
	gm::GM_delete(d->modes);
}

gm::GMWindowHandle GMUIDx11Window::create(const gm::GMWindowAttributes& wndAttrs)
{
	GMUIGameMachineWindowBase::createWindow(wndAttrs, g_classname);
	initD3D(wndAttrs);
	return getWindowHandle();
}

void GMUIDx11Window::update()
{
	D(d);
	HRESULT hr = d->swapChain->Present(d->vsync ? 1 : 0, 0);
	GM_COM_CHECK(hr);
	Base::update();
}

bool GMUIDx11Window::getInterface(gm::GameMachineInterfaceID id, void** out)
{
	D(d);
	switch (id)
	{
	case gm::GameMachineInterfaceID::D3D11Device:
		d->device->AddRef();
		(*out) = d->device.get();
		break;
	case gm::GameMachineInterfaceID::D3D11DeviceContext:
		d->deviceContext->AddRef();
		(*out) = d->deviceContext.get();
		break;
	case gm::GameMachineInterfaceID::DXGISwapChain:
		d->swapChain->AddRef();
		(*out) = d->swapChain.get();
		break;
	case gm::GameMachineInterfaceID::D3D11DepthStencilView:
		d->depthStencilView->AddRef();
		(*out) = d->depthStencilView.get();
		break;
	case gm::GameMachineInterfaceID::D3D11RenderTargetView:
		d->renderTargetView->AddRef();
		(*out) = d->renderTargetView.get();
		break;
	case gm::GameMachineInterfaceID::D3D11RasterState:
		d->rasterState->AddRef();
		(*out) = d->rasterState.get();
		break;
	default:
		return false;
	}
	return true;
}

void GMUIDx11Window::initD3D(const gm::GMWindowAttributes& wndAttrs)
{
	D(d);
	gm::GMGameMachineRunningStates gameMachineRunningState = GM.getGameMachineRunningStates();
	UINT createDeviceFlags = 0;
	DXGI_SWAP_CHAIN_DESC sc = { 0 };
	D3D11_TEXTURE2D_DESC depthTextureDesc;
	D3D11_VIEWPORT vp = { 0 };
	DXGI_ADAPTER_DESC adapterDesc = { 0 };
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	HRESULT hr;
	gm::GameMachineMessage msg;

	// COM objs
	gm::GMComPtr<IDXGIFactory> dxgiFactory;
	gm::GMComPtr<IDXGIAdapter> dxgiAdapter;
	gm::GMComPtr<IDXGIOutput> dxgiAdapterOutput;

	gm::GMComPtr<IDXGIDevice> dxgiDevice;
	gm::GMComPtr<ID3D11Texture2D> backBuffer;
	gm::GMComPtr<ID3D11Texture2D> depthStencilBuffer;
	gm::GMComPtr<ID3D11DepthStencilState> depthStencilState;

	UINT renderWidth = wndAttrs.rc.right - wndAttrs.rc.left;
	UINT renderHeight = wndAttrs.rc.bottom - wndAttrs.rc.top;
	gm::GMuint numerator = 0, denominator = 0;

#if defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 1.枚举设备属性
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	CHECK_HR(hr);
	hr = dxgiFactory->EnumAdapters(0, &dxgiAdapter);
	CHECK_HR(hr);
	hr = dxgiAdapter->EnumOutputs(0, &dxgiAdapterOutput);
	CHECK_HR(hr);
	gm::GMuint numModes;
	hr = dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	CHECK_HR(hr);
	d->modes = new DXGI_MODE_DESC[numModes];
	hr = dxgiAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, d->modes);
	CHECK_HR(hr);
	for (gm::GMuint i = 0; i < numModes; i++)
	{
		if (d->modes[i].Width == (gm::GMuint)renderWidth)
		{
			if (d->modes[i].Height == (gm::GMuint)renderHeight)
			{
				numerator = d->modes[i].RefreshRate.Numerator;
				denominator = d->modes[i].RefreshRate.Denominator;
			}
		}
	}
	hr = dxgiAdapter->GetDesc(&adapterDesc);
	CHECK_HR(hr);

	gameMachineRunningState.workingAdapterDesc = adapterDesc.Description;
	gameMachineRunningState.vsyncEnabled = d->vsync;

	// 2.创建交换链、设备和上下文

	// UINT msaaQuality = 0;
	// hr = d->device->CheckMultisampleQualityLevels(
	// 	g_bufferFormat,
	// 	wndAttrs.samples,
	// 	&msaaQuality
	// 	);
	// CHECK_HR(hr);

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

	//if (wndAttrs.samples == 0)
	if (true)
	{
		// 禁用多重采样
		gameMachineRunningState.sampleCount = sc.SampleDesc.Count = 1;
		gameMachineRunningState.sampleQuality = sc.SampleDesc.Quality = 0;
	}
	//else
	//{
	//	if (!msaaQuality)
	//	{
	//		// 不支持指定MSAA质量
	//		sc.SampleDesc.Count = 4;
	//		sc.SampleDesc.Quality = msaaQuality - 1;
	//	}
	//	else
	//	{
	//		sc.SampleDesc.Count = wndAttrs.samples;
	//		sc.SampleDesc.Quality = msaaQuality - 1;
	//	}
	//}

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&sc,
		&d->swapChain,
		&d->device,
		NULL,
		&d->deviceContext);
	CHECK_HR(hr);

	// 3.创建目标视图
	hr = d->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	CHECK_HR(hr);

	hr = d->device->CreateRenderTargetView(backBuffer, NULL, &d->renderTargetView);
	CHECK_HR(hr);

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

	hr = d->device->CreateTexture2D(&depthTextureDesc, NULL, &depthStencilBuffer);
	CHECK_HR(hr);

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = d->device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	CHECK_HR(hr);
	d->deviceContext->OMSetDepthStencilState(depthStencilState, 1);

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = d->device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &d->depthStencilView);
	CHECK_HR(hr);

	// 6.绑定渲染目标
	d->deviceContext->OMSetRenderTargets(1, &d->renderTargetView, d->depthStencilView);

	// 7.设置光栅状态
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	hr = d->device->CreateRasterizerState(&rasterDesc, &d->rasterState);
	CHECK_HR(hr);
	d->deviceContext->RSSetState(d->rasterState);

	// 8.设置视口
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	vp.Width = static_cast<gm::GMfloat>(renderWidth);
	vp.Height = static_cast<gm::GMfloat>(renderHeight);
	gameMachineRunningState.minDepth = vp.MinDepth = 0.f;
	gameMachineRunningState.maxDepth = vp.MaxDepth = 1.f;
	d->deviceContext->RSSetViewports(1, &vp);

	// 发送事件，更新状态
	GM.setGameMachineRunningStates(gameMachineRunningState);

	msg.msgType = gm::GameMachineMessageType::Dx11Ready;
	msg.objPtr = static_cast<IQueriable*>(this);
	GM.postMessage(msg);

EXIT:
	return;
}