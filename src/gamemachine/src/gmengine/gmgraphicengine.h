#ifndef __GMGRAPHICENGINE_H__
#define __GMGRAPHICENGINE_H__
#include <gmcommon.h>
#include <gmtools.h>
#include <gmmodel.h>
#include <gmcamera.h>
#include <gmrendertechnique.h>
BEGIN_NS

template <typename T>
ITechnique* newTechnique(GMOwnedPtr<ITechnique>& ptr, const IRenderContext* context)
{
	if (!ptr)
		ptr = gm_makeOwnedPtr<T>(context);
	return ptr.get();
}

#define GM_VariablesDesc (GMGraphicEngine::getDefaultShaderVariablesDesc())

template <typename T>
struct GMShaderVariablesTextureDesc
{
	T OffsetX;
	T OffsetY;
	T ScaleX;
	T ScaleY;
	T Enabled;
	T Texture;
};

template <typename T>
struct GMShaderVariablesLightDesc
{
	T Name;
	T Count;
};

template <typename T>
struct GMShaderVariablesMaterialDesc
{
	T Ka;
	T Kd;
	T Ks;
	T Shininess;
	T Refreactivity;
	T F0;
};

constexpr int GMFilterCount = 6;
template <typename T>
struct GMShaderVariablesFilterDesc
{
	T Filter;
	T KernelDeltaX;
	T KernelDeltaY;
	T Types[GMFilterCount];
};

template <typename T>
struct GMShaderVariablesScreenInfoDesc
{
	T ScreenInfo;
	T ScreenWidth;
	T ScreenHeight;
	T Multisampling;
};

template <typename T>
struct GMShaderVariableShadowInfoDesc
{
	T ShadowInfo;
	T HasShadow;
	T ShadowMatrix;
	T Position;
	T ShadowMap;
	T ShadowMapMSAA;
	T ShadowMapWidth;
	T ShadowMapHeight;
	T BiasMin;
	T BiasMax;
	T CascadedShadowLevel;
};

template <typename T>
struct GMShaderVariableGammaDesc
{
	T GammaCorrection;
	T GammaValue;
	T GammaInvValue;
};

template <typename T>
struct GMShaderVariableHDRDesc
{
	T HDR;
	T ToneMapping;
};

template <typename T>
struct GMShaderVariableDebugDesc
{
	T Normal;
};

template <typename T>
struct GMShaderVariablesDesc_t
{
	// 矩阵
	T ModelMatrix;
	T ViewMatrix;
	T ProjectionMatrix;
	T InverseTransposeModelMatrix;
	T InverseViewMatrix;

	// 骨骼
	T Bones;
	T UseBoneAnimation;

	// 位置
	T ViewPosition;

	// 材质
	GMShaderVariablesTextureDesc<T> TextureAttributes;
	T AmbientTextureName;
	T DiffuseTextureName;
	T SpecularTextureName;
	T NormalMapTextureName;
	T LightMapTextureName;
	T AlbedoTextureName;
	T MetallicRoughnessAOTextureName;
	T CubeMapTextureName;

	// 光照
	T LightCount;

	// 材质
	GMShaderVariablesMaterialDesc<T> MaterialAttributes;
	T MaterialName;

	// 滤镜
	GMShaderVariablesFilterDesc<T> FilterAttributes;

	// 状态
	GMShaderVariablesScreenInfoDesc<T> ScreenInfoAttributes;
	T RasterizerState;
	T BlendState;
	T DepthStencilState;

	// 阴影
	GMShaderVariableShadowInfoDesc<T> ShadowInfo;

	// Gamma校正
	GMShaderVariableGammaDesc<T> GammaCorrection;

	// HDR
	GMShaderVariableHDRDesc<T> HDR;

	// 模型
	T IlluminationModel;
	T ColorVertexOp;

	// 调试
	GMShaderVariableDebugDesc<T> Debug;
};

using GMShaderVariablesDesc = GMShaderVariablesDesc_t<const GMString>;
using GMShaderVariablesIndices = GMShaderVariablesDesc_t<GMint32>;

struct GMShadowSourceDesc
{
	enum Type
	{
		NoShadow,
		DirectShadow,
		CascadedShadow,
	};

	Type type;
	GMCamera camera;
	GMVec4 position;
	float biasMin = 0.0005f;
	float biasMax = 0.005f;
	GMint32 width;
	GMint32 height;
	GMint32 cascadedShadowLevel = 1; //!< GameMachine将构造出一副(width*cascadedLevel, height)的纹理来记录shadow map。如果cascadedLevel为1，那么就是普通的阴影贴图。否则，它就是CSM方式的阴影贴图。
	static GMint64 version;
};

struct GMGlobalBlendStateDesc
{
	bool enabled = false;
	GMS_BlendFunc sourceRGB;
	GMS_BlendFunc destRGB;
	GMS_BlendOp opRGB;
	GMS_BlendFunc sourceAlpha;
	GMS_BlendFunc destAlpha;
	GMS_BlendOp opAlpha;
	GMint32 blendRefCount = 0;
};

