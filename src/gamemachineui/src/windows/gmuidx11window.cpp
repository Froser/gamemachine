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

gm::GMWindowHandle GMUIDx11Window::create(const gm::GMWindowAttributes& wndAttrs)
{
	GMUIGameMachineWindowBase::createWindow(wndAttrs, g_classname);
	initD3D(wndAttrs);
	return getWindowHandle();
}

void GMUIDx11Window::update()
{
	D(d);
	HRESULT hr = d->swapChain->Present(TRUE, 0);
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
	default:
		return false;
	}
	return true;
}

void GMUIDx11Window::initD3D(const gm::GMWindowAttributes& wndAttrs)
{
	D(d);
	UINT createDeviceFlags = 0;
	DXGI_SWAP_CHAIN_DESC sc = { 0 };
	D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
	D3D11_VIEWPORT vp = { 0 };
	HRESULT hr;
	gm::GameMachineMessage msg;

	// COM objs
	gm::GMComPtr<IDXGIDevice> dxgiDevice;
	gm::GMComPtr<IDXGIAdapter> dxgiAdapter;
	gm::GMComPtr<IDXGIFactory> dxgiFactory;
	gm::GMComPtr<ID3D11Texture2D> backBuffer;
	gm::GMComPtr<ID3D11Texture2D> depthStencilBuffer;

	UINT renderWidth = wndAttrs.rc.right - wndAttrs.rc.left;
	UINT renderHeight = wndAttrs.rc.bottom - wndAttrs.rc.top;
#if defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 创建设备
	hr = D3D11CreateDevice(
		NULL,						// 默认显示适配器
		D3D_DRIVER_TYPE_HARDWARE,
		0,							// 不使用软件设备
		createDeviceFlags,
		NULL, 0,					// 默认的特征等级数组
		D3D11_SDK_VERSION,
		&d->device,
		&d->d3dFeatureLevel,
		&d->deviceContext);
	CHECK_HR(hr);

	// 创建交换链
	UINT msaaQuality = 0;
	hr = d->device->CheckMultisampleQualityLevels(
		g_bufferFormat,
		wndAttrs.samples,
		&msaaQuality
		);
	CHECK_HR(hr);

	sc.BufferDesc.Width = wndAttrs.rc.right - wndAttrs.rc.left;
	sc.BufferDesc.Height = wndAttrs.rc.bottom - wndAttrs.rc.top;
	sc.BufferDesc.RefreshRate.Numerator = 60;
	sc.BufferDesc.RefreshRate.Denominator = 1;
	sc.BufferDesc.Format = g_bufferFormat;
	sc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	if (wndAttrs.samples == 0)
	{
		// 禁用多重采样
		sc.SampleDesc.Count = 1;
		sc.SampleDesc.Quality = 0;
	}
	else
	{
		if (!msaaQuality)
		{
			// 不支持指定MSAA质量
			sc.SampleDesc.Count = 4;
			sc.SampleDesc.Quality = msaaQuality - 1;
		}
		else
		{
			sc.SampleDesc.Count = wndAttrs.samples;
			sc.SampleDesc.Quality = msaaQuality - 1;
		}
	}
	sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sc.BufferCount = 1;
	sc.OutputWindow = getWindowHandle();
	sc.Windowed = true;
	sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sc.Flags = 0;

	hr = d->device->QueryInterface(__uuidof(IDXGIDevice), (void**)(&dxgiDevice));
	CHECK_HR(hr);

	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)(&dxgiAdapter));
	CHECK_HR(hr);

	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)(&dxgiFactory));
	CHECK_HR(hr);

	hr = dxgiFactory->CreateSwapChain(d->device, &sc, &d->swapChain);
	CHECK_HR(hr);

	// 创建目标视图
	hr = d->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	CHECK_HR(hr);

	hr = d->device->CreateRenderTargetView(backBuffer, NULL, &d->renderTargetView);
	CHECK_HR(hr);

	// 创建深度模板缓存
	depthStencilDesc.Width = renderWidth;
	depthStencilDesc.Height = renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = sc.SampleDesc.Count;
	depthStencilDesc.SampleDesc.Quality = sc.SampleDesc.Quality;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = d->device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	CHECK_HR(hr);

	hr = d->device->CreateDepthStencilView(depthStencilBuffer, NULL, &d->depthStencilView);
	CHECK_HR(hr);

	// 将视图绑定到输出合并器阶段
	d->deviceContext->OMSetRenderTargets(1, &d->renderTargetView, d->depthStencilView);

	// 设置视口
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	vp.Width = static_cast<float>(renderWidth);
	vp.Height = static_cast<float>(renderHeight);
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	d->deviceContext->RSSetViewports(1, &vp);

	// 发送事件
	msg.msgType = gm::GameMachineMessageType::Dx11Ready;
	msg.objPtr = static_cast<IQueriable*>(this);
	GM.postMessage(msg);

EXIT:
	return;
}