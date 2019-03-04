#ifndef __GMDX11TECHNIQUES_H__
#define __GMDX11TECHNIQUES_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
BEGIN_NS

class GMDx11Technique_CubeMap;
class GMModel;
class GMDx11EffectVariableBank;
struct GMDx11CubeMapState;
struct GMDx11RasterizerStates;
struct GMDx11BlendStates;
struct GMDx11DepthStencilStates;

struct GMTextureAttributeBank
{
	ID3DX11EffectScalarVariable* enabled = nullptr;
	ID3DX11EffectScalarVariable* offsetX = nullptr;
	ID3DX11EffectScalarVariable* offsetY = nullptr;
	ID3DX11EffectScalarVariable* scaleX = nullptr;
	ID3DX11EffectScalarVariable* scaleY = nullptr;
};

GM_PRIVATE_OBJECT(GMDx11Technique)
{
	struct TechniqueContext
	{
		GMModel* currentModel = nullptr;
		GMScene* currentScene = nullptr;
	};

	TechniqueContext techContext;
	const IRenderContext* context = nullptr;
	GMTextureAsset whiteTexture;
	GMOwnedPtr<GMDx11RasterizerStates> rasterizerStates;
	GMOwnedPtr<GMDx11BlendStates> blendStates;
	GMOwnedPtr<GMDx11DepthStencilStates> depthStencilStates;
	GMOwnedPtr<GMDx11EffectVariableBank> bank;

	GMComPtr<ID3D11InputLayout> inputLayout;
	GMComPtr<ID3DX11Effect> effect;
	GMDebugConfig debugConfig;
	ID3D11DeviceContext* deviceContext = nullptr;
	ID3DX11EffectTechnique* technique = nullptr;
	ID3DX11EffectRasterizerVariable* rasterizer = nullptr;
	ID3DX11EffectBlendVariable* blend = nullptr;
	ID3DX11EffectDepthStencilVariable* depthStencil = nullptr;
	bool screenInfoPrepared = false;
	GMComPtr<ID3D11Resource> shadowMapResource;
	GMDx11GraphicEngine* engine = nullptr;
	GMfloat gamma = 0;
	Map<ID3DX11EffectVariable*, GMTextureAttributeBank> textureVariables;
	GMShaderVariablesIndices indexBank = { 0 };
};

class GMDx11Technique : public GMObject, public ITechnique
{
	GM_DECLARE_PRIVATE(GMDx11Technique)

public:
	GMDx11Technique(const IRenderContext* context);
	~GMDx11Technique() = default;

public:
	virtual void beginScene(GMScene* scene) override;
	virtual void endScene() override;
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void draw(GMModel* model) override;
	virtual const char* getTechniqueName() = 0;
	virtual const IRenderContext* getContext();

public:
	inline ID3DX11Effect* getEffect()
	{
		D(d);
		return d->effect;
	}

protected:
	inline GMDx11GraphicEngine* getEngine()
	{
		D(d);
		if (!d->engine)
			d->engine = gm_cast<GMDx11GraphicEngine*>(d->context->getEngine());
		return d->engine;
	}

	inline GMModel* getCurrentModel()
	{
		D(d);
		return d->techContext.currentModel;
	}

protected:
	virtual void prepareScreenInfo();
	virtual void prepareTextures(GMModel* model);
	virtual void passAllAndDraw(GMModel* model);
	virtual void prepareBuffer(GMModel* model);
	virtual void prepareLights();
	virtual void prepareMaterials(GMModel* model);
	virtual void prepareRasterizer(GMModel* model);
	virtual void prepareBlend(GMModel* model);
	virtual void prepareDepthStencil(GMModel* model);
	virtual void prepareDebug(GMModel* model);
	virtual void applyTextureAttribute(GMModel* model, GMTextureAsset texture, GMTextureType type);
	virtual ID3DX11EffectTechnique* getTechnique();
	GMTextureAsset getTexture(GMTextureSampler& sampler);
	void setGamma(IShaderProgram* shaderProgram);
	GMDx11EffectVariableBank& getVarBank();

private:
	GMTextureAsset getWhiteTexture();
	void updateBoneTransforms(IShaderProgram* shaderProgram, GMModel* model);

public:
	static const std::string& getTechniqueNameByTechniqueId(GMRenderTechinqueID id);
};

class GMDx11Technique_3D : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D";
	}
};

class GMDx11Technique_2D : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_2D";
	}

	virtual void prepareTextures(GMModel* model);
};

class GMDx11Technique_Text : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Text";
	}
};

class GMDx11Technique_CubeMap : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_CubeMap";
	}

public:
	virtual void prepareTextures(GMModel* model);
};

GM_PRIVATE_OBJECT(GMDx11Technique_Filter)
{
	struct HDRState
	{
		GMToneMapping::Mode toneMapping = GMToneMapping::Reinhard;
		bool HDR = false;
	};
	HDRState state;
};

class GMDx11Technique_Filter : public GMDx11Technique
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11Technique_Filter, GMDx11Technique)

public:
	GMDx11Technique_Filter(const IRenderContext* context);

private:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Filter";
	}

	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void draw(GMModel* model) override;
	virtual void passAllAndDraw(GMModel* model) override;

	void setHDR(IShaderProgram* shaderProgram);
};

class GMDx11Technique_Deferred_3D: public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D";
	}

	virtual void passAllAndDraw(GMModel* model);
};

class GMDx11Technique_Deferred_3D_LightPass : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D_LightPass";
	}

	virtual void prepareMaterials(GMModel* model) override {}
	virtual void prepareTextures(GMModel* model) override;

private:
	void passAllAndDraw(GMModel* model);
};

GM_PRIVATE_OBJECT(GMDx11Technique_3D_Shadow)
{
	GMCamera shadowCameras[GMGraphicEngine::getMaxCascades()];
};

class GMDx11Technique_3D_Shadow : public GMDx11Technique_3D, public ICSMTechnique
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11Technique_3D_Shadow, GMDx11Technique_3D)

public:
	using GMDx11Technique_3D::GMDx11Technique_3D;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D_Shadow";
	}

	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;

public:
	virtual void setCascadeCamera(GMCascadeLevel level, const GMCamera& camera) override;
	virtual void setCascadeEndClip(GMCascadeLevel level, GMfloat endClip) override;
};

class GMDx11Technique_Particle : public GMDx11Technique
{
public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Particle";
	}
};

GM_PRIVATE_OBJECT(GMDx11Technique_Custom)
{
	Vector<GMString> techniqueNames;
};

class GMDx11Technique_Custom : public GMDx11Technique
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11Technique_Custom, GMDx11Technique)

public:
	using GMDx11Technique::GMDx11Technique;

protected:
	virtual ID3DX11EffectTechnique* getTechnique() override;

protected:
	virtual const char* getTechniqueName() override;
};

END_NS
#endif