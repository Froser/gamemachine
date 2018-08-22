#ifndef __GMDX11GRAPHIC_ENGINE_H__
#define __GMDX11GRAPHIC_ENGINE_H__
#include <gmcommon.h>
#include <gmcom.h>
#include <gmdxincludes.h>
#include <gmmodel.h>
#include <gmtools.h>
#include <gmgraphicengine.h>
BEGIN_NS

class GMDx11Technique_CubeMap;
struct GMMVPMatrix;

struct GMDx11CubeMapState
{
	bool hasCubeMap = false;
	GMDx11Technique_CubeMap* cubeMapTechnique = nullptr;
	GMModel* model = nullptr;
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
	GMScopedPtr<IShaderProgram> shaderProgram;
	GMDx11CubeMapState cubemapState;

	bool ready = false;
	bool lightDirty = true;

	GMOwnedPtr<ITechnique> technique_3d;
	GMOwnedPtr<ITechnique> technique_2d;
	GMOwnedPtr<ITechnique> technique_text;
	GMOwnedPtr<ITechnique> technique_cubemap;
	GMOwnedPtr<ITechnique> technique_filter;
	GMOwnedPtr<ITechnique> technique_deferred_3d;
	GMOwnedPtr<ITechnique> technique_deferred_3d_lightpass;
	GMOwnedPtr<ITechnique> technique_3d_shadow;
	GMOwnedPtr<ITechnique> technique_particle;
	GMOwnedPtr<ITechnique> technique_custom;
};

class GMDx11GraphicEngine : public GMGraphicEngine
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11GraphicEngine, GMGraphicEngine)

public:
	GMDx11GraphicEngine(const IRenderContext* context);
	~GMDx11GraphicEngine() = default;

public:
	virtual void init() override;
	virtual void update(GMUpdateDataType type) override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::DefaultShaderProgram) override;
	virtual bool event(const GMMessage& e) override;
	virtual IFramebuffers* getDefaultFramebuffers() override;
	virtual ITechnique* getTechnique(GMModelType objectType) override;
	virtual GMGlyphManager* getGlyphManager() override;

public:
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual bool getInterface(GameMachineInterfaceID, void**);
	virtual void createShadowFramebuffers(OUT IFramebuffers** framebuffers) override;

public:
	virtual void activateLights(ITechnique* technique);

public:
	inline GMDx11CubeMapState& getCubeMapState()
	{
		D(d);
		return d->cubemapState;
	}

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

	const GMVec2 getCurrentFilterKernelDelta()
	{
		D_BASE(d, Base);
		return d->renderConfig.get(GMRenderConfigs::FilterKernelOffset_Vec2).toVec2();
	}

private:
	void initShaders(const IRenderContext* context);
};

END_NS
#endif