#ifndef __GMDX11RENDERERS_H__
#define __GMDX11RENDERERS_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
BEGIN_NS

class GMDx11Renderer_CubeMap;
class GMModel;
struct GMDx11CubeMapState
{
	bool hasCubeMap = false;
	GMDx11Renderer_CubeMap* cubeMapRenderer = nullptr;
	GMModel* model = nullptr;
};

struct GMTextureAttributeBank
{
	ID3DX11EffectScalarVariable* enabled = nullptr;
	ID3DX11EffectScalarVariable* offsetX = nullptr;
	ID3DX11EffectScalarVariable* offsetY = nullptr;
	ID3DX11EffectScalarVariable* scaleX = nullptr;
	ID3DX11EffectScalarVariable* scaleY = nullptr;
};

GM_PRIVATE_OBJECT(GMDx11Renderer)
{
	GMComPtr<ID3D11InputLayout> inputLayout;
	GMComPtr<ID3DX11Effect> effect;
	ID3D11DeviceContext* deviceContext = nullptr;
	ID3DX11EffectTechnique* technique = nullptr;
	ID3DX11EffectRasterizerVariable* rasterizer = nullptr;
	ID3DX11EffectBlendVariable* blend = nullptr;
	ID3DX11EffectDepthStencilVariable* depthStencil = nullptr;
	HashMap<const char*, ID3DX11EffectVariable*> textureAttributes;
	HashMap<const char*, GMTextureAttributeBank> textureVariables;
	const GMShaderVariablesDesc* variablesDesc = nullptr;
	bool screenInfoPrepared = false;
	GMComPtr<ID3D11Resource> shadowMapResource;
	GMDx11GraphicEngine* engine = nullptr;
	GMfloat gamma = 0;
};

class GMDx11Renderer : public GMObject, public IRenderer
{
	DECLARE_PRIVATE(GMDx11Renderer)

public:
	GMDx11Renderer();

public:
	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
	virtual void endModel() override;
	virtual void draw(GMModel* model) override;
	virtual const char* getTechniqueName() = 0;

public:
	inline ID3DX11Effect* getEffect()
	{
		D(d);
		return d->effect;
	}

	inline const GMShaderVariablesDesc* getVariablesDesc()
	{
		D(d);
		if (!d->variablesDesc)
		{
			d->variablesDesc = &getEngine()->getShaderProgram()->getDesc();
		}
		return d->variablesDesc;
	}

protected:
	inline GMDx11GraphicEngine* getEngine()
	{
		D(d);
		if (!d->engine)
			d->engine = gm_cast<GMDx11GraphicEngine*>(GM.getGraphicEngine());
		return d->engine;
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
	virtual void applyTextureAttribute(GMModel* model, ITexture* texture, GMTextureType type);

	ID3DX11EffectTechnique* getTechnique();
	ITexture* getTexture(GMTextureSampler& sampler);
	void setGamma(IShaderProgram* shaderProgram);

public:
	static GMDx11CubeMapState& getCubeMapState();
};

class GMDx11Renderer_3D : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D";
	}
};

class GMDx11Renderer_2D : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_2D";
	}

	virtual void prepareTextures(GMModel* model);
};

class GMDx11Renderer_Glyph : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Glyph";
	}
};

class GMDx11Renderer_CubeMap : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_CubeMap";
	}

public:
	virtual void prepareTextures(GMModel* model);
};

GM_PRIVATE_OBJECT(GMDx11Renderer_Filter)
{
	struct HDRState
	{
		GMToneMapping::Mode toneMapping = GMToneMapping::Reinhard;
		bool HDR = false;
	};
	HDRState state;
};

class GMDx11Renderer_Filter : public GMDx11Renderer
{
	DECLARE_PRIVATE_AND_BASE(GMDx11Renderer_Filter, GMDx11Renderer)

public:
	GMDx11Renderer_Filter();

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

class GMDx11Renderer_Deferred_3D: public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D";
	}

	virtual void passAllAndDraw(GMModel* model);
};

class GMDx11Renderer_Deferred_3D_LightPass : public GMDx11Renderer
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_Deferred_3D_LightPass";
	}

	virtual void prepareMaterials(GMModel* model) override {}
	virtual void prepareTextures(GMModel* model) override;

private:
	void passAllAndDraw(GMModel* model);
};

class GMDx11Renderer_3D_Shadow : public GMDx11Renderer_3D
{
	virtual const char* getTechniqueName() override
	{
		return "GMTech_3D_Shadow";
	}

	virtual void beginModel(GMModel* model, const GMGameObject* parent) override;
};

END_NS
#endif