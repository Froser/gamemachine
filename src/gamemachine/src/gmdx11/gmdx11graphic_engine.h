#ifndef __GMDX11GRAPHIC_ENGINE_H__
#define __GMDX11GRAPHIC_ENGINE_H__
#include <gmcommon.h>
#include <gmcom.h>
#include <gmdxincludes.h>
#include <gmmodel.h>
#include <gmtools.h>
#include <gmgraphicengine.h>
BEGIN_NS

struct GMMVPMatrix;
struct GMDx11GlobalBlendStateDesc
{
	bool enabled = false;
	GMS_BlendFunc source;
	GMS_BlendFunc dest;
	GMint blendRefCount = 0;
};

class GMDx11Framebuffers;
class GMDx11GBuffer;
GM_PRIVATE_OBJECT(GMDx11GraphicEngine)
{
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<IDXGISwapChain> swapChain;
	GMComPtr<ID3D11DepthStencilView> depthStencilView;
	GMComPtr<ID3D11Texture2D> depthStencilTexture;
	GMComPtr<ID3D11RenderTargetView> renderTargetView;
	GMScopePtr<IShaderProgram> shaderProgram;
	
	GMDx11GlobalBlendStateDesc blendState;
	bool ready = false;
	bool lightDirty = true;

	IRenderer* renderer_3d;
	IRenderer* renderer_2d;
	IRenderer* renderer_text;
	IRenderer* renderer_cubemap;
	IRenderer* renderer_filter;
	IRenderer* renderer_deferred_3d;
	IRenderer* renderer_deferred_3d_lightpass;
	IRenderer* renderer_3d_shadow;
};

class GMDx11GraphicEngine : public GMGraphicEngine
{
	DECLARE_PRIVATE_AND_BASE(GMDx11GraphicEngine, GMGraphicEngine)

public:
	GMDx11GraphicEngine(const GMContext* context);

public:
	virtual void init() override;
	virtual void update(GMUpdateDataType type) override;
	virtual void clearStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::DefaultShaderProgram) override;
	virtual bool event(const GMMessage& e) override;
	virtual IFramebuffers* getDefaultFramebuffers() override;
	virtual IRenderer* getRenderer(GMModelType objectType) override;
	virtual GMGlyphManager* getGlyphManager() override;

public:
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual bool getInterface(GameMachineInterfaceID, void**);
	virtual void createShadowFramebuffers(OUT IFramebuffers** framebuffers) override;

public:
	virtual void activateLights(IRenderer* renderer);

public:
	inline ID3D11Device* getDevice()
	{
		D(d);
		return d->device;
	}

	inline ID3D11DeviceContext* getDeviceContext()
	{
		D(d);
		return d->deviceContext;
	}

	inline IDXGISwapChain* getSwapChain()
	{
		D(d);
		return d->swapChain;
	}

	inline ID3D11DepthStencilView* getDepthStencilView()
	{
		D(d);
		return d->depthStencilView;
	}

	inline ID3D11RenderTargetView* getRenderTargetView()
	{
		D(d);
		return d->renderTargetView;
	}

	inline const GMDx11GlobalBlendStateDesc& getGlobalBlendState()
	{
		D(d);
		return d->blendState;
	}

	const GMVec2 getCurrentFilterKernelDelta()
	{
		D_BASE(d, Base);
		return d->renderConfig.get(GMRenderConfigs::FilterKernelOffset_Vec2).toVec2();
	}

private:
	void initShaders(const GMContext* context);
};

END_NS
#endif