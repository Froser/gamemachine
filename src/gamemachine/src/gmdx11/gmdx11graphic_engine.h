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
GM_PRIVATE_OBJECT(GMDx11GraphicEngine)
{
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<IDXGISwapChain> swapChain;
	GMComPtr<ID3D11DepthStencilView> depthStencilView;
	GMComPtr<ID3D11RenderTargetView> renderTargetView;
	GMScopePtr<IShaderProgram> shaderProgram;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	
	GMDx11GlobalBlendStateDesc blendState;
	GMStencilOptions stencilOptions;
	bool ready = false;
	Vector<GMLight> lights;
	bool needActivateLight = false;
	GMRenderConfig renderConfig;
};

class GMDx11GraphicEngine : public GMGraphicEngine
{
	DECLARE_PRIVATE(GMDx11GraphicEngine)

public:
	virtual void init() override;
	virtual void newFrame() override;
	virtual void drawObjects(GMGameObject *objects[], GMuint count, GMBufferMode bufferMode) override;
	virtual void update(GMUpdateDataType type) override;
	virtual void addLight(const GMLight& light) override;
	virtual void removeLights() override;
	virtual void clearStencil() override;
	virtual void beginBlend(GMS_BlendFunc sfactor, GMS_BlendFunc dfactor) override;
	virtual void endBlend() override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::CurrentShaderProgram) override;
	virtual bool event(const GameMachineMessage& e) override;
	virtual void setShaderLoadCallback(IShaderLoadCallback* cb) override
	{
		D(d);
		d->shaderLoadCallback = cb;
	}

	virtual void setStencilOptions(const GMStencilOptions& options) override
	{
		D(d);
		d->stencilOptions = options;
	}

	virtual const GMStencilOptions& getStencilOptions() override
	{
		D(d);
		return d->stencilOptions;
	}

public:
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual bool getInterface(GameMachineInterfaceID, void**);

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
		D(d);
		return d->lights;
	}

	inline bool needActivateLight()
	{
		D(d);
		return d->needActivateLight;
	}

	const GMFilterMode::Mode getCurrentFilterMode()
	{
		D(d);
		return d->renderConfig.get(GMRenderConfigs::FilterMode).toEnum<GMFilterMode::Mode>();
	}

	const GMVec2 getCurrentFilterKernelDelta()
	{
		D(d);
		return d->renderConfig.get(GMRenderConfigs::FilterKernelOffset_Vec2).toVec2();
	}

public:
	IRenderer* getRenderer(GMModelType objectType);

private:
	void initShaders();
	void forwardDraw(GMGameObject *objects[], GMuint count, GMFilterMode::Mode filter);
	void directDraw(GMGameObject *objects[], GMuint count, GMFilterMode::Mode filter);
	void forwardRender(GMGameObject *objects[], GMuint count);
};

END_NS
#endif