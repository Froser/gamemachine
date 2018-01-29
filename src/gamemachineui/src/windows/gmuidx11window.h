#ifndef __GMUIDX11WINDOW_H__
#define __GMUIDX11WINDOW_H__
#include "gmuidef.h"
#include "gmuiwindow.h"
#include <gmdxincludes.h>
#include "gmuiglwindow.h"
#include <gmcom.h>

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUIDx11Window)
{
	gm::GMComPtr<ID3D11Device> device;
	gm::GMComPtr<ID3D11DeviceContext> deviceContext;
	gm::GMComPtr<IDXGISwapChain> swapChain;
	gm::GMComPtr<ID3D11DepthStencilView> depthStencilView;
	gm::GMComPtr<ID3D11RenderTargetView> renderTargetView;
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