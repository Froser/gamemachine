#ifndef __GMUIDX11WINDOW_H__
#define __GMUIDX11WINDOW_H__
#include "gmuidef.h"
#include "gmuiwindow.h"
#include <d3d11.h>
#include "gmuiglwindow.h"

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUIDx11Window)
{
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	IDXGISwapChain* swapChain = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;
	ID3D11RenderTargetView* renderTargetView = nullptr;
	D3D_FEATURE_LEVEL d3dFeatureLevel;
};

class GMUIDx11Window : public GMUIGameMachineWindowBase
{
	DECLARE_PRIVATE_AND_BASE(GMUIDx11Window, GMUIGameMachineWindowBase)

public:
	GMUIDx11Window();

public:
	gm::GMWindowHandle create(const gm::GMWindowAttributes& wndAttrs);

private:
	void initD3D(const gm::GMWindowAttributes& wndAttrs);
};

END_UI_NS
#endif