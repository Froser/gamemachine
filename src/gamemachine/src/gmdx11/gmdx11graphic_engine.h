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
GM_PRIVATE_CLASS(GMDx11GraphicEngine);
class GMDx11GraphicEngine : public GMGraphicEngine
{
	GM_DECLARE_PRIVATE(GMDx11GraphicEngine)
	GM_DECLARE_BASE(GMGraphicEngine)

public:
	GMDx11GraphicEngine(const IRenderContext* context);
	~GMDx11GraphicEngine();

public:
	virtual void init() override;
	virtual void update(GMUpdateDataType type) override;
	virtual IShaderProgram* getShaderProgram(GMShaderProgramType type = GMShaderProgramType::DefaultShaderProgram) override;
	virtual bool msgProc(const GMMessage& e) override;
	virtual IFramebuffers* getDefaultFramebuffers() override;
	virtual ITechnique* getTechnique(GMModelType objectType) override;
	virtual GMGlyphManager* getGlyphManager() override;

public:
	virtual bool setInterface(GameMachineInterfaceID, void*);
	virtual bool getInterface(GameMachineInterfaceID, void**);

public:
	virtual void activateLights(ITechnique* technique);

public:
	GMDx11CubeMapState& getCubeMapState();
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getDeviceContext();
	IDXGISwapChain* getSwapChain();
	ID3D11DepthStencilView* getDepthStencilView();
	ID3D11RenderTargetView* getRenderTargetView();
	const GMVec2 getCurrentFilterKernelDelta();
};

END_NS
#endif