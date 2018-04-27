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
};

class GMDx11GraphicEngine : public GMGraphicEngine
{
	DECLARE_PRIVATE_AND_BASE(GMDx11GraphicEngine, GMGraphicEngine)

public:
	virtual void init() override;
	virtual void newFrame() override;
	virtual void update(GMUpdateDataType type) override;
	virtual void clearStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::DefaultShaderProgram) override;
	virtual bool event(const GameMachineMessage& e) override;
	virtual IFramebuffers* getDefaultFramebuffers() override;

public:
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual bool getInterface(GameMachineInterfaceID, void**);

public:
	virtual void activateLight(ID3DX11Effect* effect);

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

	inline const Vector<GMLight>& getLights()
	{
		D_BASE(d, Base);
		return d->lights;
	}

	const GMVec2 getCurrentFilterKernelDelta()
	{
		D_BASE(d, Base);
		return d->renderConfig.get(GMRenderConfigs::FilterKernelOffset_Vec2).toVec2();
	}

public:
	IRenderer* getRenderer(GMModelType objectType);

private:
	void initShaders();
};

END_NS
#endif