GM_PRIVATE_OBJECT(GMFramebuffersStack)
{
	Stack<IFramebuffers*> framebuffers;
};

class GMFramebuffersStack : public GMObject
{
	GM_DECLARE_PRIVATE(GMFramebuffersStack);

public:
	void push(IFramebuffers* framebuffers);
	IFramebuffers* pop();
	IFramebuffers* peek();
};

GM_PRIVATE_OBJECT(GMGraphicEngine)
{
	const IRenderContext* context = nullptr;
	GMCamera camera;
	GMGlyphManager* glyphManager = nullptr;
	IFramebuffers* defaultFramebuffers = nullptr;
	IFramebuffers* filterFramebuffers = nullptr;
	GMGameObject* filterQuad = nullptr;
	GMFramebuffersStack framebufferStack;
	IGBuffer* gBuffer = nullptr;
	GMRenderConfig renderConfig;
	GMDebugConfig debugConfig;
	GMStencilOptions stencilOptions;
	Vector<ILight*> lights;
	IShaderLoadCallback* shaderLoadCallback = nullptr;
	GMGlobalBlendStateDesc blendState;
	GMOwnedPtr<GMRenderTechniqueManager> renderTechniqueManager;
	GMOwnedPtr<GMPrimitiveManager> primitiveManager;

	// Shadow
	GMShadowSourceDesc shadow;
	IFramebuffers* shadowDepthFramebuffers = nullptr;
	bool isDrawingShadow = false;
};

class GMGraphicEngine : public GMObject, public IGraphicEngine
{
	GM_DECLARE_PRIVATE(GMGraphicEngine)

public:
	GMGraphicEngine(const IRenderContext* context);
	~GMGraphicEngine();

public:
	virtual void init() override;
	virtual IGBuffer* getGBuffer() override;
	virtual IFramebuffers* getFilterFramebuffers() override;
	virtual void draw(const List<GMGameObject*>& forwardRenderingObjects, const List<GMGameObject*>& deferredRenderingObjects) override;
	virtual GMLightIndex addLight(AUTORELEASE ILight* light) override;
	virtual ILight* getLight(GMLightIndex index) override;
	virtual void removeLights() override;
	virtual void setStencilOptions(const GMStencilOptions& options) override;
	virtual const GMStencilOptions& getStencilOptions() override;
	virtual void setShaderLoadCallback(IShaderLoadCallback* cb) override;
	virtual void setShadowSource(const GMShadowSourceDesc& desc) override;
	virtual GMCamera& getCamera() override;
	virtual void setCamera(const GMCamera&) override;
	virtual void beginBlend(
		GMS_BlendFunc sfactorRGB,
		GMS_BlendFunc dfactorRGB,
		GMS_BlendOp opRGB,
		GMS_BlendFunc sfactorAlpha,
		GMS_BlendFunc dfactorAlpha,
		GMS_BlendOp opAlpha
	) override;
	virtual void endBlend() override;
	virtual GMRenderTechniqueManager* getRenderTechniqueManager() override;
	virtual GMPrimitiveManager* getPrimitiveManager() override;
	virtual void createModelDataProxy(const IRenderContext* context, GMModel* model, bool transfer = true) override;

protected:
	virtual void createShadowFramebuffers(OUT IFramebuffers** framebuffers) = 0;
	virtual void createFilterFramebuffer();
	virtual void generateShadowBuffer(const List<GMGameObject*>& forwardRenderingObjects, const List<GMGameObject*>& deferredRenderingObjects);
	virtual bool needUseFilterFramebuffer();
	virtual void bindFilterFramebufferAndClear();
	virtual void unbindFilterFramebufferAndDraw();

public:
	const GMFilterMode::Mode getCurrentFilterMode();

	void draw(const List<GMGameObject*>& objects);
	IFramebuffers* getShadowMapFramebuffers();

	bool needGammaCorrection();
	GMfloat getGammaValue();

	bool needHDR();
	GMToneMapping::Mode getToneMapping();

	bool isWireFrameMode(GMModel* model);
	bool isNeedDiscardTexture(GMModel* model, GMTextureType type);

public:
	inline GMFramebuffersStack& getFramebuffersStack()
	{
		D(d);
		return d->framebufferStack;
	}

protected:
	inline GMGameObject* getFilterQuad()
	{
		D(d);
		return d->filterQuad;
	}

	inline IShaderLoadCallback* getShaderLoadCallback()
	{
		D(d);
		return d->shaderLoadCallback;
	}

	inline const Vector<ILight*>& getLights()
	{
		D(d);
		return d->lights;
	}

private:
	IGBuffer* createGBuffer();

public:
	static constexpr const GMsize_t getMaxLightCount()
	{
		return 50;
	}
	static const GMShaderVariablesDesc& getDefaultShaderVariablesDesc();

	inline const GMShadowSourceDesc& getShadowSourceDesc()
	{
		D(d);
		return d->shadow;
	}

	inline bool isDrawingShadow()
	{
		D(d);
		return d->isDrawingShadow;
	}

	inline const GMGlobalBlendStateDesc& getGlobalBlendState()
	{
		D(d);
		return d->blendState;
	}
};

END_NS
#endif